#include "StdAfx.h"
#include "GenericModule.h"

#include "Game.h"
#include "GenericSynchronizer.h"
#include "Master/MasterSynchronizer.h"

#include "../TOSGame.h"
#include "../TOSGameEventRecorder.h"

CTOSGenericModule::CTOSGenericModule()
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

CTOSGenericSynchronizer* CTOSGenericModule::GetSynchronizer() const
{
	return m_pSynchonizer;
}
