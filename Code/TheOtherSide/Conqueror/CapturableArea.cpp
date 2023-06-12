#include "StdAfx.h"

#include "IGameObject.h"
#include "CapturableArea.h"


CCapturableArea::CCapturableArea()
{

}

CCapturableArea::~CCapturableArea()
{

}

bool CCapturableArea::Init(IGameObject* pGameObject)
{
	return true;
}

void CCapturableArea::PostInit(IGameObject* pGameObject)
{

}

void CCapturableArea::Release()
{

}

void CCapturableArea::FullSerialize(TSerialize ser)
{

}

bool CCapturableArea::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return false;
}

void CCapturableArea::Update(SEntityUpdateContext& ctx, int updateSlot)
{

}

void CCapturableArea::HandleEvent(const SGameObjectEvent&)
{

}

void CCapturableArea::ProcessEvent(SEntityEvent&)
{

}

void CCapturableArea::SetAuthority(bool auth)
{

}

void CCapturableArea::GetMemoryStatistics(ICrySizer* s)
{

}
