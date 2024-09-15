// ReSharper disable CppInconsistentNaming
#pragma once

#include <IEntity.h>
#include "TheOtherSideMP/Game/Modules/EntitySpawn/EntitySpawnModule.h"

#define TOS_GET_ENTITY_CLASS(name) gEnv->pEntitySystem->GetClassRegistry()->FindClass(name)
#define TOS_GET_ENTITY(entityId) gEnv->pEntitySystem->GetEntity(entityId)
#define TOS_GET_ACTOR(entityId) g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId)
#define TOS_GET_VEHICLE(entityId) g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(entityId)
#define TOS_GET_ITEM(entityId) g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(entityId)

namespace TOS_Entity
{
	inline IEntity* Spawn(STOSEntitySpawnParams& params, bool saveParams)
	{
		return CTOSEntitySpawnModule::SpawnEntity(params, saveParams);
	}
	inline bool SpawnDelay(STOSEntityDelaySpawnParams& params, bool saveParams)
	{
		return CTOSEntitySpawnModule::SpawnEntityDelay(params, saveParams);
	}
	inline void RemoveEntityForced(const EntityId id)
	{
		CTOSEntitySpawnModule::RemoveEntityForced(id);
	}
	inline void RemoveEntityDelayed(const EntityId id, int delayInFrames)
	{
		g_pTOSGame->GetEntitySpawnModule()->RemoveEntityDelayed(id, delayInFrames);
	}

}
