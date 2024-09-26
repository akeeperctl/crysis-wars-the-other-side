#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "TheOtherSideMP\Game\Modules\Factions\FactionsModule.h"
#include "TheOtherSideMP\Game\Modules\Factions\FactionMap.h"
#include <TheOtherSideMP\Game\Modules\Factions\Logger.h>
#include <TheOtherSideMP\AI\AICommon.h>

////////////////////////////////////////////////
class CFlowNode_SetFactionsReaction : public CFlowBaseNode
{
	//CFactionMap::Reactions m_reactions;
	CFactionMap* m_pFactionMap;

public:
	explicit CFlowNode_SetFactionsReaction(SActivationInfo* pActInfo)
	{
		m_pFactionMap = g_pTOSGame->GetFactionsModule()->GetFactionMap();
		assert(m_pFactionMap);
	}

	~CFlowNode_SetFactionsReaction()
	{
	};

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SetFactionsReaction(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Sync = 0,
		EIP_FactionOne,
		EIP_FactionTwo,
		EIP_Reaction,
	};

	enum EOutputPorts
	{
		EOP_Done = 0,
	};

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Sync", _HELP("")),
			InputPortConfig<int>("FactionOne", 0, _HELP("Must be >= 0 and < 255"), "FactionOne"),
			InputPortConfig<int>("FactionTwo", 0, _HELP("Must be >= 0 and < 255"), "FactionTwo"),
			InputPortConfig<int>("Reaction", int(m_pFactionMap->GetDefaultReaction()), _HELP(""), "Reaction", _UICONFIG("enum_int:Hostile=0,Neutral=1,Friendly=2")),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void("Done", _HELP("")),
			{nullptr}
		};
		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("ONLY ON SERVER. Sets up a reaction between two factions. The reaction changes mutually.");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!gEnv->bServer)
		{
			CryLogError("%s Unable to change faction reaction. Reason: not on server", FACTIONS_LOG_PREFIX);
			return;
		}

		switch (event)
		{
			case eFE_Initialize:
			{
			}
			break;
			case eFE_SetEntityId:
			{

			}
			break;
			case eFE_Activate:
			{

				if (IsPortActive(pActInfo, EIP_Sync))
				{
					int factionOne = GetPortInt(pActInfo, EIP_FactionOne);
					int factionTwo = GetPortInt(pActInfo, EIP_FactionTwo);
					if (factionOne < 0 || factionTwo < 0)
					{
						CryLogError("%s Unable to change faction reaction. Reason: One of the faction IDs < 0", FACTIONS_LOG_PREFIX);
						return;
					}
					else if (factionOne == INVALID_SPECIES_ID || factionTwo == INVALID_SPECIES_ID)
					{
						CryLogError("%s Unable to change faction reaction. Reason: One of the faction IDs has an invalid ID.", FACTIONS_LOG_PREFIX);
						return;
					}

					IFactionMap::ReactionType reaction = static_cast<IFactionMap::ReactionType>(GetPortInt(pActInfo, EIP_Reaction));
					m_pFactionMap->SetReaction(factionOne, factionTwo, reaction);
					ActivateOutput(pActInfo, EOP_Done, 1);
				}
			}
			break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		//ser.BeginGroup("CFlowNode_SetFactionsReaction");

		//// find highest faction id
		//uint8 highestId = m_pFactionMap->GetMaxFactionCount();

		//ser.Value("SerializedFactionCount", highestId);

		//stack_string nameFormatter;
		//for (int i = 0; i < highestId; ++i)
		//{
		//	for (int j = 0; j < highestId; ++j)
		//	{
		//		nameFormatter.Format("Reaction_%" PRISIZE_T "_to_%" PRISIZE_T, i, j);
		//		ser.Value(nameFormatter.c_str(), m_pFactionMap->GetReactions()[{i, j}]);
		//	}
		//}

		//ser.EndGroup();
	}
};

////////////////////////////////////////////////
class CFlowNode_GetFactionsReaction : public CFlowBaseNode
{
	CFactionMap* m_pFactionMap;

public:
	explicit CFlowNode_GetFactionsReaction(SActivationInfo* pActInfo)
	{
		m_pFactionMap = g_pTOSGame->GetFactionsModule()->GetFactionMap();
		assert(m_pFactionMap);
	}

	~CFlowNode_GetFactionsReaction()
	{
	};

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetFactionsReaction(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Sync = 0,
		EIP_FactionOne,
		EIP_FactionTwo,
	};

	enum EOutputPorts
	{
		EOP_Reaction = 0,
	};

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Sync", _HELP("")),
			InputPortConfig<int>("FactionOne", 0, _HELP("Must be >= 0 and < 255"), "FactionOne"),
			InputPortConfig<int>("FactionTwo", 0, _HELP("Must be >= 0 and < 255"), "FactionTwo"),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig<int>("Reaction", _HELP("Hostile=0,Neutral=1,Friendly=2"), "Reaction"),
			{nullptr}
		};
		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("ONLY ON SERVER. Sets up a reaction between two factions. The reaction changes mutually.");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Initialize:
			{
			}
			break;
			case eFE_SetEntityId:
			{

			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Sync))
				{
					int factionOne = GetPortInt(pActInfo, EIP_FactionOne);
					int factionTwo = GetPortInt(pActInfo, EIP_FactionTwo);
					if (factionOne < 0 || factionTwo < 0)
					{
						CryLogError("%s Unable to get faction reaction. Reason: One of the faction IDs < 0", FACTIONS_LOG_PREFIX);
						return;
					}
					else if (factionOne == INVALID_SPECIES_ID || factionTwo == INVALID_SPECIES_ID)
					{
						CryLogError("%s Unable to get faction reaction. Reason: One of the faction IDs has an invalid ID.", FACTIONS_LOG_PREFIX);
						return;
					}

					int reaction = (int)m_pFactionMap->GetReaction(factionOne, factionTwo);
					ActivateOutput(pActInfo, EOP_Reaction, reaction);
				}
			}
			break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}
};

////////////////////////////////////////////////
class CFlowNode_SetEntityFaction : public CFlowBaseNode
{
	//CFactionMap::Reactions m_reactions;
	CFactionMap* m_pFactionMap;
	IEntity* m_pEntity;

public:
	explicit CFlowNode_SetEntityFaction(SActivationInfo* pActInfo) :
		m_pEntity(pActInfo->pEntity)
	{
		m_pFactionMap = g_pTOSGame->GetFactionsModule()->GetFactionMap();
		assert(m_pFactionMap);
	}

	~CFlowNode_SetEntityFaction()
	{
	};

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SetEntityFaction(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Sync = 0,
		EIP_Faction,
	};

	enum EOutputPorts
	{
		EOP_Success = 0,
		EOP_Faction,
	};

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Sync", _HELP("")),
			InputPortConfig<int>("Faction", 0, _HELP("Must be >= 0 and < 255"), "Faction"),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig<bool>("Success", _HELP("")),
			OutputPortConfig<int>("Faction", _HELP("")),
			{nullptr}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("ONLY ON SERVER. Sets up a reaction between two factions. The reaction changes mutually.");
		config.SetCategory(EFLN_APPROVED);
	}

	void SetFaction(SActivationInfo* pActInfo)
	{
		auto pEntity = pActInfo->pEntity;
		if (!pEntity)
			return;

		int desiredFaction = GetPortInt(pActInfo, EIP_Faction);
		if (desiredFaction < 0)
		{
			CryLogError("%s Unable to change '%s's faction. Reason: The faction ID < 0", FACTIONS_LOG_PREFIX, pEntity->GetName());
			return;
		}
		else if (desiredFaction == INVALID_SPECIES_ID)
		{
			CryLogError("%s Unable to change '%s's faction. Reason: The faction ID has an invalid ID.", FACTIONS_LOG_PREFIX, pEntity->GetName());
			return;
		}

		const bool result = g_pTOSGame->GetFactionsModule()->SetEntityFaction(pEntity->GetId(), desiredFaction);
		ActivateOutput(pActInfo, EOP_Success, result);
		ActivateOutput(pActInfo, EOP_Faction, desiredFaction);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!gEnv->bServer)
		{
			CryLogError("%s Unable to change faction reaction. Reason: not on server", FACTIONS_LOG_PREFIX);
			return;
		}

		switch (event)
		{
			case eFE_Initialize:
			{
			}
			break;
			case eFE_SetEntityId:
			{
				if (m_pEntity != pActInfo->pEntity)
				{
					SetFaction(pActInfo);
				}
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Sync))
				{
					SetFaction(pActInfo);
				}
			}
			break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{

	}
};

////////////////////////////////////////////////
class CFlowNode_GetEntityFaction : public CFlowBaseNode
{
public:
	explicit CFlowNode_GetEntityFaction(SActivationInfo* pActInfo)
	{
	}

	~CFlowNode_GetEntityFaction()
	{
	};

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetEntityFaction(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Sync = 0,
	};

	enum EOutputPorts
	{
		EOP_Faction,
	};

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Sync", _HELP("")),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig<int>("Faction", _HELP("")),
			{nullptr}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get entity faction id.");
		config.SetCategory(EFLN_APPROVED);
	}

	void GetFaction(SActivationInfo* pActInfo)
	{
		auto pEntity = pActInfo->pEntity;
		if (!pEntity)
			return;

		const int faction = g_pTOSGame->GetFactionsModule()->GetEntityFaction(pEntity->GetId());
		ActivateOutput(pActInfo, EOP_Faction, faction);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Initialize:
			{
			}
			break;
			case eFE_SetEntityId:
			{
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Sync))
				{
					GetFaction(pActInfo);
				}
			}
			break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{

	}
};


REGISTER_FLOW_NODE("AIFactions:SetReaction", CFlowNode_SetFactionsReaction)
REGISTER_FLOW_NODE("AIFactions:GetReaction", CFlowNode_GetFactionsReaction)
REGISTER_FLOW_NODE("AIFactions:SetEntityFaction", CFlowNode_SetEntityFaction)
REGISTER_FLOW_NODE("AIFactions:GetEntityFaction", CFlowNode_GetEntityFaction)