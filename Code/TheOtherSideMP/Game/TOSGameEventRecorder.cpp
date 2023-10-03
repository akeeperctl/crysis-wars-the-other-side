#include "StdAfx.h"
#include "TOSGameEventRecorder.h"

#include "Game.h"
#include "TOSGame.h"

#include "Modules/ITOSGameModule.h"
#include "Modules/EntitySpawn/EntitySpawnModule.h"

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

void CTOSGame::OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event)
{
	//This called from IGameplayRecorder and redirected to OnExtraGameplayEvent

	auto event1 = STOSGameEvent(event);
	event1.vanilla_recorder = true;

	this->OnExtraGameplayEvent(pEntity, event1);
}

void CTOSGameEventRecorder::RecordEvent(const EntityId id, const STOSGameEvent& event)
{
	g_pTOSGame->OnExtraGameplayEvent(gEnv->pEntitySystem->GetEntity(id), event);
}

void CTOSGame::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) const
{
	//This code Handle gameplay events from ALL game and The Other Side
	TOS_INIT_EVENT_VALUES(pEntity, event);
	//const char* envName = "";

	//if (gEnv->bServer)
	//{
	//	envName = "[SERVER]";
	//}
	//else if (pGO)
	//{
	//	auto pNetCh = pGO->GetNetChannel();
	//	if (pNetCh && pNetCh->IsLocal())
	//	{
	//		envName = "[LOCAL]";
	//	}

	//}
	//else if (gEnv->bClient)
	//{
	//	envName = "[CLIENT]";
	//}

	//switch(event.event)
	//{
	//case eGE_Disconnected:
	//case eGE_Connected:
	//{
	//	//if (pEntity && event.console_log)
	//	if (pEntity && gEnv->bServer)
	//	{
	//		//Net channel is null on client
	//		auto pNetCh = pGO->GetNetChannel();

	//		//Case 1
	//		CryLogAlways("[C++][%s][%s][CTOSGame::OnExtraGameplayEvent] Event: %s, Name: (CH:%s|GO:%s|Id:%i)",
	//			TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), eventName, pNetCh ? pNetCh->GetName() : "NULL", entName, pEntity->GetId());
	//	}
	//	break;
	//}
	//}

	if ((event.console_log && !event.vanilla_recorder) || event.vanilla_recorder)
	{
		const bool mustDrawDesc = eventDesc.length() > 1;
		const bool mustDrawEnt = entName.length() > 1 || entId > 0;

		if (mustDrawDesc && mustDrawEnt)
		{
			CryLogAlways("[C++][%s][%s][CTOSGame::OnExtraGameplayEvent] Event: %s, Entity: (%s|%i), Desc: %s",
				TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), eventName.c_str(), entName.c_str(), entId, eventDesc.c_str());
		}
		else if (mustDrawEnt && !mustDrawDesc)
		{
			CryLogAlways("[C++][%s][%s][CTOSGame::OnExtraGameplayEvent] Event: %s, Entity: (%s|%i)",
				TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), eventName.c_str(), entName.c_str(), entId);
		}
		else if (mustDrawDesc && !mustDrawEnt)
		{
			CryLogAlways("[C++][%s][%s][CTOSGame::OnExtraGameplayEvent] Event: %s, Desc: %s",
				TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), eventName.c_str(), eventDesc.c_str());
		}
		else
		{
			CryLogAlways("[C++][%s][%s][CTOSGame::OnExtraGameplayEvent] Event: %s",
				TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), eventName.c_str());
		}

		//Case 2
		//CryLogAlways("[C++]%s[FUNC CALL][CTOSGame::OnExtraGameplayEvent]", envName);
		//CryLogAlways("	Event: %s", eventName);
		//CryLogAlways("	Entity: %s", entName);
		//CryLogAlways("	Desc: %s", eventDesc);
	}

	for (const auto pModule : m_modules)
	{
		if (pModule)
			pModule->OnExtraGameplayEvent(pEntity, event);
	}

	for (const auto pModule : m_flowgraphModules)
	{
		if (pModule)
			pModule->OnExtraGameplayEvent(pEntity, event);
	}
}

//IEntitySystemSink
///////////////////////////////////////////////////////////////////////////////
bool CTOSGame::OnBeforeSpawn(SEntitySpawnParams& params)
{
	return true;
}

void CTOSGame::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	//assert(pEntity);

	TOS_RECORD_EVENT(pEntity->GetId(), STOSGameEvent(eEGE_EntityOnSpawn, "", false));
}

bool CTOSGame::OnRemove(IEntity* pEntity)
{
	//assert(pEntity);

	TOS_RECORD_EVENT(pEntity->GetId(), STOSGameEvent(eEGE_EntityOnRemove, "", false));

	return true;
}

void CTOSGame::OnEvent(IEntity* pEntity, SEntityEvent& event)
{

}
///////////////////////////////////////////////////////////////////////////////
//~IEntitySystemSink

//IScriptTableDumpSink
///////////////////////////////////////////////////////////////////////////////
void CTOSGame::OnElementFound(const char* sName, ScriptVarType type)
{

}

void CTOSGame::OnElementFound(int nIdx, ScriptVarType type)
{

}
///////////////////////////////////////////////////////////////////////////////
//~IScriptTableDumpSink
