#pragma once

#include "Alien.h"

class CTOSAlien : public CAlien
{
public:
	CTOSAlien();
	~CTOSAlien();


public:
	virtual void PostInit(IGameObject* pGameObject);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);



protected:
private:
};