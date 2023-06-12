#include "StdAfx.h"

#include "GameActions.h"
#include "GameUtils.h"

#include "Player.h"
#include "Scout.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDSilhouettes.h"

#include "Single.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"

TActionHandler<SSquadSystem> SSquadSystem::s_actionHandler;

SSquadSystem::SSquadSystem() :
	m_isCommandMode(false),
	m_mustShowSquadControls(true)
{
	/*if (g_pControlSystem && g_pControlSystem->pSquadSystem)
		m_pSquadSystem = g_pControlSystem->pSquadSystem;*/

	if (s_actionHandler.GetNumHandlers() == 0)
	{
#define ADD_HANDLER(action, func) s_actionHandler.AddHandler(actions.action, &SSquadSystem::func)
		const CGameActions& actions = g_pGame->Actions();

		ADD_HANDLER(squad_commandmode, OnActionCommandMode);
		ADD_HANDLER(squad_switchorder, OnActionSwitchOrder);
		ADD_HANDLER(squad_executeorder, OnActionExecuteOrder);
		ADD_HANDLER(squad_followplayer, OnActionOrderFollow);
		ADD_HANDLER(squad_selectone, OnActionSelectOne);
		ADD_HANDLER(squad_selecttwo, OnActionSelectTwo);
		ADD_HANDLER(squad_selectthree, OnActionSelectThree);
		ADD_HANDLER(squad_selectall, OnActionSelectAll);
		ADD_HANDLER(squad_selectnone, OnActionSelectNone);

		//ADD_HANDLER(squad_test, OnActionTest);

#undef ADD_HANDLER

		m_allSquads.clear();
		//CryLogAlways("[SSquadSystem] Constructor");
	}
};
SSquadSystem::~SSquadSystem()
{
	m_allSquads.clear();
	//CryLogAlways("[SSquadSystem] Destructor");
}

void SSquadSystem::Update()
{
	UpdateHUD();

	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		if (it->GetLeader() != 0)
			it->Update();
	}
	//static float clr[] = { 1,1,1,1 };

	//std::vector<EntityId>::const_iterator it = m_membersId.begin();
	//std::vector<EntityId>::const_iterator end = m_membersId.end();

	//bool debug = false;
	//if (debug)
	//{
	//	static float clr[] = { 1,1,1,1 };
	//	gEnv->pRenderer->Draw2dLabel(20, 160, 1.5, clr, false,"Slot 0 id: %d", m_membersId[0]);
	//	gEnv->pRenderer->Draw2dLabel(20, 180, 1.5, clr, false, "Selected Slot 0 id: %d", m_selectedMembersId[0]);

	//	gEnv->pRenderer->Draw2dLabel(20, 200, 1.5, clr, false, "Slot 1 id: %d", m_membersId[1]);
	//	gEnv->pRenderer->Draw2dLabel(20, 220, 1.5, clr, false, "Selected Slot 1 id: %d", m_selectedMembersId[1]);

	//	gEnv->pRenderer->Draw2dLabel(20, 240, 1.5, clr, false, "Slot 2 id: %d", m_membersId[2]);
	//	gEnv->pRenderer->Draw2dLabel(20, 260, 1.5, clr, false, "Selected Slot 2 id: %d", m_selectedMembersId[2]);

	//	gEnv->pRenderer->Draw2dLabel(20, 280, 1.5, clr, false, "Slot 3 id: %d", m_membersId[3]);
	//	gEnv->pRenderer->Draw2dLabel(20, 300, 1.5, clr, false, "Selected Slot 3 id: %d", m_selectedMembersId[3]);
	//}

	//when the player in the squad is dead, switch him to a alive actor in the squad.
	//if (g_pControlSystem->GetControlledActor() && g_pControlSystem->GetControlledActor()->GetHealth() < 0.1f )
	//{
	//	//gEnv->pRenderer->Draw2dLabel(40, 240, 3, clr, false, "DEAD");

	//	static float timer = 1.0f;

	//	if (timer != 0)
	//	{
	//		timer -= gEnv->pTimer->GetFrameTime();
	//		if (timer< 0)
	//			timer = 0;
	//	}
	//
	//	if(timer == 0)
	//	{
	//		if (IActor* pMember = GetAliveMember())
	//		{
	//			CActor* pActor = static_cast<CActor*>(pMember);

	//			g_pControlSystem->SetActor(pActor);
	//			SetLeader(pActor);

	//			timer = 1.0f;
	//		}
	//	}

		//gEnv->pRenderer->Draw2dLabel(40, 280, 3, clr, false, "TIMER %1.f", timer);
	//}
}

void SSquadSystem::UpdateHUD()
{
	if (!g_pGame->GetHUD())
		return;

	if (!m_animSquadMembers.IsLoaded())
		return;

	ShowPlayerOrder();
	ShowAllSquadControls(g_pGameCVars->sqd_HideControls == 0 ? 1 : 0);

	SControlClient* pCC = 0;
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		pCC = pDude->GetControlClient();
		if (pCC)
		{
			if (pCC->m_pControlledActor == 0)
				ShowAllSquadControlsRed(!m_isCommandMode);
			else
				ShowAllSquadControlsRed(false);
		}
		else
		{
			return;
		}
	}
	CActor* pPlayer = static_cast<CActor*>(pCC->GetControlledActor());
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pPlayer)
	{
		SSquad& squad = GetSquadFromMember(pPlayer, 1);
		TMembers::const_iterator it = squad.GetAllMembers().begin();
		TMembers::const_iterator end = squad.GetAllMembers().end();

		for (; it != end; it++)
		{
			IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(it->actorId);
			if (pMember && pMember != squad.GetLeader())
			{
				float fHealth = (pMember->GetHealth() / (float)pMember->GetMaxHealth()) * 100.0f + 1.0f;
				const int index = squad.GetIndexFromMember(pMember);

				IEntity* pMemberEntity = pMember->GetEntity();
				IAIObject* pMemberAI = pMember->GetEntity()->GetAI();

				IVehicle* pMemberVehicle = pMember->GetLinkedVehicle();
				if (pMemberVehicle)
				{
					pMemberEntity = pMemberVehicle->GetEntity();
					pMemberAI = pMemberVehicle->GetEntity()->GetAI();
					fHealth = 101.0f - pMemberVehicle->GetDamageRatio();
				}

				static char buffer[256];
				sprintf(buffer, "%d", index + 1);// in flash file numeration starts with 1
				static string sOrderName = "";
				const string sMemberNumber = buffer;

				const string sFinalHealFuncName = "setHealthMember" + sMemberNumber;
				const string sFinalOrderFuncName = "setCurrentOrder" + sMemberNumber;
				m_animSquadMembers.CheckedInvoke(sFinalHealFuncName.c_str(), (int)fHealth);

				CHUDSilhouettes* pSil = SAFE_HUD_FUNC_RET(GetSilhouettes());
				if (pSil)
				{
					if (squad.IsMemberSelected(pMember))
						pSil->SetSilhouette(pMemberEntity, 1.0f, 1.0f, 1.0f, 1.0f, -3.0f);
					else
						pSil->ResetSilhouette(pMemberEntity->GetId());
				}

				if (pMemberAI)
				{
					if (IPipeUser* pPU = pMemberAI->CastToIPipeUser())
					{
						if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_SEARCH)
							sOrderName = "Search";
						else if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_GOTO)
							sOrderName = "Go to";
						else if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_GOTO_GUARD)
							sOrderName = "Guard";
						else if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_FOLLOW || pPU->GetGoalPipeId() == GOALPIPEID_ORDER_FOLLOW_QUICKLY)
							sOrderName = "Follow";
						else if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_COOLDOWN)
							sOrderName = "Cooldown";
						else
						{
							const int iTargetThreat = pPU->GetAttentionTargetThreat();
							const bool isHaveTarget = pPU->GetAttentionTarget() ? true : false;

							if (isHaveTarget)
							{
								if (iTargetThreat == AITHREAT_AGGRESSIVE)
									sOrderName = "Combat";
							}
							else
							{
								if (iTargetThreat > AITHREAT_INTERESTING)
									sOrderName = "Alerted";
								else if (iTargetThreat == AITHREAT_INTERESTING)
									sOrderName = "Interested";
								else if (iTargetThreat == AITHREAT_NONE)
									sOrderName = "None";
							}
						}
					}
				}
				else
					sOrderName = "No AI";

				if (pMember->GetHealth() <= 0)
					sOrderName = "Dead";

				m_animSquadMembers.Invoke(sFinalOrderFuncName.c_str(), sOrderName.c_str());
			}
		}
	}
}

void SSquadSystem::FullSerialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network && !gEnv->bEditor)
	{
		TSquads::iterator it = m_allSquads.begin();
		TSquads::iterator end = m_allSquads.end();
		for (; it != end; it++)
		{
			it->Serialize(ser);
		}

		ser.BeginGroup("SSquadSystem");
		SER_VALUE(m_mustShowSquadControls);
		ser.EndGroup();
	}
}

SSquad& SSquadSystem::GetSquadFromMember(IActor* pActor, bool includeLeader)
{
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		if (!includeLeader)
		{
			if (it->IsMember(pActor))
				return *it;
		}
		else
		{
			if (it->IsMember(pActor))
				return *it;

			if (it->GetLeader() == pActor)
				return *it;
		}
	}

	//CryLogAlways("[SSquadSystem::GetSquadFromMember Warning! Return null squad from member]");
	return SSquad();
}

SSquad& SSquadSystem::GetSquadFromLeader(IActor* pLeader)
{
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		if (it->GetLeader() == pLeader)
			return *it;
	}

	CryLogAlways("Return Null squad from leader");
	return SSquad();
}

SSquad& SSquadSystem::GetSquadFromId(int squadId)
{
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		if (it->GetId() == squadId)
			return *it;
	}

	CryLogAlways("[SSquadSystem::GetSquadFromId] Warning! Return null squad");
	return SSquad();
}

bool SSquadSystem::CreateSquad(SSquad& squad)
{
	if (squad.GetLeader() != 0)
	{
		TSquads::iterator it = m_allSquads.begin();
		TSquads::iterator end = m_allSquads.end();

		bool found = false;
		for (; it != end; it++)
		{
			if (it->m_pLeader == squad.GetLeader())
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			//m_allSquads.push_back(squad);
			int index = GetFreeSquadIndex();
			squad.m_squadId = index;

			//if (squad.GetLeader() == g_pGame->GetIGameFramework()->GetClientActor())
			//	m_dudeSquad = squad;

			//CryLogAlways("index %d", index);

			//m_allSquads[index] = squad;
			m_allSquads.push_back(squad);
			return true;
		}
		else
		{
			CryLogAlways("[SSquadSystem::CreateSquad] Try create already existing squad");
		}
	}

	return false;
}

bool SSquadSystem::RemoveSquad(SSquad& squad)
{
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	bool found = false;
	for (; it != end; it++)
	{
		if (it->m_pLeader == squad.GetLeader())
		{
			*it = SSquad();
			m_allSquads.erase(it);
			return true;
		}
	}

	return false;
}

int SSquadSystem::GetFreeSquadIndex() const
{
	//if (m_allSquads.size() == 0)
	//	return 0;

	/*TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		CryLogAlways("GetFreeSquadIndex %d", it - m_allSquads.begin());
		if (it->m_squadId == -1)
		{
			int pos = it - m_allSquads.begin();
			return pos;
		}
	}*/

	return m_allSquads.size() + 1;//Get the next free index from the array
}

void SSquadSystem::ResetSystem()
{
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (; it != end; it++)
	{
		it->Reset();
	}
	m_isCommandMode = false;
	m_mustShowSquadControls = true;
}

void SSquadSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	
	TSquads::iterator it = m_allSquads.begin();
	TSquads::iterator end = m_allSquads.end();

	for (;it!=end;it++)
		it->GetMemoryStatistics(s);
}