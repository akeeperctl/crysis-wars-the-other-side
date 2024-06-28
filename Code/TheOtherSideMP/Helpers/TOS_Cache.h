#pragma once

#include "IItemSystem.h"

namespace TOS_Cache
{
	inline bool CacheObject(const char *fileName)
	{
		gEnv->pGame->GetIGameFramework()->GetIItemSystem()->CacheObject(fileName);
		return true;
	};

	inline bool CacheMaterial(const char* fileName)
	{
		IMaterialManager* matMan = gEnv->p3DEngine->GetMaterialManager();
		if (matMan)
		{
			matMan->LoadMaterial(fileName, false);
			return true;
		}

		return false;
	};
}