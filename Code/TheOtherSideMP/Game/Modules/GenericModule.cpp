/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

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

void CTOSGenericModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(this);
}

void CTOSGenericModule::Init()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_GameModuleInit, GetName(), false));
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
