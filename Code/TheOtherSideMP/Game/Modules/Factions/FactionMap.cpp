// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#include "StdAfx.h"
#include <StringUtils.h>
#include "IFactionMap.h"
#include "FactionMap.h"
#include "FactionsModule.h"
#include "Logger.h"
#include <TheOtherSideMP\Game\TOSGameEventRecorder.h>

CFactionXmlDataSource CFactionMap::s_defaultXmlDataSource("scripts/ai/factions.xml");
//const char* IFactionMap::s_logPrefix = "[FACTIONS] ";

void static FactionsLogAlways(const char* format, ...)
{
	string final = FACTIONS_LOG_PREFIX;
	final += format;

	va_list args;
	va_start(args, format);
	gEnv->pLog->LogV( ILog::eAlways,final,args );
	va_end(args);
}

void static FactionsError(const char* format, ...)
{
	string final = FACTIONS_LOG_PREFIX;
	final += format;
	format = final;

	va_list args;
	va_start(args, format);
	gEnv->pLog->LogV( ILog::eError,final,args );
	va_end(args);
}

void static FactionsWarning(const char* format, ...)
{
	string final = FACTIONS_LOG_PREFIX;
	final += format;
	format = final;

	va_list args;
	va_start(args, format);
	gEnv->pLog->LogV( ILog::eWarning,final,args );
	va_end(args);
}

CFactionMap::CFactionMap(CTOSFactionsModule* pFactionsModule, const char* xmlFilePath):
	m_pFactionsModule(pFactionsModule)
{
	string path = xmlFilePath;
	const EDataSourceLoad load = EDataSourceLoad::Now;

	CryStringUtils::toLowerInplace(path);

	if (path.empty())
		SetDataSource(&s_defaultXmlDataSource, load);
	else
		SetDataSource(new CFactionXmlDataSource(path.c_str()), load);
}

CFactionMap::~CFactionMap()
{
	SAFE_DELETE(m_pDataSource);
}

uint32 CFactionMap::GetFactionCount() const
{
	return m_factionIds.size();
}

uint32 CFactionMap::GetMaxFactionCount() const
{
	return maxFactionCount;
}

//const char* CFactionMap::GetFactionName(const uint8 factionId) const
//{
//	FactionNamesById::const_iterator it = m_namesById.find(factionId);
//	if (it != m_namesById.end())
//	{
//		return it->second.c_str();
//	}
//
//	return nullptr;
//}
//
//uint8 CFactionMap::GetFactionID(const char* szName) const
//{
//	FactionIdsByName::const_iterator it = m_idsByName.find(CONST_TEMP_STRING(szName));
//	if (it != m_idsByName.end())
//	{
//		return it->second;
//	}
//
//	return InvalidFactionID;
//}

void CFactionMap::Clear()
{
	m_factionIds.clear();
	//m_namesById.clear();
	//m_idsByName.clear();

	for (int i = 0; i < maxFactionCount; ++i)
	{
		for (int j = 0; j < maxFactionCount; ++j)
		{		
			m_reactions[{i,j}] = (i != j) ? GetDefaultReaction() : Friendly;
		}
	}

	//for (uint32 i = 0; i < maxFactionCount; ++i)
	//{
	//	for (uint32 j = 0; j < maxFactionCount; ++j)
	//	{
	//		m_reactions[i][j] = (i != j) ? Hostile : Friendly;
	//	}
	//}
}

bool CFactionMap::Reload()
{
	Clear();

	if (m_pDataSource)
	{
		if (!m_pDataSource->Load(*this))
		{
			FactionsError("Failed to load factions from data source!");
			return false;
		}
	}

	return true;
}

//void CFactionMap::RegisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback)
//{
//	m_factionReactionChangedCallback.Add(callback);
//}
//
//void CFactionMap::UnregisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback)
//{
//	m_factionReactionChangedCallback.Remove(callback);
//}

bool CFactionMap::CreateFaction(int factionId, CFactionMap::ReactionType defaultReactionType)
{
	std::pair<FactionIds::iterator, bool> createResult;

	auto found = stl::binary_find(m_factionIds.begin(), m_factionIds.end(), factionId);
	if (found == m_factionIds.end())
	{
		if (m_factionIds.size() < maxFactionCount)
		{
			createResult = m_factionIds.emplace(factionId);

			if (!createResult.second)
			{
				FactionsError("CreateFaction(...) failed. Reason: Failed to insert faction!");
				return false;
			}
		}
		else
		{
			FactionsError("CreateFaction(...) failed. Reason: Max faction count reached!");
			return false;
		}
	}

	// Ставим реакцию defaultReactionType фракции1 на все другие фракции
	for (auto it = m_reactions.begin(); it != m_reactions.end(); it++)
	{
		const int factionOne = it->first.first;

		if (factionOne == factionId) 
			it->second = defaultReactionType;
	}

	//if (reactionsCount && pReactions)
	//{
	//	const size_t bytesToCopy = std::min(reactionsCount, static_cast<uint32>(maxFactionCount));

	//	// Let the function fail in case of an overlapping copy.
	//	const size_t dstMin = reinterpret_cast<size_t>(m_reactions[factionId]);
	//	const size_t dstMax = dstMin + bytesToCopy;
	//	const size_t srcMin = reinterpret_cast<size_t>(pReactions);
	//	const size_t srcMax = srcMin + bytesToCopy;

	//	CRY_ASSERT(dstMin >= srcMax || dstMax <= srcMin);
	//	if (dstMin >= srcMax || dstMax <= srcMin)
	//	{
	//		//memcpy(m_reactions[factionId], pReactions, sizeof(uint8) * bytesToCopy);
	//		// m_reactions[factionId][factionId] = Friendly;

	//		return true;
	//	}
	//	else
	//	{
	//		FactionsError("[Factions] CreateFaction(...) failed. Reason: Overlapping copy detected!");

	//		if (createResult.second)
	//			m_factionIds.erase(factionId);
	//	}
	//}
	//else
	//{
	//	FactionsWarning("[Factions] CreateFaction(...) called with invalid reaction data. Parameters: 'reactionsCount' = '%d', 'pReactions' = '0x%p'", reactionsCount, pReactions);
	//}

	return true;
}

void CFactionMap::RemoveFaction(int factionID)
{
	if (gEnv->bEditor)
	{
		const int removeFactionId = factionID;
		const auto foundIter = stl::binary_find(m_factionIds.cbegin(), m_factionIds.cend(), removeFactionId);
		if (foundIter == m_factionIds.end())
		{
			FactionsWarning("RemoveFaction(...) faction %i already deleted or not created", removeFactionId);
			return;
		}

		m_factionIds.erase(removeFactionId);

		//if (foundIter != m_factionIds.end())
		//{
			//// создаем копию
			//uint8 oldReactions[maxFactionCount][maxFactionCount];
			//memcpy(oldReactions, m_reactions, sizeof(oldReactions));

			////сохраняем старые имена
			//FactionNamesById oldNamesById;
			//oldNamesById.swap(m_namesById);

			//// чистим idшники
			//m_idsByName.clear();

			//const size_t bytesFirstPart = sizeof(uint8) * (removeFactionId != 0 ? removeFactionId : maxFactionCount);
			//const size_t bytesSecondPart = sizeof(uint8) * (maxFactionCount - 1 - removeFactionId);

			//for (const FactionNamesById::value_type& kvPair : oldNamesById)
			//{
			//	if (kvPair.first != removeFactionId)
			//	{
			//		const uint8 newId = static_cast<uint8>(m_namesById.size());
			//		m_namesById.emplace(newId, kvPair.second.c_str());
			//		m_idsByName.emplace(kvPair.second.c_str(), newId);

			//		memcpy(m_reactions[newId], oldReactions[kvPair.first], bytesFirstPart);

			//		if (removeFactionId != 0)
			//		{
			//			memcpy(&m_reactions[newId][removeFactionId], &oldReactions[kvPair.first][removeFactionId + 1], bytesSecondPart);
			//		}
			//	}
			//}
		//}
	}
	else
	{
		FactionsWarning("RemoveFaction(...) is only allowed in editor mode.");
	}
}

void CFactionMap::SetReaction(int factionOne, int factionTwo, IFactionMap::ReactionType reaction)
{
	if ((factionOne < maxFactionCount) && (factionTwo < maxFactionCount))
	{
		ReactionType oldReaction = static_cast<ReactionType>(m_reactions[{factionOne, factionTwo}]);

		m_reactions[{factionOne, factionTwo}] = reaction;

		char buffer[256];
		sprintf(buffer, "Faction ['%i'] to Faction ['%i'] from '%s' to '%s'", int(factionOne), int(factionTwo), GetReactionName(oldReaction), GetReactionName(reaction));
		TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_FactionReactionChanged, buffer, true, false, &factionOne, int(factionTwo), int(reaction)));

		//m_reactions[factionOne][factionTwo] = reaction;
		//m_factionReactionChangedCallback.Call(factionOne, factionTwo, reaction);
	}
}

IFactionMap::ReactionType CFactionMap::GetReaction(const int factionOne, const int factionTwo) const
{
	if ((factionOne < maxFactionCount) && (factionTwo < maxFactionCount))
	{
		auto foundIter = m_reactions.find(std::make_pair(factionOne, factionTwo));
		IFactionMap::ReactionType reaction = foundIter->second;

		return static_cast<IFactionMap::ReactionType>(reaction);
	}

	//if (factionOne > InvalidFactionID && factionTwo > InvalidFactionID)
	//{
	//	return Neutral;
	//}

	//return GetDefaultReaction();
}

IFactionMap::ReactionType CFactionMap::GetDefaultReaction() const
{
	return static_cast<ReactionType>(m_pFactionsModule->tos_factions_default_reaction);
}

void CFactionMap::Serialize(TSerialize ser)
{
	//ser.BeginGroup("FactionMap");

	//// find highest faction id
	//int highestId = 0;

	//if (ser.IsWriting())
	//{
	//	FactionIds::iterator it = m_factionIds.begin();
	//	FactionIds::iterator end = m_factionIds.end();
	//	for (; it != end; ++it)
	//	{
	//		if (*it > highestId)
	//			highestId = *it;
	//	}
	//}

	//ser.Value("SerializedFactionCount", highestId);

	//stack_string nameFormatter;
	//for (size_t i = 0; i < highestId; ++i)
	//{
	//	for (size_t j = 0; j < highestId; ++j)
	//	{
	//		nameFormatter.Format("Reaction_%" PRISIZE_T "_to_%" PRISIZE_T, i, j);
	//		ser.Value(nameFormatter.c_str(), m_reactions[{i,j}]);
	//	}
	//}

	//ser.EndGroup();
}

CFactionMap::Reactions& CFactionMap::GetReactions()
{
	return m_reactions;
}

bool CFactionMap::GetReactionType(const char* szReactionName, EReaction* pReactionType)
{
	if (!stricmp(szReactionName, "Friendly"))
	{
		if (pReactionType)
		{
			*pReactionType = Friendly;
		}
	}
	else if (!stricmp(szReactionName, "Hostile"))
	{
		if (pReactionType)
		{
			*pReactionType = Hostile;
		}
	}
	else if (!stricmp(szReactionName, "Neutral"))
	{
		if (pReactionType)
		{
			*pReactionType = Neutral;
		}
	}
	else
	{
		return false;
	}

	return true;

}

const char* CFactionMap::GetReactionName(EReaction reactionType) 
{
	switch (reactionType) 
	{
		case Friendly:
			return "Friendly";
		case Hostile:
			return "Hostile";
		case Neutral:
			return "Neutral";
		default:
			return "<UNDEFINED>";
	}
}

void CFactionMap::SetDataSource(IFactionDataSource* pDataSource, EDataSourceLoad bLoad)
{
	Clear();
	m_pDataSource = pDataSource;
	if (m_pDataSource && bLoad == EDataSourceLoad::Now)
	{
		if (!m_pDataSource->Load(*this))
		{
			FactionsLogAlways("Failed to load factions from data source!");
		}
	}
}

void CFactionMap::RemoveDataSource(IFactionDataSource* pDataSource)
{
	if (m_pDataSource == pDataSource)
	{
		Clear();
		m_pDataSource = nullptr;
	}
}

bool CFactionXmlDataSource::Load(IFactionMap& factionMap)
{
	if (!gEnv->pCryPak->IsFileExist(m_fileName.c_str()))
	{
		return false;
	}

	XmlNodeRef rootNode = GetISystem()->GetXmlUtils()->LoadXmlFile(m_fileName.c_str());
	if (!rootNode)
	{
		FactionsError("Failed to open faction XML file '%s'.", m_fileName);
		return false;
	}

	FactionsLogAlways("Parsing Factions XML file '%s'", m_fileName);
	CryLogAlways("---------------------------");
	// uint8 reactions[CFactionMap::maxFactionCount][CFactionMap::maxFactionCount];

	const char* szRootName = rootNode->getTag();
	if (!stricmp(szRootName, "Factions"))
	{
		// Create factions with default reactions
		const int32 factionNodeCount = rootNode->getChildCount();
		for (int32 factionIdx = 0; factionIdx < factionNodeCount; ++factionIdx)
		{
			const XmlNodeRef factionNode = rootNode->getChild(factionIdx);

			if (!stricmp(factionNode->getTag(), "Faction"))
			{
				if (factionIdx >= CFactionMap::maxFactionCount)
				{
					FactionsError("Maximum number of allowed factions reached in file '%s' at line '%d'!", m_fileName.c_str(), factionNode->getLine());
					return false;
				}

				XmlString szFactionId;
				if (!factionNode->getAttr("id", szFactionId) || szFactionId.empty())
				{
					FactionsError("Missing or empty 'id' attribute for 'Faction' tag in file '%s' at line %d...", m_fileName.c_str(), factionNode->getLine());
					return false;
				}

				const int factionId = atoi(szFactionId); //factionMap.GetFactionID(szFactionId);
				//if (factionId != CFactionMap::InvalidFactionID)
				//{
				//	FactionsWarning("Duplicate faction '%s' in file '%s' at line %d...", szFactionId, m_fileName.c_str(), factionNode->getLine());
				//	//FactionsWarning("Invalid factionId of Faction '%s' in file '%s' at line %d...", szFactionId, m_fileName.c_str(), factionNode->getLine());
				//	return false;
				//}

				CFactionMap::ReactionType defaultReactionType = factionMap.GetDefaultReaction();

				XmlString szDefaultReaction;
				if (factionNode->getAttr("default", szDefaultReaction) && szDefaultReaction && szDefaultReaction[0])
				{
					if (!CFactionMap::GetReactionType(szDefaultReaction, &defaultReactionType))
					{
						FactionsError("Invalid default reaction '%s' in file '%s' at line '%d'...", szDefaultReaction, m_fileName.c_str(), factionNode->getLine());
						return false;
					}
				}

				//memset(reactions[factionIdx], defaultReactionType, sizeof(reactions[factionIdx]));
			
				const bool result = factionMap.CreateFaction(factionId, defaultReactionType);
				if (result)
				{
					FactionsLogAlways("['%s'] creating successfully!", szFactionId);
				}
			}
			else
			{
				FactionsError("Unexpected tag '%s' in file '%s' at line %d...", factionNode->getTag(), m_fileName.c_str(), factionNode->getLine());
				return false;
			}
		}

		// Update factions reactions
		for (int32 factionIdx = 0; factionIdx < factionNodeCount; ++factionIdx)
		{
			const XmlNodeRef factionNode = rootNode->getChild(factionIdx);

			XmlString szFactionId;
			factionNode->getAttr("id", szFactionId);
			const uint8 factionId = atoi(szFactionId);

			const int32 reactionNodeCount = factionNode->getChildCount();
			for (int32 reactionIdx = 0; reactionIdx < reactionNodeCount; ++reactionIdx)
			{
				XmlNodeRef reactionNode = factionNode->getChild(reactionIdx);
				if (!stricmp(reactionNode->getTag(), "Reaction"))
				{
					XmlString szReactionOnFaction;
					if (!reactionNode->getAttr("faction", szReactionOnFaction) || szReactionOnFaction.empty())
					{
						FactionsError("Missing or empty 'faction' attribute for 'Reaction' tag in file '%s' at line %d...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					const uint8 reactionOnfactionId = atoi(szReactionOnFaction);
					if (reactionOnfactionId <= CFactionMap::InvalidFactionID)
					{
						FactionsError("Attribute 'faction' for 'Reaction' tag in file '%s' at line %d defines an unknown faction...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					XmlString szReaction;
					if (!reactionNode->getAttr("reaction", szReaction) || szReaction.empty())
					{
						FactionsError("Missing or empty 'reaction' attribute for 'Reaction' tag in file '%s' at line %d...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					CFactionMap::ReactionType reactionType = factionMap.GetDefaultReaction();
					if (!CFactionMap::GetReactionType(szReaction, &reactionType))
					{
						FactionsWarning("Invalid reaction '%s' in file '%s' at line '%d'...", szReaction, m_fileName.c_str(), reactionNode->getLine());
						//return false;
					}

					//reactions[factionId][reactionOnfactionId] = static_cast<uint8>(reactionType);
					factionMap.SetReaction(factionId, reactionOnfactionId, reactionType); 
				}
				else
				{
					FactionsError("Unexpected tag '%s' in file '%s' at line %d...", reactionNode->getTag(), m_fileName.c_str(), reactionNode->getLine());
					return false;
				}
			}

			//factionMap.CreateFaction(szFactionId, CFactionMap::maxFactionCount, reactions[factionId]);
		}
	}
	else
	{
		FactionsError("Unexpected tag '%s' in file '%s' at line %d...", szRootName, m_fileName.c_str(), rootNode->getLine());
		return false;
	}

	CryLogAlways("---------------------------");
	FactionsLogAlways("All factions were created successfully!");
	return true;
}
