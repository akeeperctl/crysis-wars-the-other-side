#include "StdAfx.h"

#include "Actor.h"
#include "Scout.h"
#include "Player.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Conqueror/StrategicArea.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Conqueror/ConquerorChannel.h"
#include "TheOtherSide/Conqueror/ConquerorCommander.h"
#include "TheOtherSide/Conqueror/RequestsAndResponses/RARSystem.h"

#include "TheOtherSide/Helpers/TOS_AI.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "TheOtherSide/Helpers/TOS_Inventory.h"
#include "TheOtherSide/Helpers/TOS_Debug.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"

#include "Single.h"

//Air squad mate defines
constexpr auto SAFE_FLY_HEIGHT = 15.f;
constexpr auto SAFE_FLY_DISTANCE_HUNTER = 25.f;

//Search enemy radius
constexpr auto AIR_SEARCH_RADIUS = 150.0f;
constexpr auto LAND_SEARCH_RADIUS = 75.0f;
constexpr auto FOOT_SEARCH_RADIUS = 40.0f;

//AI defines
constexpr auto AI_CLEAN_START_3SEC = 300; //300 = 3 seconds;
constexpr auto AI_CLEAN_START_HALFSEC = 50; //50 = 0.5 second;
constexpr auto AI_CLEAN_FINISH = 0;

CSquad::CSquad()
{
	Init();
};

CSquad::CSquad(IActor* _Leader, uint _squadId)
{
	Init();

	m_leaderId = _Leader ? _Leader->GetEntityId() : 0;
	//m_leaderInstance = CLeaderInstance(_Leader);
	m_squadId = _squadId;
};

CSquad::~CSquad()
{
	Shutdown();
};

void CSquad::Init()
{
	m_lastConqIterationTime = 0;
	//m_lastMembersUpdateTime = 0;
	//m_eLeaderCurrentOrder = eSO_Guard;
	m_listeners.clear();
	m_members.clear();
	m_selectedMembers.clear();
	m_detachedMembers.clear();
	m_searchRadius = 20;
	//m_leaderInstance = CLeaderInstance();
	m_leaderId = 0;
	m_pConquerorOldAILeader = 0;
	m_conquerorSpecies = eST_NEUTRAL;
	m_squadId = -1;
	m_flags = 0;
	m_lastAllOrder = eSO_None;
	m_pSquadSystem = g_pControlSystem->GetSquadSystem();
	//m_pCommander = nullptr;
	m_hided = false;
	m_detachedLeadersData.clear();
	m_refs = 0;

	m_inCombat = false;
	m_lastTimeStartCombat = 0;
	m_lastTimeFinishCombat = 0;
}

void CSquad::Shutdown()
{
	m_listeners.clear();

	m_members.clear();
	m_selectedMembers.clear();
	m_detachedMembers.clear();
	m_searchRadius = 20;
	//m_leaderInstance = CLeaderInstance();
	m_leaderId = 0;
	m_pConquerorOldAILeader = 0;
	m_conquerorSpecies = eST_NEUTRAL;
	m_squadId = -1;
	m_flags = 0;
	m_lastAllOrder = eSO_None;
	m_pSquadSystem = g_pControlSystem->GetSquadSystem();
	//m_pCommander = nullptr;
	m_hided = false;
	//m_detachedLeadersData.clear();

	m_inCombat = false;
	m_lastTimeStartCombat = 0;
	m_lastTimeFinishCombat = 0;
}

void CSquad::AddMemberToSelected(const CMember* pMember)
{
	CActor* pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	//SMember member = GetMemberFromIndex(index);
	if (!pMember || !GetActor(pMember->m_entityId))
		return;

	//We cannot select the player
	if (GetActor(pMember->m_entityId) == pPlayer)
		return;

	bool finded = false;
	for (auto& selectedId : m_selectedMembers)
	{
		if (selectedId == pMember->m_entityId)
		{
			finded = true;
			break;
		}
	}

	if (!finded)
	{
		m_selectedMembers.push_back(pMember->m_entityId);

		if (m_pSquadSystem->m_isDebugLog)
			CryLogAlways("[C++][SSquad::AddMemberToSelected][%id]", pMember->m_entityId);
	}
}

bool CSquad::HasClientMember() const
{
	CActor* pPlayer = nullptr;

	if (g_pControlSystem->GetLocalControlClient())
	{
		pPlayer = static_cast<CActor*>(g_pControlSystem->GetLocalControlClient()->GetControlledActor());
		if (!pPlayer)
			pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	}

	if (pPlayer)
	{
		for (auto& member : m_members)
		{
			if (pPlayer->GetEntityId() == member.m_entityId)
				return true;
		}
	}

	return false;
}

bool CSquad::HasClientLeader() const
{
	CActor* pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());	

	if (g_pControlSystem->GetLocalControlClient() && g_pControlSystem->GetLocalControlClient()->GetControlledActor())
		pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();

	if (pPlayer)
	{
		if (pPlayer == GetLeader())
			return true;
	}

	return false;
}

void CSquad::OnPlayerAdded()
{
	m_pSquadSystem->m_animSquadMembers.Reload(true);
	m_pSquadSystem->m_animSquadMembers.SetVisible(true);

	UpdateMembersHUD();
}

void CSquad::OnPlayerRemoved()
{
	for (auto& member : m_members)
		m_pSquadSystem->ShowSquadMemberHUD(false, GetIndexFromMember(member));

	m_pSquadSystem->m_animSquadMembers.SetVisible(false);
}

void CSquad::UpdateOrdersNew(float frametime)
{
	const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	if (!GetLeader())
		return;

	static bool bDebugDraw = false;
	static float color[] = { 1,1,1,1 };

	for (auto& memberInstance : m_members)
	{
		auto pMemberActor = static_cast<CActor*>(GetActor(memberInstance.m_entityId));
		if (!pMemberActor)
			continue;

		auto pMemberVehicle = TOS_Vehicle::GetVehicle(pMemberActor);

		SOrderInfo orderInfo;
		SOrderInfo subOrderInfo;
		SOrderInfo prevOrderInfo;
		memberInstance.GetOrderInfo(orderInfo, false);
		memberInstance.GetSubOrderInfo(subOrderInfo);
		memberInstance.GetOrderInfo(prevOrderInfo, true);

		if (g_pGameCVars->sqd_debug_draw_client_squad > 0 && (HasClientLeader() || HasClientMember()))
		{
			static float color[] = { 1,1,1,1 };
			const auto size = TOS_Debug::SIZE_COMMON;
			const auto scale = 20;
			const auto xoffset = TOS_Debug::XOFFSET_COMMON;
			const auto yoffset = TOS_Debug::YOFFSET_SQUAD;

			const auto orderName = m_pSquadSystem->GetString(orderInfo.type);
			const auto subOrderName = m_pSquadSystem->GetString(subOrderInfo.type);
			const auto previousOrderName = m_pSquadSystem->GetString(prevOrderInfo.type);

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, TOS_Debug::SIZE_HEADER, color, false,
				"Player squad: ");

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + memberInstance.GetIndex() * scale, size, color, false,
				"Member: %i, Can be Updated: %i, Step: %i, Safe Fly: %i, Order Object: %i, Order: %s, Previous Order: %s, SubOrder Object: %i, SubOrder: %s",
				memberInstance.GetIndex(), memberInstance.CanBeUpdated(), memberInstance.GetMainStep(), orderInfo.safeFly, orderInfo.targetId, orderName, previousOrderName, subOrderInfo.targetId, subOrderName);
		
			auto pRef = memberInstance.GetActionRef();
			if (pRef)
			{
				const auto color = ColorB(255, 255, 255, 255);
				const auto refPos = pRef->GetWorldPos();
				const auto actPos = pMemberActor->GetEntity()->GetWorldPos();
				const auto dir = (refPos - actPos);

				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(actPos, color, actPos + dir, color);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(refPos, 0.5f, color);
			}
		}


		//Sub orders is defined in the Execute Order function
		if (subOrderInfo.type != eSO_None)
		{
			if (!pMemberVehicle)
			{
				UpdateFootSubOrder(frametime, &memberInstance);
				continue;
			}
			else
			{

				UpdateVehicleSubOrder(frametime, &memberInstance);
				continue;
			}

		}

		if (currentTime - memberInstance.GetStats()->lastTimeSubOrderFinished < 0.25f)
			continue;

		const float delay = g_pGameCVars->sqd_membersUpdateDelay;//seconds

		if (currentTime - memberInstance.m_lastUpdateTime < delay)
			continue;

		if (currentTime - pMemberActor->GetActorStats()->lastTimeRespawned < 0.5f)
			continue;

		memberInstance.m_lastUpdateTime = currentTime;

		//In this func get only from entity, because AI Action Reference Entity is created before
		auto pOrderObject = GET_ENTITY(orderInfo.targetId);
		if (!pOrderObject)
			continue;

		const auto objectPos = pOrderObject->GetWorldPos();

		if (pOrderObject == memberInstance.GetActionRef())
		{
			if (objectPos != orderInfo.targetPos)
			{
				auto mat34 = pOrderObject->GetWorldTM();
				mat34.SetTranslation(orderInfo.targetPos);
				pOrderObject->SetWorldTM(mat34);
			}
		}

		if (IsMemberDetached(&memberInstance))
			continue;

		if (!memberInstance.CanBeUpdated())
			continue;

		if ((pMemberActor != GetLeader()) && (pMemberActor->GetHealth() > 0))
		{
			//Actor information
			const auto pMemberGrabStats = pMemberActor->GetGrabStats();
			if (pMemberGrabStats->isGrabbed)
				continue;
			//~Actor information

			auto pMemberAI = pMemberActor->GetEntity()->GetAI();
			if (!pMemberAI)
				continue;

			auto pMemberPipe = pMemberAI->CastToIPipeUser();
			if (!pMemberPipe)
				continue;

			if (!pMemberVehicle)
			{
				//Main orders
				const auto isAlien = pMemberActor->IsAlien();
				if (!isAlien)
				{
					UpdateHumanFootOrder(frametime, &memberInstance);
				}
				else
				{
					UpdateAlienOrder(frametime, &memberInstance);
				}
			}
			else
			{
				UpdateHumanVehicleOrder(frametime, &memberInstance);
			}
		}
	}
}

void CSquad::UpdateOrders(float frametime)
{
	UpdateOrdersNew(frametime);
	return;

	//auto pConqueror = g_pControlSystem->GetConquerorSystem();
	//if (!pConqueror)
	//	return;

	//static bool bDebugDraw = false;
	//static float color[] = { 1,1,1,1 };
	//for (auto& squadMember : m_members)
	//{
		//const auto scale = 300;
		//const auto xoffset = 300;
		//const auto yoffset = 30;
		//if (bDebugDraw && m_members.size() > 0 /*&& HasClientLeader()*/ && squadMember.GetId() != m_pLeader->GetEntityId() && GetActor(squadMember.m_entityId))
		//{
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, yoffset, 1.2f, color, false, "Members %i Name = %s", squadMember.m_index, (GetActor(squadMember.m_entityId)->GetEntity()->GetName()));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 20), 1.2f, color, false, "Members %i Order = %d", squadMember.m_index, int(squadMember.m_currentOrder));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 40), 1.2f, color, false, "Members %i Previous Order = %d", squadMember.m_index, int(squadMember.m_previousOrder));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 60), 1.2f, color, false, "Members %i GotoState = %d", squadMember.m_index, int(squadMember.m_currentGotoState));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 80), 1.2f, color, false, "Members %i aiCleanDuration = %d", squadMember.m_index, int(squadMember.m_aiCleanDuration));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 100), 1.2f, color, false, "Members %i squadMember.m_index = %d", squadMember.m_index, int(squadMember.m_index));
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 120), 1.2f, color, false, "Members %i guardpos = (%1.f,%1.f,%1.f)", squadMember.m_index, squadMember.m_guardPos.x, squadMember.m_guardPos.y, squadMember.m_guardPos.z);
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 140), 1.2f, color, false, "Members %i previousGuardpos = (%1.f,%1.f,%1.f)", squadMember.m_index, squadMember.m_previousGuardPos.x, squadMember.m_previousGuardPos.y, squadMember.m_previousGuardPos.z);
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 160), 1.2f, color, false, "Members %i searchpos = (%1.f,%1.f,%1.f)", squadMember.m_index, squadMember.m_searchPos.x, squadMember.m_searchPos.y, squadMember.m_searchPos.z);
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 180), 1.2f, color, false, "Members %i previousSearchpos = (%1.f,%1.f,%1.f)", squadMember.m_index, squadMember.m_previousSearchPos.x, squadMember.m_previousSearchPos.y, squadMember.m_previousSearchPos.z);
		//	gEnv->pRenderer->Draw2dLabel(xoffset + squadMember.m_index * scale, (yoffset + 200), 1.2f, color, false, "Members %i processedId = %i", squadMember.m_index, squadMember.m_processedEntityId);
		//}

	//	if (g_pGameCVars->sqd_debug_draw_client_squad > 0 && (HasClientLeader() || HasClientMember()))
	//	{
	//		static float color[] = { 1,1,1,1 };
	//		const auto size = TOS_Debug::SIZE_COMMON;
	//		const auto scale = 30;
	//		const auto xoffset = TOS_Debug::XOFFSET_COMMON;
	//		const auto yoffset = TOS_Debug::YOFFSET_SQUAD;

	//		auto& orderName = m_pSquadSystem->m_ordersStringMap[squadMember.m_currentOrder];
	//		auto& previousOrderName = m_pSquadSystem->m_ordersStringMap[squadMember.m_previousOrder];

	//		if (m_pLeader)
	//		{
	//			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, TOS_Debug::SIZE_HEADER, color, false,
	//				"Player squad: ");

	//			//gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, size, color, false,
	//			//	"Average Distance to Members %1.fm",
	//			//	GetAverageDistanceToMembers(m_pLeader->GetEntity()->GetWorldPos()));
	//		}

	//		gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + squadMember.GetIndex() * scale, size, color, false,
	//			"Member %i, Can be Updated %i, Order %s, Previous Order %s",
	//			squadMember.GetIndex(), squadMember.CanBeUpdated(), orderName, previousOrderName);
	//	}

	//	if (pConqueror && pConqueror->IsGamemode())
	//	{
	//		auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(squadMember.GetId());
	//		if (pActor)
	//		{
	//			auto pAI = pActor->GetEntity()->GetAI();
	//			if (pAI)
	//			{
	//				auto params = pAI->CastToIAIActor()->GetParameters();
	//				params.m_PerceptionParams.audioScale = 0.1;

	//				//May cause bug with defining of IsInCombat
	//				params.m_PerceptionParams.perceptionScale.audio = 0.1;

	//				pAI->CastToIAIActor()->SetParameters(params);
	//			}
	//		}
	//	}

	//	if (IsMemberDetached(&squadMember))
	//		continue;

	//	if (!squadMember.CanBeUpdated())
	//		continue;

	//	//By default in the squad which not including the player, all members follow its leader
	//	const auto isSquadWithPlayer = HasClientLeader() || HasClientMember();
	//	if (!isSquadWithPlayer)
	//	{
	//		const bool notTransitOrders = 
	//			squadMember.m_currentOrder != eSO_EnterVehicle &&
	//			squadMember.m_currentOrder != eSO_ExitVehicle &&
	//			squadMember.m_currentOrder != eSO_UseVehicleTurret;

	//		if (notTransitOrders)
	//		{
	//			if (m_pCommander /*&& m_lastCommanderOrder != eSO_None*/)
	//			{
	//				//Manual handled in commander update function
	//				//if (squadMember.GetCurrentOrder() != m_lastCommanderOrder)
	//				//{
	//				//	squadMember.SetCurrentOrder(m_lastCommanderOrder);
	//				//	ExecuteOrder(m_lastCommanderOrder, &squadMember, eEOF_ExecutedByAI);
	//				//}
	//			}
	//			else
	//			{
	//				if (squadMember.m_currentOrder != eSO_FollowLeader)
	//				{
	//					squadMember.SetCurrentOrder(eSO_FollowLeader);
	//					ExecuteOrder(eSO_FollowLeader, &squadMember, eEOF_ExecutedByAI);
	//				}
	//			}
	//		}
	//	}

	//	auto* pMemberActor = static_cast<CActor*>(GetActor(squadMember.m_entityId));
	//	if (pMemberActor && pMemberActor != GetLeader())
	//	{
	//		if (pMemberActor->GetGrabStats()->isGrabbed)
	//			continue;

	//		auto* pMemberVehicle = pMemberActor->GetLinkedVehicle();

	//		auto* pMemberAI = pMemberActor->GetEntity()->GetAI();
	//		if (pMemberAI)
	//		{
	//			auto* pAIProxy = pMemberAI->CastToIAIActor()->GetProxy();
	//			auto* pPipeUser = pMemberAI->CastToIPipeUser();

	//			if (pAIProxy)
	//			{
	//				auto health = pMemberActor->GetHealth();
	//				auto alertness = pAIProxy->GetAlertnessState();

	//				if (pPipeUser && health > 0)
	//				{
	//					const string memberClassName = pMemberActor->GetEntity()->GetClass()->GetName();
	//					auto pAttentionTarget = pPipeUser->GetAttentionTarget();
	//					auto attentionThreat = pPipeUser->GetAttentionTargetThreat();

	//					auto isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);
	//					auto isGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);
	//					auto isPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);
	//					auto isInCombat = pAttentionTarget != nullptr;

	//					//Транспорт не стреляет в точку, а едет туда 30.09.2022
	//					//if (pMemberVehicle && TOS_Vehicle::ActorIsDriver(pMemberActor))
	//					//{
	//					//	auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);
	//					//	if (pAI && pAI->CastToIPipeUser())
	//					//	{
	//					//		pPipeUser = pAI->CastToIPipeUser();
	//					//		pAttentionTarget = pPipeUser->GetAttentionTarget();
	//					//		attentionThreat = pPipeUser->GetAttentionTargetThreat();
	//					//		alertness = pAI->GetProxy()->GetAlertnessState();
	//					//	}
	//					//}

	//					if (alertness > 1 && !pAttentionTarget)
	//					{
	//						auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);

	//						if (pAI)
	//							pAI->Event(AIEVENT_CLEAR, 0);
	//						else
	//							pMemberAI->Event(AIEVENT_CLEAR, 0);
	//					}

	//					if (squadMember.m_currentOrder == eSO_SearchEnemy)
	//					{
	//						if (pPipeUser->GetGoalPipeId() == GOALPIPEID_ORDER_SEARCH)
	//						{
	//							auto searchPos = squadMember.m_searchPos;
	//							auto isAirAI = false;

	//							if (memberClassName == "Trooper")
	//								m_searchRadius = FOOT_SEARCH_RADIUS;
	//							else if (memberClassName == "Scout")
	//							{
	//								m_searchRadius = AIR_SEARCH_RADIUS;
	//								isAirAI = true;

	//								auto* pScout = (CScout*)pMemberActor;
	//								if (!pScout->m_searchbeam.isActive)
	//								{
	//									pScout->EnableSearchBeam(true);

	//									auto scoutPos = pScout->GetEntity()->GetWorldPos();
	//									auto scoutPos2 = Vec3(scoutPos.x, scoutPos.y + 3.0f, scoutPos.z - 4.0f);
	//									auto scoutBeamDir = (scoutPos2 - scoutPos).GetNormalized();

	//									pScout->SetSearchBeamGoal(scoutBeamDir);
	//								}

	//								if (!pAttentionTarget && alertness > 0)
	//									gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "TO_SCOUTMOAC_IDLE", pMemberAI);
	//							}
	//							else if (pMemberVehicle)
	//							{
	//								if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//								{
	//									m_searchRadius = AIR_SEARCH_RADIUS;
	//									isAirAI = true;
	//								}
	//								else
	//									m_searchRadius = LAND_SEARCH_RADIUS;
	//							}

	//							searchPos.x += Random(-m_searchRadius, m_searchRadius);
	//							searchPos.y += Random(-m_searchRadius, m_searchRadius);
	//							// calculate pos.z coord
	//							if (isAirAI)
	//							{
	//								//DONT CHANGE, provide stable calculations
	//								searchPos.z = 2000.f;

	//								ray_hit hit;
	//								IPhysicalEntity* pSkipEntities[10];
	//								int nSkip = 0;
	//								IItem* pItem = pMemberActor->GetCurrentItem();
	//								if (pItem)
	//								{
	//									CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
	//									if (pWeapon)
	//										nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
	//								}

	//								Vec3 castPos = searchPos;
	//								castPos.z -= 1.f;

	//								Vec3 castDir = (castPos - searchPos).GetNormalizedSafe() * 2000.f;

	//								if (gEnv->pPhysicalWorld->RayWorldIntersection(searchPos, castDir, ent_terrain | ent_static,
	//									rwi_ignore_noncolliding | rwi_stop_at_pierceable, &hit, 1, pSkipEntities, nSkip))
	//								{
	//									if (hit.pCollider)
	//									{
	//										if (hit.dist < SAFE_FLY_HEIGHT)
	//										{
	//											searchPos.z += SAFE_FLY_HEIGHT - hit.dist;
	//										}
	//										else if (hit.dist > SAFE_FLY_HEIGHT)
	//										{
	//											searchPos.z -= hit.dist - SAFE_FLY_HEIGHT;
	//										}
	//									}
	//								}
	//							}

	//							pPipeUser->SetRefPointPos(searchPos);

	//							if (pMemberVehicle && isDriver)
	//							{
	//								auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);
	//								if (pAI)
	//									pAI->CastToIPipeUser()->SetRefPointPos(searchPos);
	//							}

	//						}
	//						else if (pPipeUser->GetGoalPipeId() != GOALPIPEID_ORDER_SEARCH)
	//						{
	//							if (!pAttentionTarget)
	//							{
	//								pPipeUser->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH);

	//								if (pMemberVehicle && isDriver)
	//								{
	//									auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);
	//									if (pAI)
	//										pAI->CastToIPipeUser()->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH);
	//								}
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_Guard)
	//					{
	//						//static bool mustOffAIPerception = false;
	//						static string pipeName, debugState = "";
	//						Vec3 guardPos = squadMember.m_guardPos;//m_membersGuardPoses[pMember];

	//						if (memberClassName == "Scout")
	//						{
	//							guardPos.z += SAFE_FLY_HEIGHT;
	//							if (CScout* pScoutMember = (CScout*)pMemberActor)
	//							{
	//								if (pScoutMember->m_searchbeam.isActive)
	//									pScoutMember->EnableSearchBeam(false);
	//							}
	//						}
	//						else if (pMemberVehicle)
	//						{
	//							if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//								guardPos.z += SAFE_FLY_HEIGHT;
	//							else
	//								m_searchRadius = LAND_SEARCH_RADIUS;
	//						}

	//						const auto mustFinishCleaning = pPipeUser->IsUsingPipe("ord_cooldown") ||
	//							(pPipeUser->IsUsingPipe("ord_cooldown_trooper")) &&
	//							squadMember.m_aiCleanDuration == AI_CLEAN_FINISH;

	//						switch (squadMember.m_currentGotoState)
	//						{
	//						case eGUS_CleanAI:
	//							//fix: Sometimes m_aiCleanDuration is 0 when eGUS_CleanAI is starting
	//							//if (squadMember.m_aiCleanDuration == AI_CLEAN_FINISH &&
	//							//	pPipeUser->IsUsingPipe("ord_ready_guard"))
	//							//{
	//							//	squadMember.SetGotoUpdateState(eGUS_Guard);

	//							//	pPipeUser->SetRefPointPos(guardPos);
	//							//	pPipeUser->SelectPipe(0, "ord_guard", 0, GOALPIPEID_ORDER_GOTO_GUARD);
	//							//}

	//							if (squadMember.m_aiCleanDuration != AI_CLEAN_FINISH)
	//							{
	//								//Preparing entity, resetting it
	//								//TOS_AI::OffPerception(pMemberAI, true);

	//								debugState = "CLEAN_AI";

	//								static string sEntityClass = pMemberActor->GetEntity()->GetClass()->GetName();
	//								if (sEntityClass == "Trooper")
	//									pipeName = "ord_cooldown_trooper";
	//								else
	//									pipeName = "ord_cooldown";

	//								pPipeUser->SelectPipe(0, pipeName, 0, GOALPIPEID_ORDER_COOLDOWN);
	//								if (pPipeUser->IsUsingPipe(pipeName))
	//									squadMember.m_aiCleanDuration--;
	//							}
	//							else if (mustFinishCleaning)
	//							{
	//								//if resetted and ready to action

	//								pPipeUser->SelectPipe(0, "do_nothing");
	//								pPipeUser->SetRefPointPos(guardPos);

	//								pPipeUser->SelectPipe(0, "ord_goto", 0, GOALPIPEID_ORDER_GOTO);

	//								if (pMemberVehicle && isDriver)
	//								{
	//									auto* pVehAI = pMemberVehicle->GetEntity()->GetAI();
	//									if (pVehAI)
	//									{
	//										pVehAI->CastToIPipeUser()->SetRefPointPos(guardPos);
	//										pVehAI->CastToIPipeUser()->SelectPipe(0, "ord_goto_vehicle", 0, GOALPIPEID_ORDER_GOTO);
	//									}
	//								}

	//								squadMember.SetGotoUpdateState(eGUS_GoTo);
	//							}
	//							break;
	//						case eGUS_GoTo:
	//							debugState = "GOTO";
	//							if (pPipeUser->IsUsingPipe("ord_ready_guard"))
	//							{
	//								//TOS_AI::OffPerception(pMemberAI, false);

	//								pPipeUser->SetRefPointPos(guardPos);
	//								pPipeUser->SelectPipe(0, "ord_guard", 0, GOALPIPEID_ORDER_GOTO_GUARD);

	//								squadMember.SetGotoUpdateState(eGUS_Guard);

	//								//mustOffAIPerception = false;
	//							}

	//							if (pMemberVehicle && isDriver)
	//							{
	//								auto* pVehAI = pMemberVehicle->GetEntity()->GetAI();
	//								if (pVehAI)
	//								{
	//									if (pVehAI->CastToIPipeUser()->IsUsingPipe("ord_ready_guard"))
	//									{
	//										pVehAI->CastToIPipeUser()->SetRefPointPos(guardPos);
	//										pVehAI->CastToIPipeUser()->SelectPipe(0, "ord_guard", 0, GOALPIPEID_ORDER_GOTO_GUARD);
	//									}
	//								}
	//							}

	//							break;
	//						case eGUS_Guard:
	//							debugState = "GUARD";

	//							if (!TOS_AI::IsCombatEnable(pMemberAI))
	//								TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member start guarding point");

	//							if (pAttentionTarget && attentionThreat > AITHREAT_THREATENING)
	//								squadMember.SetGotoUpdateState(eGUS_Combat);
	//							break;
	//						case eGUS_Combat:
	//							debugState = "COMBAT";
	//							if (!pAttentionTarget && attentionThreat == AITHREAT_NONE)
	//							{
	//								squadMember.m_aiCleanDuration = AI_CLEAN_START_HALFSEC;
	//								squadMember.SetGotoUpdateState(eGUS_CleanAI);
	//							}

	//							break;
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_FollowLeader)
	//					{
	//						//Member must be relaxed and when he have not target and alertess != 0 --> alertness must be 0
	//						const auto isPartiallyRelaxed = attentionThreat < AITHREAT_THREATENING && !pAttentionTarget && alertness != 0;
	//						if (isPartiallyRelaxed /*&& respawnBugFix*/)
	//							TOS_AI::ReturnToFirst(pMemberAI, 0, 0, true);

	//						if (memberClassName == "Scout")
	//						{
	//							if (auto* pScout = (CScout*)pMemberActor)
	//							{
	//								if (pScout->m_searchbeam.isActive)
	//									pScout->EnableSearchBeam(false);
	//							}
	//						}

	//						auto pLeaderEntity = GetLeader() ? GetLeader()->GetEntity() : nullptr;
	//						if (pLeaderEntity)
	//						{
	//							const bool isNotFollowOrder = pPipeUser->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW;
	//							const bool isNotBackoff = pPipeUser->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW_BACKOFF;
	//							const bool isNoThreatening = attentionThreat < AITHREAT_THREATENING && !pAttentionTarget;

	//							const auto& memberPos = pMemberActor->GetEntity()->GetWorldPos();
	//							auto leaderPos = pLeaderEntity->GetWorldPos();

	//							auto pLeader = static_cast<CActor*>(GetLeader());
	//							const bool leaderIsAlien = pLeader->IsAlien();

	//							if (!pMemberVehicle)
	//							{
	//								if (isNoThreatening)
	//								{				
	//									//const auto alienDistance = 3.5f;
	//									const auto vehicleDistance = 6;
	//									const auto humanDistance = 3.0f;
	//									const auto distance = (memberPos - leaderPos).GetLength();
	//									
	//									const auto desiredDistance = pMemberVehicle ? vehicleDistance : humanDistance;

	//									const auto needMove = distance > desiredDistance;
	//									const auto needBack = distance <= 1.0f;

	//									const string leaderClassName = pLeaderEntity->GetClass()->GetName();

	//									if (memberClassName == "Scout")
	//										leaderPos.z += (leaderClassName == "Hunter") ? SAFE_FLY_DISTANCE_HUNTER : SAFE_FLY_HEIGHT;

	//									if (needMove)
	//									{
	//										pPipeUser->CancelSubPipe(GOALPIPEID_ORDER_FOLLOW_BACKOFF);
	//										pPipeUser->SetRefPointPos(leaderPos);

	//										if (isNotFollowOrder)
	//											pPipeUser->SelectPipe(0, "ord_follow_player", 0, GOALPIPEID_ORDER_FOLLOW);
	//									}
	//									else if(needBack)
	//									{
	//										//ии идёт только в одном направленнии
	//										//сделать отодвижение ии от игрока при малой дистанции между ними

	//										if (isNotBackoff)
	//										{
	//											Vec3 dir = (memberPos - leaderPos).GetNormalizedSafe();
	//											Vec3 backoffPos = leaderPos + dir * 2.5f;

	//											pPipeUser->SetRefPointPos(backoffPos);
	//											pPipeUser->InsertSubPipe(0, "ord_follow_player_backoff", 0, GOALPIPEID_ORDER_FOLLOW_BACKOFF);
	//										}
	//									}

	//								}
	//							}
	//							else if (pMemberVehicle && isDriver && g_pGameCVars->sqd_vehicle_follow_method != 0)
	//							{
	//								auto movType = pMemberVehicle->GetMovement()->GetMovementType();
	//								auto airType = IVehicleMovement::EVehicleMovementType::eVMT_Air;

	//								auto* pVehAI = pMemberVehicle->GetEntity()->GetAI();
	//								if (pVehAI)
	//								{
	//									if (isNoThreatening)
	//									{
	//										if (movType == airType)
	//											leaderPos.z += SAFE_FLY_HEIGHT;

	//										pVehAI->CastToIPipeUser()->SetRefPointPos(leaderPos);

	//										const bool isNotFollowOrder = !pVehAI->CastToIPipeUser()->IsUsingPipe("ord_follow_player_vehicle");
	//										if (isNotFollowOrder)
	//											pVehAI->CastToIPipeUser()->SelectPipe(0, "ord_follow_player_vehicle", 0, GOALPIPEID_ORDER_FOLLOW);
	//									}
	//								}
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_EnterVehicle)
	//					{
	//						auto* pProcessedVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(squadMember.GetProcessedId(false));
	//						if (pProcessedVehicle)
	//						{
	//							Vec3 actualRefPos = pPipeUser->GetRefPoint()->GetPos();
	//							Vec3 vehiclePos = pProcessedVehicle->GetEntity()->GetWorldPos();
	//							Vec3 memberPos = pMemberActor->GetEntity()->GetWorldPos();

	//							const auto distance = (memberPos - vehiclePos).GetLengthFast();
	//							const auto isAIR = TOS_Vehicle::IsAir(pProcessedVehicle);
	//							//const auto isGrounded = pProcessedVehicle->GetStatus().altitude < 2.0f;
	//							auto threshold = isAIR ? VEHICLE_AIR_ENTER_THRESHOLD_DIST : VEHICLE_LAND_ENTER_THRESHOLD_DIST;
	//								
	//							if (distance >= threshold)
	//							{
	//								if (actualRefPos != vehiclePos)
	//								{
	//									pPipeUser->SetRefPointPos(vehiclePos);
	//								}

	//								if (TOS_AI::GetGoalPipeId(pMemberAI) != GOALPIPEID_ORDER_GOTO)
	//								{
	//									TOS_AI::SelectPipe(pMemberAI, GOALPIPEID_ORDER_GOTO, "ord_goto", "SquadUpd: Member need be close with desired vehicle");
	//								}
	//							}
	//							else
	//							{
	//								const auto freeSeatIndex = TOS_Vehicle::RequestFreeSeatIndex(pProcessedVehicle);

	//								auto pSeat = pProcessedVehicle->GetSeatById(freeSeatIndex);
	//								if (!pSeat || pMemberActor->GetActorStats()->isRagDoll)
	//									continue;

	//								if (pSeat->Enter(squadMember.m_entityId, false))
	//								{
	//									if (GetLeader())
	//									{
	//										auto* pLeaderVehicle = GetLeader()->GetLinkedVehicle();
	//										if (pLeaderVehicle)
	//											squadMember.SetCurrentOrder(squadMember.m_previousOrder);
	//										else
	//										{
	//											pPipeUser->SelectPipe(0, "ord_ready_guard");
	//											squadMember.SetGotoUpdateState(eGUS_GoTo, false);
	//											squadMember.SetGuardPos(vehiclePos, false);
	//											squadMember.SetCurrentOrder(eSO_Guard, false);
	//										}

	//										if (!TOS_AI::IsCombatEnable(pMemberAI))
	//											TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member entered in vehicle");
	//									}
	//								}

	//								return;
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_ExitVehicle)
	//					{
	//						if (!TOS_AI::IsCombatEnable(pMemberAI))
	//							TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member need exit from vehicle");
	//					}
	//					else if (squadMember.m_currentOrder == eSO_PickupItem)
	//					{
	//						if (pMemberVehicle)
	//						{
	//							squadMember.ClearProcessedId();

	//							if (squadMember.m_previousOrder != eSO_PickupItem)
	//								ExecutePreviousOrder(&squadMember);

	//							continue;
	//						}		

	//						auto pProcessedItem = static_cast<CWeapon*>(g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(squadMember.GetProcessedId(false)));
	//						if (pProcessedItem)
	//						{
	//							if (pProcessedItem->IsSelected())
	//							{
	//								squadMember.ClearProcessedId();

	//								if (squadMember.m_previousOrder != eSO_PickupItem)
	//									ExecutePreviousOrder(&squadMember);

	//								continue;
	//							}

	//							TOS_AI::DrawPrimaryWeapon(pMemberAI);

	//							auto& actualRefPos = pPipeUser->GetRefPoint()->GetPos();
	//							auto& itemPos = pProcessedItem->GetEntity()->GetWorldPos();
	//							auto& memberPos = pMemberActor->GetEntity()->GetWorldPos();
	//							auto distance = (memberPos - itemPos).GetLength();

	//							bool isPickedUP = false;

	//							if (distance > 2.0f)
	//							{
	//								if (actualRefPos != itemPos)
	//									pPipeUser->SetRefPointPos(itemPos);

	//								if (pPipeUser->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
	//									pPipeUser->SelectPipe(0, "ord_goto", 0, GOALPIPEID_ORDER_GOTO);
	//							}
	//							else
	//							{
	//								auto pMemberCurrentItem = pMemberActor->GetCurrentItem();
	//								if (pMemberCurrentItem)
	//									pMemberActor->DropItem(pMemberCurrentItem->GetEntityId());
	//								
	//								isPickedUP = pMemberActor->PickUpItem(pProcessedItem->GetEntityId(), true, true);
	//							}

	//							if (isPickedUP)
	//							{
	//								squadMember.ClearProcessedId();

	//								if (squadMember.m_previousOrder != eSO_PickupItem)
	//									ExecutePreviousOrder(&squadMember);

	//								isPickedUP = false;

	//								if (!TOS_AI::IsCombatEnable(pMemberAI))
	//									TOS_AI::EnableCombat(pMemberAI, true, false,"SquadUpd: Member pickup item success");
	//							}

	//							TOS_AI::DrawPrimaryWeapon(pMemberAI);
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_UseVehicleTurret)
	//					{
	//						auto pProcessedVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(squadMember.GetProcessedId(false));
	//						if (pProcessedVehicle)
	//						{
	//							auto& actualRefPos = pPipeUser->GetRefPoint()->GetPos();
	//							auto& vehiclePos = pProcessedVehicle->GetEntity()->GetWorldPos();
	//							auto& memberPos = pMemberActor->GetEntity()->GetWorldPos();

	//							auto distance = (memberPos - vehiclePos).GetLength();

	//							auto movType = pProcessedVehicle->GetMovement()->GetMovementType();
	//							auto airMoveType = IVehicleMovement::EVehicleMovementType::eVMT_Air;
	//							auto threshold = VEHICLE_LAND_ENTER_THRESHOLD_DIST;

	//							if (movType == airMoveType)
	//								threshold = VEHICLE_AIR_ENTER_THRESHOLD_DIST;

	//							static auto gunnerSeatIndex = TOS_Vehicle::RequestGunnerSeatIndex(pProcessedVehicle);
	//							auto pGunnerSeat = pProcessedVehicle->GetSeatById(gunnerSeatIndex);

	//							if (pGunnerSeat)
	//							{
	//								if (pGunnerSeat->GetPassenger())
	//								{
	//									if (squadMember.m_previousOrder != eSO_UseVehicleTurret)
	//									{
	//										squadMember.ClearProcessedId();
	//										squadMember.SetGuardPos(pProcessedVehicle->GetEntity()->GetWorldPos(), false);
	//										squadMember.SetSearchPos(pProcessedVehicle->GetEntity()->GetWorldPos(), false);
	//										squadMember.SetCurrentOrder(eSO_Guard, false);
	//										squadMember.SetGotoUpdateState(eGUS_GoTo, false);
	//										//ExecutePreviousOrder(&squadMember);
	//									}

	//									continue;
	//								}
	//								else if (distance > threshold)
	//								{
	//									if (actualRefPos != vehiclePos)
	//										pPipeUser->SetRefPointPos(vehiclePos);

	//									if (pPipeUser->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
	//										TOS_AI::SelectPipe(pMemberAI, GOALPIPEID_ORDER_GOTO, "ord_goto", "SquadUpd: Member need be closer to desired vehicle turret");
	//										//pPipeUser->SelectPipe(0, "ord_goto", 0, GOALPIPEID_ORDER_GOTO);
	//								}
	//								else if (pProcessedVehicle == pMemberVehicle)
	//								{
	//									TOS_Vehicle::ChangeSeat(pMemberActor, pGunnerSeat->GetSeatId(), false);

	//									if (!TOS_AI::IsCombatEnable(pMemberAI))
	//										TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member change seat successful");

	//									squadMember.SetCurrentOrder(squadMember.m_previousOrder);
	//								}
	//								else if (distance <= threshold)
	//								{
	//									if (pGunnerSeat->Enter(squadMember.m_entityId, false))
	//									{
	//										if (!TOS_AI::IsCombatEnable(pMemberAI))
	//											TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member use turret successful");

	//										squadMember.SetCurrentOrder(squadMember.m_previousOrder);
	//									}
	//								}
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_ShootAt)
	//					{
	//						bool isShooting = pPipeUser->IsUsingPipe("tos_shoot_at");
	//						bool finishOrder = false;

	//						//в т.с стрелок после стрельбы зацикливает tos_on_end_shooting
	//						if (pPipeUser->IsUsingPipe("tos_on_end_shooting") || TOS_AI::IsInCombat(pMemberAI))
	//							finishOrder = true;

	//						if (!(pMemberVehicle && TOS_Vehicle::ActorIsDriver(pMemberActor)))
	//						{
	//							if (!isShooting && !finishOrder)
	//							{
	//								if (pMemberActor->GetStance() != STANCE_STAND)
	//									TOS_AI::SelectPipe(pMemberAI, 0, "do_it_standing", "SquadUpd: Member need be in standing before start shooting");

	//								pPipeUser->SetRefPointPos(squadMember.GetProcessedPos(false));
	//								ExecuteOrder(eSO_ShootAt, &squadMember, eEOF_ExecutedByAI);
	//							}
	//						}


	//						if (isShooting)
	//						{
	//							if (squadMember.m_processedPos == pPipeUser->GetRefPoint()->GetPos())
	//								squadMember.m_processedPos.zero();
	//						}

	//						if (finishOrder)
	//						{
	//							if (!TOS_AI::IsCombatEnable(pMemberAI))
	//								TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member end the shooting order");

	//							if (squadMember.m_previousOrder != eSO_ShootAt)
	//							{
	//								if (squadMember.m_previousOrder == eSO_Guard)
	//								{
	//									pPipeUser->SelectPipe(0, "ord_ready_guard");
	//									squadMember.SetGotoUpdateState(eGUS_GoTo, false);
	//									squadMember.SetGuardPos(squadMember.m_previousGuardPos, false);
	//									squadMember.SetCurrentOrder(eSO_Guard, false);
	//								}

	//								ExecutePreviousOrder(&squadMember);
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_Conq_Search_Cover_Around_Area)
	//					{
	//						const auto properGoalPipeId = GOALPIPEID_ORDER_CMDR_SEARCH_COVER;
	//						const auto notProperGoalPipeId = pPipeUser->GetGoalPipeId() != properGoalPipeId;
	//						const auto isAir = TOS_Vehicle::IsAir(pMemberVehicle);

	//						if (isDriver && pMemberVehicle)
	//						{
	//							if (!isAir)
	//							{
	//								auto pSeat = pMemberVehicle->GetSeatForPassenger(pMemberActor->GetEntityId());
	//								if (pSeat)
	//									pSeat->Exit(true, false);
	//							}
	//						}
	//						else if (isPassenger && pMemberVehicle)
	//						{
	//							if (isAir)
	//							{
	//								auto pSeat = pMemberVehicle->GetSeatForPassenger(pMemberActor->GetEntityId());
	//								if (pSeat)
	//									pSeat->Exit(true, false);
	//							}
	//						}
	//						else if (isGunner && pMemberVehicle)
	//						{
	//							if (!isAir)
	//							{
	//								if (!isInCombat)
	//								{
	//									bool random = (Random(0, 100) < 25);

	//									auto pSeat = pMemberVehicle->GetSeatForPassenger(pMemberActor->GetEntityId());
	//									if (pSeat && random)
	//										pSeat->Exit(true, false);
	//								}
	//							}
	//						}
	//						else
	//						{
	//							const auto isInCover = pPipeUser->IsUsingPipe("tos_commander_ord_goto_sub_hide_incover");

	//							if (!isInCombat)
	//							{
	//								if (notProperGoalPipeId && !isInCover)
	//								{
	//									const auto& searchRadius = squadMember.m_searchCoverRadius;
	//									const auto& searchPos = squadMember.GetProcessedPos(false);
	//									const auto& hidespotPos = m_pSquadSystem->GetNearestHidespot(searchPos, searchRadius);
	//									
	//									pPipeUser->SetRefPointPos(hidespotPos);
	//									pPipeUser->SelectPipe(0, "sqd_search_cover", 0, properGoalPipeId);
	//								}
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_Search_Cover_Around_Point)
	//					{
	//						const auto properGoalPipeId = GOALPIPEID_ORDER_SEARCH_COVER;
	//						const auto notProperGoalPipeId = pPipeUser->GetGoalPipeId() != properGoalPipeId;

	//						if (pMemberVehicle)
	//						{
	//							auto pSeat = pMemberVehicle->GetSeatForPassenger(pMemberVehicle->GetEntityId());
	//							if (pSeat)
	//								pSeat->Exit(true, false);
	//						}
	//						else
	//						{
	//							const auto isInCover = pPipeUser->IsUsingPipe("tos_commander_ord_goto_sub_hide_incover");
	//							const auto searchPos = squadMember.GetProcessedPos(true);
	//							
	//							if (!isInCombat)
	//							{
	//								if (notProperGoalPipeId && !isInCover)
	//								{
	//									const auto& hidespotPos = m_pSquadSystem->GetNearestHidespot(searchPos);
	//									
	//									pPipeUser->SetRefPointPos(hidespotPos);
	//									pPipeUser->SelectPipe(0, "sqd_search_cover", 0, properGoalPipeId);
	//								}
	//							}
	//						}
	//					}
	//					else if (squadMember.m_currentOrder == eSO_Conq_GoTo)
	//					{
	//						const int moveDistTreshold = 10;

	//						const auto haveTarget = pMemberAI->CastToIPipeUser()->GetAttentionTarget() != nullptr;
	//						const auto isCombat = haveTarget;
	//						const auto isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);

	//						const auto memberPos = pMemberActor->GetEntity()->GetWorldPos();
	//						auto targetPos = squadMember.m_processedPos;

	//						const auto properGoalPipeId = GOALPIPEID_ORDER_CMDR_GOTO;
	//						const auto notProperGoalPipeId = TOS_AI::GetGoalPipeId(pMemberAI) != properGoalPipeId;
	//						const auto notProperTarget = TOS_AI::GetRefPoint(pMemberAI) != targetPos;

	//						if (memberClassName == "Scout")
	//						{
	//							targetPos.z += SAFE_FLY_HEIGHT;
	//							if (CScout* pScoutMember = (CScout*)pMemberActor)
	//							{
	//								if (pScoutMember->m_searchbeam.isActive)
	//									pScoutMember->EnableSearchBeam(false);
	//							}
	//						}
	//						else if (pMemberVehicle)
	//						{
	//							if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//								targetPos.z += SAFE_FLY_HEIGHT;
	//							else
	//								m_searchRadius = LAND_SEARCH_RADIUS;
	//						}

	//						const float distance2d = (targetPos - memberPos).GetLength2D();
	//						const auto needMove = distance2d >= moveDistTreshold;
	//						if (needMove)
	//						{
	//							if (isDriver && pMemberVehicle)
	//							{
	//								auto pVehAI = pMemberVehicle->GetEntity()->GetAI();
	//								if (pVehAI)
	//								{
	//									const auto vehicleNotProperGoalPipeId = TOS_AI::GetGoalPipeId(pVehAI) != properGoalPipeId;
	//									const auto vehicleNotProperTarget = TOS_AI::GetRefPoint(pVehAI) != targetPos;
	//									
	//									if (vehicleNotProperGoalPipeId)
	//										TOS_AI::SelectPipe(pVehAI, properGoalPipeId, "tos_commander_ord_goto", "SquadUpd: Vehicle need moving to target position");
	//									
	//									if (vehicleNotProperTarget)
	//										TOS_AI::SetRefPoint(pVehAI, targetPos);
	//								}
	//							}
	//							else
	//							{
	//								if (!isCombat)
	//								{
	//									if (notProperTarget)
	//										TOS_AI::SetRefPoint(pMemberAI, targetPos);

	//									if (notProperGoalPipeId)
	//										TOS_AI::SelectPipe(pMemberAI, properGoalPipeId, "tos_commander_ord_goto", "SquadUpd: Member need moving to target position");
	//								}
	//							}
	//						}
	//						else
	//						{
	//							//Controlled from Conqueror Commander line 461
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	/////////////////////// Demiurg mode /////////////////////////////////////////

	//when the controlled actor in the squad is dead, switch him to a alive actor in the squad.
	/*bool bDemiurgIsEnabled = false;
	if (!bDemiurgIsEnabled)
		return;

	if (auto* pActor = g_pControlSystem->GetLocalControlClient()->GetControlledActor())
	{
		if (pActor->GetHealth() < 0.1f)
		{
			static float timer = 1.0f;

			if (timer != 0)
			{
				timer -= gEnv->pTimer->GetFrameTime();
				if (timer < 0)
					timer = 0;
			}

			if (timer == 0)
			{
				auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, 1);
				if (pSquad)
				{
					if (auto* pMemberActor = static_cast<CActor*>(pSquad->GetActor(pSquad->GetMemberAlive()->m_entityId)))
					{
						g_pControlSystem->GetLocalControlClient()->SetActor(pMemberActor);
						SetLeader(pActor, false);

						timer = 1.0f;
					}
				}
			}
		}
	}*/

	/////////////////////// ~Demiurg mode /////////////////////////////////////////
}

void CSquad::UpdateMembersHUD()
{
	TMembers::iterator it = m_members.begin();
	TMembers::iterator end = m_members.end();
	for (; it != end; it++)
	{
		m_pSquadSystem->ShowSquadMemberHUD(true, GetIndexFromMember(*it));
	}
}

void CSquad::Serialize(TSerialize ser)
{
	ser.BeginGroup("tosSquad");
	//SER_VALUE_ENUM(m_eLeaderCurrentOrder, eSO_Guard, eSO_Last);
	SER_VALUE(m_searchRadius);
	SER_VALUE(m_squadId);

	TMembers members;
	TSMembers selectedMembers;
	if (ser.IsWriting())
	{
		members = m_members;
		selectedMembers = m_selectedMembers;

		ser.Value("squadMembers", members);
		ser.Value("squadSelectedMembers", selectedMembers);
	}
	else
	{
		ser.Value("squadMembers", members);

		TMembers::iterator it = members.begin();
		TMembers::iterator end = members.end();
		for (; it != end; it++)
		{
			AddMember(*it);
		}

		ser.Value("squadSelectedMembers", selectedMembers);

		TSMembers::iterator it2 = selectedMembers.begin();
		TSMembers::iterator end2 = selectedMembers.end();
		for (; it2 != end2; it2++)
		{
			AddMemberToSelected(*it2);
		}
	}

	TMembers::iterator it = m_members.begin();
	TMembers::iterator end = m_members.end();
	for (; it != end; it++)
		it->Serialize(ser);
	ser.EndGroup();
}

float CSquad::GetMinDistance() const
{
	if (!GetLeader())
		return 0.0f;

	IEntity* pLeaderEnt = GetLeader()->GetEntity();
	std::vector<float> values;

	for (int i = 0; i <= m_members.size(); i++)
	{
		IEntity* pMemberEnt = gEnv->pEntitySystem->GetEntity(m_members[i].m_entityId);

		if (pMemberEnt)
		{
			Vec3 leaderPos = pLeaderEnt->GetWorldPos();
			Vec3 memberPos = pMemberEnt->GetWorldPos();

			float dist = (leaderPos - memberPos).GetLength();
			values.push_back(dist);
		}
	}

	float min = *std::min_element(values.begin(), values.end());
	return min;
}

void CSquad::RemoveMemberFromSelected(const IActor* pMember)
{
	if (!pMember)
		return;

	if (IsMember(pMember))
	{
		TSMembers::const_iterator it = m_selectedMembers.begin();
		TSMembers::const_iterator end = m_selectedMembers.end();

		bool finded = false;
		for (; it != end; it++)
		{
			if (*it == pMember->GetEntityId())
			{
				finded = true;
				break;
			}
		}

		if (finded)
			m_selectedMembers.erase(it);
	}
}

void CSquad::RemoveMemberFromSelected(const int index)
{
	auto* pMember = GetMemberFromIndex(index);
	if (!pMember || !GetActor(pMember->m_entityId))
		return;

	TSMembers::const_iterator it = m_selectedMembers.begin();
	TSMembers::const_iterator end = m_selectedMembers.end();

	bool finded = false;
	for (; it != end; it++)
	{
		if (*it == pMember->m_entityId)
		{
			finded = true;
			break;
		}
	}

	if (finded)
		m_selectedMembers.erase(it);
}

//int CSquad::GetOrder(const EntityId id)
//{
//	auto* pMember = GetMemberInstance(id);
//
//	if (GetActor(pMember->m_entityId))
//		return pMember->m_currentOrder;
//
//	return eSO_None;
//}
//
//int CSquad::GetOrder(const IActor* act)
//{
//	auto* pMember = GetMemberInstance(act);
//	if (pMember && GetActor(pMember->m_entityId))
//		return pMember->m_currentOrder;
//
//	return eSO_None;
//}
//
//int CSquad::GetOrder(const int index)
//{
//	//if (!IsEnabled())
//	//	return 0;
//
//	/*if (IActor* pMember = GetMemberFromSlot(slot))
//	{
//		if (ESquadOrders currentOrder = m_membersOrders.find(pMember)->second)
//			return (int)currentOrder;
//	}*/
//
//	auto* pMember = GetMemberFromIndex(index);
//	if (GetActor(pMember->m_entityId))
//		return pMember->m_currentOrder;
//
//	return eSO_None;
//}

CMember* CSquad::GetMemberInstance(const EntityId id)
{
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (!pActor)
		return nullptr;

	for (auto& member : m_members)
	{
		if (member.m_entityId == pActor->GetEntityId())
		{
			auto* memberPtr = &member;
			return memberPtr;
		}	
	}

	return nullptr;
}

CMember* CSquad::GetMemberInstance(const IActor* pActor)
{
	if (!pActor)
		return nullptr;

	for (auto& member : m_members)
	{
		if (member.m_entityId == pActor->GetEntityId())
			return &member;
	}

	return nullptr;
}

CMember* CSquad::GetMemberFromIndex(const int index)
{
	for (auto& member : m_members)
	{
		if (member.m_index == index)
		{
			return &member;
		}
	}
}

bool CSquad::IsMember(const CMember* pMember) const
{
	for (auto& member : m_members)
	{
		if (member.m_entityId == pMember->m_entityId)
			return true;
	}

	return false;
}

bool CSquad::IsMember(const IActor* pActor) const
{
	if (!pActor)
		return false;

	TMembers::const_iterator it = m_members.begin();
	TMembers::const_iterator end = m_members.end();
	for (; it != end; it++)
	{
		if (it->m_entityId == pActor->GetEntityId())
			return true;
	}

	return false;
}

bool CSquad::IsMember(const EntityId id) const
{
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (!pActor)
		return false;

	TMembers::const_iterator it = m_members.begin();
	TMembers::const_iterator end = m_members.end();
	for (; it != end; it++)
	{
		if (it->m_entityId == pActor->GetEntityId())
			return true;
	}

	return false;
}

int CSquad::GetIndexFromMember(const CMember& Member)
{
	return Member.m_index;
}

int CSquad::GetIndexFromMember(const IActor* pActor)
{
	auto* pMember = GetMemberInstance(pActor);
	if (pMember)
		return pMember->m_index;
	
	return -1;
}

int CSquad::GetIndexFromMember(const EntityId id)
{
	auto member = GetMemberInstance(id);
	return member->m_index;
}

int CSquad::GetFreeMemberIndex() const
{
	TMembers::const_iterator it = m_members.begin();
	TMembers::const_iterator end = m_members.end();

	for (; it != end; it++)
	{
		if (it->m_entityId == 0)
			return it - m_members.begin();
	}

	return -1;
}

CMember* CSquad::GetMemberAlive()
{
	for (auto& member : m_members)
	{
		auto pActor = GetActor(member.m_entityId);

		if (pActor && pActor->GetHealth() > 0.1f)
			return &member;
	}
	return nullptr;
}

bool CSquad::IsMemberSelected(const IActor* pActor)
{
	if (!pActor)
		return false;

	TSMembers::const_iterator it = m_selectedMembers.begin();
	TSMembers::const_iterator end = m_selectedMembers.end();

	for (; it != end; it++)
	{
		if (IsMember(pActor) && pActor->GetEntityId() == *it)
			return true;
	}

	return false;
}

bool CSquad::IsMemberSelected(const int index)
{
	auto* pMember = GetMemberFromIndex(index);
	if (!GetActor(pMember->m_entityId))
		return false;

	return stl::find(m_selectedMembers, GetActor(pMember->m_entityId)->GetEntityId());
}

void CSquad::AddMemberToSelected(const IActor* pActor)
{
	if (!pActor)
		return;

	if (IsMember(pActor))
	{
		TSMembers::const_iterator it = m_selectedMembers.begin();
		TSMembers::const_iterator end = m_selectedMembers.end();

		bool finded = false;
		for (; it != end; it++)
		{
			if (*it == pActor->GetEntityId())
			{
				finded = true;
				break;
			}
		}

		if (!finded)
		{
			m_selectedMembers.push_back(pActor->GetEntityId());
		}
		else
		{
			if (m_pSquadSystem->m_isDebugLog)
				CryLogAlways("[SSquad::AddMemberToSelected] try to add already selected member");
		}
	}
}

void CSquad::AddMemberToSelected(int index)
{
	auto pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	auto pMember = GetMemberFromIndex(index);
	if (!pMember)
		return;

	if (!GetActor(pMember->m_entityId))
		return;

	//We cannot select the player
	if (GetActor(pMember->m_entityId) == pPlayer)
		return;

	auto it = m_selectedMembers.begin();
	auto end = m_selectedMembers.end();

	bool finded = false;
	for (; it != end; it++)
	{
		if (*it == pMember->m_entityId)
		{
			finded = true;
			break;
		}
	}

	if (!finded)
		m_selectedMembers.push_back(pMember->m_entityId);
	else
	{
		if (m_pSquadSystem->m_isDebugLog)
			CryLogAlways("[SSquad::AddMemberToSelected] try to add already selected member");
	}
}


void CSquad::SetLeader(const IActor* pLeaderCandidate, bool isConquerorGamemode)
{
	const auto pCurrLeader = GetLeader();

	if (!pLeaderCandidate)
	{
		//if (m_pSquadSystem->m_isDebugLog)
			CryLogAlways("[C++][Squad %i][FAIELD Set new leader][Cause: New leader is null]", GetId());

		return;
	}

	for (const auto& pSquad : g_pControlSystem->GetSquadSystem()->m_allSquads)
	{
		if (pSquad->GetLeader())
		{
			if (pSquad->GetId() != GetId() && pLeaderCandidate->GetEntityId() == pSquad->GetLeader()->GetEntityId())
			{
				//if (m_pSquadSystem->m_isDebugLog)
					CryLogAlways("[C++][Squad %i][FAIELD Set new leader][Cause: The actor is already a leader]", GetId());

				return;
			}
		}
	}

	const auto pPlayer = g_pControlSystem->GetClientActor();
	if (!pPlayer)
		return;

	if (pLeaderCandidate == pPlayer)
	{
		const auto pOldSquad = m_pSquadSystem->GetSquadFromLeader(pPlayer);
		if (pOldSquad)
		{
			const auto pActor = pOldSquad->RequestNewLeader(!isConquerorGamemode, isConquerorGamemode);
			if (pActor)
			{
				pOldSquad->SetLeader(pActor, true);

				if (m_pSquadSystem->m_isDebugLog)
					CryLogAlways("[C++][Squad %i the player has left the leader position][New leader appointed (%s)]", pOldSquad->GetId(), pActor->GetEntity()->GetName());
			}
		}
	}

	if (isConquerorGamemode)
	{
		if (pCurrLeader && pCurrLeader != pPlayer)
			m_pConquerorOldAILeader = pCurrLeader;
	}
		

	if (IsMember(pLeaderCandidate))
		RemoveMember(GetMemberInstance(pLeaderCandidate));

	if (pCurrLeader == pPlayer)
		OnPlayerRemoved();		

	//if (m_pLeader)
		//m_leadersStats[m_pLeader->GetEntityId()].reset();

	//m_leaderInstance.set;
	m_leaderId = pLeaderCandidate->GetEntityId();

	for (auto& member : m_members)
	{
		SOrderInfo info;
		member.GetOrderInfo(info, false);

		if (info.type == eSO_FollowLeader)
		{
			info.targetId = GetLeader()->GetEntityId();
			info.targetPos = GetLeader()->GetEntity()->GetWorldPos();

			member.SetOrderInfo(info, false);
		}
	}

	if (pLeaderCandidate == pPlayer)
		OnPlayerAdded();
}

bool CSquad::AddMember(const IActor* pActor)
{
	const CActor* pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (IsMember(pActor))
		return false;

	if (pActor && pActor->GetHealth() > 0)
	{
		if (pActor == pPlayer)
			OnPlayerAdded();

		CMember member(pActor->GetEntityId());
		const auto newGroupId = g_pControlSystem->GetSquadSystem()->RequestGroupId(member.m_groupId, pActor->GetEntity());

		member.SetStep(EOrderExecutingStep::NotHaveOrder, false);
		member.SetStep(EOrderExecutingStep::NotHaveOrder, true);
		member.EnableUpdating(true);
		member.SetCurrentGroupId(newGroupId);
		member.m_index = static_cast<int>(m_members.size());
		GetActionRef(&member, pActor->GetEntity()->GetWorldPos());

		//if (auto* pMemberAI = pActor->GetEntity()->GetAI())
		//{
		//	if (pMemberAI->CastToIPipeUser())
		//	{
		//		if (member.m_currentGotoState != eGUS_CleanAI)
		//			member.SetGotoUpdateState(eGUS_CleanAI);

		//		member.m_aiCleanDuration = 10;
		//	}

		//	member.EnableUpdating(true);
		//	TOS_AI::DrawPrimaryWeapon(pMemberAI);
		//}

		auto* pRadar = SAFE_HUD_FUNC_RET(GetRadar());
		if (pRadar)
		{
			pRadar->AddEntityToRadar(member.m_entityId);
			pRadar->SetTeamMate(member.m_entityId, true);
		}

		m_members.push_back(member);

		if (HasClientMember() || HasClientLeader())
			m_pSquadSystem->ShowSquadMemberHUD(true, member.m_index);

		return true;
	}

	return false;
}

bool CSquad::AddMember(CMember& member)
{
	CActor* pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (IsMember(&member))
		return false;

	auto* pMemberActor = GetActor(member.m_entityId);
	if (pMemberActor && pMemberActor->GetHealth() > 0.1f)
	{
		if (pMemberActor == pPlayer)
			OnPlayerAdded();

		const auto newGroupId = g_pControlSystem->GetSquadSystem()->RequestGroupId(member.m_groupId, pMemberActor->GetEntity());

		//member.m_previousSearchPos = 
		//	member.m_previousGuardPos = 
		//	member.m_guardPos = 
		//	member.m_searchPos = 
		//	pMemberActor->GetEntity()->GetWorldPos();

		member.SetStep(EOrderExecutingStep::NotHaveOrder, false);
		member.SetStep(EOrderExecutingStep::NotHaveOrder, true);
		member.EnableUpdating(true);
		member.SetCurrentGroupId(newGroupId);
		member.m_index = m_members.size();
		GetActionRef(&member, pMemberActor->GetEntity()->GetWorldPos());

		//member.SetCurrentOrder(eSO_FollowLeader);
		//member.SetGotoUpdateState(eGUS_Guard);

		//if (auto* pMemberAI = pMemberActor->GetEntity()->GetAI())
		//{
		//	if (pMemberAI->CastToIPipeUser())
		//	{
		//		if (member.m_currentGotoState != eGUS_CleanAI)
		//			member.SetGotoUpdateState(eGUS_CleanAI);

		//		member.m_aiCleanDuration = 10;
		//	}

		//	member.EnableUpdating(true);
		//	TOS_AI::DrawPrimaryWeapon(pMemberAI);
		//}

		auto* pRadar = SAFE_HUD_FUNC_RET(GetRadar());
		if (pRadar)
		{
			pRadar->AddEntityToRadar(member.m_entityId);
			pRadar->SetTeamMate(member.m_entityId, true);
		}

		m_members.push_back(member);

		if (HasClientMember() || HasClientLeader())
			m_pSquadSystem->ShowSquadMemberHUD(true, member.m_index);

		return true;
	}

	return false;
}

bool CSquad::RemoveMember(CMember* pMember)
{
	if (!pMember)
		return false;

	bool isFinded = false;

	auto it = m_members.begin();
	auto end = m_members.end();

	for (; it != end; it++)
	{
		if (it->m_entityId == pMember->m_entityId)
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		int lastMemberIndex = GetMembersCount() - 1;

		if (HasClientMember() || HasClientLeader())
			m_pSquadSystem->ShowDeadSquadMemberHUD(lastMemberIndex);

		CActor* pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
		if (!pPlayer)
			pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (GetActor(pMember->m_entityId) == pPlayer)
			OnPlayerRemoved();

		RemoveMemberFromSelected(pMember->m_entityId);

		auto previousGroupId = pMember->m_previousGroupId;
		pMember->SetCurrentGroupId(previousGroupId);

		m_members.erase(it);

		auto it = m_members.begin();
		auto end = m_members.end();
		for (; it != end; it++)
		{
			if (it->m_index > 0)
				it->m_index -= 1;
		}

		if (HasClientMember() || HasClientLeader())
			m_pSquadSystem->UpdateSelectedHUD();

		return true;
	}
	return false;
}
#define DEFINE_ACTOR_ORDER_TYPE(orderType) \
	switch (orderType)\
	{\
		case eSO_ConqSearchCoverAroundArea:\
		case eSO_SearchCoverAroundPoint:\
		case eSO_ConqGoTo:\
		case eSO_SearchEnemy:\
		case eSO_Guard:\
		{\
			isMainOrder = true;\
			\
			info.targetId = pOrderTarget->GetId();\
			info.targetPos = pOrderTarget->GetWorldPos();\
		}\
		break;\
		case eSO_ConqBlankAction:\
		case eSO_FollowLeader:\
		{\
			/*Correct and update the target position in UpdateOrdersNew function*/\
			isMainOrder = true;\
		}\
		break;\
		\
		case eSO_SubPrimaryShootAt:\
		case eSO_SubSecondaryShootAt:\
		case eSO_SubEnterVehicle:\
		case eSO_SubExitVehicle:\
		case eSO_SubPrimaryPickupItem:\
		case eSO_SubSecondaryPickupItem:\
		case eSO_SubUseVehicleTurret:\
			isSubOrder = true;\
			\
			info.targetId = pOrderTarget->GetId();\
			info.targetPos = pOrderTarget->GetWorldPos();\
		case eSO_DebugEnableCombat:\
		case eSO_DebugDisableCombat:\
		case eSO_DebugStanceRelaxed:\
		case eSO_DebugStanceStanding:\
		case eSO_DebugStanceStealth:\
		case eSO_DebugStanceCrouch:\
			isDebugOrder = true;\
			break;\
		\
		case eSO_None:\
		default:\
			break;\
	}\

#define DEFINE_VEHICLE_ORDER_TYPE(orderType) \
	switch (orderType)\
	{\
	case eSO_ConqSearchCoverAroundArea:\
	case eSO_SearchCoverAroundPoint:\
	case eSO_ConqGoTo:\
	case eSO_SearchEnemy:\
	case eSO_Guard:\
	{\
		isMainOrder = true;\
		info.targetId = pOrderTarget->GetId();\
		info.targetPos = pOrderTarget->GetWorldPos();\
	}\
	break;\
	case eSO_ConqBlankAction:\
	case eSO_FollowLeader:\
	{\
		isMainOrder = true;\
	}\
	break;\
	\
	case eSO_SubPrimaryPickupItem:\
	case eSO_SubSecondaryPickupItem:\
	case eSO_SubPrimaryShootAt:\
	case eSO_SubSecondaryShootAt:\
	case eSO_SubEnterVehicle:\
	case eSO_SubExitVehicle:\
	case eSO_SubUseVehicleTurret:\
		isSubOrder = true;\
		\
		info.targetId = pOrderTarget->GetId();\
		info.targetPos = pOrderTarget->GetWorldPos();\
	case eSO_DebugEnableCombat:\
	case eSO_DebugDisableCombat:\
	case eSO_DebugStanceRelaxed:\
	case eSO_DebugStanceStanding:\
	case eSO_DebugStanceStealth:\
	case eSO_DebugStanceCrouch:\
		isDebugOrder = true;\
		break;\
		\
	case eSO_None:\
	default:\
		break;\
	}\

void CSquad::ExecuteOrder(CMember * pMember, const SOrderInfo& inputOrderInfo, uint executeFlags)
{
	bool ok = false;

	if (!pMember)
		return;

	auto pMemberActor = static_cast<CActor*>(GetActor(pMember->GetId()));
	if (!pMemberActor)
		return;

	//Only local client side
	auto pDude = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return;

	auto pControl = pDude->GetControlClient();
	if (!pControl)
		return;

	const bool detachedOrder = stl::find(m_pSquadSystem->m_allowedDetachedOrders, inputOrderInfo.type);
	if (IsMemberDetached(pMember))
	{
		if (!detachedOrder)
			return;
	};

	auto pMemberAI = pMemberActor->GetEntity()->GetAI();
	if (!pMemberAI)
		return;

	auto pPipeUser = pMemberAI->CastToIPipeUser();
	if (!pPipeUser)
		return;

	const string memberClass = pMemberActor->GetEntity()->GetClass()->GetName();
	const auto memberIsAlien = pMemberActor->IsAlien(); //memberClass != "Player" && memberClass != "Grunt";
	const auto memberIsFly = memberClass == "Scout" || memberClass == "Drone";

	auto pMemberVehicle = TOS_Vehicle::GetVehicle(pMemberActor);
	auto pMemberVehicleAI = TOS_Vehicle::GetAI(pMemberVehicle);
	const auto vehIsAir = TOS_Vehicle::IsAir(pMemberVehicle);

	const auto memberIsDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);
	const auto memberIsGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);
	const auto memberIsPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);

	const int stepsCount = (int)EOrderExecutingStep::Last;
	int nullCount = 0;
	for (int i = 0; i < stepsCount; i++)
	{
		const auto currentStep = EOrderExecutingStep(i);
		if (strcmp(inputOrderInfo.stepActions.at(currentStep), "nullActionName") == 0)
		{
			nullCount++;
		}
	}

	if (nullCount == stepsCount)
	{
		CryLogAlways("%s[C++][ERROR][Squad %i][Member %s][Execute Order %s FAILED][Cause: UNDEFINED AI ACTIONS AT ALL STEPS]",
			STR_RED, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(inputOrderInfo.type));
	}

	auto pOrderTarget = inputOrderInfo.targetId > 0 ? GET_ENTITY(inputOrderInfo.targetId) : GetActionRef(pMember, inputOrderInfo.targetPos);
	if (!pOrderTarget)
	{
		CryLogAlways("%s[C++][ERROR][Squad %i][Member %s][Execute Order %s FAILED][Cause: ORDER TARGET UNDEFINED]", 
			STR_RED, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(inputOrderInfo.type));
		return;
	}

	//CryLogAlways("%s[C++][Target name: %s]", STR_GREEN ,pOrderTarget->GetName());

	if (HasClientLeader() && (executeFlags & eEOF_ExecutedByPlayer))
		m_pSquadSystem->SpawnOrderParticle(inputOrderInfo.type, pOrderTarget->GetWorldPos());

	//If inputOrderInfo.targetId not equal saved ActionRef then we need to equal it
	//if ((pOrderTarget && pMember->GetActionRef()) && (pOrderTarget != pMember->GetActionRef()))
	//	pOrderTarget = pMember->GetActionRef();

	SOrderInfo info = inputOrderInfo;
	bool isMainOrder = false;
	bool isSubOrder = false;
	bool isDebugOrder = false;
	bool isVehCompatible = true;

	if (!pMemberVehicle)
	{
		DEFINE_ACTOR_ORDER_TYPE(inputOrderInfo.type);
	}
	else
	{
		DEFINE_VEHICLE_ORDER_TYPE(inputOrderInfo.type);

		//Determination of orders incompatible with vehicles
		switch (inputOrderInfo.type)
		{
		case eSO_SubPrimaryPickupItem:
		//case eSO_SubAlienGrabPlayerSquad:
		//case eSO_SubAlienDropPlayerSquad:
		case eSO_SubSecondaryPickupItem:
		case eSO_DebugEnableCombat:
		case eSO_DebugDisableCombat:
		case eSO_DebugStanceRelaxed:
		case eSO_DebugStanceStanding:
		case eSO_DebugStanceStealth:
		case eSO_DebugStanceCrouch:
		case eSO_ConqSearchCoverAroundArea:
		case eSO_SearchCoverAroundPoint:
			isVehCompatible = false;
			break;
		}
	}

	DEFINE_STEPS;

	//Reset booked hidespot
	if (m_pSquadSystem->GetBookedHideSpot(pMemberActor))
		m_pSquadSystem->UnbookHideSpot(pMemberActor, "CSquad::ExecuteOrder: reset booked hidespot");

	//Reset member target
	//SOrderInfo memberTargetReset;
	//pMember->GetOrderInfo(memberTargetReset, false);
	//memberTargetReset.targetId = 0;
	//memberTargetReset.targetPos = Vec3(0);
	//pMember->SetOrderInfo(memberTargetReset, false);

	if (!pMemberVehicle)
	{
		//Whenever the AI is without vehicles we should cancel the action 
		//and reset the sub - order
		
		//Cancel current AI Action
		TOS_AI::AbortAIAction(pMemberAI, -1, "CSquad::ExecuteOrder need clear current ai action when start executing new order");

		//Cancel current sub order
		pMember->ResetOrder(false, true, false);

		if (isMainOrder)
		{
			//Set previous order
			SOrderInfo prevOrder;
			pMember->GetOrderInfo(prevOrder, false);
			pMember->SetOrderInfo(prevOrder, true);

			//Set current order. Further information about the order is used by the Update Orders function
			pMember->SetOrderInfo(info, false);

			//Set current execution step
			pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
		}

		if (isSubOrder)
		{
			//Set current sub order.
			pMember->SetSubOrderInfo(info);

			//Set current execution step
			pMember->SetStep(EOrderExecutingStep::GotAnOrder, true);

			//CryLogAlways("pMember->SetSubOrderInfo(info);");
		}
		else if (isDebugOrder)
		{
			//Set current sub order.
			pMember->SetSubOrderInfo(info);

			//Set current execution step
			pMember->SetStep(EOrderExecutingStep::GotAnOrder, true);
		}

		ok = true;

		if (g_pGameCVars->sqd_debug_log_executing)
		{
			CryLogAlways("%s[C++][Squad %i][Member %s][Execute Order %s Success]",
				STR_GREEN, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(inputOrderInfo.type));
		}

	}
	else
	{
		if (isVehCompatible)
		{
			//If the AI is in a vehicle, 
			//then we should cancel the action and sub - order 
			//only if the incoming order is compatible with the vehicles

			if (memberIsDriver)
			{
				//Cancel current AI Action
				TOS_AI::AbortAIAction(pMemberVehicleAI, -1, "CSquad::ExecuteOrder vehicle need clear current ai action when start executing new order");
				
				//Cancel current sub order
				pMember->ResetOrder(false, true, false);

				if (isMainOrder)
				{
					//Set previous main-order
					SOrderInfo prevOrder;
					pMember->GetOrderInfo(prevOrder, false);
					pMember->SetOrderInfo(prevOrder, true);

					//Set current main-order. 
					//Further information about the order is used by the UpdateOrders() function

					pMember->SetOrderInfo(info, false);

					//Set current execution step for main-order
					pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
				}
			}
			else if(info.type == eSO_FollowLeader || info.type == eSO_ConqBlankAction)
			{
				//Cancel current AI Action
				//TOS_AI::AbortAIAction(pMemberAI, -1, "CSquad::ExecuteOrder vehicle need clear current ai action when start executing new order");

				//Cancel current sub order
				pMember->ResetOrder(false, true, false);

				if (isMainOrder)
				{
					//Set previous main-order
					SOrderInfo prevOrder;
					pMember->GetOrderInfo(prevOrder, false);
					pMember->SetOrderInfo(prevOrder, true);

					//Set current main-order. 
					//Further information about the order is used by the UpdateOrders() function

					pMember->SetOrderInfo(info, false);

					//Set current execution step for main-order
					pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
				}
			}

			if (isSubOrder)
			{
				if (info.type == eSO_SubUseVehicleTurret)
				{
					info.stepActions[step2] = "squad_vehicle_changeseat_gunner";
					info.stepActions[step3] = "squad_vehicle_changeseat_gunner";
				}

				//Set current sub-order.
				pMember->SetSubOrderInfo(info);

				//Set current execution step for sub-order
				pMember->SetStep(EOrderExecutingStep::GotAnOrder, true);

				//CryLogAlways("pMember->SetSubOrderInfo(info);");
			}
			else if (isDebugOrder)
			{
				//Set current sub-order.
				pMember->SetSubOrderInfo(info);

				//Set current execution step for sub-order
				pMember->SetStep(EOrderExecutingStep::GotAnOrder, true);
			}

			ok = true;

			if (g_pGameCVars->sqd_debug_log_executing)
			{
				CryLogAlways("%s[C++][Squad %i][Member %s][Execute Order %s for Vehicle Success]",
					STR_GREEN, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(inputOrderInfo.type));
			}
		}
		else
		{
			//If the AI is in the vehicle 
			//and the incoming order is not compatible with it, 
			//then continue last executed order.

			if (g_pGameCVars->sqd_debug_log_executing)
			{
				CryLogAlways("%s[C++][Squad %i][Member %s][Execute Order %s for Vehicle FAILED][Cause: Order is not compatible with vehicles]",
					STR_RED, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(inputOrderInfo.type));
			}
		}
	}

	if (!ok)
	{
		for (auto pListener : m_listeners)
			pListener->OnOrderExecuteFailed(pMember, inputOrderInfo);
	}
	else
	{
		for (auto pListener : m_listeners)
			pListener->OnOrderExecuted(pMember, inputOrderInfo);
	}

	//if (order == eSO_Guard)
	//{
	//	auto& processedPos = Vec3(ZERO);

	//	//auto& processPos = Member->m_guardPos = pControl->GetCrosshairPos();
	//	if (executeFlags & eEOF_ExecutedByMouse)
	//		processedPos = pMember->GetProcessedPos(true);
	//	//else if (executeFlags & eEOF_ExecutedByMouse)
	//	//	pControl->GetCrosshairPos();

	//	if (memberClass == "Scout")
	//		processedPos.z += SAFE_FLY_HEIGHT;
	//	else if (pMemberVehicle && pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//		processedPos.z += SAFE_FLY_HEIGHT;

	//	if (!pMemberVehicle)
	//	{
	//		if ((executeFlags & eEOF_ExecutedByPlayer))
	//			TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Member need be relaxed before executing GoTo order");
	//	}

	//	pMember->SetGuardPos(processedPos);
	//	pPipeUser->SetRefPointPos(processedPos);

	//	if (pMemberVehicle && memberIsDriver)
	//	{
	//		auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);
	//		if (pAI)
	//			pAI->CastToIPipeUser()->SetRefPointPos(processedPos);
	//	}

	//	if (pMember->m_currentGotoState != eGUS_CleanAI)
	//		pMember->SetGotoUpdateState(eGUS_CleanAI);

	//	const auto attentionThreat = pPipeUser->GetAttentionTargetThreat();
	//	switch (attentionThreat)
	//	{
	//	case AITHREAT_NONE:
	//		pMember->m_aiCleanDuration = 25;//100 = 1 second
	//		break;
	//	case AITHREAT_INTERESTING:
	//		pMember->m_aiCleanDuration = 25;//75
	//		break;
	//	case AITHREAT_THREATENING:
	//		pMember->m_aiCleanDuration = 25;//100
	//		break;
	//	case AITHREAT_AGGRESSIVE:
	//		pMember->m_aiCleanDuration = 25;//300
	//		break;
	//	}

	//	if (!pMemberVehicle)
	//		TOS_AI::DrawPrimaryWeapon(pMemberAI);
	//}
	//else if (order == eSO_SearchEnemy)
	//{
	//	bool inCombat = TOS_AI::IsInCombat(pMemberAI);

	//	if (!TOS_AI::IsCombatEnable(pMemberAI) && !inCombat)
	//		TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member need be alert when he search enemy");

	//	auto pos = (executeFlags & eEOF_ExecutedByMouse) || 
	//		(executeFlags & eEOF_ExecutedByCommander) ?
	//		pMember->GetProcessedPos(true) : pControl->m_crosshairPos;

	//	pMember->SetSearchPos(pos);

	//	if (memberClass == "Scout")
	//		pos.z += SAFE_FLY_HEIGHT;
	//	else if (pMemberVehicle)
	//	{
	//		if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//			pos.z += SAFE_FLY_HEIGHT;
	//	}

	//	if (pMemberVehicle && memberIsDriver)
	//		inCombat = TOS_AI::IsInCombat(pMemberVehicle->GetEntity()->GetAI());

	//	if (!inCombat)
	//	{
	//		if (memberIsDriver && pMemberVehicle && pMemberVehicle->GetEntity()->GetAI())
	//			pPipeUser = pMemberVehicle->GetEntity()->GetAI()->CastToIPipeUser();

	//		pPipeUser->SetRefPointPos(pos);
	//		pPipeUser->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH, false);
	//	}

	//	TOS_AI::DrawPrimaryWeapon(pMemberAI);
	//}
	//else if (order == eSO_FollowLeader)
	//{
	//	if (GetLeader())
	//	{
	//		if (!TOS_AI::IsCombatEnable(pMemberAI))
	//			TOS_AI::EnableCombat(pMemberAI, true, false, "SquadUpd: Member need be in alert when following leader");

	//		auto* pLeaderVehicle = GetLeader()->GetLinkedVehicle();
	//		if (!pLeaderVehicle)
	//		{
	//			if (pMemberVehicle)
	//			{
	//				if (g_pGameCVars->sqd_vehicle_follow_method == 0)
	//				{
	//					pMember->SetCurrentOrder(eSO_ExitVehicle);
	//					pMember->SetProcessedId(pMemberVehicle->GetEntityId());

	//					ExecuteOrder(eSO_ExitVehicle, pMember, eEOF_ExecutedByAI);
	//				}
	//			}

	//		}
	//		else
	//		{
	//			if (!memberIsAlien && !pMemberVehicle)
	//			{
	//				pMember->SetCurrentOrder(eSO_EnterVehicle);
	//				pMember->SetProcessedId(pLeaderVehicle->GetEntityId());

	//				ExecuteOrder(eSO_EnterVehicle, pMember, eEOF_ExecutedByAI);
	//			}
	//		}
	//	}
	//}
	//else if (order == eSO_EnterVehicle)
	//{
	//	if (memberIsAlien)
	//	{
	//		if (pMember->m_previousOrder != eSO_EnterVehicle)
	//			ExecutePreviousOrder(pMember);

	//		return;
	//	}

	//	if (TOS_AI::IsCombatEnable(pMemberAI) && (executeFlags & eEOF_ExecutedByPlayer))
	//		TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Member need be relaxed when player send order to enter the vehicle");

	//	if (executeFlags & eEOF_ExecutedByMouse)
	//	{
	//		auto pProcessedVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pMember->m_processedEntityId);
	//		if (pProcessedVehicle)
	//		{
	//			if (pMemberVehicle)
	//			{
	//				if (pMemberVehicle == pProcessedVehicle)
	//				{
	//					if (pMember->m_previousOrder != eSO_EnterVehicle)
	//					{
	//						pMember->SetGuardPos(pMemberActor->GetEntity()->GetWorldPos(), false);
	//						pMember->SetSearchPos(pMemberActor->GetEntity()->GetWorldPos(), false);
	//						pMember->SetCurrentOrder(eSO_Guard, false);
	//						pMember->SetGotoUpdateState(eGUS_GoTo, false);
	//						//ExecutePreviousOrder(pMember);
	//					}
	//							
	//				}

	//				if (GetLeader())
	//				{
	//					auto* pLeaderVehicle = GetLeader()->GetLinkedVehicle();
	//					if (pLeaderVehicle && pProcessedVehicle == pLeaderVehicle)
	//					{
	//						if (pMember->m_previousOrder != eSO_EnterVehicle)
	//						{
	//							pMember->SetGuardPos(pMemberActor->GetEntity()->GetWorldPos(), false);
	//							pMember->SetSearchPos(pMemberActor->GetEntity()->GetWorldPos(), false);
	//							pMember->SetCurrentOrder(eSO_Guard, false);
	//							pMember->SetGotoUpdateState(eGUS_GoTo, false);
	//							//ExecutePreviousOrder(pMember);
	//						}
	//					}
	//				}
	//			}
	//		}

	//	}
	//}
	//else if (order == eSO_ExitVehicle)
	//{
	//	if (memberIsAlien)
	//	{
	//		pMember->ClearProcessedId();

	//		if (pMember->m_previousOrder != eSO_ExitVehicle)
	//			ExecutePreviousOrder(pMember);

	//		return;
	//	}

	//	//if (TOS_AI::IsCombatEnable(pMemberAI))
	//	//	TOS_AI::EnableCombat(pMemberAI, false);

	//	if (pMemberVehicle)
	//	{
	//		auto pSeat = pMemberVehicle->GetSeatForPassenger(pMember->GetId());
	//		if (pSeat)
	//		{
	//			pSeat->Exit(true, false);
	//		}
	//			
	//		if (m_pSquadSystem->m_isDebugLog)
	//			CryLogAlways("[SSquad %i][ExecuteOrder ExitVehicle][Member %s]", m_squadId, pMemberActor->GetEntity()->GetName());

	//	}
	//	else
	//	{
	//		//if (pMember->GetCurrentOrder() == eSO_EnterVehicle)
	//		//pMember->SetCurrentOrder(pMember->m_previousOrder);

	//		if (pMember->m_previousOrder != eSO_ExitVehicle)
	//			ExecutePreviousOrder(pMember);
	//	}
	//}
	//else if (order == eSO_ShootAt)
	//{
	//	const char* signalName = "TOS_SHOOT_AT";

	//	pMember->m_previousGuardPos = pMember->GetGuardPos();
	//	pMember->m_previousSearchPos = pMember->GetSearchPos();

	//	auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();

	//	auto& processedPos = pMember->GetProcessedPos(false);
	//	pPipeUser->SetRefPointPos(processedPos);

	//	if (pMemberVehicle && memberIsDriver)
	//	{
	//		signalName = "TOS_VEHICLE_SHOOT_AT";
	//		pMemberAI = pMemberVehicle->GetEntity()->GetAI();
	//		pMemberAI->CastToIPipeUser()->SetRefPointPos(processedPos);
	//		//pSignalData->point = processedPos;
	//	}
	//	else
	//	{
	//		if (TOS_AI::IsCombatEnable(pMemberAI))
	//			TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Member need be not in combat before start shooting at player's selected target");

	//		TOS_AI::DrawPrimaryWeapon(pMemberAI);
	//	}

	//	pSignalData->fValue = 7.0f; //how to long shoot
	//	pMemberAI->CastToIAIActor()->SetSignal(SIGNALFILTER_SENDER, signalName, pMemberAI->GetEntity(), pSignalData);
	//}
	//else if (order == eSO_PickupItem)
	//{
	//	if (memberIsAlien)
	//	{
	//		pMember->ClearProcessedId();

	//		if (pMember->m_previousOrder != eSO_PickupItem)
	//			ExecutePreviousOrder(pMember);

	//		return;
	//	}

	//	if (TOS_AI::IsCombatEnable(pMemberAI))
	//		TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Member need be relaxed before pickuping item");

	//	//TOS_AI::OffPerception(pMemberAI, true);

	//	pMember->m_previousGuardPos = pMember->GetGuardPos();
	//	pMember->m_previousSearchPos = pMember->GetSearchPos();

	//	//TOS_AI::OffPerception(pMemberAI, false);
	//}
	//else if (order == eSO_UseVehicleTurret)
	//{
	//	if (TOS_AI::IsCombatEnable(pMemberAI))
	//		TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Member need be relaxed before use vehicle turret");

	//	if (memberIsAlien)
	//	{
	//		pMember->ClearProcessedId();

	//		if (pMember->m_previousOrder != eSO_UseVehicleTurret)
	//			ExecutePreviousOrder(pMember);

	//		return;
	//	}

	//	pMember->m_previousGuardPos = pMember->GetGuardPos();
	//	pMember->m_previousSearchPos = pMember->GetSearchPos();
	//}
	//else if (order == eSO_DisableCombat)
	//{
	//	//TOS_AI::ReturnToFirst(pMemberAI, 0, 0, true);
	//	//TOS_AI::EnablePerception(pMemberAI,false);
	//	TOS_AI::EnableCombat(pMemberAI, false, true, "SquadUpd: Simple disable combat order");
	//}
	//else if (order == eSO_EnableCombat)
	//{
	//	//TOS_AI::EnablePerception(pMemberAI, true);
	//	TOS_AI::EnableCombat(pMemberAI, true, false, "SSquadUpd: Simple enable combat order");
	//}
	//else if (order == eSO_StanceStanding)
	//{
	//	TOS_AI::SelectPipe(pMemberAI, 0, "do_it_standing", "SSquadUpd: Simple Set stance to standing order");
	//}
	//else if (order == eSO_StanceRelaxed)
	//{
	//	TOS_AI::SelectPipe(pMemberAI, 0, "do_it_relaxed", "SSquadUpd: Simple Set stance to relaxed order");
	//}
	//else if (order == eSO_Search_Cover_Around_Point)
	//{
	//		
	//}
	//else if (order == eSO_Conq_GoTo)
	//{
	//	//auto& processPos = Member->m_guardPos = pControl->GetCrosshairPos();
	//	//if (executeFlags & eEOF_ExecutedByMouse)
	//	auto processedPos = pMember->GetProcessedPos(false);
	//	//else if (executeFlags & eEOF_ExecutedByMouse)
	//	//	pControl->GetCrosshairPos();

	//	pMember->SetGuardPos(processedPos);

	//	if (memberClass == "Scout")
	//		processedPos.z += SAFE_FLY_HEIGHT;
	//	else if (pMemberVehicle && pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//		processedPos.z += SAFE_FLY_HEIGHT;

	//	//if ((executeFlags & eEOF_ExecutedByPlayer) && !pMemberVehicle)
	//	//	TOS_AI::EnableCombat(pMemberAI, false, true);

	//	pPipeUser->SetRefPointPos(processedPos);

	//	if (pMemberVehicle && memberIsDriver)
	//	{
	//		auto pAI = TOS_Vehicle::GetAI(pMemberVehicle);
	//		if (pAI)
	//			pAI->CastToIPipeUser()->SetRefPointPos(processedPos);
	//	}

	//	TOS_AI::SelectPipe(pMemberAI, GOALPIPEID_ORDER_CMDR_GOTO, "ord_goto", "SquadUpd: Member executing order Conq_Goto!");

	//	//CryLogAlways("[C++][Member %s][Commander Order GoTo]", pMemberActor->GetEntity()->GetName());

	//	//if (pMember->m_currentGotoState != eGUS_CleanAI)
	//	//	pMember->SetGotoUpdateState(eGUS_CleanAI);

	//	//const auto attentionThreat = pPipeUser->GetAttentionTargetThreat();
	//	//switch (attentionThreat)
	//	//{
	//	//case AITHREAT_NONE:
	//	//	pMember->m_aiCleanDuration = 25;//100 = 1 second
	//	//	break;
	//	//case AITHREAT_INTERESTING:
	//	//	pMember->m_aiCleanDuration = 25;//75
	//	//	break;
	//	//case AITHREAT_THREATENING:
	//	//	pMember->m_aiCleanDuration = 25;//100
	//	//	break;
	//	//case AITHREAT_AGGRESSIVE:
	//	//	pMember->m_aiCleanDuration = 25;//300
	//	//	break;
	//	//}

	//	if (!pMemberVehicle)
	//		TOS_AI::DrawPrimaryWeapon(pMemberAI);
	//}
}

//bool CSquad::ExecuteOrderFG(ESquadOrders order, CMember& member, Vec3& refPoint)
//{
	//IAISystem* pAISys = gEnv->pAISystem;

	//if (!pAISys || !GetActor(member.m_entityId))
	//	return false;

	//CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	//if (pDude)
	//{
	//	CControlClient* pControlCl = pDude->GetControlClient();
	//	if (pControlCl)
	//	{
	//		if (IAIObject* pExecutorAI = GetActor(member.m_entityId)->GetEntity()->GetAI())
	//		{
	//			const string memberClsName = GetActor(member.m_entityId)->GetEntity()->GetClass()->GetName();

	//			IVehicle* pVehicle = GetActor(member.m_entityId)->GetLinkedVehicle();
	//			if (pVehicle)
	//			{
	//				if (pVehicle->GetDriver() == GetActor(member.m_entityId))
	//					pExecutorAI = pVehicle->GetEntity()->GetAI();
	//			}

	//			if (pExecutorAI)
	//			{
	//				const EAITargetThreat targetThreat = pExecutorAI->CastToIPipeUser()->GetAttentionTargetThreat();

	//				if (memberClsName == "Scout")
	//					pExecutorAI->CastToIAIActor()->SetSignal(0, "TO_SCOUTMOAC_IDLE", GetActor(member.m_entityId)->GetEntity());

	//				if (order == eSO_Guard)
	//				{
	//					member.SetCurrentOrder(eSO_Guard);
	//					member.m_guardPos = (refPoint.IsZero() ? pControlCl->m_crosshairPos : refPoint);
	//					//m_membersOrders[member.pActor] = ORD_GOTO;
	//					//m_membersGuardPoses[member.pActor] = (vRefPoint.IsZero() ? pControlCl->m_crosshairPos : vRefPoint);

	//					Vec3& pos = member.m_guardPos;
	//					//Vec3 pos = m_membersGuardPoses[member.pActor];

	//					if (memberClsName == "Scout")
	//						pos.z += SAFE_FLY_HEIGHT;
	//					else if (pVehicle)
	//					{
	//						if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//							pos.z += SAFE_FLY_HEIGHT;
	//					}

	//					if (member.m_currentGotoState != eGUS_CleanAI)
	//						member.SetGotoUpdateState(eGUS_CleanAI);

	//					//if (m_membersGotoState[member.pActor] != STATE_CLEAN_AI)
	//					//	m_membersGotoState[member.pActor] = STATE_CLEAN_AI;

	//					switch (targetThreat)
	//					{
	//					case AITHREAT_NONE:
	//						member.m_aiCleanDuration = 25;//100 = 1 second
	//						break;
	//					case AITHREAT_INTERESTING:
	//						member.m_aiCleanDuration = 75;
	//						break;
	//					case AITHREAT_THREATENING:
	//						member.m_aiCleanDuration = 100;
	//						break;
	//					case AITHREAT_AGGRESSIVE:
	//						member.m_aiCleanDuration = 300;
	//						break;
	//					}
	//					return true;
	//				}
	//				else if (order == eSO_SearchEnemy)
	//				{
	//					member.m_searchPos = refPoint.IsZero() ? pControlCl->m_crosshairPos : refPoint;
	//					//m_membersSearchPoses[member.pActor] = (vRefPoint.IsZero() ? pControlCl->m_crosshairPos : vRefPoint);

	//					Vec3& pos = member.m_searchPos;
	//					//Vec3 pos = m_membersSearchPoses[member.pActor];
	//					const string memberClsName = GetActor(member.m_entityId)->GetEntity()->GetClass()->GetName();

	//					if (memberClsName != "Trooper")
	//					{
	//						if (memberClsName == "Scout")
	//							pos.z += SAFE_FLY_HEIGHT;
	//						else if (pVehicle)
	//						{
	//							if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
	//								pos.z += SAFE_FLY_HEIGHT;
	//						}
	//					}
	//					pExecutorAI->CastToIPipeUser()->SetRefPointPos(pos);
	//					pExecutorAI->CastToIPipeUser()->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH, false);
	//					return true;
	//				}
	//				else if (order == eSO_FollowLeader)
	//				{
	//					if (GetLeader())
	//						pExecutorAI->CastToIPipeUser()->SelectPipe(0, "ord_follow_player", 0, GOALPIPEID_ORDER_FOLLOW, false);

	//					return true;
	//				}
	//			}
	//			member.SetCurrentOrder(order);
	//		}
	//	}
	//}
	//return false;
//}

void CSquad::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_members);
	s->AddContainer(m_selectedMembers);

	auto it = m_members.begin();
	auto end = m_members.end();
	for (; it != end; it++)
		it->GetMemoryStatistics(s);
}

void CSquad::OnActorDeath(IActor* pActor)
{
	if (!GetLeader())
		return;

	if (!pActor)
		return;

	if (IsMember(pActor))
	{
		auto pInstance = GetMemberInstance(pActor);
		if (pInstance)
		{
			//Reset orders when on member death
			pInstance->ResetOrder(true, true, true);
		}
	}

	if (m_pSquadSystem->GetBookedHideSpot(pActor))
		m_pSquadSystem->UnbookHideSpot(pActor, "CSquad::OnActorDeath: unbook hidespot of dead man");

	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (pConqueror)
	{
		const bool isConquest = pConqueror->IsGamemode();
		if (!isConquest)
		{
			RemoveMember(GetMemberInstance(pActor));

			if (IsLeader(pActor))
			{
				if (GetMembersCount() == 0)
				{
					if (m_pSquadSystem->m_isDebugLog)
						CryLogAlways("[C++][Remove Squad %i][Cause: No leader and no members]");

					m_pSquadSystem->RemoveSquad(GetId());
					return;
				}

				if (m_flags & eSCF_NewLeaderWhenOldDead)
				{
					auto pLeaderCandidate = RequestNewLeader(true, false);
					SetLeader(pLeaderCandidate, false);

					if (m_pSquadSystem->m_isDebugLog)
						CryLogAlways("[C++][Squad %i][Appointed a new leader after the death of the previous one]");
				}
			}
		}
		else
		{
			//if (!m_pCommander)
				//return;

			if (IsMember(pActor))
			{
				const bool leaderIsAlive = GetLeader()->GetHealth() > 0;
				const bool membersKilled = GetMemberAlive() != nullptr;

				if (leaderIsAlive && membersKilled)
				{
					if (Random(0.f, 100.f) < 50)
					{
						auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(GetSpecies());
						if (pCommander)
							pCommander->OnSquadCallBackup(this);
					}

					//if (Random(0.f, 100.f) < 50)
						//m_pCommander->OnSquadCallBackup(this);
				}
			}
		}
	}
}

void CSquad::RemoveAllSelected()
{
	m_selectedMembers.clear();
}


void CSquad::OnEnterVehicle(IActor* pActor)
{
	if (!pActor)
		return;

	auto pLeader = GetLeader();

	if (m_pSquadSystem->GetBookedHideSpot(pActor))
		m_pSquadSystem->UnbookHideSpot(pActor, "CSquad::OnEnterVehicle: remove a booked hidespot for a vehicle passenger");

	if (pActor == pLeader)
	{
		for (auto& member : m_members)
		{
			auto* pMemberActor = GetActor(member.m_entityId);
			if (pMemberActor && pMemberActor != pLeader)
			{
				if (pMemberActor->IsAlien())
					continue;

				auto pLeaderVehicle = TOS_Vehicle::GetVehicle(pLeader);

				SOrderInfo order;
				SOrderInfo suborder;
				member.GetOrderInfo(order, false);
				
				if (order.type == eSO_FollowLeader)
				{
					auto pVeh = TOS_Vehicle::GetVehicle(pMemberActor);
					if (pVeh && pVeh == pLeaderVehicle)
						continue;

					const auto step1 = EOrderExecutingStep::GotAnOrder;
					const auto step2 = EOrderExecutingStep::GotoTarget;
					const auto step3 = EOrderExecutingStep::PerformingAction;

					suborder.safeFly = true;
					suborder.targetId = pLeaderVehicle->GetEntityId();
					suborder.targetPos = pLeaderVehicle->GetEntity()->GetWorldPos();
					suborder.type = eSO_SubEnterVehicle;
					suborder.stepActions[step1] = "conqueror_goto_a0_d0_r3";
					suborder.stepActions[step2] = "conqueror_goto_a0_d0_r3";
					suborder.stepActions[step3] = "squad_vehicle_enter_fast";
					suborder.ignoreFlag |= eOICF_IgnoreEnemyAlways;

					ExecuteOrder(&member, suborder, eEOF_ExecutedByAI);
				}
			}
		}

		//m_leadersStats[pActor->GetEntityId()].lastTimeVehicleEnter = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		//m_leadersStats[pActor->GetEntityId()].lastOperatedVehicleId = m_pLeader->GetLinkedVehicle()->GetEntityId();
	}
	else
	{
		for (auto& member : m_members)
		{
			auto pMemberActor = GetActor(member.m_entityId);
			if (!pMemberActor)
				continue;

			SOrderInfo order;
			member.GetOrderInfo(order, false);

			//What should a member of the detachment do when he got into the vehicle?		
			if (pActor == pMemberActor && pMemberActor->GetLinkedVehicle())
			{
				//auto pVehicle = pMemberActor->GetLinkedVehicle();
				//auto movType = pVehicle->GetMovement()->GetMovementType();
				//auto airMoveType = IVehicleMovement::EVehicleMovementType::eVMT_Air;

				//if (movType == airMoveType && (TOS_Vehicle::ActorIsDriver(pMemberActor)))
				//{
				//	auto pVehAI = pVehicle->GetEntity()->GetAI();
				//	if (pVehAI)
				//	{
				//		auto vehiclePos = pVehicle->GetEntity()->GetWorldPos();
				//		vehiclePos.z += 10.f;

				//		auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();
				//		pSignalData->point = vehiclePos;

				//		pVehAI->CastToIAIActor()->SetSignal(SIGNALFILTER_SENDER, "ACT_GOTO", pVehicle->GetEntity(), pSignalData);
				//	}
				//}
				
				//if (pVehicle == m_pLeader->GetLinkedVehicle())
				//	member.ClearProcessedId();

				//member.m_stats.lastTimeVehicleEnter = gEnv->pTimer->GetFrameStartTime().GetSeconds();
				//member.m_stats.lastOperatedVehicleId = pVehicle->GetEntityId();
			}
		}
	}
}

void CSquad::OnExitVehicle(IActor* pActor)
{
	auto pLeader = GetLeader();

	if (pActor == pLeader)
	{
		for (auto& member : m_members)
		{
			auto pMemberActor = GetActor(member.m_entityId);
			if (!pMemberActor)
				continue;

			SOrderInfo order;
			SOrderInfo suborder;
			member.GetOrderInfo(order, false);
			member.GetSubOrderInfo(suborder);

			if (order.type == eSO_FollowLeader)
			{
				auto pVeh = TOS_Vehicle::GetVehicle(pMemberActor);
				if (pVeh)
				{
					SOrderInfo suborder;

					const auto step1 = EOrderExecutingStep::GotAnOrder;
					const auto step2 = EOrderExecutingStep::GotoTarget;
					const auto step3 = EOrderExecutingStep::PerformingAction;

					suborder.targetId = pVeh->GetEntityId();
					suborder.targetPos = pVeh->GetEntity()->GetWorldPos();
					suborder.type = eSO_SubExitVehicle;
					suborder.stepActions[step1] = "squad_vehicle_exit";
					suborder.stepActions[step2] = "squad_vehicle_exit";
					suborder.stepActions[step3] = "squad_vehicle_exit";
					suborder.ignoreFlag |= eOICF_IgnoreEnemyAlways;

					ExecuteOrder(&member, suborder, eEOF_ExecutedByAI);
				}
				else
				{
					if (suborder.type == eSO_SubEnterVehicle || suborder.type == eSO_SubUseVehicleTurret)
					{
						member.ResetOrder(false, true, false);
					}
				}

				//if (pVeh && pVeh == pLeaderVehicle)
				//	continue;

				//auto pUser = pMemberActor->GetEntity()->GetAI();
				//auto pObject = pLeaderVehicle->GetEntity();
				//const char* actionName = "conqueror_vehicle_enter";
				//const int maxAlertness = 120;
				//const int goalPipeId = -1;
				//EAAEFlag flag = eAAEF_DisableCombatDuringAction;
				//const char* solution = "CSquad::OnEnterVehicle: Member must follow his leader anywhere";
				//TOS_AI::ExecuteAIAction(pUser, pObject, actionName, maxAlertness, goalPipeId, flag, solution);
			}

			//if (member.m_entityId != pActor->GetEntityId())
			//{
				//member.m_stats.lastTimeLeaderVehExit= gEnv->pTimer->GetFrameStartTime().GetSeconds();

				//if (member.m_currentOrder == eSO_FollowLeader)
				//{
				//	auto pActor = GetActor(member.m_entityId);
				//	if (pActor)
				//	{
				//		if (pActor->GetLinkedVehicle())
				//		{
				//			member.SetCurrentOrder(eSO_ExitVehicle);
				//			member.SetProcessedId(pActor->GetLinkedVehicle()->GetEntityId());
				//			ExecuteOrder(eSO_ExitVehicle, &member, eEOF_ExecutedByAI);
				//		}
				//	}
				//}
				//else if (member.m_currentOrder == eSO_EnterVehicle)
				//{
				//	if (member.m_previousOrder != eSO_EnterVehicle)
				//		ExecutePreviousOrder(&member);
				//}
			//}
		}

		//m_leadersStats[pActor->GetEntityId()].lastTimeVehicleExit = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	}
	else
	{
		auto pMemberInstance = GetMemberInstance(pActor);
		if (!pMemberInstance)
			return;

		SOrderInfo order;
		pMemberInstance->GetOrderInfo(order, false);
		
		auto pObject = GET_ENTITY(order.targetId);

		//ApplyAIAction(pMemberInstance, pObject, nullptr, order.type, pMemberInstance->GetStep(), "CSquad::OnExitVehicle: Member must continue his order");

		//for (auto& member : m_members)
		//{
		//	if (member.m_entityId != pActor->GetEntityId())
		//		continue;

		//	if (!m_pLeader->GetLinkedVehicle())
		//	{
		//		member.SetCurrentOrder(eSO_FollowLeader);
		//		ExecuteOrder(eSO_FollowLeader, &member, eEOF_ExecutedByAI);
		//		member.m_processedEntityId = 0;
		//	}
		//	else
		//	{
		//		auto& pos = pActor->GetEntity()->GetWorldPos();

		//		member.SetGuardPos(pos);
		//		member.SetCurrentOrder(eSO_Guard);
		//		member.SetGotoUpdateState(eGUS_Guard);
		//		ExecuteOrder(eSO_Guard, &member, eEOF_ExecutedByAI);
		//		member.m_processedEntityId = 0;
		//	}

		//	//member.m_stats.lastTimeVehicleExit = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		//}
	}
}

void CSquad::HandleMouseFSCommand(const char* fscommand)
{
	auto pLeader = GetLeader();
	if (!pLeader)
		return;

	if (!HasClientLeader())
		return;

	const string stringFS = fscommand;

	auto finalOrderEnum = ESquadOrders(0);
	auto orderReceived = false;

	auto it = m_pSquadSystem->m_ordersStringMap.begin();
	auto end = m_pSquadSystem->m_ordersStringMap.end();

	for (;it!= end; it++)
	{
		if (it->second == stringFS)
		{
			finalOrderEnum = it->first;
			orderReceived = true;
			break;
		}
	}

	if (!orderReceived)
		return;

	if (finalOrderEnum == eSO_ScoutGrabMe)
	{
		auto pScout = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(g_pControlSystem->GetSquadSystem()->m_storedMouseId);
		if (pScout && pScout->GetEntity()->GetAI())
		{
			Script::CallMethod(pScout->GetEntity()->GetScriptTable(), "GrabEntityInTentacle", ScriptHandle(pLeader->GetEntityId()));

			for (auto& members : m_members)
			{
				auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(members.GetId());
				if (!pActor)
					continue;

				if (pActor->GetHealth() < 0)
					continue;

				Script::CallMethod(pScout->GetEntity()->GetScriptTable(), "GrabEntityInTentacle", ScriptHandle(pActor->GetEntityId()));
			}

			m_pSquadSystem->SetCommandMode(false);
			const auto pos = pLeader->GetEntity()->GetWorldPos();
			const auto pGoalArea = g_pControlSystem->GetConquerorSystem()->GetNearestStrategicArea(
				pos,
				HOSTILE,
				eAGSF_EnabledAndCapturable,
				eST_Aliens,
				eABF_NoMatter,
				EAreaFlag::ControlPoint);
			
			if (pGoalArea)
			{
				IEntity* pEntity = nullptr;

				EntityId id = pGoalArea->GetBookedUnloadSpot(pScout->GetEntity());
				if (id == 0)
					id = pGoalArea->BookFreeUnloadSpot(pScout->GetEntity());

				if (id)
					pEntity = GET_ENTITY(id);

				const char* desiredGoalName = "";
				const int goalPipeId = -1;
				const char* actionName = "conqueror_goto_and_drop";
				const char* solution = "";
				const auto maxAlertness = 102.0f; //high prioritry
				const auto flag = eAAEF_IgnoreCombatDuringAction;
				
				if (!TOS_AI::IsExecuting(pScout->GetEntity()->GetAI(), actionName))
					TOS_AI::ExecuteAIAction(pScout->GetEntity()->GetAI(), pEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}
		}
	}

	for (auto& selMemberId : m_selectedMembers)
	{
		auto* pMember = GetMemberInstance(selMemberId);
		if (pMember)
		{
			auto pMemberActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->m_entityId));
			if (pMemberActor && pMemberActor->GetHealth() > 0)
			{
				auto noVehicleCount = 0;
				auto inVehicleCount = 0;
				auto isAlienCount = 0;
				auto gunnerCount = 0;
				auto passengerCount = 0;
				auto driverCount = 0;
				auto withWeaponCount = 0;

				auto pLeaderVehicle = TOS_Vehicle::GetVehicle(pLeader);
				auto pMemberVehicle = TOS_Vehicle::GetVehicle(pMemberActor);
				const auto isAlien = pMemberActor->IsAlien();

				const auto isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);
				const auto isGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);
				const auto isPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);
				const auto isHaveWeapons = TOS_Inventory::IsHaveWeapons(pMemberActor);

				bool failExecute = false;

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

				if (isAlien)
				{
					if (finalOrderEnum == eSO_SubEnterVehicle ||
						finalOrderEnum == eSO_SubExitVehicle ||
						finalOrderEnum == eSO_SubUseVehicleTurret || 
						finalOrderEnum == eSO_SubPrimaryPickupItem ||
						finalOrderEnum == eSO_SubSecondaryPickupItem)
						failExecute = true;
				}
				else if (pMemberVehicle)
				{
					if (finalOrderEnum == eSO_SearchCoverAroundPoint ||
						finalOrderEnum == eSO_SubPrimaryPickupItem ||
						finalOrderEnum == eSO_SubSecondaryPickupItem)
						failExecute = true;
				}

				if (failExecute)
					continue;

				SOrderInfo order;
				//pMember->GetOrderInfo(order, false);

				order.type = finalOrderEnum;

				m_pSquadSystem->ClientApplyExecution(pMember, order, eEOF_ExecutedByMouse, 0, Vec3(0), true);

				//pMember->SetProcessedPos(m_pSquadSystem->m_storedMouseWorldPos);
				//pMember->SetProcessedId(m_pSquadSystem->m_storedMouseId);

				//bool isDebugOrder = finalOrderEnum == eSO_DisableCombat ||
				//	finalOrderEnum == eSO_EnableCombat ||
				//	finalOrderEnum == eSO_StanceCrouch ||
				//	finalOrderEnum == eSO_StanceRelaxed ||
				//	finalOrderEnum == eSO_StanceStanding ||
				//	finalOrderEnum == eSO_StanceStealth;

				//if (!isDebugOrder)
					//pMember->SetCurrentOrder(finalOrderEnum);

				//ExecuteOrder(pMember, , eEOF_ExecutedByMouse);
			}			
		}
	}
}

//void CSquad::ExecutePreviousOrder(CMember* pMember)
//{
//	if (pMember)
//	{
//		const bool detachedMember = stl::find(m_detachedMembers, pMember->GetId());
//		if (detachedMember)
//			return;
//
//		pMember->SetGuardPos(pMember->m_previousGuardPos, false);
//		pMember->SetSearchPos(pMember->m_previousSearchPos, false);
//		pMember->SetCurrentOrder(pMember->m_previousOrder, false);
//		pMember->SetGotoUpdateState(pMember->m_previousGotoState, false);
//		//ExecuteOrder(pMember->m_previousOrder, pMember, eEOF_ExecutedByAI);
//	}
//}

bool CSquad::IsLeader(const IActor* pActor) const noexcept
{
	const auto pLeader = GetLeader();

	return pLeader && pLeader == pActor;
}

IActor* CSquad::RequestNewLeader(bool getFromMembers, bool getFromOldConqLeader)
{
	if (getFromMembers)
	{
		auto pAliveMember = GetMemberAlive();

		if (pAliveMember)
		{
			const auto entId = pAliveMember->GetId();
			auto pActor = GetActor(entId);

			if (pActor)
			{
				if (m_pSquadSystem->m_isDebugLog)
					CryLogAlways("[C++][Squad %i][Request New Leader %s]", pActor->GetEntity()->GetName());

				return pActor;
			}		
		}
	}
	
	if(getFromOldConqLeader)
	{
		auto pOldLeader = m_pConquerorOldAILeader;
		m_pConquerorOldAILeader = 0;

		return pOldLeader;
	}

	if (m_pSquadSystem->m_isDebugLog)
		CryLogAlways("[C++][Squad %i][FALIED Request New Leader]");

	return nullptr;
}

//void CSquad::ExecuteOrderAllMembers(ESquadOrders order, uint executeFlags, EntityId processedId /*= 0*/, Vec3& processedPos /*= Vec3(0,0,0)*/, const float radius /*= 0*/)
//{
//	SetLastAllOrder(order);
//
//	for (auto& member : m_members)
//	{
//		if (processedId != 0)
//			member.SetProcessedId(processedId);
//
//		if (!processedPos.IsZero())
//			member.SetProcessedPos(processedPos);
//
//		if (radius != 0)
//			member.m_searchCoverRadius = radius;
//
//		ExecuteOrder(order, &member, executeFlags);
//	}
//}

bool CSquad::VehicleIsUsedByMember(EntityId vehicleId)
{
	if (!vehicleId)
		return false;

	for (auto& member : m_members)
	{
		auto pActor = GetActor(member.GetId());
		if (pActor)
		{
			auto pVehicle = TOS_Vehicle::GetVehicle(pActor);
			if (pVehicle)
			{
				pVehicle->GetEntityId() == vehicleId;
				return true;
			}
		}
	}

	return false;
	
}

float CSquad::GetAverageDistanceToMembers(const Vec3& startPos)
{
	int count = m_members.size();

	if (count == 0)
		return 0;

	float distSum = 0;

	for (auto& member : m_members)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(member.m_entityId);
		if (!pEntity)
			continue;

		const auto& memberPos = pEntity->GetWorldPos();
		const float distance = (startPos - memberPos).GetLength();

		distSum += distance;
	}

	return distSum / count;
}

int CSquad::GetAliveMembersCount()
{
	int count = 0;

	for (auto& member : m_members)
	{
		auto pActor = GetActor(member.GetId());
		if (!pActor)
			continue;

		if (!pActor->GetEntity()->IsHidden() && pActor->GetHealth() > 0)
			count++;
	}

	return count;
}

//void CSquad::ExecuteOrderEx(ESquadOrders order, CMember* Member, uint executeFlags, EntityId processedId, Vec3& processedPos, const float radius)
//{
//	if (!Member)
//		return;
//
//	const bool detachedOrder = stl::find(m_pSquadSystem->m_allowedDetachedOrders, order);
//
//	if (IsMemberDetached(Member))
//	{
//		if (!detachedOrder)
//		{
//			return;
//		}
//	}
//		
//
//	//if (executeFlags & eEOF_ExecutedByCommander)
//	//	SetLastCommanderOrder(order);
//
//	if (processedId != 0)
//		Member->SetProcessedId(processedId);
//
//	if (!processedPos.IsZero())
//		Member->SetProcessedPos(processedPos);
//
//	if (radius != 0)
//		Member->m_searchCoverRadius = radius;
//
//	ExecuteOrder(order, Member, executeFlags);
//}

bool CSquad::IsMemberDetached(const IActor* pActor)
{
	if (!pActor)
		return false;

	if (!IsMember(pActor))
		return false;

	return stl::find(m_detachedMembers, pActor->GetEntityId());
}

bool CSquad::IsMemberDetached(const EntityId id)
{
	auto pMember = GetMemberInstance(id);
	if (!pMember)
		return false;

	return stl::find(m_detachedMembers, pMember->GetId());
}

bool CSquad::IsMemberDetached(const int index)
{
	auto pMember = GetMemberFromIndex(index);
	if (!pMember)
		return false;

	return stl::find(m_detachedMembers, pMember->GetId());
}

void CSquad::UpdateConquerorDetachedVehicle(float frametime, IVehicle* pVehicle, const SDetachedMemberData& data)
{
	if (!pVehicle)
		return;

	const auto pVehAI = TOS_Vehicle::GetAI(pVehicle);
	if (!pVehAI)
		return;

	const auto vehIsCombat = TOS_AI::IsInCombat(pVehAI);
	const bool vehIsAir = TOS_Vehicle::IsAir(pVehicle);
	const bool vehIsLand = TOS_Vehicle::IsLand(pVehicle);
	const bool vehIsSea = TOS_Vehicle::IsSea(pVehicle);

	auto pGoalEntity = GET_ENTITY(data.targetId);
	const auto goalPathName = data.pathName;
	const auto routineType = data.routineType;

	if (goalPathName.size())
		pGoalEntity = gEnv->pEntitySystem->FindEntityByName(goalPathName);

	if (!pGoalEntity)
		return;

	const char* actionName = "nullDetachedAction";
	const int goalPipeId = -1;
	const int maxAlertness = 120;
	EAAEFlag flag = eAAEF_JoinCombatPauseAction;
	string cause = "CSquad::UpdateConquerorDetachedVehicle: ";

	switch (routineType)
	{
	default:
		break;
	case eDRT_OnFootPathPatrol:
		break;
	case eDRT_LandPathPatrol:
	{
		actionName = "detached_land_follow_path_1";
		cause += string("eDRT_LandPathPatrol: start following path");

		if (!TOS_AI::IsExecuting(pVehAI, actionName))
			TOS_AI::ExecuteAIAction(pVehAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, 0, cause.c_str());
	}
	break;
	case eDRT_LandPointGuard:
		break;
	case eDRT_AirPathPatrol:
		break;
	case eDRT_AirPointSearch:
	{
		//if (TOS_Distance::IsBigger(pVehicle, pGoalEntity, 5.0f) && (!TOS_AI::IsExecuting(pVehAI, "squad_search_enemy")))
		//{
		//	actionName = "detached_goto_a0_d0_r3";
		//	cause += string("eDRT_AirPointSearch: goto search point");

		//	if (!TOS_AI::IsExecuting(pVehAI, actionName))
		//		TOS_AI::ExecuteAIAction(pVehAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
		//}
		//else
		{
			actionName = "squad_search_enemy";
			cause += string("eDRT_AirPointSearch: start search enemy");

			if (!TOS_AI::IsExecuting(pVehAI, actionName))
				TOS_AI::ExecuteAIAction(pVehAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, 0, cause.c_str());
		}
	}
	break;
	case eDRT_WaterPathPatrol:
		break;
	case eDRT_WaterPointPatrol:
		break;
	}
}

//CLeaderInstance* CSquad::GetLeaderInstance()
//{
//	return &m_leaderInstance;
//}

void CSquad::SetConqIterationTime(float seconds)
{
	m_lastConqIterationTime = seconds;
}

float CSquad::GetConqIterationTime() const
{
	return m_lastConqIterationTime;
}

void CSquad::UpdateConquerorDetached(float frametime)
{
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	auto pRARSystem = pConqueror->GetRAR();
	if (!pRARSystem)
		return;

	for (auto id : m_detachedMembers)
	{
		const auto pLeader = GetLeader();
		const bool isLeader = pLeader && pLeader->GetEntityId() == id;

		auto pMemberInstance = GetMemberInstance(id);
		if (!(pMemberInstance || isLeader))
			continue;

		SDetachedMemberData data;

		if (isLeader)
		{
			data = m_detachedLeadersData[id];
			//m_leaderInstance.GetDetachedData(data);
		}
		else
			pMemberInstance->GetDetachedData(data);

		const float delay = g_pGameCVars->sqd_detachedUpdateDelay;//seconds

		if (currentTime - data.lastDetachedUpdateTime < delay)
			continue;

		data.lastDetachedUpdateTime = currentTime;

		if (!isLeader)
			pMemberInstance->SetDetachedData(data);
		else
			m_detachedLeadersData[id] = data;

		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
		if (!pActor)
			continue;

		const auto isAlien = pActor->IsAlien();
		const auto isPlayer = pActor->IsPlayer();

		if (!data.enableUpdate || pRARSystem->IsAssignedExecutor(id) || isPlayer)
			continue;

		//CryLogAlways("[C++][line 2746][Actor %s][Health %i]",
		//	pActor->GetEntity()->GetName(), pActor->GetHealth());

		if (pActor->GetHealth() <= 0)
			continue;

		if (pActor->GetEntity()->IsHidden())
			continue;

		const auto isActorDriver = TOS_Vehicle::ActorIsDriver(pActor);
		const auto pVeh = TOS_Vehicle::GetVehicle(pActor);

		if (isAlien)
		{
			UpdateConquerorDetachedAlien(frametime, pActor, data);
		}
		else
		{
			if (isActorDriver)
			{
				UpdateConquerorDetachedVehicle(frametime, pVeh, data);
			}
			else
			{
				//UpdateConquerorDetachedHuman(frametime, pActor, data);
			}
		}

		//auto pActorAI = pActor->GetEntity()->GetAI();
		//if (!pActorAI)
		//	continue;

		//auto pActorPipeUser = pActorAI->CastToIPipeUser();
		//if (!pActorPipeUser)
		//	continue;

		//auto pActorAIProxy = pActorAI->GetProxy();
		//if (!pActorAIProxy)
		//	continue;

		const string className = pActor->GetEntity()->GetClass()->GetName();
		const auto isAlienFlyable = (className == "Drone" || className == "Scout" || className == "Alien");
		const auto isAlienOnFoot = (className == "Pinger" || className == "Hunter" || className == "Trooper");

		//const auto alertness = pActorAIProxy->GetAlertnessState();
		//const auto isCombat = TOS_AI::IsInCombat(pActorAI);
		//const auto isNotIdle = !isCombat && alertness > 0;



		//if (pVeh)
		//{
		//	if (TOS_AI::IsInCombat(TOS_Vehicle::GetAI(pVeh)))
		//		continue;
		//}
		//else
		//{
		//	if (isCombat)
		//		continue;
		//}

		//const auto goalPosition = (data.points.size() == 0) ? 
		//pActor->GetEntity()->GetWorldPos() : data.points[0];

		//goalPosition = goalPosition == Vec3(0) ? goalPosition : pActor->GetEntity()->GetWorldPos();


		if (isAlien)
		{

		}

		auto pGoalEntity = GET_ENTITY(data.targetId);

		const auto& targetPath = data.pathName;
		const auto routineType = data.routineType;

		pGoalEntity = gEnv->pEntitySystem->FindEntityByName(targetPath.c_str());

		//Scout infinity patrol fix
	//	if (className == "Scout")
	//	{
	//		if (isNotIdle)
	//			gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "TO_SCOUTMOAC_IDLE", pActorAI);
	//	}

	//	string truePipeName = "";
	//	bool breakConditions = false;

	//	switch (routineType)
	//	{
	//	case eDRT_OnFootPathPatrol:
	//	{
	//		breakConditions = 
	//			(isActorDriver) || 
	//			(isCombat) || 
	//			(isAlien && !isAlienOnFoot);

	//		if (breakConditions)
	//			break;

	//		truePipeName = "DETACHED_FOLLOWPATH";
	//		const bool notProperPipeName = !TOS_AI::IsUsingPipe(pActorAI, truePipeName);

	//		if (notProperPipeName)
	//		{
	//			auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();

	//			//path entity id
	//			pSignalData->iValue = pPathEntity ? pPathEntity->GetId() : 0;

	//			//start nearest
	//			pSignalData->point.z = 1;

	//			//loops -1 forever
	//			pSignalData->fValue = -1;

	//			gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "DETACHED_FOLLOWPATH", pActorAI, pSignalData);
	//		}

	//		break;
	//	}
	//	case eDRT_AirPathPatrol:
	//	{
	//		breakConditions = 
	//			(isCombat) ||
	//			(isActorDriver && !vehIsAir) ||
	//			(isAlien && !isAlienFlyable);

	//		if (breakConditions)
	//			break;

	//		truePipeName = "DETACHED_FOLLOWPATH";
	//		const bool notProperPipeName = !TOS_AI::IsUsingPipe(pActorAI, truePipeName);

	//		if (notProperPipeName)
	//		{
	//			auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();

	//			//path entity id
	//			pSignalData->iValue = pPathEntity ? pPathEntity->GetId() : 0;

	//			//start nearest
	//			pSignalData->point.z = 1;

	//			//loops -1 forever
	//			pSignalData->fValue = -1;

	//			gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "DETACHED_FOLLOWPATH", pActorAI, pSignalData);
	//		}

	//		break;
	//	}
	//	case eDRT_AirPointSearch:
	//	{
	//		breakConditions =
	//			(isCombat) ||
	//			(isActorDriver && !vehIsAir) ||
	//			(isAlien && !isAlienFlyable);

	//		//CryLogAlways("[C++][line 2878][Actor %s][Health %i]",
	//		//	pActor->GetEntity()->GetName(), pActor->GetHealth());

	//		if (breakConditions)
	//			break;

	//		truePipeName = "tos_detached_search_enemy";
	//		const bool isProperPipeName = TOS_AI::IsUsingPipe(pVeh ? TOS_Vehicle::GetAI(pVeh) : pActorAI, truePipeName);

	//		if (isProperPipeName)
	//		{
	//			const auto searchRadius = 100.0f;
	//			auto searchPosition = goalPosition;

	//			if (className == "Scout")
	//			{
	//				auto pScout = (CScout*)pActor;
	//				if (!pScout->m_searchbeam.isActive)
	//				{
	//					pScout->EnableSearchBeam(true);

	//					const auto scoutPos = pScout->GetEntity()->GetWorldPos();
	//					const auto scoutPos2 = Vec3(scoutPos.x, scoutPos.y + 3.0f, scoutPos.z - 4.0f);
	//					const auto scoutBeamDir = (scoutPos2 - scoutPos).GetNormalized();

	//					pScout->SetSearchBeamGoal(scoutBeamDir);
	//				}
	//			}

	//			searchPosition.x += Random(-searchRadius, searchRadius);
	//			searchPosition.y += Random(-searchRadius, searchRadius);

	//			//DONT CHANGE, provide stable calculations
	//			searchPosition.z = 2000.f;

	//			ray_hit hit;
	//			IPhysicalEntity* pSkipEntities[10];
	//			int nSkip = 0;
	//			auto pItem = pActor->GetCurrentItem();
	//			if (pItem)
	//			{
	//				auto pWeapon = (CWeapon*)pItem->GetIWeapon();
	//				if (pWeapon)
	//					nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
	//			}

	//			auto castPos = searchPosition;
	//			castPos.z -= 1.f;

	//			const Vec3 castDir = (castPos - searchPosition).GetNormalizedSafe() * 2000.f;
	//			const int objTypes = ent_terrain | ent_static;
	//			const uint flags = rwi_ignore_noncolliding | rwi_stop_at_pierceable;

	//			if (gEnv->pPhysicalWorld->RayWorldIntersection(searchPosition, castDir, objTypes,
	//				flags, &hit, 1, pSkipEntities, nSkip))
	//			{
	//				if (hit.pCollider)
	//				{
	//					if (hit.dist < SAFE_FLY_HEIGHT)
	//					{
	//						searchPosition.z += SAFE_FLY_HEIGHT - hit.dist;
	//					}
	//					else if (hit.dist > SAFE_FLY_HEIGHT)
	//					{
	//						searchPosition.z -= hit.dist - SAFE_FLY_HEIGHT;
	//					}
	//				}
	//			}

	//			if (!isActorDriver)
	//			{
	//				TOS_AI::SetRefPoint(pActorAI, searchPosition);
	//			}
	//			else
	//			{
	//				auto pVehAI = TOS_Vehicle::GetAI(pVeh);
	//				TOS_AI::SetRefPoint(pVehAI, searchPosition);
	//			}
	//		}
	//		else
	//		{
	//			if (!isActorDriver)
	//			{
	//				TOS_AI::SelectPipe(pActorAI, 0, truePipeName, "DetachedUpd: Actor need do it his schedule");
	//			}
	//			else
	//			{
	//				auto pVehAI = TOS_Vehicle::GetAI(pVeh);
	//				TOS_AI::SelectPipe(pVehAI, 0, truePipeName, "DetachedUpd: Vehicle need do it his schedule");
	//			}
	//		}

	//		break;
	//	}
	//	case eDRT_WaterPathPatrol:
	//	{
	//		breakConditions =
	//			(isCombat) ||
	//			(isAlien) || 
	//			(isActorDriver && !vehIsSea);

	//		if (breakConditions)
	//			break;

	//		truePipeName = "DETACHED_FOLLOWPATH";
	//		const bool notProperPipeName = !TOS_AI::IsUsingPipe(pActorAI, truePipeName);

	//		if (notProperPipeName)
	//		{
	//			auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();

	//			//path entity id
	//			pSignalData->iValue = pPathEntity ? pPathEntity->GetId() : 0;

	//			//start nearest
	//			pSignalData->point.z = 1;

	//			//loops -1 forever
	//			pSignalData->fValue = -1;

	//			gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "DETACHED_FOLLOWPATH", pActorAI, pSignalData);
	//		}
	//		break;
	//	}
	//	case eDRT_LandPointGuard:
	//	{
	//		breakConditions =
	//			(isCombat) ||
	//			(isAlien) ||
	//			(isActorDriver && !vehIsLand);

	//		if (breakConditions)
	//			break;

	//		if (isActorDriver && pVeh)
	//		{
	//			auto pVehAI = pVeh->GetEntity()->GetAI();
	//			if (!pVehAI)
	//				continue;

	//			auto pVehPipe = pVehAI->CastToIPipeUser();
	//			if (!pVehPipe)
	//				continue;

	//			auto pVehAIProxy = pVehAI->GetProxy();
	//			if (!pVehAIProxy)
	//				continue;

	//			const auto vehPos = pVeh->GetEntity()->GetWorldPos();

	//			const float goalDist = (goalPosition - vehPos).GetLength();
	//			const auto needToMove = goalDist > 3.0f;
	//			const auto needToGuard = !needToMove;

	//			bool notTruePos = false;
	//			bool notTruePipeName = false;

	//			if (needToMove)
	//			{
	//				truePipeName = "tos_detached_goto";

	//				notTruePos = TOS_AI::GetRefPoint(pVehAI) != goalPosition;
	//				notTruePipeName = !TOS_AI::IsUsingPipe(pVehAI, truePipeName);

	//				if (notTruePos)
	//					TOS_AI::SetRefPoint(pVehAI, goalPosition);

	//				if (notTruePipeName)
	//					TOS_AI::SelectPipe(pVehAI, 0, truePipeName, "DetachedUpd: Vehicle need moving to target pos when do it his schedule");
	//			}
	//			else if (needToGuard)
	//			{
	//				truePipeName = "tos_detached_land_point_guard";

	//				notTruePos = TOS_AI::GetRefPoint(pVehAI) != goalPosition;
	//				notTruePipeName = !TOS_AI::IsUsingPipe(pVehAI, truePipeName);

	//				if (notTruePos)
	//					TOS_AI::SetRefPoint(pVehAI, goalPosition);

	//				if (notTruePipeName)
	//					TOS_AI::SelectPipe(pVehAI, 0, truePipeName, "DetachedUpd: Vehicle need guard the target pos when do it his schedule");
	//			}
	//		}
	//		else
	//		{
	//			const auto actorPos = pActor->GetEntity()->GetWorldPos();

	//			const float goalDist = (goalPosition - actorPos).GetLength();
	//			const auto needToMove = goalDist > 1.0f;
	//			const auto needToGuard = !needToMove;

	//			bool notTruePos = false;
	//			bool notTruePipeName = false;

	//			if (TOS_Vehicle::ActorInVehicle(pActor))
	//			{
	//				TOS_Vehicle::Exit(pActor, true, false);
	//				break;
	//			}

	//			if (needToMove)
	//			{
	//				truePipeName = "tos_detached_goto";

	//				notTruePos = TOS_AI::GetRefPoint(pActorAI) != goalPosition;
	//				notTruePipeName = !TOS_AI::IsUsingPipe(pActorAI, truePipeName);

	//				if (notTruePos)
	//					TOS_AI::SetRefPoint(pActorAI, goalPosition);

	//				if (notTruePipeName)
	//					TOS_AI::SelectPipe(pActorAI, 0, truePipeName, "DetachedUpd: Actor need moving to target pos when do it his schedule");
	//			}
	//			else if (needToGuard)
	//			{
	//				truePipeName = "tos_detached_land_point_guard";

	//				notTruePos = TOS_AI::GetRefPoint(pActorAI) != goalPosition;
	//				notTruePipeName = !TOS_AI::IsUsingPipe(pActorAI, truePipeName);

	//				if (notTruePos)
	//					TOS_AI::SetRefPoint(pActorAI, goalPosition);

	//				if (notTruePipeName)
	//					TOS_AI::SelectPipe(pActorAI, 0, truePipeName, "DetachedUpd: Actor need guard the target pos when do it his schedule");
	//			}

	//		}

	//		break;
	//	}
	//	}
	}
}

void CSquad::MarkDetached(EntityId id, const SDetachedMemberData& data)
{
	auto pLeader = GetLeader();

	if (IsMember(id))
	{
		GetMemberInstance(id)->SetDetachedData(data);
	}
	else if (pLeader && pLeader->GetEntityId() == id)
	{
		m_detachedLeadersData[id] = data;
		//m_leaderInstance.SetDetachedData(data);
	}
	else
		return;

	if (g_pGameCVars->sqd_debug_log_detached)
	{
		CryLogAlways("%s[C++][Squad %i][Mark Entity Detached %i]", STR_YELLOW, GetId(), id);
	}


	stl::push_back_unique(m_detachedMembers, id);

	auto pVehicle = TOS_Vehicle::GetVehicle(pLeader);
	if (pVehicle)
		g_pControlSystem->GetConquerorSystem()->UnbookUnloadSpot(pVehicle->GetEntity());

	auto pRARSystem = g_pControlSystem->GetConquerorSystem()->GetRAR();
	if (pRARSystem)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
		if (pActor && pActor->IsAlien())
		{
			const string className = pActor->GetEntity()->GetClass()->GetName();
			if (className == "Scout")
			{
				std::vector<ERequestType> types;
				types.push_back(eRT_AlienTaxsee);

				pRARSystem->AddExecutor(GetSpecies(), id, types);
			}
			else if (className == "Trooper")
			{
				//Not Imp
			}
			else if (className == "Hunter")
			{
				//Not Imp
			}
			else if (className == "Alien")
			{
				//Not Imp
			}
		}
	}
}

void CSquad::MarkUndetached(EntityId id)
{
	if (IsMember(id))
	{
		GetMemberInstance(id)->SetDetachedData(SDetachedMemberData());
	}
	else if (GetLeader() && GetLeader()->GetEntityId() == id)
	{
		//m_leaderInstance.SetDetachedData(SDetachedMemberData());
		m_detachedLeadersData[id] = SDetachedMemberData();
	}

	//CryLogAlways("%s[C++][Squad %i][Mark Entity Undetached %i]", STR_YELLOW, GetId(), id);

	stl::find_and_erase(m_detachedMembers, id);
}

bool CSquad::IsMemberDetached(const CMember* Member)
{
	if (!Member)
		return false;

	return stl::find(m_detachedMembers, Member->GetId());
}

//void CSquad::SetLastAllOrder(ESquadOrders order)
//{
//	m_lastAllOrder = order;
//
//	for (auto& member : m_members)
//		member.m_stats.lastTimeAllOrder = gEnv->pTimer->GetFrameStartTime().GetSeconds();
//
//	if (m_pLeader)
//		m_leadersStats[m_pLeader->GetEntityId()].lastTimeAllOrder = gEnv->pTimer->GetFrameStartTime().GetSeconds();
//}

void CSquad::UpdateStats(float frametime)
{
	std::vector<EntityId> members;
	GetMembersInCombat(members);

	const bool membersInCombat = members.size() > 0;
	const bool leaderInCombat = IsLeaderInCombat();

	if (membersInCombat || leaderInCombat)
	{
		if (!IsInCombat())
		{
			OnStartCombat();
		}
	}
	else
	{
		if (IsInCombat())
		{
			OnFinishCombat();
		}
	}

	for (auto& member : m_members)
	{
		auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
		if (!pActor)
			continue;

		if (pActor->GetHealth() > 0)
		{
			member.m_stats.deathTimer = 0;
			member.m_stats.aliveTimer += frametime;
		}
		else
		{
			member.m_stats.deathTimer += frametime;
			member.m_stats.aliveTimer = 0;
		}
	}
}

void CSquad::Hide(bool hide)
{
	if (m_members.size() > 0)
		m_hided = hide;
	else
		m_hided = false;

	for (auto& member : m_members)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(member.GetId());
		if (!pEntity)
			continue;

		pEntity->Hide(hide);
	}
}

Vec3& CSquad::GetAveragePos(bool includeLeader)
{
	//if (!m_pLeader && includeLeader)
	//	return Vec3(0, 0, 0);

	auto pLeader = GetLeader();

	const int leaderUnit = pLeader && includeLeader ? 1 : 0;
	const int count = m_members.size() + leaderUnit;

	if (count == 0)
		return Vec3(0, 0, 0);

	Vec3& vecSum = pLeader && includeLeader ? pLeader->GetEntity()->GetWorldPos() : Vec3(0, 0, 0);

	for (auto& member : m_members)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(member.m_entityId);
		if (!pEntity)
			continue;

		const auto& memberPos = pEntity->GetWorldPos();

		vecSum += memberPos;
	}

	return vecSum / count;
}

bool CSquad::IsLeaderDetached() const noexcept
{
	auto pLeader = GetLeader();

	if (!pLeader)
		return false;

	return stl::find(m_detachedMembers, pLeader->GetEntityId());
}

int CSquad::GetMembersCount(IVehicle* pVehicle)
{
	int count = 0;

	if (!pVehicle)
		return count;

	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor)
			continue;

		if (TOS_Vehicle::GetVehicle(pActor) == pVehicle)
			count++;
	}

	return count;
}

//const SMemberStats& CSquad::GetLeaderStats()
//{
	//if (m_pLeader)
		//return m_leadersStats[m_pLeader->GetEntityId()];

//	return SMemberStats();
//}

void CSquad::GetMembersNotInVehicle(const IVehicle* pVehicle, std::vector<EntityId>& members)
{
	if (!pVehicle)
		return;

	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor || (pActor && (pActor->GetHealth() < 0 || pActor->GetEntity()->IsHidden() || pActor->GetActorStats()->isGrabbed)))
			continue;

		if (TOS_Vehicle::GetVehicle(pActor) != pVehicle)
			members.push_back(pActor->GetEntityId());
	}
}

ESquadOrders CSquad::GetLastAllOrder() const
{
	return m_lastAllOrder;
}

void CSquad::GetMembersInVehicle(const IVehicle* pVehicle, std::vector<EntityId>& members)
{
	if (!pVehicle)
		return;

	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor || (pActor && (pActor->GetHealth() < 0 || pActor->GetEntity()->IsHidden() || pActor->GetActorStats()->isGrabbed)))
			continue;

		if (TOS_Vehicle::GetVehicle(pActor) == pVehicle)
			members.push_back(pActor->GetEntityId());
	}
}

bool CSquad::IsInCombat()
{
	return m_inCombat;
}

void CSquad::GetMembersInCombat(std::vector<EntityId>& members)
{
	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor || (pActor && (pActor->GetHealth() < 0 || pActor->GetEntity()->IsHidden() || pActor->GetActorStats()->isGrabbed)))
			continue;

		if (TOS_AI::IsInCombat(pActor->GetEntity()->GetAI()))
			members.push_back(pActor->GetEntityId());
	}
}

bool CSquad::IsLeaderInCombat() const
{
	auto pLeader = GetLeader();

	if (!pLeader)
		return false;

	auto pVehicle = TOS_Vehicle::GetVehicle(pLeader);
	if (pVehicle)
	{
		if (!pVehicle->IsDestroyed())
			return TOS_AI::IsInCombat(TOS_Vehicle::GetAI(pVehicle));
	}
	else
	{
		return pLeader->GetHealth() > 0 && TOS_AI::IsInCombat(pLeader->GetEntity()->GetAI());
	}

	return false; 
}

void CSquad::OnStartCombat()
{
	m_lastTimeStartCombat = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	m_inCombat = true;

//	CryLogAlways("%s[C++][Squad %i Start Combat]", STR_CYAN, GetId());

	auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(GetSpecies());
	if (pCommander)
		pCommander->OnSquadStartCombat(this);

	//if (m_pCommander)
		//m_pCommander->OnSquadStartCombat(this);
}

void CSquad::OnFinishCombat()
{
	m_lastTimeFinishCombat = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	m_inCombat = false;

	//CryLogAlways("%s[C++][Squad %i Finish Combat]", STR_CYAN, GetId());

	auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(GetSpecies());
	if (pCommander)
		pCommander->OnSquadFinishCombat(this);

	//if (m_pCommander)
		//m_pCommander->OnSquadFinishCombat(this);
}

void CSquad::GetMembersOnFoot(std::vector<EntityId>& members)
{
	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor || (pActor && (pActor->GetHealth() < 0 || pActor->GetEntity()->IsHidden() || pActor->GetActorStats()->isGrabbed)))
			continue;

		if (!TOS_Vehicle::ActorInVehicle(pActor))
			members.push_back(pActor->GetEntityId());
	}
}

void CSquad::GetMembersInRadius(std::vector<EntityId>& members, Vec3 pos, float radius)
{
	if (radius == 0)
		return;

	for (auto& member : m_members)
	{
		auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
		if (!pActor || (pActor && (pActor->GetHealth() < 0 || pActor->GetEntity()->IsHidden() || pActor->GetActorStats()->isGrabbed)))
			continue;

		const auto memberPos = pActor->GetEntity()->GetWorldPos();
		const auto dist = (pos - memberPos).GetLength();

		if (dist < radius)
			members.push_back(pActor->GetEntityId());
	}
}

void CSquad::OnActorGrabbed(IActor* pActor, EntityId grabbedId)
{
	auto pInstance = GetMemberInstance(pActor);
	if (!pInstance)
		return;

	if (!TOS_AI::IsUsingPipe(pActor, "do_nothing"))
		TOS_AI::SelectPipe(pActor, 0, "do_nothing", "SquadUpd: Grabbed actor need do nothing");
}

void CSquad::OnActorDropped(IActor* pActor, EntityId grabbedId)
{

}

#define SUB_ORDER_RETURN(pMember) \
	pMember->SetSubOrderInfo(SOrderInfo());\
	pMember->OnSubOrderFinish();\
	return

#define DEFINE_FOOT_SUB_ORDER(pMember)\
	if (!pMember)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->GetId());\
	if (!pMemberActor)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	const auto isLeader = pMemberActor == GetLeader();\
	if (isLeader)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	const auto isDead = pMemberActor->GetHealth() <= 0;\
	if (isDead)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	auto pMemberAI = pMemberActor->GetEntity()->GetAI();\
	if (!pMemberAI)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	auto pMemberPipe = pMemberAI->CastToIPipeUser();\
	if (!pMemberPipe)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	SOrderInfo subOrderInfo;\
	pMember->GetSubOrderInfo(subOrderInfo);\
	\
	const auto isDebug = subOrderInfo.type >= eSO_DebugEnableCombat && subOrderInfo.type <= eSO_DebugStanceCrouch;\
	if (isDebug)\
	{\
		UpdateDebugOrder(frametime, pMember);\
		return;\
	}\
	string leaderClass = GetLeader()->GetEntity()->GetClass()->GetName();\
	const auto step = pMember->GetSubStep()\

#define DEFINE_VEHICLE_SUB_ORDER(pMember)\
	if (!pMember)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->GetId());\
	if (!pMemberActor)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	const string leaderClass = GetLeader()->GetEntity()->GetClass()->GetName();\
	\
	auto pMemberVeh = TOS_Vehicle::GetVehicle(pMemberActor);\
	if (!pMemberVeh)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	const bool isGunner = TOS_Vehicle::ActorIsGunner(pMemberActor);\
	const bool isDriver = TOS_Vehicle::ActorIsDriver(pMemberActor);\
	const bool isPassenger = TOS_Vehicle::ActorIsPassenger(pMemberActor);\
	const auto seatWeaponCount = TOS_Vehicle::GetSeatWeaponCount(pMemberActor);\
	\
	const auto isLeader = pMemberActor == GetLeader();\
	if (isLeader)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	const auto isDead = pMemberActor->GetHealth() <= 0;\
	if (isDead)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	auto pMemberVehAI = pMemberVeh->GetEntity()->GetAI();\
	if (!pMemberVehAI)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	auto pMemberAI = pMemberActor->GetEntity()->GetAI();\
	if (!pMemberAI)\
	{\
		SUB_ORDER_RETURN(pMember);\
	}\
	\
	SOrderInfo subOrderInfo;\
	pMember->GetSubOrderInfo(subOrderInfo);\
	\
	const auto isDebug = subOrderInfo.type >= eSO_DebugEnableCombat && subOrderInfo.type <= eSO_DebugStanceCrouch;\
	if (isDebug)\
	{\
		UpdateDebugOrder(frametime, pMember);\
		return;\
	}\
	const auto step = pMember->GetSubStep()\


void CSquad::UpdateFootSubOrder(float frametime, CMember* pMember)
{
	DEFINE_FOOT_SUB_ORDER(pMember);
	DEFINE_ACTION_VALUES;

	//In this func get only from entity, because AI Action Reference Entity is created before
	const auto pOrderObject = GET_ENTITY(subOrderInfo.targetId);
	if (!pOrderObject)
	{
		CryLogAlways("%s[C++][ERROR][Squad %i][Member %s][Update Order %s FAILED][Cause: pOrderObject]",
			STR_RED, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(subOrderInfo.type));

		SUB_ORDER_RETURN(pMember);
	}

	string desiredGoalPipe = "";

	AIR_REF_REDEFENITION(pMember, pOrderObject, false, false);

	//Vec3 pos;
	//if (pMemberAI->GetValidPositionNearby(pOrderObject->GetWorldPos(), pos))
	//{
	//	auto mat34 = pRef->GetWorldTM();
	//	mat34.SetTranslation(pos);
	//	pRef->SetWorldTM(mat34);
	//}
	//else
	{
		auto mat34 = pOrderObject->GetWorldTM();
		pRef->SetWorldTM(mat34);
	}

	//For player-given order see ClientApplyExecution for more info
	//For ai-given order see moment when ExecuteOrder is called
	const char* performingActionName = subOrderInfo.stepActions[EOrderExecutingStep::PerformingAction];
	const char* actionName = subOrderInfo.stepActions[step];

	SAIActionStats actionStats;
	g_pControlSystem->GetAIActionTracker()->GetActionStats(pMemberAI, actionStats);

	const bool performed = g_pControlSystem->GetAIActionTracker()->IsFinished(pMemberAI, performingActionName);
	if (performed)
	{
		pMember->ResetOrder(false, true, false);
		return;
		//SUB_ORDER_RETURN(pMember);
	}

	if (strcmp(actionName, "NullActionName") == 0)
	{
		CryLogAlways("%s[C++][ERROR][Squad %i][Member %s][Update Order %s FAILED][Cause: NullActionName]",
			STR_RED, GetId(), pMemberActor->GetEntity()->GetName(), m_pSquadSystem->GetString(subOrderInfo.type));

		SUB_ORDER_RETURN(pMember);
	}

	switch (subOrderInfo.type)
	{
	case eSO_SubSecondaryShootAt:
	case eSO_SubPrimaryShootAt:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);

			pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		break;
		}
	}
	break;
	case eSO_SubUseVehicleTurret:
	case eSO_SubEnterVehicle:
	{
		auto pObjectVeh = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pOrderObject->GetId());
		if (!pObjectVeh)
			break;

		const auto vehRadius = TOS_Vehicle::GetEnterRadius(pObjectVeh);

		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
			pMember->SetStep(EOrderExecutingStep::GotoTarget, true);
			break;
		case EOrderExecutingStep::GotoTarget:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (TOS_Distance::IsBigger(pMemberActor, pRef, vehRadius))
			{
				if (!TOS_AI::IsExecuting(pMemberAI, actionName))
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				TOS_AI::ExecuteAIAction(pMemberAI, pObjectVeh->GetEntity(), actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		}
	}
	break;
	case eSO_SubSecondaryPickupItem:
	case eSO_SubPrimaryPickupItem:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);

			pMember->SetStep(EOrderExecutingStep::GotoTarget, true);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step goto target";

			if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 1.5f))
			{
				if (!TOS_AI::IsExecuting(pMemberAI, actionName))
					TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		}
	}
	break;
	}
	//case eSO_SubAlienDropPlayerSquad:
	//{

	//}
	//break;
	//case eSO_SubAlienGrabPlayerSquad:
	//{
	//	auto pSquad = m_pSquadSystem->GetClientSquad();
	//	if (!pSquad)
	//		return;

	//	auto pAlienTable = pOrderObject->GetScriptTable();
	//	if (!pAlienTable)
	//		return;

	//	auto pAlien = GetActor(pOrderObject->GetId());
	//	if (!pAlien)			
	//		return;

	//	switch (step)
	//	{
	//	case EOrderExecutingStep::NotHaveOrder:
	//		break;
	//	case EOrderExecutingStep::GotAnOrder:
	//	{
	//		pMember->SetStep(EOrderExecutingStep::GotoTarget, true);
	//	}
	//	break;
	//	case EOrderExecutingStep::GotoTarget:
	//	{
	//		actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
	//			eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

	//		cause = "CSquad::UpdateSubOrder: eSO_SubAlienGrabPlayerSquad step goto target";

	//		if (TOS_Distance::IsBigger(pMemberActor, pAlien, 4.5f))
	//		{
	//			if (!TOS_AI::IsExecuting(pMemberAI, actionName))
	//				TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, cause);
	//		}
	//		else
	//		{
	//			pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
	//		}
	//	}
	//	break;
	//	case EOrderExecutingStep::PerformingAction:
	//	{
	//		actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
	//			eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

	//		cause = "CSquad::UpdateSubOrder: eSO_SubAlienGrabPlayerSquad step performing action";

	//		if (!TOS_AI::IsExecuting(pMemberAI, actionName))
	//			TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, cause);
	//	}
	//	}
	//}
	//break;
	//	//Debug orders needs to be updated separately
	//	//See CSquad::UpdateDebugOrder for more info
}

void CSquad::UpdateDebugOrder(float frametime, CMember* pMember)
{
	if (!pMember)
		return;

	auto pMemberActor = static_cast<CActor*>(GetActor(pMember->m_entityId));
	const auto isAlive = pMemberActor && (pMemberActor != GetLeader()) && (pMemberActor->GetHealth() > 0);
	if (!isAlive)
	{
		SUB_ORDER_RETURN(pMember);
	}

	auto pMemberAI = pMemberActor->GetEntity()->GetAI();
	if (!pMemberAI)
	{
		SUB_ORDER_RETURN(pMember);
	}

	auto pMemberPipe = pMemberAI->CastToIPipeUser();
	if (!pMemberPipe)
	{
		SUB_ORDER_RETURN(pMember);
	}

	const auto step = pMember->GetSubStep();

	SOrderInfo subOrderInfo;
	pMember->GetSubOrderInfo(subOrderInfo);

	const char* performingActionName = subOrderInfo.stepActions[EOrderExecutingStep::PerformingAction];
	const char* actionName = subOrderInfo.stepActions[step];
	const bool isExecuting = TOS_AI::IsExecuting(pMemberAI, actionName);

	//0: set to false and write the debug functions update
	bool performed = true;

	switch (subOrderInfo.type)
	{
	case eSO_DebugEnableCombat:
		break;
	case eSO_DebugDisableCombat:
		break;
	case eSO_DebugStanceRelaxed:
		break;
	case eSO_DebugStanceStanding:
		break;
	case eSO_DebugStanceStealth:
		break;
	case eSO_DebugStanceCrouch:
		break;
	}

	SAIActionStats actionStats;
	g_pControlSystem->GetAIActionTracker()->GetActionStats(pMemberAI, actionStats);

	const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	performed = strcmp(actionStats.actionName, performingActionName) == 0 && (currentTime - actionStats.lastTimeFinished) < 0.2f;

	if (performed)
		pMember->ResetOrder(false, true, false);
}

IEntity* CSquad::GetActionRef(CMember* pMember, Vec3 position)
{
	if (!pMember)
		return nullptr;

	auto pEntity = pMember->GetActionRef();
	if (!pEntity)
		return pEntity = m_pSquadSystem->CreateActionReference(pMember, position);

	auto mat34 = pEntity->GetWorldTM();
	mat34.SetTranslation(position);
	pEntity->SetWorldTM(mat34);

	return pEntity;
}

void CSquad::UpdateHumanFootOrder(float frametime, CMember* pMember)
{
	DEFINE_SQUAD_FOOT_MAIN_ORDER_UPDATE(pMember);
	DEFINE_STEPS;
	DEFINE_ACTION_VALUES;

	//DEFINE_ACTOR_OBJ_THRESHOLDS(actObjectDist);
	//DEFINE_ACTOR_REF_THRESHOLDS(actRefDist);

	//AIR_REF_REDEFENITION(pMember, pOrderObject, false, false);
	LAND_REF_REDEFENITION(pMember, pOrderObject, 1.0f);

	string desiredGoalPipe = "";


	//Vec3 pos;
	//if (pMemberAI->GetValidPositionNearby(pOrderObject->GetWorldPos(), pos))
	//{
	//	auto mat34 = pRef->GetWorldTM();
	//	mat34.SetTranslation(pos);
	//	pRef->SetWorldTM(mat34);
	//}
	//else
	//{
	//	auto mat34 = pOrderObject->GetWorldTM();
	//	pRef->SetWorldTM(mat34);
	//}

	switch (orderInfo.type)
	{
	case eSO_FollowLeader:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 3.5f))
			{
				cause = "CSquad::UpdateOrdersNew step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!TOS_AI::IsExecuting(pMemberAI, actionName))
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 3.0f))
			{
				cause = "CSquad::UpdateOrdersNew step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				const auto onPaused = g_pControlSystem->GetAIActionTracker()->IsPaused(pMemberAI);

				//Execute blank action
				if (!TOS_AI::IsExecuting(pMemberAI, actionName))
				{
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
				}
				else if (TOS_AI::IsExecuting(pMemberAI, actionName) && !onPaused)
				{
					const bool isNotFollowOrder = !TOS_AI::IsUsingPipe(pMemberAI, "squad_follow_leader");

					//handle order there
					if (!isInCombat)
					{
						if (isNotFollowOrder)
							TOS_AI::InsertSubpipe(pMemberAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader", cause);

						TOS_AI::SetRefPoint(pMemberAI, pRef->GetWorldPos());
					}
				}
			}
			else if (TOS_Distance::IsSmaller(pMemberActor, pRef, 1))
			{
				const auto isNotBackoff = !TOS_AI::IsUsingPipe(pMemberAI, "squad_follow_leader_backoff");
				if (isNotBackoff)
				{
					cause = "CSquad::UpdateOrdersNew follow order backoff";

					const Vec3 refPos = pRef->GetWorldPos();

					const Vec3 dir = (memberPos - refPos).GetNormalizedSafe();
					const Vec3 backoffPos = refPos + dir * 2.5f;

					TOS_AI::SetRefPoint(pMemberAI, backoffPos);
					TOS_AI::InsertSubpipe(pMemberAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader_backoff", cause);
				}
			}
		}
		break;
		}
	}
	break;
	case eSO_Guard:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 4.0f))
			{
				cause = "CSquad::UpdateOrdersNew step goto target";
				//actionFlag = eAAEF_IgnoreCombatDuringAction;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 5.0f))
			{
				cause = "CSquad::UpdateOrdersNew step performing action";
				//actionFlag = eAAEF_None;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;


				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_SearchEnemy:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
		{

		}
		break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 3.0f))
			{
				cause = "CSquad::UpdateOrdersNew SearchEnemy step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			cause = "CSquad::UpdateOrdersNew SearchEnemy step performing action";

			actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			if (!isExecuting)
				TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
		}
		break;
		}
	}
	break;
	case eSO_ConqGoTo:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 3.0f))
			{
				cause = "CSquad::UpdateOrdersNew ConqGoto step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (TOS_Distance::IsBigger(pMemberActor, pRef, 2.0f) /*&& !isInCombat*/)
			{
				cause = "CSquad::UpdateOrdersNew step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_SearchCoverAroundPoint:
	case eSO_ConqSearchCoverAroundArea:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			if (isInCombat)
				break;

			auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
			if (hideId == 0)
			{
				const char* cause = "CSquad::UpdateOrders: Got an order step (search cover)";
				hideId = m_pSquadSystem->BookFreeHideSpot(pMemberActor, orderInfo.targetPos, orderInfo.targetRadius, cause);
			}

			auto pHidespot = GET_ENTITY(hideId);
			if (!pHidespot)
			{
				//If the AI did not find a place to hide, 
				//then it will go looking for the enemy around

				//If you change this also change in alien
				SOrderInfo order;
				order.type = eSO_SearchEnemy;
				order.ignoreFlag = 0;
				order.stepActions[step2] = "conqueror_goto_a0_d0_r3";
				order.stepActions[step3] = "squad_search_enemy";
				order.targetId = pOrderObject->GetId();
				order.targetPos = pOrderObject->GetWorldPos();
				order.safeFly = true;

				//SOrderInfo failedOrder = orderInfo;
				//failedOrder.type = eSO_ConqSearchCoverAroundArea;
				//failedOrder.targetRadius = gEnv->pTimer->GetFrameStartTime().GetSeconds();
				//pMember->SetFailedOrderInfo(failedOrder);

				//SOrderInfo currentOrder; 
				//pMember->GetOrderInfo(currentOrder, false); 
				//if (currentOrder.type != eSO_SearchEnemy)
				//{
				//ExecuteOrder(pMember, order, eEOF_ExecutedByAI);
				//}

				//for (auto pListener : m_listeners)
					//pListener->OnOrderPerfomingFailed(pMember, order);

				if (!g_pControlSystem->GetConquerorSystem()->IsGamemode())
				{
					pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
					pMember->SetOrderInfo(order, false);

					if (g_pGameCVars->sqd_debug_log_executing)
					{
						CryLogAlways("%s[C++][%s switch order to Search Enemy][Cause: Not any/free find hide spot]",
							STR_YELLOW, pMemberActor->GetEntity()->GetName());
					}
				}
				else
				{
					if (!TOS_AI::IsExecuting(pMemberAI, "squad_search_enemy"))
					{
						actionFlag = eAAEF_JoinCombatPauseAction;
						TOS_AI::ExecuteAIAction(pMemberAI, pRef, "squad_search_enemy", 102.0f, -1, actionFlag, desiredGoalPipe.c_str(), "Switch action to Search Enemy");
					}
				}

				//ExecuteOrder(pMember, order, eEOF_ExecutedByAI);
				return;
			}
			else
			{
				//Update current action's object to founded hide spot
				if (pOrderObject != pHidespot)
				{
					//pOrderObject = pHidespot;
					orderInfo.targetId = pHidespot->GetId();
					orderInfo.targetPos = pHidespot->GetWorldPos();

					pMember->SetOrderInfo(orderInfo, false);
				}
				else
				{
					pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
				}
			}
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (isInCombat)
				break;

			if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 1.5f))
			{
				cause = "CSquad::UpdateOrdersNew conq Search cover goto target";
				//actionFlag = eAAEF_IgnoreCombatDuringAction;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (isInCombat)
			{
				if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 5.0f))
				{
					auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
					if (hideId != 0)
					{
						const char* cause = "CSquad::UpdateOrders: In combat and far (>5m) from cover";
						m_pSquadSystem->UnbookHideSpot(pMemberActor, cause);
					}
				}
			}
			else
			{
				//if the AI does not have a booked hidespot, 
				//then he must re-find it and book it
				auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
				if (hideId == 0)
				{
					pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
				}
				else
				{
					if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 1.5f))
					{
						pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
					}
					else
					{
						cause = "CSquad::UpdateOrdersNew conq Search cover performing action";

						actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
							eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

						if (!isExecuting)
							TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
					}
				}
			}

		}
		break;
		}
	}
	break;
	case eSO_ConqBlankAction:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			//if (TOS_Distance::IsBigger(pMemberVeh, pRef, 4.5f))
			//{
				//cause = "CSquad::UpdateHumanVehicleOrder ConqGoto step goto target";

				//actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					//eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				//if (!isExecuting)
					//TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, cause.c_str());
			//}
			//else
			//{
			pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			//}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			//if (TOS_Distance::IsBigger(pMemberVeh, pRef, 2) /*&& !isInCombat*/)
			//{
			cause = "CSquad::UpdateHumanFootOrder step performing action";

			actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			if (!isExecuting)
				TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			//}
		}
		break;
		}
	}
	break;
	case eSO_None:
		break;
	}
}

void CSquad::UpdateAlienOrder(float frametime, CMember* pMember)
{
	DEFINE_SQUAD_FOOT_MAIN_ORDER_UPDATE(pMember);
	DEFINE_STEPS;
	DEFINE_ACTION_VALUES;

	//DEFINE_ACTOR_OBJ_THRESHOLDS(actObjectDist);
	//DEFINE_ACTOR_REF_THRESHOLDS(actRefDist);

	const auto isAir = memberClass == "Scout" || memberClass == "Drone";

	AIR_REF_REDEFENITION(pMember, pOrderObject, orderInfo.safeFly, isAir);

	string desiredGoalPipe = "";


	switch (orderInfo.type)
	{
	case eSO_FollowLeader:
	{
		//The main idea of redefining the position of the target 
		//is not to change the target of the order initially, 
		//but to create a reference point relative to the target. 
		//And, if necessary, raise the reference point to the height that the scout needs. 
		//So that in the end the scout does not ram, for example, 
		//the player when he follows him.

		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (memberClass == "Scout")
			{
				if (TOS_Distance::IsBigger(pMemberActor, pRef, 5))
				{
					cause = "CSquad::UpdateAlienOrder FollowLeader step goto target";

					actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
						eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

					if (!isExecuting)
						TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
				}
				else
				{
					pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
				}
			}
			else
			{
				if (TOS_Distance::IsBigger(pMemberActor, pRef, 3.5f))
				{
					cause = "CSquad::UpdateAlienOrder step goto target";

					actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
						eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

					if (!TOS_AI::IsExecuting(pMemberAI, actionName))
						TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
				}
				else
				{
					pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
				}
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (memberClass == "Scout")
			{
				if (auto pScout = (CScout*)pMemberActor)
				{
					if (pScout->m_searchbeam.isActive)
						pScout->EnableSearchBeam(false);
				}

				if (TOS_Distance::IsBigger(pMemberActor, pRef, 5))
				{
					cause = "CSquad::UpdateAlienOrder step performing action";

					actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
						eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

					const auto onPaused = g_pControlSystem->GetAIActionTracker()->IsPaused(pMemberAI);

					//Execute blank action
					if (!isExecuting)
					{
						TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
					}
					else if (isExecuting && !onPaused)
					{
						const bool isNotFollowOrder = !TOS_AI::IsUsingPipe(pMemberAI, "squad_follow_leader");

						//handle order there
						if (!isInCombat)
						{
							if (isNotFollowOrder)
								TOS_AI::InsertSubpipe(pMemberAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader", cause);

							TOS_AI::SetRefPoint(pMemberAI, pRef->GetWorldPos());
						}
					}
				}
			}
			else
			{
				if (TOS_Distance::IsBigger(pMemberActor, pRef, 5))
				{
					cause = "CSquad::UpdateAlienOrder step performing action";

					actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
						eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

					const auto onPaused = g_pControlSystem->GetAIActionTracker()->IsPaused(pMemberAI);

					//Execute blank action
					if (!TOS_AI::IsExecuting(pMemberAI, actionName))
					{
						TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
					}
					else if (TOS_AI::IsExecuting(pMemberAI, actionName) && !onPaused)
					{
						const bool isNotFollowOrder = !TOS_AI::IsUsingPipe(pMemberAI, "squad_follow_leader");

						//handle order there
						if (!isInCombat)
						{
							if (isNotFollowOrder)
								TOS_AI::InsertSubpipe(pMemberAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader", cause);

							TOS_AI::SetRefPoint(pMemberAI, pRef->GetWorldPos());
						}
					}
				}
				else if (TOS_Distance::IsSmaller(pMemberActor, pRef, 1))
				{
					const auto isNotBackoff = !TOS_AI::IsUsingPipe(pMemberAI, "squad_follow_leader_backoff");
					if (isNotBackoff)
					{
						cause = "CSquad::UpdateAlienOrder follow order backoff";

						const Vec3 refPos = pRef->GetWorldPos();
						const Vec3 dir = (memberPos - refPos).GetNormalizedSafe();
						const Vec3 backoffPos = refPos + dir * 2.5f;

						TOS_AI::SetRefPoint(pMemberAI, backoffPos);
						TOS_AI::InsertSubpipe(pMemberAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader_backoff", cause);
					}
				}
			}
		}
		break;
		}
	}
	break;
	case eSO_Guard:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (memberClass == "Scout" ? TOS_Distance::IsBigger(pMemberActor, pRef, 5) : TOS_Distance::IsBigger(pMemberActor, pRef, 3))
			{
				cause = "CSquad::UpdateAlienOrder step goto target";
				//actionFlag = eAAEF_IgnoreCombatDuringAction;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (memberClass == "Scout" ? TOS_Distance::IsBigger(pMemberActor, pRef, 5) : TOS_Distance::IsBigger(pMemberActor, pRef, 2))
			{
				cause = "CSquad::UpdateAlienOrder step performing action";
				//actionFlag = eAAEF_None;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_SearchEnemy:
	{
		//auto safeFly is on

		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
		{

		}
		break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (memberClass == "Scout" ? TOS_Distance::IsBigger(pMemberActor, pRef, 5) : TOS_Distance::IsBigger(pMemberActor, pRef, 3))
			{
				cause = "CSquad::UpdateAlienOrder SearchEnemy step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			cause = "CSquad::UpdateAlienOrder SearchEnemy step performing action";

			actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			if (!isExecuting)
				TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());

			if (memberClass == "Scout")
			{
				auto pScout = (CScout*)pMemberActor;
				if (!pScout->m_searchbeam.isActive)
				{
					pScout->EnableSearchBeam(true);

					auto scoutPos = pScout->GetEntity()->GetWorldPos();
					auto scoutPos2 = Vec3(scoutPos.x, scoutPos.y + 3.0f, scoutPos.z - 4.0f);
					auto scoutBeamDir = (scoutPos2 - scoutPos).GetNormalized();

					pScout->SetSearchBeamGoal(scoutBeamDir);
				}
			}
		}
		break;
		}
	}
	break;
	case eSO_ConqGoTo:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (memberClass == "Scout" ? TOS_Distance::IsBigger(pMemberActor, pRef, 5) : TOS_Distance::IsBigger(pMemberActor, pRef, 3))
			{
				cause = "CSquad::UpdateAlienOrder ConqGoto step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (memberClass == "Scout" ? TOS_Distance::IsBigger(pMemberActor, pRef, 5) : TOS_Distance::IsBigger(pMemberActor, pRef, 2) /*&& !isInCombat*/)
			{
				cause = "CSquad::UpdateAlienOrder step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_SearchCoverAroundPoint:
	case eSO_ConqSearchCoverAroundArea:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			if (isInCombat)
				break;

			auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
			if (hideId == 0)
			{
				const char* cause = "CSquad::UpdateOrders: Got an order step (search cover)";
				hideId = m_pSquadSystem->BookFreeHideSpot(pMemberActor, orderInfo.targetPos, orderInfo.targetRadius, cause);
			}

			auto pHidespot = GET_ENTITY(hideId);
			if (!pHidespot)
			{
				//If the AI did not find a place to hide, 
				//then it will go looking for the enemy around

				//If you change this also change in human foot
				SOrderInfo order;
				order.type = eSO_SearchEnemy;
				order.ignoreFlag = 0;
				order.stepActions[step2] = "conqueror_goto_a5_d2_r3";
				order.stepActions[step3] = "squad_search_enemy";
				order.targetId = pOrderObject->GetId();
				order.targetPos = pOrderObject->GetWorldPos();

				pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
				pMember->SetOrderInfo(order, false);

				if (g_pGameCVars->sqd_debug_log_executing)
				{
					CryLogAlways("%s[C++][%s switch order to Search Enemy][Cause: Not find any/free hide spot]",
						STR_YELLOW, pMemberActor->GetEntity()->GetName());
				}
				return;
			}
			else
			{
				//Update current action's object to founded hide spot
				if (pOrderObject != pHidespot)
				{
					//pOrderObject = pHidespot;
					orderInfo.targetId = pHidespot->GetId();
					orderInfo.targetPos = pHidespot->GetWorldPos();

					pMember->SetOrderInfo(orderInfo, false);
				}
				else
				{
					pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
				}
			}
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (isInCombat)
				break;

			if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 1.5f))
			{
				cause = "CSquad::UpdateAlienOrder conq Search cover goto target";
				//actionFlag = eAAEF_IgnoreCombatDuringAction;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (isInCombat)
			{
				if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 5))
				{
					auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
					if (hideId != 0)
					{
						const char* cause = "CSquad::UpdateOrders: In combat and far (>5m) from cover";
						m_pSquadSystem->UnbookHideSpot(pMemberActor, cause);
					}
				}
			}
			else
			{
				auto hideId = m_pSquadSystem->GetBookedHideSpot(pMemberActor);
				if (hideId == 0)
				{
					pMember->SetStep(EOrderExecutingStep::GotAnOrder, false);
				}
				else
				{
					if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 5))
					{
						pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
					}
					else
					{
						cause = "CSquad::UpdateOrdersNew conq Search cover performing action";

						actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
							eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

						if (!isExecuting)
							TOS_AI::ExecuteAIAction(pMemberAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
					}
				}
			}

		}
		break;
		}
	}
	break;
	case eSO_None:
		break;
	}
}

void CSquad::UpdateHumanVehicleOrder(float frametime, CMember* pMember)
{
	DEFINE_SQUAD_VEHICLE_MAIN_ORDER_UPDATE(pMember);
	DEFINE_STEPS;
	DEFINE_ACTION_VALUES;

	const string leaderClass = GetLeader()->GetEntity()->GetClass()->GetName();

	AIR_REF_REDEFENITION(pMember, pOrderObject, orderInfo.safeFly, isAir);

	if (!TOS_Vehicle::ActorIsDriver(pMemberActor) && orderInfo.type != eSO_ConqBlankAction)
		return;

	string desiredGoalPipe = "";

	//Main orders
	switch (orderInfo.type)
	{
	case eSO_FollowLeader:
	{
		//The main idea of redefining the position of the target 
		//is not to change the target of the order initially, 
		//but to create a reference point relative to the target. 
		//And, if necessary, raise the reference point to the height that the air vehicle needs. 
		//So that in the end the air vehicle does not ram, for example, 
		//the player when he follows him.
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 6))
			{
				cause = "CSquad::UpdateHumanVehicleOrder step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (isAir ? TOS_Distance::IsBigger(pMemberVeh, pRef, 5) : TOS_Distance::IsBigger(pMemberVeh, pRef, 4))
			{
				cause = "CSquad::UpdateHumanVehicleOrder step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				const auto onPaused = g_pControlSystem->GetAIActionTracker()->IsPaused(pMemberVehAI);

				//Execute blank action
				if (!isExecuting)
				{
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
				}
				else if (isExecuting && !onPaused)
				{
					const bool isNotFollowOrder = !TOS_AI::IsUsingPipe(pMemberVehAI, "squad_follow_leader");

					//handle order there
					if (!isInCombat)
					{
						if (isNotFollowOrder)
							TOS_AI::InsertSubpipe(pMemberVehAI, AIGOALPIPE_SAMEPRIORITY, 0, "squad_follow_leader", cause);

						TOS_AI::SetRefPoint(pMemberVehAI, pRef->GetWorldPos());
					}
				}
			}
		}
		break;
		}
	}
	break;
	case eSO_Guard:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 5))
			{
				cause = "CSquad::UpdateHumanVehicleOrder step goto target";
				//actionFlag = eAAEF_IgnoreCombatDuringAction;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 6))
			{
				cause = "CSquad::UpdateHumanVehicleOrder step performing action";
				//actionFlag = eAAEF_None;

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_SearchEnemy:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
		{

		}
		break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 5))
			{
				cause = "CSquad::UpdateHumanVehicleOrder SearchEnemy step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			cause = "CSquad::UpdateHumanVehicleOrder SearchEnemy step performing action";

			actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			if (!isExecuting)
				TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
		}
		break;
		}
	}
	break;
	case eSO_ConqGoTo:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 4.5f))
			{
				cause = "CSquad::UpdateHumanVehicleOrder ConqGoto step goto target";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			if (TOS_Distance::IsBigger(pMemberVeh, pRef, 2) /*&& !isInCombat*/)
			{
				cause = "CSquad::UpdateHumanVehicleOrder step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!isExecuting)
					TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			}
		}
		break;
		}
	}
	break;
	case eSO_ConqBlankAction:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			pMember->SetStep(EOrderExecutingStep::GotoTarget, false);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		{
			//if (TOS_Distance::IsBigger(pMemberVeh, pRef, 4.5f))
			//{
				//cause = "CSquad::UpdateHumanVehicleOrder ConqGoto step goto target";

				//actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
					//eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				//if (!isExecuting)
					//TOS_AI::ExecuteAIAction(pMemberVehAI, pRef, actionName, maxAlertness, goalPipeId, actionFlag, cause.c_str());
			//}
			//else
			//{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, false);
			//}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			//if (TOS_Distance::IsBigger(pMemberVeh, pRef, 2) /*&& !isInCombat*/)
			//{
				cause = "CSquad::UpdateHumanVehicleOrder step performing action";

				actionFlag = orderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
					eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!TOS_AI::IsExecuting(pMemberActor->GetEntity()->GetAI(), actionName))
					TOS_AI::ExecuteAIAction(pMemberActor->GetEntity()->GetAI(), pRef, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause.c_str());
			//}
		}
		break;
		}
	}
	break;
	case eSO_SearchCoverAroundPoint:
	case eSO_ConqSearchCoverAroundArea:
	break;
	case eSO_None:
	break;
	}
}

void CSquad::AddListener(ISquadEventListener* pListener)
{
	if (!pListener)
		return;

	stl::push_back_unique(m_listeners, pListener);
}

void CSquad::RemoveListener(ISquadEventListener* pListener)
{
	if (!pListener)
		return;

	stl::find_and_erase(m_listeners, pListener);
}

void CSquad::UpdateVehicleSubOrder(float frametime, CMember* pMember)
{
	DEFINE_VEHICLE_SUB_ORDER(pMember);
	DEFINE_ACTION_VALUES;

	//In this func get only from entity, because AI Action Reference Entity is created before
	const auto pOrderObject = GET_ENTITY(subOrderInfo.targetId);
	if (!pOrderObject)
	{
		SUB_ORDER_RETURN(pMember);
	}

	AIR_REF_REDEFENITION(pMember, pOrderObject, false, false);
	auto mat34 = pOrderObject->GetWorldTM();
	pRef->SetWorldTM(mat34);

	string desiredGoalPipe = "";

	//For player-given order see ClientApplyExecution for more info
	//For ai-given order see moment when ExecuteOrder is called

	const char* performingActionName = subOrderInfo.stepActions[EOrderExecutingStep::PerformingAction];
	const char* actionName = subOrderInfo.stepActions[step];

	const bool isVehicleOrder = subOrderInfo.type == eSO_SubPrimaryShootAt || subOrderInfo.type == eSO_SubSecondaryShootAt;
	const auto idealAI = (isVehicleOrder && isDriver && seatWeaponCount > 0) ? pMemberVehAI : pMemberAI;

	SAIActionStats actionStats;
	g_pControlSystem->GetAIActionTracker()->GetActionStats(idealAI, actionStats);

	const bool performed = g_pControlSystem->GetAIActionTracker()->IsFinished(idealAI, performingActionName);
	if (performed)
	{
		//pMember->ResetOrder(false, true, false);
		//pMember->OnSubOrderFinish();
		//return;
		SUB_ORDER_RETURN(pMember);
	}

	if (strcmp(actionName, "NullActionName") == 0)
	{
		//return;
		SUB_ORDER_RETURN(pMember);
	}

	switch (subOrderInfo.type)
	{
	default:
		break;
	case eSO_SubSecondaryShootAt:
	case eSO_SubPrimaryShootAt:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
		{
			if (!TOS_AI::IsExecuting(idealAI, actionName))
				TOS_AI::ExecuteAIAction(idealAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);

			pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
		}
		break;
		case EOrderExecutingStep::GotoTarget:
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(idealAI, actionName))
				TOS_AI::ExecuteAIAction(idealAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		break;
		}
	}
	break;

	case eSO_SubExitVehicle:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
			pMember->SetStep(EOrderExecutingStep::GotoTarget, true);
			break;
		case EOrderExecutingStep::GotoTarget:
			pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
			break;
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(idealAI, actionName))
				TOS_AI::ExecuteAIAction(idealAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		break;
		}

	}
	break;

	case eSO_SubUseVehicleTurret:
	case eSO_SubEnterVehicle:
	{
		switch (step)
		{
		case EOrderExecutingStep::NotHaveOrder:
			break;
		case EOrderExecutingStep::GotAnOrder:
			pMember->SetStep(EOrderExecutingStep::GotoTarget, true);
			break;
		case EOrderExecutingStep::GotoTarget:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenGotoTarget ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (TOS_Distance::IsBigger(pMemberActor, pOrderObject, 4))
			{
				if (!TOS_AI::IsExecuting(idealAI, actionName))
					TOS_AI::ExecuteAIAction(idealAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
			}
			else
			{
				pMember->SetStep(EOrderExecutingStep::PerformingAction, true);
			}
		}
		break;
		case EOrderExecutingStep::PerformingAction:
		{
			actionFlag = subOrderInfo.ignoreFlag & eOICF_IgnoreCombatWhenPerfomingAction ?
				eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			cause = "CSquad::UpdateSubOrder step performing action";

			if (!TOS_AI::IsExecuting(idealAI, actionName))
				TOS_AI::ExecuteAIAction(idealAI, pOrderObject, actionName, maxAlertness, goalPipeId, actionFlag, desiredGoalPipe.c_str(), cause);
		}
		}
	}
	break;
	}
}

IActor* CSquad::GetLeader() const
{
	auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_leaderId);
	if (pActor)
		return pActor;

	return nullptr;
	//return pActor;
	//return m_leaderId;
}

void CSquad::UpdateConquerorDetachedAlien(float frametime, IActor* pAlien, const SDetachedMemberData& data)
{
	if (!pAlien)
		return;

	const auto pAlienAI = pAlien->GetEntity()->GetAI();
	if (!pAlienAI)
		return;

	auto pGoalEntity = GET_ENTITY(data.targetId);
	const auto goalPathName = data.pathName;
	const auto routineType = data.routineType;

	if (goalPathName.size())
		pGoalEntity = gEnv->pEntitySystem->FindEntityByName(goalPathName);

	if (!pGoalEntity)
		return;

	const char* desiredGoalName = "";
	const char* actionName = "nullDetachedAction";
	const int goalPipeId = -1;
	const int maxAlertness = 120;
	EAAEFlag flag = eAAEF_JoinCombatPauseAction;
	string cause = "CSquad::UpdateConquerorDetachedAlien: ";

	switch (routineType)
	{
	default:
		break;
	case eDRT_OnFootPathPatrol:
		break;
	case eDRT_LandPathPatrol:
	{
		//actionName = "detached_land_follow_path_1";
		//cause += string("eDRT_LandPathPatrol: start following path");

		//if (!TOS_AI::IsExecuting(pAlienAI, actionName))
		//	TOS_AI::ExecuteAIAction(pAlienAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, 0, cause.c_str());
	}
	break;
	case eDRT_LandPointGuard:
		break;
	case eDRT_AirPathPatrol:
	{
		desiredGoalName = "follow_path_nearest";
		actionName = "detached_scout_follow_path_1";
		cause += string("eDRT_AirPathPatrol: start following path");

		if (!TOS_AI::IsExecuting(pAlienAI, actionName))
			TOS_AI::ExecuteAIAction(pAlienAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, cause.c_str());
	}
		break;
	case eDRT_AirPointSearch:
	{
		//if (TOS_Distance::IsBigger(pVehicle, pGoalEntity, 5.0f) && (!TOS_AI::IsExecuting(pVehAI, "squad_search_enemy")))
		//{
		//	actionName = "detached_goto_a0_d0_r3";
		//	cause += string("eDRT_AirPointSearch: goto search point");

		//	if (!TOS_AI::IsExecuting(pVehAI, actionName))
		//		TOS_AI::ExecuteAIAction(pVehAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, cause.c_str());
		//}
		//else
		{
			actionName = "squad_search_enemy";
			cause += string("eDRT_AirPointSearch: start search enemy");

			if (!TOS_AI::IsExecuting(pAlienAI, actionName))
				TOS_AI::ExecuteAIAction(pAlienAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, 0, cause.c_str());
		}
	}
	break;
	case eDRT_WaterPathPatrol:
		break;
	case eDRT_WaterPointPatrol:
		break;
	}
}

void CSquad::GetLeaderDetachedData(SDetachedMemberData& data)
{
	auto pLeader = GetLeader();
	if (!pLeader)
		return;

	data = m_detachedLeadersData[pLeader->GetEntityId()];
}

bool CSquad::IsAllMembersSelected()
{
	const int membersCount = m_members.size();
	int selectedCount = 0;

	for (auto& member : m_members)
	{
		if (IsMemberSelected(member.GetIndex()))
			selectedCount++;
	}

	return selectedCount == membersCount;
}