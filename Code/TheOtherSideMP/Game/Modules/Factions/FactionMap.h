// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#pragma once

#include "platform.h"
#include <unordered_map>
#include <StlUtils.h>
#include "IFactionMap.h"

class CFactionXmlDataSource : public IFactionDataSource
{
public:
	CFactionXmlDataSource(const char* szFileName)
		: m_fileName(szFileName)
	{}

	// IFactionDataSource
	virtual bool Load(IFactionMap& factionMap) override;
	// ~IFactionDataSource

private:
	string m_fileName;
};

class CFactionMap : public IFactionMap
{
	friend class CTOSFactionsModule;
	friend class CFactionXmlDataSource;
	friend class CFlowNode_SetFactionsReaction;

	enum { maxFactionCount = 32 };

	typedef std::set<int>																			  FactionIds;
	typedef std::map<std::pair<int, int>, IFactionMap::ReactionType>								  Reactions;
	//typedef std::unordered_map<uint8, string>                                                       FactionNamesById;
	//typedef std::unordered_map<string, uint8, stl::hash_stricmp<string>, stl::hash_stricmp<string>> FactionIdsByName;

public:
	typedef IFactionMap::ReactionType EReaction;

	/// @param xmlFilePath - путь к файлу с фракциями, например: "scripts/ai/factions.xml"
	CFactionMap(CTOSFactionsModule* pFactionsModule, const char* xmlFilePath);
	~CFactionMap();

	// IFactionMap
	virtual uint32      GetFactionCount() const override;
	virtual uint32      GetMaxFactionCount() const override;

	//virtual const char* GetFactionName(uint8 factionId) const override;
	//virtual uint8       GetFactionID(const char* szName) const override;

	//virtual bool        CreateFaction(uint8 factionID, uint32 reactionsCount, const uint8* pReactions) override;
	virtual bool		CreateFaction(int factionId, CFactionMap::ReactionType defaultReactionsType);
	virtual void		RemoveFaction(int factionId);

	virtual void        SetReaction(int factionOne, int factionTwo, IFactionMap::ReactionType reaction) override;
	virtual EReaction   GetReaction(const int factionOne, const int factionTwo) const override;
	virtual ReactionType GetDefaultReaction() const;

	virtual void        SetDataSource(IFactionDataSource* pDataSource, EDataSourceLoad bLoad) override;
	virtual void        RemoveDataSource(IFactionDataSource* pDataSource) override;

	virtual bool        Reload() override;

	//virtual void        RegisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback) override;
	//virtual void        UnregisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback) override;
	// ~IFactionMap

	void Clear();
	void Serialize(TSerialize ser);
protected:
		Reactions& GetReactions();
private:
	static bool GetReactionType(const char* szReactionName, EReaction* pReactionType);
	static const char* GetReactionName(EReaction reactionType);

	static CFactionXmlDataSource s_defaultXmlDataSource;

	IFactionDataSource*          m_pDataSource;
	//FactionNamesById             m_namesById;
	//FactionIdsByName             m_idsByName;
	FactionIds					 m_factionIds;
	//uint8                        m_reactions[maxFactionCount][maxFactionCount];
	Reactions m_reactions;
	CTOSFactionsModule*			m_pFactionsModule;

	//CFunctorsList<FactionReactionChangedCallback> m_factionReactionChangedCallback;
};
