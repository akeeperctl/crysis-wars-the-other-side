#include "StdAfx.h"

#include "Modules/ITOSGameModule.h"

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

void CTOSGame::OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event)
{
	//This called from IGameplayRecorder and redirected to OnExtraGameplayEvent

	auto event1 = STOSGameEvent(event);
	event1.vanilla_recorder = true;

	this->OnExtraGameplayEvent(pEntity, event1);
}

void CTOSGameEventRecorder::RecordEvent(EntityId id, const STOSGameEvent& event)
{
	g_pTOSGame->OnExtraGameplayEvent(gEnv->pEntitySystem->GetEntity(id), event);
}

void CTOSGame::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	//This code Handle gameplay events from ALL game and The Other Side

	switch(event.event)
	{
	case eGE_Connected:
	{
		//if (pEntity && event.console_log)
		if (pEntity && gEnv->bServer)
		{
			auto pGO = g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId());
			assert(pGO);

			//Net channel is null on client
			auto pNetCh = pGO->GetNetChannel();

			CryLogAlways("[C++][SERVER][FUNC CALL][CTOSGame::OnExtraGameplayEvent] Event: eGE_Connected, Name: (CH:%s|GO:%s)",
				pNetCh ? pNetCh->GetName() : "NULL", pGO->GetEntity()->GetName());
		}
		break;
	}
	}

	if (event.console_log && !event.vanilla_recorder)
	{
		const char* entName = pEntity ? pEntity->GetName() : "";
		const char* eventName = m_pEventRecorder->GetStringFromEnum(event.event);
		const char* eventDesc = event.description;
		const char* envName = "";

		auto pGO = pEntity ? g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId()) : nullptr;

		if (gEnv->bServer)
		{
			envName = "[SERVER]";
		}
		else if (pGO)
		{
			auto pNetCh = pGO->GetNetChannel();
			if (pNetCh && pNetCh->IsLocal())
			{
				envName = "[LOCAL]";
			}

		}
		else if (gEnv->bClient)
		{
			envName = "[CLIENT]";
		}

		CryLogAlways("[C++]%s[FUNC CALL][CTOSGame::OnExtraGameplayEvent] Event: %s, Entity: %s, Desc: %s",
			envName, eventName, entName, eventDesc);
	}

	for (auto pModule : m_modules)
	{
		if (pModule)
			pModule->OnExtraGameplayEvent(pEntity, event);
	}

	for (auto pModule : m_flowgraphModules)
	{
		if (pModule)
			pModule->OnExtraGameplayEvent(pEntity, event);
	}
}