#include "StdAfx.h"
#include "RARSystem.h"
#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "Game.h"

CRARSExecutor::CRARSExecutor()
{
	Init();
}

CRARSExecutor::CRARSExecutor(EntityId id)
{
	Init();
	m_entityId = id;
}

void CRARSExecutor::Init()
{
	m_executionState = eES_NotAssigned;
	m_requestId = 0;
	m_entityId = 0;
	m_allowedRequestTypes.clear();
	m_lastTimeRequestChange = 0;
}

void CRARSExecutor::AddAllowedRequestType(ERequestType type)
{
	stl::push_back_unique(m_allowedRequestTypes, type);
}

void CRARSExecutor::GetAllowedRequestTypes(std::vector<ERequestType>& requests)
{
	requests = m_allowedRequestTypes;
}

void CRARSExecutor::SetCurrentRequest(CRARSRequest* pRequest)
{
	if (!pRequest)
	{
		m_requestId = 0;
		return;
	}

	if ((m_requestId != pRequest->getId()) && (pRequest->getId() != 0))
		m_lastTimeRequestChange = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	m_requestId = pRequest->getId();
}

void CRARSExecutor::SetCurrentState(EExecutionState state)
{
	if (m_executionState != state)
		m_lastTimeStateChange = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	m_executionState = state;

	//auto pExecutorAct = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_entityId);
	//if (pExecutorAct)
	//{
	//	auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pExecutorAct, true);
	//	if (pSquad)
	//	{
	//		if (m_executionState != eES_NotAssigned)
	//		{
	//			if ((pSquad->IsLeaderDetached() && pSquad->GetLeader() == pExecutorAct) ||
	//				pSquad->IsMemberDetached(pExecutorAct))
	//			{
	//				pSquad->MarkUndetached(pExecutorAct->GetEntityId());
	//			}
	//		}
	//		else
	//		{
	//			SDetachedMemberData data;
	//			data.enableUpdate = true;
	//			data.routineType = eDRT_AirPathPatrol;
	//			data.targetId = ;

	//			pSquad->MarkDetached(pExecutorAct->GetEntityId(), data);
	//		}
	//	}
	//}

}
