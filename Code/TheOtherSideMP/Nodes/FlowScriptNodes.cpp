/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include <Nodes/G2FlowBaseNode.h>

class CFlowNode_CallScriptFunction : public CFlowBaseNode
{
public:
	enum EInputs
	{
		IN_CALL,
		IN_FUNCNAME,
		IN_ARG1,
		IN_ARG2,
		IN_ARG3,
		IN_ARG4,
		//IN_ARG5,
	};
	enum EOutputs
	{
		OUT_SUCCESS,
		OUT_FAILED
	};

	CFlowNode_CallScriptFunction(SActivationInfo* pActInfo)
	{}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		// declare input ports
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_Void("call",         _HELP("Call the function")),
			InputPortConfig<string>("FuncName",  _HELP("Script function name")),
			InputPortConfig_AnyType("Argument1", _HELP("Argument 1")),
			InputPortConfig_AnyType("Argument2", _HELP("Argument 2")),
			InputPortConfig_AnyType("Argument3", _HELP("Argument 3")),
			InputPortConfig_AnyType("Argument4", _HELP("Argument 4")),
			//InputPortConfig_AnyType("Argument5", _HELP("Argument 5")),
			{ 0 }
		};
		static const SOutputPortConfig out_ports[] = {
			OutputPortConfig_Void("Success", _HELP("Script function was found and called")),
			OutputPortConfig_Void("Failed",  _HELP("Script function was not found")),
			{ 0 }
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Calls a script function on the entity");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (eFE_Activate == event && IsPortActive(pActInfo, IN_CALL))
		{
			if (pActInfo->pEntity)
			{
				//Get entity's scripttable

				IEntityScriptProxy* pScriptProxy = dynamic_cast<IEntityScriptProxy*>(pActInfo->pEntity->GetProxy(ENTITY_PROXY_SCRIPT));
				IScriptTable* pTable = pScriptProxy->GetScriptTable();

				if (pTable)
				{
					//Convert all inputports to arguments for lua
					const ScriptAnyValue arg1 = FillArgumentFromAnyPort(pActInfo, IN_ARG1);
					const ScriptAnyValue arg2 = FillArgumentFromAnyPort(pActInfo, IN_ARG2);
					const ScriptAnyValue arg3 = FillArgumentFromAnyPort(pActInfo, IN_ARG3);
					const ScriptAnyValue arg4 = FillArgumentFromAnyPort(pActInfo, IN_ARG4);
					//ScriptAnyValue arg5 = FillArgumentFromAnyPort(pActInfo, IN_ARG5);

					Script::CallMethod(pTable, GetPortString(pActInfo, IN_FUNCNAME), arg1, arg2, arg3, arg4);

					ActivateOutput(pActInfo, OUT_SUCCESS, true);
				}
			}

			ActivateOutput(pActInfo, OUT_FAILED, true);
		}
	}

	ScriptAnyValue FillArgumentFromAnyPort(SActivationInfo* pActInfo, int port)
	{
		const TFlowInputData inputData = GetPortAny(pActInfo, port);

		switch (inputData.GetType())
		{
			case eFDT_Int:
				return ScriptAnyValue((float)GetPortInt(pActInfo, port));
			case eFDT_EntityId:
			{
				ScriptHandle id;
				id.n = GetPortEntityId(pActInfo, port);
				return ScriptAnyValue(id);
			}
			case eFDT_Bool:
				return ScriptAnyValue(GetPortBool(pActInfo, port));
			case eFDT_Float:
				return ScriptAnyValue(GetPortFloat(pActInfo, port));
			case eFDT_String:
				return ScriptAnyValue(GetPortString(pActInfo, port));
				;
			case eFDT_Vec3:
				return ScriptAnyValue(GetPortVec3(pActInfo, port));
				;
		}

		//Type unknown
		assert(false);

		return ScriptAnyValue();
	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};

REGISTER_FLOW_NODE("Entity:CallScriptFunction", CFlowNode_CallScriptFunction);
