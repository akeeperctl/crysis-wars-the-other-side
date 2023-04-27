#ifndef __DRONE_H__
#define __DRONE_H__

#if _MSC_VER > 1000
# pragma once
#endif
#include "Alien.h"

class CDrone :
	public CAlien
{
public:

	CDrone();
	~CDrone();

	virtual void UpdateGlow(float energy);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);

	virtual IGrabHandler* CreateGrabHanlder();

	virtual bool CreateCodeEvent(SmartScriptTable& rTable);

	virtual void Revive(bool fromInit = false);

	virtual void ProcessRotation(float frameTime);
	virtual void ProcessSwimming(float frameTime);
	virtual void ProcessMovement(float frameTime);

	virtual void	SetActorStance(SMovementRequestParams& control, int& actions)
	{
		// Empty
	}
	virtual void AnimationEvent(ICharacterInstance*, const AnimEventInstance&);

	virtual void UpdateGrab(float frameTime);

	virtual void SetActorMovement(SMovementRequestParams& control);

	virtual void FullSerialize(TSerialize ser);
	//virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags );

	void GetMemoryStatistics(ICrySizer* s);

	//Player can't grab scouts
	virtual int	 GetActorSpecies() { return eGCT_UNKNOWN; }

	virtual void GetActorInfo(SBodyInfo& bodyInfo);
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	//bool EnableSearchBeam(bool enable);
private:

	//void ProcessMovementNew(float frameTime);
	//void ProcessRotationNew(float frameTime);

protected:
};

#endif //__DRONE_H__
