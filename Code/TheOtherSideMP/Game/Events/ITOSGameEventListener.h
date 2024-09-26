#pragma once
#include <IEntity.h>

struct STOSGameEvent;
struct ITOSGameEventListener
{
	virtual void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) = 0;
};