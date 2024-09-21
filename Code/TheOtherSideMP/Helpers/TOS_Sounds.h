/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include <IEntitySystem.h>
#include <IEntityProxy.h>
#include <ISound.h>

namespace TOS_Sounds
{
	//FLAG_SOUND_2D flag for example
	inline bool Play(const IEntity* pEntity, const char* soundName, uint32 flags, ESoundSemantic semantic)
	{
		if (!pEntity)
			return false;

		const auto pProxy = (IEntitySoundProxy*)pEntity->GetProxy(ENTITY_PROXY_SOUND);
		if (!pProxy)
			return false;

		const auto zero = Vec3Constants<float>::fVec3_Zero;
		const auto oneY = Vec3Constants<float>::fVec3_OneY;

		return pProxy->PlaySound(soundName, zero, oneY, flags, semantic);
	}


	//FLAG_SOUND_2D flag for example
	inline bool PlayEx(IEntity* pEntity, const char* soundName, float minRadius, float maxRadius, float volume, uint32 flags, ESoundSemantic semantic)
	{
		if (!pEntity)
			return false;

		const auto pProxy = (IEntitySoundProxy*)pEntity->GetProxy(ENTITY_PROXY_SOUND);
		if (!pProxy)
			return false;

		const auto zero = Vec3Constants<float>::fVec3_Zero;
		const auto oneY = Vec3Constants<float>::fVec3_OneY;

		return pProxy->PlaySoundEx(soundName, zero, oneY, flags, volume, minRadius, maxRadius, semantic);
	}
}