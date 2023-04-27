#pragma once

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

class CAbility;
typedef std::vector<CAbility> TAbilities;

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
	}

	void Serialize(TSerialize ser);
	void GetMemoryStatistics(ICrySizer* s) const;

protected:
	void SetVisibleMode(EAbilityVisibleMode mode);
	EAbilityVisibleMode GetVisibleMode() const;

	void SetState(EAbilityState state);
	EAbilityState GetState() const;

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