#pragma once

#include "Ability.h"

class CAbilitiesSystem;

class CAbilityOwner
{
public:
	friend class CAbilitiesSystem;

	CAbilityOwner(EntityId id)
	{
		Reset();
		m_ownerId = id;
	}

	CAbilityOwner()
	{
		Reset();
	}

	~CAbilityOwner()
	{
		Reset();
	}

	//Summary
	//1 - Left Ability
	//2 - Center Ability
	//3 - Right Ability
	//4 - etc.
	//void ToggleAbility(const char* name);
	void ToggleAbility(int index, EntityId targetId);
	//void ToggleAbility(CAbility& ability);

	static void DeactivateAbility(CAbility* pAbility);

	int AddAbility(const char* name);
	//int AddAbility(const char* name, int index);
	int AddAbility(CAbility& ability);
	//int AddAbility(CAbility& ability, int index);

	bool RemoveAbility(const char* name);
	bool RemoveAbility(int index);
	bool RemoveAbility(const CAbility& ability);

	bool IsHaveAbility(const char* name);
	bool IsHaveAbility(int index);
	bool IsHaveAbility(const CAbility& ability);

	CAbility* GetAbility(const char* name);
	CAbility* GetAbility(int index);

	int GetAbilityCount() const;

	void SetEntityId(EntityId id);
	EntityId GetEntityId() const;

	void Reset();

	void Serialize(TSerialize ser);
	void GetMemoryStatistics(ICrySizer* s);

private:
	TAbilities m_abilities;
	EntityId m_ownerId;
};