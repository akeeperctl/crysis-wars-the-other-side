// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "IFactionMap.h"
#include "FactionMap.h"

CFactionXmlDataSource CFactionMap::s_defaultXmlDataSource("Scripts/AI/factions.xml");

static const char outputPrefix[] = "AI: ";
static const unsigned outputPrefixLen = sizeof(outputPrefix) - 1;

void static AILogAlways(const char* format, ...)
{
	string final = "AI: ";
	final += format;

	va_list args;
	va_start(args, format);
	CryLogAlways(final.c_str(), args);
	va_end(args);
}

void static AIError(const char* format, ...)
{
	string final = "AI: ";
	final += format;

	va_list args;
	va_start(args, format);
	CryLogError(final.c_str(), args);
	va_end(args);
}

void static AIWarning(const char* format, ...)
{
	string final = "AI: ";
	final += format;

	va_list args;
	va_start(args, format);
	CryLogWarning(final.c_str(), args);
	va_end(args);
}

CFactionMap::CFactionMap()
{
	SetDataSource(&s_defaultXmlDataSource, EDataSourceLoad::Now);
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

	for (uint32 i = 0; i < maxFactionCount; ++i)
	{
		for (uint32 j = 0; j < maxFactionCount; ++j)
		{		
			m_reactions[{i,j}] = (i != j) ? Hostile : Friendly;
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

void CFactionMap::Reload()
{
	Clear();

	if (m_pDataSource)
	{
		if (!m_pDataSource->Load(*this))
		{
			AILogAlways("[FactionMap] Failed to load factions from data source!");
		}
	}
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

bool CFactionMap::CreateFaction(uint8 factionId, CFactionMap::ReactionType defaultReactionType)
{
	std::pair<FactionIds::iterator, bool> createResult;

	auto found = stl::binary_find(m_factionIds.begin(), m_factionIds.end(), factionId);
	if (found != m_factionIds.end())
	{
		if (m_factionIds.size() < maxFactionCount)
		{
			createResult = m_factionIds.emplace(factionId);

			if (!createResult.second)
			{
				AIError("[FactionMap] CreateFaction(...) failed. Reason: Failed to insert faction!");
				return false;
			}
		}
		else
		{
			AIError("[FactionMap] CreateFaction(...) failed. Reason: Max faction count reached!");
			return false;
		}
	}

	// Ставим реакцию defaultReactionType фракции1 на все другие фракции
	for (auto it = m_reactions.begin(); it != m_reactions.end(); it++)
	{
		const uint8 factionOne = it->first.first;

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
	//		AIError("[FactionMap] CreateFaction(...) failed. Reason: Overlapping copy detected!");

	//		if (createResult.second)
	//			m_factionIds.erase(factionId);
	//	}
	//}
	//else
	//{
	//	AIWarning("[FactionMap] CreateFaction(...) called with invalid reaction data. Parameters: 'reactionsCount' = '%d', 'pReactions' = '0x%p'", reactionsCount, pReactions);
	//}

	return false;
}

void CFactionMap::RemoveFaction(uint8 factionID)
{
	if (gEnv->bEditor)
	{
		const uint8 removeFactionId = factionID;
		const auto foundIter = stl::binary_find(m_factionIds.cbegin(), m_factionIds.cend(), removeFactionId);
		if (foundIter == m_factionIds.end())
		{
			AIWarning("[RemoveFaction] faction %i already deleted or not created", removeFactionId);
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
		AIWarning("[FactionMap] RemoveFaction(...) is only allowed in editor mode.");
	}
}

void CFactionMap::SetReaction(uint8 factionOne, uint8 factionTwo, IFactionMap::ReactionType reaction)
{
	if ((factionOne < maxFactionCount) && (factionTwo < maxFactionCount))
	{
		m_reactions[{factionOne, factionTwo}] = reaction;

		//m_reactions[factionOne][factionTwo] = reaction;
		//m_factionReactionChangedCallback.Call(factionOne, factionTwo, reaction);
	}
}

IFactionMap::ReactionType CFactionMap::GetReaction(const uint8 factionOne, const uint8 factionTwo) const
{
	if ((factionOne < maxFactionCount) && (factionTwo < maxFactionCount))
	{
		auto found = m_reactions.find(std::make_pair(factionOne, factionTwo));
		auto reaction = found->second;

		return static_cast<IFactionMap::ReactionType>(reaction);
	}

	if (factionOne != InvalidFactionID && factionTwo != InvalidFactionID)
	{
		return Neutral;
	}

	return Hostile;
}

void CFactionMap::Serialize(TSerialize ser)
{
	ser.BeginGroup("FactionMap");

	// find highest faction id
	uint32 highestId = 0;

	if (ser.IsWriting())
	{
		FactionIds::iterator it = m_factionIds.begin();
		FactionIds::iterator end = m_factionIds.end();
		for (; it != end; ++it)
		{
			if (*it > highestId)
				highestId = *it;
		}

		//FactionIdsByName::iterator it = m_idsByName.begin();
		//FactionIdsByName::iterator end = m_idsByName.end();

		//for (; it != end; ++it)
		//{
		//	if (it->second > highestId)
		//		highestId = it->second;
		//}
	}

	ser.Value("SerializedFactionCount", highestId);

	stack_string nameFormatter;
	for (size_t i = 0; i < highestId; ++i)
	{
		for (size_t j = 0; j < highestId; ++j)
		{
			nameFormatter.Format("Reaction_%" PRISIZE_T "_to_%" PRISIZE_T, i, j);
			ser.Value(nameFormatter.c_str(), m_reactions[{i,j}]);
		}
	}

	ser.EndGroup();
}

bool CFactionMap::GetReactionType(const char* szReactionName, ReactionType* pReactionType)
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

void CFactionMap::SetDataSource(IFactionDataSource* pDataSource, EDataSourceLoad bLoad)
{
	Clear();
	m_pDataSource = pDataSource;
	if (m_pDataSource && bLoad == EDataSourceLoad::Now)
	{
		if (!m_pDataSource->Load(*this))
		{
			CryLogAlways("[FactionMap] Failed to load factions from data source!");
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
		CryLogWarning("Failed to open faction XML file '%s'.", m_fileName.c_str());
		return false;
	}

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
					AIWarning("Maximum number of allowed factions reached in file '%s' at line '%d'!", m_fileName.c_str(), factionNode->getLine());
					return false;
				}

				XmlString szFactionId = nullptr;
				if (!factionNode->getAttr("id", szFactionId) || szFactionId == nullptr || szFactionId[0] == '\0')
				{
					AIWarning("Missing or empty 'id' attribute for 'Faction' tag in file '%s' at line %d...", m_fileName.c_str(), factionNode->getLine());
					return false;
				}

				const uint8 factionId = atoi(szFactionId); //factionMap.GetFactionID(szFactionId);
				if (factionId != CFactionMap::InvalidFactionID)
				{
					AIWarning("Duplicate faction '%s' in file '%s' at line %d...", szFactionId, m_fileName.c_str(), factionNode->getLine());
					//AIWarning("Invalid factionId of Faction '%s' in file '%s' at line %d...", szFactionId, m_fileName.c_str(), factionNode->getLine());
					return false;
				}

				CFactionMap::ReactionType defaultReactionType = CFactionMap::Hostile;

				XmlString szDefaultReaction = nullptr;
				if (factionNode->getAttr("default", szDefaultReaction) && szDefaultReaction && szDefaultReaction[0])
				{
					if (!CFactionMap::GetReactionType(szDefaultReaction, &defaultReactionType))
					{
						AIWarning("Invalid default reaction '%s' in file '%s' at line '%d'...", szDefaultReaction, m_fileName.c_str(), factionNode->getLine());
						return false;
					}
				}

				//memset(reactions[factionIdx], defaultReactionType, sizeof(reactions[factionIdx]));
			
				factionMap.CreateFaction(factionId, defaultReactionType);
			}
			else
			{
				AIWarning("Unexpected tag '%s' in file '%s' at line %d...", factionNode->getTag(), m_fileName.c_str(), factionNode->getLine());
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
					XmlString szReactionOnFaction = nullptr;
					if (!reactionNode->getAttr("faction", szReactionOnFaction) || szReactionOnFaction == nullptr || szReactionOnFaction[0] == '\0')
					{
						AIWarning("Missing or empty 'faction' attribute for 'Reaction' tag in file '%s' at line %d...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					const uint8 reactionOnfactionId = atoi(szReactionOnFaction);
					if (reactionOnfactionId == CFactionMap::InvalidFactionID)
					{
						AIWarning("Attribute 'faction' for 'Reaction' tag in file '%s' at line %d defines an unknown faction...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					XmlString szReaction = nullptr;
					if (!reactionNode->getAttr("reaction", szReaction) || szReaction == nullptr || szReaction[0] == '\0')
					{
						AIWarning("Missing or empty 'reaction' attribute for 'Reaction' tag in file '%s' at line %d...", m_fileName.c_str(), reactionNode->getLine());
						return false;
					}

					CFactionMap::ReactionType reactionType = CFactionMap::Neutral;
					if (!CFactionMap::GetReactionType(szReaction, &reactionType))
					{
						AIWarning("Invalid reaction '%s' in file '%s' at line '%d'...", szReaction, m_fileName.c_str(), reactionNode->getLine());
						//return false;
					}

					//reactions[factionId][reactionOnfactionId] = static_cast<uint8>(reactionType);
					factionMap.SetReaction(factionId, reactionOnfactionId, reactionType); 
				}
				else
				{
					AIWarning("Unexpected tag '%s' in file '%s' at line %d...", reactionNode->getTag(), m_fileName.c_str(), reactionNode->getLine());
					return false;
				}
			}

			//factionMap.CreateFaction(szFactionId, CFactionMap::maxFactionCount, reactions[factionId]);
		}
	}
	else
	{
		AIWarning("Unexpected tag '%s' in file '%s' at line %d...", szRootName, m_fileName.c_str(), rootNode->getLine());
		return false;
	}

	return true;
}
