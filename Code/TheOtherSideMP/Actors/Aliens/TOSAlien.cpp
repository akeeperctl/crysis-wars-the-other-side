#include "StdAfx.h"
#include "TOSAlien.h"

CTOSAlien::CTOSAlien()
{
}

CTOSAlien::~CTOSAlien()
{
}

void CTOSAlien::PostInit(IGameObject* pGameObject)
{
	CAlien::PostInit(pGameObject);
}

void CTOSAlien::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CAlien::Update(ctx, updateSlot);
}

// ReSharper disable once CppParameterMayBeConst
bool CTOSAlien::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	CAlien::NetSerialize(ser,aspect,profile,flags);
	return false;
}
