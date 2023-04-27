/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// IGameFlashAnimationFactory.h
//
// Purpose: Interface for helper to creating a flash animation
//	object, registered with the Game HUD for updating/rendering
//
// History:
//	- 3/29/09 : File created - KAK
/////////////////////////////////////////////////////////////////

#ifndef _IGAMEFLASHANIMATIONFACTORY_H_
#define _IGAMEFLASHANIMATIONFACTORY_H_

#include "IGameFlashAnimation.h"

struct IGameFlashAnimationFactory
{
	virtual ~IGameFlashAnimationFactory() {}

	// Creates a game flash animation for usage
	virtual IGameFlashAnimation* CreateGameFlashAnimation() const = 0;

	// Deletes a game flash animation
	virtual void DeleteGameFlashAnimation(IGameFlashAnimation *pAnim) const = 0;

	// (Un)Register a game flash animation to the HUD
	virtual bool EnableGameFlashAnimation(IGameFlashAnimation *pAnim, bool bEnable) const = 0;
};

#endif //_IGAMEFLASHANIMATIONFACTORY_H_
