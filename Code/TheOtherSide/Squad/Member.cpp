#include "stdafx.h"

#include "../Control/ControlSystem.h"
#include "SquadSystem.h"

SMember::SMember()
	:isLeader(false),
	searchPos(Vec3(0, 0, 0)),
	guardPos(Vec3(0, 0, 0)),
	currentOrder(eSquadOrders_GoTo),
	currentGotoState(eGotoUpdateState_Guard),
	index(-1),
	actorId(0)
{
	m_pSquadSystem = g_pControlSystem->pSquadSystem;
};

SMember::SMember(IActor* _pAct)
	:isLeader(false),
	searchPos(Vec3(0, 0, 0)),
	guardPos(Vec3(0, 0, 0)),
	currentOrder(eSquadOrders_GoTo),
	currentGotoState(eGotoUpdateState_Guard),
	index(-1)
{
	if (_pAct)
		actorId = _pAct->GetEntityId();

	m_pSquadSystem = g_pControlSystem->pSquadSystem;
};

SMember::SMember(EntityId _id)
	:isLeader(false),
	searchPos(Vec3(0, 0, 0)),
	guardPos(Vec3(0, 0, 0)),
	currentOrder(eSquadOrders_GoTo),
	currentGotoState(eGotoUpdateState_Guard),
	aiCleanDuration(0),
	index(-1)
{
	actorId = _id;
	m_pSquadSystem = g_pControlSystem->pSquadSystem;
};

SMember::~SMember()
{
	index = -1;
	actorId = 0;
	isLeader = 0;
	searchPos.zero();
	guardPos.zero();
	m_pSquadSystem = 0;
	currentOrder = (eSquadOrders_GoTo);
	currentGotoState = (eGotoUpdateState_Guard);
};

void SMember::Reset()
{
	if (IActorSystem *pActSystem = g_pGame->GetIGameFramework()->GetIActorSystem())
	{
		guardPos = searchPos = pActSystem->GetActor(actorId) ?
			pActSystem->GetActor(actorId)->GetEntity()->GetWorldPos() : Vec3(0, 0, 0);
		isLeader = 0;
		currentOrder = eSquadOrders_GoTo;
		currentGotoState = eGotoUpdateState_Guard;
		aiCleanDuration = 0;
		index = -1;
		actorId = 0;
	}
};

void SMember::Serialize(TSerialize ser)
{
	ser.BeginGroup("ctrlSquadMember");
	SER_VALUE(aiCleanDuration);
	SER_VALUE(guardPos);
	SER_VALUE(searchPos);
	SER_VALUE(isLeader);
	SER_VALUE(actorId);
	SER_VALUE_ENUM(currentGotoState, eGotoUpdateState_CleanAI, eGotoUpdateState_ForSync);
	SER_VALUE_ENUM(currentOrder, eSquadOrders_GoTo, eSquadOrders_ForSync);
	ser.EndGroup();
}

void SMember::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}
