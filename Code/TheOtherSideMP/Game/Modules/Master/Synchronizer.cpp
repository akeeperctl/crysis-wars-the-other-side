#include "StdAfx.h"

#include "Synchronizer.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

CTOSMasterSynchronizer::CTOSMasterSynchronizer()
{
}

CTOSMasterSynchronizer::~CTOSMasterSynchronizer()
{
}

void CTOSMasterSynchronizer::PostInit(IGameObject* pGameObject)
{
	CTOSGenericSynchronizer::PostInit(pGameObject);

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_SynchronizerCreated, "MasterModule", true));
}

void CTOSMasterSynchronizer::Release()
{
	CTOSGenericSynchronizer::Release();

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_SynchronizerDestroyed, "MasterModule", true));
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