/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "TOSGameEventRecorder.h"

#include "Actor.h"
#include "Game.h"
#include "TOSGame.h"
#include "TOSGameCvars.h"

#include "Modules/ITOSGameModule.h"

#define LOG_EVENT(eventName, entName, entId, eventDesc)\
const bool mustDrawDesc = (eventDesc).length() > 1;\
const bool mustDrawEnt = (entName).length() > 1 || (entId) > 0;\
if (mustDrawDesc && mustDrawEnt)\
{\
	CryLog("[ExtraGameplayEvent] [%s] %s: %s, %s",\
		TOS_Debug::GetEnv(), (eventName).c_str(), (entName).c_str(), (eventDesc).c_str());\
}\
else if (mustDrawEnt && !mustDrawDesc)\
{\
	CryLog("[OnExtraGameplayEvent] [%s]: %s %s",\
		TOS_Debug::GetEnv(), (eventName).c_str(), (entName).c_str());\
}\
else if (mustDrawDesc && !mustDrawEnt)\
{\
	CryLog("[OnExtraGameplayEvent] [%s]: %s %s",\
		TOS_Debug::GetEnv(), (eventName).c_str(), (eventDesc).c_str());\
}\
else\
{\
	CryLog("[OnExtraGameplayEvent] [%s]: %s",\
		TOS_Debug::GetEnv(), (eventName).c_str());\
}\


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

	for (std::vector<ITOSGameModule*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->OnInputEvent(event);
		}
	}

	for (std::vector<ITOSGameModule*>::iterator it = m_flowgraphModules.begin(); it != m_flowgraphModules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->OnInputEvent(event);
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

	const bool logVanillaEvents = event.vanilla_recorder && g_pTOSGameCvars->tos_any_EventRecorderLogVanilla > 0;
	const bool logExtraEvents = event.console_log && !event.vanilla_recorder;

	if (logExtraEvents || logVanillaEvents)
	{
		if (event.event == eGE_Spectator)
		{
			const char* modeName = nullptr;

			switch (static_cast<int>(event.value))
			{
			case CActor::eASM_None:
				modeName = " eASM_None";
				break;
			case CActor::eASM_Fixed:
				modeName = " eASM_Fixed";
				break;
			case CActor::eASM_Free:
				modeName = " eASM_Free";
				break;
			case CActor::eASM_Follow:
				modeName = " eASM_Follow";
				break;
			case CActor::eASM_Cutscene:
				modeName = " eASM_Cutscene";
				break;
			default: 
				break;
			}

			eventDesc.append(modeName);
		}

		LOG_EVENT(eventName, entName, entId, eventDesc);



		//Case 2
		//CryLogAlways("[C++]%s[FUNC CALL][CTOSGame::OnExtraGameplayEvent]", envName);
		//CryLogAlways("	Event: %s", eventName);
		//CryLogAlways("	Entity: %s", entName);
		//CryLogAlways("	Desc: %s", eventDesc);
	}

	for (auto it = m_gameEventListeners.begin(); it != m_gameEventListeners.end(); ++it)
	{
		auto pEventListener = *it;
		if (pEventListener)
			pEventListener->OnExtraGameplayEvent(pEntity, event);
	}

	//for (std::vector<ITOSGameModule*>::const_iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	//{
	//	ITOSGameModule* pModule = *it;
	//	if (pModule)
	//		pModule->OnExtraGameplayEvent(pEntity, event);
	//}

	//for (std::vector<ITOSGameModule*>::const_iterator it = m_flowgraphModules.begin(); it != m_flowgraphModules.end(); ++it)
	//{
	//	ITOSGameModule* pModule = *it;
	//	if (pModule)
	//		pModule->OnExtraGameplayEvent(pEntity, event);
	//}
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
	const char* typeName = nullptr;

	switch (type)
	{
	case svtNull:
		typeName = "svtNull    ";
		break;
	case svtString:
		typeName = "svtString  ";
		break;
	case svtNumber:
		typeName = "svtNumber  ";
		break;
	case svtBool:
		typeName = "svtBool    ";
		break;
	case svtFunction:
		typeName = "svtFunction";
		break;
	case svtObject:
		typeName = "svtObject  ";
		break;
	case svtPointer:
		typeName = "svtPointer ";
		break;
	case svtUserData:
		typeName = "svtUserData";
		break;
	}

	CryLog("	Element: %s\t%s",typeName, sName);
}

void CTOSGame::OnElementFound(int nIdx, ScriptVarType type)
{

}
///////////////////////////////////////////////////////////////////////////////
//~IScriptTableDumpSink
