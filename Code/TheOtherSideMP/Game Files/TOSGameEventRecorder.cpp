#include "StdAfx.h"

#include "ITOSGameModule.h"

#include "Game.h"
#include "TOSGame.h"
#include "TOSGameEventRecorder.h"

CTOSGameEventRecorder::CTOSGameEventRecorder()
{

}

void CTOSGame::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_mouseScreenPos.x = iX;
	m_mouseScreenPos.y = iY;

	iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, iY, 0.0f, &m_mouseWorldPos.x, &m_mouseWorldPos.y, &m_mouseWorldPos.z);

	//CryLogAlways("[CControlSystem::OnHardwareMouseEvent][WorldPos] x%1.f, y%1.f, z%1.f", m_mouseWorldPos.x, m_mouseWorldPos.y, m_mouseWorldPos.z);
}

bool CTOSGame::OnInputEvent(const SInputEvent& event)
{
	if (gEnv->bEditor && g_pGame->GetIGameFramework()->IsEditing())
		return false;

	for (const auto pModule : m_modules)
	{
		if (pModule)
		{
			pModule->OnInputEvent(event);
		}
	}

	for (const auto pFGModule : m_flowgraphModules)
	{
		if (pFGModule)
		{
			pFGModule->OnInputEvent(event);
		}	
	}

	//if (m_pSquadSystem)
	//{
	//	m_pSquadSystem->OnInputEvent(event);
	//}

	//if (m_pConquerorSystem)
	//{
	//	m_pConquerorSystem->OnInputEvent(event);
	//}

	//if (m_pAbilitiesSystem)
	//{
	//	m_pAbilitiesSystem->OnInputEvent(event);
	//}

	return false;
}

void CTOSGame::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	//This code Handle gameplay events from ALL game and The Other Side

	switch(event.event)
	{
	case eGE_Connected:
		if (pEntity && event.console_log)
			CryLogAlways("[C++][FuncCall][CTOSGame::OnExtraGameplayEvent] Event: eGE_Connected, Entity: %s", 
				pEntity->GetName());
		break;
	}

	if (event.console_log && !event.vanilla_recorder)
	{
		CryLogAlways("[C++][FuncCall][CTOSGame::OnExtraGameplayEvent] Event: %s, Desc: %s",
			m_pEventRecorder->GetStringFromEnum(event.event), event.description);
	}

	for (auto pModule : m_modules)
	{
		if (pModule)
			pModule->OnExtraGameplayEvent(pEntity, event);
	}
}

void CTOSGame::OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event)
{
	//This called from IGameplayRecorder and redirected to OnExtraGameplayEvent

	auto event1 = STOSGameEvent(event);
	event1.vanilla_recorder = true;

	this->OnExtraGameplayEvent(pEntity, event1);
}

void CTOSGameEventRecorder::RecordEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	g_pTOSGame->OnExtraGameplayEvent(pEntity, event);
}

const char* CTOSGameEventRecorder::GetStringFromEnum(int value)
{
	switch (value)
	{
	case eEGE_MainMenuOpened:
		return "eEGE_MainMenuOpened";
		break;
	case eEGE_ActorGrabbed:
		return "eEGE_ActorGrabbed";
		break;
	case eEGE_ActorDropped:
		return "eEGE_ActorDropped";
		break;
	case eEGE_ActorGrab:
		return "eEGE_ActorGrab";
		break;
	case eEGE_ActorDrop:
		return "eEGE_ActorDrop";
		break;
	case eEGE_GamerulesReset:
		return "eEGE_GamerulesReset";
		break;
	case eEGE_MasterStartControl:
		return "eEGE_MasterStartControl";
		break;
	case eEGE_MasterStopControl:
		return "eEGE_MasterStopControl";
		break;
	case eEGE_MasterEnterSpectator:
		return "eEGE_MasterEnterSpectator";
		break;
	case eEGE_SlaveStartObey:
		return "eEGE_SlaveStartObey";
		break;
	case eEGE_SlaveStopObey:
		return "eEGE_SlaveStopObey";
		break;
	case eEGE_EditorGameEnter:
		return "eEGE_EditorGameEnter";
		break;
	case eEGE_EditorGameExit:
		return "eEGE_EditorGameExit";
		break;
	case eEGE_EnterGame:
		return "eEGE_EnterGame";
		break;
	case eEGE_EnterSpectator:
		return "eEGE_EnterSpectator";
		break;
	case eEGE_Last:
		return "eEGE_Last";
		break;
	}


	return "EExtraGameplayEvent UNDEFINED";
}
