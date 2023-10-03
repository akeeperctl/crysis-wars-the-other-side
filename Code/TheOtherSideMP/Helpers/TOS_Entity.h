// ReSharper disable CppInconsistentNaming
#pragma once

#include <IEntity.h>
#include "TheOtherSideMP/Game/Modules/EntitySpawn/EntitySpawnModule.h"

namespace TOS_Entity
{
	inline IEntity* Spawn(STOSEntitySpawnParams& params)
	{
		//const auto pEntity = gEnv->pEntitySystem->SpawnEntity(params);
		const auto pEntity = CTOSEntitySpawnModule::SpawnEntity(params, true);


		return pEntity;
	}
}
