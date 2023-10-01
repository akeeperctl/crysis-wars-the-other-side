#include "StdAfx.h"

#include "MasterSynchronizer.h"
#include "../../TOSGameEventRecorder.h"

CTOSMasterSynchronizer::CTOSMasterSynchronizer()
{
}

CTOSMasterSynchronizer::~CTOSMasterSynchronizer()
{
}

void CTOSMasterSynchronizer::PostInit(IGameObject* pGameObject)
{
	CTOSGenericSynchronizer::PostInit(pGameObject);
}

void CTOSMasterSynchronizer::Release()
{
	CTOSGenericSynchronizer::Release();
}

void CTOSMasterSynchronizer::FullSerialize(TSerialize ser)
{
	CTOSGenericSynchronizer::FullSerialize(ser);
}

bool CTOSMasterSynchronizer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CTOSGenericSynchronizer::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSMasterSynchronizer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CTOSMasterSynchronizer::HandleEvent(const SGameObjectEvent& event)
{
	CTOSGenericSynchronizer::HandleEvent(event);
}

void CTOSMasterSynchronizer::ProcessEvent(SEntityEvent& event)
{
	CTOSGenericSynchronizer::ProcessEvent(event);
}

void CTOSMasterSynchronizer::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}