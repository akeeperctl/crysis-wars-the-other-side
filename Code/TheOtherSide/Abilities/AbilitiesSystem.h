#pragma once

#include "../Control/IControlSystemChild.h"
#include "AbilityOwner.h"

constexpr const char* ABILITY_DEFAULT = "Default";

constexpr const char* ABILITY_TROOPER_RAGE = "TrRage";
constexpr const char* ABILITY_TROOPER_SHIELD = "TrShield";
constexpr const char* ABILITY_TROOPER_EMP = "TrEMP";
constexpr const char* ABILITY_TROOPER_CLOAK = "TrCloak";

constexpr const char* ABILITY_HUNTER_GRAB = "HrGrab";
constexpr const char* ABILITY_HUNTER_SHIELD = "HrShield";

constexpr const char* ABILITY_SCOUT_SPOTLIGHT = "ScSpotlight";
constexpr const char* ABILITY_SCOUT_ANTIGRAV = "ScAntigrav";
constexpr const char* ABILITY_SCOUT_OBJECTGRAB = "ScGrab";

constexpr const char* ABILITY_HUMAN_VEHICLE_HEALING = "HmVehicleHealing";

constexpr const char* VISIBLE_MODE_HIDE = "Hide";
constexpr const char* VISIBLE_MODE_SHOW = "Show";
constexpr const char* VISIBLE_MODE_DISABLED = "Disabled";
constexpr const char* VISIBLE_MODE_ACTIVATED = "Activated";

constexpr int LEFT_ABILITY = 1;
constexpr int CENTER_ABILITY = 2;
constexpr int RIGHT_ABILITY = 3;

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
	void		OnMainMenuEnter() override;
	void		OnGameRulesReset() override;
	void		OnActorDeath(IActor* pActor) override;
	void		OnActorGrabbed(IActor* pActor, EntityId grabberId) override;
	void		OnActorDropped(IActor* pActor, EntityId droppedId) override;
	void		OnActorGrab(IActor* pActor, EntityId grabId) override;
	void		OnActorDrop(IActor* pActor, EntityId dropId) override;
	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) override;
	void		OnExitVehicle(IActor* pActor) override;
	bool		OnInputEvent(const SInputEvent& event) override;
	void		Init() override;
	void		Update(float frameTime) override;
	void		Serialize(TSerialize ser) override;
	void		GetMemoryStatistics(ICrySizer* s) override;
	const char* GetChildName() override { return "CAbilitiesSystem"; }
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