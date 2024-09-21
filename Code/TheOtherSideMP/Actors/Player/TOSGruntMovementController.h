/*************************************************************************
Crysis Co-op Source File.
Copyright (C), Crysis Co-op
**************************************************************************/

// Created by Crysis Co-op Developers
// Adapted for TOS by Akeeper

#pragma once

#include <PlayerMovementController.h>

// Summary:
//	Extended movement controller for CCoopGrunt.
class CTOSGruntMovementController
	: public CPlayerMovementController
{
	friend class CTOSGrunt;
private:
	// Summary:
	//	Constructs a CCoopGruntMovementController class instance for the specified CCoopGrunt class instance.
	CTOSGruntMovementController(CTOSGrunt* pGrunt);

	// Summary:
	//	Destructs a CCoopGruntMovementController class instance.
	virtual ~CTOSGruntMovementController();

public:

	// IMovementController

	// Summary:
	//	Specialized movement request handling for CTOSGrunt (for ActorTarget support!)
	virtual bool RequestMovement(CMovementRequest& request);

	// ~IMovementController


private:
	// Pointer to the owner CTOSGrunt class instance.
	CTOSGrunt* m_pGrunt;

	// Boolean indicating whether the previous synchronization had an actor target or not.
	bool m_bHadActorTarget;

	// Query IDs for actor target processing.
	TAnimationGraphQueryID m_nQueryStartID;
	TAnimationGraphQueryID m_nQueryEndID;
	TAnimationGraphQueryID* m_pQueryStartID;
	TAnimationGraphQueryID* m_pQueryEndID;
};
