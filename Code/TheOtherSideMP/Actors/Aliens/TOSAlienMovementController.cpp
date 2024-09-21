/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include <StdAfx.h>
#include "TOSAlienMovementController.h"
#include "TOSAlien.h"
#include <TheOtherSideMP\Actors\DummyTargetPointVerifier.h>

// Summary:
//	Constructs a CCoopGruntMovementController class instance for the specified CCoopGrunt class instance.
CTOSAlienMovementController::CTOSAlienMovementController(CTOSAlien* pAlien)
	: CCompatibilityAlienMovementController(pAlien)
	, m_pAlien(pAlien)
	, m_bHadActorTarget(false)
	, m_nQueryStartID(0)
	, m_nQueryEndID(0)
	, m_pQueryStartID(&m_nQueryStartID)
	, m_pQueryEndID(&m_nQueryEndID)
{

}

// Summary:
//	Destructs a CCoopGruntMovementController class instance.
CTOSAlienMovementController::~CTOSAlienMovementController()
{
	
}

// Summary:
//	Specialized movement request handling for CCoopGrunt (for ActorTarget support!)
bool CTOSAlienMovementController::RequestMovement(CMovementRequest& request)
{
	if (!gEnv->bServer)
		m_pAlien->GetAnimationGraphState()->SetTargetPointVerifier(&CDummyTargetPointVerifier::Instance);

	// Special ActorTarget handling for CCoopGrunt on server (RMI to client)
	// Only send if it has, or desires to remove the actor target!
	if (gEnv->bServer && request.HasActorTarget())
	{
		this->m_pAlien->SendSpecialMovementRequest(request.m_flags, request.GetActorTarget());
		m_bHadActorTarget = true;
	}

	if (gEnv->bServer && request.RemoveActorTarget() && m_bHadActorTarget)
	{
		this->m_pAlien->SendSpecialMovementRequest(request.m_flags, request.GetActorTarget());
		m_bHadActorTarget = false;
	}

	// Special ActorTarget handling for CCoopGrunt on clients.
	if (!gEnv->bServer && request.HasActorTarget())
	{
		const SActorTargetParams& p = request.GetActorTarget();

		SAnimationTargetRequest req;
		req.position = p.location;
		req.positionRadius = std::max(p.locationRadius, DEG2RAD(0.05f));
		static float minRadius = 0.05f;
		req.startRadius = std::max(minRadius, 2.0f*p.locationRadius);
		if (p.startRadius > minRadius)
			req.startRadius = p.startRadius;
		req.direction = p.direction;
		req.directionRadius = std::max(p.directionRadius, DEG2RAD(0.05f));
		req.prepareRadius = 3.0f;
		req.projectEnd = p.projectEnd;
		req.navSO = p.navSO;

		// This is the part that differs- the start and end IDs.
		IAnimationSpacialTrigger * pTrigger = m_pAlien->GetAnimationGraphState()->SetTrigger(req, p.triggerUser, this->m_pQueryStartID, this->m_pQueryEndID);
		if (pTrigger)
		{
			//if (!p.vehicleName.empty())
			//{
			//	pTrigger->SetInput("Vehicle", p.vehicleName.c_str());
			//	pTrigger->SetInput("VehicleSeat", p.vehicleSeat);
			//}
			//if (p.speed >= 0.0f)
			//{
			//	pTrigger->SetInput(m_inputDesiredSpeed, p.speed);
			//}
			//m_animTargetSpeed = p.speed;
			//pTrigger->SetInput(m_inputDesiredTurnAngleZ, 0);
			if (!p.animation.empty())
			{
				pTrigger->SetInput(p.signalAnimation ? "Signal" : "Action", p.animation.c_str());
			}
			//if (p.stance != STANCE_NULL)
			//{
			//	m_targetStance = p.stance;
			//	pTrigger->SetInput(m_inputStance, m_pAlien->GetStanceInfo(p.stance)->name);
			//}
		}
	}
	else if (!gEnv->bServer && request.RemoveActorTarget())
	{
		if (m_pAlien->GetAnimationGraphState())
			m_pAlien->GetAnimationGraphState()->ClearTrigger(eAGTU_AI);
	}
	

	return CCompatibilityAlienMovementController::RequestMovement(request);
}