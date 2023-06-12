#include "stdafx.h"

#include "GameActions.h"
#include "GameUtils.h"

#include "HUD/HUD.h"

#include "Trooper.h"
#include "Scout.h"
#include "Hunter.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
//#include "SquadSystem.h"

TActionHandler<CAbilitiesSystem> CAbilitiesSystem::s_actionHandler;

CAbilitiesSystem::CAbilitiesSystem()
{
	if (s_actionHandler.GetNumHandlers() == 0)
	{
#define ADD_HANDLER(action, func) s_actionHandler.AddHandler(actions.action, &CAbilitiesSystem::func)
		const CGameActions& actions = g_pGame->Actions();

		ADD_HANDLER(ctrl_useleftability, OnActionLeftAbility);
		ADD_HANDLER(ctrl_usecenterability, OnActionCenterAbility);
		ADD_HANDLER(ctrl_userightability, OnActionRightAbility);

#undef ADD_HANDLER
	}

	Reset();
}

void CAbilitiesSystem::Update()
{
	UpdateEffectsHUD();

	//TODO Сделать правильное отображение visMode Activated на HUD
	TAbilityOwners::iterator it = m_abilityOwners.begin();
	TAbilityOwners::iterator end = m_abilityOwners.end();
	for (; it != end; it++)
	{
		CAbilityOwner& abilityOwner = *it;
		if (CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(abilityOwner.m_ownerId)))
		{
			const string actorClass = pActor->GetEntity()->GetClass()->GetName();
			//if (m_usedAbilitiesFlag & eActivatedAbility_LeftAbility)

			if (pActor == g_pControlSystem->GetControlClient()->GetControlledActor() && g_pGameCVars->ctrl_debug_draw == 2)
			{
				static float clr[] = { 1,1,1,1 };
				gEnv->pRenderer->Draw2dLabel(20, 120, 1.15f, clr, false, "Ability owner name: %s", pActor->GetEntity()->GetName());
				gEnv->pRenderer->Draw2dLabel(20, 140, 1.15f, clr, false, "Ability 1 name: %s", abilityOwner.GetAbility(1).GetName());
				gEnv->pRenderer->Draw2dLabel(20, 160, 1.15f, clr, false, "Ability 1 state: %i", abilityOwner.GetAbility(1).GetState());
				gEnv->pRenderer->Draw2dLabel(20, 180, 1.15f, clr, false, "Ability 1 visMode: %i", abilityOwner.GetAbility(1).GetVisibleMode());

				gEnv->pRenderer->Draw2dLabel(20, 240, 1.15f, clr, false, "Ability 2 name: %s", abilityOwner.GetAbility(2).GetName());
				gEnv->pRenderer->Draw2dLabel(20, 260, 1.15f, clr, false, "Ability 2 state: %i", abilityOwner.GetAbility(2).GetState());
				gEnv->pRenderer->Draw2dLabel(20, 280, 1.15f, clr, false, "Ability 2 visMode: %i", abilityOwner.GetAbility(2).GetVisibleMode());

				gEnv->pRenderer->Draw2dLabel(20, 340, 1.15f, clr, false, "Ability 3 name: %s", abilityOwner.GetAbility(3).GetName());
				gEnv->pRenderer->Draw2dLabel(20, 360, 1.15f, clr, false, "Ability 3 state: %i", abilityOwner.GetAbility(3).GetState());
				gEnv->pRenderer->Draw2dLabel(20, 380, 1.15f, clr, false, "Ability 3 visMode: %i", abilityOwner.GetAbility(3).GetVisibleMode());
			}

			if (actorClass == "PlayerTrooper")
			{
				CTrooper* pTrooper = static_cast<CTrooper*>(pActor);

				if (abilityOwner.IsHaveAbility(ABILITY_TROOPER_RAGE))
				{
					CAbility& rageAbility = abilityOwner.GetAbility(ABILITY_TROOPER_RAGE);
					switch (rageAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (rageAbility.GetVisibleMode() != eAbilityMode_Show)
							rageAbility.SetVisibleMode(eAbilityMode_Show);

						float maxHealth = pActor->GetMaxHealth() / 2;
						float curHealth = pActor->GetHealth();

						if (curHealth > maxHealth)
							rageAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Activated:
					{
						if (rageAbility.GetVisibleMode() != eAbilityMode_Activated)
							rageAbility.SetVisibleMode(eAbilityMode_Activated);

						if (pTrooper->m_rageMode.rageMaxDuration == 0)
							rageAbility.SetState(eAbilityState_Cooldown);
					}
					break;
					case eAbilityState_Cooldown:
					{
						if (rageAbility.GetVisibleMode() != eAbilityMode_Disabled)
							rageAbility.SetVisibleMode(eAbilityMode_Disabled);

						/*if (pTrooper->m_rageMode.rageMaxDuration != 0)
							pTrooper->m_rageMode.rageMaxDuration = 0;*/
						pTrooper->m_rageMode.ToggleMode(false);

						if (pTrooper->m_rageMode.reloadDuration == 0)
							rageAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (rageAbility.GetVisibleMode() != eAbilityMode_Disabled)
							rageAbility.SetVisibleMode(eAbilityMode_Disabled);

						const float maxHealth = pActor->GetMaxHealth() / 2;
						const float curHealth = pActor->GetHealth();

						if (curHealth <= maxHealth)
							rageAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					}
				}

				if (abilityOwner.IsHaveAbility(ABILITY_TROOPER_EMP))
				{
					CAbility& empAbility = abilityOwner.GetAbility(ABILITY_TROOPER_EMP);
					switch (empAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (empAbility.GetVisibleMode() != eAbilityMode_Show)
							empAbility.SetVisibleMode(eAbilityMode_Show);
					}
					break;
					case eAbilityState_Activated:
					{
						if (empAbility.GetVisibleMode() != eAbilityMode_Activated)
							empAbility.SetVisibleMode(eAbilityMode_Activated);

						empAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Cooldown:
						break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (empAbility.GetVisibleMode() != eAbilityMode_Disabled)
							empAbility.SetVisibleMode(eAbilityMode_Disabled);

						if (trooper.shockwaveTimer == 0)
							empAbility.SetState(eAbilityState_Ready_To_Activate);
					}
						break;
					}
				}

				if (abilityOwner.IsHaveAbility(ABILITY_TROOPER_SHIELD))
				{
					CAbility& shieldAbility = abilityOwner.GetAbility(ABILITY_TROOPER_SHIELD);
					switch (shieldAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (shieldAbility.GetVisibleMode() != eAbilityMode_Show)
							shieldAbility.SetVisibleMode(eAbilityMode_Show);

						SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, true);
						if (squad.GetLeader())
						{
/*							if (squad.GetMembersCount() > 0)
							{
								if (squad.GetMinDistance() > pTrooper->m_shieldParams.GetRange())
									shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
							}
							else*/ if (squad.GetMembersCount() == 0)
								shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
						}
					}
					break;
					case eAbilityState_Activated:
					{
						if (shieldAbility.GetVisibleMode() != eAbilityMode_Activated)
							shieldAbility.SetVisibleMode(eAbilityMode_Activated);

						SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, true);
						if (squad.GetLeader())
						{
							/*if (squad.GetMembersCount() > 0)
							{
								if (squad.GetMinDistance() < pTrooper->m_shieldParams.GetRange())
									shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
							}
							else*/ if (squad.GetMembersCount() == 0)
								shieldAbility.SetState(eAbilityState_Disabled_By_Condition);

							if (pTrooper->m_shieldParams.canGuardianShieldProj != true)
								pTrooper->m_shieldParams.canGuardianShieldProj = true;
						}
					}
					break;
					case eAbilityState_Cooldown:
						shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
						break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (shieldAbility.GetVisibleMode() != eAbilityMode_Disabled)
							shieldAbility.SetVisibleMode(eAbilityMode_Disabled);

						if (pTrooper->m_shieldParams.canGuardianShieldProj != false)
							pTrooper->m_shieldParams.canGuardianShieldProj = false;						

						SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, true);
						if (squad.GetLeader())
						{
							if (squad.GetMembersCount() > 0)
							{
								//if (squad.GetMinDistance() < pTrooper->m_shieldParams.GetRange())
									shieldAbility.SetState(eAbilityState_Ready_To_Activate);
							}
						}
					}
					break;
					}
				}
			}
			else if (actorClass == "Scout")
			{
				CScout* pScout = static_cast<CScout*>(pActor);

				if (abilityOwner.IsHaveAbility(ABILITY_SCOUT_SPOTLIGHT))
				{
					CAbility& spotAbility = abilityOwner.GetAbility(ABILITY_SCOUT_SPOTLIGHT);
					switch (spotAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (spotAbility.GetVisibleMode() != eAbilityMode_Show)
							spotAbility.SetVisibleMode(eAbilityMode_Show);

						if (!gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
							spotAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
						break;
					case eAbilityState_Activated:
					{
						if (spotAbility.GetVisibleMode() != eAbilityMode_Activated)
							spotAbility.SetVisibleMode(eAbilityMode_Activated);
					}
						break;
					case eAbilityState_Cooldown:
					{
						scout.IsSearch = false;
						//m_bDisableLookAt = false;
						g_pControlSystem->GetControlClient()->m_generic.canShoot = true;

						pScout->EnableSearchBeam(false);

						spotAbility.SetState(eAbilityState_Ready_To_Activate);
					}
						break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (spotAbility.GetVisibleMode() != eAbilityMode_Disabled)
							spotAbility.SetVisibleMode(eAbilityMode_Disabled);

						if (gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
							spotAbility.SetState(eAbilityState_Ready_To_Activate);
					}
						break;
					}
				}

				if (abilityOwner.IsHaveAbility(ABILITY_SCOUT_ANTIGRAV))
				{
					CAbility& antigravAbility = abilityOwner.GetAbility(ABILITY_SCOUT_ANTIGRAV);
					switch (antigravAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (antigravAbility.GetVisibleMode() != eAbilityMode_Show)
							antigravAbility.SetVisibleMode(eAbilityMode_Show);

						if (scout.isGyroEnabled == false)
							antigravAbility.SetState(eAbilityState_Disabled_By_Condition);

						//if (!gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
						//	antigravAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Activated:
					{
						if (antigravAbility.GetVisibleMode() != eAbilityMode_Activated)
							antigravAbility.SetVisibleMode(eAbilityMode_Activated);

						if (scout.isGyroEnabled == true)
							scout.isGyroEnabled = false;
					}
					break;
					case eAbilityState_Cooldown:
					{
						antigravAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (antigravAbility.GetVisibleMode() != eAbilityMode_Disabled)
							antigravAbility.SetVisibleMode(eAbilityMode_Disabled);

						scout.isGyroEnabled = true;
						antigravAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					}
				}
			}
			else if (actorClass == "Hunter")
			{
				CHunter* pHunter = static_cast<CHunter*>(pActor);

				CAbility& htGrabAbility = abilityOwner.GetAbility(ABILITY_HUNTER_GRAB);
				if (abilityOwner.IsHaveAbility(ABILITY_HUNTER_GRAB))
				{
					switch (htGrabAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (htGrabAbility.GetVisibleMode() != eAbilityMode_Show)
							htGrabAbility.SetVisibleMode(eAbilityMode_Show);
					}
					break;
					case eAbilityState_Activated:
					{
						if (htGrabAbility.GetVisibleMode() != eAbilityMode_Activated)
							htGrabAbility.SetVisibleMode(eAbilityMode_Activated);
					}
					break;
					case eAbilityState_Cooldown:
					{
						htGrabAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (htGrabAbility.GetVisibleMode() != eAbilityMode_Disabled)
							htGrabAbility.SetVisibleMode(eAbilityMode_Disabled);
					}
					break;
					}
				}

				if (abilityOwner.IsHaveAbility(ABILITY_HUNTER_SHIELD))
				{
					CAbility& htShieldAbility = abilityOwner.GetAbility(ABILITY_HUNTER_SHIELD);
					switch (htShieldAbility.GetState())
					{
					case eAbilityState_Ready_To_Activate:
					{
						if (htShieldAbility.GetVisibleMode() != eAbilityMode_Show)
							htShieldAbility.SetVisibleMode(eAbilityMode_Show);

						if (pHunter->GetAlienEnergy() <= 0.0f)
							htShieldAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Activated:
					{
						if (htShieldAbility.GetVisibleMode() != eAbilityMode_Activated)
							htShieldAbility.SetVisibleMode(eAbilityMode_Activated);

						//Passive energy drain of hunter's shield now disabled
						/*const float maxShieldEnergy = pHunter->GetMaxAlienEnergy();
						const float currentShieldEnergy = pHunter->GetAlienEnergy();
						const float divider = 100.0f;
						const float deltaEnergy = maxShieldEnergy / divider;

						float frametime = gEnv->pTimer->GetFrameTime();
						static float timer = 1.0f;

						if (timer > 0.0f)
							timer -= frametime;

						if (timer < 0.0f)
						{
							pHunter->SetAlienEnergy(currentShieldEnergy - deltaEnergy);
							timer = 1.0f;
						}*/

						if (pHunter->GetAlienEnergy() <= 0.0f)
							htShieldAbility.SetState(eAbilityState_Disabled_By_Condition);
					}
					break;
					case eAbilityState_Cooldown:
					{
						htShieldAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					case eAbilityState_Disabled_By_Condition:
					{
						if (htShieldAbility.GetVisibleMode() != eAbilityMode_Disabled)
							htShieldAbility.SetVisibleMode(eAbilityMode_Disabled);

						pHunter->m_energyParams.isHunterShieldEnabled = false;

						if (pHunter->GetAlienEnergy() > 0.2f)
							htShieldAbility.SetState(eAbilityState_Ready_To_Activate);
					}
					break;
					}
				}
			}
		}
	}
}

void CAbilitiesSystem::UpdateHUD()
{
	if (!g_pGame->GetHUD())
		return;

	//if (!m_animAbilitiesHUD.IsLoaded())
	//	return;

	CGameFlashAnimation* animAmmo = &g_pGame->GetHUD()->m_animPlayerStats;
	if (!animAmmo->IsLoaded())
		return;

	UpdateEffectsHUD();
	UpdateVisibleModeHUD();
}

void CAbilitiesSystem::UpdateEffectsHUD()
{
	if (!g_pGame->GetHUD())
		return;

	//if (!m_animAbilitiesHUD.IsLoaded())
	//	return;

	CGameFlashAnimation* animAmmo = &g_pGame->GetHUD()->m_animPlayerStats;
	if (!animAmmo->IsLoaded())
		return;

	SControlClient* pCC = g_pControlSystem->GetControlClient();
	if (pCC)
	{
		CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
		if (!pPlayer)
			pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (pPlayer)
		{
			if (IsAbilityOwner(pPlayer->GetEntityId()))
			{
				CAbilityOwner& abilityOwner = GetAbilityOwner(pPlayer->GetEntityId());
				if (abilityOwner.GetAbilityCount() > 0)
				{
					int count = abilityOwner.GetAbilityCount();

					for (int i = 0; i <= count; i++)
					{
						CAbility& ability = abilityOwner.GetAbility(i);

						SFlashVarValue argsEffect[2] = { i, ability.name.c_str() };
						animAmmo->Invoke("setIconEffect", argsEffect, 2);
					}
				}
			}
		}
	}
}

void CAbilitiesSystem::UpdateVisibleModeHUD()
{
	if (!g_pGame->GetHUD())
		return;

	//if (!m_animAbilitiesHUD.IsLoaded())
	//	return;

	CGameFlashAnimation* animAmmo = &g_pGame->GetHUD()->m_animPlayerStats;
	if (!animAmmo->IsLoaded())
		return;

	SControlClient* pCC = g_pControlSystem->GetControlClient();
	if (pCC)
	{
		CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
		if (!pPlayer)
			pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (pPlayer)
		{
			if (IsAbilityOwner(pPlayer->GetEntityId()))
			{
				CAbilityOwner& abilityOwner = GetAbilityOwner(pPlayer->GetEntityId());
				if (abilityOwner.GetAbilityCount() > 0)
				{
					int count = abilityOwner.GetAbilityCount();

					for (int i = 0; i <= count; i++)
					{
						CAbility& ability = abilityOwner.GetAbility(i);

						const char* visMode = 0;
						bool isActivatedMode = false;

						switch (ability.GetVisibleMode())
						{
						case eAbilityMode_Hide:
							visMode = VISIBLE_MODE_HIDE;
							break;
						case eAbilityMode_Show:
							visMode = VISIBLE_MODE_SHOW;
							break;
						case eAbilityMode_Disabled:
							visMode = VISIBLE_MODE_DISABLED;
							break;
						case eAbilityMode_Activated:
							visMode = VISIBLE_MODE_ACTIVATED;
							isActivatedMode = true;
							break;
						}

						//if (!isActivatedMode)
						//{
						//	SFlashVarValue argsMode[2] = { i, visMode };
						//	m_animAbilitiesHUD.Invoke("setIconMode", argsMode, 2);
						//}
						//else
						//	m_animAbilitiesHUD.Invoke("setIconActivated", i);

						if (!isActivatedMode)
						{
							SFlashVarValue argsMode[2] = { i, visMode };
							animAmmo->Invoke("setIconMode", argsMode, 2);
						}
						else
							animAmmo->Invoke("setIconActivated", i);

					}
				}
			}
		}
	}
}

CAbilityOwner& CAbilitiesSystem::AddAbilityOwner(IActor* pActor)
{
	if (pActor)
	{
		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();

		bool found = false;
		for (; it != end; it++)
		{
			if (it->m_ownerId == pActor->GetEntityId())
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			//m_allSquads.push_back(squad);
			//int index = GetFreeSquadIndex();
			//squad.m_squadId = index;

			//if (squad.GetLeader() == g_pGame->GetIGameFramework()->GetClientActor())
			//	m_dudeSquad = squad;

			//CryLogAlways("index %d", index);

			//m_allSquads[index] = squad;
			CAbilityOwner owner;
			owner.SetEntityId(pActor->GetEntityId());

			m_abilityOwners.push_back(owner);
			return owner;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING,
				"[CAbilitiesSystem::AddAbilityOwner] Try create already existed ability owner");
		}
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING,
		"[CAbilitiesSystem::AddAbilityOwner] Try return null ability owner");
	return CAbilityOwner();
}

bool CAbilitiesSystem::RemoveAbilityOwner(CAbilityOwner& owner)
{
	TAbilityOwners::iterator it = m_abilityOwners.begin();
	TAbilityOwners::iterator end = m_abilityOwners.end();

	bool found = false;
	for (; it != end; it++)
	{
		if (it->m_ownerId == owner.m_ownerId)
		{
			*it = CAbilityOwner();
			m_abilityOwners.erase(it);
			return true;
		}
	}

	return false;
}

void CAbilitiesSystem::OnAction(const ActionId& action, int activationMode, float value)
{
	CActor* pActor = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pActor)
		pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pActor)
	{
		if (IsAbilityOwner(pActor))
			s_actionHandler.Dispatch(this, pActor->GetEntityId(), action, activationMode, value);
	}
}

bool CAbilitiesSystem::OnActionLeftAbility(EntityId entityId, const ActionId& action, int activationMode, float value)
{
	if (CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId)))
	{
		CAbilityOwner& abilityOwner = GetAbilityOwner(entityId);
		if (abilityOwner.GetAbilityCount() != 0)
		{
			if (activationMode == eAAM_OnPress)
				abilityOwner.ToggleAbility(1);
		}
	}
	return false;
}

bool CAbilitiesSystem::OnActionCenterAbility(EntityId entityId, const ActionId& action, int activationMode, float value)
{
	CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));
	if (pActor)
	{
		CAbilityOwner& abilityOwner = GetAbilityOwner(entityId);
		if (abilityOwner.GetAbilityCount() != 0)
		{
			if (activationMode == eAAM_OnPress)
				abilityOwner.ToggleAbility(2);
		}
	}

	return false;
}

bool CAbilitiesSystem::OnActionRightAbility(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));
	if (pActor)
	{
		CAbilityOwner& abilityOwner = GetAbilityOwner(entityId);
		if (abilityOwner.GetAbilityCount() != 0)
		{
			if (activationMode == eAAM_OnPress)
				abilityOwner.ToggleAbility(3);
		}
	}

	return false;
}

CAbilityOwner& CAbilitiesSystem::GetAbilityOwner(IActor* pActor)
{
	if (pActor)
	{
		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();

		for (; it != end; it++)
		{
			if (it->m_ownerId == pActor->GetEntityId())
				return *it;
		}
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAbilityOwner& CAbilitiesSystem::GetAbilityOwner(IActor* pActor) !!!Get NULL CAbilityOwner!!!");
	return CAbilityOwner();
}

CAbilityOwner& CAbilitiesSystem::GetAbilityOwner(EntityId id)
{
	if (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
	{
		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();

		for (; it != end; it++)
		{
			if (it->m_ownerId == id)
				return *it;
		}
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAbilityOwner& CAbilitiesSystem::GetAbilityOwner(IActor* pActor) !!!Get NULL CAbilityOwner!!!");
	return CAbilityOwner();
}

bool CAbilitiesSystem::IsAbilityOwner(IActor* pActor)
{
	if (pActor)
	{
		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();

		for (; it != end; it++)
		{
			if (it->m_ownerId == pActor->GetEntityId())
				return true;
		}
	}

	return false;
}

bool CAbilitiesSystem::IsAbilityOwner(EntityId id)
{
	if (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
	{
		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();

		for (; it != end; it++)
		{
			if (it->m_ownerId == id)
				return true;
		}
	}

	return false;
}

void CAbilitiesSystem::InitHUD(const bool load)
{
	if (load)
	{
		//m_animAbilitiesHUD.Load("Libs/UI/HUD_Abilities.swf", eFD_Center);
		//m_animAbilitiesHUD.SetVisible(false);
	}
	else
	{
		//m_animAbilitiesHUD.Unload();
		//CryLogAlways("Abilities Hud unloaded");
	}
}

void CAbilitiesSystem::ShowHUD(const bool show)
{
	//m_animAbilitiesHUD.SetVisible(show);
}

void CAbilitiesSystem::Reset()
{
	trooper.Reset();
	scout.Reset();
	hunter.Reset();

	TAbilityOwners::iterator it = m_abilityOwners.begin();
	TAbilityOwners::iterator end = m_abilityOwners.end();
	for (; it != end; it++)
		it->Reset();

	m_abilityOwners.clear();
	//m_animAbilitiesHUD.Reload(false);
	//m_animAbilitiesHUD.SetVisible(true);
}

void CAbilitiesSystem::FullSerialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network && !gEnv->bEditor)
	{
		if (ser.IsReading())
			Reset();

		ser.BeginGroup("CAbilitiesSystem");

		trooper.Serialize(ser);
		scout.Serialize(ser);
		hunter.Serialize(ser);

		TAbilityOwners::iterator it = m_abilityOwners.begin();
		TAbilityOwners::iterator end = m_abilityOwners.end();
		for (; it != end; it++)
			it->Serialize(ser);

		ser.EndGroup();
	}
}

void CAbilitiesSystem::ReloadHUD()
{
	//m_animAbilitiesHUD.Reload(true);
	//m_usedAbilitiesFlag = 0;

	//string abilityCls = abilityClassName;

	//if (abilityCls == ABILITY_DEFAULT || abilityCls.size() == 0)
	//{
	//	//SetIconMode(LEFT_ABILITY, ABILITY_MODE_HIDE);
	//	//SetIconMode(CENTER_ABILITY, ABILITY_MODE_HIDE);
	//	//SetIconMode(RIGHT_ABILITY, ABILITY_MODE_HIDE);

	//	//Need for a debug
	//	SetAbilityVisibleMode(LEFT_ABILITY, VISIBLE_MODE_SHOW);
	//	SetAbilityVisibleMode(CENTER_ABILITY, VISIBLE_MODE_SHOW);
	//	SetAbilityVisibleMode(RIGHT_ABILITY, VISIBLE_MODE_SHOW);

	//	SetAbilityVisibleIcon(LEFT_ABILITY, ABILITY_DEFAULT);
	//	SetAbilityVisibleIcon(CENTER_ABILITY, ABILITY_DEFAULT);
	//	SetAbilityVisibleIcon(RIGHT_ABILITY, ABILITY_DEFAULT);
	//}
	//else if (abilityCls == "Trooper")
	//{
	//	m_usedAbilitiesFlag |= eActivatedAbility_LeftAbility;

	//	SetAbilityVisibleMode(LEFT_ABILITY, VISIBLE_MODE_SHOW);
	//	SetAbilityVisibleMode(CENTER_ABILITY, VISIBLE_MODE_HIDE);
	//	SetAbilityVisibleMode(RIGHT_ABILITY, VISIBLE_MODE_HIDE);

	//	SetAbilityVisibleIcon(LEFT_ABILITY, ABILITY_TROOPER_RAGE);
	//	SetAbilityVisibleIcon(CENTER_ABILITY, ABILITY_DEFAULT);
	//	SetAbilityVisibleIcon(RIGHT_ABILITY, ABILITY_DEFAULT);
	//}
	//else if (abilityCls == "TrooperGuardian")
	//{
	//	m_usedAbilitiesFlag |= eActivatedAbility_LeftAbility | eActivatedAbility_RightAbility;

	//	SetAbilityVisibleMode(LEFT_ABILITY, VISIBLE_MODE_SHOW);
	//	SetAbilityVisibleMode(RIGHT_ABILITY, VISIBLE_MODE_SHOW);
	//	SetAbilityVisibleMode(CENTER_ABILITY, VISIBLE_MODE_HIDE);

	//	SetAbilityVisibleIcon(RIGHT_ABILITY, ABILITY_TROOPER_EMP);
	//	SetAbilityVisibleIcon(LEFT_ABILITY, ABILITY_TROOPER_SHIELD);
	//	//SetAbilityIcon(CENTER_ABILITY, ABILITY_ICON_DEFAULT);
	//}
	//else if (abilityCls == "TrooperLeader")
	//{
	//	m_usedAbilitiesFlag |= eActivatedAbility_RightAbility;

	//	SetAbilityVisibleMode(LEFT_ABILITY, VISIBLE_MODE_HIDE);
	//	SetAbilityVisibleMode(CENTER_ABILITY, VISIBLE_MODE_HIDE);
	//	SetAbilityVisibleMode(RIGHT_ABILITY, VISIBLE_MODE_SHOW);

	//	SetAbilityVisibleIcon(LEFT_ABILITY, ABILITY_DEFAULT);
	//	SetAbilityVisibleIcon(CENTER_ABILITY, ABILITY_DEFAULT);
	//	SetAbilityVisibleIcon(RIGHT_ABILITY, ABILITY_TROOPER_EMP);
	//}
	//else if (abilityCls == "Scout")
	//{
	//}
	//else if (abilityCls == "ScoutLeader")
	//{
	//}
	//else if (abilityCls == "ScoutBomber")
	//{
	//}
	//else if (abilityCls == "Hunter")
	//{
	//}
	//else if (abilityCls == "HunterRed")
	//{
	//}
	//else if (abilityCls == "Naked")
	//{
	//}
}

//void CAbilitiesSystem::SetAbilityVisibleIcon(int iconNumber, const char* effectName)
//{
//	if (m_animAbilitiesHUD.IsLoaded())
//	{
//		SFlashVarValue args[2] = { iconNumber, effectName };
//		m_animAbilitiesHUD.Invoke("setIconEffect", args, 2);
//	}
//}
//
//void CAbilitiesSystem::SetAbilityVisibleMode(int iconNumber, const char* modeName)
//{
//	if (m_animAbilitiesHUD.IsLoaded())
//	{
//		string mode = modeName;
//
//		if (mode != "Activated")
//		{
//			SFlashVarValue args[2] = { iconNumber, mode.c_str() };
//			m_animAbilitiesHUD.Invoke("setIconMode", args, 2);
//		}
//		else
//			m_animAbilitiesHUD.Invoke("setIconActivated", iconNumber);
//
//		switch (iconNumber)
//		{
//		case LEFT_ABILITY:
//			if (mode == "Hide")
//				m_leftAbilityVisibleMode = eAbilityMode_Hide;
//			else if (mode == "Show")
//				m_leftAbilityVisibleMode = eAbilityMode_Show;
//			else if (mode == "Activated")
//				m_leftAbilityVisibleMode = eAbilityMode_Activated;
//			else if (mode == "Disabled")
//				m_leftAbilityVisibleMode = eAbilityMode_Disabled;
//			break;
//		case CENTER_ABILITY:
//			if (mode == "Hide")
//				m_centerAbilityVisibleMode = eAbilityMode_Hide;
//			else if (mode == "Show")
//				m_centerAbilityVisibleMode = eAbilityMode_Show;
//			else if (mode == "Activated")
//				m_centerAbilityVisibleMode = eAbilityMode_Activated;
//			else if (mode == "Disabled")
//				m_centerAbilityVisibleMode = eAbilityMode_Disabled;
//			break;
//		case RIGHT_ABILITY:
//			if (mode == "Hide")
//				m_rightAbilityVisibleMode = eAbilityMode_Hide;
//			else if (mode == "Show")
//				m_rightAbilityVisibleMode = eAbilityMode_Show;
//			else if (mode == "Activated")
//				m_rightAbilityVisibleMode = eAbilityMode_Activated;
//			else if (mode == "Disabled")
//				m_rightAbilityVisibleMode = eAbilityMode_Disabled;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
//const EAbilityVisibleMode CAbilitiesSystem::GetAbilityVisibleMode(int iconNumber)
//{
//	switch (iconNumber)
//	{
//	case LEFT_ABILITY:
//		return m_leftAbilityVisibleMode;
//	case CENTER_ABILITY:
//		return m_centerAbilityVisibleMode;
//	case RIGHT_ABILITY:
//		return m_rightAbilityVisibleMode;
//	default:
//		break;
//	}
//}

void CAbilitiesSystem::STrooperAbilities::Reset()
{
	shockwaveTimer = 0;
	rageDuration = 30.0f;

	eCeilingType = eCeilingType_Ceiling;

	isRageMode = false;
	//isProjectingShield = false;

	canCeiling = false;
	doCeiling = false;
	isCeiling = false;
}

void CAbilitiesSystem::STrooperAbilities::Serialize(TSerialize ser)
{
	ser.BeginGroup("TrooperAbilities");
	ser.Value("isRageMode", isRageMode);
	//ser.Value("isProjectingShield", isProjectingShield);
	ser.Value("rageDuration", rageDuration);
	ser.Value("shockwaveTimer", shockwaveTimer);
	ser.Value("canCeiling", canCeiling);
	ser.Value("doCeiling", doCeiling);
	ser.Value("isCeiling", isCeiling);
	ser.EndGroup();
}

void CAbilitiesSystem::SScoutAbilities::Reset()
{
	abilityPressNum = 0;

	IsSearch = false;
	isGyroEnabled = true;
}

void CAbilitiesSystem::SScoutAbilities::Serialize(TSerialize ser)
{
	ser.BeginGroup("ScoutAbilities");
	ser.Value("abilityPressNum", abilityPressNum);
	ser.Value("IsSearch", IsSearch);
	ser.Value("isGyroEnabled", isGyroEnabled);
	ser.EndGroup();
}

void CAbilitiesSystem::SHunterAbilities::Reset()
{
	IsShieldEnabled = false;
}

void CAbilitiesSystem::SHunterAbilities::Serialize(TSerialize ser)
{
	ser.BeginGroup("HunterAbilities");
	ser.Value("IsShieldEnabled", IsShieldEnabled);
	ser.EndGroup();
}

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

	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pPlayer->GetEntityId() == abilityOwnerId)
	{
		//g_pControlSystem->GetAbilitiesSystem()->m_animAbilitiesHUD.Reload(true);
		//g_pControlSystem->GetAbilitiesSystem()->m_animAbilitiesHUD.SetVisible(true);
		g_pControlSystem->GetAbilitiesSystem()->UpdateVisibleModeHUD();

		CryLogAlways("[CAbility %s]::SetVisibleMode(%i)", GetName(), int(mode));
	}	
}

const EAbilityVisibleMode CAbility::GetVisibleMode()
{
	return visMode;
}

void CAbility::SetState(EAbilityState newState)
{
	state = newState;
}

const EAbilityState CAbility::GetState()
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

//void CAbilityOwner::ToggleAbility(const char* name)
//{
//	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_ownerId);
//	if (!pActor)
//		return;
//
//	if (pActor->GetHealth() <= 0.0f)
//		return;
//
//	if (pActor->GetEntity()->IsHidden())
//		return;
//
//	string className = pActor->GetEntity()->GetClass()->GetName();
//	string abilityName = name;
//
//	if (IsHaveAbility(abilityName))
//	{
//		if (className == "PlayerTrooper" || 
//			className == "Trooper")
//		{
//			CTrooper* pTrooper = static_cast<CTrooper*>(pActor);
//			if (abilityName == ABILITY_TROOPER_SHIELD)
//			{
//				CAbility& shieldAbility = GetAbility(abilityName);
//				if (shieldAbility.GetState() == eAbilityState_Ready_To_Activate)
//				{
//					//Turn on a ability 
//					if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian)
//					{
//						//pTrooper->m_shieldParams.canGuardianShieldProj = !pTrooper->m_shieldParams.canGuardianShieldProj;
//						//pSys->trooper.isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj = true;
//						//pTrooper->m_shieldParams.canGuardianShieldProj = true;
//
//						//bool isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj;
//						//pSys->trooper.isProjectingShield = isProjectingShield;
//
//						shieldAbility.SetState(eAbilityState_Activated);
//					}
//					else
//					{
//						//TODO: Hud информирует о невозможности использования способности
//					}
//				}
//				else if (shieldAbility.GetState() == eAbilityState_Activated)
//				{
//					//Turn off a ability
//					//pSys->trooper.isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj = false;
//					//pTrooper->m_shieldParams.canGuardianShieldProj = false;
//					shieldAbility.SetState(eAbilityState_Cooldown);
//				}
//
//			}
//		}	
//	}
//}

void CAbilityOwner::ToggleAbility(int index)
{
	IActor* pOwnerActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_ownerId);
	if (!pOwnerActor)
		return;

	if (pOwnerActor->GetHealth() <= 0.0f)
		return;

	if (pOwnerActor->GetEntity()->IsHidden())
		return;

	CAbilitiesSystem* pSys = g_pControlSystem->GetAbilitiesSystem();
	if (!pSys)
		return;

	if (IsHaveAbility(index))
	{
		CAbility& ability = GetAbility(index);

		string abilityName = ability.GetName();
		string className = pOwnerActor->GetEntity()->GetClass()->GetName();

		if (className == "PlayerTrooper" ||
			className == "Trooper")
		{
			CTrooper* pTrooper = static_cast<CTrooper*>(pOwnerActor);
			if (abilityName == ABILITY_TROOPER_SHIELD)
			{
				CAbility& shieldAbility = GetAbility(abilityName);
				if (shieldAbility.GetState() == eAbilityState_Ready_To_Activate)
				{
					//Turn on a ability 
					if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian)
					{
						//pTrooper->m_shieldParams.canGuardianShieldProj = !pTrooper->m_shieldParams.canGuardianShieldProj;
						//pSys->trooper.isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj = true;
						//pTrooper->m_shieldParams.canGuardianShieldProj = true;

						//bool isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj;
						//pSys->trooper.isProjectingShield = isProjectingShield;

						shieldAbility.SetState(eAbilityState_Activated);
					}
					else
					{
						//TODO: Hud информирует о невозможности использования способности
					}
				}
				else if (shieldAbility.GetState() == eAbilityState_Activated)
				{
					//Turn off a ability
					//pSys->trooper.isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj = false;
					//pTrooper->m_shieldParams.canGuardianShieldProj = false;
					shieldAbility.SetState(eAbilityState_Cooldown);
				}

			}
			if (abilityName == ABILITY_TROOPER_RAGE)
			{
				CAbility& rageAbility = GetAbility(abilityName);
				if (rageAbility.GetState() == eAbilityState_Ready_To_Activate)
				{
					if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Owner)
					{
						const bool isRageMode = pSys->trooper.isRageMode = (!pTrooper->m_rageMode.isActive && !pTrooper->m_rageMode.isReloading);
						const float rageDuration = pSys->trooper.rageDuration;

						pTrooper->m_rageMode.ToggleMode(isRageMode, rageDuration);
						rageAbility.SetState(eAbilityState_Activated);
					}
					else
					{
						//TODO: Hud информирует о невозможности использования способности
					}
				}
				else if (rageAbility.GetState() == eAbilityState_Activated)
				{
					rageAbility.SetState(eAbilityState_Cooldown);
				}
			}
			if (abilityName == ABILITY_TROOPER_EMP)
			{
				CAbility& empAbility = GetAbility(abilityName);
				if (empAbility.GetState() == eAbilityState_Ready_To_Activate)
				{
					const bool isElite = pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian ||
						pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Leader;
					if (isElite && pSys->trooper.shockwaveTimer == 0)
					{
						const float timerSec = 15.0f;//150

						if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian)
							pSys->trooper.shockwaveTimer = timerSec;
						else if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Leader)
							pSys->trooper.shockwaveTimer = timerSec;

						ExplosionInfo info;
						info.weaponId = info.shooterId = pOwnerActor->GetEntityId();
						info.damage = 50;
						info.pos = pOwnerActor->GetEntity()->GetWorldPos();
						info.pos.z += 1.5;
						info.dir = pOwnerActor->GetEntity()->GetWorldTM().GetColumn2();
						info.radius = 6;
						info.angle = 0;
						info.pressure = 0;
						info.hole_size = 10;
						info.type = 21;//g_pGame->GetGameRules()->GetHitTypeId("emp");
						info.SetEffect("expansion_fx.weapons.emp_grenade", 0.8f, 0.0f);

						g_pGame->GetGameRules()->ServerExplosion(info);

						if (IsHaveAbility(ABILITY_TROOPER_SHIELD))
							GetAbility(ABILITY_TROOPER_SHIELD).SetState(eAbilityState_Disabled_By_Condition);

						SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pOwnerActor, 1);
						if (squad.GetLeader() != 0)
						{
							//Get the shield owners from the player squad members
							TMembers::const_iterator it = squad.GetAllMembers().begin();
							TMembers::const_iterator end = squad.GetAllMembers().end();
							for (; it != end; ++it)
							{
								IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(it->actorId);
								if (pMember)
								{
									const Vec3 leaderPos = pOwnerActor->GetEntity()->GetWorldPos();
									const Vec3 memberPos = pMember->GetEntity()->GetWorldPos();

									if ((leaderPos - memberPos).GetLength() < 15.0f)
									{
										const float maxHealth = pMember->GetMaxHealth();
										const float currentHealth = pMember->GetHealth();
										const float divider = 6.0f;
										float addValue = maxHealth / divider;

										if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Leader)
											addValue = maxHealth / 2;

										pMember->SetHealth(currentHealth + addValue);
									}
								}
							}
						}

						empAbility.SetState(eAbilityState_Activated);
					}
				}
			}
		}
		else if (className == "Hunter")
		{
			CHunter* pHunter = static_cast<CHunter*>(pOwnerActor);
			if (abilityName == ABILITY_HUNTER_GRAB)
			{			
				IEntity* pCrossEntity = g_pControlSystem->GetControlClient()->GetLastCrosshairEntity();
				if (pCrossEntity)
				{
					IActor* pCrossActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pCrossEntity->GetId());
					IVehicle* pCrossVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pCrossEntity->GetId());

					if (pCrossActor || pCrossVehicle)
					{
						IAIObject* pOwnerAI = pOwnerActor->GetEntity()->GetAI();
						IAIObject* pCrossEntityAI = pCrossEntity->GetAI();
							
						bool isActor = false;
						bool isVehicle = false;

						if ((pCrossActor && pCrossActor->GetHealth() > 0.0f) && (pCrossEntityAI && pOwnerAI && pCrossEntityAI->IsHostile(pOwnerAI, false)))
							isActor = true;
						else if (pCrossVehicle)
							isVehicle = true;
						else
							return;

						float distance = (pOwnerActor->GetEntity()->GetWorldPos() - pCrossEntity->GetWorldPos()).GetLength();
						if (distance <= 30.0f)
						{
							IScriptTable* pScriptTable = pOwnerActor->GetEntity()->GetScriptTable();
							if (pScriptTable)
							{
								string schemeName = "";
								if (isActor)
									schemeName = "Mouth";
								else if (isVehicle)
								{
									ICharacterInstance* pChar = pHunter->GetEntity()->GetCharacter(0);
									if (pChar)
									{
										//Hunter not have left grab animation :(

										int leftLegId = pChar->GetISkeletonPose()->GetJointIDByName("frontLegLeft12");
										int rightLegId = pChar->GetISkeletonPose()->GetJointIDByName("frontLegRight12");

										Vec3 leftLegPos = pHunter->GetBoneWorldPos("frontLegLeft12");
										Vec3 rightLegPos = pHunter->GetBoneWorldPos("frontLegRight12");

										float leftLegDist = (pCrossEntity->GetWorldPos() - leftLegPos).GetLength();
										float rightLegDist = (pCrossEntity->GetWorldPos() - rightLegPos).GetLength();

										if (leftLegDist < rightLegDist)
											schemeName = "Left";
										else
											schemeName = "Right";
									}
								}
										
								Vec3& crosshairPos = g_pControlSystem->GetControlClient()->GetCrosshairPos();

								Script::CallMethod(pScriptTable, "AbilityHunterThrow", pCrossEntity->GetId(), crosshairPos, schemeName.c_str());
							}
						}
					}
				}
				
			}
			if (abilityName == ABILITY_HUNTER_SHIELD)
			{
				CAbility& htShieldAbility = GetAbility(abilityName);
				if (htShieldAbility.GetState() == eAbilityState_Ready_To_Activate)
				{
					if (!pHunter->m_energyParams.isHunterShieldEnabled)
						htShieldAbility.SetState(eAbilityState_Activated);

					pHunter->m_energyParams.isHunterShieldEnabled = true;					
				}
				else if (htShieldAbility.GetState() == eAbilityState_Activated)
				{
					pHunter->m_energyParams.isHunterShieldEnabled = false;
					htShieldAbility.SetState(eAbilityState_Cooldown);
				}

				IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
				if (pClientActor)
				{
					const char* szSound = 0;
					if (pHunter->IsShieldEnabled())
						szSound = "sounds/environment:sphere_sfx:hunter_shield_on";
					else
						szSound = "sounds/environment:sphere_sfx:hunter_shield_off";

					IEntitySoundProxy* pSoundProxy = (IEntitySoundProxy*)pClientActor->GetEntity()->GetProxy(ENTITY_PROXY_SOUND);
					if (pSoundProxy)
						pSoundProxy->PlaySound(szSound, Vec3Constants<float>::fVec3_Zero, Vec3Constants<float>::fVec3_OneY, 0, eSoundSemantic_HUD);
				}				
			}
			
		}
		else if (className == "Scout")
		{
			CScout* pScout = static_cast<CScout*>(pOwnerActor);
			if (abilityName == ABILITY_SCOUT_SPOTLIGHT)
			{
				if (gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
				{
					if (ability.GetState() == eAbilityState_Ready_To_Activate)
					{
						pSys->scout.IsSearch = true;
						g_pControlSystem->GetControlClient()->m_generic.canShoot = false;

						pScout->EnableSearchBeam(true);
						ability.SetState(eAbilityState_Activated);
					}
					else if (ability.GetState() == eAbilityState_Activated)
						ability.SetState(eAbilityState_Cooldown);
				}
			}
			else if (abilityName == ABILITY_SCOUT_ANTIGRAV)
			{
				if (ability.GetState() == eAbilityState_Ready_To_Activate)
					ability.SetState(eAbilityState_Activated);
				else if (ability.GetState() == eAbilityState_Activated)
					ability.SetState(eAbilityState_Cooldown);
			}
		}
	}
}

int CAbilityOwner::AddAbility(const char* name)
{
	if (IsHaveAbility(name))
		return -1;

	CAbility ability;
	ability.index = m_abilities.size() + 1;
	ability.name = name;
	ability.abilityOwnerId = m_ownerId;
	//ability.SetVisibleMode(eAbilityMode_Show);
	m_abilities.push_back(ability);

	

	return ability.index;
}

//int CAbilityOwner::AddAbility(const char* name, int index)
//{
//	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
//	if (!pPlayer)
//		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
//
//	if (IsHaveAbility(name))
//		return -1;
//
//	CAbility ability;
//	ability.m_index = index;
//
//	m_abilities.push_back(ability);
//	return index;
//}

int CAbilityOwner::AddAbility(CAbility& ability)
{
	if (IsHaveAbility(ability))
		return -1;

	ability.index = m_abilities.size() + 1;
	ability.abilityOwnerId = m_ownerId;
	//ability.SetVisibleMode(eAbilityMode_Show);
	m_abilities.push_back(ability);

	/*CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pPlayer->GetEntityId() == m_ownerId)
		g_pControlSystem->GetAbilitiesSystem()->UpdateEffectsHUD();*/

	return ability.index;
}

//int CAbilityOwner::AddAbility(CAbility& ability, int index)
//{
//	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
//	if (!pPlayer)
//		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
//
//	if (IsHaveAbility(ability))
//		return -1;
//
//	ability.m_index = index;
//
//	m_abilities.push_back(ability);
//	return m_abilities.size();
//}

bool CAbilityOwner::RemoveAbility(const char* name)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		string abilityName = it->name;
		if (abilityName == name)
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		m_abilities.erase(it);

		TAbilities::iterator it = m_abilities.begin();
		TAbilities::iterator end = m_abilities.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		return true;
	}
	return false;
}

bool CAbilityOwner::RemoveAbility(int index)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		if (it->index == index)
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		m_abilities.erase(it);

		TAbilities::iterator it = m_abilities.begin();
		TAbilities::iterator end = m_abilities.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		return true;
	}
	return false;
}

bool CAbilityOwner::RemoveAbility(CAbility& ability)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		string abilityName = it->name;
		if (abilityName == ability.name)
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		m_abilities.erase(it);

		TAbilities::iterator it = m_abilities.begin();
		TAbilities::iterator end = m_abilities.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		return true;
	}
	return false;
}

bool CAbilityOwner::IsHaveAbility(const char* name)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();
	for (; it != end; it++)
	{
		if (it->GetName() == name)
			return true;
	}

	return false;
}

bool CAbilityOwner::IsHaveAbility(int index)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();
	for (; it != end; it++)
	{
		if (index == it->index)
			return true;
	}

	return false;
}

bool CAbilityOwner::IsHaveAbility(const CAbility& ability)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();
	for (; it != end; it++)
	{
		string abilityName = it->name;
		if (abilityName == ability.name)
			return true;
	}

	return false;
}

CAbility& CAbilityOwner::GetAbility(const char* name)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	for (; it != end; it++)
	{
		string abilityName = it->name;
		if (abilityName == name)
			return *it;
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "[CAbility &CAbilityOwner::GetAbility name] Get nullAbility");
	//CryLogAlways("[CAbility &CAbilityOwner::GetAbility id] Get nullAbility");
	return CAbility();
}

CAbility& CAbilityOwner::GetAbility(int index)
{
	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	for (; it != end; it++)
	{
		if (it->index == index)
			return *it;
	}

	//CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "[CAbility &CAbilityOwner::GetAbility index] Get nullAbility");
	//CryLogAlways("[CAbility &CAbilityOwner::GetAbility id] Get nullAbility");
	return CAbility();
}

int CAbilityOwner::GetAbilityCount()
{
	return m_abilities.size();
}

void CAbilityOwner::SetEntityId(EntityId id)
{
	m_ownerId = id;
}

EntityId CAbilityOwner::GetEntityId() const
{
	return m_ownerId;
}

void CAbilityOwner::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		ser.BeginGroup("CAbilityOwner");
		ser.Value("m_ownerId", m_ownerId);

		TAbilities abilities;

		if (ser.IsWriting())
		{
			abilities = m_abilities;
			ser.Value("m_abilities", abilities);
		}
		else
		{
			ser.Value("m_abilities", abilities);

			TAbilities::iterator it = m_abilities.begin();
			TAbilities::iterator end = m_abilities.end();
			for (; it != end; it++)
			{
				AddAbility(*it);
			}
		}

		TAbilities::iterator it = m_abilities.begin();
		TAbilities::iterator end = m_abilities.end();
		for (; it != end; it++)
		{
			it->Serialize(ser);
		}

		ser.EndGroup();
	}
}

void CAbilitiesSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	TAbilityOwners::iterator it = m_abilityOwners.begin();
	TAbilityOwners::iterator end = m_abilityOwners.end();

	for (; it != end; it++)
		it->GetMemoryStatistics(s);
}

void CAbilityOwner::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	TAbilities::iterator it = m_abilities.begin();
	TAbilities::iterator end = m_abilities.end();

	for (; it != end; it++)
		it->GetMemoryStatistics(s);
}


void CAbility::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}