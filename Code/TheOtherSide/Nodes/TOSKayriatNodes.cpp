// -------------------------------------------------------------------------
// Kayriat Source File.
// fuckCopyright Kayriat
// -------------------------------------------------------------------------
#include "StdAfx.h"

#include "IAISystem.h"

#include <Nodes/FGPS/FlowSystemCvarsC3.h>
#include <Nodes/G2FlowBaseNode.h>

#include "IInput.h"
#include "IWorldQuery.h"

#include "Actor.h"
#include "Alien.h"
#include "Scout.h"

#include "Game.h"
#include "GameRules.h"
#include "GameCVars.h"
#include "GameActions.h"

#include "PlayerInput.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDScopes.h"
#include "HUD/GameFlashAnimation.h"

#include "Weapon.h"

#include <StringUtils.h>

namespace
{
	// to be replaced. hud is no longer a game object
	void SendHUDEvent(SGameObjectEvent& evt)
	{
		SAFE_HUD_FUNC(HandleEvent(evt));
	}
}

class CFlowNode_SetInteger : public CFlowBaseNode
{
public:
	CFlowNode_SetInteger(SActivationInfo* pActInfo) {}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("set", _HELP("Send value to output when receiving event on this port")),
			InputPortConfig<int>("in",  _HELP("Value to be set on output")),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("out"),
			{ 0 }
		};
		config.sDescription = _HELP("Send an integer input value to the output when event on set port is received");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED); // POLICY-CHANGE: don't send to output on initialize
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				ActivateOutput(pActInfo, 0, GetPortInt(pActInfo, 1));
			}
			break;
		}
	};

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Round
class CFlowNode_Round : public CFlowBaseNode
{
public:
	CFlowNode_Round(SActivationInfo* pActInfo) {}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<float>("In"),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("outRounded"),
			{ 0 }
		};
		config.sDescription = _HELP("Rounds an input float value to an integer value");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		case eFE_Initialize:
			float inpVal = GetPortFloat(pActInfo, 0);
			int out = int_round(inpVal);
			ActivateOutput(pActInfo, 0, out);
			break;
		}
	};

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�����������
class CFlowNode_Calculate : public CFlowBaseNode
{
	enum
	{
		INP_DoCalc,
		INP_Operation,
		INP_A,
		INP_B
	};
	enum EOperation
	{
		EOP_ADD = 0,
		EOP_SUB,
		EOP_MUL,
		EOP_DIV
	};

public:
	CFlowNode_Calculate(SActivationInfo* pActInfo) {};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("DoCalc", _HELP("Do the calc and send the result to the output when receiving event on this port")),
			InputPortConfig<int>("Op",     EOP_ADD, _HELP("Operation"),0, _UICONFIG("enum_int:Add=0,Sub=1,Mul=2,Div=3")),
			InputPortConfig<float>("A",    _HELP(" ")),
			InputPortConfig<float>("B",    _HELP(" ")),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<float>("out"),
			{ 0 }
		};
		config.sDescription = _HELP("[out] = result of the specified operation with [A] and [B]");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, INP_DoCalc))
			{
				float a = GetPortFloat(pActInfo, INP_A);
				float b = GetPortFloat(pActInfo, INP_B);
				int op = GetPortInt(pActInfo, INP_Operation);
				float out = 0;
				switch (op)
				{
				case EOP_ADD:
					out = a + b;
					break;

				case EOP_SUB:
					out = a - b;
					break;

				case EOP_MUL:
					out = a * b;
					break;

				case EOP_DIV:
					if (!iszero(b))
						out = a / b;
					break;
				}

				ActivateOutput(pActInfo, 0, out);
			}
			break;
		}
		}
	};

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//In Range
class CFlowNode_InRange : public CFlowBaseNode
{
public:
	CFlowNode_InRange(SActivationInfo* pActInfo) {}

	enum
	{
		INP_In = 0,
		INP_Min,
		INP_Max,
	};
	enum EOperation
	{
		EOP_Out = 0,
		EOP_True,
		EOP_False,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<float>("In"),
			InputPortConfig<float>("Min"),
			InputPortConfig<float>("Max"),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<bool>("out",  _HELP("true when [in]>=[Min] and [in]<=[Max], false otherwise")),
			OutputPortConfig_Void("true",  _HELP("triggered if In is in range (>=min and <=max)")),
			OutputPortConfig_Void("false", _HELP("triggered if In is in range (>=min and <=max)")),
			{ 0 }
		};
		config.sDescription = _HELP("[out] is true when [in]>=[Min] and [in]<=[Max], false otherwise");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		bool bOut = false;
		switch (event)
		{
		case eFE_Activate:
		case eFE_Initialize:
			float in = GetPortFloat(pActInfo, INP_In);
			float v_min = GetPortFloat(pActInfo, INP_Min);
			float v_max = GetPortFloat(pActInfo, INP_Max);
			bOut = (in >= v_min && in <= v_max) ? true : false;

			if (bOut)
				ActivateOutput(pActInfo, EOP_True, true);
			else
				ActivateOutput(pActInfo, EOP_False, true);

			ActivateOutput(pActInfo, EOP_Out, bOut);
			break;
		}
	};

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove entity
class CFlowNode_RemoveEntity : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_RemoveEntity,
		EIP_ID,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_RemoveEntity(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_RemoveEntity(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Trigger", _HELP("Trigger to delete entity")),
			InputPortConfig<EntityId>("ID", _HELP("Give a ID of entity to delete")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Done", _HELP("Job is done")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Remove entity.Better to use with the Entity:SpawnArchetypes node if need to delete spawned entity.");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_RemoveEntity))
			{
				EntityId rEntity = GetPortEntityId(pActInfo, EIP_ID); // �������� ������ � ������

				gEnv->pEntitySystem->RemoveEntity(rEntity, true); // ������� �������� �������� ���� false �� �������� ����

				ActivateOutput(pActInfo, EOP_Done, true);
			}
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
		return new CFlowNode_RemoveEntity(pActInfo);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFlowNode_GetOtherData : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sink,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_SpeciesNum,
		EOP_GroupNum,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetOtherData(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetOtherData(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Trigger", _HELP("Trigger to start")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Done", _HELP("Job is done")),
			OutputPortConfig<int>("Species", _HELP("Get a species from entity")),
			OutputPortConfig<int>("GroupID", _HELP("Get a group from entity")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Allow Get Other data from AI interface");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Sink))
			{
				//////////////////////////////////////////////////////////////
				IEntity* pFlowEntity = pActInfo->pEntity; //get entity data
				if (!pFlowEntity)
					return;
				IAIObject* pFlowAI = pFlowEntity->GetAI(); //get ai data from entity
				if (!pFlowAI)
					return;
				IAIActor* pFlowAIActor = pFlowAI->CastToIAIActor(); // Get interface of ai actor
				int DoneSpecies = pFlowAIActor->GetParameters().m_nSpecies;
				int DoneGroup = pFlowAIActor->GetParameters().m_nGroup;
				/////////////////////////////////////////////////////////////////////
				/////////////////////////////////////////////////////////////////////
				ActivateOutput(pActInfo, EOP_Done, true);
				ActivateOutput(pActInfo, EOP_SpeciesNum, DoneSpecies);
				ActivateOutput(pActInfo, EOP_GroupNum, DoneGroup);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetOtherData(pActInfo);
	}
};

class CFlowNode_AISelectGoalPipe : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger = 0,
		EIP_Goal,
		EIP_Arg,
		EIP_Update,
		EIP_SubPipe,
	};

	enum EOutputPorts
	{
		EOP_Done = 0,
	};

	EntityId argId = 0;

	IEntity* pArgEntity = 0;
	IPipeUser* pActor = 0;

public:
	////////////////////////////////////////////////////
	CFlowNode_AISelectGoalPipe(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AISelectGoalPipe(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("Trigger to start")),
			InputPortConfig<string>("GoalPipeName", _HELP("Write a goal pipe name. You can find them in Game\Scripts\AI\GoalPipes")),
			InputPortConfig<EntityId>("TargetEntity", _HELP("")),
			InputPortConfig<bool>("Update",0, _HELP("")),
			InputPortConfig<bool>("SubPipe",0, _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Done", _HELP("Job is done")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Allow Get Other data from AI interface");
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
				string GoalPipe = GetPortString(pActInfo, EIP_Goal);
				argId = GetPortEntityId(pActInfo, EIP_Arg);

				if (pActInfo->pEntity)
				{
					IAIObject* pActorAI = pActInfo->pEntity->GetAI();

					if (pActorAI)
					{
						IAIObject* ArgAIObject;

						pActor = pActorAI->CastToIPipeUser();

						if (argId != 0)
						{
							pArgEntity = gEnv->pEntitySystem->GetEntity(argId);
							pActor->SetRefPointPos(pArgEntity->GetWorldPos());
						}
						else
							pArgEntity = NULL;

						if (pArgEntity)
							ArgAIObject = pArgEntity->GetAI();
						else
							ArgAIObject = NULL;

						if (pActor)
						{
							bool bSubPipe = GetPortBool(pActInfo, EIP_SubPipe);
							bSubPipe ? pActor->InsertSubPipe(0, GoalPipe, ArgAIObject) : pActor->SelectPipe(0, GoalPipe, ArgAIObject);

							bool bActivateUpdate = GetPortBool(pActInfo, EIP_Update);
							pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, bActivateUpdate);

							ActivateOutput(pActInfo, EOP_Done, true);
						}
					}
				}
			}

			if (IsPortActive(pActInfo, EIP_Update))
			{
				bool bActivateUpdate = GetPortBool(pActInfo, EIP_Update);
				if (!bActivateUpdate)
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case eFE_Update:
		{
			if (pArgEntity = gEnv->pEntitySystem->GetEntity(argId))
			{
				if (pActor)
					pActor->SetRefPointPos(pArgEntity->GetWorldPos());
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AISelectGoalPipe(pActInfo);
	}
};

class CFlowNode_AIIsUsingPipe : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger = 0,
		EIP_GoalPipe = 0,
	};

	enum EOutputPorts
	{
		EOP_Using = 0,
		EOP_True = 0,
		EOP_False = 0,
	};
	IPipeUser* pPipe = 0;

public:
	////////////////////////////////////////////////////
	CFlowNode_AIIsUsingPipe(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIIsUsingPipe(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("Trigger to check")),
			InputPortConfig<string>("GoalPipe", _HELP("Trigger to get")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Using", _HELP("")),
			OutputPortConfig_AnyType("True", _HELP("")),
			OutputPortConfig_AnyType("False", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
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
				string sGoalPipe = GetPortString(pActInfo, EIP_GoalPipe);

				if (pActInfo->pEntity)
				{
					IAIObject* pActorAI = pActInfo->pEntity->GetAI();

					if (pActorAI)
					{
						pPipe = pActorAI->CastToIPipeUser();
						if (pPipe)
						{
							bool bIsUsing = pPipe->IsUsingPipe(sGoalPipe);
							ActivateOutput(pActInfo, EOP_Using, bIsUsing);
							ActivateOutput(pActInfo, bIsUsing ? EOP_True : EOP_False, 1);
						}
					}
				}
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AIIsUsingPipe(pActInfo);
	}
};

class CFlowNode_EntityCheckDistance : public CFlowBaseNode
{
public:
	enum EInputs
	{
		INP_CHECK,
		INP_MIN_DIST,
		INP_MAX_DIST,
		INP_ENT00,
		INP_ENT01,
		INP_ENT02,
		INP_ENT03,
		INP_ENT04,
		INP_ENT05,
		INP_ENT06,
		INP_ENT07,
		INP_ENT08,
		INP_ENT09,
		INP_ENT10,
		INP_ENT11,
		INP_ENT12,
		INP_ENT13,
		INP_ENT14,
		INP_ENT15,
		INP_ENT16,
		INP_ENT17,
		INP_ENT18,
	};
	enum EOutputs
	{
		OUT_NEAR_ENT = 0,
		OUT_NEAR_ENT_DIST,
		OUT_FAR_ENT,
		OUT_FAR_ENT_DIST,
		OUT_NOENTITIES_IN_RANGE,
	};

	CFlowNode_EntityCheckDistance(SActivationInfo* pActInfo)
	{
	}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Check",         _HELP("Trigger to check distances")),
			InputPortConfig<float>("MinDistance", _HELP("Any entity that is nearer than this, will be ignored")),
			InputPortConfig<float>("MaxDistance", _HELP("Any entity that is farther than this, will be ignored (0=no limit)")),
			InputPortConfig<EntityId>("Entity1",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity2",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity3",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity4",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity5",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity6",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity7",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity8",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity9",  _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity10", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity11", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity12", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity13", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity14", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity15", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity16", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity17", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity18", _HELP("EntityID")),
			InputPortConfig<EntityId>("Entity19", _HELP("EntityID")),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<EntityId>("NearEntity",  _HELP("Nearest entity")),
			OutputPortConfig<float>("NearEntityDist", _HELP("Nearest entity distance")),
			OutputPortConfig<EntityId>("FarEntity",   _HELP("Farthest entity")),
			OutputPortConfig<float>("FarEntityDist",  _HELP("Farthest entity distance")),
			OutputPortConfig_AnyType("NoEntInRange",  _HELP("Trigered when none of the entities were between Min and Max distance")),
			{ 0 }
		};
		config.sDescription = _HELP("Check distance between the node entity and the entities defined in the inputs");
		config.nFlags |= EFLN_TARGET_ENTITY; //| EFLN_AISEQUENCE_SUPPORTED;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (event != eFE_Activate || !IsPortActive(pActInfo, INP_CHECK))
			return;

		IEntity* pEntityNode = pActInfo->pEntity;
		if (!pEntityNode)
			return;

		float minRangeDist = GetPortFloat(pActInfo, INP_MIN_DIST);
		float maxRangeDist = GetPortFloat(pActInfo, INP_MAX_DIST);
		float minRangeDist2 = minRangeDist * minRangeDist;
		float maxRangeDist2 = maxRangeDist * maxRangeDist;
		if (maxRangeDist2 == 0)
		{
			maxRangeDist2 = FLT_MAX; // no limit on max distance when the input is 0
		}

		EntityId minEnt = 0;
		EntityId maxEnt = 0;
		float minDist2 = maxRangeDist2;
		float maxDist2 = minRangeDist2;
		bool anyEntityInRange = false;

		for (uint32 i = INP_ENT00; i <= INP_ENT15; ++i)
		{
			EntityId entityIdCheck = GetPortEntityId(pActInfo, i);
			IEntity* pEntityCheck = gEnv->pEntitySystem->GetEntity(entityIdCheck);
			if (pEntityCheck)
			{
				float dist2 = pEntityCheck->GetWorldPos().GetSquaredDistance(pEntityNode->GetWorldPos());
				if (dist2 >= minRangeDist2 && dist2 <= maxRangeDist2)
				{
					anyEntityInRange = true;
					if (dist2 <= minDist2)
					{
						minDist2 = dist2;
						minEnt = entityIdCheck;
					}
					if (dist2 >= maxDist2)
					{
						maxDist2 = dist2;
						maxEnt = entityIdCheck;
					}
				}
			}
		}

		if (anyEntityInRange)
		{
			ActivateOutput(pActInfo, OUT_NEAR_ENT, minEnt);
			ActivateOutput(pActInfo, OUT_NEAR_ENT_DIST, sqrtf(minDist2));

			ActivateOutput(pActInfo, OUT_FAR_ENT, maxEnt);
			ActivateOutput(pActInfo, OUT_FAR_ENT_DIST, sqrtf(maxDist2));
		}
		else
		{
			ActivateOutput(pActInfo, OUT_NOENTITIES_IN_RANGE, true);
		}
	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};

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
	{
	}

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

				IEntityScriptProxy* pScriptProxy = static_cast<IEntityScriptProxy*>(pActInfo->pEntity->GetProxy(ENTITY_PROXY_SCRIPT));
				IScriptTable* pTable = pScriptProxy->GetScriptTable();

				if (pTable)
				{
					//Convert all inputports to arguments for lua
					ScriptAnyValue arg1 = FillArgumentFromAnyPort(pActInfo, IN_ARG1);
					ScriptAnyValue arg2 = FillArgumentFromAnyPort(pActInfo, IN_ARG2);
					ScriptAnyValue arg3 = FillArgumentFromAnyPort(pActInfo, IN_ARG3);
					ScriptAnyValue arg4 = FillArgumentFromAnyPort(pActInfo, IN_ARG4);
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
		TFlowInputData inputData = GetPortAny(pActInfo, port);

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

class CFlowNode_SwitchView : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_ToggleTPV = 0,
		EIP_ToggleFPV,
	};

	enum EOutputPorts
	{
		EOP_TPVActivated = 0,
		EOP_FPVActivated = 0,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SwitchView(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SwitchView(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("ToggleTPV", _HELP("Toggle third person view on input entity")),
			InputPortConfig_AnyType("ToggleFPV", _HELP("Toggle first person view on input entity")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("TPV", _HELP("Third person view is activated")),
			OutputPortConfig_AnyType("FPV", _HELP("First person view is activated")),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_ToggleTPV && pActInfo->pEntity))
			{
				IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					EntityId pID = pFlowEntity->GetId();
					if (pID)
					{
						IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pID);
						if (pActor)
						{
							pActor->ToggleThirdPerson();
							ActivateOutput(pActInfo, EOP_TPVActivated, true);
						}
					}
				}
			}

			else if (IsPortActive(pActInfo, EIP_ToggleFPV && pActInfo->pEntity))
			{
				IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					EntityId pID = pFlowEntity->GetId();
					if (pID)
					{
						IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pID);
						if (pActor && pActor->IsThirdPerson())
						{
							pActor->GetAnimationGraphState()->SetFirstPersonMode(true);
							ActivateOutput(pActInfo, EOP_FPVActivated, true);
						}
					}
				}
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SwitchView(pActInfo);
	}
};

class CFlowNode_GetEntityBounds : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get = 0,
	};

	enum EOutputPorts
	{
		EOP_Center
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetEntityBounds(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetEntityBounds(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Get", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<Vec3>("Center", _HELP("")),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Get && pActInfo->pEntity))
			{
				IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					AABB bounds;
					pFlowEntity->GetWorldBounds(bounds);
					ActivateOutput(pActInfo, EOP_Center, bounds.GetCenter());
				}
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetEntityBounds(pActInfo);
	}
};

class CFlowNode_EC_GetIdOfLook : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start = 0,
		EIP_Stop,
	};

	enum EOutputPorts
	{
		EOP_Done = 0,
		EOP_Id,
	};

	CActor* actor;
public:
	////////////////////////////////////////////////////
	CFlowNode_EC_GetIdOfLook(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EC_GetIdOfLook(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("Trigger to start")),
			InputPortConfig_AnyType("Stop", _HELP("Trigger to start")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done", _HELP("Job is done")),
			OutputPortConfig<EntityId>("EntityId", _HELP("EntityId from look target")),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Start))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				IEntity* entity = pActInfo->pEntity;
				if (entity)
				{
					EntityId id = entity->GetId();
					if (id)
					{
						actor = static_cast<CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
					}
				}
			}
			else if (IsPortActive(pActInfo, EIP_Stop))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				ActivateOutput(pActInfo, EOP_Done, true);
			}
		}
		case eFE_Update:
		{
			if (actor)
			{
				EntityId lookId = actor->GetGameObject()->GetWorldQuery()->GetLookAtEntityId();
				ActivateOutput(pActInfo, EOP_Id, lookId);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_EC_GetIdOfLook(pActInfo);
	}
};

class CFlowNode_EC_AlienMovement : public CFlowBaseNode, public IGameFrameworkListener
{
	enum EInputPorts
	{
		EIP_Sink,
		EIP_Stop,
		EIP_Speed,
		EIP_AxisType,
	};

	enum EOutputPorts
	{
	};

	EntityId m_entityId;
	CAlien* pAlien;
	CScout* pScout;
	IEntity* pFlowEntity;

public:
	////////////////////////////////////////////////////
	CFlowNode_EC_AlienMovement(SActivationInfo* pActInfo)
	{
		g_pGame->GetIGameFramework()->RegisterListener(this, "CFlowNode_EC_AlienMovement", FRAMEWORKLISTENERPRIORITY_DEFAULT); // need for game save
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EC_AlienMovement()
	{
		g_pGame->GetIGameFramework()->UnregisterListener(this);
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const char* uiConfig =
			"enum_int:Xaxis=1,Yaxis=2,Zaxis=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Start",	_HELP("Trigger to start")),
			InputPortConfig_Void("Stop",		_HELP("Trigger to stop")),
			InputPortConfig<float>("Speed",	_HELP("Movement speed from")),
			InputPortConfig<int>("AxisType", kXaxis, _HELP("Type of axis the entity will move"), 0, uiConfig),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			//OutputPortConfig_Void("Done", _HELP("Job is done")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Makes the attached non fly alien move along the selected direction. Work only aliens who use a ground for movement(trooper, hunter)");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			//case eFE_Initialize:
			//{
			//}//~case
			//break;

			//case eFE_SetEntityId:
			//{
			//}//~case
			//break;

		case eFE_Activate:
		{
			return;

			if (m_entityId)
			{
				string strClassName = pFlowEntity->GetClass()->GetName();
				if ((strClassName == "PlayerTrooper" || strClassName == "Hunter" || strClassName == "Scout")) //this condition fix crash
				{
					pAlien = static_cast<CAlien*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_entityId));
					if (!pAlien)
						return;
				}
			}
			else return;

			int m_selectedAxis = GetPortInt(pActInfo, EIP_AxisType);
			float m_fspeed = GetPortFloat(pActInfo, EIP_Speed);

			/*if(pAlien->m_moveRequest.type == eCMT_Normal || pAlien->m_moveRequest.type == eCMT_Fly)
			{
				if (m_selectedAxis==1)
				{
					if	(IsPortActive(pActInfo, EIP_Sink))
					{
						pAlien->m_input.deltaMovement.x = m_fspeed;
					}
					else if (IsPortActive(pActInfo, EIP_Stop))
					{
						pAlien->m_input.deltaMovement.x = 0;
					}
				}

				else if (m_selectedAxis==2)
				{
					if (IsPortActive(pActInfo, EIP_Sink))
					{
						pAlien->m_input.deltaMovement.y = m_fspeed;
					}
					else if (IsPortActive(pActInfo, EIP_Stop))
					{
						pAlien->m_input.deltaMovement.y = 0;
					}
				}

				else if (m_selectedAxis==3)
				{
					if (IsPortActive(pActInfo, EIP_Sink))
					{
						pAlien->m_input.deltaMovement.z = m_fspeed;
					}
					else if (IsPortActive(pActInfo, EIP_Stop))
					{
						pAlien->m_input.deltaMovement.z = 0;
					}
				}
			} end of if(request.type)*/
		}//end of case
		break;
		}//end of switch
	}//end of ProcessEvent

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual void OnSaveGame(ISaveGame* pSaveGame)
	{
		/*if (pAlien)
			pAlien->m_input.deltaMovement.zero();*/
	}
	virtual void OnLoadGame(ILoadGame* pLoadGame)
	{
		/*if (pAlien)
			pAlien->m_input.deltaMovement.zero();*/
	}
	virtual void OnLevelEnd(const char* nextLevel)
	{
		/*if (pAlien)
			pAlien->m_input.deltaMovement.zero();*/
	}
	virtual void OnActionEvent(const SActionEvent& event) {}
	virtual void OnPostUpdate(float fDeltaTime) {}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_EC_AlienMovement(pActInfo);
	}

	//////////Call variables//////////
		//-----------------------//

	enum MoveAxis
	{
		kZero = 0, //I no use
		kXaxis,
		kYaxis,
		kZaxis,
	};
	//-----------------------//
};

class CFlowNode_EC_GetOnGround : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
	};

	enum EOutputPorts
	{
		EOP_True,
		EOP_False,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_EC_GetOnGround(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EC_GetOnGround()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Get",	_HELP("Get onGround status")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("True", _HELP("")),
			OutputPortConfig_AnyType("False", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			IGameFramework* pGame = g_pGame->GetIGameFramework();
			if (pGame)
			{
				if (IsPortActive(pActInfo, EIP_Get && pActInfo->pEntity))
				{
					IEntity* pEntity = pActInfo->pEntity;
					if (pEntity)
					{
						EntityId pEntityID = pEntity->GetId();
						if (pEntityID)
						{
							CActor* pActor = static_cast <CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityID));
							if (pActor)
							{
								float onGroundValue = pActor->GetActorStats()->onGround;
								if (onGroundValue > 0.0f)
								{
									ActivateOutput(pActInfo, EOP_True, true);
								}
								else
								{
									ActivateOutput(pActInfo, EOP_False, true);
								}
							}
						}
					}
				}
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
		return new CFlowNode_EC_GetOnGround(pActInfo);
	}
};

class CFlowNode_GetMaxHealth : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_MaxHealth,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetMaxHealth(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetMaxHealth()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Get",	_HELP("Get onGround status")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("MaxHealth", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get max health of input entity");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			IGameFramework* pGame = g_pGame->GetIGameFramework();
			if (pGame)
			{
				if (IsPortActive(pActInfo, EIP_Trigger && pActInfo->pEntity))
				{
					IEntity* pEntity = pActInfo->pEntity;
					if (pEntity)
					{
						EntityId pEntityID = pEntity->GetId();
						if (pEntityID)
						{
							CActor* pActor = static_cast <CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityID));
							if (pActor)
							{
								int iMaxHealth = pActor->GetMaxHealth();
								ActivateOutput(pActInfo, EOP_MaxHealth, iMaxHealth);
							}
						}
					}
				}
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
		return new CFlowNode_GetMaxHealth(pActInfo);
	}
};

class CFlowNode_SetWeaponHost : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_SetHost,
		EIP_GetHost,
		EIP_HostID,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_GetHost,
	};

	EntityId m_hostId;
	CItem* pItem;
	IWeapon* pWeapon;

public:
	////////////////////////////////////////////////////
	CFlowNode_SetWeaponHost(SActivationInfo* pActInfo)
	{
		m_hostId = 0;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SetWeaponHost()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("SetHost",	_HELP("")),
			InputPortConfig_AnyType("GetHost",	_HELP("")),
			InputPortConfig<EntityId>("HostId",	_HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done", _HELP("")),
			OutputPortConfig<EntityId>("HostId", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Set host of input weapon");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			m_hostId = 0;
		}
		break;

		case eFE_Activate:
		{
			IGameFramework* pGame = g_pGame->GetIGameFramework();
			if (pGame)
			{
				if (IsPortActive(pActInfo, EIP_HostID))
				{
					m_hostId = GetPortEntityId(pActInfo, EIP_HostID);
				}

				if (IsPortActive(pActInfo, EIP_SetHost && pActInfo->pEntity))
				{
					if (pActInfo->pEntity->GetId())
					{
						if (m_hostId == 0)
							return;

						pItem = static_cast<CItem*>(g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pActInfo->pEntity->GetId()));
						pWeapon = pItem->GetIWeapon();
						CAlien* m_pAlien = static_cast<CAlien*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_hostId));

						if (pWeapon)
						{
							//pItem->SetOwnerId(m_hostId);
							//pItem->EnableSound(false);

							pWeapon->SetHostId(m_hostId);

							ActivateOutput(pActInfo, EOP_Done, true);
						}
					}
				}
				if (IsPortActive(pActInfo, EIP_GetHost && pActInfo->pEntity))
				{
					IEntity* pEntity = pActInfo->pEntity;
					if (pEntity)
					{
						EntityId pEntityId = pEntity->GetId();
						if (pEntityId)
						{
							IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pEntityId);

							IWeapon* pWeapon = pItem->GetIWeapon();
							if (pWeapon)
							{
								EntityId HostID = pWeapon->GetHostId();
								ActivateOutput(pActInfo, EOP_GetHost, HostID);
							}
						}
					}
				}
			}
		}//end of case
		break;

		/*case eFE_Update:
		{
			if (!m_bAiming)
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
			else
				SetDestination(m_targetPos);
		}
		break;*/
		}//end of switch
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SetWeaponHost(pActInfo);
	}
};

class CFlowNode_GetActorItemId : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_GetID,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_WeaponID,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetActorItemId(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetActorItemId()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("GetId",	_HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done", _HELP("")),
			OutputPortConfig<EntityId>("WeaponId", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get actor's weapon id");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_GetID))
			{
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId);
						if (pActor && pActor->GetHealth() > 0)
						{
							IInventory* pInventory = pActor->GetInventory();
							if (!pInventory)
								return;
							EntityId itemId = pInventory->GetCurrentItem();
							if (itemId == 0)
								return;
							IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
							if (pItem == 0)
								return;
							EntityId pNewItemId = pItem->GetEntityId();
							if (pNewItemId != 0)
							{
								ActivateOutput(pActInfo, EOP_WeaponID, pNewItemId);
								ActivateOutput(pActInfo, EOP_Done, true);
							}
						}
					}
				}
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
		return new CFlowNode_GetActorItemId(pActInfo);
	}
};

class CFlowNode_GetEntityIdByName : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
		EIP_Name,
	};

	enum EOutputPorts
	{
		EOP_Id,
		EOP_False,
		EOP_True,
	};

	IEntity* entity;

public:
	////////////////////////////////////////////////////
	CFlowNode_GetEntityIdByName(SActivationInfo* pActInfo)
	{
		entity = NULL;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetEntityIdByName()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Get", _HELP("Get id of input entity")),
			InputPortConfig<string>("Name", _HELP("Name of entity that we need.(We need full name of entity)")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("EntityId", _HELP("Id of found entity")),
			OutputPortConfig_AnyType("False", _HELP("")),
			OutputPortConfig_AnyType("True", _HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Find entity by her name");
		config.SetCategory(EFLN_APPROVED); //28.01.2020
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const string name = GetPortString(pActInfo, EIP_Name);
			IEntitySystem* pSystem = gEnv->pEntitySystem;
			if (IsPortActive(pActInfo, EIP_Get))
			{
				entity = pSystem->FindEntityByName(name);
				if (entity)
				{
					EntityId id = entity->GetId();

					ActivateOutput(pActInfo, EOP_Id, id);
					ActivateOutput(pActInfo, EOP_True, true);
				}
				else if (entity == NULL)
				{
					ActivateOutput(pActInfo, EOP_False, true);
				}
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
		return new CFlowNode_GetEntityIdByName(pActInfo);
	}
};

class CFlowNode_EntityEnableDisable : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Enable,
		EIP_Disable,
	};

	enum EOutputPorts
	{
		EOP_Enabled,
		EOP_Disabled,
		EOP_EntityId,
	};

	IEntityScriptProxy* pScriptProxy;

public:
	////////////////////////////////////////////////////
	CFlowNode_EntityEnableDisable(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EntityEnableDisable()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<bool>("Enable", _HELP("Enable input entity")),
			InputPortConfig<bool>("Disable", _HELP("Disable input entity")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Enabled", _HELP("Enable input entity")),
			OutputPortConfig<bool>("Disabled", _HELP("Disable input entity")),
			OutputPortConfig<EntityId>("EntityId", _HELP("Output entity id when active Enable/Disable ports")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Enable/Disable entity");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			IEntity* entity = pActInfo->pEntity;
			EntityId id = entity->GetId();
			pScriptProxy = (IEntityScriptProxy*)pActInfo->pEntity->GetProxy(ENTITY_PROXY_SCRIPT);

			if (IsPortActive(pActInfo, EIP_Enable))
			{
				if (entity)
				{
					if (pScriptProxy)
					{
						pScriptProxy->CallEvent("Enable");
						ActivateOutput(pActInfo, EOP_EntityId, id);
					}

					if (!entity->IsHidden())
						ActivateOutput(pActInfo, EOP_Enabled, true);
				}
			}
			else if (IsPortActive(pActInfo, EIP_Disable))
			{
				if (entity)
				{
					if (pScriptProxy)
					{
						pScriptProxy->CallEvent("Disable");
						ActivateOutput(pActInfo, EOP_EntityId, id);
					}

					if (entity->IsHidden())
						ActivateOutput(pActInfo, EOP_Disabled, true);
				}
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
		return new CFlowNode_EntityEnableDisable(pActInfo);
	}
};

class CFlowNode_EntityCheckName : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Name,
	};

	enum EOutputPorts
	{
		EOP_Result,
		EOP_True,
		EOP_False,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_EntityCheckName(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EntityCheckName()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("")),
			InputPortConfig<string>("Name", _HELP("Name to check")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Result", _HELP("")),
			OutputPortConfig_AnyType("True", _HELP("")),
			OutputPortConfig_AnyType("False", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;

		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Checks the name of the entity for the entered characters");
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
				const string& sSubName = GetPortString(pActInfo, EIP_Name);
				IEntity* entity = pActInfo->pEntity;

				if (entity)
				{
					bool result = false;

					if (strstr(entity->GetName(), sSubName) != 0)
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
		return new CFlowNode_EntityCheckName(pActInfo);
	}
};

class CFlowNode_PlayAISoundEvent : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Stop,
		EIP_Radius,
		EIP_EventType,
		//EIP_Constant,
	};

	enum EOutputPorts
	{
		EOP_Type,
	};

	float radius;
	int eventType;
	//bool bConstant;

public:
	////////////////////////////////////////////////////
	CFlowNode_PlayAISoundEvent(SActivationInfo* pActInfo)
	{
		eventType = 0;
		//bConstant = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_PlayAISoundEvent()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const char* uiConfig =
			"enum_int:Generic=0,Collision=1,Collision_Loud=2,Movement=3,Movement_Loud=4,Weapon=5,Explosion=6";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig_AnyType("Stop", _HELP("")),
			InputPortConfig<float>("Radius", 0.0f, _HELP("")),
			InputPortConfig<int>("Type", AISE_GENERIC, _HELP(""), 0, uiConfig),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Checks the name of the entity for the entered characters");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			radius = GetPortFloat(pActInfo, EIP_Radius);
			eventType = GetPortInt(pActInfo, EIP_EventType);
			//bConstant = false;
		}//end of case
		break;

		case eFE_Activate:
		{
			/*if (IsPortActive(pActInfo,EIP_Constant))
				bConstant = GetPortBool(pActInfo, EIP_Constant);*/

			if (IsPortActive(pActInfo, EIP_Radius))
				radius = GetPortFloat(pActInfo, EIP_Radius);

			if (IsPortActive(pActInfo, EIP_Start))
			{
				//if (bConstant)
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);

				if (pActInfo->pEntity->GetAI())
				{
					gEnv->pAISystem->SoundEvent(pActInfo->pEntity->GetWorldPos(), radius, EAISoundEventType(eventType), pActInfo->pEntity->GetAI());
				}
				else
					gEnv->pAISystem->SoundEvent(pActInfo->pEntity->GetWorldPos(), radius, EAISoundEventType(eventType), NULL);
			}
			if (IsPortActive(pActInfo, EIP_Stop))
			{
				//bConstant = false;
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}//end of case
		break;

		case eFE_Update:
		{
			radius = GetPortFloat(pActInfo, EIP_Radius);
			eventType = GetPortInt(pActInfo, EIP_EventType);

			if (pActInfo->pEntity->GetAI())
			{
				gEnv->pAISystem->SoundEvent(pActInfo->pEntity->GetWorldPos(), radius, EAISoundEventType(eventType), pActInfo->pEntity->GetAI());
			}

			else if (IsPortActive(pActInfo, EIP_Stop) || !pActInfo->pEntity->GetAI())
				gEnv->pAISystem->SoundEvent(pActInfo->pEntity->GetWorldPos(), radius, EAISoundEventType(eventType), NULL);
		}//end of case
		//break;
		}//end of switch
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_PlayAISoundEvent(pActInfo);
	}
};

class CFlowNode_UpdateTest : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Stop,
	};

	enum EOutputPorts
	{
		EOP_Result_Start,
		EOP_Result_Activate,
		EOP_Result_Update,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_UpdateTest(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_UpdateTest()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig_AnyType("Stop", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Result_Start",_HELP("")),
			OutputPortConfig<bool>("Result_Activate",_HELP("")),
			OutputPortConfig<bool>("Result_Update",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
		}//end of case
		break;

		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Start))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				ActivateOutput(pActInfo, EOP_Result_Start, true);
			}

			ActivateOutput(pActInfo, EOP_Result_Activate, true);
		}//end of case
		break;

		case eFE_Update:
		{
			if (IsPortActive(pActInfo, EIP_Stop))
			{
				ActivateOutput(pActInfo, EOP_Result_Update, false);
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
			else
				ActivateOutput(pActInfo, EOP_Result_Update, true);
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
		return new CFlowNode_UpdateTest(pActInfo);
	}
};

class CFlowNode_OnActionTest : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Press,
		EIP_Release,
	};

	enum EOutputPorts
	{
		EOP_Pressed,
		EOP_Released,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_OnActionTest(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_OnActionTest()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Press", _HELP("")),
			InputPortConfig_AnyType("Release", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Pressed",_HELP("")),
			OutputPortConfig_AnyType("Released",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
		}//end of case
		break;

		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
				return;

			EntityId pId = pActInfo->pEntity->GetId();
			CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pId));
			CPlayer* pPlayer = static_cast<CPlayer*>(pActor);
			CAlien* pAlien = static_cast<CAlien*>(pActor);

			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Press))
			{
				pAlien->OnAction("leanleft", eAAM_OnPress, 1.0f);
				ActivateOutput(pActInfo, EOP_Pressed, true);
			}

			if (IsPortActive(pActInfo, EIP_Release))
			{
				pAlien->OnAction("leanleft", eAAM_OnRelease, 1.0f);
				ActivateOutput(pActInfo, EOP_Released, true);
			}
		}//end of case
		break;

		case eFE_Update:
		{
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
		return new CFlowNode_OnActionTest(pActInfo);
	}
};

class CFlowNode_EC_SendItemAmmo : public CFlowBaseNode
{
	enum INPUTS
	{
		EIP_Start = 0,
		EIP_Stop,
	};

	enum OUTPUTS
	{
		EOP_Done = 0,
	};

public:
	CFlowNode_EC_SendItemAmmo(SActivationInfo* pActInfo)
	{
	}

	~CFlowNode_EC_SendItemAmmo()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_EC_SendItemAmmo(pActInfo);
	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig_AnyType("Stop", _HELP("")),
			{0}
		};
		static const SOutputPortConfig out_ports[] =
		{
			OutputPortConfig_AnyType("Done",  _HELP("")),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Invoke Overheat from entity");
		config.SetCategory(EFLN_WIP);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
		}// case end
		break;

		case eFE_Activate:
		{
			return;

			if (!pActInfo->pEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Start))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				EntityId pId = pActInfo->pEntity->GetId();
				IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pId);
				if (pItem)
				{
					//g_pGame->GetHUD()->controlItem = pItem;
				}
			}// case end
			break;

		case eFE_Update:
		{
			if (IsPortActive(pActInfo, EIP_Stop))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				//g_pGame->GetHUD()->controlItem = g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem(false);
			}
		}// case end
		break;
		} //switch end
		}
	};
};

class CFlowNode_EC_SendActor : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Stop,
		EIP_EntityId,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

	EntityId m_pEntityId;
	CActor* pActor;
	CActor* pClientActor;
	//bool bConstant;

public:
	////////////////////////////////////////////////////
	CFlowNode_EC_SendActor(SActivationInfo* pActInfo)
	{
		//bConstant = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EC_SendActor()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig_AnyType("Stop", _HELP("")),
			InputPortConfig<EntityId>("EntityId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_WIP);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
		}//end of case
		break;

		case eFE_Activate:
		{
			return;
			if (!g_pGame->GetHUD())
			{
				return;
			}

			if (IsPortActive(pActInfo, EIP_EntityId))
			{
				m_pEntityId = GetPortEntityId(pActInfo, EIP_EntityId);
			}

			if (IsPortActive(pActInfo, EIP_Start))
			{
				//pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				m_pEntityId = GetPortEntityId(pActInfo, EIP_EntityId);
				pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_pEntityId));
				if (pActor)
				{
					/*	if (!g_pGame->GetHUD()->bActorFromFG)
						{
							g_pGame->GetHUD()->bActorFromFG = true;
						}
						g_pGame->GetHUD()->controlActor = pActor;

						if (g_pGame->GetHUD()->controlActor != g_pGame->GetIGameFramework()->GetClientActor())
							ActivateOutput(pActInfo, EOP_Done, true);*/
				}
			}

			if (IsPortActive(pActInfo, EIP_Stop) || !g_pGame)
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				/*g_pGame->GetHUD()->controlActor = g_pGame->GetIGameFramework()->GetClientActor();
				if (g_pGame->GetHUD()->bActorFromFG)
				{
					g_pGame->GetHUD()->bActorFromFG = false;
				}*/
			}
		}//end of case
		break;

		case eFE_Update:
		{
			m_pEntityId = GetPortEntityId(pActInfo, EIP_EntityId);
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
		return new CFlowNode_EC_SendActor(pActInfo);
	}
};

class CFlowNode_GetMaterial : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_Material,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetMaterial(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetMaterial(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<string>("Material", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
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
				if (IEntity* pGraphEntity = pActInfo->pEntity)
				{
					SEntitySlotInfo info;
					pGraphEntity->GetSlotInfo(0, info);

					IMaterial* pMat(NULL);

					if (info.pCharacter)
						pMat = info.pCharacter->GetMaterial();

					ActivateOutput(pActInfo, EOP_Material, pMat ? (string)pMat->GetName() : "None");
				}
			}
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
		return new CFlowNode_GetMaterial(pActInfo);
	}
};

class CFlowNode_GetMaxDamage : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Component
	};

	enum EOutputPorts
	{
		EOP_Damage,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetMaxDamage(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetMaxDamage(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("")),
			InputPortConfig<string>("Component", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Damage", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
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
				if (IVehicle* pVehicle = pActInfo->pEntity ? g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId()) : 0)
				{
					string vehComponent = GetPortString(pActInfo, EIP_Component);
					IVehicleComponent* pComponent = pVehicle->GetComponent(vehComponent);

					if (pComponent)
						ActivateOutput(pActInfo, EOP_Damage, pComponent->GetMaxDamage());
				}
			}
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
		return new CFlowNode_GetMaxDamage(pActInfo);
	}
};

class CFlowNode_VisionParams : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Draw,
		EIP_Hide,
		EIP_Color,
		EIP_Alpha
	};

	enum EOutputPorts
	{
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VisionParams(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VisionParams(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Draw", _HELP("")),
			InputPortConfig_AnyType("Hide", _HELP("")),
			InputPortConfig<Vec3>("Color",Vec3(0,0,0) , _HELP("")),
			InputPortConfig<float>("Alpha",0.0f , _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Draw))
			{
				if (IEntity* pEntity = pActInfo->pEntity)
				{
					IEntityRenderProxy* pEntityRenderProxy = static_cast<IEntityRenderProxy*>(pEntity->GetProxy(ENTITY_PROXY_RENDER));
					if (!pEntityRenderProxy)
						return;

					Vec3 color = GetPortVec3(pActInfo, EIP_Color);
					float  fAlpha = GetPortFloat(pActInfo, EIP_Alpha);

					pEntityRenderProxy->SetVisionParams(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
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
		return new CFlowNode_VisionParams(pActInfo);
	}
};

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
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_StringCheck()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

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

REGISTER_FLOW_NODE("Math:SetInteger", CFlowNode_SetInteger);
REGISTER_FLOW_NODE("Math:Round", CFlowNode_Round);
REGISTER_FLOW_NODE("Math:Calculate", CFlowNode_Calculate);
REGISTER_FLOW_NODE("Math:InRange", CFlowNode_InRange);

REGISTER_FLOW_NODE("AI:AISelectGoalPipe", CFlowNode_AISelectGoalPipe);
REGISTER_FLOW_NODE("AI:AIIsUsingPipe", CFlowNode_AIIsUsingPipe);

REGISTER_FLOW_NODE("Entity:GetEntityByName", CFlowNode_GetEntityIdByName); //
REGISTER_FLOW_NODE("Entity:EnableDisable", CFlowNode_EntityEnableDisable); //
REGISTER_FLOW_NODE("Entity:CheckName", CFlowNode_EntityCheckName); //
REGISTER_FLOW_NODE("Entity:RemoveEntity", CFlowNode_RemoveEntity);
REGISTER_FLOW_NODE("Entity:GetOtherData", CFlowNode_GetOtherData);
REGISTER_FLOW_NODE("Entity:CheckDistance", CFlowNode_EntityCheckDistance);
REGISTER_FLOW_NODE("Entity:CallScriptFunction", CFlowNode_CallScriptFunction);
REGISTER_FLOW_NODE("Entity:SwitchView", CFlowNode_SwitchView);//
REGISTER_FLOW_NODE("Entity:GetBounds", CFlowNode_GetEntityBounds);//

//REGISTER_FLOW_NODE("EntityControl:AlienMovement", CFlowNode_EC_AlienMovement);
//REGISTER_FLOW_NODE("EntityControl:GetLookEntityId", CFlowNode_EC_GetIdOfLook);
//REGISTER_FLOW_NODE("EntityControl:GetOnGround", CFlowNode_EC_GetOnGround);
//REGISTER_FLOW_NODE("EntityControl:SendActor", CFlowNode_EC_SendActor);
//REGISTER_FLOW_NODE("EntityControl:SendItemAmmo", CFlowNode_EC_SendItemAmmo);

REGISTER_FLOW_NODE("Game:ActorGetMaxHealth", CFlowNode_GetMaxHealth); //
REGISTER_FLOW_NODE("Game:ActorGetItemId", CFlowNode_GetActorItemId);

//REGISTER_FLOW_NODE("Weapon:Host", CFlowNode_SetWeaponHost); //

REGISTER_FLOW_NODE("Sound:PlayAISoundEvent", CFlowNode_PlayAISoundEvent);

REGISTER_FLOW_NODE("Material:GetMaterial", CFlowNode_GetMaterial);

REGISTER_FLOW_NODE("Vehicle:GetMaxDamage", CFlowNode_GetMaxDamage);
REGISTER_FLOW_NODE("Render:VisionParams", CFlowNode_VisionParams);

REGISTER_FLOW_NODE("String:CheckStrings", CFlowNode_StringCheck);