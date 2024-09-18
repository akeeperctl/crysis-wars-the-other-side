/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// FlowStringCombine.cpp
//
// Purpose: FG node that combines two strings
//
// History:
//	- 6/03/08 : File created - KAK
/////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include <iostream>
#include <string.h>

class CFlowNode_StringCombine : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_A,
		EIP_B,
		EIP_Operation,
	};

	enum EOutputPorts
	{
		EOP_Out,
	};

	enum EOperation
	{
		EOP_ADD = 0,
		EOP_SUB,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_StringCombine(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_StringCombine(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		// Define input ports here, in same order as EInputPorts
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<string>("A", "", _HELP("[Out] = [A] op [B]"), NULL, NULL),
			InputPortConfig<string>("B", "", _HELP("[Out] = [A] op [B]"), NULL, NULL),
			InputPortConfig<int>("Op",     EOP_ADD, _HELP("Operation"), 0,  _UICONFIG("enum_int:Add=0,Sub=1")),
			{0}
		};

		// Define output ports here, in same oreder as EOutputPorts
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<string>("out", _HELP("[Out] = [A] op [B]")),
			{0}
		};

		// Fill in configuration
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Math manipulation two strings");
		config.SetCategory(EFLN_ADVANCED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Initialize:
			case eFE_Activate:
			{
				string A = GetPortString(pActInfo, EIP_A);
				string B = GetPortString(pActInfo, EIP_B);
				int op = GetPortInt(pActInfo, EIP_Operation);
				string out;
				switch (op)
				{
					case EOP_ADD:
						out = A + B;
						break;

					case EOP_SUB:
						out = Sub(A, B);
						break;
				}

				ActivateOutput(pActInfo, EOP_Out, out);
			}
			break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_StringCombine(pActInfo);
	}

	string Sub(const string& s1, const string& s2)
	{
		int pos1 = s1.find(s2);

		string res = s1.substr(0, pos1);

		return res;
	}
};

// AlienKeeper node
class CFlowNode_StringCheck : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_MainString,
		EIP_SubString,
	};

	enum EOutputPorts
	{
		EOP_Result,
		EOP_True,
		EOP_False,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_StringCheck(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_StringCheck()
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("Trigger to do check strings")),
			InputPortConfig<string>("MainString", _HELP("The MainString where we do search.")),
			InputPortConfig<string>("SubString", _HELP("Substring for search in the MainString")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Result", _HELP("")),
			OutputPortConfig_AnyType("True", _HELP("")),
			OutputPortConfig_AnyType("False", _HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Checks if MainString includes SubString");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					const string& sMainString = GetPortString(pActInfo, EIP_MainString);
					const string& sSubString = GetPortString(pActInfo, EIP_SubString);

					bool result = false;

					if (strstr(sMainString, sSubString) != 0)
					{
						ActivateOutput(pActInfo, EOP_True, true);
						result = true;
					}
					else
					{
						ActivateOutput(pActInfo, EOP_False, true);
						result = false;
					}

					ActivateOutput(pActInfo, EOP_Result, result);
				}
			}//end of case
			break;
		}//end of switch
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_StringCheck(pActInfo);
	}
};
// ~AlienKeeper node


////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("String:CheckStrings", CFlowNode_StringCheck);
REGISTER_FLOW_NODE("String:Combine", CFlowNode_StringCombine);