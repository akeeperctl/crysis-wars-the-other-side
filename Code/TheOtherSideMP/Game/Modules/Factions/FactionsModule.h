// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#pragma once

#include "FactionMap.h"
#include "TheOtherSideMP\Game\Modules\GenericModule.h"

//struct SFactionID
//{
//	SFactionID(const uint8 id = IFactionMap::InvalidFactionID)
//		: id(id)
//	{
//	}
//
//	bool IsValid() const
//	{
//		return id != IFactionMap::InvalidFactionID;
//	}
//
//	inline bool operator==(const SFactionID& other) const
//	{
//		if (id == IFactionMap::InvalidFactionID)
//			return false;
//
//		return id == other.id;
//	}
//
//	uint8 id;
//	string name;
//};

//struct SFactionFlagsMask
//{
//	SFactionFlagsMask() : mask(0)
//	{
//	}
//	SFactionFlagsMask(uint32 mask) : mask(mask)
//	{
//	}
//
//	inline bool operator==(const SFactionFlagsMask& other) const
//	{
//		return mask == other.mask;
//	}
//
//	uint32 mask;
//};

class CTOSFactionsModule : public CTOSGenericModule
{
public:
	typedef Functor2 <uint8 /*factionID*/, IFactionMap::ReactionType /*reaction*/> ReactionChangedCallback;

	/// @param xmlFilePath - путь к файлу с фракциями, например: "scripts/ai/factions.xml"
	CTOSFactionsModule(IAISystem* pAISystem, const char* xmlFilePath);
	~CTOSFactionsModule();

	//ITOSGameModule
	virtual	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	virtual void GetMemoryStatistics(ICrySizer* s);
	virtual void InitCCommands(IConsole* pConsole);
	virtual void ReleaseCCommands();

	virtual const char* GetName()
	{
		return "ModuleFactions";
	};
	virtual void Serialize(TSerialize ser);
	//~ITOSGameModule

	CFactionMap* GetFactionMap()
	{
		return m_pFactionMap;
	}

	bool	   SetEntityFaction(EntityId entityId, uint8 factionId);
	uint8	   GetEntityFaction(EntityId entityId) const;
	bool	   Reload();


private:
	//Console commands
	static void CmdReloadFactions(IConsoleCmdArgs* cmdArgs);

	CFactionMap* m_pFactionMap;
	IAISystem* m_pAISystem;
};