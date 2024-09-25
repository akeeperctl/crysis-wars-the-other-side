#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "TheOtherSideMP\Game\Modules\Factions\FactionsModule.h"
#include "TheOtherSideMP\Game\Modules\Factions\FactionMap.h"
#include <TheOtherSideMP\Game\Modules\Factions\Logger.h>

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
			InputPortConfig<int>("FactionOne", 0, _HELP("Must be > 0"), "FactionOne"),
			InputPortConfig<int>("FactionTwo", 0, _HELP("Must be > 0"), "FactionTwo"),
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
		config.sDescription = _HELP("ONLY ON SERVER. Sets up a reaction between two factions.");
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

					auto reaction = static_cast<IFactionMap::ReactionType>(GetPortInt(pActInfo, EIP_Reaction));

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
	SActivationInfo m_actInfo;
};

REGISTER_FLOW_NODE("Factions:SetReaction", CFlowNode_SetFactionsReaction)