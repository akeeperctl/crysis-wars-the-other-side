#include "StdAfx.h"
#include "GenericModule.h"

#include "Game.h"
#include "GenericSynchronizer.h"
#include "Master/MasterSynchronizer.h"

#include "../TOSGame.h"
#include "../TOSGameEventRecorder.h"

CTOSGenericModule::CTOSGenericModule()
	: m_debugLogMode(0)
{
	m_pSynchonizer = nullptr;
	g_pTOSGame->ModuleAdd(this, false);
}

CTOSGenericModule::~CTOSGenericModule()
{
	g_pTOSGame->ModuleRemove(this, false);
}

void CTOSGenericModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{

}

void CTOSGenericModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(this);
}

void CTOSGenericModule::Init()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_GameModuleInit, GetName(), true));
}

void CTOSGenericModule::Update(float frametime)
{
}

void CTOSGenericModule::Serialize(TSerialize ser)
{
}

bool CTOSGenericModule::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

CTOSGenericSynchronizer* CTOSGenericModule::GetSynchronizer() const
{
	return m_pSynchonizer;
}

void CTOSGenericModule::RegisterSynchronizer(CTOSGenericSynchronizer* pSynch)
{
	assert(pSynch);
	m_pSynchonizer = pSynch;
}
