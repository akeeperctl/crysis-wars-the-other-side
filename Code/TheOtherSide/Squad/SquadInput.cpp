#include "StdAfx.h"

#include <IHardwareMouse.h>

#include "Menus/FlashMenuObject.h"
#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDSilhouettes.h"

#include "Actor.h"
#include "Player.h"
#include "GameActions.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"

bool CSquadSystem::OnInputEvent(const SInputEvent& event)
{
	auto* pClientPlayer = static_cast<CActor*>(g_pControlSystem->GetClientActor());
	if (!pClientPlayer)
		return false;

	if (gEnv->pConsole->IsOpened())
		return false;

	if (event.keyName.c_str() && event.keyName.c_str()[0] == 0)
		return false;

	//bool inMenu = g_pGame->GetMenu()->IsActive();
	//bool inCutscene = g_pGame->;

	//if (inCutscene)
	//	return false;

	auto inGamePaused = g_pGame->GetIGameFramework()->IsGamePaused();
	auto playerId = pClientPlayer->GetEntityId();

	if (!inGamePaused && pClientPlayer->GetHealth() > 0)
	{
		if (event.deviceId == eDI_Keyboard)
		{
			if (event.keyId == eKI_LAlt)
				OnInputCommandMode(playerId, event.state, event.value);

			auto pSquad = GetSquadFromLeader(pClientPlayer);
			if (!pSquad)
				return false;

			if (event.keyId == eKI_1)
			{
				const auto selectedAll = pSquad->IsAllMembersSelected();
				const auto selected1 = pSquad->IsMemberSelected(0) && !selectedAll;

				if (!selected1)
					OnInputSelectMember(0, playerId, event.state, event.value);
				else
					OnInputSelectAll(playerId, event.state, event.value);

				//Deselect All
				if (selectedAll)
					OnInputSelectMember(-1, playerId, event.state, event.value);
			}
			else if (event.keyId == eKI_2)
				OnInputSelectMember(1, playerId, event.state, event.value);
			else if (event.keyId == eKI_3)
				OnInputSelectMember(2, playerId, event.state, event.value);
			else if (event.keyId == eKI_4)
				OnInputSelectMember(3, playerId, event.state, event.value);
			else if (event.keyId == eKI_5)
				OnInputSelectMember(4, playerId, event.state, event.value);
			else if (event.keyId == eKI_6)
				OnInputSelectMember(5, playerId, event.state, event.value);
			else if (event.keyId == eKI_7)
				OnInputSelectMember(6, playerId, event.state, event.value);
			else if (event.keyId == eKI_8)
				OnInputSelectMember(7, playerId, event.state, event.value);
			else if (event.keyId == eKI_9)
				OnInputSelectMember(8, playerId, event.state, event.value);

			if (event.keyId == eKI_Z)
				OnInputOrderFollow(playerId, event.state, event.value);
			
			if (event.keyId == eKI_F)
			{
				if (strcmp(pClientPlayer->GetEntity()->GetClass()->GetName(), "Trooper") == 0)
				{
					const bool clientGrabbed = pClientPlayer->GetGrabStats()->isGrabbed;
					auto pGrabber = GET_ENTITY(pClientPlayer->GetGrabStats()->grabberId);

					if (clientGrabbed && pGrabber && strcmp(pGrabber->GetClass()->GetName(), "Scout") == 0)
						Script::CallMethod(pGrabber->GetScriptTable(), "DropEntitiesFromTentacles");
				}

			}
			//else if (event.keyId == eKI_T)
			//	OnInputSwitchOrder(playerId, event.state, event.value);
			//else if (event.keyId == eKI_R)
			//	OnInputExecuteOrder(playerId, event.state, event.value);
		}
		else if (event.deviceId == eDI_Mouse)
		{
			if (event.keyId == eKI_Mouse2 && event.state == eIS_Pressed)
				UpdateMouseOrdersHUD();
			
		}
	}

	return false;
}

bool CSquadSystem::OnInputSelectMember(int index, EntityId entityId, EInputState activationMode, float value)
{
	if (!m_isCommandMode)
		return false;

	auto* pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));

	if (activationMode == eIS_Pressed)
	{
		auto pSquad = GetSquadFromLeader(pPlayer);
		if (pSquad)
		{
			pSquad->RemoveAllSelected();

			if (pSquad->HasClientLeader() && index != -1)
				pSquad->AddMemberToSelected(index);	

			UpdateSelectedHUD();
		}
	}

	return false;
}

bool CSquadSystem::OnInputSelectAll(EntityId entityId, EInputState activationMode, float value)
{
	if (!m_isCommandMode)
		return false;

	if (activationMode == eIS_Pressed)
	{
		auto* pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));

		auto pSquad = GetSquadFromLeader(pPlayer);
		if (pSquad)
		{
			if (pSquad->HasClientLeader())
			{
				pSquad->RemoveAllSelected();
				
				for (auto& member : pSquad->GetAllMembers())
					pSquad->AddMemberToSelected(&member);

				UpdateSelectedHUD();
			}
		}
	}

	return false;
}

bool CSquadSystem::OnInputOrderFollow(EntityId entityId, EInputState activationMode, float value)
{
	if (activationMode != eIS_Pressed)
		return false;

	if (!m_isCommandMode)
		return false;

	auto* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	auto* pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));
	if (!pPlayer)
		return false;

	auto* pSquad = GetSquadFromLeader(pPlayer);
	if (pSquad && pSquad->HasClientLeader())
	{
		pSquad->RemoveAllSelected();

		for (auto& member : pSquad->GetAllMembers())
			pSquad->AddMemberToSelected(&member);

		for (auto& memberId : pSquad->m_selectedMembers)
		{
			auto pMember = pSquad->GetMemberInstance(memberId);
			if (pMember)
			{
				//Ignore enemy when goto to leader but when reached leader combat is enabled
				SOrderInfo order;

				//Ignore flag setup it from ClientApplyExecution
				//order.ignoreFlag = eOIEF_IgnoreEnemyWhenGotoTarget;

				const auto leaderPos = pSquad->GetLeader()->GetEntity()->GetWorldPos();
				const auto memberPos = GET_ENTITY(pMember->GetId())->GetWorldPos();
				const auto id = pSquad->GetLeader()->GetEntityId();//Leader id

				order.type = eSO_FollowLeader;

				ClientApplyExecution(pMember, order, eEOF_ExecutedByKeyboard, id, leaderPos, true);
				
				if (strcmp(pPlayer->GetEntity()->GetClass()->GetName(), "Scout") == 0)
				{
					if ((leaderPos - memberPos).GetLength() < 40.0f)
					{
						if (g_pControlSystem->GetAbilitiesSystem()->IsAbilityOwner(pPlayer))
						{
							const auto pAbilityOwner = g_pControlSystem->GetAbilitiesSystem()->GetAbilityOwner(pPlayer);
							if (pAbilityOwner && pAbilityOwner->IsHaveAbility(ABILITY_SCOUT_OBJECTGRAB))
							{
								const auto pAbility = pAbilityOwner->GetAbility(ABILITY_SCOUT_OBJECTGRAB);
								if (pAbility)
									pAbilityOwner->ToggleAbility(pAbility->index, memberId);
							}
						}
					}
				}
			}
		}

		pSquad->RemoveAllSelected();
		UpdateSelectedHUD();
	}
	
	return false;
}

bool CSquadSystem::OnInputCommandMode(EntityId entityId, EInputState activationMode, float value)
{
	auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
	//if (pActor != g_pGame->GetIGameFramework()->GetClientActor())
	if (!pActor)
		return false;

	auto pSquad = GetSquadFromLeader(pActor);
	if (!pSquad || (pSquad && pSquad->GetMembersCount() == 0))
		return false;

	if (activationMode == eIS_Pressed)
		SetCommandMode(true);
	else if (activationMode == eIS_Released)
		SetCommandMode(false);

	return false;
}