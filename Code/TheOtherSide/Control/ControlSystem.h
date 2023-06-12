#ifndef ControlSystem_H
#define ControlSystem_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <IGameFramework.h>
#include <IGameRulesSystem.h>

#include "Item.h"
#include <Actor.h>

struct SControlClient;
struct SSquadSystem;

class CAbilitiesSystem;


struct SControlSystem
{
	SControlSystem();
	~SControlSystem();

	//	Returns true when client is controlling some actor
	ILINE bool	GetEnabled() { return isEnabled; };
	void		Start();//Called from FG
	void		Stop();//Called from FG
	void		ResetSystem();

	// Summary
	//   Returns the pointer to the Abilites System
	// Returns
	//   A pointer to the Abilities System which define the usable abilities
	CAbilitiesSystem* GetAbilitiesSystem() { return pAbilitiesSystem; };

	//Get Squad System
	SSquadSystem* GetSquadSystem() { return pSquadSystem; };

	// Summary
	//   Returns the actor control structure implementation
	// Returns
	//   A pointer to the current control client implementation, or null
	//   if none as been registered
	// See Also
	//   RegisterControlClient
	SControlClient* GetControlClient() { return pControlClient; };

	// Summary
	//   Performs the registration of control client implementation
	// Parameters
	//   pControlClient - a pointer to the control client implementation
	// Notes
	//   Only one control client implementation can be registered
	// See Also
	//   GetControlClient
	void RegisterControlClient(SControlClient* pControlClient);

	//
	void GetMemoryStatistics(ICrySizer* s);

	CAbilitiesSystem* pAbilitiesSystem;
	SSquadSystem* pSquadSystem;
	SControlClient* pControlClient;
	bool			isEnabled;
};

#include "Nanosuit.h"
//#include "HUD/GameFlashAnimation.h"
//#include "Control/AbilitiesSystem.h"
//goalpipe ids of orders
#define GOALPIPEID_ORDER_SEARCH 770
#define GOALPIPEID_ORDER_COOLDOWN 771
#define GOALPIPEID_ORDER_GOTO 772
#define GOALPIPEID_ORDER_GOTO_GUARD 773
#define GOALPIPEID_ORDER_FOLLOW 774
#define GOALPIPEID_ORDER_FOLLOW_QUICKLY 775

//Trooper timers of he actions
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

struct SSquadSystem;
struct IHitListener;

class CGameFlashAnimation;

typedef struct SGenericControlParams
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

typedef struct SHunterControlParams
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

typedef struct SScoutControlParams
{
	SScoutControlParams()
	{
		speedMult = 0.6f;

		modelQuat.SetIdentity();

		doDodge = false;
		canDodge = true;
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		speedMult = 0.6f;

		modelQuat.SetIdentity();

		doDodge = false;
		canDodge = true;
	}

	float		speedMult;

	bool		canDodge;
	bool		doDodge;

	Quat		modelQuat;
};

typedef struct STrooperControlParams
{
	STrooperControlParams()
	{
		canDodge = true;
		isBinocular = false;
		isAiming = false;

		canJumpMelee = true;
		doJumpMelee = false;

		canMelee = true;
		doMelee = false;

		speedMult = 1.0f;
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		canDodge = true;
		isBinocular = false;
		isAiming = false;

		canJumpMelee = true;
		doJumpMelee = false;

		canMelee = true;
		doMelee = false;

		speedMult = 1.0f;
	}

	float		speedMult;
	bool		isAiming;
	bool		isBinocular;
	bool		canDodge;
	bool		canJumpMelee;
	bool		doJumpMelee;
	bool		canMelee;
	bool		doMelee;
};

typedef struct SAlienControlParams
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

struct SControlClient : public IHitListener
{
	friend struct SSquadSystem;
	friend struct SSquad;
	friend struct SMember;
	friend struct SControlSystem;

	SControlClient();
	~SControlClient();

	//Methods
public:

	//IHitListener
	virtual void OnHit(const HitInfo&);
	virtual void OnExplosion(const ExplosionInfo&);
	virtual void OnServerExplosion(const ExplosionInfo&);
	//~IHitListener

	void		OnAction(const ActionId& action, int activationMode, float value);//Called  from PlayerInput.cpp
	void		ApplyMovement(const Vec3& delta);
	void		FullSerialize(TSerialize ser);

	void		TestFunction();

	//Updating the main mechanics of the system
	//Called from Game.cpp
	void		Update(float frametime);

	//Updating the camera view relatively to the controlled actor
	//Called from PlayerView.cpp
	void		UpdateView(SViewParams& viewParams);
	void		UpdateUsability(IEntity* pTarget);
	void		UpdateCrosshair();
	void		UpdateScout();
	void		UpdateTrooper();
	void		UpdateHunter();
	//void		UpdateAlien();
	void		UpdateDeath(IActor* pNomadPlayer);

	void		SetDudeSpecies(const int species);

	//Changing the current controlling actor
	void		SetActor(CActor* act);

	//Beaming the dude (Nomad) player to the controlled actor's position
	void		ToggleDudeHide(const bool toggle);

	//Beaming the dude (Nomad) player to the controlled actor's position
	void		ToggleDudeBeam(const bool toggle);

	//Tutorial mode - its mode when the controlled actor will be revive after he die and squad system will not be change controlled actor.
	void		ToggleTutorialMode(const bool mode);

	CActor* GetControlledActor() { return m_pControlledActor; };
	IEntity* GetControlledEntity() { if (m_pControlledActor) { return m_pControlledActor->GetEntity(); } return nullptr; };
	CWeapon* GetCurrentWeapon(CActor* pActor);

	//Get the entity which being in the ray with length == 3 from the weapon's position to crosshair point position
	IEntity* GetMeleeTarget();

	//Get the current entity under the crosshair
	IEntity* GetLastCrosshairEntity();

	//Get the current entity under the crosshair
	IEntity* GetCrosshairEntity();

	//Get the current crosshair's position vector in the world space
	Vec3		GetCrosshairPos() { return m_crosshairPos; };

	//Get the entity which being in the ray with length == 5000 from weapon's position to crosshair point position
	IEntity* GetFireTarget();

	//Get the class name from the current controlled actor
	string GetActorClassName() const { if (m_pControlledActor) return m_pControlledActor->GetEntity()->GetClass()->GetName(); else return ""; };

	//Get the current item class name from the current controlled actor
	string GetItemClassName() const { if (m_pControlledActor) { if (m_pControlledActor->GetInventory()) { EntityId pItemId = m_pControlledActor->GetInventory()->GetCurrentItem(); if (pItemId) { IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pItemId); if (pItem) { return pItem->GetEntity()->GetClass()->GetName(); } else return ""; } else return ""; } else return ""; } else return ""; };

	//Tutorial mode - its mode when controlled actor will be revive when he die and squad system will not be change controlled actor when he die.
	bool GetTutorialMode() const;

	//Get the pointer to the material of the current controlled actor
	IMaterial* GetMaterial();

	std::map<ActionId, int> GetActions() { return m_actionsMap; };

	//Get structures with variables of the system and controlled actor
	SGenericControlParams& GetGenericParams() { return m_generic; };
	SScoutControlParams& GetScoutParams() { return m_scout; };
	SAlienControlParams& GetNakedParams() { return m_nakedAlien; };
	SHunterControlParams& GetHunterParams() { return m_hunter; };
	STrooperControlParams& GetTrooperParams() { return m_trooper; };

	//Showing the hit fx on the hud when the controlled actor is under taking damage
	//Calling from CGameRules::ClientHit(const HitInfo &hitInfo)
	void		ShowClientHit(const HitInfo&);

	//Showing the hit indicator fx on the hud when the controlled actor is under taking damage
	//Called from CGameRules::ServerHit(const HitInfo &hitInfo)
	void		ShowHitIndicator(const HitInfo&);

	//Un/Registering the AI of controlled actor to according with the AI type of the dude player
	void		SetActorAI(IActor* pActor, bool bPlayer);

	//Preparing a new actor to enter in the controlled state
	bool		DoNextActor(CActor* pActor);

	//Preparing a old actor to enter in the uncontrolled state
	bool		DoPrevActor(CActor* pActor);

	//These functions allows us to find out the type of controlled actor
	bool	IsNakedAlien() const { return GetActorClassName() == "Alien"; }
	bool	IsTrooper() const { return GetActorClassName() == "PlayerTrooper" || GetActorClassName() == "Trooper"; }
	bool	IsHunter() const { return GetActorClassName() == "Hunter"; }
	bool	IsScout() const { return GetActorClassName() == "Scout"; }

	//Sets a PostFX on the client screen
	//void		SetPostFX(const char* effectname, const float value);

private:
	//Subtracting the alien player's energy value from the different situations
	void		SubEnergy(const float subtractValue);

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
	bool		CheckPassTime(float passedSec);

	//Resetting the structures with stored variables of the system and controlled actor
	void		ResetParams();

	//Resetting the current controlled actor to the normal state i.e before having owner
	void		Reset();

	//Preparing a dude actor (Nomad) for to be in a controlling state
	void		InitDudeToControl(bool toLink);

	void		SetAmmoHealthHUD(IActor* pActor, const char* file);

	IAIObject* GetCrosshairTargetAI();
	void		SendPipeToAIGroup(const char* name, int groupId, bool useInComm = 0, Vec3 refPoint = Vec3(0, 0, 0));

	//Controlled actor start to playing the animation action which stored in animation graph
	bool		PlayAnimAction(const char* action, bool looping);

	//Spawning the particle effect in the controlled actor's position
	bool		SpawnParticleEffect(const char* effectName);

	//Mem
	void		GetMemoryStatistics(ICrySizer*);

	//Members
private:

	std::map<ActionId, int>				  m_actionsMap;
	static TActionHandler<SControlClient> s_actionHandler;

	//SSquadSystem* m_pSquadSystem;
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
public:
	SAlienControlParams		m_nakedAlien;
	STrooperControlParams	m_trooper;
	SScoutControlParams		m_scout;
	SHunterControlParams	m_hunter;
	SGenericControlParams	m_generic;
private:
	//Generic variables
	bool		m_mustBeamDude;
	bool		m_mustHideDude;
	bool		m_canProceedActions;
	bool		m_isHitListener;

	CActor* m_pControlledActor;

	EntityId	m_fireTargetId;
	EntityId	m_meleeTargetId;
	EntityId	m_crosshairTargetId;
	EntityId	m_lastCrosshairTargetId;

	ray_hit		m_meleeRayhit;
	ray_hit		m_crosshairRayHit;
	CMovementRequest m_lookRequest;// Needed only for look and aim processing

	CAbilitiesSystem* m_pAbilitiesSystem;

public:
	static const int INPUT_ASPECT = eEA_GameClientDynamic;
	static const int ALIVE_ASPECT = eEA_GameServerDynamic;
	static const int OWNER_ASPECT = eEA_GameServerStatic;
};

//We have need the global variable, therefore we use extern word.
extern SControlSystem* g_pControlSystem;
#endif //ControlSystem_H