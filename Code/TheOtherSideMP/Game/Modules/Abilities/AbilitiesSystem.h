#pragma once

#include "../Control/IControlSystemChild.h"
#include "AbilityOwner.h"

const const char* ABILITY_DEFAULT = "Default";

const const char* ABILITY_TROOPER_RAGE = "TrRage";
const const char* ABILITY_TROOPER_SHIELD = "TrShield";
const const char* ABILITY_TROOPER_EMP = "TrEMP";
const const char* ABILITY_TROOPER_CLOAK = "TrCloak";

const const char* ABILITY_HUNTER_GRAB = "HrGrab";
const const char* ABILITY_HUNTER_SHIELD = "HrShield";

const const char* ABILITY_SCOUT_SPOTLIGHT = "ScSpotlight";
const const char* ABILITY_SCOUT_ANTIGRAV = "ScAntigrav";
const const char* ABILITY_SCOUT_OBJECTGRAB = "ScGrab";

const const char* ABILITY_HUMAN_VEHICLE_HEALING = "HmVehicleHealing";

const const char* VISIBLE_MODE_HIDE = "Hide";
const const char* VISIBLE_MODE_SHOW = "Show";
const const char* VISIBLE_MODE_DISABLED = "Disabled";
const const char* VISIBLE_MODE_ACTIVATED = "Activated";

const int LEFT_ABILITY = 1;
const int CENTER_ABILITY = 2;
const int RIGHT_ABILITY = 3;

class CAbility;
class CAbilityOwner;

typedef std::vector<CAbilityOwner> TAbilityOwners;

enum EActivatedAbility
{
	eActivatedAbility_LeftAbility = (1 << 0),
	eActivatedAbility_CenterAbility = (1 << 1),
	eActivatedAbility_RightAbility = (1 << 2),
};

enum ECeilingType
{
	eCeilingType_Ceiling = (1 << 0),
	eCeilingType_WallRight = (1 << 1),
	eCeilingType_WallLeft = (1 << 2),
};

class CAbilitiesSystem : public IControlSystemChild
{
public:
	CAbilitiesSystem();
	~CAbilitiesSystem();

	friend class CAbility;
	friend class CAbilityOwner;

	//IControlSystemChild
	void		OnMainMenuEnter() ;
	void		OnGameRulesReset() ;
	void		OnActorDeath(IActor* pActor) ;
	void		OnActorGrabbed(IActor* pActor, EntityId grabberId) ;
	void		OnActorDropped(IActor* pActor, EntityId droppedId) ;
	void		OnActorGrab(IActor* pActor, EntityId grabId) ;
	void		OnActorDrop(IActor* pActor, EntityId dropId) ;
	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) ;
	void		OnExitVehicle(IActor* pActor) ;
	bool		OnInputEvent(const SInputEvent& event) ;
	void		Init() ;
	void		Update(float frameTime) ;
	void		Serialize(TSerialize ser) ;
	void		GetMemoryStatistics(ICrySizer* s) ;
	const char* GetChildName()  { return "CAbilitiesSystem"; }
	//~IControlSystemChild


	struct STrooperAbilities
	{
		STrooperAbilities()
		{
			Reset();
		}

		void Reset();
		void Serialize(TSerialize ser);

		ECeilingType eCeilingType;

		bool canCeiling;
		bool doCeiling;
		bool isCeiling;
		bool isRageMode;
		//bool isProjectingShield;

		float rageDuration;
		float shockwaveTimer;
	};
	struct SScoutAbilities
	{
		SScoutAbilities()
		{
			Reset();
		}

		void Reset();
		void Serialize(TSerialize ser);

		int	abilityPressNum;
		bool IsSearch;
		bool isGyroEnabled;// gyroscope is enabled by defauld
	};
	struct SHunterAbilities
	{
		SHunterAbilities()
		{
			Reset();
		}

		void Reset();
		void Serialize(TSerialize ser);

		bool IsShieldEnabled;
	};

	void UpdateHUD();
	void UpdateEffectsHUD();
	void UpdateVisibleModeHUD();

	CAbilityOwner* AddAbilityOwner(const IActor* pActor);
	bool RemoveAbilityOwner(const CAbilityOwner& owner);

	CAbilityOwner* GetAbilityOwner(const IActor* pActor);
	CAbilityOwner* GetAbilityOwner(EntityId id);

	bool IsAbilityOwner(const IActor* pActor);
	bool IsAbilityOwner(EntityId id);

	void Reset();
	void OnAddingAbility(const char* abilityName, EntityId ownerId);

protected:

	bool OnInputLeftAbility(EntityId entityId, EInputState activationMode, float value);
	bool OnInputCenterAbility(EntityId entityId, EInputState activationMode, float value);
	bool OnInputRightAbility(EntityId entityId, EInputState activationMode, float value);

public:
	STrooperAbilities trooper;
	SScoutAbilities scout;
	SHunterAbilities hunter;
private:

	TAbilityOwners m_abilityOwners;
	bool m_isDebugLog;
};