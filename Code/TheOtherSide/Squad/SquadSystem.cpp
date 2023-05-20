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
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Conqueror/ConquerorCommander.h"
#include "TheOtherSide/Conqueror/StrategicArea.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Helpers/TOS_Debug.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "TheOtherSide/Helpers/TOS_AI.h"
#include "TheOtherSide/Helpers/TOS_Inventory.h"

#include <ITimer.h>
#include <ISound.h>

//TActionHandler<CSquadSystem> CSquadSystem::s_actionHandler;

CSquadSystem::CSquadSystem()
{
	m_isCommandMode= false;
	m_isDebugLog = false;
	m_showSquadControls = true;
	m_allSquads.clear();
	m_availableMouseOrders.clear();
	m_bookedHidespots.clear();
	m_storedMouseId = 0;
	m_storedMouseWorldPos = Vec3(0, 0, 0);

	m_allowedDetachedOrders.push_back(eSO_Guard);
	m_allowedDetachedOrders.push_back(eSO_ConqGoTo);
	m_allowedDetachedOrders.push_back(eSO_SubPrimaryPickupItem);
	m_allowedDetachedOrders.push_back(eSO_SubSecondaryPickupItem);
	m_allowedDetachedOrders.push_back(eSO_SubPrimaryShootAt);
	m_allowedDetachedOrders.push_back(eSO_SubSecondaryShootAt);
	m_allowedDetachedOrders.push_back(eSO_DebugStanceCrouch);
	m_allowedDetachedOrders.push_back(eSO_DebugStanceStanding);
	m_allowedDetachedOrders.push_back(eSO_DebugStanceStealth);
	m_allowedDetachedOrders.push_back(eSO_DebugStanceRelaxed);

	m_ordersStringMap.clear();
	for (auto i = 0; i < eSO_Last; i++)
	{
		ESquadOrders order = ESquadOrders(i);
		if (order == eSO_Guard || order == eSO_ConqGoTo)
			m_ordersStringMap[order] = "tos_goto";
		else if (order == eSO_SearchEnemy)
			m_ordersStringMap[order] = "tos_search_enemy";
		else if (order == eSO_FollowLeader)
			m_ordersStringMap[order] = "tos_follow_leader";
		else if (order == eSO_SubPrimaryShootAt)
			m_ordersStringMap[order] = "tos_primary_shoot_at";
		else if (order == eSO_SubSecondaryShootAt)
			m_ordersStringMap[order] = "tos_secondary_shoot_at";
		else if (order == eSO_SubEnterVehicle)
			m_ordersStringMap[order] = "tos_enter_vehicle";
		else if (order == eSO_SubExitVehicle)
			m_ordersStringMap[order] = "tos_exit_vehicle";
		else if (order == eSO_SubPrimaryPickupItem)
			m_ordersStringMap[order] = "tos_primary_pickup_item";
		else if (order == eSO_SubSecondaryPickupItem)
			m_ordersStringMap[order] = "tos_secondary_pickup_item";
		//else if (order == eSO_SubAlienGrabPlayerSquad)
			//m_ordersStringMap[order] = "tos_alien_grab_squad";
		//else if (order == eSO_SubAlienDropPlayerSquad)
			//m_ordersStringMap[order] = "tos_alien_drop_squad";
		else if (order == eSO_SubUseVehicleTurret)
			m_ordersStringMap[order] = "tos_use_vehicle_turret";
		else if (order == eSO_DebugEnableCombat)
			m_ordersStringMap[order] = "tos_enable_combat";
		else if (order == eSO_DebugDisableCombat)
			m_ordersStringMap[order] = "tos_disable_combat";
		else if (order == eSO_DebugStanceCrouch)
			m_ordersStringMap[order] = "tos_stance_crouch";
		else if (order == eSO_DebugStanceRelaxed)
			m_ordersStringMap[order] = "tos_stance_relaxed";
		else if (order == eSO_DebugStanceStanding)
			m_ordersStringMap[order] = "tos_stance_standing";
		else if (order == eSO_DebugStanceStealth)
			m_ordersStringMap[order] = "tos_stance_stealth";		
		else if (order == eSO_SearchCoverAroundPoint || order == eSO_ConqSearchCoverAroundArea)
			m_ordersStringMap[order] = "tos_search_cover";
		else if (order == eSO_ConqBlankAction)
			m_ordersStringMap[order] = "tos_blank_action";
		else if (order == eSO_ScoutGrabMe)
			m_ordersStringMap[order] = "tos_scout_grab_me";

	}
};
CSquadSystem::~CSquadSystem()
{
	if (g_pControlSystem)
		g_pControlSystem->RemoveChild(this, false);
}

void CSquadSystem::OnMainMenuEnter()
{
	OnGameRulesReset();
}

void CSquadSystem::Init()
{
	g_pControlSystem->AddChild(this, false);
}

void CSquadSystem::Update(float frametime)
{
	m_isDebugLog = g_pGameCVars->tos_debug_log_all == 1;

	UpdateHUD();

	auto it = m_allSquads.begin();
	const auto end = m_allSquads.end();
	for (; it != end; it++)
	{
		CSquad* pSquad = *it;

		pSquad->UpdateStats(frametime);

		if (pSquad->GetLeader() != 0)
			pSquad->UpdateOrders(frametime);

		if (g_pControlSystem->GetConquerorSystem()->IsGamemode())
			pSquad->UpdateConquerorDetached(frametime);

		//Debug
		if (g_pGameCVars->sqd_debug_draw_Info > 0)
		{
			static float color[] = { 1,1,1,1 };
			const auto size = 1.2f;
			const auto scale = 30;
			const auto xoffset = 60;
			const auto yoffset = 30;

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + pSquad->GetId() * scale, size, color, false,
				"Squad %i, Size %i, Is have leader %i, Player is Leader %i, Species %i",
				pSquad->GetId(), pSquad->GetMembersCount(), pSquad->GetLeader() != nullptr, pSquad->GetLeader() ? pSquad->GetLeader() == g_pControlSystem->GetClientActor() : false, (int)pSquad->GetSpecies());
		}
	}

	//if (gEnv->pAISystem)
	//{
	//	std::vector<EntityId> forDelete;

	//	for (auto& hideSpot : m_bookedHidespots)
	//	{
	//		auto pEntity = gEnv->pEntitySystem->GetEntity(hideSpot.first);
	//		if (!pEntity)
	//			continue;

	//		auto pAI = pEntity->GetAI();
	//		if (!pAI)
	//			continue;

	//		//auto isBusy = gEnv->pAISystem->CheckSmartObjectStates(pEntity, "Busy");
	//		//if (isBusy)
	//		{
	//			if (hideSpot.second > 0)
	//				hideSpot.second -= frametime;
	//			else if (hideSpot.second < 0)
	//				hideSpot.second = 0;

	//			if (hideSpot.second == 0)
	//			{
	//				gEnv->pAISystem->ModifySmartObjectStates(pEntity, "-Busy");
	//				forDelete.push_back(pEntity->GetId());
	//				//CryLogAlways("$6[C++]Hidespot Entity %s is now UNBUSY", pEntity->GetName());
	//			}
	//		}
	//	}

	//	for (auto id : forDelete)
	//	{
	//		auto it = m_bookedHidespots.begin();
	//		auto end = m_bookedHidespots.end();
	//		for (; it != end; it++)
	//		{
	//			if (it->first == id)
	//				m_bookedHidespots.erase(id);
	//		}
	//	}
	//}
}

void CSquadSystem::UpdateHUD()
{
	if (!g_pControlSystem->GetLocalControlClient())
		return;

	if (!g_pGame->GetHUD())
		return;

	UpdatePlayerOrderHUD();

	ShowAllSquadControlsHUD(g_pGameCVars->sqd_HideControls == 0 ? 1 : 0);

	const CControlClient* pCC = g_pControlSystem->GetLocalControlClient();
	if (!pCC)
		return;

	if (pCC->m_pControlledActor == 0)
		ShowAllSquadControlsRedHUD(!m_isCommandMode);
	else
		ShowAllSquadControlsRedHUD(false);

	const auto pPlayer = static_cast<CActor*>(g_pControlSystem->GetClientActor());
	if (pPlayer)
	{
		auto* pSquad = GetSquadFromMember(pPlayer, 1);
		if (!pSquad)
			return;

		for (auto& member : pSquad->GetAllMembers())
		{
			const auto pEntity = gEnv->pEntitySystem->GetEntity(member.GetId());
			if (!pEntity)
				continue;

			SOrderInfo info;
			member.GetOrderInfo(info, false);

			const auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (pMemberActor && pMemberActor != pSquad->GetLeader())
			{
				const float fHealth = (pMemberActor->GetHealth() / (float)pMemberActor->GetMaxHealth()) * 100.0f + 1.0f;
				const int index = pSquad->GetIndexFromMember(pMemberActor);

				const auto pMemberEntity = pMemberActor->GetEntity();
				auto pMemberAI = pMemberActor->GetEntity()->GetAI();
				const auto pMemberVehicle = pMemberActor->GetLinkedVehicle();

				static char buffer[256];
				sprintf(buffer, "%d", index + 1);// in my flash file numeration starts with 1
				static string sOrderName = "";
				const string sMemberNumber = buffer;

				const string sFinalHealFuncName = "setHealthMember" + sMemberNumber;
				const string sFinalOrderFuncName = "setCurrentOrder" + sMemberNumber;
				m_animSquadMembers.CheckedInvoke(sFinalHealFuncName.c_str(), (int)fHealth);

				const auto pSil = SAFE_HUD_FUNC_RET(GetSilhouettes());
				if (pSil)
				{
					if (pSquad->IsMemberSelected(pMemberActor))
						pSil->SetSilhouette(pMemberEntity, 1.0f, 1.0f, 1.0f, 1.0f, -3.0f);
					else
						pSil->ResetSilhouette(pMemberEntity->GetId());
				}

				if (!pMemberVehicle)
				{
					if (pMemberActor->GetHealth() <= 0.f)
						sOrderName = "Dead";
					else if (pMemberAI && pMemberActor->GetHealth() > 0.f)
					{
						if (auto pPU = pMemberAI->CastToIPipeUser())
						{
							if (info.type == eSO_SearchEnemy)
								sOrderName = "Search";
							else if (info.type == eSO_SubEnterVehicle)
								sOrderName = "Enter Vehicle";
							else if (info.type == eSO_SubExitVehicle)
								sOrderName = "Exit Vehicle";
							else if (info.type == eSO_SubPrimaryShootAt)
								sOrderName = "Shoot At";
							else if (info.type == eSO_SubSecondaryShootAt)
								sOrderName = "Shoot At";
							else if (info.type == eSO_SubPrimaryPickupItem || info.type == eSO_SubSecondaryPickupItem)
								sOrderName = "Pickup Item";
							else if (info.type == eSO_SubUseVehicleTurret)
								sOrderName = "Sentry";
							else if (info.type == eSO_SearchCoverAroundPoint || info.type == eSO_ConqSearchCoverAroundArea)
								sOrderName = "Covering";
							else if (info.type == eSO_Guard || info.type == eSO_ConqGoTo)
								sOrderName = "Goto/Guard";
							else if (info.type == eSO_FollowLeader)
								sOrderName = "Follow";
							else
							{
								const auto pProxy = pMemberAI->GetProxy();
								if (pProxy)
								{
									const auto alertness = pProxy->GetAlertnessState();
									if (alertness == 0)
										sOrderName = "Relaxed";
									else if (alertness == 1)
										sOrderName = "Alerted";
									else if (alertness == 2)
										sOrderName = "Combat";
								}
							}
						}
					}
					else
						sOrderName = "No AI";			
				
				}
				else 
				{
					pMemberAI = pMemberVehicle->GetEntity()->GetAI();
					if (!pMemberAI)
						continue;

					const auto pProxy = pMemberAI->GetProxy();
					if (pProxy)
					{
						const auto alertness = pProxy->GetAlertnessState();
						if (alertness == 0)
							sOrderName = "Relaxed";
						else if (alertness == 1)
							sOrderName = "Alerted";
						else if (alertness == 2)
							sOrderName = "Combat";
					}

					const bool isGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);
					const bool isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);
					const bool isPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);

					if(info.type == eSO_SubEnterVehicle)
						sOrderName = "Enter Vehicle";
					else if (info.type == eSO_SubExitVehicle)
						sOrderName = "Exit Vehicle";
					else if (isGunner && !(isDriver || isPassenger))
						sOrderName = "Sentry";
					else if (info.type == eSO_Guard)
						sOrderName = "Go to";
					else if (info.type == eSO_SearchEnemy)
						sOrderName = "Search";
					else if (info.type == eSO_SubPrimaryShootAt || info.type == eSO_SubSecondaryShootAt)
						sOrderName = "Shooting";
				}
				m_animSquadMembers.Invoke(sFinalOrderFuncName.c_str(), sOrderName.c_str());

				const auto mustShow = pMemberActor->GetHealth() > 0 && pMemberVehicle;

				const char* seatType = "passenger";
				if (TOS_Vehicle::ActorIsGunner(pMemberActor))
					seatType = "gunner";
				else if (TOS_Vehicle::ActorIsDriver(pMemberActor))
					seatType = "driver";
				else if (TOS_Vehicle::ActorIsDriver(pMemberActor) && TOS_Vehicle::ActorIsDriver(pMemberActor))
					seatType = "driver";

				ShowVehicleIndicatorHUD(mustShow, index+1, seatType);
			}
		}
	}
}

void CSquadSystem::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network && !gEnv->bEditor)
	{
		auto it = m_allSquads.begin();
		const auto end = m_allSquads.end();
		for (; it != end; it++)
		{
			CSquad* pSquad = *it;

			pSquad->Serialize(ser);
		}

		ser.BeginGroup("tosSquadSystem");
		SER_VALUE(m_showSquadControls);
		SER_VALUE(m_storedMouseId);
		SER_VALUE(m_storedMouseWorldPos);
		ser.EndGroup();
	}
}

std::vector<int> CSquadSystem::GetSquadIdsFromSpecies(ESpeciesType species, bool ignoreBookedVehicle)
{
	TIntVec collected;

	for (CSquad* pSquad : m_allSquads)
	{
		if (pSquad->IsLeaderDetached())
			continue;

		if (!ignoreBookedVehicle)
		{
			const auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(species);

			//FIX: Now a leaders in vehicles will be ignored because it cause engine stop and heli down as rock
			const auto pVehicle = TOS_Vehicle::GetVehicle(pSquad->GetLeader());
			if (pVehicle || (pCommander && pCommander->GetBookedVehicle(pSquad->GetId())))
				continue;
		}

		if (pSquad->GetSpecies() == species)
			collected.push_back(pSquad->GetId());
	}

	return collected;
}

CSquad* CSquadSystem::GetSquadFromMember(const IActor* pActor, bool includeLeader)
{
	if (!pActor)
		return nullptr;

	for (CSquad* pSquad : m_allSquads) 
	{
		if (!includeLeader)
		{
			if (pSquad->IsMember(pActor))
				return pSquad;
		}
		else
		{
			if (pSquad->GetLeader() == pActor)
				return pSquad;

			if (pSquad->IsMember(pActor))
				return pSquad;
		}
	}

	return nullptr;
}

CSquad* CSquadSystem::GetSquadFromLeader(IActor* pLeader)
{
	for (CSquad* pSquad : m_allSquads)
	{
		if (pSquad->GetLeader() == pLeader)
			return pSquad;
	}

	return nullptr;
}

CSquad* CSquadSystem::GetSquadFromId(int squadId) const
{
	//TODO: fix O(n^2)
	for (CSquad* pSquad : m_allSquads)
	{
		if (pSquad->GetId() == squadId)
			return pSquad;
	}
	
	return nullptr;
}

CSquad* CSquadSystem::GetClientSquad() const
{
	for (CSquad* pSquad : m_allSquads)
	{
		if (pSquad->HasClientLeader() || pSquad->HasClientMember())
			return pSquad;
	}

	return nullptr;
}

int CSquadSystem::GetSquadsCount() const
{
	return m_allSquads.size();
}

CSquad* CSquadSystem::CreateSquad()
{
	//bool found = false;
	//for (CSquad* foundSquad : m_allSquads)
	//{
	//	if (foundSquad->GetId() == squad.GetId() && squad.GetId() != -1)
	//	{
	//		found = true;
	//		break;
	//	}
	//}

	//if (!found)
	//{

	CSquadPtr pSquad = new CSquad();
	pSquad->SetId(GetFreeSquadIndex());

	m_allSquads.push_back(pSquad);
	
	if (g_pGameCVars->sqd_debug_log_creating)
		CryLogAlways("[C++][Squad System][Create Squad %i] Success!", pSquad->GetId());

		//return true;
	//}

	return pSquad;
}

bool CSquadSystem::RemoveSquad(uint squadId)
{
	auto it = m_allSquads.cbegin();
	const auto end = m_allSquads.cend();

	for (; it != end; it++)
	{
		const CSquad* pSquad = *it;

		if (pSquad->GetId() == squadId)
		{
			//*it = SSquad();
			m_allSquads.erase(it);

			if (g_pGameCVars->sqd_debug_log_creating)
				CryLogAlways("[Squad System][Remove Squad %i] Success", squadId);
			return true;
		}
	}

	if (g_pGameCVars->sqd_debug_log_creating)
		CryLogAlways("[Squad System][Remove Squad %i] Wasted", squadId);

	return false;

}

//bool CSquadSystem::RemoveSquad(CSquad& squad)
//{
//	TSquads::iterator it = m_allSquads.begin();
//	TSquads::iterator end = m_allSquads.end();
//
//	bool found = false;
//	for (; it != end; it++)
//	{
//		if (it->m_squadId == squad.GetId())
//		{
//			//*it = SSquad();
//			m_allSquads.erase(it);
//
//			if (m_isDebugLog)
//				CryLogAlways("[Squad System][Remove Squad %i] Success", squad.GetId());
//			return true;
//		}
//	}
//
//	if (m_isDebugLog)
//		CryLogAlways("[Squad System][Remove Squad %i] Wasted", squad.GetId());
//
//	return false;
//}

int CSquadSystem::GetFreeSquadIndex() const
{
	//Get the next, FREE index
	return m_allSquads.size() + 1;
}

int CSquadSystem::RequestGroupId(int excludeId, IEntity* pEntity)
{
	return Random(excludeId+ pEntity->GetId() + Random(1, 50), excludeId + Random(1,700) + pEntity->GetId());
}

void CSquadSystem::Reset()
{
	//InitOrderActions(true);

	auto it = m_allSquads.begin();
	const auto end = m_allSquads.end();

	for (; it != end; it++)
	{
		CSquad* pSquad = *it;

		pSquad->Reset();
	}

	m_isDebugLog = false;
	m_isCommandMode = false;
	m_showSquadControls = true;
	
	for (const auto id : m_actionReferences)
	{
		const auto pEntity = GET_ENTITY(id);
		if (!pEntity)
			continue;

		gEnv->pEntitySystem->RemoveEntity(id, true);
	}
}

//void CSquadSystem::InitOrderActions(bool reset)
//{
//	if (reset)
//		m_orderStepActions.clear();
//
//	for (int i = 0; i < ESquadOrders::eSO_Last; i++)
//	{
//		const auto order = ESquadOrders(i);
//		auto it = m_orderStepActions[order].begin();
//
//		const auto step0 = (int)EOrderExecutingStep::NotHaveOrder;
//		const auto step1 = (int)EOrderExecutingStep::GotAnOrder;
//		const auto step2 = (int)EOrderExecutingStep::GotoTarget;
//		const auto step3 = (int)EOrderExecutingStep::PerfomingAction;
//
//		switch (order)
//		{
//		case eSO_Guard:
//			m_orderStepActions[order].insert(it + step0, "");
//			m_orderStepActions[order].insert(it + step1, "");
//			m_orderStepActions[order].insert(it + step2, "");
//			m_orderStepActions[order].insert(it + step3, "");
//			break;
//		case eSO_SearchEnemy:
//			break;
//		case eSO_FollowLeader:
//			break;
//		case eSO_ShootAt:
//			break;
//		case eSO_EnterVehicle:
//			break;
//		case eSO_ExitVehicle:
//			break;
//		case eSO_PickupItem:
//			break;
//		case eSO_UseVehicleTurret:
//			break;
//		case eSO_EnableCombat:
//			break;
//		case eSO_DisableCombat:
//			break;
//		case eSO_StanceRelaxed:
//			break;
//		case eSO_StanceStanding:
//			break;
//		case eSO_StanceStealth:
//			break;
//		case eSO_StanceCrouch:
//			break;
//		case eSO_Conq_Search_Cover_Around_Area:
//			break;
//		case eSO_Conq_GoTo:
//			break;
//		case eSO_Search_Cover_Around_Point:
//			break;
//		case eSO_None:
//			break;
//		}
//
//		auto it = m_orderStepActions[order].begin();
//		const auto last = int(EOrderExecutingStep::Last);
//
//		for (int y = 0; y <= last;y++)
//		{
//			const auto step = EOrderExecutingStep(y);
//			m_orderStepActions[order].insert(it + y, "");
//		}
//	}
//}

void CSquadSystem::OnGameRulesReset()
{
	m_allSquads.clear();
	m_availableMouseOrders.clear();
	m_bookedHidespots.clear();
	Reset();

	//CryLogAlways("[C++][CSquadSystem][OnGameRulesReset]");
}

void CSquadSystem::SpawnOrderParticle(ESquadOrders order, const Vec3& pos)
{
	if (order == eSO_Guard ||
		order == eSO_SubPrimaryShootAt ||
		order == eSO_SubSecondaryShootAt ||
		order == eSO_SearchEnemy ||
		order == eSO_SearchCoverAroundPoint ||
		order == eSO_SubPrimaryPickupItem ||
		order == eSO_SubSecondaryPickupItem)
	{
		const auto pParticle = gEnv->p3DEngine->FindParticleEffect("Squad.Squad_command.Ring");

		if (pParticle)
			pParticle->Spawn(true, IParticleEffect::ParticleLoc(pos));
	}
}

//void CSquadSystem::PlayOrderOffHandAnimation(COrder* pOrder)
//{
//	if (pOrder)
//	{
//
//	}
//}

void CSquadSystem::OnActorDeath(IActor* pActor)
{
	if (!pActor)
		return;

	auto it = m_allSquads.begin();
	const auto end = m_allSquads.end();

	for (; it != end; it++)
	{
		CSquad* pSquad = *it;
		pSquad->OnActorDeath(pActor);
	}

	if (pActor == g_pControlSystem->GetClientActor())
	{
		SetCommandMode(false);
	}

	//CryLogAlways("[C++][CSquadSystem][OnActorDeath %s]", pActor->GetEntity()->GetName());
}

void CSquadSystem::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
	for (CSquad* pSquad : m_allSquads)
		pSquad->OnEnterVehicle(pActor);
}

void CSquadSystem::OnExitVehicle(IActor* pActor)
{
	for (CSquad* pSquad : m_allSquads)
		pSquad->OnExitVehicle(pActor);
}

void CSquadSystem::OnActorGrabbed(IActor* pActor, EntityId grabberId)
{
	for (const CSquadPtr pSquad : m_allSquads)
		pSquad->OnActorDropped(pActor, grabberId);
}

void CSquadSystem::OnActorDropped(IActor* pActor, EntityId droppedId)
{
	for (const CSquadPtr pSquad : m_allSquads)
		pSquad->OnActorDropped(pActor, droppedId);
}

void CSquadSystem::OnActorGrab(IActor* pActor, EntityId grabId)
{

}

void CSquadSystem::OnActorDrop(IActor* pActor, EntityId dropId)
{

}

void CSquadSystem::OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId)
{
	//for (auto& squad : m_allSquads)
	//	squad.OnGoalPipeEvent(pPipeUser, event, goalPipeId);
}

void CSquadSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	
	for (CSquad* pSquad : m_allSquads)
		pSquad->GetMemoryStatistics(s);
}

//Vec3 CSquadSystem::GetNearestHidespot(const Vec3& fromPoint)
//{
//	const int inBusySeconds = 5;
//
//	IEntity* pFirstEntity = nullptr;
//	IEntity* pNearestEntity = nullptr;
//	auto minDist = 0;
//
//	AutoAIObjectIter it(gEnv->pAISystem->GetFirstAIObject(IAISystem::OBJFILTER_TYPE, 320));
//	for (; it->GetObject(); it->Next())
//	{
//		auto pAI = it->GetObject();
//		if (!pAI)
//			continue;
//
//		auto pEnt = pAI->GetEntity();
//		if (!pEnt)
//			continue;
//
//		if (pAI->GetAIType() != 320) // COMBAT_HIDESPOT
//			continue;
//
//		//const auto isBusy = gEnv->pAISystem->CheckSmartObjectStates(pEnt, "Busy");
//		//if (isBusy || inRecently)
//		const auto inRecently = stl::find_in_map(m_bookedHidespots, pEnt->GetId(), false);
//		if (inRecently)
//			continue;
//
//		const auto& pos = pEnt->GetWorldPos();
//		const auto dist = (pos - fromPoint).GetLength();
//
//		if (minDist == 0)
//		{
//			minDist = dist;
//			pFirstEntity = pEnt;
//		}
//		else
//		{
//			if (dist < minDist)
//			{
//				minDist = dist;
//				pNearestEntity = pEnt;
//			}				
//		}
//
//		if (pNearestEntity == nullptr)
//			pNearestEntity = pFirstEntity;
//	}
//
//	if (pNearestEntity)
//	{
//		//gEnv->pAISystem->ModifySmartObjectStates(pNearestEntity, "Busy");
//		m_bookedHidespots.insert(std::make_pair(pNearestEntity->GetId(), inBusySeconds));
//		//CryLogAlways("$6[C++]Hidespot Entity %s is now BUSY", pNearestEntity->GetName());
//
//		return pNearestEntity->GetWorldPos();
//	}
//
//	return Vec3(0, 0, 0);
//}

EntityId CSquadSystem::GetNearestFreeHidespot(const Vec3& fromPoint, const float radius)
{
	const int inBusySeconds = 5;

	IEntity* pFirstEntity = nullptr;
	const IEntity* pNearestEntity = nullptr;
	auto minDist = 0;

	// 320 - COMBAT_HIDESPOT
	AutoAIObjectIter it(gEnv->pAISystem->GetFirstAIObjectInRange(IAISystem::OBJFILTER_TYPE, 320, fromPoint, radius, false));
	for (; it->GetObject(); it->Next())
	{
		const auto pAI = it->GetObject();
		if (!pAI)
			continue;

		const auto pEnt = pAI->GetEntity();
		if (!pEnt)
			continue;

		if (IsBookedHideSpot(pEnt->GetId()))
			continue;

		const auto pos = pEnt->GetWorldPos();
		const auto dist = (pos - fromPoint).GetLength();

		if (minDist == 0)
		{
			minDist = dist;
			pFirstEntity = pEnt;
		}
		else
		{
			if (dist < minDist)
			{
				minDist = dist;
				pNearestEntity = pEnt;
			}
		}

		if (pNearestEntity == nullptr)
			pNearestEntity = pFirstEntity;
	}

	if (pNearestEntity)
		return pNearestEntity->GetId();

	return 0;
}

string CSquadSystem::GetString(ESquadOrders orderType) const
{
	return stl::find_in_map(m_ordersStringMap, orderType, "NullOrderString");
}

void CSquadSystem::ClientApplyExecution(CMember* pMember, SOrderInfo& order, EExecuteOrderFlag executionFlag, EntityId targetId /*= 0*/, Vec3 targetPos /*= Vec3(0)*/, bool safeFly /*= false*/)
{
	if (!pMember)
		return;

	const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->GetId()));
	if (!pActor)
		return;

	const auto pSquad = GetSquadFromMember(pActor, false);
	if (!pSquad)
		return;

	const auto pLeader = pSquad->GetLeader();
	if (!pLeader)
		return;

	const auto isAlien = pActor->IsAlien();
	const string memClass = pActor->GetEntity()->GetClass()->GetName();
	const auto pMemberVehicle = TOS_Vehicle::GetVehicle(pActor);

	//Define AI Actions per one order step
	DEFINE_STEPS;

	//Define order ignore combat flags
	uint ignoreFlag = 0;

	if (!pMemberVehicle)
	{
		switch (order.type)
		{
		case eSO_Guard:
		case eSO_FollowLeader:
			ignoreFlag |= eOICF_IgnoreCombatWhenGotoTarget;
			break;
		case eSO_SubPrimaryShootAt:
		case eSO_SubSecondaryShootAt:
		case eSO_SubEnterVehicle:
		case eSO_SubExitVehicle:
		case eSO_SubPrimaryPickupItem:
		case eSO_SubSecondaryPickupItem:
		case eSO_SubUseVehicleTurret:
		//case eSO_SubAlienGrabPlayerSquad:
		//case eSO_SubAlienDropPlayerSquad:
			ignoreFlag |= eOICF_IgnoreEnemyAlways;
			break;
		case eSO_SearchEnemy:
		case eSO_DebugEnableCombat:
		case eSO_DebugDisableCombat:
		case eSO_DebugStanceRelaxed:
		case eSO_DebugStanceStanding:
		case eSO_DebugStanceStealth:
		case eSO_DebugStanceCrouch:
		case eSO_ConqSearchCoverAroundArea:
		case eSO_ConqGoTo:
		case eSO_SearchCoverAroundPoint:
		case eSO_None:
		case eSO_Last:
			ignoreFlag = 0;
			break;
		}

		if (isAlien)
		{
			APPLY_ALIEN_EXECUTION(order);
		}
		else
		{
			APPLY_HUMAN_EXECUTION(order);
		}
	}
	else
	{
		APPLY_VEHICLE_EXECUTION(order);
	}

	order.ignoreFlag = ignoreFlag;
	order.safeFly = safeFly;

	if (!GET_ENTITY(order.targetId))
		order.targetId = m_storedMouseId;

	if (order.targetPos == Vec3(0))
		order.targetPos = m_storedMouseWorldPos;

	if (GET_ENTITY(targetId))
		order.targetId = targetId;

	if (targetPos != Vec3(0))
		order.targetPos = targetPos;

	pSquad->ExecuteOrder(pMember, order, executionFlag);
}

IEntity* CSquadSystem::CreateActionReference(CMember* pMember, Vec3 position)
{
	if (!pMember)
		return nullptr;

	const auto pMemberEntity = GET_ENTITY(pMember->GetId());
	if (!pMemberEntity)
		return nullptr;

	SEntitySpawnParams params;
	params.bStaticEntityId = true;
	params.vPosition = position;
	params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	params.sName = string(pMemberEntity->GetName()) + string("_action_reference");

	const auto pEntity = gEnv->pEntitySystem->SpawnEntity(params, true);
	pMember->SetActionRef(pEntity);

	//m_actionReferences.push_back(pEntity->GetId());

	return pEntity;
}

EntityId CSquadSystem::BookFreeHideSpot(IActor* pActor, Vec3 pos, float radius, const char* cause)
{
	if (!pActor)
		return 0;

	if (!CanBookHideSpot(pActor))
		return 0;

	const auto actId = pActor->GetEntityId();
	const auto hidespotId = GetNearestFreeHidespot(pos, radius);

	const auto pHidespotEntity = GET_ENTITY(hidespotId);
	if (!pHidespotEntity)
		return 0;

	m_bookedHidespots[hidespotId] = actId;

	if (g_pGameCVars->sqd_debug_log_hidespots)
	{
		CryLogAlways("%s[C++][Book hidespot: %s][Actor: %s][Cause: %s]",
			STR_YELLOW, pHidespotEntity->GetName(), pActor->GetEntity()->GetName(), cause);
	}

	return pHidespotEntity->GetId();
}

EntityId CSquadSystem::GetBookedHideSpot(IActor* pActor)
{
	if (!pActor)
		return 0;

	for (const auto& spotPair : m_bookedHidespots)
	{
		if (spotPair.second == pActor->GetEntityId())
			return spotPair.first;
	}

	return 0;
}

bool CSquadSystem::IsBookedHideSpot(EntityId spotId)
{
	for (const auto& spotPair : m_bookedHidespots)
	{
		if (spotPair.first == spotId)
			return true;
	}

	return false;
}

bool CSquadSystem::UnbookHideSpot(IActor* pActor, const char* cause)
{
	if (!pActor)
		return false;

	for (const auto& spotPair : m_bookedHidespots)
	{
		if (spotPair.second != pActor->GetEntityId())
			continue;

		m_bookedHidespots.erase(spotPair.first);

		if (g_pGameCVars->sqd_debug_log_hidespots)
		{
			CryLogAlways("%s[C++][UnBook hidespot][Actor: %s][Cause: %s]",
				STR_YELLOW, pActor->GetEntity()->GetName(), cause);
		}

		return true;
	}

	if (g_pGameCVars->sqd_debug_log_hidespots)
	{
		CryLogAlways("%s[C++][UnBook hidespot][Actor: %s][FAILED][Cause1: already unbooked][Cause2: %s]",
			STR_RED, pActor->GetEntity()->GetName(), cause);
	}
	return false;
}

bool CSquadSystem::CanBookHideSpot(IActor* pActor)
{
	if (!pActor)
		return false;

	if (pActor->GetHealth() <= 0.0f)
		return false;

	if (TOS_Vehicle::ActorInVehicle(pActor))
		return false;

	return true;
}

void CSquadSystem::SetCommandMode(bool value)
{
	const bool hasChanged = (value != m_isCommandMode);
	if (!hasChanged)
		return;

	m_storedMouseId = 0;
	m_storedMouseWorldPos = Vec3(0, 0, 0);

	if (m_animSquadMouseOrders.IsLoaded())
	{
		if (!value)
			m_animSquadMouseOrders.SetVisible(false);

		if (g_pGame->GetHUD())
		{
			const auto pAnim = value ? &m_animSquadMouseOrders : nullptr;
			g_pGame->GetHUD()->SwitchToModalHUD(pAnim,true);
		}
	}

	if (gEnv->pSoundSystem)
	{
		if (!m_pSoundCommandBegin)
			m_pSoundCommandBegin = gEnv->pSoundSystem->CreateSound("Sounds/interface/hud/slowmotion_activate_01.mp3", FLAG_SOUND_2D | FLAG_SOUND_RELATIVE | FLAG_SOUND_16BITS | FLAG_SOUND_LOAD_SYNCHRONOUSLY);

		if (!m_pSoundCommandEnd)
			m_pSoundCommandEnd = gEnv->pSoundSystem->CreateSound("Sounds/interface/hud/slowmotion_deactivate_01.mp3", FLAG_SOUND_2D | FLAG_SOUND_RELATIVE | FLAG_SOUND_16BITS | FLAG_SOUND_LOAD_SYNCHRONOUSLY);

		if (value)
		{
			gEnv->pSoundSystem->SetSoundActiveState(m_pSoundCommandBegin, eSoundState_None);

			if (m_pSoundCommandEnd)
				m_pSoundCommandEnd->Stop();
			if (m_pSoundCommandBegin)
				m_pSoundCommandBegin->Play();

			gEnv->pSoundSystem->SetMasterPitch(-0.5f);
			gEnv->pTimer->SetTimeScale(0.25f);
		}
		else
		{
			gEnv->pSoundSystem->SetSoundActiveState(m_pSoundCommandEnd, eSoundState_None);
			if (m_pSoundCommandEnd)
				m_pSoundCommandEnd->Play();
			if (m_pSoundCommandBegin)
				m_pSoundCommandBegin->Stop();

			gEnv->pSoundSystem->SetMasterPitch(0.0f);
			gEnv->pTimer->SetTimeScale(1.0);
		}
	}

	m_isCommandMode = value;
}

//void CSquadSystem::GetNearestHideSpots(const Vec3& fromPoint, const float radius, std::vector<EntityId>& spots)
//{
//	IEntity* pFirstEntity = nullptr;
//	IEntity* pNearestEntity = nullptr;
//	auto minDist = 0;
//
//	AutoAIObjectIter it(gEnv->pAISystem->GetFirstAIObjectInRange(IAISystem::OBJFILTER_TYPE, 320, fromPoint, radius, false));
//	for (; it->GetObject(); it->Next())
//	{
//		auto pAI = it->GetObject();
//		if (!pAI)
//			continue;
//
//		auto pEnt = pAI->GetEntity();
//		if (!pEnt)
//			continue;
//
//		if (pAI->GetAIType() != 320) // COMBAT_HIDESPOT
//			continue;
//
//		const auto isBooked = IsBookedHideSpot(pEnt->GetId());
//		if (isBooked)
//			continue;
//
//		const auto pos = pEnt->GetWorldPos();
//		const auto dist = (pos - fromPoint).GetLength();
//
//		if (minDist == 0)
//		{
//			minDist = dist;
//			pFirstEntity = pEnt;
//		}
//		else
//		{
//			if (dist < minDist)
//			{
//				minDist = dist;
//				pNearestEntity = pEnt;
//			}
//		}
//
//		if (pNearestEntity == nullptr)
//			pNearestEntity = pFirstEntity;
//	}
//
//	if (pNearestEntity)
//	{
//		//gEnv->pAISystem->ModifySmartObjectStates(pNearestEntity, "Busy");
//		m_bookedHidespots[pNearestEntity->GetId()] = inBusySeconds;
//		return pNearestEntity->GetWorldPos();
//	}
//}

void CSquadSystem::ShowAllSquadControlsRedHUD(bool active)
{
	if (!m_animSquadMembers.IsLoaded() && !m_showSquadControls)
		return;

	m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.gotoAndStop", active ? "Red" : "Show");
}

void CSquadSystem::SetMouseOrdersPosHUD(int x, int y)
{
	int x0, y0, width, heigth = 0;
	float ratio = 0;

	if (m_isDebugLog)
		CryLogAlways("[CSquadSystem::SetMouseOrdersPosHUD] x:%i, y:%i",x,y);

	m_animSquadMouseOrders.GetFlashPlayer()->GetViewport(x0, y0, width, heigth, ratio);
	m_animSquadMouseOrders.GetFlashPlayer()->SetViewport(x, y, width, heigth, ratio);
}

void CSquadSystem::ShowMouseOrderHUD(int index, bool show)
{
	const string showing = show ? "Show" : "Hide";

	const SFlashVarValue args[2] = { index, showing.c_str() };
	m_animSquadMouseOrders.Invoke("setButtonVisible", args, 2);
}

void CSquadSystem::ShowAllSquadControlsHUD(bool active)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	m_showSquadControls = active;

	m_animSquadMembers.CheckedInvoke("_root.Root.controlsText.gotoAndStop", m_showSquadControls ? "Show" : "Hide");
}

void CSquadSystem::InitHUD(bool show)
{
	if (show)
	{
		m_animSquadMembers.Load("Libs/UI/HUD_SquadMembers.swf", eFD_Left);
		m_animSquadMembers.SetVisible(false);

		m_animSquadMouseOrders.Load("Libs/UI/HUD_SquadMouseOrders.swf", eFD_Left);
		m_animSquadMouseOrders.SetVisible(false);
	}
	else
	{
		m_animSquadMembers.Unload();
		m_animSquadMouseOrders.Unload();
	}
}

void CSquadSystem::AnySquadClientLeft()
{
	CSquad nullSquad;
	nullSquad.OnPlayerRemoved();

	ClearAvailableOrders();
}

bool CSquadSystem::AnySquadActorIsLeader(const IActor* pActor) const noexcept
{
	for (const CSquad* pSquad : m_allSquads)
	{
		if (pSquad->IsLeader(pActor))
			return true;	
	}

	return false;
}

bool CSquadSystem::AnySquadActorIsMember(IActor* pActor) const noexcept
{
	for (const CSquad* pSquad : m_allSquads)
	{
		if (pSquad->IsMember(pActor))
			return true;
	}

	return false;
}

void CSquadSystem::ShowSquadMemberHUD(const bool active, const int slot)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	//CryLogAlways("ShowSquadMember active = %d, slot = %d", active, slot);

	ShowSquadControlHUD(slot, active);

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

void CSquadSystem::ShowDeadSquadMemberHUD(int slot)
{
	if (!m_animSquadMembers.IsLoaded())
		return;

	ShowSquadControlHUD(slot, false);

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

void CSquadSystem::ShowVehicleIndicatorHUD(bool show, int slot, const char* seatType)
{
	if (m_animSquadMembers.IsLoaded())
	{
		const SFlashVarValue args[3] = { (int)show, slot, seatType};

		m_animSquadMembers.Invoke("showVehicleIndicator", args, 3);
	}
}

void CSquadSystem::ShowSquadControlHUD(int index, bool show)
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

void CSquadSystem::UpdateSelectedHUD()
{
	auto* pSquad = GetClientSquad();
	if (pSquad && pSquad->GetLeader() != 0 && pSquad->GetMembersCount() > 0)
	{
		if (m_animSquadMembers.IsLoaded())
		{
			m_animSquadMembers.Invoke("setSelectedMember1", pSquad->IsMemberSelected(0) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember2", pSquad->IsMemberSelected(1) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember3", pSquad->IsMemberSelected(2) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember4", pSquad->IsMemberSelected(3) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember5", pSquad->IsMemberSelected(4) ? 1 : 0);
			m_animSquadMembers.Invoke("setSelectedMember6", pSquad->IsMemberSelected(5) ? 1 : 0);
		}
	}
}
void CSquadSystem::UpdatePlayerOrderHUD()
{
	//CActor* pPlayer = nullptr;

	//if (g_pControlSystem->GetLocalControlClient())
	//{
	//	pPlayer = static_cast<CActor*>(g_pControlSystem->GetLocalControlClient()->GetControlledActor());
	//	if (!pPlayer)
	//		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	//}

	//auto* pSquad = GetSquadFromMember(pPlayer, 1);
	//if (pSquad && (pSquad->HasClientMember() || pSquad->HasClientLeader()) && m_animSquadMembers.IsLoaded())
	//{
	//	string sPlayerOrder = "";
	//	switch (pSquad->m_eLeaderCurrentOrder)
	//	{
	//	case eSO_Guard:
	//		sPlayerOrder = "Go to";
	//		break;
	//	case eSO_SearchEnemy:
	//		sPlayerOrder = "Search";
	//		break;
	//	case eSO_FollowLeader:
	//		sPlayerOrder = "Follow";
	//		break;
	//	case eSO_ShootAt:
	//		sPlayerOrder = "Shoot At";
	//		break;
	//	}

	//	m_animSquadMembers.Invoke("setPlayerCurrentOrder", sPlayerOrder.c_str());
	//}
}

void CSquadSystem::UpdateMouseOrdersHUD(bool updateMousePos /*= true*/)
{
	const auto pSquad = GetClientSquad();
	if (pSquad && pSquad->HasClientLeader() && m_isCommandMode)
	{
		if (updateMousePos)
		{
			const auto mousePos = g_pControlSystem->GetMouseScreenPos();
			SetMouseOrdersPosHUD(mousePos.x, mousePos.y);
		}

		if (!m_animSquadMouseOrders.GetVisible())
			m_animSquadMouseOrders.SetVisible(true);

		//Clearing Buttons and Orders when press mouse
		ClearAvailableOrders();
		//m_storedMouseId = 0;
		m_storedMouseId = g_pControlSystem->GetMouseEntityID();
		m_storedMouseWorldPos = Vec3(0, 0, 0);

		const auto selectedMemCount = pSquad->m_selectedMembers.size();

		const auto pLeader = dynamic_cast<CActor*>(pSquad->GetLeader());
		if (pLeader && pLeader->IsAlien() && !pLeader->GetGrabStats()->isGrabbed)
		{
			const auto pMouseEntity = gEnv->pEntitySystem->GetEntity(m_storedMouseId);
			if (pMouseEntity && strcmp(pMouseEntity->GetClass()->GetName(), "Scout") == 0)
			{
				const auto pScout = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_storedMouseId));
				
				if (pScout->GetGrabStats()->grabbedIds.size() == 0 && strcmp(pLeader->GetEntity()->GetClass()->GetName(), "Scout") != 0)
				{
					AddAvailableOrder(eSO_ScoutGrabMe);
				}

				return;
			}
		}

		for (const auto id : pSquad->m_selectedMembers)
		{
			const auto* pMember = pSquad->GetMemberInstance(id);
			if (pMember)
			{
				SOrderInfo order;
				pMember->GetOrderInfo(order, false);

				const auto pMemberActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->GetId()));
				if (pMemberActor && pMemberActor->GetHealth() > 0)
				{
					auto noVehicleCount = 0;
					auto inVehicleCount = 0;
					auto isAlienCount = 0;
					auto gunnerCount = 0;
					auto passengerCount = 0;
					auto driverCount = 0;
					auto withWeaponCount = 0;

					auto pLeaderVehicle = TOS_Vehicle::GetVehicle(pSquad->GetLeader());
					const auto pMemberVehicle = TOS_Vehicle::GetVehicle(pMemberActor);
					const auto isAlien = pMemberActor->IsAlien();

					const auto isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);
					const auto isGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);
					const auto isPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);
					const auto isHaveWeapons = TOS_Inventory::IsHaveWeapons(pMemberActor);
					auto processedIsHostile = false;

					if (isGunner)
						gunnerCount++;
					else if (isDriver)
						driverCount++;
					else if (isPassenger)
						passengerCount++;
					else if (isHaveWeapons)
						withWeaponCount++;

					if (pMemberVehicle)
						inVehicleCount++;
					else
						noVehicleCount++;

					if (isAlien)
						isAlienCount++;

					m_storedMouseId = g_pControlSystem->GetMouseEntityID();

					const auto pMouseEntity = gEnv->pEntitySystem->GetEntity(m_storedMouseId);

					if (!pMouseEntity)
					{
						AddAvailableOrder(eSO_Guard);
						AddAvailableOrder(eSO_SearchEnemy);

						if (noVehicleCount > 0)
							AddAvailableOrder(eSO_SearchCoverAroundPoint);
					}

					auto pMouseActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_storedMouseId);
					const auto pMouseVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_storedMouseId);
					const auto pMouseItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_storedMouseId);

					if (pMouseEntity)
					{
						if (pMouseEntity && pMouseEntity->GetAI())
							processedIsHostile = TOS_AI::IsHostile(pMouseEntity->GetAI(), pMemberActor->GetEntity()->GetAI(), false);

						if (!isAlien)
						{
							if (pMouseVehicle && !pMouseVehicle->IsDestroyed() && !processedIsHostile)
							{
								if (!pMemberVehicle)
									AddAvailableOrder(eSO_SubEnterVehicle);

								const auto gunnerIndex = TOS_Vehicle::RequestGunnerSeatIndex(pMouseVehicle);
								const auto pGunnerSeat = pMouseVehicle->GetSeatById(gunnerIndex);

								if (pGunnerSeat && !pGunnerSeat->GetPassenger() && (!pMemberVehicle || (pMemberVehicle && pMemberVehicle->GetEntity() == pMouseEntity)))
								{
									if (selectedMemCount > 1 && gunnerCount != selectedMemCount)
									{
										AddAvailableOrder(eSO_SubUseVehicleTurret);
									}
									else if (selectedMemCount == 1)
									{
										if (!isGunner)
											AddAvailableOrder(eSO_SubUseVehicleTurret);
									}
								}
							}

							if (pMouseItem)
							{
								if (!pMemberVehicle)
								{
									AddAvailableOrder(eSO_SubPrimaryPickupItem);
									AddAvailableOrder(eSO_SubSecondaryPickupItem);
								}
							}
						}
					}

					const bool isHaveSameVehicle = pMemberVehicle && pMemberVehicle->GetEntity() == pMouseEntity;

					if (!processedIsHostile)
					{
						if ((!isAlien && inVehicleCount != 0) || isHaveSameVehicle)
							AddAvailableOrder(eSO_SubExitVehicle);
					}

					if (!pMouseEntity && order.type != eSO_FollowLeader)
						AddAvailableOrder(eSO_FollowLeader);

					AddAvailableOrder(eSO_SubPrimaryShootAt);
					AddAvailableOrder(eSO_SubSecondaryShootAt);

					//AddAvailableOrder(eSO_StanceStanding);
					//AddAvailableOrder(eSO_StanceRelaxed);
					//AddAvailableOrder(eSO_EnableCombat);
					//AddAvailableOrder(eSO_DisableCombat);
				}
			}
		}
	}
}

void CSquadSystem::AddAvailableOrder(ESquadOrders orderToAdd)
{
	for (const auto order : m_availableMouseOrders)
	{
		if (order == orderToAdd)
			return;
	}
	m_availableMouseOrders.push_back(orderToAdd);

	const int buttonIdx = m_availableMouseOrders.size() - 1;

	SFlashVarValue args[2] = { buttonIdx,"Show"};
	m_animSquadMouseOrders.Invoke("setButtonVisible", args, 2);

	//Processed in function SSquad::HandleMouseFSCommand
	const string order = m_ordersStringMap[orderToAdd];

	args[1] = order.c_str();
	m_animSquadMouseOrders.Invoke("setButtonOrder", args, 2);

	const string localizedOrder = '@' + order;
	args[1] = localizedOrder.c_str();
	m_animSquadMouseOrders.Invoke("setButtonLocalizedText", args, 2);
}

void CSquadSystem::RemoveAvailableOrder(ESquadOrders orderToRemove)
{
	auto it = m_availableMouseOrders.begin();
	const auto end = m_availableMouseOrders.end();

	auto isFounded = false;

	for (; it != end; it++)
	{
		if (*it == orderToRemove)
		{
			isFounded = true;
			break;
		}
	}

	if (isFounded)
	{
		m_availableMouseOrders.erase(it);

		const int index = it - m_availableMouseOrders.begin();

		const SFlashVarValue args[2] = { index,"Hide" };
		m_animSquadMouseOrders.Invoke("setButtonVisible", args, 2);
	}
}

void CSquadSystem::ClearAvailableOrders()
{
	m_availableMouseOrders.clear();

	//Hide all order buttons
	for (auto i = 0; i < 20; i++)
		ShowMouseOrderHUD(i, false);
}

//void CSquad::ApplyAIAction(const CMember* pMember, IEntity* pObject, IVehicle* pMemberVehicle, ESquadOrders type, EOrderExecutingStep step, string cause)
//{
//	if (!pMember)
//		return;
//
//	if (!pObject)
//		return;
//
//	auto pEntity = GET_ENTITY(pMember->GetId());
//	if (!pEntity)
//		return;
//
//	auto pUser = pEntity->GetAI();
//	if (!pUser)
//		return;
//
//
//	switch (type)
//	{
//	case eSO_Guard:
//	{
//		if (step == eES_GotoTarget)
//		{
//			actionName = "conqueror_goto_a0_d0_r3";
//			flag = eAAEF_DisableCombatDuringAction;
//
//			if (!TOS_AI::IsExecuting(pUser, actionName))
//				TOS_AI::ExecuteAIAction(pUser, pObject, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
//		}
//		else if (step == eES_PerfomingAction)
//		{
//			//AI Action not implemented
//			//squad_guard_location
//
//			//actionName = "conqueror_goto_a0_d0_r3";
//			//flag = eAAEF_DisableCombatDuringAction;
//
//			//if (!TOS_AI::IsExecuting(pUser, actionName))
//			//	TOS_AI::ExecuteAIAction(pUser, pObject, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
//		}
//	}
//		break;
//	case eSO_SearchEnemy:
//		break;
//	case eSO_FollowLeader:
//	{
//		if (step == eES_GotoTarget)
//		{
//			actionName = "conqueror_goto_a0_d0_r3";
//			flag = eAAEF_DisableCombatDuringAction;
//
//			if (!TOS_AI::IsExecuting(pUser, actionName))
//				TOS_AI::ExecuteAIAction(pUser, pObject, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
//		}
//		else if (step == eES_PerfomingAction)
//		{
//			//Action "squad_follow_leader" not implemented
//			// 
//			actionName = "squad_follow_leader";
//			flag = eAAEF_PausingActionWhenCombat;
//
//			if (!TOS_AI::IsExecuting(pUser, actionName))
//				TOS_AI::ExecuteAIAction(pUser, pObject, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
//		}
//	}
//		break;
//	case eSO_ShootAt:
//		break;
//	case eSO_EnterVehicle:
//		break;
//	case eSO_ExitVehicle:
//		break;
//	case eSO_PickupItem:
//		break;
//	case eSO_UseVehicleTurret:
//		break;
//	case eSO_EnableCombat:
//		break;
//	case eSO_DisableCombat:
//		break;
//	case eSO_StanceRelaxed:
//		break;
//	case eSO_StanceStanding:
//		break;
//	case eSO_StanceStealth:
//		break;
//	case eSO_StanceCrouch:
//		break;
//	case eSO_Conq_Search_Cover_Around_Area:
//		break;
//	case eSO_Conq_GoTo:
//		break;
//	case eSO_Search_Cover_Around_Point:
//		break;
//	case eSO_None:
//		break;
//	}
//}

void CMember::SetFailedOrderInfo(const SOrderInfo& info)
{
	//m_failedOrderInfo = info;
}

void CMember::GetFailedOrderInfo(SOrderInfo& info)
{
	//info.ignoreFlag = m_failedOrderInfo.ignoreFlag;
	//info.safeFly = m_failedOrderInfo.safeFly;
	//info.stepActions = m_failedOrderInfo.stepActions;
	//info.targetId = m_failedOrderInfo.targetId;
	//info.targetPos = m_failedOrderInfo.targetPos;
	//info.targetRadius = m_failedOrderInfo.targetRadius;
	//info.type = m_failedOrderInfo.type;
}
