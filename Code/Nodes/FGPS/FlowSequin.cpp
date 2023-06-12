#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "Nodes/FGPS/FlowSystemCvarsC3.h"
#include "Cry_Math.h"

class CFlowNode_Sequentializer : public CFlowBaseNode
{
public:

	enum
	{
		NUM_OUTPUTS = 11,
		PORT_NONE = 0xffffffff,
	};

	CFlowNode_Sequentializer(SActivationInfo* pActInfo)
		: m_needToCheckConnectedPorts(true)
		, m_closed(false)
		, m_lastTriggeredPort(PORT_NONE)
		, m_numConnectedPorts(0)
	{
		m_connectedPorts;
	}

	enum INPUTS
	{
		IN_Input = 0,
		IN_Closed,
		IN_Open,
		IN_Close,
		IN_Reset,
		IN_Reverse
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static_assert(PORT_NONE + 1 == 0, "Unexpected enum value!"); // or else the automatic boundary checks when incrememting the port number would not work

		static const SInputPortConfig in_config[] = {
			InputPortConfig_AnyType("In",    _HELP("Input")),
			InputPortConfig<bool>("Closed",  false,                                         _HELP("If true blocks all signals.")),
			InputPortConfig_Void("Open",     _HELP("Sets [Closed] to false.")),
			InputPortConfig_Void("Close",    _HELP("Sets [Closed] to true.")),
			InputPortConfig_Void("Reset",    _HELP("Forces next output to be Port1 again")),
			InputPortConfig<bool>("Reverse", false,                                         _HELP("If true, the order of output activation is reversed.")),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_AnyType("Out0",  _HELP("Output0")),
			OutputPortConfig_AnyType("Out1",  _HELP("Output1")),
			OutputPortConfig_AnyType("Out2",  _HELP("Output2")),
			OutputPortConfig_AnyType("Out3",  _HELP("Output3")),
			OutputPortConfig_AnyType("Out4",  _HELP("Output4")),
			OutputPortConfig_AnyType("Out5",  _HELP("Output5")),
			OutputPortConfig_AnyType("Out6",  _HELP("Output6")),
			OutputPortConfig_AnyType("Out7",  _HELP("Output7")),
			OutputPortConfig_AnyType("Out8",  _HELP("Output8")),
			OutputPortConfig_AnyType("Out9",  _HELP("Output9")),
			OutputPortConfig_AnyType("Out10", _HELP("Output10")),
			{ 0 }
		};
		config.sDescription = _HELP("On each [In] trigger, triggers one of the connected outputs in sequential order.");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		//		config.nFlags |= EFLN_AISEQUENCE_SUPPORTED;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
			m_closed = GetPortBool(pActInfo, IN_Closed);

		case eFE_ConnectOutputPort:
		case eFE_DisconnectOutputPort:
		{
			m_lastTriggeredPort = PORT_NONE;
			m_needToCheckConnectedPorts = true;
			m_numConnectedPorts = 0;
			break;
		}

		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, IN_Closed))
				m_closed = GetPortBool(pActInfo, IN_Closed);
			if (IsPortActive(pActInfo, IN_Open))
				m_closed = false;
			if (IsPortActive(pActInfo, IN_Close))
				m_closed = true;

			if (IsPortActive(pActInfo, IN_Reset))
			{
				m_lastTriggeredPort = PORT_NONE;
			}

			if (m_needToCheckConnectedPorts)
			{
				m_needToCheckConnectedPorts = false;
				m_numConnectedPorts = 0;
				for (int port = 0; port < NUM_OUTPUTS; ++port)
				{
					if (IsOutputConnected(pActInfo, port))
					{
						m_connectedPorts[m_numConnectedPorts] = port;
						m_numConnectedPorts++;
					}
				}
			}

			if (IsPortActive(pActInfo, IN_Input) && m_numConnectedPorts > 0 && !m_closed)
			{
				bool reversed = GetPortBool(pActInfo, IN_Reverse);
				unsigned int port = m_lastTriggeredPort;

				if (reversed)
				{
					port = min(m_numConnectedPorts - 1, port - 1); // это заботится как о начальном состоянии, когда оно имеет значение PORT_NONE, так и о переполнении в нормальной ситуации уменьшения
				}
				else
				{
					port = (port + 1) % m_numConnectedPorts;
				}

				ActivateOutput(pActInfo, m_connectedPorts[port], GetPortAny(pActInfo, IN_Input));
				m_lastTriggeredPort = port;
			}
			break;
		}
		}
	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		CFlowNode_Sequentializer* pClone = new CFlowNode_Sequentializer(pActInfo);
		return pClone;
	}

	virtual void Serialize(SActivationInfo*, TSerialize ser)
	{
		ser.Value("m_lastTriggeredPort", m_lastTriggeredPort);
		ser.Value("m_closed", m_closed);
	}

	bool IsOutputConnected(SActivationInfo* pActInfo, int nPort) const
	{
		SFlowAddress addr(pActInfo->myID, nPort, true);
		return pActInfo->pGraph->IsOutputConnected(addr);
	}

	bool         m_needToCheckConnectedPorts;
	bool         m_closed;
	unsigned int m_lastTriggeredPort;
	unsigned int m_numConnectedPorts;
	int          m_connectedPorts[NUM_OUTPUTS];
};

REGISTER_FLOW_NODE("Logic:Sequentializer", CFlowNode_Sequentializer);