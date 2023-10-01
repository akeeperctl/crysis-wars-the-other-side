#include "StdAfx.h"
#include "../Control/ControlSystem.h"
#include "AbilitiesSystem.h"

void CAbility::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		string abilityName = name;

		ser.BeginGroup("CAbility");
		ser.EnumValue("m_visMode", visMode, EAbilityVisibleMode::eAbilityMode_Hide, EAbilityVisibleMode::eAbilityMode_ForSync);
		ser.EnumValue("m_state", state, EAbilityState::eAbilityState_Ready_To_Activate, EAbilityState::eAbilityState_ForSync);
		ser.Value("m_name", abilityName);
		ser.Value("m_index", index);
		ser.Value("m_abilityOwnerId", abilityOwnerId);
		ser.EndGroup();
	}
}

void CAbility::SetVisibleMode(EAbilityVisibleMode mode)
{
	visMode = mode;

	const auto pPlayer = g_pControlSystem->GetClientActor();

	if (pPlayer && pPlayer->GetEntityId() == abilityOwnerId)
		g_pControlSystem->GetAbilitiesSystem()->UpdateVisibleModeHUD();
}

EAbilityVisibleMode CAbility::GetVisibleMode() const
{
	return visMode;
}

void CAbility::SetState(EAbilityState newState)
{
	state = newState;
}

EAbilityState CAbility::GetState() const
{
	return state;
}

void CAbility::SetName(const char* newName)
{
	name = newName;
}

string CAbility::GetName()
{
	return name;
}

void CAbility::GetMemoryStatistics(ICrySizer* s) const
{
	s->Add(*this);
	s->Add(name);
}
