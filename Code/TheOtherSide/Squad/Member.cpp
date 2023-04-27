#include "stdafx.h"

#include "Player.h"

#include "../Control/ControlSystem.h"
#include "SquadSystem.h"

#include <IEntity.h>

CMember::CMember()
{
	Init();
};

CMember::CMember(IActor* _pAct)
{
	Init();

	if (_pAct)
	{
		auto pEntity = _pAct->GetEntity();
		if (pEntity && pEntity->GetAI())
		{
			m_previousGroupId = pEntity->GetAI()->GetGroupId();
			m_entityId = pEntity->GetId();
		}
	}
};

CMember::CMember(EntityId _id)
{
	Init();

	m_entityId = _id;

	auto pEntity = gEnv->pEntitySystem->GetEntity(m_entityId);
	if (pEntity && pEntity->GetAI())
	{
		m_previousGroupId = pEntity->GetAI()->GetGroupId();
	}
};

void CMember::Init()
{
	m_lastUpdateTime = 0;
	m_actionRefId = 0;
	m_isUpdating = true;
	m_index=(-1);
	m_entityId=(0);
	m_groupId=(0);
	m_previousGroupId=(0);
	m_pSquadSystem = g_pControlSystem->GetSquadSystem();

	m_detachedData.reset();
	m_stats.reset();

	m_previousOrderInfo = SOrderInfo();
	m_currentOrderInfo = SOrderInfo();
	m_subOrderInfo = SOrderInfo();
	//m_failedOrderInfo = SOrderInfo();
	m_mainStep = EOrderExecutingStep::NotHaveOrder;
	m_subStep = EOrderExecutingStep::NotHaveOrder;
}

void CMember::Reset()
{
	Init();
};

void CMember::Serialize(TSerialize ser)
{
	ser.BeginGroup("tosSquadMember");
	//SER_VALUE(m_aiCleanDuration);
	//SER_VALUE(m_guardPos);
	//SER_VALUE(m_previousGuardPos);
	//SER_VALUE(m_searchPos);
	//SER_VALUE(m_previousSearchPos);
	//SER_VALUE(m_isLeader);
	SER_VALUE(m_isUpdating);
	SER_VALUE(m_entityId);
	//SER_VALUE(m_processedEntityId);
	SER_VALUE(m_groupId);
	SER_VALUE(m_previousGroupId);
	//SER_VALUE(m_searchCoverRadius);
	//SER_VALUE_ENUM(m_previousOrder, eSO_Guard, eSO_ForSync);
	//SER_VALUE_ENUM(m_previousGotoState, eGUS_CleanAI, eGUS_ForSync);
	//SER_VALUE_ENUM(m_currentGotoState, eGUS_CleanAI, eGUS_ForSync);
	//SER_VALUE_ENUM(m_currentOrder, eSO_Guard, eSO_ForSync);
	ser.EndGroup();
}

void CMember::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CMember::SetCurrentGroupId(int id)
{
	m_previousGroupId = m_groupId;
	m_groupId = id;

	auto pEntity = gEnv->pEntitySystem->GetEntity(m_entityId);
	if (pEntity && pEntity->GetAI())
		pEntity->GetAI()->SetGroupId(m_groupId);
}

EntityId CMember::GetId() const
{
	return m_entityId;
}

int CMember::GetIndex() const
{
	return m_index;
}

int CMember::GetGroupId() const
{
	return m_groupId;
}

void CMember::EnableUpdating(bool enable)
{
	m_isUpdating = enable;
}

bool CMember::CanBeUpdated() const
{
	return m_isUpdating;
}

void CMember::GetDetachedData(SDetachedMemberData &data) const
{
	data = m_detachedData;
}

void CMember::SetDetachedData(const SDetachedMemberData& data)
{
	m_detachedData = data;
}

void CMember::SetOrderInfo(const SOrderInfo& info, bool previous)
{
	if (previous)
	{
		m_previousOrderInfo = info;
	}
	else
	{
		if (info.type != m_currentOrderInfo.type)
			m_stats.lastTimeNewOrder = gEnv->pTimer->GetFrameStartTime().GetSeconds();

		m_currentOrderInfo = info;
	}
}

void CMember::GetOrderInfo(SOrderInfo& info, bool previous) const
{
	info.ignoreFlag = previous ? m_previousOrderInfo.ignoreFlag : m_currentOrderInfo.ignoreFlag;
	info.targetId = previous ? m_previousOrderInfo.targetId : m_currentOrderInfo.targetId;
	info.targetPos = previous ? m_previousOrderInfo.targetPos : m_currentOrderInfo.targetPos;
	info.targetRadius = previous ? m_previousOrderInfo.targetRadius : m_currentOrderInfo.targetRadius;
	info.type = previous ? m_previousOrderInfo.type : m_currentOrderInfo.type;
	info.stepActions = previous ? m_previousOrderInfo.stepActions : m_currentOrderInfo.stepActions;
	info.safeFly = previous ? m_previousOrderInfo.safeFly : m_currentOrderInfo.safeFly;
}

SMemberStats* CMember::GetStats()
{
	return &m_stats;
}

EOrderExecutingStep CMember::GetMainStep() const
{
	return m_mainStep;
}

void CMember::SetStep(EOrderExecutingStep step, bool sub)
{
	if (sub)
	{
		if (m_subStep != step)
		{
			m_stats.lastTimeSetSubStep = gEnv->pTimer->GetFrameStartTime().GetSeconds();
			m_subStep = step;
		}
	}
	else
	{
		if (m_mainStep != step)
		{
			m_stats.lastTimeSetStep = gEnv->pTimer->GetFrameStartTime().GetSeconds();
			m_mainStep = step;
		}
	}
}

void CMember::SetActionRef(IEntity* pEntity)
{
	if (!pEntity)
		return;

	m_actionRefId = pEntity->GetId();
}

IEntity* CMember::GetActionRef() const
{
	return GET_ENTITY(m_actionRefId);
}

void CMember::SetSubOrderInfo(const SOrderInfo& info)
{
	if (info.type != m_subOrderInfo.type)
		m_stats.lastTimeNewSubOrder = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	m_subOrderInfo = info;
}

void CMember::GetSubOrderInfo(SOrderInfo& info) const
{
	info.ignoreFlag = m_subOrderInfo.ignoreFlag;
	info.targetId = m_subOrderInfo.targetId;
	info.targetPos = m_subOrderInfo.targetPos;
	info.targetRadius = m_subOrderInfo.targetRadius;
	info.type = m_subOrderInfo.type;
	info.stepActions = m_subOrderInfo.stepActions;
	info.safeFly = m_subOrderInfo.safeFly;
}

EOrderExecutingStep CMember::GetSubStep() const
{
	return m_subStep;
}

void CMember::ResetOrder(bool main, bool sub, bool previous)
{
	if (main)
		SetOrderInfo(SOrderInfo(), false);

	if (sub)
		SetSubOrderInfo(SOrderInfo());

	if (previous)
		SetOrderInfo(SOrderInfo(), true);
}

void CMember::OnSubOrderFinish()
{
	m_stats.lastTimeSubOrderFinished = gEnv->pTimer->GetFrameStartTime().GetSeconds();
}

//CLeaderInstance::CLeaderInstance()
//{
//	CMember::CMember();
//}
//
//CLeaderInstance::CLeaderInstance(IActor* _pAct)
//{
//	CMember::CMember(_pAct);
//}
//
//CLeaderInstance::CLeaderInstance(EntityId _id)
//{
//	CMember::CMember(_id);
//}