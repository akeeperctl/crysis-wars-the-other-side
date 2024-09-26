// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.
// Adapted to CE2 by AlienKeeper

#pragma once

#include "TheOtherSideMP\Game\Modules\GenericModule.h"

class CFactionMap;
class CTOSPersonalHostiles;
class CTOSFactionsModule : public CTOSGenericModule
{
public:
	//typedef Functor2 <uint8 /*factionID*/, IFactionMap::ReactionType /*reaction*/> ReactionChangedCallback;

	/// @param xmlFilePath - путь к файлу с фракциями, например: "scripts/ai/factions.xml"
	CTOSFactionsModule(IAISystem* pAISystem, IEntitySystem* pEntitySystem, const char* xmlFilePath);
	~CTOSFactionsModule();

	//ITOSGameModule
	virtual	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	virtual void GetMemoryStatistics(ICrySizer* s);
	virtual void InitCCommands(IConsole* pConsole);
	virtual void ReleaseCCommands();
	virtual void InitCVars(IConsole* pConsole);
	virtual void ReleaseCVars();
	virtual void Update(float frametime);
	virtual const char* GetName()
	{
		return "ModuleFactions";
	};
	virtual void Serialize(TSerialize ser);
	//~ITOSGameModule

	bool	   SetEntityFaction(EntityId entityId, int factionId);
	int		   GetEntityFaction(EntityId entityId) const;

	bool	   SetAIFaction(IAIObject* pObject, int factionId) const;
	int		   GetAIFaction(const IAIObject* pObject) const;

	bool	   SetAIFaction(IAIActor* pAIActor, int factionId) const;
	int		   GetAIFaction(const IAIActor* pAIActor) const;
	bool	   Reload();

	inline CTOSPersonalHostiles* CTOSFactionsModule::GetPersonalHostiles()
	{
		return m_pPersonalHostiles;
	}

	inline CFactionMap* GetFactionMap()
	{
		return m_pFactionMap;
	}

	//Console commands
	static void CmdReloadFactions(IConsoleCmdArgs* cmdArgs);

	//Console variables
	int tos_factions_default_reaction;

private:
	CTOSPersonalHostiles* m_pPersonalHostiles;
	CFactionMap* m_pFactionMap;
	IAISystem* m_pAISystem;
};