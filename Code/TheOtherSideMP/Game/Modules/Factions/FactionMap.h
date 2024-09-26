// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once
#include <unordered_map>
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
	friend class CFactionXmlDataSource;

	enum { maxFactionCount = 32 };

	typedef std::unordered_map<uint8, string>                                                       FactionNamesById;
	typedef std::map<string, uint8>														          FactionIdsByName;
	//typedef std::unordered_map<const char*, uint8, std::hash<const char*>, std::equal_to<const char*>>		  FactionIdsByName;
	//typedef std::unordered_map<string, uint8, std::hash<std::string>, std::equal_to<std::string>>   FactionIdsByName;
	//typedef std::unordered_map<string, uint8, stl::hash_stricmp<string>, stl::hash_stricmp<string>> FactionIdsByName;

public:
	typedef IFactionMap::ReactionType EReaction;

	CFactionMap(CTOSFactionsModule* pFactionsModule, const char* xmlFilePath);
	~CFactionMap();

	// IFactionMap
	virtual uint32      GetFactionCount() const override;
	virtual uint32      GetMaxFactionCount() const override;

	virtual const char* GetFactionName(uint8 factionId) const override;
	virtual uint8         GetFactionID(const char* szName) const override;

	virtual uint8         CreateOrUpdateFaction(const char* szName, uint8 reactionsCount, const uint8* pReactions) override;
	virtual void        RemoveFaction(const char* szName) override;

	virtual void        SetReaction(uint8 factionOne, uint8 factionTwo, IFactionMap::ReactionType reaction) override;
	virtual EReaction   GetReaction(uint8 factionOne, uint8 factionTwo) const override;


	virtual void        SetDataSource(IFactionDataSource* pDataSource, EDataSourceLoad bLoad) override;
	virtual void        RemoveDataSource(IFactionDataSource* pDataSource) override;

	virtual bool        Reload() override;
	virtual void DebugPrintReactions() const override;
	// ~IFactionMap

	void Clear();
	void Serialize(TSerialize ser);
	IFactionMap::ReactionType GetDefaultReaction() const;

private:
	static bool GetReactionType(const char* szReactionName, EReaction* pReactionType);
	const char* GetReactionName(EReaction reactionType) const;


	static CFactionXmlDataSource s_defaultXmlDataSource;

	CTOSFactionsModule*			 m_pFactionsModule;
	IFactionDataSource*          m_pDataSource;
	FactionNamesById             m_namesById;
	FactionIdsByName             m_idsByName;
	uint8                          m_reactions[maxFactionCount][maxFactionCount];
	CFunctorsList<FactionReactionChangedCallback> m_factionReactionChangedCallback;
};
