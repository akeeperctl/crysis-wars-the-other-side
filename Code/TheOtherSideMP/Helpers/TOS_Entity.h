// ReSharper disable CppInconsistentNaming
#pragma once

#include <IEntity.h>
#include "TheOtherSideMP/Game/Modules/EntitySpawn/EntitySpawnModule.h"

namespace TOS_Entity
{
	inline IEntity* Spawn(STOSEntitySpawnParams& params)
	{
		const auto pEntity = CTOSEntitySpawnModule::SpawnEntity(params, true);

		return pEntity;
	}
	inline bool SpawnDelay(STOSEntityDelaySpawnParams& params)
	{
		const auto spawned = CTOSEntitySpawnModule::SpawnEntityDelay(params, true);

		return spawned;
	}
	inline void RemoveEntityForced(const EntityId id)
	{
		CTOSEntitySpawnModule::RemoveEntityForced(id);
	}

}
