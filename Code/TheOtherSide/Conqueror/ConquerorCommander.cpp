#include "StdAfx.h"

#include "IAgent.h"
#include "IVehicleSystem.h"

#include "Actor.h"
#include "Player.h"

#include "ConquerorSystem.h"
#include "StrategicArea.h"
#include "ConquerorStrategy.h"
#include "ConquerorSpeciesClass.h"
#include "ConquerorChannel.h"

#include "../Control/ControlSystem.h"
#include "../Squad/SquadSystem.h"
#include "../Helpers/TOS_AI.h"
#include "../Helpers/TOS_Vehicle.h"
#include "../Helpers/TOS_Debug.h"
#include "../AI Files/AIActionTracker.h"

#include "SerializeFwd.h"
#include "ConquerorCommander.h"
//#include "ConquerorStrategy.h"

CConquerorCommander::CConquerorCommander(ESpeciesType species)
{
	Init(species);
}

CConquerorCommander::CConquerorCommander()
{
	Init(eST_NEUTRAL);
}

 CConquerorCommander::~CConquerorCommander()
{
	 //m_species = eST_NEUTRAL;
	 //m_loosingAdvantage = false;
	 //m_lastTimeLoosingAdvantage = 0;
	 //m_currentStrategyGoals.clear();
	 //m_currentStrategyName.clear();
	 //m_lastStrategyName.clear();
	 //m_strategiesNamesHistory.clear();
	 //m_targettedAreasMap.clear();
	 //m_squadBookedVehicle.clear();
	 //m_squadParatroopers.clear();
	 //m_currentStrategyTimeLimit = 0;
	 //m_areaFlagPriorities.clear();
}

void CConquerorCommander::Init(ESpeciesType species)
{
	m_pSquadSystem = g_pControlSystem->GetSquadSystem();
	m_pConqueror = g_pControlSystem->GetConquerorSystem();

	assert(m_pSquadSystem);
	assert(m_pConqueror);

	m_species = species;
	m_loosingAdvantage = false;
	m_lastTimeLoosingAdvantage = 0;
	m_currentStrategyGoals.clear();
	m_currentStrategyName.clear();
	m_lastStrategyName.clear();
	m_strategiesNamesHistory.clear();
	m_targettedAreasMap.clear();
	m_squadBookedVehicle.clear();
	m_squadParatroopers.clear();
	//m_squadsVehicles.clear();
	//m_disableSquadUpdate.clear();
	m_currentStrategyTimeLimit = 0;
	m_refs = 0;
	
	for (int i = 0; i < eSGT_Last; i++)
		m_currentStrategyGoals[EStrategyGoalTemplates(i)] = 0;

	for (auto pArea : m_pConqueror->m_strategicAreas)
		m_targettedAreasMap.insert(std::make_pair(pArea, 0));

	const auto first = static_cast<int>(EAreaFlag::FirstType);
	const auto last = static_cast<int>(EAreaFlag::LastType);

	for (int i = first; i < last; i++)
	{
		const auto enumI = static_cast<EAreaFlag>(i);
		m_areaFlagPriorities[HOSTILE][enumI] = 100.f;
		m_areaFlagPriorities[NEUTRAL][enumI] = 100.f;
		m_areaFlagPriorities[OWNED][enumI] = 100.f;
		m_areaFlagPriorities[ANY][enumI] = 100.f;

		for (int j = eST_FirstPlayableSpecies; j <= eST_LastPlayableSpecies; j++)
		{
			const auto species = static_cast<ESpeciesType>(j);
			const auto name = m_pConqueror->GetSpeciesName(species);
			m_areaFlagPriorities[name][enumI] = 100.f;
		}
	}

	if (m_species != eST_NEUTRAL)
	{
		auto& speciesSquadIds = m_pSquadSystem->GetSquadIdsFromSpecies(m_species, true);
		for (auto id : speciesSquadIds)
		{
			auto pSquad = m_pSquadSystem->GetSquadFromId(id);
			if (!pSquad)
				continue;

			auto pLeader = pSquad->GetLeader();
			if (!pLeader)
				continue;

			m_subordinateSquadIds.push_back(pSquad->GetId());
			//pSquad->m_pCommander = this;

			auto targetId = EntityId(0);
			auto targetPos = pLeader->GetEntity()->GetWorldPos();

			auto pBaseArea = GetArea(eAGSF_Enabled, OWNED, eABF_NoMatter, EAreaFlag::Base);
			if (pBaseArea)
			{
				targetId = pBaseArea->GetEntityId();
				targetPos = pBaseArea->GetWorldPos();
			}
			else
			{
				CryLogAlways("[C++][ERROR][CConquerorCommander][Init][Species %s][NO BASE AREA]", m_pConqueror->GetSpeciesName(m_species));
			}

			m_commanderOrdersMap[pLeader->GetEntityId()] = CCommanderOrder(eCO_Defend, targetPos, eOES_Starting, targetId);
			m_targettedAreasMap[pBaseArea].push_back(pSquad->GetId());
		}

		if (g_pGameCVars->conq_debug_log_commander)
			CryLogAlways("[C++][Create Commander for species (%s)]", m_pConqueror->GetSpeciesName(m_species));
	}
}

//Update the commander when all bots are spawned
void CConquerorCommander::Update(float frameTime)
{
	UpdateNew(frameTime);
	return;
}

void CConquerorCommander::UpdateNew(float frameTime)
{
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	if (m_pConqueror && !m_pConqueror->IsGamemode())
		return;

	auto pRARSystem = m_pConqueror->GetRAR();
	if (!pRARSystem)
		return;

	if (m_species == eST_NEUTRAL)
		return;

	const auto allAreasCount = m_pConqueror->GetStrategicAreaCount(eAGSF_EnabledAndCapturable);
	const auto neutralCount = m_pConqueror->GetStrategicAreaCount(eST_NEUTRAL, eAGSF_EnabledAndCapturable);

	const auto friendlyAreasCount = m_pConqueror->GetStrategicAreaCount(GetSpecies(), eAGSF_EnabledAndCapturable);
	const auto enemyAreasCount = max(0, allAreasCount - neutralCount - friendlyAreasCount);

	if (friendlyAreasCount < enemyAreasCount)
	{
		if (m_loosingAdvantage == false)
		{
			m_loosingAdvantage = true;
			m_lastTimeLoosingAdvantage = currentTime;
			OnStartLoosingAdvantage();
		}
	}
	else
	{
		if (m_loosingAdvantage == true)
		{
			m_loosingAdvantage = false;
			OnStopLoosingAdvantage();
		}
	}

	if (m_loosingAdvantage && currentTime - m_lastTimeLoosingAdvantage > 5.0f)
	{
		if (m_pConqueror->GetSpeciesReinforcements(GetSpecies()) > 1)
		{
			m_pConqueror->AddSpeciesReinforcements(GetSpecies(), 1, -1);
			m_lastTimeLoosingAdvantage = currentTime;
		}
	}

	//TODO: fix O(n^2)
	for (auto id : m_subordinateSquadIds)
	{
		auto pSquad = m_pSquadSystem->GetSquadFromId(id);
		if (!pSquad)
			continue;

		auto pLeader = pSquad->GetLeader();
		if (!pLeader)
			continue;

		auto pOrder = GetCommanderOrder(pLeader);
		if (!pOrder)
			continue;

		//TODO: fix O(n^3) oh my god... 06.05.2023
		for (auto pArea : m_pConqueror->m_strategicAreas)
		{
			auto it = m_targettedAreasMap[pArea].begin();
			auto end = m_targettedAreasMap[pArea].end();

			for (; it != end; it++)
			{
				if (*it == id)
				{
					if (pOrder->m_targetId != pArea->GetEntityId())
					{
						m_targettedAreasMap[pArea].erase(it);
						break;
					}
				}
			}
		}
	}

	for (auto squadId : m_subordinateSquadIds)
	{
		auto pSquad = m_pSquadSystem->GetSquadFromId(squadId);
		if (!pSquad)
			continue;

		//const auto squadSpecies = pSquad->GetSpecies();

		auto pLeaderActor = static_cast<CActor*>(pSquad->GetLeader());
		if (!pLeaderActor || (pLeaderActor && pLeaderActor->GetHealth() < 0))
			continue;

		//const auto& leaderVehStats = pLeaderActor->m_vehicleStats;

		//const auto pChannel = m_pConqueror->GetConquerorChannel(pLeaderActor->GetEntity());
		//if (!pChannel)
		//	continue;

		//if (pChannel->GetState() == eCCS_Dead || pChannel->GetState() == eCCS_Spectator)
		//	continue;

		auto pLeaderOrder = GetCommanderOrder(pLeaderActor->GetEntityId());
		if (!pLeaderOrder)
			continue;

		if (!(pSquad->HasClientLeader() || pSquad->HasClientMember()))
		{
			//Leader AI stuff
			auto pLeaderAI = pLeaderActor->GetEntity()->GetAI();
			if (!pLeaderAI)
				continue;
			
			//const string leaderClassName = pLeaderActor->GetEntity()->GetClass()->GetName();

			const auto isCombat = TOS_AI::IsInCombat(pLeaderAI);
			const auto isEnabledCombat = TOS_AI::IsCombatEnable(pLeaderAI);

			//const auto leaderPos = pLeaderActor->GetEntity()->GetWorldPos();
			//const auto truePipeId = GetProperGoalPipeId(pLeaderOrder);
			//~Leader AI stuff

			//const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			//Alien stuff
			//auto pLeaderRequest = pRARSystem->GetRequestFromEntity(pLeaderActor->GetEntityId());	
			//const auto executorId = pRARSystem->GetAssignedExecutorId(pLeaderRequest);	
			//const auto pLeaderGrabStats = pLeaderActor->GetGrabStats();
			const auto isAlien = pLeaderActor->IsAlien();
			//~Alien stuff

			//Leader Vehicle stuff
			auto pLeaderVehicle = TOS_Vehicle::GetVehicle(pLeaderActor);
			auto pLeaderVehicleAI = TOS_Vehicle::GetAI(pLeaderVehicle);

			const auto isVehEnabledCombat = TOS_AI::IsCombatEnable(pLeaderVehicleAI);
			const auto isLeaderDriver = TOS_Vehicle::ActorIsDriver(pLeaderActor);
			const auto isVehCombat = TOS_AI::IsInCombat(pLeaderVehicleAI);
			//const auto isCar = TOS_Vehicle::IsCar(pLeaderVehicle);
			//~Leader Vehicle stuff

			//Order goal definition
			auto pGoalArea = m_pConqueror->GetStrategicArea(pLeaderOrder->m_targetId, 0);

			auto& goalPosition = pLeaderOrder->m_pos;
			const auto targetId = pLeaderOrder->m_targetId;
			const bool noTarget = goalPosition == Vec3(0, 0, 0) && targetId == 0;
			
			if (noTarget)
			{
				CryLogAlways("[C++][Species Commander][update FAILED][Cause: Leader not have order target or pos]");
				continue;
			}
			else if (goalPosition == Vec3(0, 0, 0))
			{
				if (pGoalArea)
					goalPosition = pGoalArea->GetWorldPos();
			}
			//~Order goal definition

			//Debug draw
			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			{
				if (g_pGameCVars->conq_debug_draw_commander_squads > 0)
				{
					static float color[] = { 1,1,1,1 };
					const auto size = 1.1f;
					const auto scale = 20;
					const auto xoffset = TOS_Debug::XOFFSET_COMMON;
					const auto yoffset = TOS_Debug::YOFFSET_CONQ_CMDR_SQUADS;

					gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, 1.3f, color, false,
						"%s commander squads: ", GetSpeciesName());

					string goalName = pGoalArea ? pGoalArea->GetEntity()->GetName() : "NullGoalName";

					const auto pVehicle = GetBookedVehicle(pSquad->GetId());
					string vehName = pVehicle ? pVehicle->GetEntity()->GetName() : "NullVehName";

					gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + pSquad->GetId() * scale, size, color, false,
						"Squad leader id: %i, Is driving: %i, Is combat: %i / %i, Is vehicle combat: %i / %i, Order state: %s, Order goal: %s, Booked vehicle: %s, ",
						pSquad->GetLeader()->GetEntityId(), isLeaderDriver, isCombat, isEnabledCombat, isVehCombat, isVehEnabledCombat, GetString(pLeaderOrder->m_state), goalName, vehName);
				}
			}

			//~Debug draw

			//FIX: leader ai can not insert ai action goalpipe
			const float delay = g_pGameCVars->conq_squads_update_delay;//seconds

			if (currentTime - pSquad->GetConqIterationTime() < delay)
				continue;

			if (currentTime - pLeaderActor->GetActorStats()->lastTimeRespawned < 0.5f)
				continue;

			pSquad->SetConqIterationTime(currentTime);

			if (isAlien)
			{
				UpdateLeaderAlien(frameTime, pSquad);
			}
			else
			{
				if (pLeaderVehicle)
				{
					UpdateLeaderHumanVehicle(frameTime, pSquad);
				}
				else
				{
					UpdateLeaderHumanFoot(frameTime, pSquad);
				}
			}

			//if (!isAlien)
			//{
			//	UpdateLeaderHumanFoot(frameTime, pSquad);

			//	//Combat always takes precedence over commander or leader orders
			//	if (pLeaderVehicle)
			//	{
			//		UpdateLeaderHumanVehicle(frameTime, pSquad);

			//		//Vehicles with paratroopers or cars always ignore combat until they reach their destination
			//		//if (!isCar)
			//		//{
			//		//	if (GetParatroopsCount(pSquad, pLeaderVehicle) == 0)
			//		//	{
			//		//		if (isVehCombat)
			//		//		{
			//		//			//CryLogAlways("%s[C++][Vehicle %s is in combat]", STR_PURPLE, pLeaderVehicle->GetEntity()->GetName());
			//		//			continue;
			//		//		}
			//		//	}
			//		//}				
			//	}
			//	else
			//	{
			//		//If leader on foot

			//		const bool notStarting = pLeaderOrder->m_state != eOES_Starting;
			//		bool notProfitable = false;

			//		//Pre Book Vehicle
			//		if (pLeaderOrder->m_state == eOES_Starting)
			//		{
			//			const float radius = 30.0f;
			//			const uint flags = eVGF_NearestOrLinks;
			//			const int minSeatCount = 1;

			//			auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
			//			if (!pBookedVehicle)
			//				pBookedVehicle = BookFreeVehicle(pSquad->GetId(), radius, flags, minSeatCount);

			//			auto vehIsProfitable = false;
			//			if (pBookedVehicle)
			//			{
			//				const auto vehPos = pBookedVehicle->GetEntity()->GetWorldPos();
			//				const float distFromVehToGoal = (goalPosition - vehPos).GetLength();
			//				const float distFromLeaderToGoal = (goalPosition - leaderPos).GetLength();
			//				const float distFromLeaderToVeh = (vehPos - leaderPos).GetLength();
			//				const bool vehicleIsNear = (distFromLeaderToVeh < (distFromLeaderToGoal / 4) || distFromLeaderToVeh <= radius);

			//				vehIsProfitable = distFromLeaderToGoal >= 130.f && vehicleIsNear;
			//			}

			//			if (vehIsProfitable)
			//			{
			//				string vehName = pBookedVehicle->GetEntity()->GetName();
			//				string debugSolution = string("Pre book vehicle result:") + vehName + string(" is profitable");

			//				if (TOS_AI::IsCombatEnable(pLeaderActor))
			//					TOS_AI::EnableCombat(pLeaderActor, false, true, debugSolution.c_str());
			//			}
			//		}

			//		if(notStarting || notProfitable)
			//		{
			//			if (isCombat)
			//			{
			//				//Update non combat members here
			//				//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
			//				//	pSquad->SetLastAllOrder(eSO_FollowLeader);

			//				for (auto& member : pSquad->GetAllMembers())
			//				{
			//					//if (member.GetCurrentOrder() != eSO_FollowLeader)
			//					//{
			//					//	member.SetCurrentOrder(eSO_FollowLeader);
			//						//CryLogAlways("%s[C++][Leader %s][Apply afk fix to member %i]", STR_CYAN, pLeaderActor->GetEntity()->GetName(), member.GetId());
			//					//}
			//				}

			//				continue;
			//			}
			//				
			//		}
			//	}
			//	//--Starting--
			//	//Set last command order to follow
			//	//Find free vehicle and book it
			//	//Enter vehicle or not
			//	//Go to state MovingToTarget

			//	//--MovingToTarget--
			//	//Drive/GoTo to goal position
			//	//Unload paratroops
			//	//Exit from vehicle
			//	//Go to state In Action

			//	//--Perfoming Action--
			//	//If in vehicle then exit from it
			//	//Defend the goal position

			//	switch (pLeaderOrder->m_state)
			//	{
			//	case eOES_Starting:
			//	{
			//		//1.1 Initial
			//		const float lastTimeStrategyChange = gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_lastTimeStrategyChange;
			//		if (lastTimeStrategyChange < 0.01f)
			//		{
			//			//By default squad leader and members are not detached
			//			if (pSquad->IsLeaderDetached())
			//				pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

			//			if (!m_pConqueror->CanBookUnloadSpot(pLeaderVehicle))
			//				m_pConqueror->SetCanBookSpot(pLeaderVehicle, 1);

			//			for (auto& member : pSquad->m_members)
			//			{
			//				if (pSquad->IsMemberDetached(member.GetId()))
			//					pSquad->MarkUndetached(member.GetId());

			//				//if (member.GetCurrentOrder() != eSO_FollowLeader)
			//				//{
			//				//	pSquad->ExecuteOrder(eSO_FollowLeader, &member, eEOF_ExecutedByAI);
			//				//	member.SetCurrentOrder(eSO_FollowLeader);
			//				//}
			//			}

			//			//if (pSquad->GetLastCommanderOrder() != eSO_FollowLeader)
			//			//	pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByCommander, 0, Vec3(0, 0, 0), 0);
			//			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//			{
			//				CryLogAlways("%s[C++][%s CMDR][Update][On Strategy Change][Initial Squad Leader %i]", STR_YELLOW, GetSpeciesName()
			//					, pSquad->GetLeader()->GetEntityId());
			//			}
			//		}

			//		if (isLeaderDriver && pLeaderVehicle)
			//		{
			//			const int membersInVehCount = pSquad->GetMembersCount(pLeaderVehicle);
			//			const int membersCount = pSquad->GetAliveMembersCount();
			//			const int seatsCount = pLeaderVehicle->GetSeatCount();

			//			//The driver will wait for the rest of the squad for defined seconds. 
			//			//After which he will start moving
			//			const float waitSecTimer = 13.0f;

			//			const bool driverWaitMembers = (currentTime - leaderVehStats.lastTimeEnter) < waitSecTimer;

			//			//The whole squad must get into the leader vehicle
			//			if ((seatsCount >= membersCount) && (membersInVehCount < membersCount) && driverWaitMembers)
			//			{
			//				//Wait for members
			//				continue;
			//			}
			//			else
			//			{
			//				if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//				{
			//					CryLogAlways("%s[C++][Squad Leader Id %i][Driver][pLeaderOrder->SetState(eOES_InMovingToTarget)]",
			//						STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//				}

			//				pLeaderOrder->SetState(eOES_InMovingToTarget);
			//			}
			//		}
			//		else
			//		{
			//			//1.2 Book free vehicle to squad

			//			const float radius = 30.0f;
			//			const uint flags = eVGF_NearestOrLinks;
			//			const int minSeatCount = 1;

			//			auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
			//			if (!pBookedVehicle)
			//				pBookedVehicle = BookFreeVehicle(pSquad->GetId(), radius, flags, minSeatCount);

			//			auto vehIsProfitable = false;
			//			auto playerEntered = false;
			//			auto vehicleIsNear = false;
			//			auto bigGoalDist = false;

			//			if (pBookedVehicle)
			//			{
			//				const auto vehPos = pBookedVehicle->GetEntity()->GetWorldPos();
			//				const float distFromVehToGoal = (goalPosition - vehPos).GetLength();
			//				const float distFromLeaderToGoal = (goalPosition - leaderPos).GetLength();
			//				const float distFromLeaderToVeh = (vehPos - leaderPos).GetLength();

			//				const int playerEnterLastTime = currentTime - clientVehicleStats.lastTimeEnter;
			//				const bool playerEnterInBooked = clientVehicleStats.lastOperatedVehicleId == pBookedVehicle->GetEntityId();
			//				
			//				playerEntered = playerEnterLastTime < 0.1f && playerEnterInBooked;
			//				vehicleIsNear = (distFromLeaderToVeh < (distFromLeaderToGoal / 4) || distFromLeaderToVeh <= radius);
			//				bigGoalDist = distFromLeaderToGoal >= 130.f;

			//				if (playerEntered)
			//				{
			//					if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//					{
			//						CryLogAlways("[C++][Player enter the booked vehicle %s -> cancel entering for ai agent]", pBookedVehicle->GetEntity()->GetName());
			//					}
			//				}

			//				vehIsProfitable = !playerEntered && bigGoalDist && vehicleIsNear;
			//			}

			//			const auto needEnter = (pBookedVehicle && vehIsProfitable);
			//			if (needEnter)
			//			{
			//				if (TOS_AI::IsCombatEnable(pLeaderActor))
			//					TOS_AI::EnableCombat(pLeaderActor, false, false, "Need enter to booked vehicle");

			//				//1.3 Leader Enter the booked vehicle

			//				const auto actualRefPos = pLeaderPipe->GetRefPoint()->GetPos();
			//				const auto vehPos = pBookedVehicle->GetEntity()->GetWorldPos();
			//				const float distToVehicle = (vehPos - leaderPos).GetLength();

			//				auto vehDistThreshold = VEHICLE_LAND_ENTER_THRESHOLD_DIST;
			//				if (TOS_Vehicle::IsAir(pBookedVehicle))
			//					vehDistThreshold = VEHICLE_AIR_ENTER_THRESHOLD_DIST;

			//				const auto needGetCloser = distToVehicle >= vehDistThreshold;
			//				if (needGetCloser)
			//				{
			//					//if (actualRefPos != vehPos)
			//					//	pLeaderPipe->SetRefPointPos(vehPos);

			//					//if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
			//					//	pLeaderPipe->SelectPipe(0, "ord_goto", 0, GOALPIPEID_ORDER_GOTO);

			//					if (actualRefPos != vehPos)
			//						TOS_AI::SetRefPoint(pLeaderActor, vehPos);

			//					//if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
			//					//	TOS_AI::SelectPipe(pLeaderActor, GOALPIPEID_ORDER_GOTO, "ord_goto", "Need get closer with booked vehicle");
			//				}
			//				else
			//				{
			//					const auto freeSeatIndex = TOS_Vehicle::RequestFreeSeatIndex(pBookedVehicle);

			//					const auto pSeat = pBookedVehicle->GetSeatById(freeSeatIndex);
			//					if (pSeat)
			//					{
			//						pSeat->Enter(pLeaderActor->GetEntityId(), false);

			//						if (!TOS_AI::IsCombatEnable(pLeaderActor))
			//							TOS_AI::EnableCombat(pLeaderActor, true, false, "Actor is entered in the booked vehicle");
			//					}
			//				}

			//				//Go to 1.4
			//			}
			//			else
			//			{
			//				//if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//				//{
			//					//if (!vehIsProfitable && !playerEntered && pBookedVehicle)
			//					//{
			//					//	CryLogAlways("%s[C++][Squad Leader Id %i][Go to goal on Foot][Vehicle Not Profitable]",
			//					//		STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//					//}
			//					//else if (!vehIsProfitable && playerEntered && pBookedVehicle)
			//					//{
			//					//	CryLogAlways("%s[C++][Squad Leader Id %i][Find new booked veh][Player enter old vehicle]",
			//					//		STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//					//}
			//					//else if (!pBookedVehicle)
			//					//{
			//					//	CryLogAlways("%s[C++][Squad Leader Id %i][Goto goal on Foot][Vehicle Not Found]",
			//					//		STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//					//}
			//					//else 
			//					//{
			//					//	CryLogAlways("%s[C++][Squad Leader Id %i][Goto goal on Foot][Undefined reason]",
			//					//		STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//					//}
			//				//}

			//				auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
			//				if (pBookedVehicle)
			//				{
			//					if (UnbookVehicle(pSquad->GetId()) && playerEntered)
			//					{
			//						//Go to start and retry find vehicle

			//						if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//						{
			//							CryLogAlways("%s[C++][Squad Leader Id %i][Go to start and retry find vehicle]",
			//								STR_YELLOW, pSquad->GetLeader()->GetEntityId());
			//						}

			//						pLeaderOrder->SetState(eOES_Starting);
			//						continue;
			//					}
			//				}

			//				if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//				{
			//					CryLogAlways("%s[C++][Squad Leader Id %i][Goto goal on Foot][Vehicle: profitable %i, playerEntered %i, isNear %i, bigDistToGoal %i]",
			//						STR_YELLOW, pSquad->GetLeader()->GetEntityId(), vehIsProfitable, playerEntered, vehicleIsNear, bigGoalDist);
			//				}

			//				//1.5 Leader go to goal pos on foot
			//				//Members following leader
			//				pLeaderOrder->SetState(eOES_InMovingToTarget);
			//			}
			//		}
			//	}
			//	break;
			//	case eOES_InMovingToTarget:
			//	{
			//		//todo move to vehicle update
			//		auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
			//		if (pBookedVehicle && !isLeaderDriver)
			//		{
			//			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//			{
			//				CryLogAlways("%s[C++][Squad Leader %i: Lost Booked Vehicle: %s]",
			//					STR_CYAN, pSquad->GetLeader()->GetEntityId(), pBookedVehicle->GetEntity()->GetName());
			//			}

			//			pLeaderOrder->SetState(eOES_Starting);
			//			continue;
			//			//Enter the lost booked vehicle in next update iteration
			//		}

			//		if (isLeaderDriver && pLeaderVehicle)
			//		{
			//			//1.4 Leader drive vehicle to goal pos
			//			const auto leaderVehPos = pLeaderVehicle->GetEntity()->GetWorldPos();

			//			auto pLeaderVehAI = pLeaderVehicle->GetEntity()->GetAI();
			//			if (!pLeaderVehAI)
			//				continue;

			//			auto pLeaderVehPipe = pLeaderVehAI->CastToIPipeUser();
			//			if (!pLeaderVehPipe)
			//				continue;

			//			auto pLeaderVehAIProxy = pLeaderVehAI->GetProxy();
			//			if (!pLeaderVehAIProxy)
			//				continue;

			//			auto pGunnerSeat = pLeaderVehicle->GetSeatById(TOS_Vehicle::RequestGunnerSeatIndex(pLeaderVehicle));
			//			const auto isHaveGunnerSeat = pGunnerSeat != nullptr;

			//			const auto vehAlertness = pLeaderVehAIProxy->GetAlertnessState();
			//			const auto vehHaveTarget = pLeaderVehPipe->GetAttentionTarget() != nullptr;

			//			const auto isAir = TOS_Vehicle::IsAir(pLeaderVehicle);
			//			const auto isCar = TOS_Vehicle::IsCar(pLeaderVehicle);
			//			const auto isPLV = TOS_Vehicle::IsPLV(pLeaderVehicle);
			//			const auto isSea = TOS_Vehicle::IsSea(pLeaderVehicle);

			//			//1.5 (for members) go to target on foot
			//			std::vector<EntityId> members;
			//			pSquad->GetMembersNotInVehicle(pLeaderVehicle, members);

			//			for (auto id : members)
			//			{
			//				auto pMemberInstance = pSquad->GetMemberInstance(id);
			//				if (!pMemberInstance)
			//					continue;

			//				auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
			//				if (!pMemberActor)
			//					continue;

			//				if (pMemberActor->GetHealth() == 0 || pMemberActor->GetEntity()->IsHidden())
			//					continue;

			//				//if (pMemberInstance->GetCurrentOrder() != eSO_Conq_GoTo)
			//				//{
			//				//	pSquad->ExecuteOrderEx(eSO_Conq_GoTo, pMemberInstance, eEOF_ExecutedByAI, 0, goalPosition, 0);
			//				//	pMemberInstance->SetCurrentOrder(eSO_Conq_GoTo);
			//				//}
			//			}

			//			//Get vehicle stuck here


			//			//Get vehicle compatibility unload spot here
			//			if (pGoalArea)
			//			{
			//				auto id = pGoalArea->GetBookedUnloadSpot(pLeaderVehicle);
			//				if (id == 0)
			//					id = pGoalArea->BookFreeUnloadSpot(pLeaderVehicle);

			//				auto pSpotEntity = gEnv->pEntitySystem->GetEntity(id);
			//				if (pSpotEntity)
			//					goalPosition = pSpotEntity->GetWorldPos();
			//			}

			//			//Drive to goal pos here
			//			const float goalDist = (goalPosition - leaderVehPos).GetLength();
			//			const auto needToMove = goalDist > goalDistTreshold;
			//			const auto needToUnloadParatroopers = !needToMove && (GetParatroopsCount(pSquad, pLeaderVehicle) > 0);
			//			const auto goalMoveDone = !needToMove && (GetParatroopsCount(pSquad, pLeaderVehicle) == 0);

			//			if (needToMove)
			//			{
			//				const auto notTruePos = TOS_AI::GetRefPoint(pLeaderVehAI) != goalPosition;
			//				const auto notTruePipeId = TOS_AI::GetGoalPipeId(pLeaderVehAI) != truePipeId;

			//				if (notTruePos)
			//				{
			//					if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//					{
			//						CryLogAlways("[C++][Squad Leader %i][Vehicle %i][Need Move]", pSquad->GetLeader()->GetEntityId(), pLeaderVehicle->GetEntityId());
			//					}

			//					TOS_AI::SetRefPoint(pLeaderVehAI, goalPosition);
			//				}

			//				if (notTruePipeId)
			//					TOS_AI::SelectPipe(pLeaderVehAI, truePipeId, "tos_commander_ord_goto", "Vehicle need move to target area");
			//			}
			//			else if (needToUnloadParatroopers)
			//			{
			//				if (m_pConqueror->CanBookUnloadSpot(pLeaderVehicle))
			//					m_pConqueror->SetCanBookSpot(pLeaderVehicle, false);

			//				if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//				{
			//					//CryLogAlways("[C++][Squad Leader %i][Vehicle %i][needToUnloadParatroopers]", pSquad->GetLeader()->GetEntityId(), pLeaderVehicle->GetEntityId());
			//				}

			//				//ignore combat when unload paratroopers
			//				if (TOS_AI::IsCombatEnable(pLeaderVehAI))
			//					TOS_AI::EnableCombat(pLeaderVehAI, false, true, "Vehicle need unload paratroopers");

			//				//Drive to landing pos here
			//				const Vec3 airLandingSpot = m_pConqueror->GetNearestLandingSpot(pLeaderVehicle->GetEntity());

			//				//Unload spot is goal position now because it defined in past
			//				//Vec3 unloadSpot = goalPosition;

			//				const float distLandingSpot = (pLeaderVehicle->GetEntity()->GetWorldPos() - airLandingSpot).GetLength();
			//				const float distUnloadSpot = (pLeaderVehicle->GetEntity()->GetWorldPos() - goalPosition).GetLength();
			//				const auto landingNear = distLandingSpot < distUnloadSpot;

			//				const Vec3 unloadPoint = isAir ? (landingNear ? airLandingSpot : goalPosition) : goalPosition;

			//				const float goalDist = (unloadPoint - leaderPos).GetLength();
			//				const auto needToMove = goalDist > goalDistTreshold;

			//				if (needToMove)
			//				{
			//					const auto notTruePos = TOS_AI::GetRefPoint(pLeaderVehAI) != unloadPoint;
			//					const auto notTruePipeId = TOS_AI::GetGoalPipeId(pLeaderVehAI) != truePipeId;

			//					if (notTruePos)
			//					{
			//						//pLeaderVehPipe->SetRefPointPos(unloadPoint);
			//						TOS_AI::SetRefPoint(pLeaderVehAI, unloadPoint);
			//						if (GetSpecies() == m_pConqueror->GetClientSpecies())
			//						{
			//							CryLogAlways("[C++][Squad Leader %i][Vehicle %i][needToUnloadParatroopers][NeedMove]", pSquad->GetLeader()->GetEntityId(), pLeaderVehicle->GetEntityId());
			//						}

			//					}

			//					if (notTruePipeId)
			//						TOS_AI::SelectPipe(pLeaderVehAI, truePipeId, "tos_commander_ord_goto", "Vehicle need be closer with unload point when he unloading paratroopers");
			//				}
			//				else
			//				{
			//					OnParatroopersUnloaded(pLeaderVehicle);
			//					auto paratroopersOrder = eSO_ConqSearchCoverAroundArea;

			//					if (IsAssignedSquad(pGoalArea, pSquad))
			//					{
			//						const int assignedNumber = GetAssignedSquadNumber(pGoalArea, pSquad);
			//						if (assignedNumber == 1)
			//						{
			//							paratroopersOrder = eSO_ConqSearchCoverAroundArea;
			//						}
			//						else if (assignedNumber > 1)
			//						{
			//							paratroopersOrder = eSO_SearchEnemy;
			//						}
			//					}

			//					//pSquad->SetLastAllOrder(paratroopersOrder);
			//					
			//					//I don't understand why I wrote this? 0_o
			//					for (auto& member : pSquad->m_members)
			//					{
			//						auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
			//						if (!pMemberActor)
			//							continue;

			//						if (pMemberActor->GetHealth() < 0 || pMemberActor->GetEntity()->IsHidden())
			//							continue;

			//						//if (member.GetCurrentOrder() != eSO_Conq_GoTo)
			//						//{
			//						//	pSquad->ExecuteOrderEx(eSO_Conq_GoTo, &member, eEOF_ExecutedByAI, 0, goalPosition, 0);
			//						//	member.SetCurrentOrder(eSO_Conq_GoTo);
			//						//}
			//					}

			//					std::vector<EntityId> paratroopers;
			//					GetParatroopers(pSquad, pLeaderVehicle, paratroopers);

			//					for (auto id : paratroopers)
			//					{
			//						auto pMemberInstance = pSquad->GetMemberInstance(id);
			//						if (!pMemberInstance)
			//							continue;

			//						auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMemberInstance->GetId()));
			//						if (!pActor)
			//							continue;

			//						auto pVeh = TOS_Vehicle::GetVehicle(pActor);
			//						if (!pVeh)
			//							continue;

			//						const auto lastExitSeconds = gEnv->pTimer->GetFrameStartTime().GetSeconds() - pActor->m_vehicleStats.lastTimeExit;
			//						const auto vehPos = pVeh->GetEntity()->GetWorldPos();
			//						const auto distToTarget = (vehPos - goalPosition).GetLength();
			//						const auto needExit = (distToTarget <= goalDistTreshold + 7.0f) && (lastExitSeconds > 5);

			//						if (needExit)
			//						{
			//							TOS_Vehicle::Exit(pActor, true, false);

			//							//The paratrooper after disembarking will automatically follow the last order of the commander
			//							//See CConquerorCommander::OnExitVehicle for more info
			//						}
			//					}
			//				}
			//			}
			//			else if (goalMoveDone)
			//			{
			//				//enable combat after unload paratroopers
			//				//if (!TOS_AI::IsCombatEnable(pLeaderVehAI))
			//					//TOS_AI::EnableCombat(pLeaderVehAI, true, false, "Vehicle moving to goal is done");

			//				if (isCar)
			//					TOS_Vehicle::Exit(pLeaderActor, true, false);

			//				if (!m_pConqueror->CanBookUnloadSpot(pLeaderVehicle))
			//					m_pConqueror->SetCanBookSpot(pLeaderVehicle, true);

			//				//go to 1.7 defending strategic zone when leader driving a vehicle
			//				pLeaderOrder->SetState(eOES_PerformingAction);
			//			}

			//		}
			//		else
			//		{
			//			//1.5 Leader go to goal pos on foot
			//			//Members following leader

			//			//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
			//				//pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

			//			const auto actualRefPos = pLeaderPipe->GetRefPoint()->GetPos();
			//			const float goalDist = (goalPosition - leaderPos).GetLength();
			//			const auto leaderNeedToMove = goalDist > goalDistTreshold;
			//			const auto leaderGoalMoveDone = !leaderNeedToMove;

			//			if (leaderNeedToMove)
			//			{
			//				if (actualRefPos != goalPosition)
			//					TOS_AI::SetRefPoint(pLeaderActor, goalPosition);

			//			//	if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
			//			//		TOS_AI::SelectPipe(pLeaderActor, GOALPIPEID_ORDER_GOTO, "tos_commander_ord_goto", "Leader need moving to target area");
			//			}
			//			else if (leaderGoalMoveDone)
			//			{
			//				//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
			//				//	pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

			//				pLeaderOrder->SetState(eOES_PerformingAction);
			//				//go to 1.6 defending strategic zone when leader on foot
			//			}
			//		}
			//	}
			//	break;
			//	case eOES_PerformingAction:
			//	{
			//		//1.7 Defending strategic zone when leader driving a vehicle 
			//		if (isLeaderDriver && pLeaderVehicle)
			//		{
			//			auto pLeaderVehAI = pLeaderVehicle->GetEntity()->GetAI();
			//			if (!pLeaderVehAI)
			//				continue;

			//			auto pLeaderVehPipe = pLeaderVehAI->CastToIPipeUser();
			//			if (!pLeaderVehPipe)
			//				continue;

			//			auto pLeaderVehAIProxy = pLeaderVehAI->GetProxy();
			//			if (!pLeaderVehAIProxy)
			//				continue;

			//			const auto isAir = TOS_Vehicle::IsAir(pLeaderVehicle);
			//			const auto isTank = TOS_Vehicle::IsTank(pLeaderVehicle);
			//			const auto isSea = TOS_Vehicle::IsSea(pLeaderVehicle);

			//			//Give a task to the leader
			//			if (isAir || isTank || isSea)
			//			{
			//				if (!pSquad->IsLeaderDetached())
			//				{
			//					SDetachedMemberData data;
			//					data.enableUpdate = true;

			//					if (isAir)
			//					{
			//						data.routineType = eDRT_AirPointSearch;
			//						data.points.push_back(goalPosition);
			//					}
			//					else if (isTank)
			//					{
			//						//27.12.2022
			//						//eDRT_LandPathPatrol not Implemented yet

			//						data.routineType = eDRT_LandPointGuard;
			//						data.points.push_back(pLeaderVehicle->GetEntity()->GetWorldPos());
			//					}
			//					else if (isSea)
			//					{
			//						//27.12.2022
			//						//eDRT_WaterPathPatrol not Implemented yet

			//						data.routineType = eDRT_WaterPathPatrol;
			//						data.pathName = "";
			//					}

			//					//Updating actions is now in the hands of the squad
			//					//See CSquad::UpdateConquerorDetached for more info
			//					pSquad->MarkDetached(pLeaderActor->GetEntityId(), data);
			//				}
			//			}

			//			//Give orders to foot members
			//			std::vector<EntityId> members;
			//			pSquad->GetMembersNotInVehicle(pLeaderVehicle, members);

			//			for (auto memId : members)
			//			{
			//				auto pMemberInstance = pSquad->GetMemberInstance(memId);
			//				if (!pMemberInstance)
			//					continue;

			//				auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMemberInstance->GetId());
			//				if (!pActor)
			//					continue;

			//				const auto lastOrder = pSquad->GetLastAllOrder();
			//				//if (pMemberInstance->GetCurrentOrder() != lastOrder)
			//				{
			//					//pSquad->ExecuteOrderEx(lastOrder, pMemberInstance, eEOF_ExecutedByAI, 0, pGoalArea->GetWorldPos(), 30);
			//				}
			//			}

			//			//End
			//		}
			//		else
			//		{
			//			//1.6 Defending strategic zone when leader on foot

			//			//Get cover search radius
			//			auto anchorRadius = goalDistTreshold;
			//			if (pGoalArea)
			//				anchorRadius = TOS_AI::GetAnchorRadius(pGoalArea->GetAIAnchor());

			//			//Give a task to the leader
			//			const auto isInCover = pLeaderPipe->IsUsingPipe("tos_commander_ord_goto_sub_hide_incover");
			//			const auto notTruePipe = TOS_AI::IsUsingPipe(pLeaderAI, "sqd_search_cover");

			//			//if (!notTruePipe && !isInCover)
			//			//{
			//			//	auto hidespotPos = m_pSquadSystem->GetNearestHidespot(goalPosition, anchorRadius);

			//			//	if (hidespotPos == Vec3(0, 0, 0))
			//			//		hidespotPos = goalPosition;

			//			//	TOS_AI::SetRefPoint(pLeaderAI, hidespotPos);
			//			//	TOS_AI::SelectPipe(pLeaderAI, 0, "sqd_search_cover", "Leader defending the target area");
			//			//}

			//			//Give a task to the members if they are close
			//			auto commanderOrder = eSO_ConqSearchCoverAroundArea;

			//			if (IsAssignedSquad(pGoalArea, pSquad))
			//			{
			//				const int assignedNumber = GetAssignedSquadNumber(pGoalArea, pSquad);
			//				if (assignedNumber == 1)
			//				{
			//					commanderOrder = eSO_ConqSearchCoverAroundArea;
			//				}
			//				else if (assignedNumber > 1)
			//				{
			//					commanderOrder = eSO_SearchEnemy;
			//				}
			//			}

			//			std::vector<EntityId> members;
			//			pSquad->GetMembersOnFoot(members);

			//			for (auto id : members)
			//			{
			//				auto pMemberInstance = pSquad->GetMemberInstance(id);
			//				if (!pMemberInstance)
			//					continue;

			//				auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMemberInstance->GetId());
			//				if (!pActor)
			//					continue;

			//				//if (pMemberInstance->GetCurrentOrder() != commanderOrder)
			//				//{
			//				//	pSquad->ExecuteOrderEx(commanderOrder, pMemberInstance, eEOF_ExecutedByAI, 0, goalPosition, 30);
			//				//	pMemberInstance->SetCurrentOrder(commanderOrder);
			//				//}
			//			}

			//			//const auto distToMembers = pSquad->GetAverageDistanceToMembers(leaderPos);
			//			//if (distToMembers < 10 && (pSquad->GetLastAllOrder() != commanderOrder))
			//			//{
			//			//	//pSquad->SetLastCommanderOrder(commanderOrder);
			//			//	pSquad->ExecuteOrderAllMembers(commanderOrder, eEOF_ExecutedByAI, 0, goalPosition, 30.0f);
			//			//}
			//		}
			//	}
			//	break;
			//	}
			//}
			//else
			//{
			//	UpdateLeaderAlien(frameTime, pSquad);

			//	if (isCombat)
			//		continue;

			//	switch (pLeaderOrder->m_state)
			//	{
			//	case eOES_Starting:
			//	{
			//		if (leaderClassName == "Scout")
			//		{

			//		}
			//		else if (leaderClassName == "Trooper")
			//		{
			//			//1.1 Initial
			//			const float lastTimeStrategyChange = gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_lastTimeStrategyChange;
			//			if (lastTimeStrategyChange < 0.01f)
			//			{
			//				//By default squad leader and members are not detached
			//				if (pSquad->IsLeaderDetached())
			//					pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

			//				for (auto& member : pSquad->m_members)
			//				{
			//					if (pSquad->IsMemberDetached(member.GetId()))
			//						pSquad->MarkUndetached(member.GetId());

			//					//if (member.GetCurrentOrder() != eSO_FollowLeader)
			//					//{
			//					//	pSquad->ExecuteOrder(eSO_FollowLeader, &member, eEOF_ExecutedByAI);
			//					//	member.SetCurrentOrder(eSO_FollowLeader);
			//					//}
			//				}

			//				//if (pSquad->GetLastCommanderOrder() != eSO_FollowLeader)
			//				//	pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByCommander, 0, Vec3(0, 0, 0), 0);

			//				//CryLogAlways("%s[C++][%s CMDR][Update][Initial Squad Leader %i]", STR_YELLOW, GetSpeciesName()
			//					//, pSquad->GetLeader()->GetEntityId());
			//			}

			//			if (pLeaderRequest && pRARSystem->GetAssignedExecutorId(pLeaderRequest))
			//			{
			//				if (pLeaderRequest->state == eRQ_Assigned)
			//				{
			//					//Stay in place and wait grabbing
			//					if (TOS_AI::GetRefPoint(pLeaderActor) != leaderPos)
			//						TOS_AI::SetRefPoint(pLeaderActor, leaderPos);

			//					if (!TOS_AI::IsUsingPipe(pLeaderActor, "tos_rar_wait"))
			//					{
			//						TOS_AI::SelectPipe(pLeaderActor, 0, "tos_rar_wait", "Requester need stay in place and wait grabbing");

			//						//CryLogAlways("[C++][%s requestIsAssigned][wait executioner]",
			//							//pLeaderActor->GetEntity()->GetName());
			//					}

			//					continue;
			//				}
			//				else if (pLeaderRequest->state == eRQ_Executing)
			//				{
			//					if (pLeaderGrabStats->IsGrabbedBy(executorId))
			//					{
			//						//Chill when is grabbed by scout
			//						//Leader
			//						if (!TOS_AI::IsUsingPipe(pLeaderActor, "do_nothing"))
			//							TOS_AI::SelectPipe(pLeaderActor, 0, "do_nothing", "Requester need be do nothing when is grabbed by executor");

			//						//CryLogAlways("[C++][%s IsGrabbedBy %i][MovingToTarget]",
			//							//pLeaderActor->GetEntity()->GetName(), executorId);

			//						//go to 1.4
			//						pLeaderOrder->SetState(eOES_InMovingToTarget);
			//					}

			//					//Chill when is grabbed by scout
			//					//Members
			//					for (auto& member : pSquad->m_members)
			//					{
			//						auto pMemberActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
			//						if (!pMemberActor)
			//							continue;

			//						if (pMemberActor->GetGrabStats()->IsGrabbedBy(executorId))
			//						{
			//							if (!TOS_AI::IsUsingPipe(pMemberActor, "do_nothing"))
			//								TOS_AI::SelectPipe(pMemberActor, 0, "do_nothing", "Requester members need be do nothing when is grabbed by executor");
			//						}
			//					}
			//				}

			//			}
			//			else
			//			{
			//				//1.2 Sending a request to transfer the customer's squad to the purpose of the order

			//				const auto distToOrderGoal = (goalPosition - leaderPos).GetLength();
			//				const bool bigDistanceToGoal = distToOrderGoal >= 130.0f;
			//				const auto leaderId = pLeaderActor->GetEntityId();

			//				auto pRequesterInstance = pRARSystem->GetRequesterInstance(leaderId);
			//				auto pCurrentRequest = pRARSystem->GetRequestFromEntity(leaderId);
			//				auto executorAssigned = false;

			//				if (bigDistanceToGoal)
			//				{
			//					if (!pCurrentRequest && !pRequesterInstance)
			//					{
			//						pCurrentRequest = pRARSystem->CreateRequest(leaderId, goalPosition, eRT_AlienTaxsee);
			//					}
			//					else if (!pCurrentRequest && pRequesterInstance)
			//					{
			//						const float lastTimeCreated = currentTime - pRequesterInstance->GetLastTime(eRQ_Created);
			//						if (lastTimeCreated > 120.0f)
			//						{
			//							pCurrentRequest = pRARSystem->CreateRequest(leaderId, goalPosition, eRT_AlienTaxsee);
			//						}
			//					}
			//				}

			//				const float lastTimeFailed = pCurrentRequest ? currentTime - pCurrentRequest->lastTimeFailed : 0.0f;

			//				if (pCurrentRequest)
			//				{
			//					if (pRequesterInstance && pRequesterInstance->GetRequestId() == pCurrentRequest->getId())
			//					{
			//						if (pCurrentRequest->state == eRQ_Created)
			//						{
			//							const float createdTime = pRequesterInstance->GetLastTime(eRQ_Created);
			//							const float lastTimeCreated = currentTime - createdTime;

			//							if (lastTimeCreated > 3.0f)
			//							{
			//								executorAssigned = false;
			//								pRARSystem->FailRequest(pCurrentRequest->getId(), 1);
			//							}
			//						}
			//						else if ((pCurrentRequest->state == eRQ_Failed || pCurrentRequest->state == eRQ_FailedByExecutorKilled) && lastTimeFailed < 0.1f)
			//						{
			//							//CryLogAlways("[C++][Aliens][Set Leader %i to Moving To Target on Foot]", pSquad->GetLeader()->GetEntityId());
			//							pLeaderOrder->SetState(eOES_InMovingToTarget);
			//						}
			//					}
			//				}
			//				else
			//				{
			//					//CryLogAlways("[C++][Aliens][Set Leader %i to Moving To Target on Foot]", pSquad->GetLeader()->GetEntityId());
			//					pLeaderOrder->SetState(eOES_InMovingToTarget);
			//				}

			//				//if (pCurrentRequest)
			//				//{
			//				//	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
			//				//	const float lastTimeCreated = currentTime - pCurrentRequest->lastTimeCreated;

			//				//	if (pCurrentRequest->state == eRQ_Created && lastTimeCreated > 2.9f)
			//				//	{
			//				//		executorAssigned = false;
			//				//	}
			//				//	else if (pCurrentRequest->state == eRQ_Assigned)
			//				//	{
			//				//		executorAssigned = true;
			//				//	}
			//				//}

			//				//const bool useExecutorToMove = bigDistanceToGoal && executorAssigned;

			//				//if (useExecutorToMove)
			//				//{
			//				//	continue;
			//				//}
			//				//else
			//				//{
			//				//	pLeaderOrder->SetState(eOES_InMovingToTarget);
			//				//}
			//			}
			//		}
			//		else if (leaderClassName == "Hunter")
			//		{

			//		}
			//		else if (leaderClassName == "Alien")
			//		{

			//		}
			//	}
			//	break;
			//	case eOES_InMovingToTarget:
			//	{
			//		//1.5 go to target on foot

			//		if (leaderClassName == "Scout")
			//		{

			//		}
			//		else if (leaderClassName == "Trooper")
			//		{
			//			if (pLeaderGrabStats->IsGrabbedBy(executorId))
			//			{
			//				continue;
			//			}
			//			else
			//			{
			//				//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
			//					//pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

			//				const auto actualRefPos = pLeaderPipe->GetRefPoint()->GetPos();
			//				const float goalDist = (goalPosition - leaderPos).GetLength();
			//				const auto leaderNeedToMove = goalDist > goalDistTreshold;
			//				const auto leaderGoalMoveDone = !leaderNeedToMove;

			//				if (leaderNeedToMove)
			//				{
			//					if (actualRefPos != goalPosition)
			//						TOS_AI::SetRefPoint(pLeaderActor, goalPosition);

			//					//if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
			//					//	TOS_AI::SelectPipe(pLeaderActor, GOALPIPEID_ORDER_GOTO, "tos_commander_ord_goto", "Leader need moving to target area");
			//				}
			//				else if (leaderGoalMoveDone)
			//				{
			//					pLeaderOrder->SetState(eOES_PerformingAction);
			//					//go to 1.6 defending strategic zone when leader on foot
			//				}

			//			}
			//		}
			//		else if (leaderClassName == "Hunter")
			//		{

			//		}
			//		else if (leaderClassName == "Alien")
			//		{

			//		}
			//	}
			//	break;
			//	case eOES_PerformingAction:
			//	{
			//		if (leaderClassName == "Scout")
			//		{

			//		}
			//		else if (leaderClassName == "Trooper")
			//		{
			//			//1.6 defending strategic zone when leader on foot

			//			auto anchorRadius = goalDistTreshold;

			//			if (pGoalArea)
			//				anchorRadius = TOS_AI::GetAnchorRadius(pGoalArea->GetAIAnchor());

			//			//Give a task to the leader
			//			const auto isInCover = pLeaderPipe->IsUsingPipe("tos_commander_ord_goto_sub_hide_incover");
			//			const auto notTruePipe = TOS_AI::IsUsingPipe(pLeaderAI, "sqd_search_cover");

			//			//if (!notTruePipe && !isInCover)
			//			//{
			//			//	auto hidespotPos = m_pSquadSystem->GetNearestHidespot(goalPosition, anchorRadius);

			//			//	if (hidespotPos == Vec3(0, 0, 0))
			//			//		hidespotPos = goalPosition;

			//			//	TOS_AI::SetRefPoint(pLeaderAI, hidespotPos);
			//			//	TOS_AI::SelectPipe(pLeaderAI, 0, "sqd_search_cover", "Leader defending the target area");
			//			//}

			//			//Give a task to the members if they are close
			//			auto commanderOrder = eSO_ConqSearchCoverAroundArea;

			//			if (IsAssignedSquad(pGoalArea, pSquad))
			//			{
			//				const int assignedNumber = GetAssignedSquadNumber(pGoalArea, pSquad);
			//				if (assignedNumber == 1)
			//				{
			//					commanderOrder = eSO_ConqSearchCoverAroundArea;
			//				}
			//				else if (assignedNumber > 1)
			//				{
			//					commanderOrder = eSO_SearchEnemy;
			//				}
			//			}

			//			std::vector<EntityId> members;
			//			pSquad->GetMembersOnFoot(members);

			//			for (auto id : members)
			//			{
			//				auto pMemberInstance = pSquad->GetMemberInstance(id);
			//				if (!pMemberInstance)
			//					continue;

			//				auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMemberInstance->GetId());
			//				if (!pActor)
			//					continue;

			//				//if (pMemberInstance->GetCurrentOrder() != commanderOrder)
			//				//{
			//				//	pSquad->ExecuteOrderEx(commanderOrder, pMemberInstance, eEOF_ExecutedByAI, 0, goalPosition, 30);
			//				//	pMemberInstance->SetCurrentOrder(commanderOrder);
			//				//}
			//			}

			//		}
			//		else if (leaderClassName == "Hunter")
			//		{

			//		}
			//		else if (leaderClassName == "Alien")
			//		{

			//		}
			//	}
			//	break;
			//	}
			//}
		}
	}

	if (GetCurrentStrategy())
	{
		m_currentStrategyTimeLimit -= frameTime;
		if (m_currentStrategyTimeLimit <= 0)
			m_currentStrategyTimeLimit = 0;

		if (m_currentStrategyTimeLimit == 0)
			RequestNewStrategy(m_pConqueror->m_gameStatus == eGS_GameStart);
	}
}

#define FOOT_GOAL_DIST_THRESHOLD 7.0f
#define VEHICLE_GOAL_DIST_THRESHOLD 7.0f

#define DEFINE_LEADER_UPDATE(pSquad)\
	DEFINE_STEPS;\
	\
	if (!pSquad || (pSquad && (pSquad->HasClientLeader() || pSquad->HasClientMember())))\
		return;\
	\
	const auto pLeaderActor = static_cast<CActor*>(pSquad->GetLeader());\
	if (!pLeaderActor || (pLeaderActor && pLeaderActor->GetHealth() < 0))\
		return;\
	\
	const auto pLeaderChannel = m_pConqueror->GetConquerorChannel(pLeaderActor->GetEntity());\
	if (!pLeaderChannel || (pLeaderChannel && pLeaderChannel->GetState() == eCCS_Spectator))\
		return;\
	\
	auto& commanderOrder = m_commanderOrdersMap[pLeaderActor->GetEntityId()];\
	if (commanderOrder.m_state == eOES_NotStarted)\
		return;\
	\
	auto pGoalEntity = GET_ENTITY(commanderOrder.m_targetId);\
	const auto pGoalArea = m_pConqueror->GetStrategicArea(pGoalEntity->GetId(), 0);\
	auto goalPosition = pGoalArea ? pGoalArea->GetWorldPos() : commanderOrder.m_pos;\
	const bool noTarget = goalPosition == Vec3(0) && !pGoalArea;\
	\
	if (pGoalArea->GetAIAnchor())\
		pGoalEntity = pGoalArea->GetAIAnchor();\
	\
	if (noTarget)\
	{\
		CryLogAlways("%s[C++][%s Commander][ERROR][update FAILED][Cause: Leader not have order target or pos]",\
			STR_RED, GetSpeciesName());\
		return;\
	}\

#define MEMBER_EXECUTE_SEARCH_HIDESPOT(order, pSquad, pMemAct, memInstance, goalPosition, anchorRadius)\
	order.safeFly = true;\
	order.targetId = 0;\
	order.targetPos = goalPosition;\
	order.targetRadius = anchorRadius;\
	order.type = eSO_ConqSearchCoverAroundArea;\
	order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
	order.stepActions[step3] = "squad_hiding_in_cover";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	memInstance.GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_ConqSearchCoverAroundArea)\
	{\
		pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_POINTER_EXECUTE_SEARCH_HIDESPOT(order, pSquad, pMemAct, pMemInstance, goalPosition, anchorRadius)\
	order.safeFly = true;\
	order.targetId = 0;\
	order.targetPos = goalPosition;\
	order.targetRadius = anchorRadius;\
	order.type = eSO_ConqSearchCoverAroundArea;\
	order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
	order.stepActions[step3] = "squad_hiding_in_cover";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	pMemInstance->GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_ConqSearchCoverAroundArea)\
	{\
		pSquad->ExecuteOrder(pMemInstance, order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_EXECUTE_EXIT_VEHICLE(order, pSquad, pMemAct, memInstance, pMemVeh)\
	order.safeFly = true;\
	order.targetId = pMemVeh->GetEntityId();\
	order.targetPos = pMemVeh->GetEntity()->GetWorldPos();\
	order.type = eSO_SubExitVehicle;\
	order.stepActions[step1] = "squad_vehicle_exit";\
	order.stepActions[step2] = "squad_vehicle_exit";\
	order.stepActions[step3] = "squad_vehicle_exit";\
	order.ignoreFlag = eOICF_IgnoreEnemyAlways;\
	\
	SOrderInfo currentOrder;\
	memInstance.GetSubOrderInfo(currentOrder);\
	if (currentOrder.type != eSO_SubExitVehicle)\
	{\
		pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_POINTER_EXECUTE_EXIT_VEHICLE(order, pSquad, pMemAct, pMemInstance, pMemVeh)\
	order.safeFly = true;\
	order.targetId = pMemVeh->GetEntityId();\
	order.targetPos = pMemVeh->GetEntity()->GetWorldPos();\
	order.type = eSO_SubExitVehicle;\
	order.stepActions[step1] = "squad_vehicle_exit";\
	order.stepActions[step2] = "squad_vehicle_exit";\
	order.stepActions[step3] = "squad_vehicle_exit";\
	order.ignoreFlag = eOICF_IgnoreEnemyAlways;\
	\
	SOrderInfo currentSubOrder;\
	pMemInstance->GetSubOrderInfo(currentSubOrder);\
	if (currentSubOrder.type != eSO_SubExitVehicle)\
	{\
		pSquad->ExecuteOrder(pMemInstance, order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, memInstance)\
	order.safeFly = true;\
	order.targetId = 0;\
	order.targetPos = pMemAct->GetEntity()->GetWorldPos();\
	order.type = eSO_ConqBlankAction;\
	order.stepActions[step2] = "squad_blank_action";\
	order.stepActions[step3] = "squad_blank_action";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	memInstance.GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_ConqBlankAction)\
	{\
		pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance)\
	order.safeFly = true;\
	order.targetId = pLeaderActor->GetEntityId();\
	order.targetPos = pLeaderActor->GetEntity()->GetWorldPos();\
	order.type = eSO_FollowLeader;\
	order.stepActions[step2] = "conqueror_goto_a0_d3_r3";\
	order.stepActions[step3] = "squad_blank_action";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	memInstance.GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_FollowLeader)\
	{\
		pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_POINTER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, pMemInstance, pLeaderActor)\
	order.safeFly = true;\
	order.targetId = pLeaderActor->GetEntityId();\
	order.targetPos = pLeaderActor->GetEntity()->GetWorldPos();\
	order.type = eSO_FollowLeader;\
	order.stepActions[step2] = "conqueror_goto_a0_d3_r3";\
	order.stepActions[step3] = "squad_blank_action";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	pMemInstance->GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_FollowLeader)\
	{\
		pSquad->ExecuteOrder(pMemInstance, order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_POINTER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, pMemInstance)\
	order.safeFly = true;\
	order.targetId = 0;\
	order.targetPos = pMemAct->GetEntity()->GetWorldPos();\
	order.type = eSO_ConqBlankAction;\
	order.stepActions[step2] = "squad_blank_action";\
	order.stepActions[step3] = "squad_blank_action";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	pMemInstance->GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_ConqBlankAction)\
	{\
		pSquad->ExecuteOrder(pMemInstance, order, eEOF_ExecutedByCommander);\
	}\

#define MEMBER_POINTER_EXECUTE_SEARCH_ENEMY(order, pSquad, pMemAct, pMemInstance, goalPosition)\
	order.safeFly = true;\
	order.targetId = 0;\
	order.targetPos = goalPosition;\
	order.type = eSO_SearchEnemy;\
	order.stepActions[step2] = "conqueror_goto_a5_d2_r3";\
	order.stepActions[step3] = "squad_search_enemy";\
	order.ignoreFlag = 0;\
	\
	SOrderInfo currentOrder;\
	pMemInstance->GetOrderInfo(currentOrder, false);\
	if (currentOrder.type != eSO_SearchEnemy)\
	{\
		pSquad->ExecuteOrder(pMemInstance, order, eEOF_ExecutedByCommander);\
	}\

void CConquerorCommander::UpdateLeaderHumanFoot(float frameTime, CSquad* pSquad)
{
	DEFINE_LEADER_UPDATE(pSquad);

	//LeaderAI
	auto pLeaderAI = pLeaderActor->GetEntity()->GetAI();
	if (!pLeaderAI)
		return;

	const auto leaderIsInCombat = TOS_AI::IsInCombat(pLeaderAI);
	//~LeaderAI

	//Generic AI Action values
	const char* desiredGoalName = "";
	const char* actionName = "nullActionName";
	const float maxAlertness = 102.0f; //high prioritry
	int goalPipeId = -1;
	auto flag = eAAEF_IgnoreCombatDuringAction;
	const char* solution = "CConquerorCommander::UpdateLeaderHumanFoot:";
	//~Generic AI Action values

	const auto leaderPos = pLeaderActor->GetEntity()->GetWorldPos();
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	const auto& clientVehicleStats = g_pControlSystem->GetClientActor()->m_vehicleStats;

	const auto lastTimeRespawned = currentTime - pLeaderActor->GetActorStats()->lastTimeRespawned;

	switch (commanderOrder.m_state)
	{
	default:
	break;
	case eOES_NotStarted:
	break;
	case eOES_Starting:
	{

		//1.1 Initial
		const float lastTimeStrategyChange = currentTime - m_lastTimeStrategyChange;
		if (lastTimeStrategyChange < 0.01f)
		{
			//By default squad leader and members are not detached
			if (pSquad->IsLeaderDetached())
				pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

			for (auto& member : pSquad->m_members)
			{
				if (pSquad->IsMemberDetached(member.GetId()))
					pSquad->MarkUndetached(member.GetId());
			}

			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			{
				if (g_pGameCVars->conq_debug_log_commander)
				{
					CryLogAlways("%s[C++][%s CMDR][Update][On Strategy Change][Initial Squad Leader %i]",
						STR_YELLOW, GetSpeciesName(), pSquad->GetLeader()->GetEntityId());
				}
			}
			
			goalPipeId = -1;
			actionName = "squad_clear_action";
			solution = "CConquerorCommander::UpdateLeaderHumanFoot: On eOES_Starting";
			if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
				TOS_AI::ExecuteAIAction(pLeaderAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
		}

		//When commander set current strategy we need regroup members and give it follow leader order
		auto members = pSquad->GetAllMembers();
		for (auto& memInstance : members)
		{
			DEFINE_MEMBER_ACTOR(memInstance);

			SOrderInfo order;
			MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance);
		}

		//1.2 Book free vehicle to squad

		const float radius = 30.0f;
		const uint flags = eVGF_NearestOrLinks;
		const int minSeatCount = 1;

		auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
		if (!pBookedVehicle)
			pBookedVehicle = BookFreeVehicle(nullptr, pSquad->GetId(), radius, flags, minSeatCount);

		auto vehIsProfitable = false;
		auto playerEntered = false;
		auto vehicleIsNear = false;
		auto bigGoalDist = false;

		if (pBookedVehicle)
		{
			const auto vehPos = pBookedVehicle->GetEntity()->GetWorldPos();
			const float distFromVehToGoal = (goalPosition - vehPos).GetLength();
			const float distFromLeaderToGoal = (goalPosition - leaderPos).GetLength();
			const float distFromLeaderToVeh = (vehPos - leaderPos).GetLength();

			const int playerEnterLastTime = currentTime - clientVehicleStats.lastTimeEnter;
			const bool playerEnterInBooked = clientVehicleStats.lastOperatedVehicleId == pBookedVehicle->GetEntityId();

			playerEntered = playerEnterLastTime < 0.1f && playerEnterInBooked;
			vehicleIsNear = (distFromLeaderToVeh < (distFromLeaderToGoal / 4) || distFromLeaderToVeh <= radius);
			bigGoalDist = distFromLeaderToGoal >= GetCurrentStrategy()->GetSettings().m_vehicleUseDistance;
			//bigGoalDist = distFromLeaderToGoal >= 130.f;

			if (playerEntered)
			{
				if (GetSpecies() == m_pConqueror->GetClientSpecies())
				{
					CryLogAlways("[C++][Player enter the booked vehicle %s -> cancel entering for ai agent]", pBookedVehicle->GetEntity()->GetName());
				}
			}

			vehIsProfitable = !playerEntered && bigGoalDist && vehicleIsNear;
		}

		const auto needEnter = (pBookedVehicle && vehIsProfitable);
		if (needEnter)
		{
			const auto vehRadius = TOS_Vehicle::GetEnterRadius(pBookedVehicle);

			const auto needGetCloser = TOS_Distance::IsBigger(pLeaderActor, pBookedVehicle->GetEntity(), vehRadius);
			if (needGetCloser)
			{
				//Create action info and execute it
				desiredGoalName = "action_goto0";
				actionName = "conqueror_goto_a0_d0_r3";
				flag = eAAEF_IgnoreCombatDuringAction;
				solution = "CConquerorCommander::UpdateLeaderHumanFoot: goto booked vehicle";
				goalPipeId = -1;

				if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderAI, pBookedVehicle->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}
			else
			{
				actionName = "conqueror_vehicle_enter_fast";
				flag = eAAEF_IgnoreCombatDuringAction;
				solution = "CConquerorCommander::UpdateLeaderHumanFoot: enter booked vehicle";
				goalPipeId = -1;

				if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderAI, pBookedVehicle->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}
		}
		else if (!needEnter)
		{
			auto pBookedVehicle = GetBookedVehicle(pSquad->GetId());
			if (pBookedVehicle)
				UnbookVehicle(pSquad->GetId());

			if (playerEntered)
			{
				if (GetSpecies() == m_pConqueror->GetClientSpecies())
				{
					CryLogAlways("%s[C++][Squad Leader Id %i][Go to start and retry find vehicle]",
						STR_YELLOW, pSquad->GetLeader()->GetEntityId());
				}

				//Go to start and retry find vehicle
				commanderOrder.m_state = (eOES_Starting);
				return;
			}

			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			{
				if (g_pGameCVars->conq_debug_log_commander)
				{
					CryLogAlways("%s[C++][Squad Leader Id %i][Goto goal on Foot][Vehicle: profitable %i, playerEntered %i, isNear %i, bigDistToGoal %i]",
						STR_YELLOW, pSquad->GetLeader()->GetEntityId(), vehIsProfitable, playerEntered, vehicleIsNear, bigGoalDist);
				}
			}

			//1.5 Leader goto goal pos on foot
			commanderOrder.m_state = eOES_InMovingToTarget;
		}
		//Go to 1.4 Leader goto goal pos on vehicle
		//Now update being in the UpdateLeaderHumanVehicle() function
	}
	break;
	case eOES_InMovingToTarget:
	{
		//1.5 Leader go to goal pos on foot
		//Members following leader

		//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
		//pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

		//const auto actualRefPos = pLeaderPipe->GetRefPoint()->GetPos();
		//const float goalDist = (goalPosition - leaderPos).GetLength();

		const auto leaderNeedToMove = TOS_Distance::IsBigger(pLeaderActor, pGoalArea->GetAIAnchor(), FOOT_GOAL_DIST_THRESHOLD);
		const auto leaderGoalMoveDone = !leaderNeedToMove;

		if (leaderNeedToMove)
		{
			desiredGoalName = "action_goto0";
			actionName = "conqueror_goto_a0_d0_r3";
			flag = eAAEF_JoinCombatPauseAction;
			solution = "CConquerorCommander::UpdateLeaderHumanFoot: goto goal entity";
			goalPipeId = -1;

			if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
				TOS_AI::ExecuteAIAction(pLeaderAI, pGoalArea->GetAIAnchor(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);

			auto members = pSquad->GetAllMembers();
			for (auto& memInstance : members)
			{
				DEFINE_MEMBER_ACTOR(memInstance);

				SOrderInfo order;
				MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance);
				//order.safeFly = true;
				//order.targetId = pLeaderActor->GetEntityId();
				//order.targetPos = pLeaderActor->GetEntity()->GetWorldPos();
				//order.type = eSO_FollowLeader;
				//order.stepActions[step2] = "conqueror_goto_a0_d3_r3";
				//order.stepActions[step3] = "squad_blank_action";
				//order.ignoreFlag = eOICF_IgnoreCombatWhenGotoTarget;

				//SOrderInfo currentOrder;
				//memInstance.GetOrderInfo(currentOrder, false);
				//if (currentOrder.type != eSO_FollowLeader)
				//{
				//	pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);
				//}
			}

			//if (actualRefPos != goalPosition)
				//TOS_AI::SetRefPoint(pLeaderActor, goalPosition);

			//	if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
			//		TOS_AI::SelectPipe(pLeaderActor, GOALPIPEID_ORDER_GOTO, "tos_commander_ord_goto", "Leader need moving to target area");
		}
		else if (leaderGoalMoveDone)
		{
			//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
			//	pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

			//pLeaderOrder->SetState(eOES_PerformingAction);
			commanderOrder.m_state = (eOES_PerformingAction);
			//go to 1.6 defending strategic zone when leader on foot
		}

	}
	break;
	case eOES_PerformingAction:
	{
		//1.6 Defending strategic zone when leader on foot
		
		//Get cover search radius
		auto anchorRadius = 10.0f;
		if (pGoalArea)
			anchorRadius = TOS_AI::GetAnchorRadius(pGoalArea->GetAIAnchor());

		auto members = pSquad->GetAllMembers();
		for (auto& memInstance : members)
		{
			DEFINE_MEMBER_ACTOR(memInstance);

			if (TOS_Vehicle::GetSeatWeaponCount(pMemAct) > 0 && !TOS_Vehicle::ActorIsDriver(pMemAct))
			{
				SOrderInfo order;
				MEMBER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, memInstance);
			}
			else
			{
				SOrderInfo order;
				order.safeFly = true;

				order.targetId = 0;
				order.targetPos = goalPosition;
				order.targetRadius = anchorRadius;
				order.type = eSO_ConqSearchCoverAroundArea;
				order.stepActions[step2] = "conqueror_goto_a0_d0_r3";
				order.stepActions[step3] = "squad_hiding_in_cover";
				order.ignoreFlag = 0;

				SOrderInfo currentOrder;
				memInstance.GetOrderInfo(currentOrder, false);
				if (currentOrder.type != eSO_ConqSearchCoverAroundArea)
				{
					pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);
				}
			}
		}

		if (pLeaderActor->GetHealth() > 0)
		{
			auto hideId = m_pSquadSystem->GetBookedHideSpot(pLeaderActor);
			if (hideId == 0)
			{
				const char* cause = "CConquerorCommander::UpdateLeaderHumanFoot: leader book hidespot (search cover)";
				hideId = m_pSquadSystem->BookFreeHideSpot(pLeaderActor, goalPosition, anchorRadius, cause);
			}

			auto pHidespot = GET_ENTITY(hideId);
			if (!pHidespot)
			{
				//If the AI did not find a place to hide, 
				//then it will go looking for the enemy around

				actionName = "squad_search_enemy";
				flag = eAAEF_JoinCombatPauseAction;
				solution = "CConquerorCommander::UpdateLeaderHumanFoot: search enemy";
				goalPipeId = -1;

				if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderAI, pGoalArea->GetAIAnchor(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}
			else
			{
				if (TOS_Distance::IsBigger(pLeaderActor, pHidespot, 1.5f))
				{
					//actionFlag = eAAEF_IgnoreCombatDuringAction;
					desiredGoalName = "action_goto0";
					actionName = "conqueror_goto_a0_d0_r3";
					solution = "CConquerorCommander::UpdateLeaderHumanFoot conq Search cover goto target";
					flag = eAAEF_JoinCombatPauseAction;
					goalPipeId = -1;

					if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
						TOS_AI::ExecuteAIAction(pLeaderAI, pHidespot, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
				}
				else
				{
					actionName = "squad_hiding_in_cover";
					solution = "CConquerorCommander::UpdateLeaderHumanFoot conq Search Cover hide in cover";
					flag = eAAEF_JoinCombatPauseAction;
					goalPipeId = -1;

					if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
						TOS_AI::ExecuteAIAction(pLeaderAI, pHidespot, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
				}
			}
		}
	}
	break;
	case eOES_Last:
	break;
	}
}

void CConquerorCommander::UpdateLeaderHumanVehicle(float frameTime, CSquad* pSquad)
{
	DEFINE_LEADER_UPDATE(pSquad);

	const auto debugLog = g_pGameCVars->conq_debug_log_commander;

	//LeaderVehAI
	auto pLeaderVeh = TOS_Vehicle::GetVehicle(pLeaderActor);
	if (!pLeaderVeh)
		return;

	auto pLeaderVehEnt = pLeaderVeh->GetEntity();

	auto pLeaderVehAI = TOS_Vehicle::GetAI(pLeaderVeh);
	if (!pLeaderVehAI)
		return;

	auto pLeaderAI = pLeaderActor->GetEntity()->GetAI();
	if (!pLeaderAI)
		return;

	const auto isDriver = TOS_Vehicle::ActorIsDriver(pLeaderActor);
	const auto vehIsInCombat = TOS_AI::IsInCombat(pLeaderVehAI);
	const auto isCar = TOS_Vehicle::IsCar(pLeaderVeh);
	const auto isPLV = TOS_Vehicle::IsPLV(pLeaderVeh);
	const auto isSea = TOS_Vehicle::IsSea(pLeaderVeh);
	const auto isTank = TOS_Vehicle::IsTank(pLeaderVeh);
	const auto isAir = TOS_Vehicle::IsAir(pLeaderVeh);

	const auto& leaderVehStats = pLeaderActor->m_vehicleStats;
	//~LeaderVehAI

	//Generic AI Action values
	const char* desiredGoalName = "";
	const char* actionName = "nullActionName";
	const float maxAlertness = 102.0f; //high prioritry
	const int goalPipeId = -1;
	auto flag = eAAEF_IgnoreCombatDuringAction;
	const char* solution = debugLog ? "CConquerorCommander::UpdateLeaderHumanVehicle:" : "";
	//~Generic AI Action values

	const auto leaderVehPos = pLeaderVehEnt->GetWorldPos();
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	const auto& clientVehicleStats = g_pControlSystem->GetClientActor()->m_vehicleStats;

	auto anchorRadius = 10.0f;
	if (pGoalArea)
		anchorRadius = TOS_AI::GetAnchorRadius(pGoalArea->GetAIAnchor());

	if (!isDriver)
		return;

	switch (commanderOrder.m_state)
	{
	default:
	break;
	case eOES_NotStarted:
	break;
	case eOES_Starting:
	{
		//1.1 Initial
		//const float lastTimeStrategyChange = currentTime - m_lastTimeStrategyChange;
		//if (lastTimeStrategyChange < 2.01f)
		//{
		//}

		const int membersInVehCount = pSquad->GetMembersCount(pLeaderVeh);
		const int membersCount = pSquad->GetAliveMembersCount();
		const int seatsCount = pLeaderVeh->GetSeatCount();

		//The driver will wait for the rest of the squad for defined seconds. 
		//After which he will start moving
		const float waitSecTimer = 15.0f;
		const bool driverWaitMembers = (currentTime - leaderVehStats.lastTimeEnter) < waitSecTimer;

		//The whole squad must get into the leader vehicle
		if ((seatsCount >= membersCount) && (membersInVehCount < membersCount) && driverWaitMembers)
		{
			//Wait for members
			return;
		}
		else
		{
			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			{
				if (debugLog)
				{
					CryLogAlways("%s[C++][Squad Leader Id %i][Driver][pLeaderOrder->SetState(eOES_InMovingToTarget)]",
						STR_YELLOW, pSquad->GetLeader()->GetEntityId());
				}
			}

			commanderOrder.m_state = eOES_InMovingToTarget;
		}
	}
	break;
	case eOES_InMovingToTarget:
	{
		const bool stucked = g_pControlSystem->IsVehicleStuck(pLeaderVeh);
		if (stucked)
		{
			if (isCar)
			{
				actionName = "squad_vehicle_exit";
				solution = debugLog ? "CConquerorCommander::UpdateLeaderHumanVehicle: Vehicle Stuck" : "";
				flag = eAAEF_IgnoreCombatDuringAction;

				if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderAI, pLeaderVehEnt, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);

				auto members = pSquad->GetAllMembers();
				for (auto& memInstance : members)
				{
					DEFINE_MEMBER_ACTOR(memInstance);

					const auto pMemVeh = TOS_Vehicle::GetVehicle(pMemAct);
					if (pMemVeh && pMemVeh == pLeaderVeh)
					{
						SOrderInfo order;
						MEMBER_EXECUTE_EXIT_VEHICLE(order, pSquad, pMemAct, memInstance, pMemVeh);
					}
				}
			}

			return;
		}

		auto members = pSquad->GetAllMembers();
		for (auto& memInstance : members)
		{
			DEFINE_MEMBER_ACTOR(memInstance);

			const auto pMemVeh = TOS_Vehicle::GetVehicle(pMemAct);
			if (pMemVeh && pMemVeh == pLeaderVeh)
			{
				if (TOS_Vehicle::GetSeatWeaponCount(pMemAct) > 0 && !TOS_Vehicle::ActorIsDriver(pMemAct))
				{
					SOrderInfo order;
					MEMBER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, memInstance);
				}
				else
				{
					SOrderInfo order;
					MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance);
				}

			}
			else if (!pMemVeh)
			{
				SOrderInfo order;
				MEMBER_EXECUTE_SEARCH_HIDESPOT(order, pSquad, pMemAct, memInstance, goalPosition, anchorRadius);
			}
			else if (pMemVeh && pMemVeh != pLeaderVeh)
			{
				SOrderInfo order;
				MEMBER_EXECUTE_EXIT_VEHICLE(order, pSquad, pMemAct, memInstance, pMemVeh);
			}
		}

		//VTOL always prefer landing spots than unload spots
		EntityId id = 0;

		const auto isVtol =	pLeaderVehEnt->GetClass()->GetName() == "US_vtol";
		if (isVtol)
		{
			auto pEntity = m_pConqueror->GetNearestLandingSpot(pLeaderVehEnt);
			if (pEntity)
				id = pEntity->GetId();
		}
		else
		{
			id = pGoalArea->GetBookedUnloadSpot(pLeaderVehEnt);
		}

		if (id == 0)
			id = pGoalArea->BookFreeUnloadSpot(pLeaderVehEnt);

		auto pSpotEntity = gEnv->pEntitySystem->GetEntity(id);
		if (pSpotEntity)
		{
			goalPosition = pSpotEntity->GetWorldPos();
			pGoalEntity = pSpotEntity;
		}

		const auto dist = isAir ? VEHICLE_GOAL_DIST_THRESHOLD + 7 : VEHICLE_GOAL_DIST_THRESHOLD;

		const auto needToMove = TOS_Distance::IsBigger(pLeaderVeh, pGoalEntity, dist);
		const auto paratroopersExist = GetParatroopsCount(pSquad, pLeaderVeh) > 0;
		const auto needUnloadParatroopers = paratroopersExist;
		const auto goalMoveDone = !needToMove && !paratroopersExist;

		if (needUnloadParatroopers)
		{
			//Not shure about this
			// 30.12.2022

			//Drive to landing pos here
			//const auto pAirLandingSpotEnt = m_pConqueror->GetNearestLandingSpot(pLeaderVehEnt);
			//const auto airLandingPos = p

			//Unload spot is goal position now because it defined in past
			//Vec3 unloadSpot = goalPosition;

			//const float distLandingSpot = (leaderVehPos - airLandingSpot).GetLength();
			//const float distUnloadSpot = (leaderVehPos - pGoalEntity).GetLength();
			//const auto landingNear = distLandingSpot < distUnloadSpot;

			const auto pUnloadEntityPoint = pGoalEntity;
			//const auto pUnloadEntityPoint = isAir ? (landingNear ? airLandingSpot : goalPosition) : goalPosition;

			//const float goalDist = (unloadPoint - leaderPos).GetLength();
			const auto dist = isAir ? VEHICLE_GOAL_DIST_THRESHOLD + 7 : VEHICLE_GOAL_DIST_THRESHOLD;
			const auto needMoveToUnload = TOS_Distance::IsBigger(pLeaderVeh, pUnloadEntityPoint, dist);
			if (needMoveToUnload)
			{
				if (m_pConqueror->CanBookUnloadSpot(pLeaderVehEnt))
					m_pConqueror->SetCanBookSpot(pLeaderVeh, false, "Need unload paratroopers");

				//desiredGoalName = "action_goto0";
				actionName = "conqueror_goto_unload_a0_d0_r3";
				solution = debugLog ? "CConquerorCommander::UpdateLeaderHumanVehicle goto unload spot" : "";
				
				flag = isAir ? eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

				if (!TOS_AI::IsExecuting(pLeaderVehAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderVehAI, pUnloadEntityPoint, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}
			else if (!needMoveToUnload)
			{
				std::vector<EntityId> paratroopers;
				GetParatroopers(pSquad, pLeaderVeh, paratroopers);

				for (auto id : paratroopers)
				{
					DEFINE_MEMBER_ACTOR_FROM_ID(id, pSquad);

					auto pMemVeh = TOS_Vehicle::GetVehicle(pMemAct);
					if (!pMemVeh)
						continue;

					const auto lastExitSeconds = currentTime - pMemAct->m_vehicleStats.lastTimeExit;
					const auto needExit = TOS_Distance::IsSmaller(pMemVeh, pGoalEntity, VEHICLE_GOAL_DIST_THRESHOLD + 7.0f) && (lastExitSeconds > 5);
					if (needExit)
					{
						SOrderInfo mainOrder;
						MEMBER_POINTER_EXECUTE_SEARCH_HIDESPOT(mainOrder, pSquad, pMemAct, pMemInstance, goalPosition, anchorRadius);

						SOrderInfo subOrder;
						MEMBER_POINTER_EXECUTE_EXIT_VEHICLE(subOrder, pSquad, pMemAct, pMemInstance, pMemVeh);
					}
				}
			}
		}
		else if (needToMove && !needUnloadParatroopers)
		{
			//desiredGoalName = "action_goto0";
			actionName = "conqueror_goto_a0_d0_r3";
			solution = debugLog ? "CConquerorCommander::UpdateLeaderHumanVehicle conq Goto target" : "";

			//if (isCar || paratroopersExist)
			//{
			//	flag = eAAEF_IgnoreCombatDuringAction;
			//}
			//else
			//{
				//flag = eAAEF_JoinCombatPauseAction;
			//}

			flag = isCar ? eAAEF_IgnoreCombatDuringAction : eAAEF_JoinCombatPauseAction;

			if (!TOS_AI::IsExecuting(pLeaderVehAI, actionName))
				TOS_AI::ExecuteAIAction(pLeaderVehAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
		}
		else if (goalMoveDone)
		{
			if (isCar)
			{
				std::vector<EntityId> members;
				pSquad->GetMembersInVehicle(pLeaderVeh, members);
				for (auto memId : members)
				{
					DEFINE_MEMBER_ACTOR_FROM_ID(memId, pSquad);

					if (TOS_Vehicle::GetSeatWeaponCount(pMemAct) > 0 && !TOS_Vehicle::ActorIsDriver(pMemAct))
					{
						SOrderInfo order;
						MEMBER_POINTER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, pMemInstance);
					}
					else
					{
						SOrderInfo order;
						MEMBER_POINTER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, pMemInstance, pLeaderActor);
					}
				}

				actionName = "squad_vehicle_exit";
				solution = debugLog ? "CConquerorCommander::UpdateLeaderHumanVehicle: On move in car to goal is done" : "";
				flag = eAAEF_IgnoreCombatDuringAction;

				if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
					TOS_AI::ExecuteAIAction(pLeaderAI, pLeaderVehEnt, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
			}

			if (!m_pConqueror->CanBookUnloadSpot(pLeaderVehEnt))
				m_pConqueror->SetCanBookSpot(pLeaderVeh, true, "Move to goal is done");

			//go to 1.7 defending strategic zone when leader driving a vehicle
			commanderOrder.m_state = eOES_PerformingAction;
		}
	}
	break;
	case eOES_PerformingAction:
	{
		std::vector<EntityId> members;
		pSquad->GetMembersNotInVehicle(pLeaderVeh, members);
		for (auto memId : members)
		{
			DEFINE_MEMBER_ACTOR_FROM_ID(memId, pSquad);

			//SOrderInfo failedOrder;
			//pMemInstance->GetFailedOrderInfo(failedOrder);

			//const auto searchFailed = (failedOrder.type == eSO_ConqSearchCoverAroundArea)/* && (currentTime - failedOrder.targetRadius < 0.1f)*/;
			//if (!searchFailed)

			//SOrderInfo currentOrder;
			//pMemInstance->GetOrderInfo(currentOrder, false);

			//const auto id = m_pSquadSystem->GetNearestFreeHidespot(goalPosition, anchorRadius);
			//if(id && currentOrder.type != eSO_SearchEnemy)
			//{
				SOrderInfo order;
				MEMBER_POINTER_EXECUTE_SEARCH_HIDESPOT(order, pSquad, pMemAct, pMemInstance, goalPosition, anchorRadius);
			//}
			//else if((id == 0 && m_pSquadSystem->GetBookedHideSpot(pMemAct) == 0) || currentOrder.type == eSO_SearchEnemy)
			//{
			//	SOrderInfo order;
			//	MEMBER_POINTER_EXECUTE_SEARCH_ENEMY(order, pSquad, pMemAct, pMemInstance, goalPosition);
			//}
		}

		members.clear();
		pSquad->GetMembersInVehicle(pLeaderVeh, members);
		for (auto memId : members)
		{
			DEFINE_MEMBER_ACTOR_FROM_ID(memId, pSquad);

			if (TOS_Vehicle::GetSeatWeaponCount(pMemAct) > 0 && !TOS_Vehicle::ActorIsDriver(pMemAct))
			{
				SOrderInfo order;
				MEMBER_POINTER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, pMemInstance);
			}
			else
			{
				SOrderInfo order;
				MEMBER_POINTER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, pMemInstance, pLeaderActor);
			}
		}

		if (isTank || isAir || isSea)
		{
			if (!pSquad->IsLeaderDetached())
			{
				SDetachedMemberData data;
				data.enableUpdate = true;

				if (isAir)
				{
					data.routineType = eDRT_AirPointSearch;
					//data.points.push_back(goalPosition);
					data.targetId = pGoalEntity->GetId();
				}
				else if (isTank)
				{	
					//27.12.2022
					//eDRT_LandPathPatrol not Implemented yet

					//data.routineType = eDRT_LandPointGuard;
					data.routineType = eDRT_LandPathPatrol;
					//data.points.push_back(leaderVehPos);
					data.targetId = pGoalEntity->GetId();

					//int random = Random();
					//data.pathName = "tank_detached_path_1";
				}
				else if (isSea)
				{
					//27.12.2022
					//eDRT_WaterPathPatrol not Implemented yet

					data.routineType = eDRT_WaterPathPatrol;
					data.pathName = "";
				}

				pSquad->MarkDetached(pLeaderActor->GetEntityId(), data);
			}
		}
	}
	break;
	}
}

void CConquerorCommander::UpdateLeaderAlien(float frameTime, CSquad* pSquad)
{
	DEFINE_LEADER_UPDATE(pSquad);

	//LeaderAI
	auto pLeaderAI = pLeaderActor->GetEntity()->GetAI();
	if (!pLeaderAI)
		return;

	const auto leaderIsInCombat = TOS_AI::IsInCombat(pLeaderAI);
	//~LeaderAI

	//Generic AI Action values
	const char* desiredGoalName = "";
	const char* actionName = "nullActionName";
	const auto maxAlertness = 102.0f; //high prioritry
	int goalPipeId = -1;
	auto flag = eAAEF_IgnoreCombatDuringAction;
	string solution = "CConquerorCommander::UpdateLeaderAlien: ";
	//~Generic AI Action values

	//Request
	auto pRARSystem = m_pConqueror->GetRAR();
	auto pLeaderRequest = pRARSystem->GetRequestFromEntity(pLeaderActor->GetEntityId());
	const auto executorId = (pRARSystem->GetAssignedExecutorId(pLeaderRequest));
	
	//Other leader data
	const auto pLeaderGrabStats = pLeaderActor->GetGrabStats();
	const auto leaderPos = pLeaderActor->GetEntity()->GetWorldPos();
	const string leaderClassName = pLeaderActor->GetEntity()->GetClass()->GetName();


	const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	const auto lastTimeRespawned = currentTime - pLeaderActor->GetActorStats()->lastTimeRespawned;

	switch (commanderOrder.m_state)
	{
	case eOES_NotStarted:
	{

	}
		break;
	case eOES_Starting:
	{
		//1.1 Initial
		const auto lastTimeStrategyChange = currentTime - m_lastTimeStrategyChange;
		if (lastTimeStrategyChange < 0.01f)
		{
			//By default squad leader and members are not detached
			if (pSquad->IsLeaderDetached())
				pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

			for (auto& member : pSquad->m_members)
			{
				if (pSquad->IsMemberDetached(member.GetId()))
					pSquad->MarkUndetached(member.GetId());
			}

			if (GetSpecies() == m_pConqueror->GetClientSpecies())
			{
				if (g_pGameCVars->conq_debug_log_commander)
				{
					CryLogAlways("%s[C++][%s CMDR][Update][On Strategy Change][Initial Squad Leader %i]",
						STR_YELLOW, GetSpeciesName(), pSquad->GetLeader()->GetEntityId());
				}
			}

			desiredGoalName = "";
			goalPipeId = -1;
			actionName = "squad_clear_action";
			solution += string("On eOES_Starting");
			if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
				TOS_AI::ExecuteAIAction(pLeaderAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
		}

		//When commander set current strategy we need regroup members and give it follow leader order
		auto members = pSquad->GetAllMembers();
		for (auto& memInstance : members)
		{
			DEFINE_MEMBER_ACTOR(memInstance);

			SOrderInfo order;
			MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance);
		}

		if (pLeaderRequest && pRARSystem->GetAssignedExecutorId(pLeaderRequest))
		{
			if (pLeaderRequest->state == eRQ_Assigned)
			{
				//Stay in place and wait grabbing
				if (TOS_AI::GetRefPoint(pLeaderActor) != leaderPos)
					TOS_AI::SetRefPoint(pLeaderActor, leaderPos);

				if (!TOS_AI::IsUsingPipe(pLeaderActor, "tos_rar_wait"))
				{
					TOS_AI::SelectPipe(pLeaderActor, 0, "tos_rar_wait", "Requester need stay in place and wait grabbing");

					//CryLogAlways("[C++][%s requestIsAssigned][wait executioner]",
						//pLeaderActor->GetEntity()->GetName());
				}

				//continue;
			}
			else if (pLeaderRequest->state == eRQ_Executing)
			{
				if (pLeaderGrabStats->IsGrabbedBy(executorId))
				{
					//Chill when is grabbed by scout
					//Leader
					if (!TOS_AI::IsUsingPipe(pLeaderActor, "do_nothing"))
						TOS_AI::SelectPipe(pLeaderActor, 0, "do_nothing", "Requester need be do nothing when is grabbed by executor");

					//CryLogAlways("[C++][%s IsGrabbedBy %i][MovingToTarget]",
						//pLeaderActor->GetEntity()->GetName(), executorId);

					//go to 1.4
					commanderOrder.SetState(eOES_InMovingToTarget);
				}

				//Chill when is grabbed by scout
				//Members
				for (auto& member : pSquad->m_members)
				{
					auto pMemberActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId()));
					if (!pMemberActor)
						continue;

					if (pMemberActor->GetGrabStats()->IsGrabbedBy(executorId))
					{
						//Cause crash in CryAISystem some times
						if (!TOS_AI::IsUsingPipe(pMemberActor, "do_nothing"))
							TOS_AI::SelectPipe(pMemberActor, 0, "do_nothing", "Requester members need be do nothing when is grabbed by executor");
					}
				}
			}
		}
		else
		{
			//If Request not exist here
			//1.2 Sending a request to transfer the customer's squad to the purpose of the order

			//const auto distToOrderGoal = (goalPosition - leaderPos).GetLength();
			//const bool bigDistanceToGoal = distToOrderGoal >= 130.0f;
			const auto needCreateRequest = TOS_Distance::IsBigger(pLeaderActor, pGoalEntity, GetCurrentStrategy()->GetSettings().m_vehicleUseDistance);
			const auto leaderId = pLeaderActor->GetEntityId();

			auto pRequesterInstance = pRARSystem->GetRequesterInstance(leaderId);
			auto pCurrentRequest = pRARSystem->GetRequestFromEntity(leaderId);
			auto executorAssigned = false;

			if (needCreateRequest)
			{
				if (!pCurrentRequest && !pRequesterInstance)
				{
					pCurrentRequest = pRARSystem->CreateRequest(leaderId, pGoalArea->GetEntityId(), eRT_AlienTaxsee);
				}
				else if (!pCurrentRequest && pRequesterInstance)
				{
					const auto lastTimeCreated = currentTime - pRequesterInstance->GetLastTime(eRQ_Created);
					if (lastTimeCreated > 10.0f)
					{
						pCurrentRequest = pRARSystem->CreateRequest(leaderId, pGoalArea->GetEntityId(), eRT_AlienTaxsee);
					}
				}
			}

			const auto lastTimeFailed = pCurrentRequest ? currentTime - pCurrentRequest->lastTimeFailed : 0.0f;

			if (pCurrentRequest)
			{
				if (pRequesterInstance && pRequesterInstance->GetRequestId() == pCurrentRequest->getId())
				{
					if (pCurrentRequest->state == eRQ_Created)
					{
						const auto createdTime = pRequesterInstance->GetLastTime(eRQ_Created);
						const auto lastTimeCreated = currentTime - createdTime;

						if (lastTimeCreated > 3.0f)
						{
							executorAssigned = false;
							pRARSystem->FailRequest(pCurrentRequest->getId(), 1);
						}
					}
					else if ((pCurrentRequest->state == eRQ_Failed || pCurrentRequest->state == eRQ_FailedByExecutorKilled) && lastTimeFailed < 0.1f)
					{
						//CryLogAlways("[C++][Aliens][Set Leader %i to Moving To Target on Foot]", pSquad->GetLeader()->GetEntityId());
						commanderOrder.SetState(eOES_InMovingToTarget);
					}
				}
			}
			else
			{
				//CryLogAlways("[C++][Aliens][Set Leader %i to Moving To Target on Foot]", pSquad->GetLeader()->GetEntityId());
				commanderOrder.SetState(eOES_InMovingToTarget);
			}

			if (pCurrentRequest)
			{
				const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
				const auto lastTimeCreated = currentTime - pCurrentRequest->lastTimeCreated;

				if (pCurrentRequest->state == eRQ_Created && lastTimeCreated > 2.9f)
				{
					executorAssigned = false;
				}
				else if (pCurrentRequest->state == eRQ_Assigned)
				{
					executorAssigned = true;
				}
			}

			const auto useExecutorToMove = needCreateRequest && executorAssigned;
			if (!useExecutorToMove)
			{
				commanderOrder.SetState(eOES_InMovingToTarget);
			}
		}
	}
		break;
	case eOES_InMovingToTarget:
	{
		//1.5 go to target on foot
		if (leaderClassName == "Scout")
		{

		}
		else if (leaderClassName == "Trooper")
		{
			if (!pLeaderGrabStats->IsGrabbedBy(executorId))
			{
				//if (pSquad->GetLastAllOrder() != eSO_FollowLeader)
					//pSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

				//const auto actualRefPos = pLeaderPipe->GetRefPoint()->GetPos();
				//const float goalDist = (goalPosition - leaderPos).GetLength();
				
				//const auto leaderNeedToMove = goalDist > goalDistTreshold;
				const auto leaderNeedToMove = TOS_Distance::IsBigger(pLeaderActor, pGoalArea->GetAIAnchor(), FOOT_GOAL_DIST_THRESHOLD);
				const auto leaderGoalMoveDone = !leaderNeedToMove;

				if (leaderNeedToMove)
				{
					//if (actualRefPos != goalPosition)
						//TOS_AI::SetRefPoint(pLeaderActor, goalPosition);

					//if (pLeaderPipe->GetGoalPipeId() != GOALPIPEID_ORDER_GOTO)
					//	TOS_AI::SelectPipe(pLeaderActor, GOALPIPEID_ORDER_GOTO, "tos_commander_ord_goto", "Leader need moving to target area");
				
					desiredGoalName = "action_goto0";
					actionName = "conqueror_goto_a0_d0_r3";
					flag = eAAEF_JoinCombatPauseAction;
					solution += string("goto goal entity");
					goalPipeId = -1;

					if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
						TOS_AI::ExecuteAIAction(pLeaderAI, pGoalArea->GetAIAnchor(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
				}
				else if (leaderGoalMoveDone)
				{
					
					commanderOrder.SetState(eOES_PerformingAction);
					//go to 1.6 defending strategic zone when leader on foot
				}
			}
		}
		else if (leaderClassName == "Hunter")
		{

		}
		else if (leaderClassName == "Alien")
		{

		}

	}
		break;
	case eOES_PerformingAction:
	{
		if (leaderClassName == "Scout")
		{

		}
		else if (leaderClassName == "Trooper")
		{
			//1.6 Defending strategic zone when leader on foot

			//Get cover search radius
			auto anchorRadius = 10.0f;
			if (pGoalArea)
				anchorRadius = TOS_AI::GetAnchorRadius(pGoalArea->GetAIAnchor());

			auto members = pSquad->GetAllMembers();
			for (auto& memInstance : members)
			{
				DEFINE_MEMBER_ACTOR(memInstance);

				if (TOS_Vehicle::GetSeatWeaponCount(pMemAct) > 0 && !TOS_Vehicle::ActorIsDriver(pMemAct))
				{
					SOrderInfo order;
					MEMBER_EXECUTE_BLANK_ACTION(order, pSquad, pMemAct, memInstance);
				}
				else
				{
					SOrderInfo order;
					order.safeFly = true;

					order.targetId = 0;
					order.targetPos = goalPosition;
					order.targetRadius = anchorRadius;
					order.type = eSO_ConqSearchCoverAroundArea;
					order.stepActions[step2] = "conqueror_goto_a0_d0_r3";
					order.stepActions[step3] = "squad_hiding_in_cover";
					order.ignoreFlag = 0;

					SOrderInfo currentOrder;
					memInstance.GetOrderInfo(currentOrder, false);
					if (currentOrder.type != eSO_ConqSearchCoverAroundArea)
					{
						pSquad->ExecuteOrder(pSquad->GetMemberInstance(pMemAct), order, eEOF_ExecutedByCommander);
					}
				}
			}

			if (pLeaderActor->GetHealth() > 0)
			{
				auto hideId = m_pSquadSystem->GetBookedHideSpot(pLeaderActor);
				if (hideId == 0)
				{
					const char* cause = "CConquerorCommander::UpdateLeaderAlien: leader book hidespot (search cover)";
					hideId = m_pSquadSystem->BookFreeHideSpot(pLeaderActor, goalPosition, anchorRadius, cause);
				}

				auto pHidespot = GET_ENTITY(hideId);
				if (!pHidespot)
				{
					//If the AI did not find a place to hide, 
					//then it will go looking for the enemy around

					actionName = "squad_search_enemy";
					flag = eAAEF_JoinCombatPauseAction;
					solution = "CConquerorCommander::UpdateLeaderAlien: search enemy";
					goalPipeId = -1;

					if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
						TOS_AI::ExecuteAIAction(pLeaderAI, pGoalArea->GetAIAnchor(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
				}
				else
				{
					if (TOS_Distance::IsBigger(pLeaderActor, pHidespot, 1.5f))
					{
						//actionFlag = eAAEF_IgnoreCombatDuringAction;
						desiredGoalName = "action_goto0";
						actionName = "conqueror_goto_a0_d0_r3";
						solution = "CConquerorCommander::UpdateLeaderAlien Search cover goto target";
						flag = eAAEF_JoinCombatPauseAction;
						goalPipeId = -1;

						if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
							TOS_AI::ExecuteAIAction(pLeaderAI, pHidespot, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
					}
					else
					{
						actionName = "squad_hiding_in_cover";
						solution = "CConquerorCommander::UpdateLeaderAlien Search Cover hide in cover";
						flag = eAAEF_JoinCombatPauseAction;
						goalPipeId = -1;

						if (!TOS_AI::IsExecuting(pLeaderAI, actionName))
							TOS_AI::ExecuteAIAction(pLeaderAI, pHidespot, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
					}
				}
			}
		}
		else if (leaderClassName == "Hunter")
		{

		}
		else if (leaderClassName == "Alien")
		{

		}

	}
		break;
	}
}

//void CConquerorCommander::UpdateTactics(ECommanderTactics tactics)
//{
//	if (tactics == eCT_Expansion)
//	{
//
//	}
//}

ESpeciesType CConquerorCommander::GetSpecies() const
{
	return m_species;
}

//ECommanderTactics CConquerorCommander::GetCurrentTactics()
//{
//	return m_currentTactics;
//}
//
//ECommanderTactics CConquerorCommander::GetPreviousTactics()
//{
//	return m_prevTactics;
//}

CCommanderOrder* CConquerorCommander::GetCommanderOrder(EntityId leaderId)
{
	return &m_commanderOrdersMap[leaderId];
}

CCommanderOrder* CConquerorCommander::GetCommanderOrder(IActor* pLeader)
{
	if (!pLeader)
		return nullptr;

	return &m_commanderOrdersMap[pLeader->GetEntityId()];
}

//CCommanderOrder* CConquerorCommander::RequestOrderFromTactics(CSquad* pSquad, ECommanderTactics tactics)
//{
//	if (!pSquad)
//		return nullptr;
//
//	auto pLeader = pSquad->GetLeader();
//	if (!pLeader)
//		return nullptr;
//
//	auto& leaderPos = pLeader->GetEntity()->GetWorldPos();
//
//	if (tactics == eCT_Expansion)
//	{
//		auto pArea = GetNearestEnemyCapturableArea(leaderPos, eABF_NoMatter, false);
//
//		CRY_ASSERT_TRACE(pArea, "[Conqueror Commander of %s Request Order From Tactics][NULL ENEMY AREA]", m_pConqueror->GetSpeciesName(m_species));
//
//		if (!pArea)
//			return nullptr;
//
//		auto& areaPos = pArea->GetEntity()->GetWorldPos();
//
//		auto pAnchor = pArea->GetAIAchor();
//		if (pAnchor)
//			areaPos = pAnchor->GetWorldPos();
//	
//		auto order = CCommanderOrder(eCO_GoTo, areaPos, eOES_InMovingToTarget, pArea->GetEntityId());
//
//		return &order;
//	}
//
//	return nullptr;
//}

int CConquerorCommander::GetEnemyCountAroundArea(CStrategicArea* pArea, bool vehicle)
{
	int count = 0;

	if (!pArea)
		return count;

	auto pAnchor = pArea->GetAIAnchor();
	if (!pAnchor)
		return count;

	const float radius = TOS_AI::GetAnchorRadius(pAnchor);
	const auto& anchorPos = pAnchor->GetWorldPos();

	auto it = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while (auto pActor = it->Next())
	{
		if (pActor->GetHealth() <= 0)
			continue;

		if (!pActor->GetEntity()->GetAI())
			continue;

		if (!pActor->GetLinkedVehicle() && vehicle)
			continue;

		const auto species = (ESpeciesType)TOS_AI::GetSpecies(pActor->GetEntity()->GetAI(), false);
		if (species == m_species)
			continue;

		const auto& pos = pActor->GetEntity()->GetWorldPos();
		const float distance = (anchorPos - pos).GetLength();

		if (distance > radius)
			continue;

		count++;
	}

	return count;
}

CStrategicArea* CConquerorCommander::GetNearestArea(const Vec3& pos, const string& areaStatus, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	return m_pConqueror->GetNearestStrategicArea(pos, areaStatus, eAGSF_EnabledAndCapturable, m_species, busyFlags, areaFlag);
}

//CCapturableArea* CConquerorCommander::GetNearestArea(const Vec3& pos, bool friendly, EAreaFlag areaFlag, int maxAssignedSquads)
//{
//	std::vector<CCapturableArea*> allAreasVector;
//	for (auto pArea : m_pConqueror->m_capturableAreas)
//	{
//		if ((pArea->GetSpecies() == m_species) && friendly)
//		{
//			if (!pArea->IsEnabled())
//				continue;
//
//			if (!pArea->IsCapturable())
//				continue;
//
//			const auto& flags = pArea->GetFlags();
//
//			if (!stl::find(flags, areaFlag))
//				continue;
//
//			if (maxAssignedSquads >= 0)
//			{
//				const auto assignedCount = GetAssignedSquadsForArea(pArea)->size();
//				if (assignedCount > maxAssignedSquads)
//					continue;
//			}
//
//			//if (busyFlags == eABF_AreaIsSquadTarget)
//			//{
//			//	const auto pSquads = GetAssignedSquadsForArea(pArea);
//			//	if (!pSquads || (pSquads && pSquads->size() == 0))
//			//		continue;
//			//}
//			//else if (busyFlags == eABF_AreaIsHaveEnemyGuards)
//			//{
//			//	if (GetEnemyCountAroundArea(pArea, false) <= 0)
//			//		continue;
//			//}
//			//else if (busyFlags == eABF_AreaIsNOTSquadTarget)
//			//{
//			//	const auto pSquads = GetAssignedSquadsForArea(pArea);
//			//	if (!pSquads || (pSquads && pSquads->size() == 0))
//			//		continue;
//			//}
//			//else if (busyFlags == eABF_AreaIsNOTHaveEnemyGuards)
//			//{
//			//	if (GetEnemyCountAroundArea(pArea, false) > 0)
//			//		continue;
//			//}
//
//
//			allAreasVector.push_back(pArea);
//		}
//	}
//
//	if (allAreasVector.size() == 0)
//		return nullptr;
//
//	auto pFirstArea = allAreasVector[0];
//	if (!pFirstArea)
//	{
//		//CryLogAlways("[C++][ERROR][Conqueror System Get Nearest Enemy Area][NULL FIRST AREA]");
//		return nullptr;
//	}
//
//	//CryLogAlways("[C++][Species %s][Enemy Areas Count %i]", m_pConqueror->GetSpeciesName(m_species), enemyAreasVector.size());
//
//	Vec3 firstAreaPos = pFirstArea->GetEntity()->GetWorldPos();
//	float minDistance = (firstAreaPos - pos).GetLength();
//
//	//CryLogAlways("[C++][Species %s][Distance to first area %1.f]", m_pConqueror->GetSpeciesName(m_species), minDistance);
//
//	CCapturableArea* pNearestArea = nullptr;
//
//	for (auto pArea : allAreasVector)
//	{
//		auto& areaPos = pArea->GetEntity()->GetWorldPos();
//		float distance = (areaPos - pos).GetLength();
//
//		if (distance < minDistance)
//		{
//			minDistance = distance;
//			pNearestArea = pArea;
//		}
//	}
//
//	if (pNearestArea == nullptr)
//		pNearestArea = pFirstArea;
//
//	return pNearestArea;
//}

CStrategicArea* CConquerorCommander::GetNearestArea(const Vec3& pos, const string& areaStatus, bool needMostPrioritry, int maxAssignedSquads, EAreaGameStatusFlag gameStatus)
{
	std::vector<CStrategicArea*> allAreasVector;
	std::vector<CStrategicArea*> mostPrioritryAreasVector;
	CStrategicArea* pNearestArea = nullptr;

	for (auto pArea : m_pConqueror->m_strategicAreas)
	{
		if (maxAssignedSquads > 0)
		{
			const auto pSquads = GetAssignedSquadsForArea(pArea);
			if (!pSquads)
				continue;

			if (pSquads->size() >= maxAssignedSquads)
				continue;
		}

		if (areaStatus == OWNED)
		{
			if (pArea->GetSpecies() != m_species)
				continue;
		}
		else if (areaStatus == HOSTILE)
		{
			if (pArea->GetSpecies() == m_species)
				continue;
		}

		if (gameStatus == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}
		else if (gameStatus == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameStatus == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}

		allAreasVector.push_back(pArea);
	}

	if (allAreasVector.size() == 0)
		return nullptr;

	auto pFirstArea = allAreasVector[0];
	if (!pFirstArea)
		return nullptr;

	float minDistance = (pFirstArea->GetWorldPos() - pos).GetLength();

	//CryLogAlways("[C++][%s][First Area %s: Min Distance %1.f]", GetSpeciesName(), pFirstArea->GetEntity()->GetName(), minDistance);

	for (auto pArea : allAreasVector)
	{
		if (needMostPrioritry)
		{
			const auto& areaFlags = pArea->GetFlags();
			bool areaIsMostPrioritry = false;

			//Get Max Prioritry Flags
			const auto& mostPriorityFlags = GetMostPriorityFlags(areaStatus);
			const auto mostCount = mostPriorityFlags.size();

			//Get Prioritry from area flags and compare it
			for (auto flag : areaFlags)
			{
				if (stl::find(mostPriorityFlags, flag))
				{
					areaIsMostPrioritry = true;
					break;
				}
			}

			if (areaIsMostPrioritry)
			{
				mostPrioritryAreasVector.push_back(pArea);
				//CryLogAlways("$3[C++][%s Commander][One of Priority Area is %s]",
				//	GetSpeciesName(), pArea->GetEntity()->GetName());
			}
			else
			{
				//CryLogAlways("$6[C++][%s Commander][Area %s is not Priority Area]",
				//	GetSpeciesName(), pArea->GetEntity()->GetName());
			}
		}
		else
		{
			const auto areaPos = pArea->GetWorldPos();
			const float distance = (areaPos - pos).GetLength();

			if (distance < minDistance)
			{
				minDistance = distance;
				pNearestArea = pArea;
			}
		}
	}

	for (auto pArea : mostPrioritryAreasVector)
	{
		const auto areaPos = pArea->GetWorldPos();
		const float distance = (areaPos - pos).GetLength();

		//CryLogAlways("$3[C++][%s Commander][mostPrioritryAreasVector][Area %s: Distance to Leader %1.f / Min distance: %1.f]",
			//GetSpeciesName(), pArea->GetEntity()->GetName(), distance, minDistance);

		if (distance < minDistance)
		{
			minDistance = distance;
			pNearestArea = pArea;
		}
	}

	if (pNearestArea == nullptr)
	{
		//CryLogAlways("[C++][%s][pNearestArea is null -> select First Area %s]", GetSpeciesName() ,pFirstArea ? pFirstArea->GetEntity()->GetName() : "NullName");
		pNearestArea = pFirstArea;
	}

	return pNearestArea;
}

CStrategicArea* CConquerorCommander::GetNearestArea(const Vec3& pos, ESpeciesType targetSpecies, bool mostPrioritry, int maxAssignedSquads, EAreaGameStatusFlag gameStatus)
{
	std::vector<CStrategicArea*> allAreasVector;
	std::vector<CStrategicArea*> mostPrioritryAreasVector;
	CStrategicArea* pNearestArea = nullptr;

	for (auto pArea : m_pConqueror->m_strategicAreas)
	{
		/*if (areaStatus == OWNED)
		{
			if (pArea->GetSpecies() != m_species)
				continue;
		}
		else if (areaStatus == HOSTILE)
		{
			if (pArea->GetSpecies() == m_species)
				continue;
		}*/

		if (pArea->GetSpecies() != targetSpecies)
			continue;

		if (gameStatus == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}
		else if (gameStatus == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameStatus == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}

		if (maxAssignedSquads > 0)
		{
			const int assignedCount = GetAssignedSquadsForArea(pArea)->size();
			if (assignedCount >= maxAssignedSquads)
				continue;
		}

		allAreasVector.push_back(pArea);
	}

	if (allAreasVector.size() == 0)
		return nullptr;

	auto pFirstArea = allAreasVector[0];
	if (!pFirstArea)
	{
		return nullptr;
	}

	Vec3 firstAreaPos = pFirstArea->GetWorldPos();
	float minDistance = (firstAreaPos - pos).GetLength();

	for (auto pArea : allAreasVector)
	{
		if (mostPrioritry)
		{
			const auto& areaFlags = pArea->GetFlags();
			bool areaIsMostPrioritry = false;

			//Get Max Prioritry Flags
			const auto& mostPriorityFlags = GetMostPriorityFlags(targetSpecies);
			const auto mostCount = mostPriorityFlags.size();

			//Get Prioritry from area flags and compare it
			for (auto flag : areaFlags)
			{
				if (stl::find(mostPriorityFlags, flag))
				{
					areaIsMostPrioritry = true;
					break;
				}
			}

			if (areaIsMostPrioritry)
			{
				mostPrioritryAreasVector.push_back(pArea);
				//CryLogAlways("$3[C++][%s Commander][One of Priority Area is %s]",
				//	GetSpeciesName(), pArea->GetEntity()->GetName());
			}
			else
			{
				//CryLogAlways("$6[C++][%s Commander][Area %s is not Priority Area]",
				//	GetSpeciesName(), pArea->GetEntity()->GetName());
			}
		}
		else
		{
			auto areaPos = pArea->GetWorldPos();
			float distance = (areaPos - pos).GetLength();

			if (distance < minDistance)
			{
				minDistance = distance;
				pNearestArea = pArea;
			}
		}
	}

	for (auto pArea : mostPrioritryAreasVector)
	{
		auto areaPos = pArea->GetWorldPos();
		float distance = (areaPos - pos).GetLength();

		if (distance < minDistance)
		{
			minDistance = distance;
			pNearestArea = pArea;
		}
	}

	if (pNearestArea == nullptr)
		pNearestArea = pFirstArea;

	return pNearestArea;
}

CStrategicArea* CConquerorCommander::GetNearestArea(const Vec3& pos, ESpeciesType targetSpecies, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	return m_pConqueror->GetNearestStrategicArea(pos, targetSpecies, eAGSF_EnabledAndCapturable, m_species, busyFlags, areaFlag);
}

CStrategicArea* CConquerorCommander::GetArea(EAreaGameStatusFlag gameFlag, const string& areaStatus, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	for (auto pArea : m_pConqueror->m_strategicAreas)
	{
		if (areaStatus == OWNED)
		{
			if (pArea->GetSpecies() != m_species)
				continue;
		}
		else if (areaStatus == HOSTILE)
		{
			if (pArea->GetSpecies() == m_species)
				continue;
		}

		if (gameFlag == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameFlag == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameFlag == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}

		if (busyFlags == eABF_AreaIsSquadTarget)
		{
			const auto pSquads = GetAssignedSquadsForArea(pArea);
			if (!pSquads || (pSquads && pSquads->size() == 0))
				continue;
		}
		else if (busyFlags == eABF_AreaIsHaveEnemyGuards)
		{
			if (GetEnemyCountAroundArea(pArea, false) <= 0)
				continue;
		}
		else if (busyFlags == eABF_AreaIsNOTSquadTarget)
		{
			const auto pSquads = GetAssignedSquadsForArea(pArea);
			if (!pSquads || (pSquads && pSquads->size() == 0))
				continue;
		}
		else if (busyFlags == eABF_AreaIsNOTHaveEnemyGuards)
		{
			if (GetEnemyCountAroundArea(pArea, false) > 0)
				continue;
		}

		const auto& flags = pArea->GetFlags();

		if (!stl::find(flags, areaFlag))
			continue;

		return pArea;
	}

	return nullptr;
}

const char* CConquerorCommander::GetSpeciesName()
{
	return m_pConqueror->GetSpeciesName(m_species);
}

void CConquerorCommander::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->Add(m_lastStrategyName);
	s->Add(m_areaFlagPriorities);
	s->Add(m_commanderOrdersMap);
	s->Add(m_currentStrategyGoals);
	s->Add(m_currentStrategyName);
	s->Add(m_squadBookedVehicle);
	s->Add(m_squadParatroopers);
	s->Add(m_strategiesNamesHistory);
	s->Add(m_targettedAreasMap);
}

void CConquerorCommander::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		//ser.EnumValue("currentTactics", m_currentTactics, eCT_FirstTactic, eCT_LastTactic);
		//ser.EnumValue("previousTactics", m_prevTactics, eCT_FirstTactic, eCT_LastTactic);
		ser.EnumValue("species", m_species, eST_FirstPlayableSpecies, eST_LastPlayableSpecies);
		//ser.Value("subordinateSquads", m_subordinateSquadIds);
		//ser.MappedValue("leadersMap", m_leaderOrdersMap, m_leaderOrdersMap);
	}
}

void CConquerorCommander::OnAreaCaptured(const CStrategicArea* pArea)
{
	if (!pArea)
		return;

	for (auto& pair : m_commanderOrdersMap)
	{
		auto& order = pair.second;

		const auto isCurrent = order.m_targetId == pArea->GetEntityId();
		const auto isAttack = order.m_type == eCO_Attack;

		if (isCurrent && isAttack)
			order.m_type = eCO_Defend;
	}

	//Работает в целом, но нужно определять когда ИИ просто ожидает в тот момент, 
	//когда его точки находятся под атакой и переопределять текущую стратегию
	m_currentStrategyGoals[eSGT_CapturedAreasCount] += 1;

	if (m_pConqueror->ReadStrategyGoals(this))
		RequestNewStrategy(m_pConqueror->m_gameStatus == eGS_GameStart);
}

void CConquerorCommander::OnAreaLost(const CStrategicArea* pArea)
{
	m_currentStrategyGoals[eSGT_CapturedAreasCount] -= 1;

	if (m_currentStrategyGoals[eSGT_CapturedAreasCount] < 0)
		m_currentStrategyGoals[eSGT_CapturedAreasCount] = 0;
}

void CConquerorCommander::OnAreaUnderAttack(const CStrategicArea* pArea)
{

}

void CConquerorCommander::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
	if (!pActor || !pVehicle)
		return;

	//Define the paratrooper
	auto pSquad = m_pSquadSystem->GetSquadFromMember(pActor, true);
	if (!pSquad)
		return;

	const bool isOurSquad = stl::find(m_subordinateSquadIds, pSquad->GetId());
	if (!isOurSquad)
		return;

	const bool vehIsAir = TOS_Vehicle::IsAir(pVehicle);
	const bool vehIsPLV = TOS_Vehicle::IsPLV(pVehicle);

	const bool isPassenger = TOS_Vehicle::ActorIsPassenger(pActor);
	const bool isLeader = pSquad->GetLeader() == pActor;

	if (isPassenger && !isLeader)
	{
		if (vehIsAir || vehIsPLV)
		{
			MarkParatrooper(pSquad, pActor->GetEntityId());
			OnParatrooperEnter(pVehicle, pActor);
		}
	}

	//Is leader condition cause bug #55
	if (/*isLeader || */pActor == g_pControlSystem->GetClientActor())
	{
		UnbookVehicle(pVehicle);
	}

	if (g_pGameCVars->conq_debug_log)
	{
		CryLogAlways("%s[C++][Vehicle Enter][Actor: %s][Vehicle: %s]",
			STR_YELLOW, pActor->GetEntity()->GetName(), pVehicle->GetEntity()->GetName());
	}
}

void CConquerorCommander::OnExitVehicle(IActor* pActor)
{
	if (!pActor)
		return;

	//undefine the paratrooper when leader exit from vehicle
	auto pSquad = m_pSquadSystem->GetSquadFromMember(pActor, true);
	if (!pSquad)
		return;

	//const bool isOurSquad = stl::find(m_subordinateSquadIds, pSquad->GetId());
	const bool isOurSquad = pSquad->GetSpecies() == m_species;
	if (!isOurSquad)
		return;

	if (IsParatrooper(pActor->GetEntityId()))
	{
		pSquad = m_pSquadSystem->GetSquadFromMember(pActor, false);
		if (!pSquad)
			return;

		auto pMemberInstance = pSquad->GetMemberInstance(pActor);
		if (!pMemberInstance)
			return;

		auto pMemberEntity = gEnv->pEntitySystem->GetEntity(pMemberInstance->GetId());
		if (!pMemberEntity)
			return;

		//if (pSquad->GetLastAllOrder() == eSO_Conq_Search_Cover_Around_Area)
		//	pMemberInstance->SetSearchCoverRadius(30);

		//pMemberInstance->SetCurrentOrder(eSO_Conq_Search_Cover_Around_Area);
		//pSquad->ExecuteOrder(pSquad->GetLastAllOrder(), pMemberInstance, eEOF_ExecutedByAI);

		//CryLogAlways("[C++][%s][Squad Leader %s][Member %s: execute order %s]",
			//GetSpeciesName(), pSquad->GetLeader()->GetEntity()->GetName(), pMemberEntity->GetName(), GetString(pSquad->GetLastAllOrder()));		
	}

	if (IsHaveParatroopers(pSquad) && (pActor == pSquad->GetLeader()))
	{
		for (auto& member : pSquad->m_members)
			UnmarkParatrooper(member.GetId());
	}
	if (g_pGameCVars->conq_debug_log)
	{
		CryLogAlways("%s[C++][Vehicle Exit][Actor: %s]",
			STR_YELLOW, pActor->GetEntity()->GetName());
	}
}

void CConquerorCommander::OnVehicleDestroyed(IVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	UnbookVehicle(pVehicle);

	for (auto pChannel : m_pConqueror->m_conquerorChannels)
	{
		auto pActor = static_cast<CActor*>(pChannel->GetActor());
		if (!pActor)
			continue;

		if (pActor->m_vehicleStats.lastOperatedVehicleId == pVehicle->GetEntityId())
		{
			auto pSquad = m_pSquadSystem->GetSquadFromLeader(pActor);
			if (!pSquad)
				continue;

			for (auto& member : pSquad->GetAllMembers())
				UnmarkParatrooper(member.GetId());
		}
	}
}

void CConquerorCommander::OnVehicleStuck(IVehicle* pVehicle, bool stuck)
{
	if (!pVehicle)
		return;

	//if (stuck)
	//{
	//	if (m_pConqueror->IsHaveUnloadSpot(pVehicle->GetEntity()))
	//		m_pConqueror->UnbookUnloadSpot(pVehicle->GetEntity());
	//}
}

void CConquerorCommander::OnActorDeath(IActor* pActor)
{
	if (!pActor)
		return;

	auto pSquad = m_pSquadSystem->GetSquadFromLeader(pActor);
	if (!pSquad)
		return;

	UnbookVehicle(pSquad->GetId());

	//undefine the paratrooper when leader death
	//const bool isOurSquad = stl::find(m_subordinateSquadIds, pSquad->GetId());

	const bool isOurSquad = pSquad->GetSpecies() == GetSpecies();
	if (!isOurSquad)
		return;

	if (IsHaveParatroopers(pSquad) && (pActor == pSquad->GetLeader()))
	{
		for (auto& member : pSquad->m_members)
			UnmarkParatrooper(member.GetId());
	}
}

void CConquerorCommander::OnSquadCallBackup(CSquad* pSquad)
{

}

void CConquerorCommander::OnNewStrategicTarget(CSquad* pSquad, CStrategicArea* pArea)
{
	//if (!pSquad)
	//	return;

	//auto pSquadLeader = pSquad->GetLeader();
	//if (!pSquadLeader)
	//	return;

	//if (!pArea)
	//	return;

	//const auto squadId = pSquad->GetId();
	//const auto leaderPos = pSquadLeader->GetEntity()->GetWorldPos();
	//const auto thresholdTargetDist = 200.0f;

	////Off squad order update
	//stl::push_back_unique(m_disableSquadUpdate, squadId);

	////The nearby vehicle is exist?
	//const uint flags = eVGF_Nearest | eVGF_MustHaveGun;
	//const int seatCount = pSquad->GetMembersCount();

	//auto pVeh = GetFreeVehicle(leaderPos, flags, seatCount);
	//if (!pVeh)
	//{
	//	//On squad order update
	//	stl::find_and_erase(m_disableSquadUpdate, pSquad->GetId());
	//}
	//else
	//{
	//	const auto targetPos = pArea->GetEntity()->GetWorldPos();
	//	const auto distToTarget = (targetPos - leaderPos).GetLength();

	//	//If Distance to target is very long
	//	if (distToTarget > thresholdTargetDist)
	//	{
	//		//Enter near vehicle
	//		//auto vehDistThreshold = VEHICLE_LAND_ENTER_THRESHOLD_DIST;

	//		//if (flags & eVGF_Air)
	//		//	vehDistThreshold = VEHICLE_AIR_ENTER_THRESHOLD_DIST;

	//		//const auto vehPos = pVeh->GetEntity()->GetWorldPos();
	//		//const auto distToVehicle = (vehPos - leaderPos).GetLength();

	//		const auto freeSeatIndex = TOS_Vehicle::RequestFreeSeatIndex(pVeh);

	//		auto pSeat = pVeh->GetSeatById(freeSeatIndex);
	//		if (pSeat)
	//			pSeat->Enter(pSquadLeader->GetEntityId(), true);
	//	}
	//}

	//Handle request of the get nearby vehicle to Squad Leader
	//Off update-->> If Distance to target is very long -->> enter near vehicle -->>on update
}

void CConquerorCommander::OnSquadLeaderSpawned(CSquad* pSquad)
{
	if (!pSquad)
		return;

	auto pLeaderActor = pSquad->GetLeader();
	if (!pLeaderActor)
		return;

	if (pLeaderActor == g_pControlSystem->GetClientActor())
	{
		DEFINE_STEPS;

		auto members = pSquad->GetAllMembers();
		for (auto& member : members)
		{
			auto pMemAct = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());

			SOrderInfo order;
			MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, member);
		}

	}

	if (!stl::find(m_subordinateSquadIds, pSquad->GetId()))
		return;

	if (!pSquad->HasClientLeader())
	{
		auto pOrder = GetCommanderOrder(pSquad->GetLeader());
		if (pOrder)
		{
			if (pOrder->m_state != eOES_Starting)
			{
				//BookFreeVehicle(pSquad->GetId(), 50, eVGF_Nearest, 1);

				if (m_pConqueror->GetClientSpecies() == GetSpecies())
				{
					if (g_pGameCVars->conq_debug_log_commander)
					{
						CryLogAlways("[C++][Squad Leader %i: set order state from %s to starting]",
							pSquad->GetLeader()->GetEntityId(), GetString(pOrder->m_state));
					}
				}

				const auto pOldOrder = pOrder;
				m_commanderOrdersMap[pSquad->GetLeader()->GetEntityId()] = CCommanderOrder(pOldOrder->m_type, pOldOrder->m_pos, eOES_Starting, pOldOrder->m_targetId);
			}
		}
		else
		{
			CryLogAlways("%s[C++][ERROR][OnSquadLeaderSpawned][Cause: Can not define leader %i order]",
				STR_RED, pSquad->GetLeader()->GetEntityId());
		}
	}

	pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());
	//UnbookVehicle(pSquad->GetId());
}

void CConquerorCommander::OnSquadMemberSpawned(CSquad* pSquad, CMember* pMember)
{
	if (!pSquad || !pMember)
		return;

	auto pLeaderActor = pSquad->GetLeader();
	if (!pLeaderActor)
		return;

	if (pLeaderActor == g_pControlSystem->GetClientActor())
	{
		DEFINE_STEPS;

		auto members = pSquad->GetAllMembers();
		for (auto& member : members)
		{
			auto pMemAct = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());

			SOrderInfo order;
			MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, member);
		}
	}


	if (!stl::find(m_subordinateSquadIds, pSquad->GetId()))
		return;

	//Fast enter leader's vehicle
	auto pLeaderVeh = TOS_Vehicle::GetVehicle(pSquad->GetLeader());
	if (pLeaderVeh && TOS_Vehicle::IsHaveFreeSeats(pLeaderVeh))
	{
		const int seatId = TOS_Vehicle::RequestFreeSeatIndex(pLeaderVeh);

		auto pSeat = pLeaderVeh->GetSeatById(seatId);
		if (pSeat)
		{
			auto pMemAct = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMember->GetId());

			OnParatrooperRespawned(pLeaderVeh, pMemAct);
			pSeat->Enter(pMember->GetId(), false);
		}
	}
}

void CConquerorCommander::OnSquadStartCombat(CSquad* pSquad)
{
	if (!pSquad)
		return;

	if (g_pGameCVars->conq_debug_log)
	{
		CryLogAlways("%s[C++][%s][Squad %i Start Combat]",
			STR_CYAN, GetSpeciesName(), pSquad->GetId());
	}

	UnbookVehicle(pSquad->GetId());

	auto pVeh = TOS_Vehicle::GetVehicle(pSquad->GetLeader());
	if (pVeh && pSquad->IsLeaderInCombat() && TOS_Vehicle::ActorIsDriver(pSquad->GetLeader()))
		m_pConqueror->UnbookUnloadSpot(pVeh->GetEntity());

	if (pVeh && !m_pConqueror->CanBookUnloadSpot(pVeh->GetEntity()))
		m_pConqueror->SetCanBookSpot(pVeh, true, "Squad start combat");
}

void CConquerorCommander::OnSquadFinishCombat(CSquad* pSquad)
{
	if (!pSquad)
		return;

	if (!pSquad->GetLeader())
		return;

	//CryLogAlways("%s[C++][%s][Squad %i Finish Combat]", STR_CYAN, GetSpeciesName(), pSquad->GetId());

	auto pOrder = GetCommanderOrder(pSquad->GetLeader());
	if (!pOrder)
		return;

	if (pOrder->m_state != eOES_Starting)
	{
		if (m_pConqueror->GetClientSpecies() == GetSpecies())
		{
			if (g_pGameCVars->conq_debug_log)
			{
				CryLogAlways("%s[C++][%s][Squad %i Finish Combat]",
					STR_CYAN, GetSpeciesName(), pSquad->GetId());
			}
		}

		//31.12.2022 вертолёт зацикливается если он detached
		//const auto pOldOrder = pOrder;
		//m_commanderOrdersMap[pSquad->GetLeader()->GetEntityId()] = CCommanderOrder(pOldOrder->m_type, pOldOrder->m_pos, eOES_Starting, pOldOrder->m_targetId);
	}
}

void CConquerorCommander::OnAIJoinGame()
{
	RequestNewStrategy(m_pConqueror->m_gameStatus == eGS_GameStart);
}

string CConquerorCommander::GetString(EOrderExecuteState state)
{
	switch (state)
	{
	case eOES_NotStarted:
		return "NotStarted";
		break;
	case eOES_Starting:
		return "Starting";
		break;
	case eOES_InMovingToTarget:
		return "InMovingToTarget";
		break;
	case eOES_PerformingAction:
		return "PerformingAction";
		break;
	}
}

string CConquerorCommander::GetString(ECommanderOrders types)
{
	switch (types)
	{
	case eCO_FirstOrder:
		return "FirstOrder";
		break;
	case eCO_Attack:
		return "Attack";
		break;
	case eCO_Defend:
		return "Defend";
		break;
	}
}

string CConquerorCommander::GetString(EAreaFlag flag)
{
	switch (flag)
	{
	case EAreaFlag::Centre:
		return "Centre";
		break;
	case EAreaFlag::AirSpawner:
		return "AirSpawner";
		break;
	case EAreaFlag::LandSpawner:
		return "LandSpawner";
		break;
	case EAreaFlag::SeaSpawner:
		return "SeaSpawner";
		break;
	case EAreaFlag::SoldierSpawner:
		return "SoldierSpawner";
		break;
	case EAreaFlag::Bridge:
		return "Bridge";
		break;
	case EAreaFlag::Base:
		return "Base";
		break;
	case EAreaFlag::AirField:
		return "AirField";
		break;
	case EAreaFlag::SupplyPoint:
		return "SupplyPoint";
		break;
	case EAreaFlag::ControlPoint:
		return "ControlPoint";
		break;
	case EAreaFlag::North:
		return "North";
		break;
	case EAreaFlag::West:
		return "West";
		break;
	case EAreaFlag::South:
		return "South";
		break;
	case EAreaFlag::East:
		return "East";
		break;
	case EAreaFlag::Safe:
		return "Safe";
		break;
	case EAreaFlag::Neutral:
		return "Neutral";
		break;
	case EAreaFlag::Front:
		return "Front";
		break;
	}
}

string CConquerorCommander::GetString(ESquadOrders orders)
{
	switch (orders)
	{
	case eSO_Guard:
		return "Goto";
		break;
	case eSO_SearchEnemy:
		return "SearchEnemy";
		break;
	case eSO_FollowLeader:
		return "FollowLeader";
		break;
	case eSO_SubPrimaryShootAt:
		return "PrimaryShootAt";
		break;
	case eSO_SubSecondaryShootAt:
		return "SecondaryShootAt";
		break;
	case eSO_SubEnterVehicle:
		return "EnterVehicle";
		break;
	case eSO_SubExitVehicle:
		return "ExitVehicle";
		break;
	case eSO_SubPrimaryPickupItem:
		return "PrimaryPickupItem";
		break;
	case eSO_SubSecondaryPickupItem:
		return "SecondarPickupItem";
		break;
	case eSO_SubUseVehicleTurret:
		return "UseVehicleTurret";
		break;
	case eSO_DebugEnableCombat:
		return "EnableCombat";
		break;
	case eSO_DebugDisableCombat:
		return "DisableCombat";
		break;
	case eSO_DebugStanceRelaxed:
		return "StanceRelaxed";
		break;
	case eSO_DebugStanceStanding:
		return "StanceStanding";
		break;
	case eSO_DebugStanceStealth:
		return "StanceStealth";
		break;
	case eSO_DebugStanceCrouch:
		return "StanceCrouch";
		break;
	case eSO_ConqSearchCoverAroundArea:
		return "ConqSearchCoverAroundArea";
		break;
	case eSO_ConqGoTo:
		return "ConqGoTo";
		break;
	case eSO_SearchCoverAroundPoint:
		return "SearchCoverAroundArea";
		break;
	case eSO_None:
		return "None";
	break;	
	case eSO_ConqBlankAction:
		return "BlankAction";
		break;
	case eSO_ScoutGrabMe:
		return "ScoutGrabMe";
		break;
	}

	return "UndefinedOrder";
}

void CConquerorCommander::OnStartLoosingAdvantage()
{
	const auto index = m_pConqueror->GetSpeciesFlagIndex(GetSpecies());
	m_pConqueror->HUDSOMSetFlagLoseAdvantage(index, 1);
}

void CConquerorCommander::OnStopLoosingAdvantage()
{
	const auto index = m_pConqueror->GetSpeciesFlagIndex(GetSpecies());
	m_pConqueror->HUDSOMSetFlagLoseAdvantage(index, 0);
}

void CConquerorCommander::OnParatroopersUnloaded(IVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	m_pConqueror->UnbookUnloadSpot(pVehicle->GetEntity());
}

void CConquerorCommander::OnParatrooperEnter(IVehicle* pVehicle, IActor* pParatrooperActor)
{
	if (!pVehicle || !pParatrooperActor)
		return;

	auto pTrooperChannel = m_pConqueror->GetConquerorChannel(pParatrooperActor->GetEntity());
	if (pTrooperChannel)
	{
		if ((pTrooperChannel->GetStateDuration(eCCS_Alive) > 0.25f))
		{
			//TODO Сделать сообщение высадки между респавном паратрупера и вертолёта 
			//TODO Добавить делей на update при респавне ИИ

			//auto pAI = pParatrooperActor->GetEntity()->GetAI();
			//TOS_AI::ExecuteAIAction(pAI, pVehicle->GetEntity(), "squad_vehicle_exit", 102, -1, eAAEF_IgnoreCombatDuringAction, "CConquerorCommander::OnParatrooperEnter");

			//	//When leader spawned and enter vehicle and after paratrooper Enter

			//	//CryLogAlways("%s[C++][Vehicle %s][When leader spawned and enter vehicle and after paratrooper Enter]",
			//		//STR_CYAN, pVehicle->GetEntity()->GetName());

			//	//if (TOS_AI::IsCombatEnable(TOS_Vehicle::GetAI(pVehicle)))
			//		//TOS_AI::EnableCombat(TOS_Vehicle::GetAI(pVehicle), false, true, "CMDR: Vehicle need move to unload spot and drop paratroopers");

			//	return;
		}
	}
		//else
		//{
		//	TOS_Vehicle::Exit(pParatrooperActor, true, false);
		//}
	//}
		

	//If the paratrooper spawned in the leader's vehicle, 
	//then force exit from vehicle

	auto pVehicleAI = TOS_Vehicle::GetAI(pVehicle);
	if (!pVehicleAI)
		return;

	auto pLeaderDriver = pVehicle->GetDriver();
	if (!pLeaderDriver || (pLeaderDriver && pLeaderDriver == pParatrooperActor))
		return;

	auto pSquad = m_pSquadSystem->GetSquadFromLeader(pLeaderDriver);
	if (!pSquad)
		return;

	const auto pAssignedArea = GetAssignedArea(pSquad);
	if (!pAssignedArea)
		return;

	//auto pLeaderOrder = GetCommanderOrder(pLeaderDriver);
	//if (!pLeaderOrder)
	//	return;

	if (m_commanderOrdersMap[pLeaderDriver->GetEntityId()].m_state != eOES_Starting)
	{
		//m_commanderOrdersMap[pLeaderDriver->GetEntityId()].m_pos = pAssignedArea->GetWorldPos();
		//m_commanderOrdersMap[pLeaderDriver->GetEntityId()].m_targetId = pAssignedArea->GetEntityId();

		//if (m_commanderOrdersMap[pLeaderDriver->GetEntityId()].m_state != eOES_Starting)
		//	m_commanderOrdersMap[pLeaderDriver->GetEntityId()].SetState(eOES_Starting);

		//if (pSquad->IsLeaderDetached())
		//	pSquad->MarkUndetached(pLeaderDriver->GetEntityId());

		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("%s[C++][OnParatrooperEnter][Vehicle: %s]",
				STR_CYAN, pVehicle->GetEntity()->GetName());
		}
	}
}

void CConquerorCommander::OnParatrooperRespawned(IVehicle* pVehicle, IActor* pParatrooperActor)
{
	if (!pVehicle || !pParatrooperActor)
		return;

	auto pVehicleAI = TOS_Vehicle::GetAI(pVehicle);
	if (!pVehicleAI)
		return;

	auto pLeaderDriver = pVehicle->GetDriver();
	if (!pLeaderDriver || (pLeaderDriver && pLeaderDriver == pParatrooperActor))
		return;

	auto pSquad = m_pSquadSystem->GetSquadFromLeader(pLeaderDriver);
	if (!pSquad)
		return;

	const auto pAssignedArea = GetAssignedArea(pSquad);
	if (!pAssignedArea)
		return;

	auto* pOrder = &m_commanderOrdersMap[pLeaderDriver->GetEntityId()];
	if (pOrder->m_state != eOES_InMovingToTarget)
	{
		//TODO: need test

		pAssignedArea->UnbookUnloadSpot(pVehicle->GetEntity());
		pOrder->SetState(eOES_InMovingToTarget);
	}

	if (pSquad->IsLeaderDetached())
		pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

	if (g_pGameCVars->conq_debug_log_commander)
	{
		CryLogAlways("%s[C++][OnParatrooperRespawned][Vehicle: %s]",
			STR_CYAN, pVehicle->GetEntity()->GetName());
	}
}

bool CConquerorCommander::IsHaveParatroopers(const CSquad* pSquad)
{
	if (!pSquad)
		return false;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
		return iter->second.size() > 0;

	return false;
}

void CConquerorCommander::GetParatroopers(const CSquad* pSquad, std::vector<EntityId>& paratroopers)
{
	if (!pSquad)
		return;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
	{
		for (EntityId id : iter->second)
			paratroopers.push_back(id);
	}
}

void CConquerorCommander::GetParatroopers(const CSquad* pSquad, const IVehicle* pVehicle, std::vector<EntityId>& paratroopers)
{
	if (!pSquad || !pVehicle)
		return;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
	{
		for (EntityId id : iter->second)
		{
			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
			if (!pActor || (pActor && pActor->GetHealth() < 0))
				continue;

			if (TOS_Vehicle::GetVehicle(pActor) == pVehicle)
				paratroopers.push_back(pActor->GetEntityId());
		}
	}
}

int CConquerorCommander::GetParatroopsCount(const CSquad* pSquad, const IVehicle* pVehicle)
{
	int count = 0;

	if (!pSquad || !pVehicle)
		return count;

	if (TOS_Vehicle::IsCar(pVehicle))
		return count;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
	{
		for (EntityId id : iter->second)
		{
			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
			if (!pActor || (pActor && pActor->GetHealth() < 0))
				continue;

			if (TOS_Vehicle::GetVehicle(pActor) == pVehicle)
				count++;
		}
	}

	return count;
}

int CConquerorCommander::GetParatroopsCount(const CSquad* pSquad)
{
	int count = 0;

	if (!pSquad)
		return count;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
	{
		for (EntityId id : iter->second)
		{
			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
			if (!pActor || (pActor && pActor->GetHealth() < 0))
				continue;

			count++;
		}
	}

	return count;
}

bool CConquerorCommander::IsParatrooper(EntityId memberId)
{
	auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(memberId);
	if (!pActor)
		return false;

	auto pSquad = m_pSquadSystem->GetSquadFromMember(pActor, false);
	if (!pSquad)
		return false;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
		return stl::find(iter->second, memberId);

	return false;
}

void CConquerorCommander::MarkParatrooper(const CSquad* pSquad, EntityId memberId)
{
	if (!pSquad)
		return;

	if (!pSquad->IsMember(memberId))
		return;

	m_squadParatroopers[pSquad->GetId()].push_back(memberId);

	if (g_pGameCVars->conq_debug_log_commander)
		CryLogAlways("%s[C++][%s][Member %i: Mark as paratrooper]", STR_YELLOW, GetSpeciesName() ,memberId);
}

void CConquerorCommander::UnmarkParatrooper(EntityId memberId)
{
	auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(memberId);
	if (!pActor)
		return;

	auto pSquad = m_pSquadSystem->GetSquadFromMember(pActor, false);
	if (!pSquad)
		return;

	auto iter = m_squadParatroopers.find(pSquad->GetId());
	if (iter != m_squadParatroopers.end())
	{
		stl::find_and_erase(iter->second, memberId);

		if (g_pGameCVars->conq_debug_log_commander)
			CryLogAlways("%s[C++][%s][Member %i: Unmark paratrooper]", STR_YELLOW, GetSpeciesName(), memberId);
	}
}

bool CConquerorCommander::IsBookedVehicle(IVehicle* pVehicle)
{
	if (!pVehicle)
		return false;

	auto it = m_squadBookedVehicle.begin();
	auto end = m_squadBookedVehicle.end();

	for (; it!=end; it++)
	{
		if (it->second == pVehicle->GetEntityId())
			return true;
	}

	return false;
}

IVehicle* CConquerorCommander::GetBookedVehicle(int squadId)
{
	auto iter = m_squadBookedVehicle.find(squadId);

	if (iter != m_squadBookedVehicle.end())
		return g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(iter->second);

	return nullptr;
}

IVehicle* CConquerorCommander::BookFreeVehicle(CStrategicArea* pArea, int squadId, float radius, uint flags, int minSeatCount /*= -1*/)
{
	auto pSquad = m_pSquadSystem->GetSquadFromId(squadId);
	if (!pSquad)
		return nullptr;

	if (!pSquad->GetLeader())
		return nullptr;

	auto pFreeVeh = GetFreeVehicle(pArea, pSquad->GetLeader()->GetEntity(), radius, flags, minSeatCount);
	if (pFreeVeh)
	{
		m_squadBookedVehicle[squadId] = pFreeVeh->GetEntityId();

		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("%s[C++][%s CMDR][Book Free Vehicle][Vehicle %i][Squad Leader Id %i]", STR_YELLOW, GetSpeciesName()
				, pFreeVeh->GetEntityId(), pSquad->GetLeader()->GetEntityId());
		}
	}

	return pFreeVeh;
}

bool CConquerorCommander::UnbookVehicle(const IVehicle* pVehicle)
{
	if (!pVehicle)
		return false;

	int targetSquadId = 0;

	auto it = m_squadBookedVehicle.cbegin();
	auto end = m_squadBookedVehicle.cend();
	for (; it != end; it++)
	{
		if (it->second == pVehicle->GetEntityId())
		{
			targetSquadId = it->first;
			break;
		}
	}

	if (targetSquadId == 0)
		return false;

	if (UnbookVehicle(targetSquadId))
	{
		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("[C++][Unbook vehicle %s from squad %i][Count of booked: %i]",
				pVehicle->GetEntity()->GetName(), targetSquadId, m_squadBookedVehicle.size());
		}

		return true;
	}
	
	return false;
}

bool CConquerorCommander::UnbookVehicle(int squadId)
{
	auto iter = m_squadBookedVehicle.find(squadId);
	if (iter != m_squadBookedVehicle.end())
	{
		m_squadBookedVehicle.erase(iter);
		return true;
	}

	return false;
}

//int CConquerorCommander::GetProperGoalPipeId(const CCommanderOrder* pOrder)
//{
//	if (!pOrder)
//		return COMMANDER_ORD_DEFEND_GOALPIPE_ID;
//
//	if (pOrder->m_type == eCO_Attack)
//	{
//		if (pOrder->m_state == eOES_InMovingToTarget)
//			return COMMANDER_ANY_ORD_GOTO_GOALPIPE_ID;
//		else if (pOrder->m_state == eOES_PerformingAction)
//			return COMMANDER_ORD_GOTO_SUB_HIDE_GOALPIPE_ID;
//		else
//			return COMMANDER_ORD_DEFEND_GOALPIPE_ID;
//	}
//	else if (pOrder->m_type == eCO_Defend)
//	{
//		if (pOrder->m_state == eOES_InMovingToTarget)
//			return COMMANDER_ANY_ORD_GOTO_GOALPIPE_ID;
//		else if (pOrder->m_state == eOES_PerformingAction)
//			return COMMANDER_ORD_GOTO_SUB_HIDE_GOALPIPE_ID;
//		else
//			return COMMANDER_ORD_DEFEND_GOALPIPE_ID;
//	}
//}

void CConquerorCommander::SetSpecies(ESpeciesType species)
{
	m_species = species;
}

//void CConquerorCommander::SetTactics(ECommanderTactics tactics)
//{
//	if (tactics != m_currentTactics)
//	{
//		m_prevTactics = m_currentTactics;
//		m_currentTactics = tactics;
//	}
//}

void CConquerorCommander::SetLeaderOrder(EntityId leaderId, CCommanderOrder& order)
{
	m_commanderOrdersMap[leaderId] = order;

	DEFINE_STEPS;

	auto pLeaderActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(leaderId);
	if (!pLeaderActor)
		return;

	auto pSquad = m_pSquadSystem->GetSquadFromLeader(pLeaderActor);
	if (!pSquad)
		return;

	if (pSquad->IsLeaderDetached())
		pSquad->MarkUndetached(leaderId);

	//By default squad leader and members are not detached
	if (pSquad->IsLeaderDetached())
		pSquad->MarkUndetached(pSquad->GetLeader()->GetEntityId());

	for (auto& member : pSquad->m_members)
	{
		if (pSquad->IsMemberDetached(member.GetId()))
			pSquad->MarkUndetached(member.GetId());
	}

	if (GetSpecies() == m_pConqueror->GetClientSpecies())
	{
		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("%s[C++][%s CMDR][Update][On Strategy Change][Initial Squad Leader %i]",
				STR_YELLOW, GetSpeciesName(), pSquad->GetLeader()->GetEntityId());
		}
	}

	//When commander set current strategy we need regroup members and give it follow leader order
	auto members = pSquad->GetAllMembers();
	for (auto& memInstance : members)
	{
		DEFINE_MEMBER_ACTOR(memInstance);

		SOrderInfo order;
		MEMBER_EXECUTE_FOLLOW_LEADER(order, pSquad, pMemAct, memInstance);
	}

	auto pArea = m_pConqueror->GetStrategicArea(order.m_targetId, 0);
	if (pArea)
	{
		auto pVeh = TOS_Vehicle::GetVehicle(pLeaderActor);
		if (pVeh)
			pArea->UnbookUnloadSpot(pVeh->GetEntity());
	}
}

CSquad* CConquerorCommander::GetNearestSquad(const Vec3& fromPoint, const bool useVehicle)
{
	auto pSquadSystem = g_pControlSystem->GetSquadSystem();
	assert(pSquadSystem);

	float minDist = 0;
	CSquad* pNearestSquad = nullptr;
	CSquad* pFirstSquad = nullptr;

	for (CSquad* pSquad : pSquadSystem->m_allSquads)
	{
		if (pSquad->IsLeaderDetached())
			continue;

		if (pSquad->GetSpecies() != m_species)
			continue;

		if (!pSquad->GetLeader())
			continue;

		if (pSquad->GetLeader()->GetHealth() <= 0)
			continue;

		if (!pSquad->GetLeader()->GetLinkedVehicle() && useVehicle)
			continue;

		const auto& leaderPos = pSquad->GetLeader()->GetEntity()->GetWorldPos();
		const auto distance = (fromPoint - leaderPos).GetLength();

		if (minDist == 0)
		{
			pFirstSquad = pSquad;
			minDist = distance;
		}
		else
		{
			if (distance < minDist)
			{
				pNearestSquad = pSquad;
				minDist = distance;
			}
		}

		if (pNearestSquad == nullptr)
			pNearestSquad = pFirstSquad;
	}

	return pNearestSquad;
}

CSquad* CConquerorCommander::GetNearestSquad(const ECommanderOrders commanderOrder, const EOrderExecuteState orderState, const Vec3& fromPoint, const bool useVehicle)
{
	auto pSquadSystem = g_pControlSystem->GetSquadSystem();
	assert(pSquadSystem);

	float minDist = 0;
	CSquad* pNearestSquad = nullptr;
	CSquad* pFirstSquad = nullptr;

	for (CSquad* pSquad : pSquadSystem->m_allSquads)
	{
		if (pSquad->IsLeaderDetached())
			continue;

		if (pSquad->GetSpecies() != m_species)
			continue;

		if (!pSquad->GetLeader())
			continue;

		if (pSquad->GetLeader()->GetHealth() <= 0)
			continue;

		if (!pSquad->GetLeader()->GetLinkedVehicle() && useVehicle)
			continue;

		auto pOrder = GetCommanderOrder(pSquad->GetLeader()->GetEntityId());
		if (pOrder && (pOrder->m_state != orderState) && (pOrder->m_type != commanderOrder))
			continue;
		else if (!pOrder)
			continue;

		const auto& leaderPos = pSquad->GetLeader()->GetEntity()->GetWorldPos();
		const auto distance = (fromPoint - leaderPos).GetLength();

		if (minDist == 0)
		{
			pFirstSquad = pSquad;
			minDist = distance;
		}
		else
		{
			if (distance < minDist)
			{
				pNearestSquad = pSquad;
				minDist = distance;
			}
		}

		if (pNearestSquad == nullptr)
			pNearestSquad = pFirstSquad;
	}

	return pNearestSquad;
}

CSquad* CConquerorCommander::GetNearestSquad(const ECommanderOrders commanderOrder, const Vec3& fromPoint, const bool useVehicle)
{
	auto pSquadSystem = g_pControlSystem->GetSquadSystem();
	assert(pSquadSystem);

	float minDist = 0;
	CSquad* pNearestSquad = nullptr;
	CSquad* pFirstSquad = nullptr;

	for (CSquad* pSquad : pSquadSystem->m_allSquads)
	{
		if (pSquad->IsLeaderDetached())
			continue;

		if (pSquad->GetSpecies() != m_species)
			continue;

		if (!pSquad->GetLeader())
			continue;

		if (pSquad->GetLeader()->GetHealth() <= 0)
			continue;

		if (!pSquad->GetLeader()->GetLinkedVehicle() && useVehicle)
			continue;

		auto pOrder = GetCommanderOrder(pSquad->GetLeader()->GetEntityId());
		if (pOrder && (pOrder->m_type != commanderOrder))
			continue;
		else if (!pOrder)
			continue;

		const auto& leaderPos = pSquad->GetLeader()->GetEntity()->GetWorldPos();
		const auto distance = (fromPoint - leaderPos).GetLength();

		if (minDist == 0)
		{
			pFirstSquad = pSquad;
			minDist = distance;
		}
		else
		{
			if (distance < minDist)
			{
				pNearestSquad = pSquad;
				minDist = distance;
			}
		}

		if (pNearestSquad == nullptr)
			pNearestSquad = pFirstSquad;
	}

	return pNearestSquad;
}

std::vector<int>* CConquerorCommander::GetAssignedSquadsForArea(CStrategicArea* pArea)
{
	if (!pArea)
		return nullptr;

	return &m_targettedAreasMap[pArea];
}

bool CConquerorCommander::IsAssignedSquad(CStrategicArea* pArea, CSquad* pSquad)
{
	if (!pArea || !pSquad)
		return false;

	auto iter = m_targettedAreasMap.find(pArea);
	if (iter != m_targettedAreasMap.end())
		return stl::find(iter->second, pSquad->GetId());

	return false;
}

CStrategicArea* CConquerorCommander::GetAssignedArea(CSquad* pSquad)
{
	if (!pSquad)
		return nullptr;

	for (auto& areasPair : m_targettedAreasMap)
	{
		if (stl::find(areasPair.second, pSquad->GetId()))
			return areasPair.first;
	}

	return nullptr;
}

int CConquerorCommander::GetAssignedSquadNumber(CStrategicArea* pArea, CSquad* pSquad)
{
	if (!pArea || !pSquad)
		return 0;

	auto iter = m_targettedAreasMap.find(pArea);
	if (iter != m_targettedAreasMap.end())
	{
		auto& squadArray = iter->second;

		auto it = std::find(squadArray.begin(), squadArray.end(), pSquad->GetId());
		if (it != squadArray.end())
			return int(it - squadArray.begin());
	}

	return 0;
}

float CConquerorCommander::GetDistanceToOrderTarget(CSquad* pSquad)
{
	if (!pSquad)
		return 0;

	auto pOrder = GetCommanderOrder(pSquad->GetLeader());
	if (!pOrder)
		return 0;

	auto& squadPos = pSquad->GetAveragePos(true);
	auto& orderPos = pOrder->m_pos;

	return (squadPos - orderPos).GetLength();
}

float CConquerorCommander::GetElapsedOrderChangesTime(CCommanderOrder* pOrder, bool getState)
{
	if (!pOrder)
		return 0;

	auto now = gEnv->pTimer->GetFrameStartTime();
	auto& orderTime = getState ? pOrder->m_lastStateChange : pOrder->m_lastTypeChange;

	float elapsedSeconds = (now - orderTime).GetSeconds();

	//CryLogAlways("[C++][Conqueror Commander Get Order Last Changes Elapsed Seconds %1.f, now time %1.f]", elapsedSeconds, now.GetSeconds());

	return elapsedSeconds;
}

SConquerorStrategy* CConquerorCommander::GetCurrentStrategy() const
{
	return m_pConqueror->GetStrategy(m_currentStrategyName.c_str());
}

void CConquerorCommander::SetCurrentStrategy(const SConquerorStrategy* pStrategy)
{
	if (!pStrategy)
		return;
	
	const string strategyName = pStrategy->GetName();
	const auto isNewStrategy = m_currentStrategyName != strategyName;
	const auto isFirstStrategy = (m_currentStrategyName.size() == 0) && (m_currentStrategyName != strategyName);
	
	if (isNewStrategy || isFirstStrategy)
	{
		for (int i = 0; i < eSGT_Last; i++)
			m_currentStrategyGoals[EStrategyGoalTemplates(i)] = 0;

		if (!isFirstStrategy)
			m_lastStrategyName = m_currentStrategyName;

		m_currentStrategyName = strategyName;

		//Setup timelimit
		m_currentStrategyTimeLimit = pStrategy->GetSettings().m_timeLimit;

		stl::push_back_unique(m_strategiesNamesHistory, strategyName);

		//Setup area's flags priorities
		if (!ReadAreaFlagPriorities(pStrategy))
		{
			CryLogAlways("$4[C++][ERROR][%s Commander][Select new strategy(%i,%s) cannot read areas priorities]",
				m_pConqueror->GetSpeciesName(m_species), pStrategy->GetIndex(), pStrategy->GetName());
			return;
		}

		const bool ignoreClientSquad = true;
		const bool clientInFaction = m_pConqueror->GetClientSpecies() == GetSpecies();

		auto name = m_pConqueror->GetSpeciesName(m_species);
		const float aggression = pStrategy->GetSettings().m_aggression;
		const int subordinateCount = m_subordinateSquadIds.size();
		//const int squadsCount = ignoreClientSquad ? (clientInFaction ? subordinateCount - 1 : subordinateCount) : subordinateCount;
		const int squadsCount = subordinateCount;
		//CryLogAlways("[C++][Check1][%s Commander][aggression %1.f][squadsCount %d]", name, aggression, squadsCount);

		float assignedSquadsCount = (squadsCount * aggression) / 100;
		if (assignedSquadsCount > (int)assignedSquadsCount)
			assignedSquadsCount = (int)assignedSquadsCount + 1;

		//CryLogAlways("[C++][Check2][%s][Strategy: %s][assignedSquadsCount %1.f][squadsCount %i][aggression %1.f] ", GetSpeciesName(), pStrategy->GetName(),assignedSquadsCount, squadsCount, aggression);

		//CryLogAlways("[C++][Check2][%s Commander][assignedSquadsCount %1.f]", name, assignedSquadsCount);

		//if (ignorePlayerSquad)
		//{
		//	if (IsHaveClientSquad())
		//		assignedSquadsCount--;
		//}

		std::vector<int> selectedSquadsIds;
		for (auto id : m_subordinateSquadIds)
		{
			auto pSquad = m_pSquadSystem->GetSquadFromId(id);
			if (!pSquad)
				continue;

			if (pSquad->HasClientLeader() && ignoreClientSquad)
				continue;

			if (selectedSquadsIds.size() < assignedSquadsCount)
				selectedSquadsIds.push_back(id);			
		}

		//CryLogAlways("[C++][Check3][%s Commander][subordinateSquadIds %d]", name, m_subordinateSquadIds.size());
		//CryLogAlways("[C++][Check4][%s Commander][selectedSquadsIds %d]", name, selectedSquadsIds.size());

		if (selectedSquadsIds.size() == 0)
		{
			return;
		}

		auto executedAttacksCount = 0;
		auto executedDefencesCount = 0;
		int attacksCount = pStrategy->GetSettings().m_numberOfAttacks;
		int defencesCount = pStrategy->GetSettings().m_numberOfDefences;
		const int uncapturableSelect = pStrategy->GetSettings().m_uncapturableSelect;
		const auto& targetSpecies = pStrategy->GetSettings().m_targetSpecies;

		//CryLogAlways("[C++][Check5][%s Commander][ assignedSquadsCount / attacksCount %1.f]", name, assignedSquadsCount / attacksCount);

		std::vector<int> busySquadIds;
		//int count = 0;

		//Iter selected squads
		for (auto id : selectedSquadsIds)
		{
			if (stl::find(busySquadIds,id))
				continue;

			auto pSquad = m_pSquadSystem->GetSquadFromId(id);
			if (!pSquad)
				continue;

			auto pLeader = pSquad->GetLeader();
			if (!pLeader)
				continue;

			//auto pLeaderOrder = GetLeaderOrder(pLeader);
			//if (!pLeaderOrder)
			//	continue;

			//auto pSquadTarget = gEnv->pEntitySystem->GetEntity(pLeaderOrder->m_targetId);

			//We need more that 0 enemy areas count available to capturing
			const auto hostileAreasCount = m_pConqueror->GetHostileAreasCount(m_species, eAGSF_Enabled);
			const auto friendlyAreasCount = m_pConqueror->GetStrategicAreaCount(m_species, eAGSF_Enabled);

			const auto canAttack = (hostileAreasCount > 0) && (attacksCount > 0) && (executedAttacksCount < attacksCount);
			const auto canDefend = (friendlyAreasCount > 0) && (defencesCount > 0) && (executedDefencesCount < defencesCount);
			
			const auto& squadPos = pLeader->GetEntity()->GetWorldPos();
			const auto haveTargetSpecies = targetSpecies.size() > 0;
			const auto speciesType = m_pConqueror->GetSpeciesTypeFromString(targetSpecies);

			if (attacksCount > assignedSquadsCount)
				attacksCount = assignedSquadsCount;

			if (defencesCount > assignedSquadsCount)
				defencesCount = assignedSquadsCount;

			const auto areaGameStatus = uncapturableSelect > 0 ? eAGSF_Enabled : eAGSF_EnabledAndCapturable;

			if (canAttack)
			{
				float maxAssignedSquadsPerArea = (assignedSquadsCount / attacksCount);
				if (maxAssignedSquadsPerArea > (int)maxAssignedSquadsPerArea)
					maxAssignedSquadsPerArea = (int)maxAssignedSquadsPerArea + 1;

				if (g_pGameCVars->conq_debug_log_strategies)
				{
					CryLogAlways("[C++][Check5][%s Commander][ maxAssignedSquadsPreArea %f = assignedSquadsCount %1.f/ attacksCount %i]",
						name, maxAssignedSquadsPerArea, assignedSquadsCount, attacksCount);
				}
				
				auto pArea = haveTargetSpecies ? 
					GetNearestArea(squadPos, speciesType, true, maxAssignedSquadsPerArea, areaGameStatus) :
					GetNearestArea(squadPos, (string)HOSTILE, true, maxAssignedSquadsPerArea, areaGameStatus);

				if (pArea)
				{
					//CryLogAlways("[C++][%s Commander][Attack][Nearest Area %s]", name, pArea->GetEntity()->GetName());

					//count++;
					auto areaPos = pArea->GetWorldPos();

					//auto pAnchor = pArea->GetAIAchor();
					//if (pAnchor)
					//	areaPos = pAnchor->GetWorldPos();

					//Select new orders which based on current strategy
					OnNewStrategicTarget(pSquad, pArea);

					auto order = CCommanderOrder(eCO_Attack, areaPos, eOES_Starting, pArea->GetEntityId());
					SetLeaderOrder(pLeader->GetEntityId(), order);

					m_targettedAreasMap[pArea].push_back(pSquad->GetId());

					int assignedAreaSquads = GetAssignedSquadsForArea(pArea)->size();
					if (assignedAreaSquads == maxAssignedSquadsPerArea)
					{
						executedAttacksCount++;

						//CryLogAlways("[C++][assignedAreaSquads %d][maxAssignedSquadsPreArea %d][executedAttacksCount %d]", 
							//assignedAreaSquads, maxAssignedSquadsPerArea, executedAttacksCount);
					}

					busySquadIds.push_back(pSquad->GetId());

					if (g_pGameCVars->conq_debug_log_commander)
					{
						CryLogAlways("$3[C++][%s Commander][First strategy %i][Select new strategy(%i,%s)][Area %s]",
							m_pConqueror->GetSpeciesName(m_species), isFirstStrategy, pStrategy->GetIndex(), pStrategy->GetName(), pArea->GetEntity()->GetName());
					}


					continue;
				}
			}

			if (canDefend)
			{
				const auto maxAssignedSquadsPreArea = assignedSquadsCount / defencesCount;

				//auto pArea = GetNearestArea(squadPos, OWNED, true, maxAssignedSquadsPreArea, eAGSF_Enabled);
				
				if (g_pGameCVars->conq_debug_log_strategies)
				{
					CryLogAlways("[C++][Check5][%s Commander][ maxAssignedSquadsPreArea %i = assignedSquadsCount %1.f/ attacksCount %i]",
						name, (int)maxAssignedSquadsPreArea, assignedSquadsCount, attacksCount);
				}

				auto pArea = haveTargetSpecies ? 
					GetNearestArea(squadPos, speciesType, true, maxAssignedSquadsPreArea, areaGameStatus) :
					GetNearestArea(squadPos, OWNED, true, maxAssignedSquadsPreArea, areaGameStatus);

				if (pArea)
				{
					auto areaPos = pArea->GetWorldPos();

					//auto pAnchor = pArea->GetAIAchor();
					//if (pAnchor)
					//	areaPos = pAnchor->GetWorldPos();

					//Select new orders which based on current strategy
					auto order = CCommanderOrder(eCO_Defend, areaPos, eOES_Starting, pArea->GetEntityId());
					SetLeaderOrder(pLeader->GetEntityId(), order);
					m_targettedAreasMap[pArea].push_back(pSquad->GetId());

					executedDefencesCount++;
					busySquadIds.push_back(pSquad->GetId());

					continue;
				}
			}
		}

		//CryLogAlways("");
		//CryLogAlways("[C++][%s Commander][Successful get area Count %i]", name, count);
		//CryLogAlways("[C++][%s Commander][Strategy %s][Assigned Squads Count %d]", name, strategyName, assignedSquadsCount);
		//CryLogAlways("");

		m_lastTimeStrategyChange = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	}
	else
	{
		//Setup timelimit
		m_currentStrategyTimeLimit = pStrategy->GetSettings().m_timeLimit;

		
		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("[C++][%s Commander][Update old timelimit strategy(%i,%s)]",
				m_pConqueror->GetSpeciesName(m_species), pStrategy->GetIndex(), pStrategy->GetName());
		}
	}
}

bool CConquerorCommander::RequestNewStrategy(bool gameStart)
{
	bool success = false;

	const auto& allowableStrategies = m_pConqueror->GetAllowableStrategies(this, !gameStart);
	const float strategiesCount = allowableStrategies.size();

	if (strategiesCount > 1)
	{
		//const float ranf = Random(allowableStrategies[0], allowableStrategies[strategiesCount-1]+1);
		
		const int minElementIndex = 0;
		const int maxElementIndex = allowableStrategies.size() - 1;
		
		const float ranf = Random(minElementIndex, maxElementIndex);
		const int ran = round(ranf);

		//if (ranf > (int)ranf)
		//	ran++;

		if (g_pGameCVars->conq_debug_log_commander)
			CryLogAlways("[C++][%s Commander][Request New Strategy][Random (i%i, f%f)]", GetSpeciesName(),ran, ranf);

		//for (auto strategyIndex : allowableStrategies)
		//{
		//	if (strategyIndex != ran)
		//		continue;

		//	auto pStrategy = m_pConqueror->GetStrategy(strategyIndex);
		//	if (!pStrategy)
		//		continue;

		//	SetCurrentStrategy(pStrategy);
		//	success = true;

		//	break;
		//}
		for (int i = 0; i < allowableStrategies.size(); i++)
		{
			if (i != ran)
				continue;

			auto pStrategy = m_pConqueror->GetStrategy(allowableStrategies[i]);
			if (!pStrategy)
				continue;

			SetCurrentStrategy(pStrategy);
			success = true;

			break;
		}

		//CryLogAlways("[C++][Request New Strategy][Random %d][Strategies Count %d]", ran, strategiesCount);
	}
	else if (strategiesCount == 1)
	{
		for (auto strategyIndex : allowableStrategies)
		{
			//CryLogAlways("[C++][Commander of %s][Request New Strategy][strategyIndex = %i]", m_pConqueror->GetSpeciesName(m_species), strategyIndex);

			auto pStrategy = m_pConqueror->GetStrategy(strategyIndex);
			if (!pStrategy)
				break;

			SetCurrentStrategy(pStrategy);
			success = true;

			//CryLogAlways("[C++][Commander of %s][Request New Strategy][Select one strategy index [%i] name [%s]]", m_pConqueror->GetSpeciesName(m_species),pStrategy->GetIndex() ,pStrategy->GetName());
			//CryLogAlways("[C++][Commander of %s][Request New Strategy][allowableStrategies[0] = %i]", m_pConqueror->GetSpeciesName(m_species), allowableStrategies[0]);

			break;
		}
	}

	auto pCurrent = GetCurrentStrategy();

	//Reset time limit for the current strategy
	if (success == false && pCurrent)
	{
		m_currentStrategyTimeLimit = pCurrent->GetSettings().m_timeLimit;

		if (g_pGameCVars->conq_debug_log_commander)
		{
			CryLogAlways("[C++][%s Commander][Request New Strategy][Reset timer for Current Strategy %s]",
				m_pConqueror->GetSpeciesName(m_species), pCurrent->GetName());
		}
	}
	else if (!pCurrent)
	{
		CryLogAlways("[C++][%s Commander][Request New Strategy][From game start FAIL]", m_pConqueror->GetSpeciesName(m_species));
	}

	return success;
}

bool CConquerorCommander::IsHaveClientSquad()
{
	for (auto ids : m_subordinateSquadIds)
	{
		auto pSquad = m_pSquadSystem->GetSquadFromId(ids);
		if (!pSquad)
			continue;

		if (pSquad->HasClientLeader())
			return true;
	}

	return false;
}

std::vector<EAreaFlag> CConquerorCommander::GetMostPriorityFlags(string areaStatus)
{
	const auto firstFlagType = EAreaFlag::FirstType;
	const auto firstPriorityValue = m_areaFlagPriorities[areaStatus][firstFlagType];

	auto maxValue = firstPriorityValue;

	std::vector<EAreaFlag> mostPriorityFlags;
	//find max priority value
	for (auto& flagPrioritiesPair : m_areaFlagPriorities[areaStatus])
	{
		const auto areaFlagPriority = flagPrioritiesPair.second;

		if (areaFlagPriority > maxValue)
			maxValue = areaFlagPriority;
	}

	//find all flags with max priority
	for (auto& flagPrioritiesPair : m_areaFlagPriorities[areaStatus])
	{
		const auto areaFlag = flagPrioritiesPair.first;
		const auto areaFlagPriority = flagPrioritiesPair.second;

		if (areaFlagPriority == maxValue)
		{
			stl::push_back_unique(mostPriorityFlags, areaFlag);

			if (g_pGameCVars->conq_debug_log_commander)
			{
				CryLogAlways("$7[C++][%s Commander][Max Priority = %1.f][Priority Area Flag: %s]",
					GetSpeciesName(), maxValue, GetString(areaFlag));
			}
		}
	}
	
	return mostPriorityFlags;
}

std::vector<EAreaFlag> CConquerorCommander::GetMostPriorityFlags(ESpeciesType targetSpecies)
{
	return GetMostPriorityFlags(m_pConqueror->GetSpeciesName(targetSpecies));
}

//float CConquerorCommander::GetAreaFlagPriority(string areaStatus, string targetSpecies, EAreaFlag flag)
//{
//	//if (strcmp(areaStatus, "Hostile") != 0)
//	//	return 100.f;
//	//else if (strcmp(areaStatus, "Neutral") != 0)
//	//	return 100.f;
//	//else if (strcmp(areaStatus, "Owned") != 0)
//	//	return 100.f;
//	//else if (strcmp(areaStatus, "Any") != 0)
//	//	return 100.f;
//
//	return m_areaFlagPriorities[areaStatus][flag];
//}

void CConquerorCommander::SetAreaFlagPriority(string areaStatus, string targetSpecies, EAreaFlag flag, float prioritry)
{
	if (areaStatus != "")
	{
		if (strcmp(areaStatus, ANY) == 0)
		{
			m_areaFlagPriorities[HOSTILE][flag] = prioritry;
			m_areaFlagPriorities[NEUTRAL][flag] = prioritry;
			m_areaFlagPriorities[OWNED][flag] = prioritry;
		}
		else
		{
			m_areaFlagPriorities[areaStatus][flag] = prioritry;
		}
	}
	else if (targetSpecies != "")
	{
		//Handle the targetSpecies in this block
		m_areaFlagPriorities[targetSpecies][flag] = prioritry;
	}
}

bool CConquerorCommander::ReadAreaFlagPriorities(const SConquerorStrategy* pStrategy)
{
	if (!pStrategy)
		return false;

	const auto& priorities = pStrategy->GetPriorities();
	const auto maxCount = priorities.size();
	//auto counter = 0;

	auto it = priorities.begin();
	auto end = priorities.end();

	for (; it!=end;it++)
	{
		const char* areaFlagString = it->m_areaFlag;
		const char* areaStatusString = it->m_status;
		const char* areaTargetSpecies = it->m_targetSpecies;
		auto floatValue = it->m_priority;

		auto areaFlag = EAreaFlag::FirstType;

		if (strstr(areaFlagString, "Centre"))
			areaFlag = (EAreaFlag::Centre);
		else if (strstr(areaFlagString, "AirSpawner"))
			areaFlag = (EAreaFlag::AirSpawner);
		else if (strstr(areaFlagString, "LandSpawner"))
			areaFlag = (EAreaFlag::LandSpawner);
		else if (strstr(areaFlagString, "SeaSpawner"))
			areaFlag = (EAreaFlag::SeaSpawner);
		else if (strstr(areaFlagString, "SoldierSpawner"))
			areaFlag = (EAreaFlag::SoldierSpawner);
		else if (strstr(areaFlagString, "Bridge"))
			areaFlag = (EAreaFlag::Bridge);
		else if (strstr(areaFlagString, "Base"))
			areaFlag = (EAreaFlag::Base);
		else if (strstr(areaFlagString, "AirField"))
			areaFlag = (EAreaFlag::AirField);
		else if (strstr(areaFlagString, "SupplyPoint"))
			areaFlag = (EAreaFlag::SupplyPoint);
		else if (strstr(areaFlagString, "ControlPoint"))
			areaFlag = (EAreaFlag::ControlPoint);
		else if (strstr(areaFlagString, "North"))
			areaFlag = (EAreaFlag::North);
		else if (strstr(areaFlagString, "West"))
			areaFlag = (EAreaFlag::West);
		else if (strstr(areaFlagString, "South"))
			areaFlag = (EAreaFlag::South);
		else if (strstr(areaFlagString, "East"))
			areaFlag = (EAreaFlag::East);
		else if (strstr(areaFlagString, "Safe"))
			areaFlag = (EAreaFlag::Safe);
		else if (strstr(areaFlagString, "Neutral"))
			areaFlag = (EAreaFlag::Neutral);
		else if (strstr(areaFlagString, "Front"))
			areaFlag = (EAreaFlag::Front);

		SetAreaFlagPriority(areaStatusString, areaTargetSpecies, areaFlag, floatValue);
		//counter++;
	}

	//if (counter == maxCount)
		//return true;

	return true;
}

IVehicle* CConquerorCommander::GetFreeVehicle(CStrategicArea* pArea, const IEntity* pRequester, float radius, uint flags, int minSeatCount /*= -1*/)
{
	if (!pRequester)
		return nullptr;

	std::vector<EntityId> vehInRadius;

	auto pVehicleIter = g_pGame->GetIGameFramework()->GetIVehicleSystem()->CreateVehicleIterator();
	while (auto pVehicle = pVehicleIter->Next())
	{
		const auto vehPos = pVehicle->GetEntity()->GetWorldPos();
		const auto dist = (vehPos - pRequester->GetWorldPos()).GetLength();

		if (dist > radius)
			continue;

		if (pVehicle->GetEntity()->IsHidden())
			continue;

		if (pVehicle->IsDestroyed())
			continue;

		if (IsBookedVehicle(pVehicle))
			continue;

		if (pVehicle->GetStatus().passengerCount != 0)
			continue;

		if ((minSeatCount != -1 && minSeatCount > 0) && (pVehicle->GetSeatCount() < minSeatCount))
			continue;

		auto movType = pVehicle->GetMovement()->GetMovementType();

		if (flags & eVGF_Air)
		{
			if (movType != IVehicleMovement::eVMT_Air)
				continue;
		}
		else if (flags & eVGF_Land)
		{
			if (movType != IVehicleMovement::eVMT_Land)
				continue;
		}
		else if (flags & eVGF_Amphibious)
		{
			if (movType != IVehicleMovement::eVMT_Amphibious)
				continue;
		}
		else if (flags & eVGF_Sea)
		{
			if (movType != IVehicleMovement::eVMT_Sea)
				continue;
		}

		if (flags & eVGF_MustHaveGun)
		{
			const auto seatIdx = TOS_Vehicle::RequestGunnerSeatIndex(pVehicle);
			if (seatIdx == -1)
				continue;
		}

		stl::push_back_unique(vehInRadius, pVehicle->GetEntityId());
	}

	if (flags & eVGF_NearestOrLinks)
	{
		IVehicle* pNearestVehicle = nullptr;
		auto minDist = 0.0f;

		for (auto vehId : vehInRadius)
		{
			auto pVeh = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(vehId);
			if (!pVeh)
				continue;

			const auto pos = pVeh->GetEntity()->GetWorldPos();
			const auto dist = (pos - pRequester->GetWorldPos()).GetLength();;

			if (minDist == 0 && pNearestVehicle == nullptr)
			{
				minDist = dist;
				pNearestVehicle = pVeh;
			}			
			else if (dist < minDist)
			{
				minDist = dist;
				pNearestVehicle = pVeh;
			}
		}

		//Get random free vehicle from links at nearby area 
		if (pNearestVehicle == nullptr)
		{
			auto pChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(pRequester->GetId());
			if (pChannel)
			{
				//не ласт ареа а ближайшая ареа должна быть
				auto pNearestArea = m_pConqueror->GetNearestStrategicArea(pChannel->GetEntity()->GetWorldPos(), ANY, eAGSF_Enabled, m_species, eABF_NoMatter, EAreaFlag::SoldierSpawner);
				//auto pNearestArea = GetNearestArea(pChannel->GetEntity()->GetWorldPos(), ANY, eABF_NoMatter, EAreaFlag::SoldierSpawner);
				//auto pLastArea = pChannel->GetLastArea();
				//if (!pLastArea)
				//	pLastArea = pChannel->GetSelectedArea();

				if (pNearestArea)
				{
					EntityId vehicleId = 0;
					std::vector<EntityId> vehicles;
					pNearestArea->GetSpawnedVehicles(vehicles, flags, minSeatCount);

					auto species = m_pConqueror->GetSpeciesFromEntity(gEnv->pEntitySystem->GetEntity(pRequester->GetId()));

					vehicleId = TOS_STL::GetRandomFromSTL<std::vector<EntityId>, EntityId>(vehicles);
					pNearestVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(vehicleId);
				}
				else
				{
					CryLogAlways("%s[C++][Get Free Vehicle][Result: FAIL, Return NULL ][Cause: NO NEAREST AREA]", STR_RED);
				}
			}
		}

		return pNearestVehicle;
	}
	else
	{
		const int minElementIndex = 0;
		const int maxElementIndex = vehInRadius.size() - 1;

		const float ranf = Random(minElementIndex, maxElementIndex);
		const int ran = round(ranf);

		for (auto i = 0; i < vehInRadius.size(); i++)
		{
			if (i != ran)
				continue;

			auto pRandomVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(vehInRadius[i]);
			if (!pRandomVehicle)
				continue;

			//if (pRandomVehicle)
			//	CryLogAlways("[C++][Get Free Vehicle][Result: %s]", pRandomVehicle->GetEntity()->GetName());
			//else
			//	CryLogAlways("[C++][Get Free Vehicle][Result: RANDOM FAIL]");

			return pRandomVehicle;
		}
	}

	CryLogAlways("%s[C++][Get Free Vehicle][Result: FAIL, Return NULL ][Cause: NO AREA OR NOT HAVE FREE VEH'S]", STR_RED);

	return nullptr;
}

int CConquerorCommander::GetCurrentStrategyGoals(EStrategyGoalTemplates goal) const
{
	return stl::find_in_map(m_currentStrategyGoals, goal, 0);
}