#pragma once

#include <IAnimationGraph.h>

// Summary:
//	Dummy target point verifier to prevent AI system based crashes on client.
class CDummyTargetPointVerifier
	: public IAnimationGraphTargetPointVerifier
{
public:
	CDummyTargetPointVerifier()
	{

	}

	virtual ~CDummyTargetPointVerifier()
	{

	}

	static CDummyTargetPointVerifier Instance;

	/// Returns true if the path can be modified to use request.targetPoint, and byproducts 
	/// of the test are cached in request.
	virtual ETriState CanTargetPointBeReached(class CTargetPointRequest &request) const { return ETriState::eTS_true; }
	/// Returns true if the request is still valid/can be used, false otherwise.
	virtual bool UseTargetPointRequest(const class CTargetPointRequest &request) { return true; }
	virtual void NotifyFinishPoint(const Vec3& pt){}
	virtual void NotifyAllPointsNotReachable() {}
};