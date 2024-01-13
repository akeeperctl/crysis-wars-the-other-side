#include "StdAfx.h"

#include "Game.h"

#include "Nodes/G2FlowBaseNode.h"

#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

class CFlowMCStartControl final : public CFlowBaseNode
{
public:
	explicit CFlowMCStartControl(SActivationInfo* pActInfo)
	{
	}

	~CFlowMCStartControl() override = default;

	IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowMCStartControl(pActInfo);
	}

	enum EInputPorts
	{
		EIP_Start = 0,
		EIP_Cancel,
		EIP_BeamDude,
		EIP_HideDude,
		EIP_DisableSuit,
		EIP_DisableActions,
	};

	enum EOutputPorts
	{
		EOP_Started = 0,
		EOP_Done,
	};

	void GetConfiguration(SFlowNodeConfig& config) override
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Start", _HELP("Trigger to start controlling input actor")),
			InputPortConfig_Void("Cancel", _HELP("Trigger to cancel controlling of input actor")),
			InputPortConfig<bool>("BeamDude", _HELP("Beam dude to input actor position")),
			InputPortConfig<bool>("HideDude", _HELP("Disable dude's model from rendering")),
			InputPortConfig<bool>("DisableSuit", _HELP("Disable dude's nanosuit")),
			InputPortConfig<bool>("DisableActions", _HELP("Disable dude's human actions")),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void("Started", _HELP("")),
			OutputPortConfig_Void("Done", _HELP("")),
			{nullptr}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Master Client Start/Stop");
		config.SetCategory(EFLN_DEBUG);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo) override
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
			if (!pMC)
				return;

			if (pMC->GetSlaveEntity())
				pMC->StopControl(true);
		}
		break;
		case eFE_SetEntityId:
		{
			
		}
		break;
		case eFE_Activate:
		{
			const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
			if (!pMC)
				return;

			const auto pInputEntity = pActInfo->pEntity;
			if (!pInputEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Start))
			{
				if (pMC->GetSlaveEntity())
					pMC->StopControl(true);

				uint flags = 0;
				if (GetPortBool(pActInfo, EIP_BeamDude))
					flags |= TOS_DUDE_FLAG_BEAM_MODEL;

				if (GetPortBool(pActInfo, EIP_HideDude))
					flags |= TOS_DUDE_FLAG_HIDE_MODEL;

				if (GetPortBool(pActInfo, EIP_DisableSuit))
					flags |= TOS_DUDE_FLAG_DISABLE_SUIT;

				if (GetPortBool(pActInfo, EIP_DisableActions))
					flags |= TOS_DUDE_FLAG_ENABLE_ACTION_FILTER;

				pMC->StartControl(pInputEntity, flags, true);
			}
			else if (IsPortActive(pActInfo, EIP_Cancel))
			{
				if (pMC->GetSlaveEntity())
					pMC->StopControl(true);
			}

		}
		break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
	}

	SActivationInfo m_actInfo;
	//~INanoSuitListener
};

REGISTER_FLOW_NODE("TOSMasterClient:StartControl", CFlowMCStartControl);