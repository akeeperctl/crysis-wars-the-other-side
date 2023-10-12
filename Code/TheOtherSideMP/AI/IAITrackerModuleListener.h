#pragma once

#include "IAgent.h"

struct IAIActionTrackerListener
{
	virtual void OnGoalPipeEvent(IAIObject* pObject, EGoalPipeEvent event, int goalPipeId) = 0;
	virtual void OnActionPaused(IAIObject* pObject) = 0;
	virtual void OnActionUnpaused(IAIObject* pObject) = 0;
	virtual void OnActorDeath(IActor* pActor) = 0;

};