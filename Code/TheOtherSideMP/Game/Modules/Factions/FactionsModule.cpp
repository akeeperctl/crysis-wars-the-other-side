// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#include "StdAfx.h"
#include <windows.h>
#include "CryLibrary.h"

#include "FactionMap.h"
#include "FactionsModule.h"
#include "PersonalHostiles.h"
#include "Logger.h"

#include "TheOtherSideMP\Helpers\TOS_AI.h"
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include "TheOtherSideMP\Game\TOSGameEventRecorder.h"

CTOSFactionsModule::CTOSFactionsModule(IAISystem* pAISystem, IEntitySystem* pEntitySystem, const char* xmlFilePath)
	: tos_factions_default_reaction(int(IFactionMap::Hostile))
{
	assert(pAISystem);

	//m_factionMap.RegisterFactionReactionChangedCallback(functor(*this, &CTOSFactionsModule::OnFactionReactionChanged));
	m_pAISystem = pAISystem;
	m_pFactionMap = new CFactionMap(this, xmlFilePath);
	m_pPersonalHostiles = new CTOSPersonalHostiles(pEntitySystem);
}

CTOSFactionsModule::~CTOSFactionsModule()
{
	//m_factionMap.UnregisterFactionReactionChangedCallback(functor(*this, &CTOSFactionsModule::OnFactionReactionChanged));
	SAFE_DELETE(m_pFactionMap);
	SAFE_DELETE(m_pPersonalHostiles);
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

void CTOSFactionsModule::InitCVars(IConsole* pConsole)
{
	pConsole->Register("tos_factions_default_reaction", &tos_factions_default_reaction, IFactionMap::Hostile, VF_CHEAT | VF_REQUIRE_LEVEL_RELOAD,
					   "Default reaction to faction relations. Require factions reloading\n"
					   "Usage: tos_factions_default_reaction [0/1/2]\n"
					   "Default is 0 (Hostile)\n"
					   "0 - hostile\n"
					   "1 - neutral\n"
					   "2 - friendly\n");
}

void CTOSFactionsModule::ReleaseCVars()
{
	gEnv->pConsole->UnregisterVariable("tos_factions_default_reaction", true);
}

void CTOSFactionsModule::Update(float frametime)
{
	auto pPD = g_pGame->GetIGameFramework()->GetIPersistantDebug();
	pPD->Begin("CTOSFactionsModule", true);

	//for (auto it1 = m_pFactionMap->m_factionIds.begin(); it1 != m_pFactionMap->m_factionIds.end(); it1++)
	//{
	//	for (auto it2 = m_pFactionMap->m_factionIds.begin(); it2 != m_pFactionMap->m_factionIds.end(); it2++)
	//	{
	//		int index1 = std::distance( m_pFactionMap->m_factionIds.begin(), it1);
	//		int index2 = std::distance( m_pFactionMap->m_factionIds.begin(), it2);
	//		pPD->AddText(100, 150 + 20 * (index1) + 100 * (index2), 1.3f, ColorF(1, 1, 1, 1), 15.0f, "Faction '%i' to Faction '%i' is '%s'",
	//					 *(it2),  *(it1), CFactionMap::GetReactionName(m_pFactionMap->GetReaction(*(it2),  *(it1))));
	//	}

	//}

	//auto it = m_pFactionMap->GetReactions().begin();
	//auto end = m_pFactionMap->GetReactions().end();
	//for (; it != end; it++)
	//{
	//	auto iter1 = stl::binary_find(m_pFactionMap->m_factionIds.begin(), m_pFactionMap->m_factionIds.end(), it->first.first);
	//	auto iter2 = stl::binary_find(m_pFactionMap->m_factionIds.begin(), m_pFactionMap->m_factionIds.end(), it->first.second);
	//	if (iter1 != m_pFactionMap->m_factionIds.end() && iter2 != m_pFactionMap->m_factionIds.end())
	//	{
	//		int index = std::distance(m_pFactionMap->GetReactions().begin(), it);
	//		pPD->AddText(300, 300 + 20 * index, 1.3f, ColorF(1, 1, 1, 1), 15.0f, "Faction '%i' to Faction '%i' is '%s'",
	//					 it->first.first, it->first.second, CFactionMap::GetReactionName(it->second));

	//	}
	//}
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

bool CTOSFactionsModule::SetEntityFaction(EntityId entityId, int factionId)
{
	IEntity* pEntity = TOS_GET_ENTITY(entityId);
	if (!pEntity)
		return false;

	IAIObject* pAI = pEntity->GetAI();
	if (!pAI)
		return false;

	const int oldFaction = GetEntityFaction(entityId);
	const bool result = TOS_AI::SetSpecies(pAI, factionId);
	if (result && oldFaction != INVALID_SPECIES_ID && oldFaction != factionId)
	{
		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_EntityFactionChanged, "", true, false, nullptr, 0, factionId));
	}

	return result;
}

int CTOSFactionsModule::GetEntityFaction(EntityId entityId) const
{
	IEntity* pEntity = TOS_GET_ENTITY(entityId);
	if (!pEntity)
		return -1;

	IAIObject* pAI = pEntity->GetAI();
	if (pAI)
		return TOS_AI::GetSpecies(pAI, false);
	else
		return TOS_AI::GetSpecies(pAI, true);

}

bool CTOSFactionsModule::SetAIFaction(IAIObject* pObject, int factionId) const
{
	if (!pObject)
		return false;

	const int oldFaction = GetAIFaction(pObject);
	const bool result = TOS_AI::SetSpecies(pObject, factionId);
	if (result && oldFaction != INVALID_SPECIES_ID && oldFaction != factionId)
	{
		TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_EntityFactionChanged, "", true, false, nullptr, 0, factionId));
	}

	return result;

}

int CTOSFactionsModule::GetAIFaction(const IAIObject* pObject) const
{
	return TOS_AI::GetSpecies(pObject, false);
}

bool CTOSFactionsModule::SetAIFaction(IAIActor* pObject, int factionId) const
{
	if (!pObject)
		return false;

	const int oldFaction = GetAIFaction(pObject);
	const bool result = TOS_AI::SetSpecies(pObject, factionId);
	if (result && oldFaction != INVALID_SPECIES_ID && oldFaction != factionId)
	{
		TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_EntityFactionChanged, "", true, false, nullptr, 0, factionId));
	}

	return result;

}

int CTOSFactionsModule::GetAIFaction(const IAIActor* pObject) const
{
	return TOS_AI::GetSpecies(pObject);
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