// ReSharper disable CppInconsistentNaming
#pragma once

#include <IEntity.h>
#include "TheOtherSideMP/Game/Modules/EntitySpawn/EntitySpawnModule.h"

#define TOS_GET_ENTITY(entityId) gEnv->pEntitySystem->GetEntity(entityId)

namespace TOS_Entity
{
	inline IEntity* Spawn(STOSEntitySpawnParams& params)
	{
		return CTOSEntitySpawnModule::SpawnEntity(params, true);
	}
	inline bool SpawnDelay(STOSEntityDelaySpawnParams& params)
	{
		return CTOSEntitySpawnModule::SpawnEntityDelay(params, true);
	}
	inline void RemoveEntityForced(const EntityId id)
	{
		CTOSEntitySpawnModule::RemoveEntityForced(id);
	}

}
