#include <StdAfx.h>

#include "HUD/HUD.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"

void SSquadSystem::ShowAllSquadControlsRed(bool active)
{
	if (!m_animSquadMembers.IsLoaded() && !m_mustShowSquadControls)
		return;

	m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.gotoAndStop", active ? "Red" : "Show");
}

void SSquadSystem::ShowAllSquadControls(bool active)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	m_mustShowSquadControls = active;

	m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.gotoAndStop", m_mustShowSquadControls ? "Show" : "Hide");
}

void SSquadSystem::InitHUD(bool show)
{
	if (show)
	{
		m_animSquadMembers.Load("Libs/UI/HUD_SquadMembers.swf", eFD_Left);
		m_animSquadMembers.SetVisible(false);
	}
	else
	{
		m_animSquadMembers.Unload();
	}
}

void SSquadSystem::RemoveHUD()
{
	SSquad nullSquad;
	nullSquad.PlayerRemoved();
}

void SSquadSystem::ShowSquadMember(const bool active, const int slot)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	//CryLogAlways("ShowSquadMember active = %d, slot = %d", active, slot);

	ShowSquadControl(slot, active);

	if (active)
	{
		if (slot == 0)
			m_animSquadMembers.CheckedInvoke("_root.Root.member1_hud.gotoAndStop", "Show");
		else if (slot == 1)
			m_animSquadMembers.CheckedInvoke("_root.Root.member2_hud.gotoAndStop", "Show");
		else if (slot == 2)
			m_animSquadMembers.CheckedInvoke("_root.Root.member3_hud.gotoAndStop", "Show");
		else if (slot == 3)
			m_animSquadMembers.CheckedInvoke("_root.Root.member4_hud.gotoAndStop", "Show");
		else if (slot == 4)
			m_animSquadMembers.CheckedInvoke("_root.Root.member5_hud.gotoAndStop", "Show");
		else if (slot == 5)
			m_animSquadMembers.CheckedInvoke("_root.Root.member6_hud.gotoAndStop", "Show");
	}
	else
	{
		if (slot == 0)
			m_animSquadMembers.CheckedInvoke("_root.Root.member1_hud.gotoAndStop", "Hide");
		else if (slot == 1)
			m_animSquadMembers.CheckedInvoke("_root.Root.member2_hud.gotoAndStop", "Hide");
		else if (slot == 2)
			m_animSquadMembers.CheckedInvoke("_root.Root.member3_hud.gotoAndStop", "Hide");
		if (slot == 3)
			m_animSquadMembers.CheckedInvoke("_root.Root.member4_hud.gotoAndStop", "Hide");
		else if (slot == 4)
			m_animSquadMembers.CheckedInvoke("_root.Root.member5_hud.gotoAndStop", "Hide");
		else if (slot == 5)
			m_animSquadMembers.CheckedInvoke("_root.Root.member6_hud.gotoAndStop", "Hide");
	}
}

void SSquadSystem::ShowDeadSquadMember(int slot)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	ShowSquadControl(slot, false);

	if (slot == 0)
		m_animSquadMembers.CheckedInvoke("_root.Root.member1_hud.gotoAndPlay", "Dead");
	else if (slot == 1)
		m_animSquadMembers.CheckedInvoke("_root.Root.member2_hud.gotoAndPlay", "Dead");
	else if (slot == 2)
		m_animSquadMembers.CheckedInvoke("_root.Root.member3_hud.gotoAndPlay", "Dead");
	else if (slot == 3)
		m_animSquadMembers.CheckedInvoke("_root.Root.member4_hud.gotoAndPlay", "Dead");
	else if (slot == 4)
		m_animSquadMembers.CheckedInvoke("_root.Root.member5_hud.gotoAndPlay", "Dead");
	else if (slot == 5)
		m_animSquadMembers.CheckedInvoke("_root.Root.member6_hud.gotoAndPlay", "Dead");
}

void SSquadSystem::ShowSquadControl(int index, bool show)
{
	if (index == 0)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.one.gotoAndStop", show ? "Show" : "Hide");
	else if (index == 1)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.two.gotoAndStop", show ? "Show" : "Hide");
	else if (index == 2)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.three.gotoAndStop", show ? "Show" : "Hide");
	else if (index == 3)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.four.gotoAndStop", show ? "Show" : "Hide");
	else if (index == 4)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.five.gotoAndStop", show ? "Show" : "Hide");
	else if (index == 5)
		m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.six.gotoAndStop", show ? "Show" : "Hide");
}

void SSquadSystem::UpdateSelectedHUD()
{
	CActor* pPlayer = static_cast<CActor*>(g_pControlSystem->GetControlClient()->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromMember(pPlayer, 1);
	if (squad.GetLeader() != 0 && squad.GetMembersCount() > 0)
	{
		if (m_animSquadMembers.IsLoaded())
		{
			m_animSquadMembers.Invoke("setSelectedMember1", squad.IsMemberSelected(0) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember2", squad.IsMemberSelected(1) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember3", squad.IsMemberSelected(2) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember4", squad.IsMemberSelected(3) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember5", squad.IsMemberSelected(4) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember6", squad.IsMemberSelected(5) ? 1 : 0);
		}
	}
}
void SSquadSystem::ShowPlayerOrder()
{
	CActor* pPlayer = static_cast<CActor*>(g_pControlSystem->GetControlClient()->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SSquad& squad = GetSquadFromMember(pPlayer, 1);

	if ((squad.isPlayerMember() || squad.isPlayerLeader()) && m_animSquadMembers.IsLoaded())
	{
		string sPlayerOrder = "";
		switch (squad.m_eLeaderCurrentOrder)
		{
		case eSquadOrders_GoTo:
			sPlayerOrder = "Go to";
			break;
		case eSquadOrders_SearchEnemy:
			sPlayerOrder = "Search";
			break;
		}

		m_animSquadMembers.Invoke("setPlayerCurrentOrder", sPlayerOrder.c_str());
	}
}