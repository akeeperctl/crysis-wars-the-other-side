// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#include "StdAfx.h"
#include "FactionsModule.h"
#include "Logger.h"

#include "TheOtherSideMP\Helpers\TOS_AI.h"
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include "TheOtherSideMP\Game\TOSGameEventRecorder.h"

#include "CryLibrary.h"
#include <windows.h>


CTOSFactionsModule::CTOSFactionsModule(IAISystem* pAISystem, const char* xmlFilePath)
{
	assert(pAISystem);

	//m_factionMap.RegisterFactionReactionChangedCallback(functor(*this, &CTOSFactionsModule::OnFactionReactionChanged));
	m_pAISystem = pAISystem;
	m_pFactionMap = new CFactionMap(xmlFilePath);
}

CTOSFactionsModule::~CTOSFactionsModule()
{
	//m_factionMap.UnregisterFactionReactionChangedCallback(functor(*this, &CTOSFactionsModule::OnFactionReactionChanged));
	SAFE_DELETE(m_pFactionMap);
}

void CTOSFactionsModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	switch (event.event)
	{
		case eEGE_FactionReactionChanged:
		{
			
			break;
		}
		default:
			break;
	}
}

void CTOSFactionsModule::GetMemoryStatistics(ICrySizer* s)
{

}

void CTOSFactionsModule::InitCCommands(IConsole* pConsole)
{
	pConsole->AddCommand("tos_factions_reload", CmdReloadFactions, 0, "Easy way to reload the factions XML data");
}

void CTOSFactionsModule::ReleaseCCommands()
{
	gEnv->pConsole->RemoveCommand("tos_factions_reload");
}

void CTOSFactionsModule::Serialize(TSerialize ser)
{
	m_pFactionMap->Serialize(ser);
}

//void CTOSFactionsModule::OnFactionReactionChanged(const uint8 factionOne, const uint8 factionTwo, const IFactionMap::ReactionType reactionType)
//{
//	const EntitiesWithCallbackSet& entitiesInFactionSet = m_entitiesInFactionsMap[factionOne];
//	for (const auto& entityWithCallback : entitiesInFactionSet)
//	{
//		if (entityWithCallback.second)
//		{
//			entityWithCallback.second(factionTwo, reactionType);
//		}
//	}
//}

//void CTOSFactionsModule::SetEntityFaction(EntityId entityId, const SFactionID& newFactionId, const ReactionChangedCallback& reactionChangedCallback)
//{
	//auto findIt = m_factionForEntitiesMap.find(entityId);
	//if (findIt != m_factionForEntitiesMap.end())
	//{
	//	uint8 oldFactionIdRaw = findIt->second;
	//	if (oldFactionIdRaw != newFactionId.id)
	//	{
	//		EntitiesWithCallbackSet& entitiesInFactionSet = m_entitiesInFactionsMap[oldFactionIdRaw];

	//		stl::find_and_erase_if(entitiesInFactionSet, [=](std::pair<EntityId, ReactionChangedCallback> entry)
	//		{
	//			return entry.first == entityId;
	//		});

	//		if (newFactionId.IsValid())
	//		{
	//			findIt->second = newFactionId.id;
	//			m_entitiesInFactionsMap[newFactionId.id].emplace(std::make_pair(entityId, reactionChangedCallback));
	//		}
	//		else
	//		{
	//			m_factionForEntitiesMap.erase(findIt);
	//		}
	//	}
	//}
	//else if(newFactionId.IsValid())
	//{
	//	m_factionForEntitiesMap[entityId] = newFactionId.id;
	//	m_entitiesInFactionsMap[newFactionId.id].emplace(std::make_pair(entityId, reactionChangedCallback));
	//}
//}

bool CTOSFactionsModule::SetEntityFaction(EntityId entityId, uint8 factionId)
{
	IEntity* pEntity = TOS_GET_ENTITY(entityId);
	if (!pEntity)
		return false;

	IAIObject* pAI = pEntity->GetAI();
	if (!pAI)
		return false;

	const uint8 oldFaction = GetEntityFaction(entityId);
	const bool result = TOS_AI::SetSpecies(pAI, factionId);
	if (result && oldFaction != INVALID_SPECIES_ID && oldFaction != factionId)
	{
		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_EntityFactionChanged, "", true, false, nullptr, 0, factionId));
	}

	return result;
}

uint8 CTOSFactionsModule::GetEntityFaction(EntityId entityId) const
{
	IEntity* pEntity = TOS_GET_ENTITY(entityId);
	if (!pEntity)
		return false;

	IAIObject* pAI = pEntity->GetAI();
	if (!pAI)
		return false;

	return TOS_AI::GetSpecies(pAI, false);
}

bool CTOSFactionsModule::Reload()
{
	return m_pFactionMap->Reload();
}

void CTOSFactionsModule::CmdReloadFactions(IConsoleCmdArgs* cmdArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("%s Factions reloading fail. Reason: Only on server.", FACTIONS_LOG_PREFIX);
	}

	if (g_pTOSGame->GetFactionsModule()->Reload())
	{
		CryLogAlways("%s Factions reloading success!", FACTIONS_LOG_PREFIX);
	}
}