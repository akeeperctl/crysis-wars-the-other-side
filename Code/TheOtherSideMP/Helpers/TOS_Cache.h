#pragma once

#include "IItemSystem.h"

namespace TOS_Cache
{
	inline void CacheObject(const char *fileName)
	{
		gEnv->pGame->GetIGameFramework()->GetIItemSystem()->CacheObject(fileName);
	};
}