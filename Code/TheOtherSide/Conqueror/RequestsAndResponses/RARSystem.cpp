#include "StdAfx.h"
#include "GameCVars.h"
#include "..\StrategicArea.h"
#include "..\..\Squad\SquadSystem.h"
#include "..\..\Helpers\TOS_AI.h"
#include "..\..\Helpers\TOS_Vehicle.h"
#include "..\..\Helpers\TOS_Script.h"
#include "RARSystem.h"
#include "Scout.h"

CRequestsAndResponses::CRequestsAndResponses()
{
	m_speciesExecutors.clear();
	m_requests.clear();
	m_requesters.clear();
	m_lastUpdateTime = 0.0f;
}

CRequestsAndResponses::~CRequestsAndResponses()
{

}

CRARSRequest* CRequestsAndResponses::CreateRequest(EntityId requesterId, EntityId goalEntityId, ERequestType type)
{
	for (auto& req : m_requests)
	{
		if (req.requesterId == requesterId)
		{
			if (g_pGameCVars->conq_debug_log_requests)
			{
				CryLogAlways("%s[C++][Cannot create request by requester %id][Cause: only one request per requester can be created]",
					STR_RED, requesterId);

			}
			return nullptr;
		}
	}

	//Create the request
	CRARSRequest request;
	request.requesterId = requesterId;
	request.goalEntityId = goalEntityId;
	request.type = type;
	request.state = eRQ_Created;
	request.id = m_requests.size() + 1;
	request.lastTimeCreated = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	auto it = m_requests.begin();
	auto end = m_requests.end();

	for (;it != end; it++)
	{
		if (it->id == request.id)
			return nullptr;
	}

	m_requests.push_back(request);

	//Create the instance of the requester
	CRARSRequester requester;
	requester.m_entityId = requesterId;
	requester.m_lastTimes[request.state] = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	requester.m_requestId = request.id;

	if (!stl::find(m_requesters, requester))
	{
		m_requesters.push_back(requester);
	}
	else
	{
		auto pInstance = GetRequesterInstance(requesterId);
		if (!pInstance)
		{
			if (g_pGameCVars->conq_debug_log_requests)
			{
				CryLogAlways("%s[C++][FAILED Create Request %i from requester %i][Cause: Requester Instance not defined]",
					STR_RED, request.id, requesterId);
			}

			return nullptr;
		}

		pInstance->UpdateStateTime(request.state);
		pInstance->SetRequestId(request.id);
	}

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][Create Request %i from requester %i]",
			STR_GREEN, request.id, requesterId);
	}

	return &request;
}

CRARSRequest* CRequestsAndResponses::GetRequestById(uint id)
{
	for (auto& request : m_requests)
	{
		if (request.getId() == id)
			return &request;
	}

	return nullptr;
}

CRARSRequest* CRequestsAndResponses::GetRequestFromEntity(EntityId requesterId)
{
	for (auto& request : m_requests)
	{
		if (request.requesterId == requesterId)
			return &request;
	}

	return nullptr;
}

void CRequestsAndResponses::CompleteRequest(uint id)
{
	auto pRequestInstance = GetRequestById(id);
	if (!pRequestInstance)
		return;

	auto pExEntity = GET_ENTITY(GetAssignedExecutorId(pRequestInstance));
	auto pConqueror = g_pControlSystem->GetConquerorSystem();

	if (pConqueror && pConqueror->IsHaveUnloadSpot(pExEntity))
		pConqueror->UnbookUnloadSpot(pExEntity);

	auto pRequesterInstance = GetRequesterInstance(pRequestInstance->requesterId);
	if (pRequesterInstance)
		pRequesterInstance->SetRequestId(0);

	RemoveRequest(id);
}

void CRequestsAndResponses::CancelRequest(uint id)
{
	auto pRequestInstance = GetRequestById(id);
	if (!pRequestInstance)
		return;

	auto pRequesterInstance = GetRequesterInstance(pRequestInstance->requesterId);
	if (!pRequesterInstance)
		return;

	pRequestInstance->state = eRQ_Cancelled;

	auto pExecutorInstance = GetExecutorInstance(GetAssignedExecutorId(pRequestInstance));
	if (pExecutorInstance)
	{
		pExecutorInstance->SetCurrentState(eES_NotAssigned);
		pExecutorInstance->SetCurrentRequest(nullptr);

		auto pExEntity = GET_ENTITY(pExecutorInstance->GetEntityId());
		auto pConqueror = g_pControlSystem->GetConquerorSystem();

		if (pConqueror && pConqueror->IsHaveUnloadSpot(pExEntity))
			pConqueror->UnbookUnloadSpot(pExEntity);
	}

	pRequesterInstance->UpdateStateTime(eRQ_Cancelled);
	pRequesterInstance->SetRequestId(0);

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][Cancel Request %i]",
			STR_YELLOW, pRequestInstance->getId());
	}

	RemoveRequest(id);
}

void CRequestsAndResponses::RemoveRequest(uint id)
{
	auto it = m_requests.begin();
	auto end = m_requests.end();

	for (; it!=end; it++)
	{
		if (it->getId() == id)
		{
			m_requests.erase(it);
			return;
		}
	}
}

void CRequestsAndResponses::FailRequest(uint id, int cause)
{
	auto pRequestInstance = GetRequestById(id);
	if (!pRequestInstance)
		return;

	auto pRequesterInstance = GetRequesterInstance(pRequestInstance->requesterId);
	if (!pRequesterInstance)
		return;

	if (cause == 0)
	{
		//Failed because executor is dead
		pRequestInstance->state = eRQ_FailedByExecutorKilled;
	}
	else if (cause == 1)
	{
		//Failed because executor not assigned
		pRequestInstance->state = eRQ_Failed;
	}

	auto pExEntity = GET_ENTITY(GetAssignedExecutorId(pRequestInstance));
	auto pConqueror = g_pControlSystem->GetConquerorSystem();

	if (pConqueror && pConqueror->IsHaveUnloadSpot(pExEntity))
		pConqueror->UnbookUnloadSpot(pExEntity);

	pRequestInstance->lastTimeFailed = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	pRequesterInstance->UpdateStateTime(pRequestInstance->state);

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][Fail Request %i]",
			STR_YELLOW, pRequestInstance->getId());
	}
}

EntityId CRequestsAndResponses::AssignExecutor(CRARSRequest* pRequest)
{
	if (!pRequest)
		return 0;

	auto pRequester = gEnv->pEntitySystem->GetEntity(pRequest->requesterId);
	if (!pRequester)
	{
		if (g_pGameCVars->conq_debug_log_requests)
		{
			CryLogAlways("%s[C++][Requester id %i][Can not assign executor to request][Cause: Requester not defined]",
				STR_RED, pRequest->requesterId);
		}

		return 0;
	}
	else if (pRequest->type == eRT_NotDefined)
	{
		if (g_pGameCVars->conq_debug_log_requests)
		{
			CryLogAlways("%s[C++][Requester id %i][Can not assign executor to request][Cause: Request type not defined]",
				STR_RED, pRequest->requesterId);
		}

		return 0;
	}

	auto pRequesterInstance = GetRequesterInstance(pRequester->GetId());
	if (!pRequesterInstance)
	{
		if (g_pGameCVars->conq_debug_log_requests)
		{
			CryLogAlways("%s[C++][Can not assign executor to request][Cause: Requester Instance not defined]",
				STR_RED, pRequest->requesterId);
		}
		return 0;
	}

	const auto species = (ESpeciesType)g_pControlSystem->GetConquerorSystem()->GetSpeciesFromEntity(pRequester);
	const auto pos = pRequester->GetWorldTM().GetTranslation();
	const uint flags = eEGF_IsNearest | eEGF_IsNotInCombat | eEGF_IsFree;

	const auto executorId = GetExecutorIdByFlags(species, pos, flags, pRequest->type);
	auto pExecutorEnt = GET_ENTITY(executorId);

	auto pExecutorInstance = GetExecutorInstance(executorId);
	if (!pExecutorInstance || !pExecutorEnt)
	{
		if (g_pGameCVars->conq_debug_log_requests)
		{
			CryLogAlways("%s[C++][Requester id %i][Can not assign executor to request][Cause: Suitable executor not found]",
				STR_RED, pRequest->requesterId);
		}

		return 0;
	}

	pRequest->state = eRQ_Assigned;
	pRequest->lastTimeAssigned = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	pExecutorInstance->SetCurrentRequest(pRequest);
	pExecutorInstance->SetCurrentState(eES_Assigned);

	pRequesterInstance->UpdateStateTime(pRequest->state);

	if (strcmp(pExecutorEnt->GetClass()->GetName(), "Scout") == 0)
	{
		//auto pGoalEntity = GET_ENTITY(pRequest->goalEntityId);
		//if (pGoalEntity)
		//{
			//CryLogAlways("GOAL %s", pGoalEntity->GetName());
		//}

		auto pRequestGoalArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pRequest->goalEntityId, 0);
		if (pRequestGoalArea)
		{
			EntityId id = pRequestGoalArea->GetBookedUnloadSpot(pExecutorEnt);
			if (id == 0)
				id = pRequestGoalArea->BookFreeUnloadSpot(pExecutorEnt);

			if (id)
				SetRequestGoal(executorId, id);
		}
	}

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][Assign Executor %i to Request %i]",
			STR_GREEN, pExecutorInstance->GetEntityId(), pRequest->getId());
	}

	return pExecutorInstance->GetEntityId();
}

void CRequestsAndResponses::AddExecutor(ESpeciesType species, EntityId id, const std::vector<ERequestType>& requestTypes)
{
	auto pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (!pEntity)
		return;

	auto& executors = m_speciesExecutors[species];
	for (auto& executor : executors)
	{
		if (executor.GetEntityId() == id)
			return;
	}

	CRARSExecutor executor(id);

	for (auto i : requestTypes)
		executor.AddAllowedRequestType(i);

	executor.SetCurrentRequest(nullptr);
	executors.push_back(executor);

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][%s][Add Executor %i]",
			STR_GREEN, g_pControlSystem->GetConquerorSystem()->GetSpeciesName(species), id);
	}
}

CRARSExecutor* CRequestsAndResponses::GetExecutorInstance(EntityId id)
{
	if (id == 0)
		return nullptr;

	auto it = m_speciesExecutors.begin();
	auto end = m_speciesExecutors.end();

	for (; it != end; it++)
	{
		auto& executors = it->second;
		for (auto& executor : executors)
		{
			if (executor.GetEntityId() == id)
				return &executor;
		}
	}

	return nullptr;
}

CRARSRequester* CRequestsAndResponses::GetRequesterInstance(EntityId id)
{
	if (id == 0)
		return nullptr;

	for (auto& instance : m_requesters)
	{
		if (instance.m_entityId == id)
			return &instance;
	}

	return nullptr;
}

EntityId CRequestsAndResponses::GetExecutorIdByFlags(ESpeciesType species, const Vec3& requesterPos, uint flags)
{
	std::vector<EntityId> iteratedExecutors;

	auto iter = m_speciesExecutors.find(species);
	if (iter != m_speciesExecutors.end())
	{
		auto& executors = iter->second;
		for (auto& executor : executors)
		{
			auto pEntity = gEnv->pEntitySystem->GetEntity(executor.GetEntityId());
			if (!pEntity)
				continue;

			auto pInstance = GetExecutorInstance(pEntity->GetId());
			if (!pInstance)
				continue;

			if (flags & eEGF_IsFree)
			{
				if (pInstance->GetCurrentState() != eES_NotAssigned)
					continue;
			}
			else if (flags & eEGF_IsPerformingRequest)
			{
				if (pInstance->GetCurrentState() != eES_PerformingRequest)
					continue;
			}

			if (flags & eEGF_IsInCombat)
			{
				if (!TOS_AI::IsInCombat(pEntity->GetAI()))
					continue;
			}
			else if (flags & eEGF_IsNotInCombat)
			{
				if (TOS_AI::IsInCombat(pEntity->GetAI()))
					continue;
			}

			iteratedExecutors.push_back(pEntity->GetId());
		}
	}

	EntityId executorId = 0;

	if (flags & eEGF_IsNearest)
	{
		float minDistance = 0.0f;

		for (auto id : iteratedExecutors)
		{
			auto pEntity = gEnv->pEntitySystem->GetEntity(iteratedExecutors[0]);
			if (!pEntity)
				return 0;

			const auto pos = pEntity->GetWorldPos();
			const auto dist = (pos - requesterPos).GetLength();;

			if (minDistance == 0 && executorId == 0)
			{
				minDistance = (pEntity->GetWorldPos() - pos).GetLength();
				executorId = pEntity->GetId();
			}
			else if (dist < minDistance)
			{
				minDistance = dist;
				executorId = pEntity->GetId();
			}
		}
	}
	else
	{
		executorId = TOS_STL::GetRandomFromSTL<std::vector<EntityId>, EntityId>(iteratedExecutors);
	}

	return executorId;
}

EntityId CRequestsAndResponses::GetExecutorIdByFlags(ESpeciesType species, const Vec3& pos, uint flags, ERequestType allowedRequestType)
{
	const auto executorId = GetExecutorIdByFlags(species, pos, flags);
	const auto pInstance = GetExecutorInstance(executorId);

	if (pInstance)
	{
		std::vector<ERequestType> types;
		pInstance->GetAllowedRequestTypes(types);

		if (stl::find(types, allowedRequestType))
			return executorId;
	}

	if (g_pGameCVars->conq_debug_log_requests)
	{
		CryLogAlways("%s[C++][RAR System][Can't get executor by flags and allowed request type]", STR_RED);
	}

	return 0;
}

EntityId CRequestsAndResponses::GetAssignedExecutorId(CRARSRequest* pRequest)
{
	if (!pRequest)
		return 0;

	auto it = m_speciesExecutors.cbegin();
	auto end = m_speciesExecutors.cend();

	for (; it != end; it++)
	{
		auto& executors = it->second;
		for (auto& executor : executors)
		{
			if (executor.GetCurrentRequestId() == pRequest->getId())
				return executor.GetEntityId();
		}
	}

	return 0;
}

bool CRequestsAndResponses::IsAssignedExecutor(EntityId id)
{
	auto it = m_speciesExecutors.cbegin();
	auto end = m_speciesExecutors.cend();

	for (; it != end; it++)
	{
		auto& executors = it->second;
		for (auto& executor : executors)
		{
			if (executor.GetEntityId() == id)
			{
				return executor.GetCurrentRequestId() != 0;
			}
		}
	}

	return false;
}

bool CRequestsAndResponses::IsExecutor(EntityId id)
{
	auto it = m_speciesExecutors.cbegin();
	auto end = m_speciesExecutors.cend();

	for (; it != end; it++)
	{
		auto& executors = it->second;
		for (auto& executor : executors)
		{
			if (executor.GetEntityId() == id)
				return true;
		}
	}

	return false;
}

bool CRequestsAndResponses::IsExecutor(ESpeciesType species, EntityId id)
{
	auto iter = m_speciesExecutors.find(species);
	if (iter != m_speciesExecutors.end())
	{
		auto& executors = iter->second;
		for (auto& executor : executors)
		{
			if (executor.GetEntityId() == id)
				return true;
		}
	}

	return false;
}

void CRequestsAndResponses::RemoveExecutor(EntityId id)
{
	auto it = m_speciesExecutors.begin();
	auto end = m_speciesExecutors.end();

	for (; it != end; it++)
	{
		auto& executors = it->second;

		auto it2 = executors.begin();
		auto end2 = executors.end();

		for (; it2 != end2; it2++)
		{
			if (it2->GetEntityId() == id)
			{
				executors.erase(it2);
				return;
			}
		}
	}
}

void CRequestsAndResponses::RemoveRequester(EntityId id)
{
	auto pInstance = GetRequesterInstance(id);
	if (!pInstance)
		return;

	stl::find_and_erase(m_requesters, pInstance);
}

void CRequestsAndResponses::Reset()
{
	m_requests.clear();
	m_speciesExecutors.clear();
	m_requesters.clear();
	m_lastUpdateTime = 0.0f;
}

void CRequestsAndResponses::Update(float frametime)
{
	const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	for (auto& request : m_requests)
	{
		const float lastTimeCreated = currentTime - request.lastTimeCreated;
		const float lastTimeCompleted = currentTime - request.lastTimeCompleted;
		const float lastTimeAssigned = currentTime - request.lastTimeAssigned;
		const float lastTimeFailed = currentTime - request.lastTimeFailed;

		const auto id = request.getId();

		float color[] = { 1,1,1,1 };
		const auto size = 1.2f;
		const auto scale = 20;
		const auto xoffset = 60;
		const auto yoffset = 30;

		if (g_pGameCVars->conq_debug_draw_rar_requests == 1)
		{
			auto pRequester = GET_ENTITY(request.requesterId);

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + id * scale, size, color, false, "Requester %s: Request %i, state %s, created %1.f, completed %1.f, assigned %1.f, failed %1.f",
				pRequester ? pRequester->GetName() : "", id, GetString(request.state), lastTimeCreated, lastTimeCompleted, lastTimeAssigned, lastTimeFailed);
		}
	}

	if (currentTime - m_lastUpdateTime < 0.5f)
		return;

	m_lastUpdateTime = currentTime;

	UpdateRequests(frametime);

	//Math constant definition
	const auto goalDistTreshold = 15.0f;

	for (auto& speciesPair : m_speciesExecutors)
	{
		auto& executors = speciesPair.second;
		for (auto& executorInstance : executors)
		{
			auto pExecutor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(executorInstance.GetEntityId()));
			if (!pExecutor || (pExecutor && (pExecutor->GetHealth() < 0 || pExecutor->GetEntity()->IsHidden())))
				continue;

			auto pExecutorAI = pExecutor->GetEntity()->GetAI();
			if (!pExecutorAI)
				continue;

			//Generic AI Action values
			const char* desiredGoalName = "";
			const char* actionName = "nullActionName";
			const float maxAlertness = 102.0f; //high prioritry
			int goalPipeId = -1;
			auto flag = eAAEF_IgnoreCombatDuringAction;
			string solution = "CRequestsAndResponses::Update: ";
			//~Generic AI Action values

			const auto executorIsAlien = pExecutor->IsAlien();
			const auto executorPos = pExecutor->GetEntity()->GetWorldPos();
			const auto executorClassName = pExecutor->GetEntity()->GetClass()->GetName();

			const auto executorIsDriver = TOS_Vehicle::ActorIsDriver(pExecutor);
			auto pExVehicle = TOS_Vehicle::GetVehicle(pExecutor);

			const auto exIsCombat = TOS_AI::IsInCombat(pExecutorAI);

			const auto pRequest = GetRequestById(executorInstance.GetCurrentRequestId());
			if (!pRequest)
				continue;

			auto pRequesterActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pRequest->requesterId));
			if (!pRequesterActor)
				continue;

			auto pRequesterSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pRequesterActor, true);
			if (!pRequesterSquad)
				continue;

			auto pReqSqdLeader = static_cast<CActor*>(pRequesterSquad->GetLeader());
			if (!pReqSqdLeader)
				continue;
			
			const auto& requesterPos = pRequesterActor->GetEntity()->GetWorldTM().GetTranslation();
			IEntity* pGoalEntity = GET_ENTITY(pRequest->goalEntityId);
			const bool noTarget = !pGoalEntity;

			const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			if (noTarget)
			{
				if (g_pGameCVars->conq_debug_log_requests)
				{
					CryLogAlways("[C++][RaR System][update FAILED][Cause: Executor not have request target or pos]");
				}

				continue;
			}

			if (!executorIsAlien)
			{

			}
			else
			{
				if (pRequest->type == eRT_AlienTaxsee)
				{
					switch (executorInstance.GetCurrentState())
					{
					case eES_Assigned:
					{
						const float lastTimeAssigned = currentTime - executorInstance.m_lastTimeStateChange;
						if (lastTimeAssigned < 0.01f)
						{
							//Ignore combat when start executing the request

							//if (!TOS_AI::IsUsingPipe(pExecutor, "do_nothing"))
								//TOS_AI::SelectPipe(pExecutor, 0, "do_nothing", "RARUpd: Clear active goals when assigned to Request");

							//Executor
							//if (TOS_AI::IsCombatEnable(pExecutor))
								//TOS_AI::EnableCombat(pExecutor, false, true, "RARUpd: Disable Executor Combat when assigned to Request");

							desiredGoalName = "";
							goalPipeId = -1;
							actionName = "squad_clear_action";
							solution =+ string("Request Assigned now");
							if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
								TOS_AI::ExecuteAIAction(pExecutorAI, pExecutor->GetEntity(), actionName, maxAlertness, goalPipeId, flag, 0, solution);

							//Leader
							if (TOS_AI::IsCombatEnable(pReqSqdLeader))
								TOS_AI::EnableCombat(pReqSqdLeader, false, false, "RARUpd: Disable Squad Leader Combat when assigned to Request");

							//Members
							for (auto& member : pRequesterSquad->GetAllMembers())
							{
								auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
								if (!pActor)
									continue;

								if (TOS_AI::IsCombatEnable(pActor))
									TOS_AI::EnableCombat(pActor, false, false, "RARUpd: Disable Member Combat when executor assigned to Request");
							}
						}

						if (pRequest->state != eRQ_Assigned)
						{
							//pRequest->lastTimeAssigned = gEnv->pTimer->GetFrameStartTime().GetSeconds();
							pRequest->state = eRQ_Assigned;

							if (g_pGameCVars->conq_debug_log_requests)
							{
								CryLogAlways("%s[C++][Request %i][Executor %i][New State is %i]",
									STR_YELLOW, pRequest->getId(), pExecutor->GetEntityId(), pRequest->state);
							}
						}

						executorInstance.SetCurrentState(eES_MoveToRequester);
					}
					break;
					case eES_MoveToRequester:
					{
						const auto executorNeedToMove = TOS_Distance::IsBigger(pExecutor, pReqSqdLeader->GetEntity(), 40.0f/*goalDistTreshold*/);
						const auto executorGoalMoveDone = !executorNeedToMove;

						//const float goalDist = (requesterPos - executorPos).GetLength();
						//const auto needToMove = goalDist > goalDistTreshold;

						if (executorNeedToMove)
						{
							desiredGoalName = "action_goto0";
							actionName = "rar_goto_a0_d0_r3";
							flag = eAAEF_IgnoreCombatDuringAction;
							solution += string("go to requester entity");
							goalPipeId = -1;

							if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
								TOS_AI::ExecuteAIAction(pExecutorAI, pReqSqdLeader->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);

							//const auto airOffset = Vec3(0, 0, 10);
							//const auto goalPosWithOffset = requesterPos;// + airOffset;

							//const auto notTruePos = TOS_AI::GetRefPoint(pExecutor) != goalPosWithOffset;
							//const auto notTruePipe = !TOS_AI::IsUsingPipe(pExecutor, "tos_rar_goto");

							//if (notTruePos)
							//{
							//	TOS_AI::SetRefPoint(pAI, goalPosWithOffset);

							//	if (g_pGameCVars->conq_debug_log_requests)
							//	{
							//		CryLogAlways("[C++][Scout %s Not True Goal Pos]",
							//			pExecutor->GetEntity()->GetName());
							//	}

							//}

							//if (notTruePipe)
								//TOS_AI::SelectPipe(pAI, 0, "tos_rar_goto", "RARUpd: Executor need move to requester");

						}
						else if (executorGoalMoveDone)
						{
							if (strcmp(executorClassName, "Scout") == 0)
							{
								//if (!TOS_AI::IsUsingPipe(pExecutor, "do_nothing"))
									//TOS_AI::SelectPipe(pExecutor, 0, "do_nothing", "RARUpd: Clear active goals when executor's class is the Scout");

								auto pTable = pExecutor->GetEntity()->GetScriptTable();
								if (!pTable)
									continue;

								const auto isGrabbed = pReqSqdLeader->GetGrabStats()->isGrabbed;
								if (!isGrabbed)
								{
									if (g_pGameCVars->conq_debug_log_requests)
									{
										CryLogAlways("[C++][Executor %s][Grab Leader %s]",
											pExecutor->GetEntity()->GetName(), pReqSqdLeader->GetEntity()->GetName());
									}

									//desiredGoalName = "";
									//actionName = "rar_alien_grab_entity";
									//flag = eAAEF_IgnoreCombatDuringAction;
									//solution += string("grab requester");
									//goalPipeId = -1;

									//if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
										//TOS_AI::ExecuteAIAction(pExecutorAI, pReqSqdLeader->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
									Script::CallMethod(pTable, "GrabEntityInTentacle", pReqSqdLeader->GetEntityId());
								}

								const auto leaderPos = pReqSqdLeader->GetEntity()->GetWorldPos();

								std::vector<EntityId> members;
								pRequesterSquad->GetMembersInRadius(members, leaderPos, 40);
						
								for (auto id : members)
								{
									auto pMemberActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
									if (!pMemberActor)
										continue;

									const auto notGrabbed = !pMemberActor->GetGrabStats()->isGrabbed;
									if (notGrabbed)
									{
										if (Script::CallMethod(pTable, "GrabEntityInTentacle", pMemberActor->GetEntityId()))
										{
											if (TOS_AI::IsCombatEnable(pMemberActor))
												TOS_AI::EnableCombat(pMemberActor, false, false, "RARUpd: Disable combat when member is grabbed by executor");
										}
									
										//desiredGoalName = "";
										//actionName = "rar_alien_grab_entity";
										//flag = eAAEF_IgnoreCombatDuringAction;
										//solution += string("grab requester members");
										//goalPipeId = -1;

										////if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
										//TOS_AI::ExecuteAIAction(pExecutorAI, pMemberActor->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);

										if (g_pGameCVars->conq_debug_log_requests)
										{
											CryLogAlways("[C++][Executor %s][Grab Member %s]",
												pExecutor->GetEntity()->GetName(), pMemberActor->GetEntity()->GetName());
										}
									}
								}

								const auto executorNeedWait = currentTime - pExecutor->GetGrabStats()->lastTimeGrab < 2.0f;
								if (!executorNeedWait)
									executorInstance.SetCurrentState(eES_PerformingRequest);
							}
						}
					}
					break;
					case eES_PerformingRequest:
					{
						if (pRequest->state != eRQ_Executing)
						{
							pRequest->state = eRQ_Executing;

							if (g_pGameCVars->conq_debug_log_requests)
							{
								CryLogAlways("%s[C++][Request %i][Executor %i][New State is %i]",
									STR_YELLOW, pRequest->getId(), pExecutor->GetEntityId(), pRequest->state);
							}
						}

						//const float goalDist = (goalPosition - executorPos).GetLength();
						//const auto needToMove = goalDist > goalDistTreshold;

						const auto executorNeedToMove = TOS_Distance::IsBigger(pExecutor, pGoalEntity, goalDistTreshold * 2.5);
						//const auto needToMove = goalDist > goalDistTreshold;
						const auto needToFinishRequest = !executorNeedToMove;

						if (strcmp(executorClassName, "Scout") == 0)
						{
							if (executorNeedToMove)
							{
								desiredGoalName = "action_goto0";
								actionName = "conqueror_goto_a0_d0_r3";
								flag = eAAEF_IgnoreCombatDuringAction;
								solution += string("Executor are moving to goal entity");
								goalPipeId = -1;

								if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
									TOS_AI::ExecuteAIAction(pExecutorAI, pGoalEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);

								//const auto airOffset = Vec3(0, 0, 10);
								//const auto goalPosWithOffset = pGoalEntity->GetWorldPos() + airOffset;

								//const auto notTruePos = TOS_AI::GetRefPoint(pExecutor) != goalPosWithOffset;
								//const auto notTruePipe = !TOS_AI::IsUsingPipe(pExecutor, "tos_rar_goto");

								//if (notTruePos)
									//TOS_AI::SetRefPoint(pExecutor, goalPosWithOffset);

								//if (notTruePipe)
									//TOS_AI::SelectPipe(pExecutor, 0, "tos_rar_goto", "RARUpd: Executor need moving to the request's goal position");

							}
							else if (needToFinishRequest)
							{
								auto pTable = pExecutor->GetEntity()->GetScriptTable();
								if (!pTable)
									continue;

								//if (!TOS_AI::IsUsingPipe(pExecutor, "do_nothing"))
									//TOS_AI::SelectPipe(pExecutor, 0, "do_nothing", "RARUpd: Executor clear active goals when need to finish request");

								const float lastTimeDrop = currentTime - pExecutor->GetGrabStats()->lastTimeDrop;
								if (lastTimeDrop > 2.0f)
								{
									Script::CallMethod(pTable, "DropEntitiesFromTentacles", pGoalEntity->GetWorldPos());

									//desiredGoalName = "";
									//actionName = "rar_alien_drop_entity";
									//flag = eAAEF_IgnoreCombatDuringAction;
									//solution += string("goto booked vehicle");
									//goalPipeId = -1;

									//if (!TOS_AI::IsExecuting(pExecutorAI, actionName))
									//	TOS_AI::ExecuteAIAction(pExecutorAI, pExecutor->GetEntity(), actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
								}

								if (!TOS_AI::IsCombatEnable(pReqSqdLeader))
									TOS_AI::EnableCombat(pReqSqdLeader, true, false, "RARUpd: Requester enable combat when finish request");

								const auto leaderPos = pReqSqdLeader->GetEntity()->GetWorldPos();

								std::vector<EntityId> members;
								pRequesterSquad->GetMembersInRadius(members, leaderPos, 10);

								for (auto id : members)
								{
									auto pMemberActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
									if (!pMemberActor)
										continue;

									if (!TOS_AI::IsCombatEnable(pMemberActor->GetEntity()->GetAI()))
										TOS_AI::EnableCombat(pMemberActor->GetEntity()->GetAI(), true, false, "RARUpd: Requester members enable combat when finish request");
								}

								executorInstance.SetCurrentState(eES_FinishRequest);
							}
						}
					}
					break;
					case eES_FinishRequest:
					{
						const float lastTimeDrop = currentTime - pExecutor->GetGrabStats()->lastTimeDrop;
						if (lastTimeDrop > 2.0f)
						{
							//Executor
							if (!TOS_AI::IsCombatEnable(pExecutor))
								TOS_AI::EnableCombat(pExecutor, true, true, "RARUpd: Executor enable combat when Finish Request");
						}
						
						if (lastTimeDrop > 6.0f)
						{
							if (pRequest->state != eRQ_Completed)
							{
								pRequest->lastTimeCompleted = gEnv->pTimer->GetFrameStartTime().GetSeconds();
								pRequest->state = eRQ_Completed;

								if (g_pGameCVars->conq_debug_log_requests)
								{
									CryLogAlways("%s[C++][Request %i][Executor %i][New State is %i]",
										STR_YELLOW, pRequest->getId(), pExecutor->GetEntityId(), pRequest->state);

								}
							}

							executorInstance.SetCurrentState(eES_NotAssigned);
							executorInstance.SetCurrentRequest(nullptr);
						}

						//Enable combat for all

						//Leader
						if (!TOS_AI::IsCombatEnable(pReqSqdLeader))
							TOS_AI::EnableCombat(pReqSqdLeader, true, false, "RARUpd: Requester enable combat when Finish Request");

						//Members
						for (auto& member : pRequesterSquad->GetAllMembers())
						{
							auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
							if (!pActor)
								continue;

							if (!TOS_AI::IsCombatEnable(pActor))
								TOS_AI::EnableCombat(pActor, true, false, "RARUpd: Requester member enable combat when Finish Request");
						}
					}
					break;
					}
				}
			}
		}
	}
}

void CRequestsAndResponses::UpdateExecutors(float frametime)
{

}

void CRequestsAndResponses::UpdateRequests(float frametime)
{
	for (auto& request : m_requests)
	{
		const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

		const float lastTimeCreated = currentTime - request.lastTimeCreated;
		const float lastTimeCompleted = currentTime - request.lastTimeCompleted;
		const float lastTimeAssigned = currentTime - request.lastTimeAssigned;
		const float lastTimeFailed = currentTime - request.lastTimeFailed;

		const auto id = request.getId();

		float color[] = { 1,1,1,1 };
		const auto size = 1.2f;
		const auto scale = 20;
		const auto xoffset = 60;
		const auto yoffset = 30;

		const float tryAssignSeconds = 4.0f;

		if (request.state == eRQ_Created)
		{
			if (lastTimeCreated <= tryAssignSeconds)
				AssignExecutor(&request);
			else
				FailRequest(request.getId(), 1);
		}
		if (request.state == eRQ_FailedByExecutorKilled || request.state == eRQ_Failed || request.state == eRQ_NotAssigned)
		{
			//if (lastTimeFailed >= 30.0f)
				CancelRequest(request.getId());
		}
		if (request.state == eRQ_Completed)
		{
			//if (lastTimeCompleted >= 30.0f)
				CompleteRequest(request.getId());
		}
	}
}

void CRequestsAndResponses::OnActorGrab(IActor* pActor, EntityId grabId)
{

}

void CRequestsAndResponses::OnActorDrop(IActor* pActor, EntityId dropId)
{
	auto pInstance = GetExecutorInstance(pActor->GetEntityId());
	if (!pInstance)
		return;

	auto pRequest = GetRequestById(pInstance->GetCurrentRequestId());
	if (!pRequest)
		return;

	if (pRequest->type == eRT_AlienTaxsee)
		CancelRequest(pRequest->getId());
}

bool CRequestsAndResponses::SetRequestGoal(EntityId ExecutorId, EntityId goalId)
{
	auto pExecutorInstance = GetExecutorInstance(ExecutorId);
	if (pExecutorInstance)
	{
		auto pRequest = GetRequestById(pExecutorInstance->GetCurrentRequestId());
		if (pRequest)
		{
			pRequest->goalEntityId = goalId;
			return true;
		}
	}

	return false;
}
