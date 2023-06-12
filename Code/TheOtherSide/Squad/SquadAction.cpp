#include "StdAfx.h"

#include "Player.h"
#include "GameActions.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"

bool SSquadSystem::OnAction(const ActionId& actionId, int activationMode, float value)
{
	s_actionHandler.Dispatch(this, 0, actionId, activationMode, value);

	return false;
}

bool SSquadSystem::OnActionSelectOne(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (activationMode == eAAM_OnPress)
	{
		SSquad& squad = GetSquadFromLeader(pPlayer);

		if (squad.isPlayerLeader())
		{
			//m_selectedMembersId = { 0,0,0,0 };
			squad.m_squadSelectedMembers.clear();

			/*if (IActor* pMember = GetMemberFromSlot(0))
				m_selectedMembersId[0] = pMember->GetEntityId();*/

			squad.AddMemberToSelected(0);

			UpdateSelectedHUD();
		}
	}

	return false;
}

bool SSquadSystem::OnActionSelectTwo(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (activationMode == eAAM_OnPress)
	{
		SSquad& squad = GetSquadFromLeader(pPlayer);

		if (squad.isPlayerLeader())
		{
			//m_selectedMembersId = { 0,0,0,0 };
			squad.m_squadSelectedMembers.clear();

			/*if (IActor* pMember = GetMemberFromSlot(0))
				m_selectedMembersId[0] = pMember->GetEntityId();*/

			squad.AddMemberToSelected(1);

			UpdateSelectedHUD();
		}
	}

	return false;
}

bool SSquadSystem::OnActionSelectThree(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (activationMode == eAAM_OnPress)
	{
		SSquad& squad = GetSquadFromLeader(pPlayer);

		if (squad.isPlayerLeader())
		{
			//m_selectedMembersId = { 0,0,0,0 };
			squad.m_squadSelectedMembers.clear();

			/*if (IActor* pMember = GetMemberFromSlot(0))
				m_selectedMembersId[0] = pMember->GetEntityId();*/

			squad.AddMemberToSelected(2);

			UpdateSelectedHUD();
		}
	}

	return false;

	/*CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		SControlClient* pCC = pDude->GetControlClient();
		if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
			return false;
	}

	if (activationMode == eAAM_OnPress)
	{
		if (IsEnabled())
		{
			m_selectedMembersId = { 0,0,0,0 };

			if (IActor* pMember = GetMemberFromSlot(2))
			{
				m_selectedMembersId[2] = pMember->GetEntityId();
			}

			CheckInvoked();
		}
	}
	return false;*/
}

bool SSquadSystem::OnActionSelectAll(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (activationMode == eAAM_OnPress)
	{
		SSquad& squad = GetSquadFromLeader(pPlayer);

		if (squad.isPlayerLeader())
		{
			squad.m_squadSelectedMembers.clear();

			TMembers::const_iterator it = squad.m_squadMembers.begin();
			TMembers::const_iterator end = squad.m_squadMembers.end();

			for (; it != end; it++)
				squad.AddMemberToSelected(*it);

			UpdateSelectedHUD();
		}
	}

	return false;
}

bool SSquadSystem::OnActionSelectNone(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (activationMode == eAAM_OnPress)
	{
		SSquad& squad = GetSquadFromLeader(pPlayer);

		if (squad.isPlayerLeader())
		{
			squad.m_squadSelectedMembers.clear();
			UpdateSelectedHUD();
		}
	}

	return false;

	/*CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		SControlClient* pCC = pDude->GetControlClient();
		if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
			return false;
	}

	if (activationMode == eAAM_OnPress)
	{
		if (IsEnabled())
			m_selectedMembersId.clear();

		CheckInvoked();
	}
	return false;*/
}

bool SSquadSystem::OnActionTest(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CryLogAlways("[SSquadSystem::OnActionTest()] Called");
	return false;
}

bool SSquadSystem::OnActionOrderFollow(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromLeader(pPlayer);

	if (squad.isPlayerLeader())
	{
		TSMembers::iterator it = squad.m_squadSelectedMembers.begin();
		TSMembers::iterator end = squad.m_squadSelectedMembers.end();
		for (; it != end; it++)
		{
			SMember& member = squad.GetMember(*it);

			member.currentOrder = eSquadOrders_FollowLeader;
			squad.ExecuteOrder(eSquadOrders_FollowLeader, member);
		}
	}
	return false;
}

bool SSquadSystem::OnActionSwitchOrder(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromLeader(pPlayer);

	if (squad.isPlayerLeader())
	{
		if (activationMode == eAAM_OnPress)
		{
			static int pressNum = 0;

			pressNum++;

			if (pressNum == 1)
			{
				squad.m_eLeaderCurrentOrder = eSquadOrders_SearchEnemy;
			}
			else if (pressNum == 2)
			{
				squad.m_eLeaderCurrentOrder = eSquadOrders_GoTo;
				pressNum = 0;
			}
		}
	}
	return false;
}

bool SSquadSystem::OnActionExecuteOrder(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromLeader(pPlayer);
	if (squad.isPlayerLeader())
	{
		TSMembers::iterator it = squad.m_squadSelectedMembers.begin();
		TSMembers::iterator end = squad.m_squadSelectedMembers.end();
		for (; it != end; it++)
		{
			SMember& member = squad.GetMember(*it);

			member.currentOrder = squad.m_eLeaderCurrentOrder;
			squad.ExecuteOrder(squad.m_eLeaderCurrentOrder, member);
		}
	}
	return false;
}

bool SSquadSystem::OnActionCommandMode(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude->GetSlaveId() != 0)
		return false;

	if (activationMode == eAAM_OnPress)
	{
		m_isCommandMode = true;
	}

	else if (activationMode == eAAM_OnRelease)
	{
		m_isCommandMode = false;
	}

	return false;
}

bool SSquadSystem::OnActionSwitchControlled(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return false;

	SControlClient* pCC = pDude->GetControlClient();
	if (pCC->m_pControlledActor == 0 && !m_isCommandMode)
		return false;

	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromLeader(pPlayer);

	if (squad.isPlayerLeader())
	{
		if (activationMode == eAAM_OnPress)
		{
			/*static std::vector<EntityId> members = m_membersId;
			std::vector<EntityId>::iterator it = members.begin();

			for (; it != members.end(); it++)
			{
				if (IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it))
				{
					if (pMember->GetHealth() > 0.0f && pMember != GetLeader())
					{
						CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
						if (pDude)
						{
							SControlClient* pCC = pDude->GetControlClient();
							if (pCC)
							{
								pCC->SetActor((CActor*)pMember);
								SetLeader(pMember);
							}
						}
					}
				}
			}*/
		}
	}

	return false;
}