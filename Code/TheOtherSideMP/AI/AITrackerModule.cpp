#include "StdAfx.h"
//#include "TheOtherSideMP/Control System/ControlSystem.h"
#include "AITrackerModule.h"

#include "Game.h"
#include "GameCVars.h"
#include "IAIAction.h"
#include "IAITrackerModuleListener.h"

#include "../Helpers/TOS_AI.h"
#include "../Helpers/TOS_Debug.h"
#include "../Helpers/TOS_Entity.h"
#include "../Helpers/TOS_Vehicle.h"

#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Helpers/TOS_STL.h"

#define IS_VOID_HOLDER(entityId, isHolder)\
for (auto& voidPair : m_voidHolders)\
{\
	if (voidPair.first.id == (entityId))\
	{\
		(isHolder) = true;\
		break;\
	}\
}

СTOSAIModule::СTOSAIModule() { Reset(); }

СTOSAIModule::~СTOSAIModule()
{
	//g_pControlSystem->RemoveChild(this, false);
}

bool СTOSAIModule::IsExecuting(const IAIObject* pUserAI, const int actionGoalPipeId)
{
	if (!pUserAI)
		return false;

	const auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		if (iter->second.goalPipeId == actionGoalPipeId)
			return true;

	return false;
}

bool СTOSAIModule::IsExecuting(const IAIObject* pUserAI, const char* actionName)
{
	if (!pUserAI)
		return false;

	const auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		if (strcmp(iter->second.name, actionName) == 0)
			return true;

	return false;
}

bool СTOSAIModule::IsExecuting(const IAIObject* pUserAI, const char* actionName, const IEntity* pObject)
{
	if (!pUserAI || !pObject)
		return false;

	const auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		if (strcmp(iter->second.name, actionName) == 0)
		{
			const auto pAIAction = gEnv->pAISystem->GetAIAction(actionName);
			if (pAIAction && pAIAction->GetObjectEntity() == pObject)
				return true;
		}

	return false;
}

bool СTOSAIModule::IsExecuting(const IAIObject* pUserAI, const char* actionName, const char* desiredGoalPipe)
{
	if (!pUserAI)
		return false;

	const auto iter = m_entitiesActions.find(pUserAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		if ((strcmp(iter->second.name, actionName) == 0) && (strcmp(iter->second.desiredGoalPipe, desiredGoalPipe) == 0))
			return true;

	return false;
}

void СTOSAIModule::Update(const float frametime)
{
	for (auto& holderPair : m_voidHolders)
	{
		if (holderPair.second > 0)
			holderPair.second -= frametime;

		if (holderPair.second <= 0)
		{
			holderPair.second = 0;

			const auto pEntity = TOS_GET_ENTITY(holderPair.first.id);
			if (pEntity)
			{
				const auto actionInfo = holderPair.first.actionInfo;
				const auto pObject = TOS_GET_ENTITY(actionInfo.objectId);

				TOS_AI::ExecuteAIAction(pEntity->GetAI(), pObject, actionInfo.name, actionInfo.maxAlertness, -1, actionInfo.flag, actionInfo.desiredGoalPipe, "Implement void fix");

				m_voidHolders.erase(holderPair.first);
			}
			break;
		}
	}

	static float color[] = {1, 1, 1, 1};
	constexpr auto   size = 1.1f;
	constexpr auto   scale = 20;
	constexpr auto   xoffset = TOS_Debug::XOFFSET_COMMON;
	constexpr auto   yoffset = TOS_Debug::YOFFSET_AIACTION_TRACKER;

	if (GetDebugLog() > 0)
		gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, 1.3f, color, false, "AI Action Tracker: %i objects", m_entitiesActions.size());

	auto       it = m_entitiesActions.cbegin();
	const auto end = m_entitiesActions.cend();

	for (; it != end; ++it)
	{
		const SAIActionInfo  info = it->second;
		const EntityId       entityId = it->first;
		const EntityId       objectId = info.objectId;
		const SAIActionStats stats = m_entitiesStats[entityId];
		const int            index = TOS_STL::GetIndexFromMapKey(m_entitiesActions, entityId);

		const auto pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		if (!pEntity)
			continue;

		const auto pAI = pEntity->GetAI();
		if (!pAI)
			continue;

		const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		const bool  isCombat = TOS_AI::IsInCombat(pAI);

		if (GetDebugLog() > 0)
		{
			const auto        pObject = gEnv->pEntitySystem->GetEntity(objectId);
			const char* const objectName = pObject ? pObject->GetName() : "NullName";

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + index * scale, size, color, false, "Tracking Name: %s, Action Name: %s, GoalPipeId: %i, Paused: %i, MaxAlertness: %i, Object Name: %s, Flag: %s", pAI->GetName(), info.name, info.goalPipeId, info.paused, info.maxAlertness, objectName, GetString(info.flag));

			//gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + index * scale, size, color, false,
			//	"Tracking Name: %s, Action Name: %s, Started %1.f, Finished %1.f,",
			//	pAI->GetName(), stats.actionName, currentTime- stats.lastTimeStarted, currentTime-stats.lastTimeFinished);
		}

		const auto pVehicle = TOS_Vehicle::GetVehicle(pEntity);
		if (pVehicle)
			if (!pVehicle->GetDriver())
				TOS_AI::AbortAIAction(pAI, -1, "driver out from vehicle");

		//pVehicle->GetStatus().vel

		//if (pVehicle->GetStatus().
		//{
		//}

		if (isCombat)
		{
			if (info.flag == eAAEF_JoinCombatPauseAction)
				if (!IsPaused(pAI))
				{
					SetPausedAction(pAI, true);

					/*
					const auto isConquest = g_pControlSystem->GetConquerorSystem()->IsGamemode();
					if (isConquest)
					{
						const auto isPLV = pVehicle && TOS_Vehicle::IsPLV(pVehicle);
						if (isPLV && TOS_Vehicle::IsHavePassengers(pVehicle))
						{
							const auto seats = pVehicle->GetSeatCount();

							for (auto i = 0; i < seats; i++)
							{
								const auto pSeat = pVehicle->GetSeatById(i);
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
					*/
				}
		}
		else
		{
			if (info.flag == eAAEF_JoinCombatPauseAction)
				if (IsPaused(pAI))
					SetPausedAction(pAI, false);
		}

		const auto startedTime = currentTime - m_entitiesStats[entityId].lastTimeStarted;
		const auto detectedTime = currentTime - m_entitiesStats[entityId].lastTimeVoidDetect;

		bool isVoidHolder = false;
		IS_VOID_HOLDER(entityId, isVoidHolder);

		if ((!info.paused) && (!isCombat) && (!isVoidHolder) && startedTime >= 1.5f /*&& startedTime < 1.5f*/ && detectedTime > 0.25f)
			//Detect void action goal bug
			if ((info.desiredGoalPipe != nullptr) && !TOS_AI::IsUsingPipe(pAI, info.desiredGoalPipe))
			{
				m_entitiesStats[entityId].lastTimeVoidDetect = currentTime;
				m_voidHolders[SVoidHolder(entityId, info)] = 0.5f;

				if (GetDebugLog() > 0)
					CryLogAlways("%s[C++][Detect and Fix Void Action][Victim: %s]", STR_RED, pEntity->GetName());

				TOS_AI::AbortAIAction(pAI, info.goalPipeId, false, "Void Action Fix");
			}
	}
}

void СTOSAIModule::Init()
{
	Reset();

	//g_pControlSystem->AddChild(this, false);

	CTOSGenericModule::Init();
}

bool СTOSAIModule::IsTracking(const IAIObject* pAI) const
{
	if (!pAI)
		return false;

	const auto iter = m_entitiesActions.find(pAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
		return true;

	return false;
}

bool СTOSAIModule::IsTracking(const EntityId Id) const
{
	const auto iter = m_entitiesActions.find(Id);
	if (iter != m_entitiesActions.end())
		return true;

	return false;
}

bool СTOSAIModule::StartTracking(IAIObject* pAI, const SAIActionInfo& actionInfo)
{
	if (!pAI)
		return false;

	const auto pPipeUser = pAI->CastToIPipeUser();
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

void СTOSAIModule::StopTracking(IAIObject* pAI)
{
	if (!pAI)
		return;

	const auto pPipeUser = pAI->CastToIPipeUser();
	if (!pPipeUser)
		return;

	const auto iter = m_entitiesActions.find(pAI->GetEntity()->GetId());
	if (iter != m_entitiesActions.end())
	{
		const auto actionInfo = iter->second;

		if (actionInfo.flag == eAAEF_IgnoreCombatDuringAction)
			if (!TOS_AI::IsCombatEnable(pAI))
			{
				const char* solution = "CAIActionTracker::StopTracking: enable combat because action used DisableCombatDuringAction";

				TOS_AI::EnableCombat(pAI, true, false, solution);
			}

		pPipeUser->UnRegisterGoalPipeListener(this, actionInfo.goalPipeId);
		m_entitiesActions.erase(iter);
	}
}

void СTOSAIModule::StopTracking(const EntityId Id)
{
	const auto pEntity = TOS_GET_ENTITY(Id);
	if (!pEntity)
		return;

	const auto pAI = pEntity->GetAI();
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

bool СTOSAIModule::IsPaused(IAIObject* pAI) const
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

void СTOSAIModule::SetPausedAction(IAIObject* pAI, const bool paused)
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
			OnActionPaused(pAI);
		else
			OnActionUnpaused(pAI);
	}

	//CryLogAlways("SetPausedAction: Tracking Name: %s, Action Name: %s, GoalPipeId: %i, Paused: %i, MaxAlertness: %i, Object Name: %s, Flag: %s",
	//pAI->GetName(), info.name, info.goalPipeId, info.paused, info.maxAlertness, "NULL", GetString(info.flag));
}

bool СTOSAIModule::IsFinished(const IAIObject* pAI, const char* actionName) const
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

void СTOSAIModule::GetActionInfo(const IAIObject* pAI, SAIActionInfo& actionInfo) const
{
	if (!pAI)
		return;

	const EntityId id = pAI->GetEntity()->GetId();
	//GetActionInfo(id, *&actionInfo);
	const auto iter = m_entitiesActions.find(id);
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

void СTOSAIModule::GetActionInfo(const EntityId id, SAIActionInfo& actionInfo) const
{
	const auto iter = m_entitiesActions.find(id);
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

bool СTOSAIModule::GetActionStats(const IAIObject* pAI, SAIActionStats& actionStats) const
{
	if (!pAI)
		return false;

	const EntityId id = pAI->GetEntity()->GetId();
	const auto     iter = m_entitiesStats.find(id);
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
/*
void СTOSAIModule::AddListener(IAIActionTrackerListener* pListener)
{
	if (pListener)
		stl::push_back_unique(m_listeners, pListener);
}

void СTOSAIModule::RemoveListener(IAIActionTrackerListener* pListener)
{
	if (pListener)
		stl::find_and_erase(m_listeners, pListener);
}
*/
void СTOSAIModule::OnActionPaused(IAIObject* pAIObject) const
{
	if (!pAIObject)
		return;

	//When the action is paused we must cancel current AIAction on entity's AI
	TOS_AI::AbortPausedAIAction(pAIObject, -1, "CAIActionTracker:: OnActionPaused event has been called");

	//for (const auto pListener : m_listeners) { pListener->OnActionPaused(pAIObject); }
}

void СTOSAIModule::OnActionUnpaused(IAIObject* pAIObject) const
{
	TOS_AI::ContinuePausedAIAction(pAIObject, "CAIActionTracker:: OnActionUnpaused event has been called");

	//for (const auto pListener : m_listeners) { pListener->OnActionUnpaused(pAIObject); }
}

void СTOSAIModule::OnActionAborted(const IAIObject* pAIObject)
{
	if (!pAIObject)
		return;

	const EntityId id = pAIObject->GetEntity()->GetId();

	SAIActionInfo action;
	GetActionInfo(pAIObject, action);

	m_entitiesStats[id].actionName = action.name;
	m_entitiesStats[id].lastTimeAborted = gEnv->pTimer->GetFrameStartTime().GetSeconds();
}

void СTOSAIModule::Reset()
{
	for (const auto& actionPair : m_entitiesActions)
	{
		const auto pEntity = gEnv->pEntitySystem->GetEntity(actionPair.first);
		if (!pEntity)
			continue;

		if (pEntity->GetAI() && pEntity->GetAI()->CastToIPipeUser())
		{
			IPipeUser* pPipeUser = pEntity->GetAI()->CastToIPipeUser();
			assert(pPipeUser);

			pPipeUser->UnRegisterGoalPipeListener(this, actionPair.second.goalPipeId);
		}
	}

	m_entitiesActions.clear();
	m_entitiesStats.clear();
	m_voidHolders.clear();
	//m_listeners.clear();
}

void СTOSAIModule::SetActionInfo(const IAIObject* pAI, const SAIActionInfo& actionInfo)
{
	if (!pAI)
		return;

	const auto id = pAI->GetEntity()->GetId();

	for (const auto& actionPair : m_entitiesActions)
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

void СTOSAIModule::Serialize(TSerialize ser) {}

void СTOSAIModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	switch (event.event)
	{
	case eEGE_ActorDead:
	{
		if (!pEntity)
			break;

		const auto pAI = pEntity->GetAI();
		if (!pAI)
			break;

		if (!IsTracking(pAI))
			break;

		const EntityId id = pAI->GetEntity()->GetId();

		SAIActionInfo info;
		GetActionInfo(pAI, info);

		m_entitiesStats[id].actionName = info.name;
		m_entitiesStats[id].lastTimeFailed = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		StopTracking(pAI);

		//for (const auto pListener : m_listeners) { pListener->OnActorDeath(pActor); }

		break;
	}
	case eEGE_VehicleDestroyed:
	{
		if (!pEntity)
			break;

		if (!IsTracking(entId))
			break;

		SAIActionInfo info;
		GetActionInfo(entId, info);

		m_entitiesStats[entId].actionName = info.name;
		m_entitiesStats[entId].lastTimeFailed = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		StopTracking(entId);

		break;
	}
	case eEGE_MainMenuOpened:
	case eEGE_GamerulesReset:
		Reset();
		break;
	default:
		break;
	}
}

void СTOSAIModule::GetMemoryStatistics(ICrySizer* s) { s->Add(*this); }

void СTOSAIModule::OnGoalPipeEvent(IPipeUser* pPipeUser, const EGoalPipeEvent event, const int goalPipeId)
{
	IEntity* pPipeOwner = nullptr;

	for (const auto& actionPair : m_entitiesActions)
	{
		if (actionPair.second.goalPipeId == goalPipeId)
		{
			const auto pEntity = gEnv->pEntitySystem->GetEntity(actionPair.first);
			if (!pEntity)
				continue;

			pPipeOwner = pEntity;
			break;
		}
	}

	if (!pPipeOwner)
		return;

	const auto pAI = pPipeOwner->GetAI();
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
	case ePN_Removed: //also called when the ai action on pause or also on start action with goalPipeId == 2
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
	case ePN_Deselected: //also called when the AI dies
		//CryLogAlways("Deselected");
		StopTracking(pPipeOwner->GetAI());
		break;
	case ePN_Suspended:
	case ePN_Resumed:
	case ePN_AnimStarted:
	case ePN_RefPointMoved:
		break;
	}

	//for (const auto pListener : m_listeners) { pListener->OnGoalPipeEvent(pPipeOwner->GetAI(), event, goalPipeId); }
}
