/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include <CompatibilityAlienMovementController.h>
#include "TOSAlien.h"

// Summary:
//	Extended movement controller for CCoopGrunt.
class CTOSAlienMovementController
	: public CCompatibilityAlienMovementController
{
	friend class CTOSAlien;
private:
	// Summary:
	//	Constructs a CCoopGruntMovementController class instance for the specified CCoopGrunt class instance.
	CTOSAlienMovementController(CTOSAlien* pAlien);

	// Summary:
	//	Destructs a CCoopGruntMovementController class instance.
	virtual ~CTOSAlienMovementController();

public:

	// IMovementController

	// Summary:
	//	Specialized movement request handling for CTOSGrunt (for ActorTarget support!)
	virtual bool RequestMovement(CMovementRequest& request);

	// ~IMovementController


private:
	// Pointer to the owner CTOSGrunt class instance.
	CTOSAlien* m_pAlien;

	// Boolean indicating whether the previous synchronization had an actor target or not.
	bool m_bHadActorTarget;

	// Query IDs for actor target processing.
	TAnimationGraphQueryID m_nQueryStartID;
	TAnimationGraphQueryID m_nQueryEndID;
	TAnimationGraphQueryID* m_pQueryStartID;
	TAnimationGraphQueryID* m_pQueryEndID;
};
