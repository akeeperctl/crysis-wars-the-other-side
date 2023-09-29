#include "StdAfx.h"
#include "TheOtherSideMP/Control System/ControlSystem.h"
#include "AIActionTracker.h"
#include "IAIActionTrackerListener.h"
#include "GameCVars.h"
#include "../Helpers/TOS_Debug.h"
#include "../Helpers/TOS_AI.h"
#include "../Helpers/TOS_Vehicle.h"
#include "IAIAction.h"

#define IS_VOID_HOLDER(entityId, isHolder)\
for (auto& voidPair : m_voidHolders)\
{\
	if (voidPair.first.id == entityId)\
	{\
		isHolder = true;\
		break;\
	}\
}\


CAIActionTracker::CAIActionTracker()
{
	Reset();
}

CAIActionTracker::~CAIActionTracker()
{
	g_pControlSystem->RemoveChild(this, false);
}

bool CAIActionTracker::IsExecuting(const IAIObject* pUserAI, int actionGoalPipeId)
{
	if (!pUserAI)
		return false;

	auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		if (iter->second.goalPipeId == actionGoalPipeId)
			return true;
	}

	return false;
}

bool CAIActionTracker::IsExecuting(const IAIObject* pUserAI, const char* actionName)
{
	if (!pUserAI)
		return false;

	auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		if (strcmp(iter->second.name, actionName) == 0)
			return true;
	}

	return false;
}

bool CAIActionTracker::IsExecuting(const IAIObject* pUserAI, const char* actionName, IEntity* pObject)
{
	if (!pUserAI || !pObject)
		return false;

	auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		if (strcmp(iter->second.name, actionName) == 0)
		{
			auto pAIAction = gEnv->pAISystem->GetAIAction(actionName);
			if (pAIAction && pAIAction->GetObjectEntity() == pObject)
				return true;
		}
			
	}

	return false;

}

bool CAIActionTracker::IsExecuting(const IAIObject* pUserAI, const char* actionName, const char* desiredGoalPipe)
{
	if (!pUserAI)
		return false;

	auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		if ((strcmp(iter->second.name, actionName) == 0) && 
			(strcmp(iter->second.desiredGoalPipe, desiredGoalPipe) == 0))
			return true;
	}

	return false;
}

void CAIActionTracker::OnVehicleDestroyed(IVehicle* pVeh)
{
	if (!pVeh)
		return;

	auto id = pVeh->GetEntityId();

	if (!IsTracking(id))
		return;

	SAIActionInfo info;
	GetActionInfo(id, info);

	m_entitiesStats[id].actionName = info.name;
	m_entitiesStats[id].lastTimeFailed = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	StopTracking(id);
}

void CAIActionTracker::OnMainMenuEnter()
{
	Reset();
}

void CAIActionTracker::OnGameRulesReset()
{
	Reset();
}

void CAIActionTracker::OnActorDeath(IActor* pActor)
{
	if (!pActor)
		return;

	auto pAI = pActor->GetEntity()->GetAI();
	if (!pAI)
		return;

	if (!IsTracking(pAI))
		return;

	const EntityId id = pAI->GetEntity()->GetId();

	SAIActionInfo info;
	GetActionInfo(pAI, info);

	m_entitiesStats[id].actionName = info.name;
	m_entitiesStats[id].lastTimeFailed = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	StopTracking(pAI);

	for (auto pListener : m_listeners)
		pListener->OnActorDeath(pActor);
}

void CAIActionTracker::OnActorGrabbed(IActor* pActor, EntityId grabberId)
{

}

void CAIActionTracker::OnActorDropped(IActor* pActor, EntityId droppedId)
{

}

void CAIActionTracker::OnActorGrab(IActor* pActor, EntityId grabId)
{

}

void CAIActionTracker::OnActorDrop(IActor* pActor, EntityId dropId)
{

}

void CAIActionTracker::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{

}

void CAIActionTracker::OnExitVehicle(IActor* pActor)
{
	//if (!pActor)
	//	return;

	//auto pNewActor = static_cast<CActor*>(pActor);
	//auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pNewActor->m_vehicleStats.lastOperatedVehicleId);
	//if (pVehicle && TOS_Vehicle::ActorIsDriver(pActor))
	//{
	//	auto pAI = pVehicle->GetEntity()->GetAI();
	//	if (!pAI)
	//		return;
	//	
	//	if (IsTracking(pAI))
	//		TOS_AI::AbortAIAction(pAI, -1, "CAIActionTracker::OnExitVehicle: driver out from vehicle");
	//}
}

bool CAIActionTracker::OnInputEvent(const SInputEvent& event)
{
	return true;
}

void CAIActionTracker::Update(float frametime)
{
	for (auto& holderPair : m_voidHolders)
	{
		if (holderPair.second > 0)
			holderPair.second -= frametime;

		if (holderPair.second <= 0)
		{
			holderPair.second = 0;

			auto pEntity = GET_ENTITY(holderPair.first.id);
			if (pEntity)
			{
				const auto actionInfo = holderPair.first.actionInfo;
				const auto pObject = GET_ENTITY(actionInfo.objectId);

				TOS_AI::ExecuteAIAction(pEntity->GetAI(),
					pObject,
					actionInfo.name,
					actionInfo.maxAlertness,
					-1,
					actionInfo.flag,
					actionInfo.desiredGoalPipe, 
					"Implement void fix");

				m_voidHolders.erase(holderPair.first);
			}
			break;
		}
	}

	static float color[] = { 1,1,1,1 };
	const auto size = 1.1f;
	const auto scale = 20;
	const auto xoffset = TOS_Debug::XOFFSET_COMMON;
	const auto yoffset = TOS_Debug::YOFFSET_AIACTION_TRACKER;

	if (g_pGameCVars->tos_debug_draw_aiactiontracker > 0)
	{
		gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, 1.3f, color, false,
			"AI Action Tracker: %i objects", m_entitiesActions.size());
	}

	auto it = m_entitiesActions.cbegin();
	auto end = m_entitiesActions.cend();

	for (; it != end; it++)
	{
		const SAIActionInfo info = it->second;
		const EntityId entityId = it->first;
		const EntityId objectId = info.objectId;
		const SAIActionStats stats = m_entitiesStats[entityId];
		const int index = TOS_STL::GetIndexFromMapKey(m_entitiesActions, entityId);

		const auto pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		if (!pEntity)
			continue;

		const auto pAI = pEntity->GetAI();
		if (!pAI)
			continue;

		const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		const bool isCombat = TOS_AI::IsInCombat(pAI);

		if (g_pGameCVars->tos_debug_draw_aiactiontracker > 0)
		{
			const auto pObject = gEnv->pEntitySystem->GetEntity(objectId);
			const char* const objectName = pObject ? pObject->GetName() : "NullName";

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + index * scale, size, color, false,
				"Tracking Name: %s, Action Name: %s, GoalPipeId: %i, Paused: %i, MaxAlertness: %i, Object Name: %s, Flag: %s",
				pAI->GetName(), info.name, info.goalPipeId, info.paused, info.maxAlertness, objectName, GetString(info.flag));
			
			//gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + index * scale, size, color, false,
			//	"Tracking Name: %s, Action Name: %s, Started %1.f, Finished %1.f,",
			//	pAI->GetName(), stats.actionName, currentTime- stats.lastTimeStarted, currentTime-stats.lastTimeFinished);
		}

		auto pVehicle = TOS_Vehicle::GetVehicle(pEntity);
		if (pVehicle)
		{
			if (!pVehicle->GetDriver())
			{
				TOS_AI::AbortAIAction(pAI, -1, "driver out from vehicle");
			}

			//pVehicle->GetStatus().vel

			//if (pVehicle->GetStatus().
			//{
			//}
		}

		if (isCombat)
		{
			if (info.flag == eAAEF_JoinCombatPauseAction)
			{
				if (!IsPaused(pAI))
				{
					SetPausedAction(pAI, true);

					const auto isConquest = g_pControlSystem->GetConquerorSystem()->IsGamemode();
					if (isConquest)
					{
						const auto isPLV = pVehicle && TOS_Vehicle::IsPLV(pVehicle);
						if (isPLV && TOS_Vehicle::IsHavePassengers(pVehicle))
						{
							const auto seats = pVehicle->GetSeatCount();

							for (auto i = 0; i < seats; i++)
							{
								auto pSeat = pVehicle->GetSeatById(i);
								if (!pSeat)
									continue;

								if (pSeat->IsDriver())
									continue;

								if (pSeat->IsGunner())
									continue;

								pSeat->Exit(true);
								//CryLogAlways("[C++][PLV in combat][%s][Passengers will be unloaded]", pVehicle->GetEntity()->GetName());
							}
						}
					}
				}
			}
		}
		else
		{
			if (info.flag == eAAEF_JoinCombatPauseAction)
			{
				if (IsPaused(pAI))
				{
					SetPausedAction(pAI, false);
				}
			}
		}

		const auto startedTime = currentTime - m_entitiesStats[entityId].lastTimeStarted;
		const auto detectedTime = currentTime - m_entitiesStats[entityId].lastTimeVoidDetect;

		bool isVoidHolder = false;
		IS_VOID_HOLDER(entityId, isVoidHolder);

		if ((!info.paused) && (!isCombat) && (!isVoidHolder) && startedTime >= 1.5f /*&& startedTime < 1.5f*/ && detectedTime > 0.25f)
		{
			//Detect void action goal bug
			if ((info.desiredGoalPipe != 0) && !TOS_AI::IsUsingPipe(pAI, info.desiredGoalPipe))
			{
				m_entitiesStats[entityId].lastTimeVoidDetect = currentTime;
				m_voidHolders[SVoidHolder(entityId, info)] = 0.5f;

				if (g_pGameCVars->tos_debug_log_aiactiontracker)
				{
					CryLogAlways("%s[C++][Detect and Fix Void Action][Victim: %s]",
						STR_RED, pEntity->GetName());
				}

				TOS_AI::AbortAIAction(pAI, info.goalPipeId, false, "Void Action Fix");
			}
		}

	}
}

void CAIActionTracker::Init()
{
	Reset();

	g_pControlSystem->AddChild(this, false);
}

bool CAIActionTracker::IsTracking(IAIObject* pAI) const
{
	if (!pAI)
		return false;

	auto iter = m_entitiesActions.find(pAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		return true;

	return false;
}

bool CAIActionTracker::IsTracking(EntityId Id) const
{
	auto iter = m_entitiesActions.find(Id);
	if (iter != m_entitiesActions.end())
		return true;

	return false;
}

bool CAIActionTracker::StartTracking(IAIObject* pAI, const SAIActionInfo& actionInfo)
{
	if (!pAI)
		return false;

	auto pPipeUser = pAI->CastToIPipeUser();
	if (!pPipeUser)
		return false;

	if (IsTracking(pAI))
		return false;

	pPipeUser->RegisterGoalPipeListener(this, actionInfo.goalPipeId, "CAIActionTracker::StartTracking");
	
	const auto id = pAI->GetEntity()->GetId();

	m_entitiesActions[id] = actionInfo;

	m_entitiesStats[id].actionName = actionInfo.name;
	m_entitiesStats[id].lastTimeStarted = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	return true;
}

void CAIActionTracker::StopTracking(IAIObject* pAI)
{
	if (!pAI)
		return;

	auto pPipeUser = pAI->CastToIPipeUser();
	if (!pPipeUser)
		return;

	auto iter = m_entitiesActions.find(pAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		const auto actionInfo = iter->second;

		if (actionInfo.flag == eAAEF_IgnoreCombatDuringAction)
		{
			if (!TOS_AI::IsCombatEnable(pAI))
			{
				const char* solution = "CAIActionTracker::StopTracking: enable combat because action used DisableCombatDuringAction";

				TOS_AI::EnableCombat(pAI, true, false, solution);
			}
		}

		pPipeUser->UnRegisterGoalPipeListener(this, actionInfo.goalPipeId);
		m_entitiesActions.erase(iter);
	}
}

void CAIActionTracker::StopTracking(EntityId Id)
{
	auto pEntity = GET_ENTITY(Id);
	if (!pEntity)
		return;

	auto pAI = pEntity->GetAI();
	if (!pAI)
		return;

	StopTracking(pAI);

	//auto pPipeUser = pAI->CastToIPipeUser();
	//if (!pPipeUser)
	//	return;

	//auto iter = m_entitiesActions.find(pAI->GetEntity()->GetId());
	//if (iter != m_entitiesActions.end())
	//{
	//	const auto actionInfo = iter->second;

	//	if (actionInfo.flag == eAAEF_IgnoreCombatDuringAction)
	//	{
	//		if (!TOS_AI::IsCombatEnable(pAI))
	//		{
	//			const char* solution = "CAIActionTracker::StopTracking: enable combat because action used DisableCombatDuringAction";

	//			TOS_AI::EnableCombat(pAI, true, false, solution);
	//		}
	//	}

	//	pPipeUser->UnRegisterGoalPipeListener(this, actionInfo.goalPipeId);
	//	m_entitiesActions.erase(iter);
	//}

}

bool CAIActionTracker::IsPaused(IAIObject* pAI)
{
	if (!pAI)
		return false;

	if (!IsTracking(pAI))
		return false;

	SAIActionInfo info;
	GetActionInfo(pAI, info);

	if (info.paused == true)
		return true;

	return false;
}

void CAIActionTracker::SetPausedAction(IAIObject* pAI, bool paused)
{
	if (!pAI)
		return;

	if (!IsTracking(pAI))
		return;

	SAIActionInfo info;
	GetActionInfo(pAI, info);

	if (info.goalPipeId == -1)
		return;

	bool changed = false;

	if (paused)
	{
		if (!info.paused)
		{
			info.paused = true;
			changed = true;
		}
	}
	else
	{
		if (info.paused)
		{
			info.paused = false;
			changed = true;
		}
	}

	//Apply changes
	SetActionInfo(pAI, info);

	//Call events
	if (changed)
	{
		if (paused)
		{
			OnActionPaused(pAI);
		}
		else
		{
			OnActionUnpaused(pAI);
		}
	}

	//CryLogAlways("SetPausedAction: Tracking Name: %s, Action Name: %s, GoalPipeId: %i, Paused: %i, MaxAlertness: %i, Object Name: %s, Flag: %s",
		//pAI->GetName(), info.name, info.goalPipeId, info.paused, info.maxAlertness, "NULL", GetString(info.flag));
}

bool CAIActionTracker::IsFinished(IAIObject* pAI, const char* actionName)
{
	if (!pAI)
		return false;

	//if (!IsTracking(pAI))
	//	return false;

	const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	
	SAIActionStats stats;
	GetActionStats(pAI, stats);

	const auto sameNames = stats.actionName == actionName;
	const auto isNow = (currentTime - stats.lastTimeFinished) < 0.01f;
	const auto isFinished = isNow && sameNames;

	return isFinished;
}

void CAIActionTracker::GetActionInfo(const IAIObject* pAI, SAIActionInfo& actionInfo) const
{
	if (!pAI)
		return;

	EntityId id = pAI->GetEntity()->GetId();
	//GetActionInfo(id, *&actionInfo);
	auto iter = m_entitiesActions.find(id);
	if (iter != m_entitiesActions.end())
	{
		actionInfo.goalPipeId = iter->second.goalPipeId;
		actionInfo.name = iter->second.name;
		actionInfo.flag = iter->second.flag;
		actionInfo.maxAlertness = iter->second.maxAlertness;
		actionInfo.objectId = iter->second.objectId;
		actionInfo.paused = iter->second.paused;
		actionInfo.desiredGoalPipe = iter->second.desiredGoalPipe;
	}
}

void CAIActionTracker::GetActionInfo(EntityId id, SAIActionInfo& actionInfo) const
{
	auto iter = m_entitiesActions.find(id);
	if (iter != m_entitiesActions.end())
	{
		actionInfo.goalPipeId = iter->second.goalPipeId;
		actionInfo.name = iter->second.name;
		actionInfo.flag = iter->second.flag;
		actionInfo.maxAlertness = iter->second.maxAlertness;
		actionInfo.objectId = iter->second.objectId;
		actionInfo.paused = iter->second.paused;
		actionInfo.desiredGoalPipe = iter->second.desiredGoalPipe;
	}
}

bool CAIActionTracker::GetActionStats(const IAIObject* pAI, SAIActionStats& actionStats) const
{
	if (!pAI)
		return false;

	EntityId id = pAI->GetEntity()->GetId();
	auto iter = m_entitiesStats.find(id);
	if (iter != m_entitiesStats.end())
	{
		actionStats.actionName = iter->second.actionName;
		actionStats.lastTimeAborted = iter->second.lastTimeAborted;
		actionStats.lastTimeFailed = iter->second.lastTimeFailed;
		actionStats.lastTimeFinished = iter->second.lastTimeFinished;
		actionStats.lastTimeStarted = iter->second.lastTimeStarted;
		actionStats.lastTimeVoidDetect = iter->second.lastTimeVoidDetect;

		return true;
	}

	return false;
}

void CAIActionTracker::AddListener(IAIActionTrackerListener* pListener)
{
	if (pListener)
		stl::push_back_unique(m_listeners, pListener);
}

void CAIActionTracker::RemoveListener(IAIActionTrackerListener* pListener)
{
	if (pListener)
		stl::find_and_erase(m_listeners, pListener);
}

void CAIActionTracker::OnActionPaused(IAIObject* pAIObject)
{
	if (!pAIObject)
		return;

	//When the action is paused we must cancel current AIAction on entity's AI
	TOS_AI::AbortPausedAIAction(pAIObject, -1, "CAIActionTracker:: OnActionPaused event has been called");

	for (auto pListener : m_listeners)
		pListener->OnActionPaused(pAIObject);
}

void CAIActionTracker::OnActionUnpaused(IAIObject* pAIObject)
{
	TOS_AI::ContinuePausedAIAction(pAIObject, "CAIActionTracker:: OnActionUnpaused event has been called");

	for (auto pListener : m_listeners)
		pListener->OnActionUnpaused(pAIObject);
}

void CAIActionTracker::OnActionAborted(IAIObject* pAIObject)
{
	if (!pAIObject)
		return;

	const EntityId id = pAIObject->GetEntity()->GetId();

	SAIActionInfo action;
	GetActionInfo(pAIObject, action);

	m_entitiesStats[id].actionName = action.name;
	m_entitiesStats[id].lastTimeAborted = gEnv->pTimer->GetFrameStartTime().GetSeconds();
}

void CAIActionTracker::Reset()
{
	for (auto& actionPair : m_entitiesActions)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(actionPair.first);
		if (!pEntity)
			continue;

		IPipeUser* pPipeUser = nullptr;
		if (pEntity->GetAI() && pEntity->GetAI()->CastToIPipeUser())
		{
			pPipeUser = pEntity->GetAI()->CastToIPipeUser();
			pPipeUser->UnRegisterGoalPipeListener(this, actionPair.second.goalPipeId);
		}
	}

	m_entitiesActions.clear();
	m_entitiesStats.clear();
	m_voidHolders.clear();
	//m_listeners.clear();
}

void CAIActionTracker::SetActionInfo(const IAIObject* pAI, const SAIActionInfo& actionInfo)
{
	if (!pAI)
		return;

	const auto id = pAI->GetEntity()->GetId();

	for (auto& actionPair : m_entitiesActions)
	{
		if (actionPair.first == id)
		{
			m_entitiesActions[id].flag = actionInfo.flag;
			m_entitiesActions[id].goalPipeId = actionInfo.goalPipeId;
			m_entitiesActions[id].maxAlertness = actionInfo.maxAlertness;
			m_entitiesActions[id].name = actionInfo.name;
			m_entitiesActions[id].objectId = actionInfo.objectId;
			m_entitiesActions[id].paused = actionInfo.paused;
			m_entitiesActions[id].desiredGoalPipe = actionInfo.desiredGoalPipe;
			break;
		}
	}
}

void CAIActionTracker::Serialize(TSerialize ser)
{

}

void CAIActionTracker::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CAIActionTracker::OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId)
{
	IEntity* pPipeOwner = nullptr;

	for (auto& actionPair : m_entitiesActions)
	{
		if (actionPair.second.goalPipeId == goalPipeId)
		{
			auto pEntity = gEnv->pEntitySystem->GetEntity(actionPair.first);
			if (!pEntity)
				continue;

			pPipeOwner = pEntity;
			break;
		}
	}

	if (!pPipeOwner)
		return;

	auto pAI = pPipeOwner->GetAI();
	if (!pAI)
		return;

	//CryLogAlways("[C++][AIActionTracker][AI %s: OnGoalPipeEvent %s]", pPipeOwner->GetName(), GetString(event));

	const auto id = pPipeOwner->GetId();

	SAIActionInfo info;
	GetActionInfo(pAI, info);

	//CryLogAlways("OnGoalPipeEvent: Tracking Name: %s, Action Name: %s, GoalPipeId: %i, Paused: %i, MaxAlertness: %i, Object Name: %s, Flag: %s",
	//	pAI->GetName(), info.name, info.goalPipeId, info.paused, info.maxAlertness, "NULL", GetString(info.flag));

	bool isVoidHolder = false;
	IS_VOID_HOLDER(pAI->GetEntityID(), isVoidHolder);

	switch (event)
	{
	case ePN_Removed://also called when the ai action on pause or also on start action with goalPipeId == 2
	{
		if (!info.paused && !isVoidHolder)
		{
			//CryLogAlways("Finished");
			m_entitiesStats[id].actionName = info.name;
			m_entitiesStats[id].lastTimeFinished = gEnv->pTimer->GetFrameStartTime().GetSeconds();
			StopTracking(pPipeOwner->GetAI());
		}

		break;
	}	
	case ePN_OwnerRemoved:
	case ePN_Finished:
		m_entitiesStats[id].actionName = info.name;
		m_entitiesStats[id].lastTimeFinished = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	case ePN_Deselected://also called when the AI dies
		//CryLogAlways("Deselected");
		StopTracking(pPipeOwner->GetAI());
		break;
	case ePN_Suspended:
		break;
	case ePN_Resumed:
		break;
	case ePN_AnimStarted:
		break;
	case ePN_RefPointMoved:
		break;
	}

	for (auto pListener : m_listeners)
		pListener->OnGoalPipeEvent(pPipeOwner->GetAI(), event, goalPipeId);
}

string CAIActionTracker::GetString(EGoalPipeEvent event) const
{
	switch (event)
	{
	case ePN_Removed:
		return "Removed";
		break;
	case ePN_OwnerRemoved:
		return "OwnerRemoved";
		break;
	case ePN_Finished:
		return "Finished";
		break;
	case ePN_Deselected:
		return "Deselected";
		break;
	case ePN_Suspended:
		return "Suspended";
		break;
	case ePN_Resumed:
		return "Resumed";
		break;
	case ePN_AnimStarted:
		return "AnimStarted";
		break;
	case ePN_RefPointMoved:
		return "RefPointMoved";
		break;
	}

	return "NullEvent";
}

string CAIActionTracker::GetString(EAAEFlag flag) const
{
	switch (flag)
	{
	case eAAEF_None:
		return "None";
		break;
	case eAAEF_IgnoreCombatDuringAction:
		return "IgnoreCombatDuringAction";
		break;
	case eAAEF_JoinCombatPauseAction:
		return "JoinCombatPauseAction";
		break;
	}

	return "NullFlag";
}
