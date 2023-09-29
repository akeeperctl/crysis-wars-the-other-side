#pragma once

#include "Trooper.h"

class CTOSTrooper : public CTrooper
{
public:
	CTOSTrooper();
	~CTOSTrooper();


public:
	virtual void PostInit(IGameObject* pGameObject);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);



protected:
private:
};