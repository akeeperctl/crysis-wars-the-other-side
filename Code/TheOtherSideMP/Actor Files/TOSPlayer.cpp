#include "StdAfx.h"
#include "Player\TOSPlayer.h"

CTOSPlayer::CTOSPlayer()
{
}

CTOSPlayer::~CTOSPlayer()
{
}

void CTOSPlayer::PostInit(IGameObject* pGameObject)
{
	CPlayer::PostInit(pGameObject);
}

void CTOSPlayer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CPlayer::Update(ctx,updateSlot);
}

bool CTOSPlayer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	CPlayer::NetSerialize(ser,aspect,profile,flags);

	return false;
}
