/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"

#include "Game.h"
#include "Nodes/G2FlowBaseNode.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"

class CFlowNode_MakePlayerZeus  : public CFlowBaseNode
{

public:
	explicit CFlowNode_MakePlayerZeus(SActivationInfo* pActInfo)
	{
	}

	~CFlowNode_MakePlayerZeus() {};

	IFlowNodePtr Clone(SActivationInfo* pActInfo) 
	{
		return new CFlowNode_MakePlayerZeus(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Trigger = 0,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

	void GetConfiguration(SFlowNodeConfig& config) 
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Trigger", _HELP("Trigger to make player zeus")),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void("Done", _HELP("")),
			{nullptr}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo) 
	{
		if (gEnv->bMultiplayer)
			return;

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
			if (IsPortActive(pActInfo, EIP_Trigger) && pActInfo->pEntity)
			{
				auto pPlayer = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId()));
				if (!pPlayer)
					return;

				g_pTOSGame->GetZeusModule()->NetMakePlayerZeus(pPlayer);

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
		ser.BeginGroup("CFlowNode_MakePlayerZeus");
		ser.EndGroup();
	}
	SActivationInfo m_actInfo;
};

REGISTER_FLOW_NODE("TOSZeus:MakePlayerZeus", CFlowNode_MakePlayerZeus);