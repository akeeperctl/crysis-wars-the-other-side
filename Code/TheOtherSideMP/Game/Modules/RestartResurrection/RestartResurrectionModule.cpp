#include "StdAfx.h"

#include "RestartResurrectionModule.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

#include "Game.h"

CTOSRestartResurrectionModule::CTOSRestartResurrectionModule()
{
	
}

CTOSRestartResurrectionModule::~CTOSRestartResurrectionModule()
{
}

void CTOSRestartResurrectionModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	//TOS_INIT_EVENT_VALUES(pEntity, event);
}

void CTOSRestartResurrectionModule::Init()
{
	CTOSGenericModule::Init();
}

void CTOSRestartResurrectionModule::Update(float frametime)
{
}

void CTOSRestartResurrectionModule::Serialize(TSerialize ser)
{
}
