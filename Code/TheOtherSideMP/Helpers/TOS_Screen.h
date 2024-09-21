/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include "IRenderer.h"

namespace TOS_Screen
{
	inline void ProjectToScreen(const Vec3& worldCoords, Vec3& screenCoords)
	{
		gEnv->pRenderer->ProjectToScreen(worldCoords.x, worldCoords.y, worldCoords.z, &screenCoords.x, &screenCoords.y, &screenCoords.z);
		const float fWidth = gEnv->pRenderer->GetWidth();
		const float fHeight = gEnv->pRenderer->GetHeight();

		//scale projected values to the actual screen resolution
		screenCoords.x *= 0.01f * fWidth;
		screenCoords.y *= 0.01f * fHeight;
	}

	inline Vec3 ProjectToScreen(const Vec3& worldCoords)
	{
		Vec3 screenCoords(ZERO);

		gEnv->pRenderer->ProjectToScreen(worldCoords.x, worldCoords.y, worldCoords.z, &screenCoords.x, &screenCoords.y, &screenCoords.z);
		const float fWidth = gEnv->pRenderer->GetWidth();
		const float fHeight = gEnv->pRenderer->GetHeight();

		//scale projected values to the actual screen resolution
		screenCoords.x *= 0.01f * fWidth;
		screenCoords.y *= 0.01f * fHeight;

		return screenCoords;
	}
}