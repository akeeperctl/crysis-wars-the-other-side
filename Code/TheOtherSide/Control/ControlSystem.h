# pragma once

#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IVehicleSystem.h>

#include "IControlSystemChild.h"
#include "Item.h"
#include "Actor.h"
#include "Nanosuit.h"
#include "HUD/GameFlashAnimation.h"

//Trooper timers of he actions
#define TROOPER_JUMP_AFTER_DODGE_REST_TIME 1.0f

#define TROOPER_DODGE_REST_TIME 0.7f
#define TROOPER_MELEE_REST_TIME 1.20f
#define TROOPER_MELEE_JUMP_REST_TIME 2.0f

#define TROOPER_MELEE_DAMAGE_DISTANCE 2.5f
#define TROOPER_MELEE_JUMP_DAMAGE_DISTANCE 3.0f

#define TROOPER_MELEE_ENERGY_COST 25.0f
#define TROOPER_MELEE_JUMP_ENERGY_COST 60.0f
#define TROOPER_JUMP_ENERGY_COST 50.0f

//Scout timers of he actions
#define SCOUT_DODGE_REST_TIME 2.0f

#define GET_ENTITY(entityId) gEnv->pEntitySystem->GetEntity(entityId)

#define GET_LOCATION_FROM_GROUND(pActor, inputPos, height)\
	ray_hit hit;\
	IPhysicalEntity* pSkipEntities[10];\
	int nSkip = 0;\
	auto pItem = (pActor)->GetCurrentItem();\
	if (pItem)\
	{\
		auto pWeapon = (CWeapon*)pItem->GetIWeapon();\
		if (pWeapon)\
			nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);\
	}\
	\
	auto castPos = inputPos;\
	castPos.z -= 0.25f;\
	\
	const auto castDir = (castPos - (inputPos)).GetNormalizedSafe() * 2000.f;\
	\
	if (gEnv->pPhysicalWorld->RayWorldIntersection(inputPos, castDir, ent_terrain | ent_static,\
		rwi_ignore_noncolliding | rwi_stop_at_pierceable, &hit, 1, pSkipEntities, nSkip))\
	{\
		if (hit.pCollider)\
		{\
			if (hit.dist < (height))\
			{\
				(inputPos).z += (height) - hit.dist;\
			}\
			else if (hit.dist > (height))\
			{\
				(inputPos).z -= hit.dist - (height);\
			}\
		}\
	}\

#define GET_SAFEFLY_LOCATION_FROM_TARGET(pActor, castPos, targetPos, height)\
	ray_hit hit;\
	IPhysicalEntity* pSkipEntities[10];\
	int nSkip = 0;\
	auto pItem = (pActor)->GetCurrentItem();\
	if (pItem)\
	{\
		auto pWeapon = (CWeapon*)pItem->GetIWeapon();\
		if (pWeapon)\
			nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);\
	}\
	\
	const auto castDir = ((targetPos) - (castPos));\
	\
	if (gEnv->pPhysicalWorld->RayWorldIntersection(castPos, castDir, ent_terrain | ent_static,\
		rwi_ignore_noncolliding | rwi_stop_at_pierceable, &hit, 1, pSkipEntities, nSkip))\
	{\
		auto pColl = hit.pCollider;\
		if (pColl)\
		{\
			(targetPos) = hit.pt;\
			(targetPos).z += (height);\
		}\
	}\
	else\
	{\
		(targetPos).z += (height);\
	}\

namespace TOS_Distance
{
	inline bool IsBigger(const IVehicle* pVehicle, const IEntity* pRef, float threshold)
	{
		if (!pVehicle)
			return false;

		if (!pRef)
		{
			CryLogAlways("%s[C++][TOS_Distance::IsBigger][ERROR][pVehicle: %s][pRef not defined]",
				STR_RED, pVehicle->GetEntity()->GetName());
			return false;
		}

		const auto vehPos = pVehicle->GetEntity()->GetWorldPos();
		const auto refPos = pRef->GetWorldPos();

		const float dist = (refPos - vehPos).GetLength();
		//CryLogAlways("dist %1.f", dist);

		return dist > threshold;
	}

	inline bool IsBigger(const IActor* pActor, const IEntity* pRef, float threshold)
	{
		if (!pActor)
			return false;

		if (!pRef)
			return false;

		const auto actPos = pActor->GetEntity()->GetWorldPos();
		const auto refPos = pRef->GetWorldPos();

		const float dist = (refPos - actPos).GetLength();
		//CryLogAlways("dist %1.f", dist);

		return dist > threshold;
	}

	inline bool IsSmaller(const IActor* pActor, const IEntity* pRef, float threshold)
	{
		if (!pActor)
			return false;

		if (!pRef)
			return false;

		const auto actPos = pActor->GetEntity()->GetWorldPos();
		const auto refPos = pRef->GetWorldPos();

		const float dist = (refPos - actPos).GetLength();
		//CryLogAlways("dist %1.f", dist);

		return dist <= threshold;
	}	
	
	inline bool IsSmaller(const IVehicle* pVehicle, const IEntity* pRef, float threshold)
	{
		if (!pVehicle)
			return false;

		if (!pRef)
			return false;

		const auto vehPos = pVehicle->GetEntity()->GetWorldPos();
		const auto refPos = pRef->GetWorldPos();

		const float dist = (refPos - vehPos).GetLength();
		//CryLogAlways("dist %1.f", dist);

		return dist <= threshold;
	}
}

constexpr auto VEHICLE_AIR_ENTER_THRESHOLD_DIST = 15.0f;
constexpr auto VEHICLE_LAND_ENTER_THRESHOLD_DIST = 4.0f;

class CAIActionTracker;
class CControlClient;
class CSquadSystem;
class CGameFlashAnimation;
class CConquerorSystem;
class CAbilitiesSystem;
class CVehicleMovementBase;

struct IHardwareMouseEventListener;
struct IHitListener;

struct SGenericControlParams
{
	SGenericControlParams()
	{
		Reset();
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		isAiming = false;
		isShooting = false;
		canShoot = true;
		canUseLeftAbility = true;
		canUseCenterAbility = true;
		canUseRightAbility = true;

		meleeTargetIsFriend = false;
		fireTargetIsFriend = false;
		cannotMove = false;
		cannotLookAt = false;
		isTargetHaveAI = false;
		isTutorialMode = false;
		isUsingBinocular = false;

		zoomScale = 0;
		jumpCount = 0;

		canMoveMult = 1;
		fSoundRestTimer = 0;

		moveDir = Vec3(0, 0, 0);
	}

	bool isAiming;
	bool isShooting;
	bool cannotMove;
	bool cannotLookAt;
	bool fireTargetIsFriend;
	bool meleeTargetIsFriend;
	bool canShoot;
	bool isTargetHaveAI;
	bool canUseLeftAbility;
	bool canUseCenterAbility;
	bool canUseRightAbility;
	bool isTutorialMode;
	bool isUsingBinocular;

	float fSoundRestTimer;
	float canMoveMult;

	int zoomScale;
	int jumpCount;

	Vec3 moveDir;
};

struct SHunterControlParams
{
	SHunterControlParams()
	{
		//Reset();
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
	}
};

struct SScoutControlParams
{
	SScoutControlParams()
	{
		speedMult = 0.6f;

		modelQuat.SetIdentity();

		doDodge = false;
		canDodge = true;
		targetLocked = false;
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		speedMult = 0.6f;

		modelQuat.SetIdentity();

		doDodge = false;
		canDodge = true;
		targetLocked = false;
	}

	float		speedMult;

	bool		canDodge;
	bool		doDodge;
	bool		targetLocked;

	Quat		modelQuat;
};

struct STrooperControlParams
{
	STrooperControlParams()
	{
		canDodge = true;
		isBinocular = false;
		isAiming = false;
		isSprinting = false;

		canJumpMelee = true;
		doJumpMelee = false;

		canMelee = true;
		doMelee = false;

		speedMult = 1.0f;
		sprintMult = 1.0f;
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		canDodge = true;
		isBinocular = false;
		isAiming = false;
		isSprinting = false;

		canJump = true;
		canJumpMelee = true;
		doJumpMelee = false;

		canMelee = true;
		doMelee = false;

		speedMult = 1.0f;
		sprintMult = 1.0f;
	}

	float		speedMult; // 0.0...1.0
	float		sprintMult;
	bool		isAiming;
	bool		isBinocular;
	bool		isSprinting;
	bool		canDodge;
	bool		canJump;
	bool		canJumpMelee;
	bool		doJumpMelee;
	bool		canMelee;
	bool		doMelee;
};

struct SAlienControlParams
{
	SAlienControlParams()
	{
		aimSpeedMult = 1.0f;
		speedMult = 1.0f;

		modelQuat.SetIdentity();
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		aimSpeedMult = 1.0f;
		speedMult = 1.0f;

		modelQuat.SetIdentity();
	}

	float		speedMult;
	float		aimSpeedMult;

	Quat		modelQuat;
};



class CControlClient : public IHitListener
{	//Methods
public:
	friend class CGameRules;
	friend class CControlSystem;
	friend class CNetControlClient;
	friend class CSquadSystem;
	friend class CConquerorSystem;
	friend class CPlayerInput;

	friend class CAbilitiesSystem;
	friend class CAbilityOwner;

	friend class CSquad;
	friend class CMember;


	explicit CControlClient(CPlayer* _player);
	~CControlClient();

	//IHitListener
	void OnHit(const HitInfo&) override;
	void OnExplosion(const ExplosionInfo&) override;
	void OnServerExplosion(const ExplosionInfo&) override;
	//~IHitListener

	void		ApplyMovement(const Vec3& delta);
	void		FullSerialize(TSerialize ser);

	//void		TestFunction();

	void		Update();
	void		UpdateView(SViewParams& viewParams);
	void		UpdateUsability(const IEntity* pTarget) const;
	void		UpdateCrosshair();
	void		UpdateScout();
	void		UpdateTrooper();
	void		UpdateHunter() const;

	static void		SetDudeSpecies(const int species);
	void		SetActor(CActor* act);

	void		ToggleDudeHide(const bool toggle);
	void		ToggleDudeBeam(const bool toggle);
	//Tutorial mode - its mode when the controlled actor will be revive after he die and squad system will not be change controlled actor.
	void		ToggleTutorialMode(const bool mode);

	CActor*		GetControlledActor() const { return m_pControlledActor; };
	IEntity*	GetControlledEntity() const
	{ if (m_pControlledActor) { return m_pControlledActor->GetEntity(); } return nullptr; };

	//Get the class name from the current controlled actor
	string		GetActorClassName() const { if (m_pControlledActor) return m_pControlledActor->GetEntity()->GetClass()->GetName(); else return ""; };

	//Get the current item class name from the current controlled actor
	string		GetItemClassName() const { if (m_pControlledActor) { if (m_pControlledActor->GetInventory()) { const EntityId pItemId = m_pControlledActor->GetInventory()->GetCurrentItem(); if (pItemId) { const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pItemId); if (pItem) { return pItem->GetEntity()->GetClass()->GetName(); } else return ""; } else return ""; } else return ""; } else return ""; };

	//Tutorial mode - its mode when controlled actor will be revive when he die and squad system will not be change controlled actor when he die.
	bool		GetTutorialMode() const;

	//Get the pointer to the material of the current controlled actor
	IMaterial*	GetMaterial() const;

	//Get the current crosshair's position vector in the world space
	Vec3		GetCrosshairPos() const { return m_crosshairPos; };
	Vec3		GetMeleeHitPos() const { return m_meleeRayhit.pt; };
	float		GetMeleeHits() const { return m_meleeHits; };

	static CWeapon* GetCurrentWeapon(const CActor* pActor);

	std::map<ActionId, int> GetActions() { return m_actionsMap; };

	//Get structures with variables of the system and controlled actor
	SGenericControlParams& GetGenericParams() { return m_generic; };
	SScoutControlParams& GetScoutParams() { return m_scout; };
	SAlienControlParams& GetNakedParams() { return m_nakedAlien; };
	SHunterControlParams& GetHunterParams() { return m_hunter; };
	STrooperControlParams& GetTrooperParams() { return m_trooper; };
	
	//These functions allows us to find out the type of controlled actor
	bool		IsNakedAlien() const { return GetActorClassName() == "Alien"; }
	bool		IsTrooper() const { return GetActorClassName() == "Trooper"; }
	bool		IsHunter() const { return GetActorClassName() == "Hunter"; }
	bool		IsScout() const { return GetActorClassName() == "Scout"; }

private:

	//Events
	void		OnChangedSpectatorMode(const IActor* pActor, uint8 mode, EntityId targetId, bool resetAll);
	void		OnClientHit(const HitInfo&) const;
	void		OnActorDeath(IActor* pActor);
	void		OnAction(const ActionId& action, int activationMode, float value);//Called  from PlayerInput.cpp
	//~Events

	void		SubEnergy(const float subtractValue) const;

	bool		OnActionViewLock(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionMoveBack(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	void		OnActionCrouch(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionJump(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionSprint(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionBinoculars(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionAttack(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionAim(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionLeanLeft(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionLeanRight(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionNextItem(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionPrevItem(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionSpeedUp(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionSpeedDown(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionZoomIn(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionZoomOut(string classname, const ActionId& action, int activationMode, float value);
	void		OnActionUse(string classname, const ActionId& action, int activationMode, float value);

	//Storing the current time in the variable
	//See also CheckPassTime();
	void		StoreCurrTime();

	//Checks how much time has passed from last time storing
	//See also StoreCurrTime();
	bool		CheckPassTime(float passedSec) const;

	//Resetting the structures with stored variables of the system and controlled actor
	void		ResetParams();

	//Resetting the current controlled actor to the normal state i.e before having owner
	void		Reset(bool toEditor);

	//Preparing a dude actor (Nomad) for to be in a controlling state
	void		InitDudeToControl(bool toLink);

	//Un/Registering the AI of controlled actor to according with the AI type of the dude player
	void		SetActorAI(const IActor* pActor, bool bToPlayerAI) const;

	//Preparing a new actor to enter in the controlled state
	bool		DoNextActor(CActor* pNextActor);

	//Preparing a old actor to enter in the uncontrolled state
	bool		DoPrevActor(CActor* pActor) const;

	//Get the entity which being in the ray with length == 3 from the weapon's position to crosshair point position
	IEntity*	GetMeleeTarget() const;

	//Get the current entity under the crosshair
	IEntity*	GetLastCrosshairEntity() const;

	//Get the current entity under the crosshair
	IEntity*	GetCrosshairEntity() const;

	//Get the current entity under the crosshair
	EntityId	GetCrosshairEntityId() const;

	EntityId	GetScoutAutoAimTargetId() const;

	//Get the entity which being in the ray with length == 5000 from weapon's position to crosshair point position
	IEntity*	GetFireTarget() const;

	static void		SetInventoryHUD(IActor* pActor, const char* file);
	static void		SetAmmoHealthHUD(IActor* pActor, const char* file);
	IAIObject* GetCrosshairTargetAI() const;
	void		SendPipeToAIGroup(const char* name, int groupId, bool useInComm = 0, Vec3 refPoint = Vec3(0, 0, 0)) const;
	bool		NetPlayAnimAction(const char* action, bool looping) const;
	bool		NetSpawnParticleEffect(const char* effectName) const;
	void		GetMemoryStatistics(ICrySizer*) const;

private:

	CGameFlashAnimation m_animScoutFlyInterface;

	std::map<ActionId, int>				  m_actionsMap;
	static TActionHandler<CControlClient> s_actionHandler;
	
	bool		m_isDebugLog;
	uint8		m_lastSpectatorMode;
	EntityId	m_mpLastControlledId;
	Vec3		m_finalFireTargetPos;
	ENanoMode	m_lastDudeNanoMode;
	Quat		m_lastDudeRotation;
	Vec3		m_lastDudePosition;
	int			m_lastDudeSpecies;
	float		m_lastDudeSuitEnergy;
	float		m_storedTime;
	float		m_meleeHits;
	float		m_currentFov;
	Vec3		m_dudeLastPos;
	Vec3		m_camViewDir;
	Vec3		m_camPos;
	Vec3		m_camViewCoords;
	Vec3		m_crosshairPos;
	SAlienControlParams		m_nakedAlien;
	STrooperControlParams	m_trooper;
	SScoutControlParams		m_scout;
	SHunterControlParams	m_hunter;
	SGenericControlParams	m_generic;
	bool		m_mustBeamDude;
	bool		m_mustHideDude;
	bool		m_canProceedActions;
	bool		m_isHitListener;
	CPlayer*	m_pLocalDude;
	CActor*		m_pControlledActor;
	EntityId	m_fireTargetId;
	EntityId	m_meleeTargetId;
	EntityId	m_crosshairTargetId;
	EntityId	m_lastCrosshairTargetId;
	EntityId	m_scoutAimTargetId;
	ray_hit		m_meleeRayhit;
	ray_hit		m_crosshairRayHit;
	CMovementRequest m_lookRequest;// Needed only for look and aim processing
	CAbilitiesSystem* m_pAbilitiesSystem;

public:
	static constexpr int INPUT_ASPECT = eEA_GameClientDynamic;
	static constexpr int ALIVE_ASPECT = eEA_GameServerDynamic;
	static constexpr int OWNER_ASPECT = eEA_GameServerStatic;
};

class CNetControlClient
{
public:

	friend class CPlayer;
	friend class CControlSystem;

	CNetControlClient(CPlayer* pActor);
	~CNetControlClient();

	

	//Desired Actor
	//Summary
	//The desired Actor is needed in order to remember it until it is possible to exercise control over it.
	//An example of one such situation would be when you want to take control of some character, 
	//but the local player has not been declared until that moment.
	//And when a local player is declared at some point, 
	//The Control System immediately executes the command to take control of the Desired Actor
	void	UpdateLocal(float frametime);
	void	StartLocalDesired(IActor* pActor, bool dudeHide = 1, bool dudeBeam = 1);
	void	SetDesiredActor(IActor* pActor, bool reset);
	void	ResetLocalDesired();
	//~Desired Actor

private:
	CPlayer* m_pLocalActor;
	IActor* m_pDesiredActor;
	bool m_IsDesiredDudeHide;
	bool m_IsDesiredDudeBeam;
};

class CControlSystem : public 
	ILevelSystemListener, 
	IGameplayListener,
	IInputEventListener,
	IItemSystemListener
	//IHardwareMouseEventListener
{
public:
	typedef int ChannelId;

	CControlSystem();
	~CControlSystem();

	friend class CControlClient;
	friend class CAbilitiesSystem;
	friend class CSquadSystem;
	friend class CConquerorSystem;
	friend class CHUD;

	//Events
	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener
	// 
	//IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event);
	virtual bool OnInputEventUI(const SInputEvent& event) { return false; }
	//~IInputEventListener

	//IGameplayListener
	virtual void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event);
	//~IGameplayListener
	
	//ILevelSystemListener
	virtual void OnLevelNotFound(const char* levelName);
	virtual void OnLoadingStart(ILevelInfo* pLevel);
	virtual void OnLoadingComplete(ILevel* pLevel);
	virtual void OnLoadingError(ILevelInfo* pLevel, const char* error);
	virtual void OnLoadingProgress(ILevelInfo* pLevel, int progressAmount);
	//~ILevelSystemListener

	// IItemSystemListener
	virtual void OnSetActorItem(IActor* pActor, IItem* pItem);
	virtual void OnDropActorItem(IActor* pActor, IItem* pItem);
	virtual void OnSetActorAccessory(IActor* pActor, IItem* pItem) {};
	virtual void OnDropActorAccessory(IActor* pActor, IItem* pItem) {};
	// ~IItemSystemListener

	void		OnMainMenuEnter();
	void		OnActorDeath(IActor* pActor);
	void		OnActorGrabbed(IActor* pActor, EntityId grabberId);
	void		OnActorDropped(IActor* pActor, EntityId droppedId);
	void		OnActorGrab(IActor* pActor, EntityId grabId);
	void		OnActorDrop(IActor* pActor, EntityId dropId);
	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle, const char* strSeatName, bool bThirdPerson);
	void		OnExitVehicle(IActor* pActor);
	void		OnGameRulesReset();
	void		OnVehicleStuck(IVehicle* pVehicle, bool stuck);
	//~Events

	void Init(const char* version);
	void Update(float frametime);
	void Shutdown();

	Vec3 GetMouseScreenPos();
	Vec3 GetMouseWorldPos();
	EntityId GetMouseEntityID();

	//Local machine functions
	bool GetLocalEnabled();
	void StartLocal(IActor* pActor, bool dudeHide = 1, bool dudeBeam = 1);
	void StopLocal(bool toEditor);
	void ResetLocal();

	CControlClient* GetLocalControlClient() { return m_pLocalControlClient; };
	CNetControlClient* GetNetLocalControlClient() { return m_pLocalNetControlClient; };

	void RegisterLocalControlClient(CControlClient* pControlClient);
	void RegisterNetLocalControlClient(CNetControlClient* pControlClient);
	//~Local machine functions

	//Global functions
	CAbilitiesSystem* GetAbilitiesSystem() { return m_pAbilitiesSystem; };
	CSquadSystem*	  GetSquadSystem() { return m_pSquadSystem; };
	CConquerorSystem* GetConquerorSystem() { return m_pConquerorSystem; };
	CAIActionTracker* GetAIActionTracker() { return m_pAIActionTracker; };

	void AddChild(IControlSystemChild* pChild, bool FG);
	void RemoveChild(IControlSystemChild* pChild, bool FG);
	//~Global functions

	CActor* GetClientActor();

	void AddVehicleMovementInfo(EntityId vehicleId, CVehicleMovementBase* info);
	CVehicleMovementBase* GetVehicleMovementInfo(EntityId vehicleId);
	bool IsVehicleStuck(const IVehicle* pVehicle) const;

	void GetMemoryStatistics(ICrySizer* s);
private:
	CAIActionTracker* m_pAIActionTracker;
	CConquerorSystem* m_pConquerorSystem;
	CAbilitiesSystem* m_pAbilitiesSystem;
	CSquadSystem* m_pSquadSystem;
	CControlClient* m_pLocalControlClient;
	CNetControlClient* m_pLocalNetControlClient;
	
	std::map<EntityId, CVehicleMovementBase*> m_vehiclesMovementInfo;
	std::map<EntityId, float> m_vehiclesStuckTime;
	std::map<EntityId, int> m_vehiclesStuckFlag;
	std::vector<IControlSystemChild*> m_childs;
	std::vector<IControlSystemChild*> m_fgChilds;

	float m_screenMouseX;
	float m_screenMouseY;
	Vec3 m_mouseWorldPos;

	bool m_isDebugLog;
};
//We have need the global variable, therefore we use extern word.
extern CControlSystem* g_pControlSystem;