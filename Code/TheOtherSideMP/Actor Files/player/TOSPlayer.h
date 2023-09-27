#pragma once

#include "Player.h"

class CTOSPlayer : public CPlayer
{
	friend class CPlayerMovement;
	friend class CPlayerRotation;
	friend class CPlayerInput;
	friend class CPlayerView;
	friend class CNetPlayerInput;

public:
	CTOSPlayer();
	~CTOSPlayer();


public:
	virtual void PostInit( IGameObject * pGameObject );
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);



protected:
private:
};