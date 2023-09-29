#pragma once

#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IVehicleSystem.h>

#include "TheOtherSideMP/Actors/player/TOSPlayer.h"
#include "TheOtherSideMP/Game/Modules/ITOSGameModule.h"


class CTOSAIActionTracker;
class CTOSMasterClient;
class CSquadSystem;
class CGameFlashAnimation;
class CConquerorSystem;
class CTOSAbilitiesSystem;
class CVehicleMovementBase;

struct IHardwareMouseEventListener;
struct IHitListener;


 /**
 * TOS MasterClient
 * Класс описывает то, как будет происходить взаимодействие между Мастером и Рабом.
 * В частности перемещение, управление, угол обзора и т.д
 * Автоудаление: отсутствует.
 */
class CTOSMasterClient
{	//Methods
public:
	friend class CGameRules;
	//friend class CControlSystem;
	//friend class CNetControlClient;
	//friend class CSquadSystem;
	//friend class CConquerorSystem;
	friend class CPlayerInput;

	friend class CTOSAbilitiesSystem;
	friend class CAbilityOwner;

	//friend class CSquad;
	//friend class CMember;


	explicit CTOSMasterClient(CTOSPlayer* _player);
	~CTOSMasterClient();

	//IHitListener
	//void OnHit(const HitInfo&) override;
	//void OnExplosion(const ExplosionInfo&) override;
	//void OnServerExplosion(const ExplosionInfo&) override;
	//~IHitListener

	//void		ApplyMovement(const Vec3& delta);
	//void		FullSerialize(TSerialize ser);

	//void		Update();
	//void		UpdateView(SViewParams& viewParams);
	//void		UpdateUsability(const IEntity* pTarget) const;
	//void		UpdateCrosshair();
	//void		UpdateScout();
	//void		UpdateTrooper();
	//void		UpdateHunter() const;

	//static void	SetDudeSpecies(const int species);
	//void		SetSlaveActor(IActor* act);

	//void		ToggleDudeHide(const bool toggle);
	//void		ToggleDudeBeam(const bool toggle);
	////Tutorial mode - its mode when the controlled actor will be revive after he die and squad system will not be change controlled actor.
	//void		ToggleTutorialMode(const bool mode);

	//IActor* GetSlaveActor() const { return m_pSlaveActor; };
	//IEntity* GetSlaveEntity() const
	//{
	//	if (m_pSlaveActor) { return m_pSlaveActor->GetEntity(); } return nullptr;
	//};

	////Get the class name from the current controlled actor
	//string		GetActorClassName() const { if (m_pSlaveActor) return m_pSlaveActor->GetEntity()->GetClass()->GetName(); else return ""; };

	////Get the current item class name from the current controlled actor
	//string		GetItemClassName() const { if (m_pSlaveActor) { if (m_pSlaveActor->GetInventory()) { const EntityId pItemId = m_pSlaveActor->GetInventory()->GetCurrentItem(); if (pItemId) { const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pItemId); if (pItem) { return pItem->GetEntity()->GetClass()->GetName(); } else return ""; } else return ""; } else return ""; } else return ""; };

	//Tutorial mode - its mode when controlled actor will be revive when he die and squad system will not be change controlled actor when he die.
	//bool		GetTutorialMode() const;

	////Get the pointer to the material of the current controlled actor
	//IMaterial* GetMaterial() const;

	////Get the current crosshair's position vector in the world space
	//Vec3		GetCrosshairPos() const { return m_crosshairPos; };
	//Vec3		GetMeleeHitPos() const { return m_meleeRayhit.pt; };
	//float		GetMeleeHits() const { return m_meleeHits; };

	//static CWeapon* GetCurrentWeapon(const CActor* pActor);

	//std::map<ActionId, int> GetActions() { return m_actionsMap; };

	////Get structures with variables of the system and controlled actor
	//SGenericControlParams& GetGenericParams() { return m_generic; };
	//SScoutControlParams& GetScoutParams() { return m_scout; };
	//SAlienControlParams& GetNakedParams() { return m_nakedAlien; };
	//SHunterControlParams& GetHunterParams() { return m_hunter; };
	//STrooperControlParams& GetTrooperParams() { return m_trooper; };

	////These functions allows us to find out the type of controlled actor
	//bool		IsNakedAlien() const { return GetActorClassName() == "Alien"; }
	//bool		IsTrooper() const { return GetActorClassName() == "Trooper"; }
	//bool		IsHunter() const { return GetActorClassName() == "Hunter"; }
	//bool		IsScout() const { return GetActorClassName() == "Scout"; }

private:

	//Events
	//void		OnChangedSpectatorMode(const IActor* pActor, uint8 mode, EntityId targetId, bool resetAll);
	//void		OnClientHit(const HitInfo&) const;
	//void		OnActorDeath(IActor* pActor);
	//void		OnAction(const ActionId& action, int activationMode, float value);//Called  from PlayerInput.cpp
	////~Events

	//void		SubEnergy(const float subtractValue) const;

	//bool		OnActionViewLock(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	//bool		OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	//bool		OnActionMoveBack(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	//bool		OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	//bool		OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	//void		OnActionCrouch(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionJump(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionSprint(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionBinoculars(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionAttack(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionAim(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionLeanLeft(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionLeanRight(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionNextItem(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionPrevItem(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionSpeedUp(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionSpeedDown(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionZoomIn(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionZoomOut(string classname, const ActionId& action, int activationMode, float value);
	//void		OnActionUse(string classname, const ActionId& action, int activationMode, float value);

	////Storing the current time in the variable
	////See also CheckPassTime();
	//void		StoreCurrTime();

	////Checks how much time has passed from last time storing
	////See also StoreCurrTime();
	//bool		CheckPassTime(float passedSec) const;

	////Resetting the structures with stored variables of the system and controlled actor
	//void		ResetParams();

	////Resetting the current controlled actor to the normal state i.e before having owner
	//void		Reset(bool toEditor);

	//Preparing a dude actor (Nomad) for to be in a controlling state
	//void InitDudeMaster(bool toStart);

	////Un/Registering the AI of controlled actor to according with the AI type of the dude player
	//void		SetActorAI(const IActor* pActor, bool bToPlayerAI) const;

	////Preparing a new actor to enter in the controlled state
	//bool		DoNextActor(CActor* pNextActor);

	////Preparing a old actor to enter in the uncontrolled state
	//bool		DoPrevActor(CActor* pActor) const;

	////Get the entity which being in the ray with length == 3 from the weapon's position to crosshair point position
	//IEntity* GetMeleeTarget() const;

	////Get the current entity under the crosshair
	//IEntity* GetLastCrosshairEntity() const;

	////Get the current entity under the crosshair
	//IEntity* GetCrosshairEntity() const;

	////Get the current entity under the crosshair
	//EntityId	GetCrosshairEntityId() const;

	//EntityId	GetScoutAutoAimTargetId() const;

	////Get the entity which being in the ray with length == 5000 from weapon's position to crosshair point position
	//IEntity* GetFireTarget() const;

	//static void		SetInventoryHUD(IActor* pActor, const char* file);
	//static void		SetAmmoHealthHUD(IActor* pActor, const char* file);
	//IAIObject* GetCrosshairTargetAI() const;
	//void		SendPipeToAIGroup(const char* name, int groupId, bool useInComm = 0, Vec3 refPoint = Vec3(0, 0, 0)) const;
	//bool		NetPlayAnimAction(const char* action, bool looping) const;
	//bool		NetSpawnParticleEffect(const char* effectName) const;
	//void		GetMemoryStatistics(ICrySizer*) const;

private:

	//CGameFlashAnimation m_animScoutFlyInterface;

	//std::map<ActionId, int>				  m_actionsMap;
	//static TActionHandler<CTOSMasterClient> s_actionHandler;

	//bool		m_isDebugLog;
	//uint8		m_lastSpectatorMode;
	//EntityId	m_mpLastControlledId;
	//Vec3		m_finalFireTargetPos;
	//ENanoMode	m_lastDudeNanoMode;
	//Quat		m_lastDudeRotation;
	//Vec3		m_lastDudePosition;
	//int			m_lastDudeSpecies;
	//float		m_lastDudeSuitEnergy;
	//float		m_storedTime;
	//float		m_meleeHits;
	//float		m_currentFov;
	//Vec3		m_dudeLastPos;
	//Vec3		m_camViewDir;
	//Vec3		m_camPos;
	//Vec3		m_camViewCoords;
	//Vec3		m_crosshairPos;
	////SAlienControlParams		m_nakedAlien;
	////STrooperControlParams	m_trooper;
	////SScoutControlParams		m_scout;
	////SHunterControlParams	m_hunter;
	////SGenericControlParams	m_generic;
	//bool		m_mustBeamDude;
	//bool		m_mustHideDude;
	//bool		m_canProceedActions;
	//bool		m_isHitListener;

	//IActor*		m_pSlaveActor;
	//EntityId	m_fireTargetId;
	//EntityId	m_meleeTargetId;
	//EntityId	m_crosshairTargetId;
	//EntityId	m_lastCrosshairTargetId;
	//EntityId	m_scoutAimTargetId;
	//ray_hit		m_meleeRayhit;
	//ray_hit		m_crosshairRayHit;
	//CMovementRequest m_lookRequest;// Needed only for look and aim processing
	//CTOSAbilitiesSystem* m_pAbilitiesSystem;

	CTOSPlayer* m_pLocalDude;

public:
	static constexpr int INPUT_ASPECT = eEA_GameClientDynamic;
	static constexpr int ALIVE_ASPECT = eEA_GameServerDynamic;
	static constexpr int OWNER_ASPECT = eEA_GameServerStatic;
};