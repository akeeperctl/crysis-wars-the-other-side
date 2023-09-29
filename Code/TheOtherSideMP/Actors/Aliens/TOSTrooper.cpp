#include "StdAfx.h"
#include "TOSTrooper.h"

CTOSTrooper::CTOSTrooper()
{
}

CTOSTrooper::~CTOSTrooper()
{
}

void CTOSTrooper::PostInit(IGameObject* pGameObject)
{
	CTrooper::PostInit(pGameObject);
}

void CTOSTrooper::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CTrooper::Update(ctx,updateSlot);
}

bool CTOSTrooper::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	CTrooper::NetSerialize(ser, aspect, profile, flags);
	return false;
}
