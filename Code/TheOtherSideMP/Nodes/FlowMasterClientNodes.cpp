#include "StdAfx.h"

#include "Game.h"

#include "Nodes/G2FlowBaseNode.h"

#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"

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
				pMC->StopControl();
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
					pMC->StopControl();

				pMC->StartControl(pInputEntity, TOS_DUDE_FLAG_DISABLE_SUIT | TOS_DUDE_FLAG_ENABLE_ACTION_FILTER);
			}
			else if (IsPortActive(pActInfo, EIP_Cancel))
			{
				if (pMC->GetSlaveEntity())
					pMC->StopControl();
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