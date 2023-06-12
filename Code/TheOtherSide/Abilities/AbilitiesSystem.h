#pragma once
#include "ISerialize.h"
#include "HUD/GameFlashAnimation.h"

constexpr const char* ABILITY_DEFAULT = "Default";

constexpr const char* ABILITY_TROOPER_RAGE = "TrRage";
constexpr const char* ABILITY_TROOPER_SHIELD = "TrShield";
constexpr const char* ABILITY_TROOPER_EMP = "TrEMP";

constexpr const char* ABILITY_HUNTER_GRAB = "HrGrab";
constexpr const char* ABILITY_HUNTER_SHIELD = "HrShield";

constexpr const char* ABILITY_SCOUT_SPOTLIGHT = "ScSpotlight";
constexpr const char* ABILITY_SCOUT_ANTIGRAV = "ScAntigrav";

constexpr const char* VISIBLE_MODE_HIDE = "Hide";
constexpr const char* VISIBLE_MODE_SHOW = "Show";
constexpr const char* VISIBLE_MODE_DISABLED = "Disabled";
constexpr const char* VISIBLE_MODE_ACTIVATED = "Activated";

constexpr int LEFT_ABILITY = 1;
constexpr int CENTER_ABILITY = 2;
constexpr int RIGHT_ABILITY = 3;

class CAbility;
class CAbilityOwner;
class CAbilitiesSystem;

typedef std::vector<CAbility> TAbilities;
typedef std::vector<CAbilityOwner> TAbilityOwners;

enum EActivatedAbility
{
	eActivatedAbility_LeftAbility = (1 << 0),
	eActivatedAbility_CenterAbility = (1 << 1),
	eActivatedAbility_RightAbility = (1 << 2),
};

enum EAbilityState
{
	eAbilityState_Ready_To_Activate = (1 << 0),
	eAbilityState_Activated = (1 << 1),
	eAbilityState_Cooldown = (1 << 2),
	eAbilityState_Disabled_By_Condition = (1 << 3),
	eAbilityState_ForSync = (1 << 4),
};

enum EAbilityVisibleMode
{
	eAbilityMode_Hide = (1 << 0),
	eAbilityMode_Show = (1 << 1),
	eAbilityMode_Disabled = (1 << 2),
	eAbilityMode_Activated = (1 << 3),
	eAbilityMode_ForSync = (1 << 4),
};

enum ECeilingType
{
	eCeilingType_Ceiling = (1 << 0),
	eCeilingType_WallRight = (1 << 1),
	eCeilingType_WallLeft = (1 << 2),
};

class CAbility
{
public:
	friend class CAbilityOwner;
	friend class CAbilitiesSystem;

	CAbility()
	{
		Reset();
	}
	~CAbility()
	{
		Reset();
	}
	//~CAbility();

	void Reset()
	{
		SetVisibleMode(eAbilityMode_Hide);
		state = eAbilityState_Ready_To_Activate;
		//m_ownerId = 0;
		name = "";
		index = 0;
		abilityOwnerId = 0;
	};

	void Serialize(TSerialize ser);
	void GetMemoryStatistics(ICrySizer* s);

protected:
	void SetVisibleMode(EAbilityVisibleMode mode);
	const EAbilityVisibleMode GetVisibleMode();

	void SetState(EAbilityState state);
	const EAbilityState GetState();

	void SetName(const char* name);
	string GetName();

public:
	EAbilityVisibleMode visMode;
	EAbilityState state;
	//EntityId m_ownerId;
	string name;

	//The index of the ability is starting with 1
	int index;
	EntityId abilityOwnerId;
};

class CAbilityOwner
{
public:
	friend class CAbilitiesSystem;

	CAbilityOwner()
	{
		Reset();
	};
	~CAbilityOwner()
	{
		Reset();
	};

	//Summary
	//1 - Left Ability
	//2 - Center Ability
	//3 - Right Ability
	//4 - etc.
	//void ToggleAbility(const char* name);
	void ToggleAbility(int index);
	//void ToggleAbility(CAbility& ability);

	int AddAbility(const char* name);
	//int AddAbility(const char* name, int index);
	int AddAbility(CAbility& ability);
	//int AddAbility(CAbility& ability, int index);

	bool RemoveAbility(const char* name);
	bool RemoveAbility(int index);
	bool RemoveAbility(CAbility& ability);

	bool IsHaveAbility(const char* name);
	bool IsHaveAbility(int index);
	bool IsHaveAbility(const CAbility& ability);

	CAbility& GetAbility(const char* name);
	CAbility& GetAbility(int index);

	int GetAbilityCount();

	void SetEntityId(EntityId id);
	EntityId GetEntityId() const;

	void Reset()
	{
		TAbilities::iterator it = m_abilities.begin();
		TAbilities::iterator end = m_abilities.end();

		for (; it != end; ++it)
		{
			it->Reset();
		}

		m_abilities.clear();
		m_ownerId = 0;
	};

	void Serialize(TSerialize ser);
	void GetMemoryStatistics(ICrySizer* s);

private:
	TAbilities m_abilities;
	EntityId m_ownerId;
};

class CAbilitiesSystem
{
public:
	CAbilitiesSystem();

	friend class CAbility;

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

	void InitHUD(const bool load);
	void ShowHUD(const bool show);

	void Reset();
	void FullSerialize(TSerialize ser);

	void ReloadHUD();

	void OnAction(const ActionId& action, int activationMode, float value);
	void Update();
	void UpdateHUD();
	void UpdateEffectsHUD();
	void UpdateVisibleModeHUD();

	CAbilityOwner& AddAbilityOwner(IActor* pActor);
	bool RemoveAbilityOwner(CAbilityOwner& owner);

	CAbilityOwner& GetAbilityOwner(IActor* pActor);
	CAbilityOwner& GetAbilityOwner(EntityId id);

	bool IsAbilityOwner(IActor* pActor);
	bool IsAbilityOwner(EntityId id);

	void GetMemoryStatistics(ICrySizer* s);
protected:

	bool OnActionLeftAbility(EntityId entityId, const ActionId& action, int activationMode, float value);
	bool OnActionCenterAbility(EntityId entityId, const ActionId& action, int activationMode, float value);
	bool OnActionRightAbility(EntityId entityId, const ActionId& actionId, int activationMode, float value);

	//void SetAbilityState(int abilNumber, EAbilityState state);
	//EAbilityState GetAbilityState(int abilNumber);

	// Summary
	// Sets the current effect of the selected icon
	// First arg: 1, 2, 3
	// Second arg: Default, TrShield, TrEMP, TrRage
	//void SetAbilityVisibleIcon(int iconNumber, const char* effectName);
	//const char* GetAbilityVisibleIcon(int iconNumber);

	// Summary
	// Sets the current mode of the selected icon
	// First arg: 1, 2, 3
	// Second arg: Hide, Show, Activated(play anim), Disabled
	//void SetAbilityVisibleMode(int iconNumber, const char* modeName);
	//const EAbilityVisibleMode GetAbilityVisibleMode(int iconNumber);
public:
	STrooperAbilities trooper;
	SScoutAbilities scout;
	SHunterAbilities hunter;
protected:
	//CGameFlashAnimation m_animAbilitiesHUD;
private:
	//EAbilityState m_leftAbilityState;
	//EAbilityState m_rightAbilityState;
	//EAbilityState m_centerAbilityState;

	//EAbilityVisibleMode m_leftAbilityVisibleMode;
	//EAbilityVisibleMode m_rightAbilityVisibleMode;
	//EAbilityVisibleMode m_centerAbilityVisibleMode;

	//uint32 m_usedAbilitiesFlag;
	static TActionHandler<CAbilitiesSystem> s_actionHandler;

	TAbilityOwners m_abilityOwners;
};