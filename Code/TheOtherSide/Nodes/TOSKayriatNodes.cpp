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

#include "VehicleMovementBase.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Conqueror/ConquerorSpeciesClass.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Helpers/TOS_Script.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "TheOtherSide/Helpers/TOS_AI.h"
#include "TheOtherSide/AI Files/IAIActionTrackerListener.h"
#include "IAgent.h"

namespace
{
	// to be replaced. hud is no longer a game object
	void SendHUDEvent(const SGameObjectEvent& evt)
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
			const float inpVal = GetPortFloat(pActInfo, 0);
			const int out = int_round(inpVal);
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
	enum EInputPorts
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
				const float a = GetPortFloat(pActInfo, INP_A);
				const float b = GetPortFloat(pActInfo, INP_B);
				const int op = GetPortInt(pActInfo, INP_Operation);
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

	enum EInputPorts
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
			const float in = GetPortFloat(pActInfo, INP_In);
			const float v_min = GetPortFloat(pActInfo, INP_Min);
			const float v_max = GetPortFloat(pActInfo, INP_Max);
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
				const EntityId rEntity = GetPortEntityId(pActInfo, EIP_ID); // �������� ������ � ������

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

class CFlowNode_AIGetSpecies : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sink,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_SpeciesNum,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AIGetSpecies(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIGetSpecies(void)
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
			InputPortConfig_AnyType("Sync", _HELP("Trigger to start")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done", _HELP("Job is done")),
			OutputPortConfig<int>("Species", _HELP("Get a species from ai")),
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
				const IAIActor* pFlowAIActor = pFlowAI->CastToIAIActor(); // Get interface of ai actor
				const int DoneSpecies = pFlowAIActor->GetParameters().m_nSpecies;
				/////////////////////////////////////////////////////////////////////
				/////////////////////////////////////////////////////////////////////
				ActivateOutput(pActInfo, EOP_Done, true);
				ActivateOutput(pActInfo, EOP_SpeciesNum, DoneSpecies);
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
		return new CFlowNode_AIGetSpecies(pActInfo);
	}
};

class CFlowNode_AISelectGoalPipe : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger = 0,
		EIP_Goal,
		EIP_Arg,
		EIP_ArgPos,
		EIP_Update,
		EIP_SubPipe,
		EIP_GoalFlag,
	};

	enum EOutputPorts
	{
		EOP_Done = 0,
	};

	EntityId argId = 0;

	Vec3 argPos = Vec3(0);
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
		const char* ui_config = "enum_int:Loop=0,RunOnce=1,NotDuplicate=2,HighPriority=4,SamePriority=8,DontResetAG=16,KeepLastSubpipe=32";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("Trigger to start")),
			InputPortConfig<string>("GoalPipeName", _HELP("Write a goal pipe name. You can find them in Game\Scripts\AI\GoalPipes")),
			InputPortConfig<EntityId>("TargetEntity", _HELP("")),
			InputPortConfig<Vec3>("TargetPos", _HELP("")),
			InputPortConfig<bool>("Update",0, _HELP("")),
			InputPortConfig<bool>("SubPipe",0, _HELP("")),
			InputPortConfig<int>("GoalFlag",0, _HELP(""), "GoalFlag", ui_config),

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
				const string GoalPipe = GetPortString(pActInfo, EIP_Goal);
				argId = GetPortEntityId(pActInfo, EIP_Arg);
				argPos = GetPortVec3(pActInfo, EIP_ArgPos);

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
						else if (argPos != Vec3(0))
						{
							pActor->SetRefPointPos(argPos);
						}
						else
							pArgEntity = NULL;

						if (pArgEntity)
							ArgAIObject = pArgEntity->GetAI();
						else
							ArgAIObject = NULL;

						if (pActor)
						{
							const int goalFlag = GetPortInt(pActInfo, EIP_GoalFlag);
							const bool bSubPipe = GetPortBool(pActInfo, EIP_SubPipe);
							bSubPipe ? pActor->InsertSubPipe(goalFlag, GoalPipe, ArgAIObject) : pActor->SelectPipe(goalFlag, GoalPipe, ArgAIObject);

							const bool bActivateUpdate = GetPortBool(pActInfo, EIP_Update);
							pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, bActivateUpdate);

							ActivateOutput(pActInfo, EOP_Done, true);
						}
					}
				}
			}

			if (IsPortActive(pActInfo, EIP_Update))
			{
				const bool bActivateUpdate = GetPortBool(pActInfo, EIP_Update);
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
			else
			{
				if (pActor && argPos != Vec3(0))
					pActor->SetRefPointPos(argPos);
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
		EIP_Sync = 0,
		EIP_GoalPipe,
		EIP_GoalPipeId ,
	};

	enum EOutputPorts
	{
		EOP_Using = 0,
		EOP_True,
		EOP_False,
	};
	IPipeUser* pPipeUser;

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
			InputPortConfig_AnyType("Trigger", _HELP(""), "Sync"),
			InputPortConfig<string>("GoalPipe", string(""), _HELP("Goalpipe name, or subpipe name")),
			InputPortConfig<int>("GoalPipeId", 0, _HELP("Goalpipe id or AIAction id")),
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
		config.sDescription = _HELP("Checks if the AI is using a specific pipe. Doesn't show AIAсtion usage by name, only by id, before the subpipe was inserted");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Sync))
			{
				const string sGoalPipe = GetPortString(pActInfo, EIP_GoalPipe);

				if (pActInfo->pEntity)
				{
					IAIObject* pActorAI = pActInfo->pEntity->GetAI();

					if (pActorAI)
					{
						pPipeUser = pActorAI->CastToIPipeUser();
						if (pPipeUser)
						{
							const auto pipeId = GetPortInt(pActInfo, EIP_GoalPipeId);

							if (pipeId != 0)
							{
								const bool bIsUsing = pPipeUser->GetGoalPipeId() == pipeId;
								ActivateOutput(pActInfo, EOP_Using, bIsUsing);
								ActivateOutput(pActInfo, bIsUsing ? EOP_True : EOP_False, 1);
							}
							else
							{
								const bool bIsUsing = pPipeUser->IsUsingPipe(sGoalPipe.c_str());
								ActivateOutput(pActInfo, EOP_Using, bIsUsing);
								ActivateOutput(pActInfo, bIsUsing ? EOP_True : EOP_False, 1);
							}
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

		const IEntity* pEntityNode = pActInfo->pEntity;
		if (!pEntityNode)
			return;

		const float minRangeDist = GetPortFloat(pActInfo, INP_MIN_DIST);
		const float maxRangeDist = GetPortFloat(pActInfo, INP_MAX_DIST);
		const float minRangeDist2 = minRangeDist * minRangeDist;
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
			const EntityId entityIdCheck = GetPortEntityId(pActInfo, i);
			const IEntity* pEntityCheck = gEnv->pEntitySystem->GetEntity(entityIdCheck);
			if (pEntityCheck)
			{
				const float dist2 = pEntityCheck->GetWorldPos().GetSquaredDistance(pEntityNode->GetWorldPos());
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
				const IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					const EntityId pID = pFlowEntity->GetId();
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
				const IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					const EntityId pID = pFlowEntity->GetId();
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
	};

	enum EOutputPorts
	{
		EOP_Center,
		EOP_Size,
		EOP_Radius,
		EOP_Volume,
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
			//InputPortConfig_AnyType("Get", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<Vec3>("Center", _HELP("")),
			OutputPortConfig<Vec3>("Size", _HELP("")),
			OutputPortConfig<float>("Radius", _HELP("")),
			OutputPortConfig<float>("Volume", _HELP("")),
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
		case eFE_SetEntityId:
		{
			const IEntity* pFlowEntity = pActInfo->pEntity;
			if (pFlowEntity)
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
			}
			else
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case eFE_Update:
		{
			IEntity* pFlowEntity = pActInfo->pEntity;
			if (pFlowEntity)
			{
				AABB bounds;
				pFlowEntity->GetWorldBounds(bounds);
				ActivateOutput(pActInfo, EOP_Center, bounds.GetCenter());
				ActivateOutput(pActInfo, EOP_Size, bounds.GetSize());
				ActivateOutput(pActInfo, EOP_Radius, bounds.GetRadius());
				ActivateOutput(pActInfo, EOP_Volume, bounds.GetVolume());
			}
		}
		break;
		case eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
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
				const IEntity* entity = pActInfo->pEntity;
				if (entity)
				{
					const EntityId id = entity->GetId();
					if (id)
					{
						actor = dynamic_cast<CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
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
				const EntityId lookId = actor->GetGameObject()->GetWorldQuery()->GetLookAtEntityId();
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
				if (strClassName == "Trooper" || strClassName == "Hunter" || strClassName == "Scout") //this condition fix crash
				{
					pAlien = dynamic_cast<CAlien*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_entityId));
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
			const IGameFramework* pGame = g_pGame->GetIGameFramework();
			if (pGame)
			{
				if (IsPortActive(pActInfo, EIP_Get && pActInfo->pEntity))
				{
					const IEntity* pEntity = pActInfo->pEntity;
					if (pEntity)
					{
						const EntityId pEntityID = pEntity->GetId();
						if (pEntityID)
						{
							CActor* pActor = dynamic_cast<CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityID));
							if (pActor)
							{
								const float onGroundValue = pActor->GetActorStats()->onGround;
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
		config.sDescription = _HELP("When input [Get] is triggered output [MaxHealth] will return max health of [Attached Entity]");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const IGameFramework* pGame = g_pGame->GetIGameFramework();
			if (pGame)
			{
				if (IsPortActive(pActInfo, EIP_Trigger && pActInfo->pEntity))
				{
					const IEntity* pEntity = pActInfo->pEntity;
					if (pEntity)
					{
						const EntityId pEntityID = pEntity->GetId();
						if (pEntityID)
						{
							const CActor* pActor = dynamic_cast<CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityID));
							if (pActor)
							{
								const int iMaxHealth = pActor->GetMaxHealth();
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

// class CFlowNode_SetWeaponHost : public CFlowBaseNode
// {
// 	enum EInputPorts
// 	{
// 		EIP_SetHost,
// 		EIP_GetHost,
// 		EIP_HostID,
// 	};
//
// 	enum EOutputPorts
// 	{
// 		EOP_Done,
// 		EOP_GetHost,
// 	};
//
// 	EntityId m_hostId;
// 	CItem* pItem;
// 	IWeapon* pWeapon;
//
// public:
// 	////////////////////////////////////////////////////
// 	CFlowNode_SetWeaponHost(SActivationInfo* pActInfo)
// 	{
// 		m_hostId = 0;
// 		pItem = nullptr;
// 		pWeapon = nullptr;
// 	}
//
// 	////////////////////////////////////////////////////
// 	virtual ~CFlowNode_SetWeaponHost()
// 	{
// 	}
//
// 	////////////////////////////////////////////////////
// 	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
// 	{
// 	}
//
// 	////////////////////////////////////////////////////
// 	virtual void GetConfiguration(SFlowNodeConfig& config)
// 	{
// 		static const SInputPortConfig inputs[] =
// 		{
// 			InputPortConfig_AnyType("SetHost",	_HELP("")),
// 			InputPortConfig_AnyType("GetHost",	_HELP("")),
// 			InputPortConfig<EntityId>("HostId",	_HELP("")),
// 			{0}
// 		};
//
// 		static const SOutputPortConfig outputs[] =
// 		{
// 			OutputPortConfig_AnyType("Done", _HELP("")),
// 			OutputPortConfig<EntityId>("HostId", _HELP("")),
// 			{0}
// 		};
//
// 		config.nFlags |= EFLN_TARGET_ENTITY;
// 		config.pInputPorts = inputs;
// 		config.pOutputPorts = outputs;
// 		config.sDescription = _HELP("Set host of input weapon");
// 		config.SetCategory(EFLN_APPROVED);
// 	}
//
// 	////////////////////////////////////////////////////
// 	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
// 	{
// 		switch (event)
// 		{
// 		case eFE_Initialize:
// 		{
// 			m_hostId = 0;
// 		}
// 		break;
//
// 		case eFE_Activate:
// 		{
// 			const IGameFramework* pGame = g_pGame->GetIGameFramework();
// 			if (pGame)
// 			{
// 				if (IsPortActive(pActInfo, EIP_HostID))
// 				{
// 					m_hostId = GetPortEntityId(pActInfo, EIP_HostID);
// 				}
//
// 				if (IsPortActive(pActInfo, EIP_SetHost && pActInfo->pEntity))
// 				{
// 					if (pActInfo->pEntity->GetId())
// 					{
// 						if (m_hostId == 0)
// 							return;
//
// 						pItem = dynamic_cast<CItem*>(g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pActInfo->pEntity->GetId()));
// 						pWeapon = pItem->GetIWeapon();
// 						auto m_pAlien = dynamic_cast<CAlien*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_hostId));
//
// 						if (pWeapon)
// 						{
// 							//pItem->SetOwnerId(m_hostId);
// 							//pItem->EnableSound(false);
//
// 							pWeapon->SetHostId(m_hostId);
//
// 							ActivateOutput(pActInfo, EOP_Done, true);
// 						}
// 					}
// 				}
// 				if (IsPortActive(pActInfo, EIP_GetHost && pActInfo->pEntity))
// 				{
// 					const IEntity* pEntity = pActInfo->pEntity;
// 					if (pEntity)
// 					{
// 						const EntityId pEntityId = pEntity->GetId();
// 						if (pEntityId)
// 						{
// 							IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pEntityId);
//
// 							const IWeapon* pWeapon = pItem->GetIWeapon();
// 							if (pWeapon)
// 							{
// 								const EntityId HostID = pWeapon->GetHostId();
// 								ActivateOutput(pActInfo, EOP_GetHost, HostID);
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}//end of case
// 		break;
//
// 		/*case eFE_Update:
// 		{
// 			if (!m_bAiming)
// 			{
// 				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
// 			}
// 			else
// 				SetDestination(m_targetPos);
// 		}
// 		break;*/
// 		}//end of switch
// 	}//end of ProcessEvent
//
// 	////////////////////////////////////////////////////
// 	virtual void GetMemoryStatistics(ICrySizer* s)
// 	{
// 		s->Add(*this);
// 	}
//
// 	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
// 	{
// 		return new CFlowNode_SetWeaponHost(pActInfo);
// 	}
// };

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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						const IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId);
						if (pActor && pActor->GetHealth() > 0)
						{
							const IInventory* pInventory = pActor->GetInventory();
							if (!pInventory)
								return;
							const EntityId itemId = pInventory->GetCurrentItem();
							if (itemId == 0)
								return;
							const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
							if (pItem == 0)
								return;
							const EntityId pNewItemId = pItem->GetEntityId();
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
			const IEntitySystem* pSystem = gEnv->pEntitySystem;
			if (IsPortActive(pActInfo, EIP_Get))
			{
				entity = pSystem->FindEntityByName(name);
				if (entity)
				{
					const EntityId id = entity->GetId();

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
			const IEntity* entity = pActInfo->pEntity;
			const EntityId id = entity->GetId();
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
				const IEntity* entity = pActInfo->pEntity;

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

			const EntityId pId = pActInfo->pEntity->GetId();
			CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pId));
			CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActor);
			CAlien* pAlien = dynamic_cast<CAlien*>(pActor);

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
				pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_pEntityId));
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
				if (const IEntity* pGraphEntity = pActInfo->pEntity)
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

class CFlowNode_VehicleGetMaxDamage : public CFlowBaseNode
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
	CFlowNode_VehicleGetMaxDamage(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleGetMaxDamage(void)
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
					const string vehComponent = GetPortString(pActInfo, EIP_Component);
					const IVehicleComponent* pComponent = pVehicle->GetComponent(vehComponent);

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
		return new CFlowNode_VehicleGetMaxDamage(pActInfo);
	}
};

class CFlowNode_VehicleGetFreeSeat : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_IsGunner,
	};

	enum EOutputPorts
	{
		EOP_SeatId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleGetFreeSeat(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleGetFreeSeat(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<bool>("IsGunner", _HELP("Get only free seats which have gun")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get free seat of vehicle");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto* pVehicle = pActInfo->pEntity ?
					g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId()) : 0;

				if (pVehicle)
				{
					const auto isGunner = GetPortBool(pActInfo, EIP_IsGunner);
					const int index = isGunner ? TOS_Vehicle::RequestGunnerSeatIndex(pVehicle) : TOS_Vehicle::RequestFreeSeatIndex(pVehicle);

					ActivateOutput(pActInfo, EOP_SeatId, index);
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
		return new CFlowNode_VehicleGetFreeSeat(pActInfo);
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
				if (const IEntity* pEntity = pActInfo->pEntity)
				{
					IEntityRenderProxy* pEntityRenderProxy = dynamic_cast<IEntityRenderProxy*>(pEntity->GetProxy(ENTITY_PROXY_RENDER));
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

class CFlowNode_ActorLowerWeapon : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Enable,
		EIP_Disable,
	};

	enum EOutputPorts
	{
		EOP_Enabled,
		EOP_Disabled
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_ActorLowerWeapon(SActivationInfo* pActInfo)
	{
		m_pCurrentActor = 0;
		m_currentItemId = 0;
		m_activateItemId = 0;
		m_activateToLower = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorLowerWeapon()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	CPlayer* m_pCurrentActor;
	EntityId m_currentItemId;
	EntityId m_activateItemId;
	bool m_activateToLower;

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Enable", _HELP("")),
			InputPortConfig_AnyType("Disable", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Enabled", _HELP("")),
			OutputPortConfig_AnyType("Disabled", _HELP("")),
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
		case eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		}
		case eFE_SetEntityId:
		{
			if (pActInfo->pEntity)
			{
				m_pCurrentActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId()));
			}
			break;
		}
		case eFE_Update:
		{
			if (m_pCurrentActor)
			{
				m_currentItemId = m_pCurrentActor->GetCurrentItemId(false);
				if (m_currentItemId != m_activateItemId)
				{
					OnOldItem(pActInfo, m_activateItemId);
					m_activateItemId = m_currentItemId;
				}
				else
				{
					OnNewItem(pActInfo, m_currentItemId);
				}
			}
				
			break;
		}
		case eFE_Activate:
		{
			if (m_pCurrentActor)
			{
				if (IsPortActive(pActInfo, EIP_Enable))
					m_activateToLower = true;
				if (IsPortActive(pActInfo, EIP_Disable))
					m_activateToLower = false;

				m_activateItemId = m_pCurrentActor->GetCurrentItemId(false);	

				OnNewItem(pActInfo, m_activateItemId);
				ActivateOutput(pActInfo, m_activateToLower ? EIP_Enable : EIP_Disable, 1);

				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, m_activateToLower);
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
		return new CFlowNode_ActorLowerWeapon(pActInfo);
	}

	void OnNewItem(SActivationInfo* pActInfo, EntityId itemId)
	{
		if (!m_pCurrentActor)
			return;

		CWeapon* pWeapon = m_pCurrentActor->GetWeapon(itemId);
		if (pWeapon)
		{
			if (SPlayerStats* pStats = static_cast<SPlayerStats*>(m_pCurrentActor->GetActorStats()))
			{
				pStats->bForceLowerWeapon = m_activateToLower;
				pWeapon->StopFire();
				pWeapon->SetBusy(m_activateToLower);
			}
		}
	}

	void OnOldItem(SActivationInfo* pActInfo, EntityId itemId)
	{
		if (!m_pCurrentActor)
			return;

		CItem* pItem= m_pCurrentActor->GetItem(itemId);
		if (pItem)
			pItem->SetBusy(false);
	}
};

class CFlowNode_ActorSetModel : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Sync,
		EIP_Model,
		EIP_fp3pModel,
		EIP_ArmsModel,
		EIP_Material,
		EIP_ArmsMaterial,
		EIP_HelmetMaterial,
	};
	enum EOutputs
	{
		EOP_Done,
	};


public:
	bool m_modelHasChanged;

	////////////////////////////////////////////////////
	CFlowNode_ActorSetModel(SActivationInfo* pActInfo)
	{
		m_modelHasChanged = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorSetModel()
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<string>("obj_Model", _HELP("Full path to multiplayer thirdperson model"),"obj_Model"),
			InputPortConfig<string>("obj_fp3pModel", _HELP("Full path to fp3p model"),"obj_fp3pModel"),
			InputPortConfig<string>("obj_ArmsModel", _HELP("Full path to arms model"),"obj_ArmsModel"),
			InputPortConfig<string>("Material", _HELP("Full path to material"),"Material"),
			InputPortConfig<string>("ArmsMaterial", _HELP("Full path to arms material"),"ArmsMaterial"),
			InputPortConfig<string>("HelmetMaterial", _HELP("Full path to helmet material"),"HelmetMaterial"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("Done", _HELP("Outputs the id of the input entity")),
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
			if (IsPortActive(pActInfo, EIP_Sync))
			{
				const auto pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					const auto pGraphActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
					if (pGraphActor)
					{
						SClassModel modelInfo;
						modelInfo.m_character = GetPortString(pActInfo, EIP_Model);
						modelInfo.m_fp3p = GetPortString(pActInfo, EIP_fp3pModel);
						modelInfo.m_arms = GetPortString(pActInfo, EIP_ArmsModel);
						modelInfo.m_mat = GetPortString(pActInfo, EIP_Material);
						modelInfo.m_helmetMat = GetPortString(pActInfo, EIP_HelmetMaterial);
						modelInfo.m_armsMat = GetPortString(pActInfo, EIP_ArmsMaterial);

						if (g_pGame->GetHUD())
							g_pGame->GetHUD()->ActorRevive(pGraphActor);

						pGraphActor->SetHealth(pGraphActor->GetMaxHealth());
						pGraphActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);
						pGraphActor->Revive(false);

						if (pGraphActor->IsClient())
						{
							pGraphActor->SupressViewBlending(); // no view bleding when respawning // CActor::Revive resets it.
							if (g_pGame->GetHUD())
								g_pGame->GetHUD()->GetRadar()->Reset();
						}

						if (g_pControlSystem->GetConquerorSystem())
						{
							g_pControlSystem->GetConquerorSystem()->SetPlayerModel(pGraphActor, modelInfo);
							g_pControlSystem->GetConquerorSystem()->SetPlayerMaterial(pGraphActor, modelInfo);
							pGraphActor->SetCustomModelInfo(modelInfo);
						}

						ActivateOutput(pActInfo, EOP_Done, pGraphEntity->GetId());

					}
				}
			}

			break;
		}
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorSetModel(pActInfo);
	}
};

class CFlowNode_ActorHumanMode : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Enable,
		EIP_Disable,
	};
	enum EOutputs
	{
		EOP_Enabled,
		EOP_Disabled,
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_ActorHumanMode(SActivationInfo* pActInfo)
	{

	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorHumanMode()
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
			InputPortConfig_AnyType("Enable"),
			InputPortConfig_AnyType("Disable"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Enabled"),
			OutputPortConfig<int>("Disabled"),
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
			const auto pGraphEntity = pActInfo->pEntity;
			if (pGraphEntity)
			{
				const auto pGraphActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
				if (pGraphActor)
				{
					if (IsPortActive(pActInfo,EIP_Enable))
					{
						pGraphActor->TurnOnHumanMode();

						if (pGraphActor->IsHumanMode())
							ActivateOutput(pActInfo, EOP_Enabled, 1);
					}
					else if (IsPortActive(pActInfo, EIP_Disable))
					{
						pGraphActor->TurnOFFHumanMode();

						if (!pGraphActor->IsHumanMode())
							ActivateOutput(pActInfo, EOP_Disabled, 1);
					}
				}
			}

			break;
		}
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorHumanMode(pActInfo);
	}
};

class CFlowNode_ActorOnlyTPMode : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Enable,
		EIP_Disable,
	};
	enum EOutputs
	{
		EOP_Enabled,
		EOP_Disabled,
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_ActorOnlyTPMode(SActivationInfo* pActInfo)
	{

	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorOnlyTPMode()
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
			InputPortConfig_AnyType("Enable"),
			InputPortConfig_AnyType("Disable"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Enabled"),
			OutputPortConfig<int>("Disabled"),
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
			const auto pGraphEntity = pActInfo->pEntity;
			if (pGraphEntity)
			{
				const auto pGraphActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
				if (pGraphActor)
				{
					if (IsPortActive(pActInfo, EIP_Enable))
					{
						pGraphActor->TurnOnOnlyThirdPerson();

						if (pGraphActor->IsOnlyThirdPerson())
							ActivateOutput(pActInfo, EOP_Enabled, 1);
					}
					else if (IsPortActive(pActInfo, EIP_Disable))
					{
						pGraphActor->TurnOFFOnlyThirdPerson();

						if (!pGraphActor->IsOnlyThirdPerson())
							ActivateOutput(pActInfo, EOP_Disabled, 1);
					}
				}
			}

			break;
		}
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorOnlyTPMode(pActInfo);
	}
};

class CFlowNode_ActorKill : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Trigger,
	};
	enum EOutputs
	{
		EOP_Killed,
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_ActorKill(SActivationInfo* pActInfo)
	{

	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorKill()
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
			InputPortConfig_AnyType("Trigger"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Killed"),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Kill the input actor");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const auto pGraphEntity = pActInfo->pEntity;
			if (pGraphEntity)
			{
				const auto pGraphActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
				if (pGraphActor)
				{
					if (IsPortActive(pActInfo, EIP_Trigger))
					{
						IScriptTable* pTable = pGraphActor->GetEntity()->GetScriptTable();
						if (pTable)
							Script::CallMethod(pTable, "Event_Kill");

						if (pGraphActor->GetHealth() < 0)
							ActivateOutput(pActInfo, EOP_Killed, 1);
					}
				}
			}

			break;
		}
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorKill(pActInfo);
	}
};

//class CFlowNode_GetPosFromMap : public CFlowBaseNode
//{
//	enum EInputs
//	{
//		EIP_Trigger,
//		EIP_MapPos,
//	};
//	enum EOutputs
//	{
//		EOP_Pos,
//	};
//
//
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_GetPosFromMap(SActivationInfo* pActInfo)
//	{
//
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_GetPosFromMap()
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetConfiguration(SFlowNodeConfig& config)
//	{
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Trigger"),
//			InputPortConfig<Vec3>("MapPos"),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<Vec3>("Pos"),
//			{0}
//		};
//
//		//config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("Map Pos 0..1");
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	////////////////////////////////////////////////////
//	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
//	{
//		switch (event)
//		{
//		case eFE_Activate:
//		{
//			if (IsPortActive(pActInfo, EIP_Trigger))
//			{
//				Vec3 mapPos = GetPortVec3(pActInfo, EIP_MapPos);
//				Vec3 pos = Vec3(0, 0, 0);
//				//g_pGame->GetHUD()->GetRadar()->GetWorldPosFromMap(mapPos.x, mapPos.y, pos.x, pos.y, true);
//
//				ActivateOutput(pActInfo, EOP_Pos, pos);
//			}
//			break;
//		}
//		}
//	}//end of ProcessEvent
//
//	////////////////////////////////////////////////////
//	virtual void GetMemoryStatistics(ICrySizer* s)
//	{
//		s->Add(*this);
//	}
//
//	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
//	{
//		return new CFlowNode_GetPosFromMap(pActInfo);
//	}
//};

class CFlowNode_NewFollowPath : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Trigger,
		EIP_PathName,
	};
	enum EOutputs
	{
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_NewFollowPath(SActivationInfo* pActInfo)
	{

	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_NewFollowPath()
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
			InputPortConfig_AnyType("Trigger"),
			InputPortConfig<string>("PathName"),
			{0}
		};

		//static const SOutputPortConfig outputs[] =
		//{
		//	OutputPortConfig_AnyType("Killed"),
		//	{0}
		//};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		//config.pOutputPorts = outputs;
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
			const auto pGraphEntity = pActInfo->pEntity;
			if (pGraphEntity)
			{
				const auto pGraphActor = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
				if (pGraphActor)
				{
					if (IsPortActive(pActInfo, EIP_Trigger))
					{
						const auto pathEntity = gEnv->pEntitySystem->FindEntityByName(GetPortString(pActInfo, EIP_PathName));

						const auto pSignalData = gEnv->pAISystem->CreateSignalExtraData();
						pSignalData->iValue = pathEntity ? pathEntity->GetId() : 0;

						//start nearest
						pSignalData->point.z = 1;

						//loops
						pSignalData->fValue = 9000;

						gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "ACT_FOLLOWPATH", pGraphActor->GetEntity()->GetAI(), pSignalData);

					}
				}
			}

			break;
		}
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_NewFollowPath(pActInfo);
	}
};

class CFlowNode_VehicleLightsOn : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_TurnOn,
		EIP_TurnOff,
	};

	enum EOutputPorts
	{
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleLightsOn(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleLightsOn(void)
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
			InputPortConfig_AnyType("TurnOn", _HELP("")),
			InputPortConfig_AnyType("TurnOff", _HELP("")),
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
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_TurnOn))
			{
				auto* pVehicle = pActInfo->pEntity ?
					g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId()) : 0;

				if (pVehicle)
				{
					pVehicle->OnAction(eVAI_ToggleLights, eAAM_OnPress, 1, g_pGame->GetIGameFramework()->GetClientActorId());
				}
			}
			else if (IsPortActive(pActInfo, EIP_TurnOff))
			{
				auto* pVehicle = pActInfo->pEntity ?
					g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId()) : 0;

				if (pVehicle)
				{
					pVehicle->OnAction(eVAI_ToggleLights, eAAM_OnPress, 0, g_pGame->GetIGameFramework()->GetClientActorId());
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
		return new CFlowNode_VehicleLightsOn(pActInfo);
	}
};

class CFlowNode_VehicleDebugDraw : public CFlowBaseNode
{

public:

	enum EInputs
	{
		IN_SHOW,
		IN_SIZE,
		IN_PARTS,
	};

	enum EOutputs
	{

	};

	CFlowNode_VehicleDebugDraw(SActivationInfo* pActInfo)
	{

	};

	~CFlowNode_VehicleDebugDraw()
	{

	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleDebugDraw(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] =
		{
			InputPortConfig_Void("Trigger", _HELP("show debug informations on screen")),
			InputPortConfig<float>("Size", 1.5f, _HELP("font size")),
			InputPortConfig<string>("vehicleParts_Parts", _HELP("select vehicle parts"), 0, _UICONFIG("ref_entity=entityId")),
			{0}
		};

		static const SOutputPortConfig out_config[] =
		{
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_DEBUG);
	}

	string currentParam;
	string currentSetting;

	float column1;
	float column2;
	int loops;

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		float red[] = { 1,0,0,1 };
		float cyan[] = { 0,0.5,0.5,1 };

		IVehicleSystem* pVehicleSystem = NULL;
		IVehicle* pVehicle = NULL;

		switch (event)
		{
		case eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		}

		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
				return;

			pVehicleSystem = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem();
			pVehicle = pVehicleSystem->GetVehicle(pActInfo->pEntity->GetId());

			if (!pVehicleSystem || !pVehicle)
				return;

			const string givenString = GetPortString(pActInfo, IN_PARTS);
			currentParam = givenString.substr(0, givenString.find_first_of(":"));
			currentSetting = givenString.substr(givenString.find_first_of(":") + 1, (givenString.length() - givenString.find_first_of(":")));

			column1 = 10.f;
			column2 = 100.f;

			if (IsPortActive(pActInfo, IN_SHOW))
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);

			break;
		}

		case eFE_Update:
		{
			IRenderer* pRenderer = gEnv->pRenderer;

			pVehicleSystem = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem();
			pVehicle = pVehicleSystem->GetVehicle(pActInfo->pEntity->GetId());

			if (!pVehicleSystem || !pActInfo->pEntity || !pVehicle)
				return;

			pRenderer->Draw2dLabel(column1, 10, GetPortFloat(pActInfo, IN_SIZE) + 2.f, cyan, false, pActInfo->pEntity->GetName());

			if (currentParam == "Seats")
			{
				loops = 0;

				for (uint32 i = 0; i < pVehicle->GetSeatCount(); i++)
				{
					IVehicleSeat* currentSeat;

					if (currentSetting == "All")
					{
						currentSeat = pVehicle->GetSeatById(i + 1);
					}
					else
					{
						currentSeat = pVehicle->GetSeatById(pVehicle->GetSeatId(currentSetting));
						i = pVehicle->GetSeatCount() - 1;
					}

					loops += 1;

					// column 1
					string pMessage = ("%s:", currentSeat->GetSeatName());

					if (column2 < pMessage.size() * 8 * GetPortFloat(pActInfo, IN_SIZE))
						column2 = pMessage.size() * 8 * GetPortFloat(pActInfo, IN_SIZE);
					pRenderer->Draw2dLabel(column1, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessage);

					// column 2
					if (currentSeat->GetPassenger(true))
					{
						float color[] = { 0,0.5,0.5,1 };
						pMessage = ("- %s", gEnv->pEntitySystem->GetEntity(currentSeat->GetPassenger(true))->GetName());
						pRenderer->Draw2dLabel(column2, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessage);
					}
				}
			}

			else if (currentParam == "Wheels")
			{
				pRenderer->Draw2dLabel(column1, 50.f, GetPortFloat(pActInfo, IN_SIZE) + 1.f, red, false, "!");
			}

			else if (currentParam == "Weapons")
			{
				loops = 0;

				for (int i = 0; i < pVehicle->GetWeaponCount(); i++)
				{
					const IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
					IWeapon* currentWeapon;
					EntityId currentEntityId;
					IItem* pItem;

					if (currentSetting == "All")
					{
						currentEntityId = pVehicle->GetWeaponId(i + 1);
					}
					else
					{
						currentEntityId = gEnv->pEntitySystem->FindEntityByName(currentSetting)->GetId();
						i = pVehicle->GetWeaponCount() - 1;
					}

					if (!pItemSystem->GetItem(currentEntityId))
						return;

					pItem = pItemSystem->GetItem(currentEntityId);
					currentWeapon = pItem->GetIWeapon();

					loops += 1;


					// column 1
					string pMessageName = string().Format("%s", gEnv->pEntitySystem->GetEntity(currentEntityId)->GetName());
					pRenderer->Draw2dLabel(column1, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessageName);

					if (column2 < pMessageName.size() * 8 * GetPortFloat(pActInfo, IN_SIZE))
						column2 = pMessageName.size() * 8 * GetPortFloat(pActInfo, IN_SIZE);

					// column 2
					string pMessageValue = string().Format("seat: %s firemode: %i", pVehicle->GetWeaponParentSeat(currentEntityId)->GetSeatName(), currentWeapon->GetCurrentFireMode()).c_str();
					pRenderer->Draw2dLabel(column2, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessageValue);
				}
			}

			else if (currentParam == "Components")
			{
				loops = 0;

				for (int i = 0; i < pVehicle->GetComponentCount(); i++)
				{
					const IVehicleComponent* currentComponent = nullptr;

					if (currentSetting == "All")
					{
						currentComponent = pVehicle->GetComponent(i);
					}
					else
					{
						currentComponent = pVehicle->GetComponent(currentSetting);
						i = pVehicle->GetComponentCount() - 1;
					}

					if (!currentComponent)
						return;

					loops += 1;

					ColorF labelColor;
					labelColor = ColorF(currentComponent->GetDamageRatio(), (1.f - currentComponent->GetDamageRatio()), 0.f);

					// column 1
					string pMessageName = string().Format("%s", currentComponent->GetComponentName()).c_str();
					pRenderer->Draw2dLabel(column1, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessageName);

					if (column2 < pMessageName.size() * 8 * GetPortFloat(pActInfo, IN_SIZE))
						column2 = pMessageName.size() * 8 * GetPortFloat(pActInfo, IN_SIZE);

					// column 2
					string pMessageValue = string().Format("%5.2f (%3.2f)", currentComponent->GetDamageRatio() * currentComponent->GetMaxDamage(), currentComponent->GetDamageRatio()).c_str();
					pRenderer->Draw2dLabel(column2, (15 * (float(loops + 1)) * GetPortFloat(pActInfo, IN_SIZE)), GetPortFloat(pActInfo, IN_SIZE), cyan, false, pMessageValue);
				}
			}

			else
			{
				pRenderer->Draw2dLabel(column1, 50.f, GetPortFloat(pActInfo, IN_SIZE) + 1.f, red, false, "no param given");
			}
			break;
		}
		}
	};
};

class CFlowNode_DrawBox : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Show,
		EIP_Hide,
		EIP_Pos,
		EIP_Rot,
		EIP_Scale,
		EIP_EntityType1,
		EIP_EntityType2,
		EIP_EntityType3,
		EIP_EntityType4,
		EIP_EntityType5,
		EIP_RWIFlag1,
		EIP_RWIFlag2,
		EIP_RWIFlag3,
		EIP_RWIFlag4,
		EIP_CollType,
	};
	enum EOutputs
	{
		EOP_Intersection
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_DrawBox(SActivationInfo* pActInfo)
	{
		m_box = primitives::box();
		m_matrix34.SetIdentity();
		m_collType = 0;

		for (int i = 0; i < 5; i++)
		{
			m_entityType[i] = 0;

			if (i < 4)
				m_rwiFlags[i] = 0;
		}
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_DrawBox()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const auto uiConfig1 = "enum_int:All=287,Static=1,Rigid=4,Living=8,Independent=16";
		const auto uiConfig2 = "enum_int:StopAtPierceable=15, CollTypeAny=1024, CollTypeBit=16, IngoreTerrainHoles=32, IgnoreNonColliding=64, IgnoreBackFaces=128, IgnoreSolidBackFaces=256, SeparateImportantHits=512, ForcePierceableNonColl=4096, Queue=2048,";
		const auto uiConfig3 = "enum_int:Player=2, Vehicle=8, FoliageProxy=8192, Solid=4091, Explosion=4, Foliage=16, Derbis=32, Obstruct=16384";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Show"),
			InputPortConfig_AnyType("Hide"),
			InputPortConfig<Vec3>("Position"),
			InputPortConfig<Vec3>("Rotation"),
			InputPortConfig<Vec3>("Scale"),
			InputPortConfig<int>("EntityType1", 0, _HELP("Intersection Entity type"), "EntityType1", uiConfig1),
			InputPortConfig<int>("EntityType2", 0, _HELP("Intersection Entity type"), "EntityType2", uiConfig1),
			InputPortConfig<int>("EntityType3", 0, _HELP("Intersection Entity type"), "EntityType3", uiConfig1),
			InputPortConfig<int>("EntityType4", 0, _HELP("Intersection Entity type"), "EntityType4", uiConfig1),
			InputPortConfig<int>("EntityType5", 0, _HELP("Intersection Entity type"), "EntityType5", uiConfig1),
			InputPortConfig<int>("RWIFlags1", 0, _HELP("Intersection RWI Flags"), "RWIFlags1", uiConfig2),
			InputPortConfig<int>("RWIFlags2", 0, _HELP("Intersection RWI Flags"), "RWIFlags2", uiConfig2),
			InputPortConfig<int>("RWIFlags3", 0, _HELP("Intersection RWI Flags"), "RWIFlags3", uiConfig2),
			InputPortConfig<int>("RWIFlags4", 0, _HELP("Intersection RWI Flags"), "RWIFlags4", uiConfig2),
			InputPortConfig<int>("CollType", 0, _HELP("Intersection Coll Type"), "CollType", uiConfig3),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Intersection"),
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
		IRenderAuxGeom* pRAG = gEnv->pRenderer->GetIRenderAuxGeom();

		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Show))
			{
				const auto position = GetPortVec3(pActInfo, EIP_Pos);
				const auto rotation = GetPortVec3(pActInfo, EIP_Rot);
				const auto size = GetPortVec3(pActInfo, EIP_Scale);

				m_entityType[0] = GetPortInt(pActInfo, EIP_EntityType1);
				m_entityType[1] = GetPortInt(pActInfo, EIP_EntityType2);
				m_entityType[2] = GetPortInt(pActInfo, EIP_EntityType3);
				m_entityType[3] = GetPortInt(pActInfo, EIP_EntityType4);
				m_entityType[4] = GetPortInt(pActInfo, EIP_EntityType5);

				m_rwiFlags[0] = GetPortInt(pActInfo, EIP_RWIFlag1);
				m_rwiFlags[1] = GetPortInt(pActInfo, EIP_RWIFlag2);
				m_rwiFlags[2] = GetPortInt(pActInfo, EIP_RWIFlag3);
				m_rwiFlags[3] = GetPortInt(pActInfo, EIP_RWIFlag4);

				m_collType = GetPortInt(pActInfo, EIP_CollType);

				m_matrix34 = Matrix34::CreateTranslationMat(position);
				m_matrix34.SetRotationXYZ(Ang3(rotation), position);

				m_box.Basis = Matrix33(m_matrix34);
				m_box.center = position;
				m_box.size = size;
				m_box.bOriented = true;

				pRAG->SetRenderFlags(e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone);

				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
			}
			else if (IsPortActive(pActInfo, EIP_Hide))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
			break;
		case IFlowNode::eFE_Update:
		{
			OBB obb;
			obb.c = m_box.center;
			obb.m33 = m_box.Basis;
			obb.h = m_box.size;

			gEnv->pRenderer->GetIRenderAuxGeom()->DrawOBB(obb, ZERO, false, ColorF(1, 0, 0.4f, 0.f), eBBD_Extremes_Color_Encoded);

			const auto entTypes = m_entityType[0] | m_entityType[1] | m_entityType[2] | m_entityType[3] | m_entityType[4];
			auto rwiflags = 0;

			if (m_rwiFlags[0] == rwi_colltype_any)
			{
				rwiflags = m_rwiFlags[0] | m_rwiFlags[1] | m_rwiFlags[2] | m_rwiFlags[3];
			}
			else if (m_rwiFlags[0] == rwi_colltype_bit)
			{
				rwiflags = (m_collType << m_rwiFlags[0]) | m_rwiFlags[1] | m_rwiFlags[2] | m_rwiFlags[3];
			}

			geom_contact* pContact = 0;
			const float dist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(m_box.type, &m_box, ZERO, entTypes,
			                                                                    &pContact, 0, rwiflags);

			if (dist > 0.0001f)
			{
				ActivateOutput(pActInfo, EOP_Intersection, 1);
			}
		}
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			m_box = primitives::box();
			m_matrix34.SetIdentity();
		}
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_DrawBox(pActInfo);
	}

	primitives::box m_box;
	Matrix34 m_matrix34;
	int m_entityType[5];
	int m_rwiFlags[4];
	int m_collType;
};

class CFlowNode_DrawRay : public CFlowBaseNode
{
	enum EInputs
	{
		EIP_Show,
		EIP_Hide,
		EIP_Pos,
		EIP_Target,
		EIP_Len,
		EIP_EntityType1,
		EIP_EntityType2,
		EIP_EntityType3,
		EIP_EntityType4,
		EIP_EntityType5,
		EIP_RWIFlag1,
		EIP_RWIFlag2,
		EIP_RWIFlag3,
		EIP_RWIFlag4,
		EIP_CollType,
	};
	enum EOutputs
	{
		EOP_Intersection
	};


public:
	////////////////////////////////////////////////////
	CFlowNode_DrawRay(SActivationInfo* pActInfo)
	{
		m_rayhit = ray_hit();
		m_matrix34.SetIdentity();
		m_collType = 0;

		for (int i = 0; i < 5; i++)
		{
			m_entityType[i] = 0;

			if (i < 4)
				m_rwiFlags[i] = 0;
		}
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_DrawRay()
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const auto uiConfig1 = "enum_int:All=287,Static=1,Rigid=4,Living=8,Independent=16";
		const auto uiConfig2 = "enum_int:StopAtPierceable=15, CollTypeAny=1024, CollTypeBit=16, IngoreTerrainHoles=32, IgnoreNonColliding=64, IgnoreBackFaces=128, IgnoreSolidBackFaces=256, SeparateImportantHits=512, ForcePierceableNonColl=4096, Queue=2048,";
		const auto uiConfig3 = "enum_int:Player=2, Vehicle=8, FoliageProxy=8192, Solid=4091, Explosion=4, Foliage=16, Derbis=32, Obstruct=16384";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Show"),
			InputPortConfig_AnyType("Hide"),
			InputPortConfig<Vec3>("Position"),
			InputPortConfig<Vec3>("Target"),
			InputPortConfig<float>("Len"),
			InputPortConfig<int>("EntityType1", 0, _HELP("Intersection Entity type"), "EntityType1", uiConfig1),
			InputPortConfig<int>("EntityType2", 0, _HELP("Intersection Entity type"), "EntityType2", uiConfig1),
			InputPortConfig<int>("EntityType3", 0, _HELP("Intersection Entity type"), "EntityType3", uiConfig1),
			InputPortConfig<int>("EntityType4", 0, _HELP("Intersection Entity type"), "EntityType4", uiConfig1),
			InputPortConfig<int>("EntityType5", 0, _HELP("Intersection Entity type"), "EntityType5", uiConfig1),
			InputPortConfig<int>("RWIFlags1", 0, _HELP("Intersection RWI Flags"), "RWIFlags1", uiConfig2),
			InputPortConfig<int>("RWIFlags2", 0, _HELP("Intersection RWI Flags"), "RWIFlags2", uiConfig2),
			InputPortConfig<int>("RWIFlags3", 0, _HELP("Intersection RWI Flags"), "RWIFlags3", uiConfig2),
			InputPortConfig<int>("RWIFlags4", 0, _HELP("Intersection RWI Flags"), "RWIFlags4", uiConfig2),
			InputPortConfig<int>("CollType", 0, _HELP("Intersection Coll Type"), "CollType", uiConfig3),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Intersection"),
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
		IRenderAuxGeom* pRAG = gEnv->pRenderer->GetIRenderAuxGeom();

		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Show))
			{
				const auto position = GetPortVec3(pActInfo, EIP_Pos);
				const auto targetPosition = GetPortVec3(pActInfo, EIP_Target);
				const Vec3 dirToTarget = (targetPosition - position).GetNormalizedSafe();

				m_entityType[0] = GetPortInt(pActInfo, EIP_EntityType1);
				m_entityType[1] = GetPortInt(pActInfo, EIP_EntityType2);
				m_entityType[2] = GetPortInt(pActInfo, EIP_EntityType3);
				m_entityType[3] = GetPortInt(pActInfo, EIP_EntityType4);
				m_entityType[4] = GetPortInt(pActInfo, EIP_EntityType5);

				m_rwiFlags[0] = GetPortInt(pActInfo, EIP_RWIFlag1);
				m_rwiFlags[1] = GetPortInt(pActInfo, EIP_RWIFlag2);
				m_rwiFlags[2] = GetPortInt(pActInfo, EIP_RWIFlag3);
				m_rwiFlags[3] = GetPortInt(pActInfo, EIP_RWIFlag4);

				m_collType = GetPortInt(pActInfo, EIP_CollType);

				m_matrix34 = Matrix34(Matrix33::CreateRotationVDir(dirToTarget));
				m_matrix34.SetTranslation(position);

				pRAG->SetRenderFlags(e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone);
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
			}
			else if (IsPortActive(pActInfo, EIP_Hide))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case IFlowNode::eFE_Update:
		{
			const auto length = GetPortFloat(pActInfo, EIP_Len);
			const auto pos = m_matrix34.GetTranslation();
			const auto dir1 = pos + m_matrix34.GetColumn1().GetNormalizedSafe() * length;
			const auto dir2 = m_matrix34.GetColumn1().GetNormalizedSafe() * length;

			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(pos, ColorB(255,255,255),dir1 , ColorB(255, 255, 255), 1.0f);

			const auto entTypes = m_entityType[0] | m_entityType[1] | m_entityType[2] | m_entityType[3] | m_entityType[4];
			auto rwiflags = 0;

			if (m_rwiFlags[0] == rwi_colltype_any)
			{
				rwiflags = m_rwiFlags[0] | m_rwiFlags[1] | m_rwiFlags[2] | m_rwiFlags[3];
			}
			else if (m_rwiFlags[0] == rwi_colltype_bit)
			{
				rwiflags = (m_collType << m_rwiFlags[0]) | m_rwiFlags[1] | m_rwiFlags[2] | m_rwiFlags[3];
			}

			const int col = gEnv->pPhysicalWorld->RayWorldIntersection(pos, dir2, entTypes, rwiflags, &m_rayhit, 1);
			if (col)
			{
				ActivateOutput(pActInfo, EOP_Intersection, 1);
			}
		}
		break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			m_rayhit = ray_hit();
			m_matrix34.SetIdentity();
			m_collType = 0;

			for (int i = 0; i < 5; i++)
			{
				m_entityType[i] = 0;

				if (i < 4)
					m_rwiFlags[i] = 0;
			}
		}
		break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		}
	}//end of ProcessEvent

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_DrawRay(pActInfo);
	}

	ray_hit m_rayhit;
	Matrix34 m_matrix34;
	int m_entityType[5];
	int m_rwiFlags[4];
	int m_collType;
};

class CFlowNode_SetScriptValue : public CFlowBaseNode
{
public:
	enum EInputs
	{
		EIP_Sync,
		EIP_Table,
		EIP_ValueName,
		EIP_FloatValue,
		EIP_StringValue,
	};
	enum EOutputs
	{
		EOP_Success,
		EOP_Failed,
		EOP_Done,
	};

	CFlowNode_SetScriptValue(SActivationInfo* pActInfo)
	{
	}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		// declare input ports
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_AnyType("Sync",      _HELP("Trigger to set the value")),
			InputPortConfig<string>("TableName", string("Properties"),  _HELP("In lua: entity.TableName")),
			InputPortConfig<string>("ValueName", string(""),  _HELP("In lua: entity.TableName.ValueName or entity.ValueName if TableName is equal 0")),
			InputPortConfig<float>("FloatValue", 0.0f,  _HELP("")),
			InputPortConfig<string>("StringValue", string(""),  _HELP("")),
			{ 0 }
		};
		static const SOutputPortConfig out_ports[] = {
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Failed", _HELP("")),
			OutputPortConfig_AnyType("Done", _HELP("")),
			{ 0 }
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Sets entity's some script table value");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Sync))
		{
			if (pActInfo->pEntity)
			{
				//Get entity's scripttable
				const auto pScriptProxy = dynamic_cast<IEntityScriptProxy*>(pActInfo->pEntity->GetProxy(ENTITY_PROXY_SCRIPT));
				const auto pTable = pScriptProxy->GetScriptTable();
				if (pTable)
				{
					const string table = GetPortString(pActInfo, EIP_Table);
					const string valueName = GetPortString(pActInfo, EIP_ValueName);
					const string strValue = GetPortString(pActInfo, EIP_StringValue);
					const float fValue = GetPortFloat(pActInfo, EIP_FloatValue);
					bool success = false;

					if (table != "")
					{
						if (strValue.size() == 0)
						{
							success = TOS_Script::SetEntityScriptValue(pActInfo->pEntity, table, valueName, fValue);
						}
						else
						{
							success = TOS_Script::SetEntityScriptValue(pActInfo->pEntity, table, valueName, strValue.c_str());
						}
					}
					else
					{
						if (strValue.size() == 0)
						{
							success = TOS_Script::SetEntityScriptValue(pActInfo->pEntity, valueName, fValue);
						}
						else
						{
							success = TOS_Script::SetEntityScriptValue(pActInfo->pEntity, valueName, strValue.c_str());
						}
					}


					ActivateOutput(pActInfo, success ? EOP_Success : EOP_Failed, 1);
				}
			}

		}

		ActivateOutput(pActInfo, EOP_Done, 1);
	}

	//ScriptAnyValue FillArgumentFromAnyPort(SActivationInfo* pActInfo, int port)
	//{
	//	TFlowInputData inputData = GetPortAny(pActInfo, port);

	//	switch (inputData.GetType())
	//	{
	//	case eFDT_Int:
	//		return ScriptAnyValue((float)GetPortInt(pActInfo, port));
	//	case eFDT_EntityId:
	//	{
	//		ScriptHandle id;
	//		id.n = GetPortEntityId(pActInfo, port);
	//		return ScriptAnyValue(id);
	//	}
	//	case eFDT_Bool:
	//		return ScriptAnyValue(GetPortBool(pActInfo, port));
	//	case eFDT_Float:
	//		return ScriptAnyValue(GetPortFloat(pActInfo, port));
	//	case eFDT_String:
	//		return ScriptAnyValue(GetPortString(pActInfo, port));
	//		;
	//	case eFDT_Vec3:
	//		return ScriptAnyValue(GetPortVec3(pActInfo, port));
	//		;
	//	}

		////Type unknown
		//assert(false);

		//return ScriptAnyValue();
	//}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};

class CFlowNode_GetScriptValue : public CFlowBaseNode
{
public:
	enum EInputs
	{
		EIP_Sync,
		EIP_Table,
		EIP_ValueName,
		EIP_ValueType,
	};
	enum EOutputs
	{
		EOP_Output,
	};

	CFlowNode_GetScriptValue(SActivationInfo* pActInfo)
	{
	}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:String=1, Float=2";

		// declare input ports
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_AnyType("Sync",      _HELP("Trigger to Get the value")),
			InputPortConfig<string>("TableName", string("Properties"),  _HELP("In lua: entity.TableName")),
			InputPortConfig<string>("ValueName", string(""),  _HELP("In lua: entity.TableName.ValueName or entity.ValueName if TableName is equal 0")),
			InputPortConfig<int>("ValueType", 1,  _HELP(""), "Type", ui_config),
			{ 0 }
		};
		static const SOutputPortConfig out_ports[] = {
			OutputPortConfig_AnyType("Output", _HELP("")),
			{ 0 }
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Gets entity's some script table value");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Sync))
		{
			if (pActInfo->pEntity)
			{
				//Get entity's scripttable
				const auto pScriptProxy = dynamic_cast<IEntityScriptProxy*>(pActInfo->pEntity->GetProxy(ENTITY_PROXY_SCRIPT));
				const auto pTable = pScriptProxy->GetScriptTable();
				if (pTable)
				{
					const string table = GetPortString(pActInfo, EIP_Table);
					const string valueName = GetPortString(pActInfo, EIP_ValueName);
					const int type = GetPortInt(pActInfo, EIP_ValueType);

					if (table != "")
					{
						if (type == 1)//String
						{
							const char* getted = 0;
							if (TOS_Script::GetEntityScriptValue(pActInfo->pEntity, table, valueName, getted))
								ActivateOutput(pActInfo, EOP_Output, string(getted));
						}
						else if (type == 2)//Float
						{
							float getted = 0;
							if (TOS_Script::GetEntityScriptValue(pActInfo->pEntity, table, valueName, getted))
								ActivateOutput(pActInfo, EOP_Output, getted);
						}
					}
					else
					{
						if (type == 1)//String
						{
							const char* getted = 0;
							if (TOS_Script::GetEntityScriptValue(pActInfo->pEntity, valueName, getted))
								ActivateOutput(pActInfo, EOP_Output, string(getted));
						}
						else if (type == 2)//Float
						{
							float getted = 0;
							if (TOS_Script::GetEntityScriptValue(pActInfo->pEntity, valueName, getted))
								ActivateOutput(pActInfo, EOP_Output, getted);
						}
					}
				}
			}

		}
	}

	//ScriptAnyValue FillArgumentFromAnyPort(SActivationInfo* pActInfo, int port)
	//{
	//	TFlowInputData inputData = GetPortAny(pActInfo, port);

	//	switch (inputData.GetType())
	//	{
	//	case eFDT_Int:
	//		return ScriptAnyValue((float)GetPortInt(pActInfo, port));
	//	case eFDT_EntityId:
	//	{
	//		ScriptHandle id;
	//		id.n = GetPortEntityId(pActInfo, port);
	//		return ScriptAnyValue(id);
	//	}
	//	case eFDT_Bool:
	//		return ScriptAnyValue(GetPortBool(pActInfo, port));
	//	case eFDT_Float:
	//		return ScriptAnyValue(GetPortFloat(pActInfo, port));
	//	case eFDT_String:
	//		return ScriptAnyValue(GetPortString(pActInfo, port));
	//		;
	//	case eFDT_Vec3:
	//		return ScriptAnyValue(GetPortVec3(pActInfo, port));
	//		;
	//	}

		////Type unknown
		//assert(false);

		//return ScriptAnyValue();
	//}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}
};

class CFlowNode_VehicleEnterNew : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
		EIP_SeatId
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleEnterNew(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleEnterNew(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("Vehicle", _HELP("")),
			InputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Failed", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Put the actor in vehicle fast");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
				return;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));	
				if (pVehicle)
				{
					const auto pSeat = pVehicle->GetSeatById(GetPortInt(pActInfo, EIP_SeatId));
					if (pSeat)
					{
						if (pSeat->Enter(pActor->GetEntityId(), false))
						{
							ActivateOutput(pActInfo, EOP_Success, 1);
						}
						else
						{
							ActivateOutput(pActInfo, EOP_Failed, 1);
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleEnterNew(pActInfo);
	}
};

class CFlowNode_VehicleStatus : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_Velocity,
		EOP_Speed,
		EOP_Flipped,
		EOP_Altitude,
		EOP_SubmergedRatio,
		EOP_PassengerCount,
		EOP_Power,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleStatus(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleStatus(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<Vec3>("Velocity", _HELP("")),
			OutputPortConfig<float>("Speed", _HELP("")),
			OutputPortConfig<float>("Flipped", _HELP("")),
			OutputPortConfig<float>("Altitude", _HELP("")),
			OutputPortConfig<float>("SubmergedRatio", _HELP("")),
			OutputPortConfig<int>("PassengersCount", _HELP("")),
			OutputPortConfig<float>("Power", _HELP("")),
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
			if (!pActInfo->pEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId());
				if (pVehicle)
				{
					const auto velocity = pVehicle->GetStatus().vel;
					const auto speed = pVehicle->GetStatus().speed;
					const auto flipped = pVehicle->GetStatus().flipped;
					const auto altitude = pVehicle->GetStatus().altitude;
					const auto submergedRatio = pVehicle->GetStatus().submergedRatio;
					const auto passengerCount = pVehicle->GetStatus().passengerCount;

					const auto pInfo = g_pControlSystem->GetVehicleMovementInfo(pVehicle->GetEntityId());
					if (pInfo)
					{
						const auto& action = pInfo->GetMovementAction();

						ActivateOutput(pActInfo, EOP_Power, action.power);
					}

					ActivateOutput(pActInfo, EOP_Velocity, velocity);
					ActivateOutput(pActInfo, EOP_Speed, speed);
					ActivateOutput(pActInfo, EOP_Flipped, flipped);
					ActivateOutput(pActInfo, EOP_Altitude, altitude);
					ActivateOutput(pActInfo, EOP_SubmergedRatio, submergedRatio);
					ActivateOutput(pActInfo, EOP_PassengerCount, passengerCount);
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
		return new CFlowNode_VehicleStatus(pActInfo);
	}
};

class CFlowNode_VehicleChangeSeat : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
		EIP_SeatId
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleChangeSeat(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleChangeSeat(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("Vehicle", _HELP("")),
			InputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Failed", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Put the actor in vehicle fast");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
				return;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));
				if (pVehicle)
				{
					const auto pSeat = pVehicle->GetSeatById(GetPortInt(pActInfo, EIP_SeatId));
					if (pSeat && !pSeat->GetPassenger())
					{
						TOS_Vehicle::ChangeSeat(pActor, pSeat->GetSeatId(), false);
						ActivateOutput(pActInfo, EOP_Success, 1);
					}
					else
					{
						ActivateOutput(pActInfo, EOP_Failed, 1);
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleChangeSeat(pActInfo);
	}
};

class CFlowNode_AIGetTarget : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_TargetType,
		EOP_TargetThreat,
		EOP_TargetId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AIGetTarget(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIGetTarget()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const string sTypes = "0 - AITARGET_NONE, \
						1 - AITARGET_SOUND, \
						2 - AITARGET_MEMORY, \
						3 - AITARGET_VISUAL, \
						4 - AITARGET_ENEMY, \
						5 - AITARGET_FRIENDLY, \
						6 - AITARGET_BEACON, \
						7 - AITARGET_GRENADE, \
						8 - AITARGET_RPG";
		const string sThreat = "0 - AITHREAT_NONE, \
						1 - AITHREAT_INTERESTING, \
						2 - AITHREAT_THREATENING, \
						3 - AITHREAT_AGGRESSIVE, \
						4 - AITHREAT_LAST";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Type",_HELP("Output the type of ai attention target." + sTypes)),
			OutputPortConfig<int>("Threat",_HELP("Output the threat of ai attention target." + sThreat)),
			OutputPortConfig<EntityId>("TargetId",_HELP("Output the entity Id of ai attention target.")),
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
			const IAISystem* pAI = gEnv->pAISystem;
			if (pAI)
			{
				IEntity* pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					if (IAIObject* pGraphAI = pGraphEntity->GetAI())
					{
						if (const IPipeUser* pPU = pGraphAI->CastToIPipeUser())
						{
							const int type = pPU->GetAttentionTargetType();
							const int threat = pPU->GetAttentionTargetThreat();

							const IAIObject* pTarget = pPU->GetAttentionTarget();
							if (pTarget)
								ActivateOutput(pActInfo, EOP_TargetId, pTarget->GetEntityID());

							ActivateOutput(pActInfo, EOP_TargetType, type);
							ActivateOutput(pActInfo, EOP_TargetThreat, threat);
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
		return new CFlowNode_AIGetTarget(pActInfo);
	}
};

class CFlowNode_AIReset : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_ReferenceTarget,
		EIP_Delay,
		EIP_ZeroPerception,
		EIP_RestorePerception,
		EIP_Ignore,
		EIP_ResetIgnore,
		EIP_AcquireTarget,
		EIP_MakeMeIgnorant,
		EIP_MakeMeUnignorant,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

	IPipeUser* m_pPipe = 0;
	float m_currtime, m_delay, m_oldVisualScale, m_oldAudioScale = 0;
	IEntity* m_pGraphEntity = 0;
	string m_sPipeName = "";
	AgentParameters agentParams;
	bool m_bRestorePerc,
		m_bZeroPerc,
		m_bIgnore,
		m_bResetIgnore,
		m_bAcquireTarget,
		m_makeMeIgnorant = false;
	EntityId m_eRefTarget = 0;

public:
	////////////////////////////////////////////////////
	CFlowNode_AIReset(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIReset()
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
			InputPortConfig_Void("Trigger", _HELP("Trigger to reset the input AI entity")),
			InputPortConfig<EntityId>("RefTarget", _HELP("")),
			InputPortConfig<float>("Delay",0.5f, _HELP("During delay in sec input entity will be selecting special reset goalpipe every frame. Its is necessary because AI can't be resetted normally")),
			InputPortConfig<bool>("ZeroPerception",true, _HELP("Perception scale visual and audio set to 0 if Trigger port")),
			InputPortConfig<bool>("RestorePerception",false, _HELP("Perception scale visual and audio will be set to 1 after the Done port")),
			InputPortConfig<bool>("Ignore",false, _HELP("FG Ignore set to 1 if Trigger port")),
			InputPortConfig<bool>("ResetIgnore",false, _HELP("FG Ignore set will be set to 0 after the Done port")),
			InputPortConfig<bool>("AcquireTarget",false, _HELP("Acquire the reference point as the AI target")),
			InputPortConfig<bool>("MakeMeIgnorant",false, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("Done",_HELP("When AI is resetted, outputting this port with id of entity")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("The node resets the AI and makes it capable of reacting to other AI nodes");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			m_delay = GetPortFloat(pActInfo, EIP_Delay);
			m_bRestorePerc = GetPortBool(pActInfo, EIP_RestorePerception);
			m_bZeroPerc = GetPortBool(pActInfo, EIP_ZeroPerception);
			m_bIgnore = GetPortBool(pActInfo, EIP_Ignore);
			m_bResetIgnore = GetPortBool(pActInfo, EIP_ResetIgnore);
			m_eRefTarget = GetPortEntityId(pActInfo, EIP_ReferenceTarget);
			m_bAcquireTarget = GetPortBool(pActInfo, EIP_AcquireTarget);
			m_makeMeIgnorant = GetPortBool(pActInfo, EIP_MakeMeIgnorant);

			if (IsPortActive(pActInfo, EIP_Trigger))
			{
				if (IAISystem* pAI = gEnv->pAISystem)
				{
					if (m_pGraphEntity = pActInfo->pEntity)
					{
						const string sEntityClass = m_pGraphEntity->GetClass()->GetName();
						if (sEntityClass == "Trooper")
						{
							m_sPipeName = "ord_cooldown_trooper";
						}
						else
						{
							m_sPipeName = "ord_cooldown";
						}

						if (IAIObject* pGraphAI = m_pGraphEntity->GetAI())
						{
							//dont touch this block
							if (m_pPipe = pGraphAI->CastToIPipeUser())
							{
								m_pPipe->MakeIgnorant(m_makeMeIgnorant);

								pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
								m_currtime = gEnv->pTimer->GetCurrTime();
								//pPipe->MakeIgnorant(true); not work in use with scout

								agentParams = pGraphAI->CastToIAIActor()->GetParameters();
								m_oldVisualScale = agentParams.m_PerceptionParams.perceptionScale.visual;
								m_oldAudioScale = agentParams.m_PerceptionParams.perceptionScale.audio;

								agentParams.m_bAiIgnoreFgNode = m_bIgnore;

								if (m_bZeroPerc)
								{
									agentParams.m_PerceptionParams.perceptionScale.audio = 0;
									agentParams.m_PerceptionParams.perceptionScale.visual = 0;
								}

								pGraphAI->CastToIAIActor()->SetParameters(agentParams);
								pGraphAI->CastToIAIActor()->GetState()->FullReset();
								//A very important signal, notifies the AI of the "death" of its target,
								//giving a perception visual and audio scales to work without randomly resetting
								//pAI->SendSignal(SIGNALFILTER_SENDER, 1, "OnTargetDead", pGraphAI);

								m_pPipe->SelectPipe(0, "do_nothing");
								m_pPipe->SelectPipe(0, m_sPipeName);

								if (m_bZeroPerc)
								{
									agentParams.m_PerceptionParams.perceptionScale.audio = 0;
									agentParams.m_PerceptionParams.perceptionScale.visual = 0;
									pGraphAI->CastToIAIActor()->SetParameters(agentParams);
								}

								if (m_bAcquireTarget)
								{
									IEntity* pTarget = gEnv->pEntitySystem->GetEntity(m_eRefTarget);
									if (pTarget)
									{
										IAIObject* pTargetAI = pTarget->GetAI();
										m_pPipe->InsertSubPipe(0, "acquire_target", pTargetAI ? pTargetAI : 0);
									}
								}
							}
						}
					}
				}
			}
		}
		break;
		case IFlowNode::eFE_Update:
			if (m_pGraphEntity && m_pGraphEntity->GetAI())
			{
				//dont touch this block
				if (m_pPipe)
				{
					//pPipe->SelectPipe(0, "clear_all", 0, 0);
					m_pPipe->SelectPipe(0, m_sPipeName, 0, 0);

					if (m_bZeroPerc)
					{
						agentParams = m_pGraphEntity->GetAI()->CastToIAIActor()->GetParameters();
						agentParams.m_PerceptionParams.perceptionScale.audio = 0;
						agentParams.m_PerceptionParams.perceptionScale.visual = 0;
						m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
					}
				}

				if (gEnv->pTimer->GetCurrTime() - m_currtime > m_delay)
				{
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
					//pPipe->InsertSubPipe(0, "clear_all", 0, 0); // зацикливается
					//m_pPipe->SelectPipe(0, m_sPipeName, 0, 0);
					//pPipe->MakeIgnorant(false);

					if (m_bZeroPerc)
					{
						agentParams.m_PerceptionParams.perceptionScale.audio = 0;
						agentParams.m_PerceptionParams.perceptionScale.visual = 0;
						m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
					}

					if (m_bRestorePerc)
					{
						agentParams.m_PerceptionParams.perceptionScale.audio = 1;
						agentParams.m_PerceptionParams.perceptionScale.visual = 1;
						m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
					}

					if (m_bResetIgnore)
						agentParams.m_bAiIgnoreFgNode = false;

					if (const IEntity* pTarget = gEnv->pEntitySystem->GetEntity(m_eRefTarget))
						m_pGraphEntity->GetAI()->CastToIPipeUser()->SetRefPointPos(pTarget->GetWorldPos());

					m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
					//CryLogAlways("%s Visual %f Audio %f", pActInfo->pEntity->GetName(), agentParams.m_PerceptionParams.perceptionScale.visual, agentParams.m_PerceptionParams.perceptionScale.audio);
					ActivateOutput(pActInfo, EOP_Done, m_pGraphEntity->GetId());
				}
			}
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_AIReset(pActInfo);
	}
};

class CFlowNode_AIEnableCombat : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Enable,
		EIP_Disable,
		EIP_ToFirst,
	};

	enum EOutputPorts
	{
		EOP_Enabled,
		EOP_Disabled,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AIEnableCombat(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIEnableCombat()
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
			InputPortConfig_AnyType("Enable",false, _HELP("")),
			InputPortConfig_AnyType("Disable",false, _HELP("")),
			InputPortConfig<bool>("ToFirst",false, _HELP("Returns the AI to the first selected behavior. Usually it's an idle")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Enabled",_HELP("")),
			OutputPortConfig_AnyType("Disabled",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Calms down the AI if it is in combat and the AI does not engage in combat again");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IAISystem* pAI = gEnv->pAISystem)
			{
				if (const auto pEntity = pActInfo->pEntity)
				{
					if (const auto pGraphAI = pEntity->GetAI())
					{
						if (IsPortActive(pActInfo, EIP_Enable))
						{
							TOS_AI::EnableCombat(pGraphAI, true, GetPortBool(pActInfo, EIP_ToFirst), "CFlowNode_AIEnableCombat");
							ActivateOutput(pActInfo, EOP_Enabled, 1);
						}
						else if (IsPortActive(pActInfo, EIP_Disable))
						{
							TOS_AI::EnableCombat(pGraphAI, false, GetPortBool(pActInfo, EIP_ToFirst), "CFlowNode_AIEnableCombat");
							ActivateOutput(pActInfo, EOP_Disabled, 1);
						}
					}
				}
			}


		}
		break;
		case IFlowNode::eFE_Update:
			//if (m_pGraphEntity && m_pGraphEntity->GetAI())
			//{
			//	//dont touch this block
			//	if (m_pPipe)
			//	{
			//		//pPipe->SelectPipe(0, "clear_all", 0, 0);
			//		m_pPipe->SelectPipe(0, m_sPipeName, 0, 0);

			//		if (m_bZeroPerc)
			//		{
			//			agentParams = m_pGraphEntity->GetAI()->CastToIAIActor()->GetParameters();
			//			agentParams.m_PerceptionParams.perceptionScale.audio = 0;
			//			agentParams.m_PerceptionParams.perceptionScale.visual = 0;
			//			m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
			//		}
			//	}

			//	if (gEnv->pTimer->GetCurrTime() - m_currtime > m_delay)
			//	{
			//		pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			//		//pPipe->InsertSubPipe(0, "clear_all", 0, 0); // зацикливается
			//		//m_pPipe->SelectPipe(0, m_sPipeName, 0, 0);
			//		//pPipe->MakeIgnorant(false);

			//		if (m_bZeroPerc)
			//		{
			//			agentParams.m_PerceptionParams.perceptionScale.audio = 0;
			//			agentParams.m_PerceptionParams.perceptionScale.visual = 0;
			//			m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
			//		}

			//		if (m_bRestorePerc)
			//		{
			//			agentParams.m_PerceptionParams.perceptionScale.audio = 1;
			//			agentParams.m_PerceptionParams.perceptionScale.visual = 1;
			//			m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
			//		}

			//		if (m_bResetIgnore)
			//			agentParams.m_bAiIgnoreFgNode = false;

			//		if (IEntity* pTarget = gEnv->pEntitySystem->GetEntity(m_eRefTarget))
			//			m_pGraphEntity->GetAI()->CastToIPipeUser()->SetRefPointPos(pTarget->GetWorldPos());

			//		m_pGraphEntity->GetAI()->CastToIAIActor()->SetParameters(agentParams);
			//		//CryLogAlways("%s Visual %f Audio %f", pActInfo->pEntity->GetName(), agentParams.m_PerceptionParams.perceptionScale.visual, agentParams.m_PerceptionParams.perceptionScale.audio);
			//		ActivateOutput(pActInfo, EOP_Done, m_pGraphEntity->GetId());
			//	}
			//}
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_AIEnableCombat(pActInfo);
	}
};

class CFlowNode_AIExecuteAction : public CFlowBaseNode, IAIActionTrackerListener
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Stop,
		EIP_Name,
		EIP_ObjectId,
		EIP_GoalPipeId,
		EIP_MaxAlertness,
		EIP_HighPriority,
		EIP_ExecutingFlag,
		EIP_DesiredGoalName,
	};

	enum EOutputPorts
	{
		EOP_Started,
		EOP_Finished,
		EOP_Failed,
		EOP_Stopped,
		EOP_Paused,
		EOP_Unpaused,
	};

public:

	//IAIActionTrackerListener
	void OnActorDeath(IActor* pActor) override
	{
		if (!pActor)
			return;

		if (m_started)
			ActivateOutput(&m_actInfo, EOP_Failed, 1);
	}

	void OnGoalPipeEvent(IAIObject* pObject, EGoalPipeEvent event, int goalPipeId) override
	{
		if (!pObject)
			return;

		SAIActionInfo info;
		g_pControlSystem->GetAIActionTracker()->GetActionInfo(pObject, info);

		if (m_goalPipeId == goalPipeId)
		{
			switch (event)
			{
			case ePN_Deselected:
			case ePN_OwnerRemoved:
			{
				if (m_started)
					ActivateOutput(&m_actInfo, EOP_Failed, 1);
			}
			break;
			case ePN_Suspended:
				break;
			case ePN_Resumed:
				break;
			case ePN_Finished:
			case ePN_Removed:
			{
				if (m_started && !info.paused)
					ActivateOutput(&m_actInfo, EOP_Finished, 1);
			}
			break;
			case ePN_AnimStarted:
				break;
			case ePN_RefPointMoved:
				break;
			default:
				break;
			}
		}
	}

	void OnActionPaused(IAIObject* pObject) override
	{
		ActivateOutput(&m_actInfo, EOP_Paused, 1);
	}
	void OnActionUnpaused(IAIObject* pObject) override
	{
		ActivateOutput(&m_actInfo, EOP_Unpaused, 1);
	}
	//~IAIActionTrackerListener

	////////////////////////////////////////////////////
	CFlowNode_AIExecuteAction(SActivationInfo* pActInfo)
	{
		m_goalPipeId = 0;
		m_started = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIExecuteAction()
	{
		if (g_pControlSystem->GetAIActionTracker())
			g_pControlSystem->GetAIActionTracker()->RemoveListener(this);
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config1 = "enum_int:Idle=0,Alerted=1,Combat=2";
		const char* ui_config2 = "enum_int:DisableCombatDuringAction=1,PausingActionWhenCombat=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start",false, _HELP("")),
			InputPortConfig_AnyType("Stop",false, _HELP("")),
			InputPortConfig<string>("soaction_Name",string(""), _HELP(""), "Name"),
			InputPortConfig<EntityId>("ObjectId",0, _HELP("")),
			InputPortConfig<int>("GoalPipeId",-1, _HELP("If set to -1 then goalPipeId will be generated by itself")),
			InputPortConfig<int>("MaxAlertness",2, _HELP(""), "MaxAlertness", ui_config1),
			InputPortConfig<bool>("HighPriority",false, _HELP("+100 to MaxAlertness")),
			InputPortConfig<int>("ExecutingFlag",1, _HELP(""), "ExecutingFlag",ui_config2),
			InputPortConfig<string>("DesiredGoalName", string(""), _HELP("Used for void goal pipe detection"), "DesiredGoalName"),
			//InputPortConfig<bool>("IgnoreCombat",false, _HELP("AI ignores combat when performing an action")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Started",_HELP("When activated Start port")),
			OutputPortConfig_AnyType("Finished",_HELP("When ai success finish action")),
			OutputPortConfig_AnyType("Failed",_HELP("When ai die")),
			OutputPortConfig_AnyType("Stopped",_HELP("When activated Stop port")),
			OutputPortConfig_AnyType("Paused",_HELP("")),
			OutputPortConfig_AnyType("Unpaused",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Causes the AI to perform an action based on input. This node is tracked by the AI Action Tracker (tos_debug_draw_ai...). Attention: the old AI:AIExecute node is not relevant");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto pAI = pEntity->GetAI();
			if (!pAI)
				return;

			if (IsPortActive(pActInfo, EIP_Start))
			{
				const auto pObject = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo, EIP_ObjectId));
				if (!pObject)
					return;

				const char* actionName = GetPortString(pActInfo, EIP_Name);
				const bool highPriority = GetPortBool(pActInfo, EIP_HighPriority);
				const auto executingFlag = EAAEFlag(GetPortInt(pActInfo, EIP_ExecutingFlag));
				const string desiredGoalName = GetPortString(pActInfo, EIP_DesiredGoalName);

				int alertness = GetPortInt(pActInfo, EIP_MaxAlertness);
				if (highPriority)
					alertness += 100;

				m_goalPipeId = GetPortInt(pActInfo, EIP_GoalPipeId);
				m_goalPipeId = TOS_AI::ExecuteAIAction(pAI, pObject, actionName, alertness, m_goalPipeId, executingFlag, desiredGoalName.c_str(), "CFlowNode_AIExecuteAction::ProcessEvent");

				if (m_goalPipeId != -1)
				{
					m_started = true;
					ActivateOutput(pActInfo, EOP_Started, 1);
				}
			}
			else if (IsPortActive(pActInfo, EIP_Stop))
			{
				TOS_AI::AbortAIAction(pEntity->GetAI(), m_goalPipeId, "CFlowNode_AIExecuteAction::ProcessEvent");
				ActivateOutput(pActInfo, EOP_Stopped, 1);
			}
		}
		break;
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			if (g_pControlSystem->GetAIActionTracker())
				g_pControlSystem->GetAIActionTracker()->AddListener(this);

			m_actInfo = *pActInfo;
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_AIExecuteAction(pActInfo);
	}

	bool m_started;
	SActivationInfo m_actInfo;
	int m_goalPipeId;
};

class CFlowNode_PauseAIAction : public CFlowBaseNode, IAIActionTrackerListener
{
	enum EInputPorts
	{
		EIP_Pause,
		EIP_Unpause,
	};

	enum EOutputPorts
	{
		EOP_Paused,
		EOP_Unpaused,
	};

public:

	//IAIActionTrackerListener
	void OnGoalPipeEvent(IAIObject* pObject, EGoalPipeEvent event, int goalPipeId) override
	{
	}

	void OnActorDeath(IActor* pActor) override
	{
		if (!pActor)
			return;
	}

	void OnActionPaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		ActivateOutput(&m_actInfo, EOP_Paused, 1);
	}

	void OnActionUnpaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		ActivateOutput(&m_actInfo, EOP_Unpaused, 1);
	}
	//~IAIActionTrackerListener

	////////////////////////////////////////////////////
	CFlowNode_PauseAIAction(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_PauseAIAction()
	{
		if (g_pControlSystem->GetAIActionTracker())
			g_pControlSystem->GetAIActionTracker()->RemoveListener(this);
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
			InputPortConfig_AnyType("Pause",false, _HELP("")),
			InputPortConfig_AnyType("Unpause",false, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Paused",_HELP("")),
			OutputPortConfig_AnyType("Unpaused",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("The node removes the current action from the AI behavior, but does not remove the tracking in the AI Action Tracker so that you can return to the action later.");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto pAI = pEntity->GetAI();
			if (!pAI)
				return;

			if (IsPortActive(pActInfo, EIP_Pause))
			{
				TOS_AI::PauseAction(pAI);
			}
			else if (IsPortActive(pActInfo, EIP_Unpause))
			{
				TOS_AI::UnpauseAction(pAI);
			}
		}
		break;
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			if (g_pControlSystem->GetAIActionTracker())
				g_pControlSystem->GetAIActionTracker()->AddListener(this);

			m_actInfo = *pActInfo;
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_PauseAIAction(pActInfo);
	}

	SActivationInfo m_actInfo;
};

class CFlowNode_AbortAIAction : public CFlowBaseNode, IAIActionTrackerListener
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:

	//IAIActionTrackerListener
	void OnGoalPipeEvent(IAIObject* pObject, EGoalPipeEvent event, int goalPipeId) override
	{
	}

	void OnActorDeath(IActor* pActor) override
	{
		if (!pActor)
			return;
	}

	void OnActionPaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		//ActivateOutput(&m_actInfo, EOP_Paused, 1);
	}

	void OnActionUnpaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		//ActivateOutput(&m_actInfo, EOP_Unpaused, 1);
	}
	//~IAIActionTrackerListener

	////////////////////////////////////////////////////
	CFlowNode_AbortAIAction(SActivationInfo* pActInfo)
	{

	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AbortAIAction()
	{
		g_pControlSystem->GetAIActionTracker()->RemoveListener(this);
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
			InputPortConfig_AnyType("Sync",false, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("The node removes the current action from the AI behavior and remove the tracking in the AI Action Tracker.");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto pAI = pEntity->GetAI();
			if (!pAI)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				TOS_AI::AbortAIAction(pAI, -1, "CFlowNode_AbortAIAction");
				ActivateOutput(pActInfo, EOP_Done, 1);
			}
		}
		break;
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			m_actInfo = *pActInfo;
			if (g_pControlSystem->GetAIActionTracker())
				g_pControlSystem->GetAIActionTracker()->AddListener(this);
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_AbortAIAction(pActInfo);
	}

	SActivationInfo m_actInfo;
};

class CFlowNode_GetActionFlag : public CFlowBaseNode, IAIActionTrackerListener
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_Flag,
	};

public:

	//IAIActionTrackerListener
	void OnGoalPipeEvent(IAIObject* pObject, EGoalPipeEvent event, int goalPipeId) override
	{
	}

	void OnActorDeath(IActor* pActor) override
	{
		if (!pActor)
			return;
	}

	void OnActionPaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		//ActivateOutput(&m_actInfo, EOP_Paused, 1);
	}

	void OnActionUnpaused(IAIObject* pObject) override
	{
		if (!pObject)
			return;

		//ActivateOutput(&m_actInfo, EOP_Unpaused, 1);
	}
	//~IAIActionTrackerListener

	////////////////////////////////////////////////////
	CFlowNode_GetActionFlag(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetActionFlag()
	{
		g_pControlSystem->GetAIActionTracker()->RemoveListener(this);
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
			InputPortConfig_AnyType("Sync",false, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Flag",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get entity's current action's flag");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto pAI = pEntity->GetAI();
			if (!pAI)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				if (g_pControlSystem->GetAIActionTracker()->IsTracking(pAI))
				{
					SAIActionInfo info;
					g_pControlSystem->GetAIActionTracker()->GetActionInfo(pAI, info);

					ActivateOutput(pActInfo, EOP_Flag, int(info.flag));
				}
			}
		}
		break;
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			if (g_pControlSystem->GetAIActionTracker())
				g_pControlSystem->GetAIActionTracker()->AddListener(this);
			m_actInfo = *pActInfo;
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_GetActionFlag(pActInfo);
	}

	SActivationInfo m_actInfo;
};


class CFlowNode_SetRefPoint : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sink,
		EIP_Position,
	};

	enum EOutputPorts
	{
		EOP_Synced,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SetRefPoint(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SetRefPoint()
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<Vec3>("Position", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Synced",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Set AI reference point");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			const IAISystem* pAI = gEnv->pAISystem;
			if (pAI)
			{
				IEntity* pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					if (IAIObject* pGraphAI = pGraphEntity->GetAI())
					{
						if (IPipeUser* pPU = pGraphAI->CastToIPipeUser())
						{
							pPU->SetRefPointPos(GetPortVec3(pActInfo, EIP_Position));
							ActivateOutput(pActInfo, EOP_Synced, 1);
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
		return new CFlowNode_SetRefPoint(pActInfo);
	}
};

class CFlowNode_AIGetPerc : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_Visual,
		EOP_Audio
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AIGetPerc(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIGetPerc()
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
			InputPortConfig_Void("Trigger", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<float>("Visual",_HELP("")),
			OutputPortConfig<float>("Audio",_HELP("")),
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
				if (IAISystem* pAI = gEnv->pAISystem)
				{
					if (IEntity* m_pGraphEntity = pActInfo->pEntity)
					{
						if (IAIObject* pGraphAI = m_pGraphEntity->GetAI())
						{
							const AgentParameters agentParams = pGraphAI->CastToIAIActor()->GetParameters();
							const float m_oldVisualScale = agentParams.m_PerceptionParams.perceptionScale.visual;
							const float m_oldAudioScale = agentParams.m_PerceptionParams.perceptionScale.audio;

							float clr[] = { 1,1,1,1 };
							gEnv->pRenderer->Draw2dLabel(20, 100, 1.1f, clr, false, "%s Visual %1.0f Audio %1.0f", m_pGraphEntity->GetName(), m_oldVisualScale, m_oldAudioScale);
						}
					}
				}
			}
		}
		break;
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
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
		return new CFlowNode_AIGetPerc(pActInfo);
	}
};

class CFlowNode_ActorAddItem : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_ItemClass,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorAddItem(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorAddItem()
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
			InputPortConfig<string>("Item", _HELP("Item class name")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Done",_HELP("")),
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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							if (IInventory* pInventory = pActor->GetInventory())
							{
								if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
								{
									const string itemClassName = GetPortString(pActInfo, EIP_ItemClass);

									pClassRegistry->IteratorMoveFirst();
									const IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

									if (pEntityClass)
									{
										g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemClassName, false, true, false);
										ActivateOutput(pActInfo, EOP_Done, true);
									}
								}
							}
						}
					}
				}
				//ActivateOutput(pActInfo, EOP_Done, false);
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
		return new CFlowNode_ActorAddItem(pActInfo);
	}
};

class CFlowNode_ActorGetCurrentItem : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync
	};

	enum EOutputPorts
	{
		EOP_ItemId,
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorGetCurrentItem(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorGetCurrentItem()
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
			InputPortConfig_AnyType("Sync", _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("ItemId",_HELP("")),
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
			if (IsPortActive(pActInfo, EIP_Sync))
			{
				const auto pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const auto pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							const auto pCurrentItem = pActor->GetCurrentItem();

							ActivateOutput(pActInfo, EOP_ItemId, pCurrentItem ? pCurrentItem->GetEntityId() : 0);
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
		return new CFlowNode_ActorGetCurrentItem(pActInfo);
	}
};

class CFlowNode_ActorPickupItem : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_ItemId,
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed,
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorPickupItem(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorPickupItem()
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
			InputPortConfig<EntityId>("ItemId", _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success",_HELP("")),
			OutputPortConfig_AnyType("Failed",_HELP("")),
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
				const auto pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const auto pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							const auto itemId = GetPortEntityId(pActInfo, EIP_ItemId);
							const auto pInputItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
							if (!pInputItem)
								return;

							const string inputWeaponClass = pInputItem->GetEntity()->GetClass()->GetName();

							const char* primary = 0;
							const char* secondary = 0;
							TOS_Script::GetEntityScriptValue(pEntity, "primaryWeapon", primary);
							TOS_Script::GetEntityScriptValue(pEntity, "secondaryWeapon", secondary);

							const auto pCurrentItem = pActor->GetCurrentItem();
							if (pCurrentItem)
							{
								const string weaponClass = pCurrentItem->GetEntity()->GetClass()->GetName();
								if (weaponClass == primary)
								{
									primary = inputWeaponClass.c_str();
									TOS_Script::SetEntityScriptValue(pEntity, "primaryWeapon", primary);
								}
								else if (weaponClass == secondary)
								{
									secondary = inputWeaponClass.c_str();
									TOS_Script::SetEntityScriptValue(pEntity, "secondaryWeapon", secondary);
								}

								pActor->DropItem(pCurrentItem->GetEntityId());
							}

							const auto pickedUp = pActor->PickUpItem(itemId, true, true);

							ActivateOutput(pActInfo, pickedUp ? EOP_Success : EOP_Failed, 1);
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
		return new CFlowNode_ActorPickupItem(pActInfo);
	}
};

class CFlowNode_ActorRemoveAllItems : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorRemoveAllItems(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorRemoveAllItems()
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
			OutputPortConfig<bool>("Done",_HELP("")),
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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						const CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							if (IInventory* pInventory = pActor->GetInventory())
								pInventory->RemoveAllItems();
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
		return new CFlowNode_ActorRemoveAllItems(pActInfo);
	}
};

class CFlowNode_ActorRemoveItem : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_ItemClass,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorRemoveItem(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorRemoveItem()
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
			InputPortConfig<string>("Item", _HELP("Item class name")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Done",_HELP("")),
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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						const CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							if (const IInventory* pInventory = pActor->GetInventory())
							{
								if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
								{
									pClassRegistry->IteratorMoveFirst();
									IEntityClass* pEntityClass = pClassRegistry->FindClass(GetPortString(pActInfo, EIP_ItemClass));

									if (pEntityClass)
									{
										const EntityId itemId = pInventory->GetItemByClass(pEntityClass);
										g_pGame->GetIGameFramework()->GetIItemSystem()->RemoveItem(itemId);

										ActivateOutput(pActInfo, EOP_Done, true);
									}
								}
							}
						}
					}
				}
				//ActivateOutput(pActInfo, EOP_Done, false);
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
		return new CFlowNode_ActorRemoveItem(pActInfo);
	}
};

class CFlowNode_AIEvent : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Position,
		EIP_Event,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

	enum EAIEventType
	{
		EAIEVENT_ONBODYSENSOR = 1,
		EAIEVENT_ONVISUALSTIMULUS,
		EAIEVENT_ONPATHDECISION,
		EAIEVENT_ONSOUNDEVENT,
		EAIEVENT_AGENTDIED,
		EAIEVENT_SLEEP,
		EAIEVENT_WAKEUP,
		EAIEVENT_ENABLE,
		EAIEVENT_DISABLE,
		EAIEVENT_REJECT,
		EAIEVENT_PATHFINDON,
		EAIEVENT_PATHFINDOFF,
		EAIEVENT_CLEAR = 15,
		EAIEVENT_DROPBEACON = 17,
		EAIEVENT_USE = 19,
		EAIEVENT_CLEARACTIVEGOALS = 22,
		EAIEVENT_DRIVER_IN,
		EAIEVENT_DRIVER_OUT,
		EAIEVENT_FORCEDNAVIGATION,
		EAIEVENT_ADJUSTPATH,
		EAIEVENT_LOWHEALTH,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AIEvent(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AIEvent()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:OnBodySensor=1,OnVisualStimulus=2,OnPathDecision=3,OnSoundEvent=4,AgentDied=5,Sleep=6,WakeUp=7,Enable=8,Disable=9,Reject=10,PathfindOn=11,PathfindOff=12,Clear=15,DropBeacon=17,Use=19,ClearActiveGoals=22,DriverIn=23,DriverOut=24,ForcedNav=25,AdjustPath=26,LowHealth=27";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Trigger", _HELP("")),
			InputPortConfig<Vec3>("Pos", Vec3(0,0,0), _HELP("")),
			InputPortConfig<int>("Event", 0, _HELP(""),0,ui_config),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP("")),
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
			const IAISystem* pAI = gEnv->pAISystem;
			if (pAI)
			{
				IEntity* pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					if (IAIObject* pGraphAI = pGraphEntity->GetAI())
					{
						const int iEvent = GetPortInt(pActInfo, EIP_Event);
						const auto eventPos = GetPortVec3(pActInfo, EIP_Position);

						SAIEVENT event;
						if (iEvent == AIEVENT_FORCEDNAVIGATION)
						{
							event.vForcedNavigation = eventPos;
							event.vPosition = eventPos;
						}

						pGraphAI->Event(iEvent, &event);

						ActivateOutput(pActInfo, EOP_Done, 1);
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
		return new CFlowNode_AIEvent(pActInfo);
	}
};

class CFlowNode_PlayAction : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Play,
		EIP_Action,
		EIP_Looping,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_PlayAction(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_PlayAction()
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
			InputPortConfig_AnyType("Play", _HELP("")),
			InputPortConfig<string>("Action","idle", _HELP("")),
			InputPortConfig<bool>("Looping","0", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
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
			if (IsPortActive(pActInfo, EIP_Play))
			{
				if (const IEntity* pGraphEntity = pActInfo->pEntity)
				{
					CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
					if (pActor)
						PlayAction(pActor, GetPortString(pActInfo, EIP_Action), GetPortBool(pActInfo, EIP_Looping));
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
		return new CFlowNode_PlayAction(pActInfo);
	}

	void PlayAction(CActor* pActor, string action, bool looping)
	{
		if (!pActor)
			return;
		if (!pActor->GetAnimatedCharacter())
			return;

		if (looping)
		{
			if (pActor->GetAnimatedCharacter()->GetAnimationGraphState()->SetInput("Action", action))
				return;
			else
				return;
		}
		else
		{
			if (pActor->GetAnimatedCharacter()->GetAnimationGraphState()->SetInput("Signal", action))
				return;
			else
				return;
		}
	}
};

class CFlowNode_ScoutDropBomb : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Force,
		EIP_Target,
	};

	enum EOutputPorts
	{
		EOP_Done
	};

	IEntity* pEntity = 0;
public:
	////////////////////////////////////////////////////
	CFlowNode_ScoutDropBomb(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ScoutDropBomb()
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
			InputPortConfig<float>("Force", 5000.f, _HELP("")),
			InputPortConfig<Vec3>("Target", Vec3(0,0,0), _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("Done",_HELP("")),
			//OutputPortConfig<bool>("Disabled",_HELP("")),
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
				if (pEntity = pActInfo->pEntity)
				{
					const CScout* pGraphScout = dynamic_cast<CScout*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
					if (pGraphScout)
					{
						const float fForce = GetPortFloat(pActInfo, EIP_Force);
						const Vec3 vTarget = GetPortVec3(pActInfo, EIP_Target);

						if (const IEntity* pBomb = DropHealBomb(fForce, vTarget))
							ActivateOutput(pActInfo, EOP_Done, pBomb->GetId());
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
		return new CFlowNode_ScoutDropBomb(pActInfo);
	}

	IEntity* DropHealBomb(float fForce, Vec3 vTarget)
	{
		if (!pEntity)
			return 0;

		SEntitySpawnParams params;
		IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry();

		if (pClassRegistry)
		{
			pClassRegistry->IteratorMoveFirst();
			IEntityClass* pEntityClass = pClassRegistry->FindClass("AlienHealBomb");
			if (pEntityClass)
			{
				string strScoutName = pEntity->GetName();
				string strClassName = pEntityClass->GetName();

				params.pClass = pEntityClass;
				params.sName = strScoutName + " " + strClassName;

				Matrix34 scMat34 = pEntity->GetWorldTM();
				Vec3 vScPos = scMat34.GetTranslation();
				Vec3 vBombPos(0, 0, 0);

				vBombPos.x = vScPos.x;
				vBombPos.z = vScPos.z - 1.5f;
				vBombPos.y = vScPos.y + 1;
				params.vPosition = vBombPos;

				Vec3 vDir = (vTarget - vBombPos).GetNormalizedSafe();
				Vec3 vNewDir(vDir);
				vNewDir.x *= -1;
				vNewDir.y *= -1;

				Quat qRotation;
				qRotation.SetRotationVDir(vNewDir);
				params.qRotation = qRotation;

				IEntity* pBomb = gEnv->pEntitySystem->SpawnEntity(params);
				if (pBomb)
				{
					if (IPhysicalEntity* pPhysEntity = pBomb->GetPhysics())
					{
						pe_action_impulse impulse;
						if (vTarget == Vec3(0, 0, 0))
						{
							Matrix34 bombMat34 = pBomb->GetWorldTM();
							Vec3 zDir = bombMat34.GetColumn2();
							impulse.impulse = -zDir * fForce;
						}
						else
							impulse.impulse = vDir * fForce;

						pPhysEntity->Action(&impulse);
						return pBomb;
					}
				}
			}
		}
		return NULL;
	}
};

class CFlowNode_SetEntityMaterial : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Reset,
		EIP_Material,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_Material
	};

	CActor* m_pActor = 0;
	string sOldMaterial = "";
public:
	////////////////////////////////////////////////////
	CFlowNode_SetEntityMaterial(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SetEntityMaterial()
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
			InputPortConfig_AnyType("Reset", _HELP("")),
			InputPortConfig<string>("Material","", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("Done",_HELP("")),
			OutputPortConfig<string>("Material",_HELP("")),
			//OutputPortConfig<bool>("Disabled",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Set input entity material");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (const IEntity* pEntity = pActInfo->pEntity)
			{
				m_pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					if (m_pActor)
					{
						const char* strMaterial = GetPortString(pActInfo, EIP_Material);
						m_pActor->ReplaceMaterial(strMaterial);
						ActivateOutput(pActInfo, EOP_Done, pEntity->GetId());
						ActivateOutput(pActInfo, EOP_Material, (string)strMaterial);
					}
				}

				if (IsPortActive(pActInfo, EIP_Reset))
				{
					if (m_pActor)
						m_pActor->ReplaceMaterial(0);
				}
			}
		}
		break;
		case IFlowNode::eFE_Initialize:
			if (m_pActor)
				m_pActor->ReplaceMaterial(0);
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
		return new CFlowNode_SetEntityMaterial(pActInfo);
	}
};

class CFlowNode_GetSystemEnvironment : public CFlowBaseNode
{
	enum INPUTS
	{
		EIP_Get = 0,
	};

	enum OUTPUTS
	{
		EOP_Client = 0,
		EOP_Server,
		EOP_Editor,
		EOP_Dedicated,
		EOP_Remote,
	};

public:
	CFlowNode_GetSystemEnvironment(SActivationInfo* pActInfo) { }

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_Void("Get", _HELP("Gets the system environment parameters.")),
			{ 0 }
		};
		static const SOutputPortConfig out_ports[] =
		{
			OutputPortConfig_Void("Client", _HELP("Called if the engine is a client instance.")),
			OutputPortConfig_Void("Server", _HELP("Called if the engine is a server instance.")),
			OutputPortConfig_Void("Editor", _HELP("Called if the engine is a editor instance.")),
			OutputPortConfig_Void("Dedicated", _HELP("Called if the engine is a dedicated server instance.")),
			OutputPortConfig_Void("Remote", _HELP("Called if the engine is a client and not server instance.")),
			{ 0 }
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Gets system runtime environment parameters.");
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Get))
		{
			if (gEnv->bClient)
				ActivateOutput(pActInfo, EOP_Client, GetPortAny(pActInfo, EIP_Get));

			if (gEnv->bServer)
				ActivateOutput(pActInfo, EOP_Server, GetPortAny(pActInfo, EIP_Get));

			if (gEnv->bEditor)
				ActivateOutput(pActInfo, EOP_Editor, GetPortAny(pActInfo, EIP_Get));

			if (gEnv->bServer && !gEnv->bClient)
				ActivateOutput(pActInfo, EOP_Dedicated, GetPortAny(pActInfo, EIP_Get));

			if (!gEnv->bServer && gEnv->bClient)
				ActivateOutput(pActInfo, EOP_Remote, GetPortAny(pActInfo, EIP_Get));
		}
	}
};

//class CFlowNode_AIGotoSpeedStanceNew : public CFlowBaseNode, IGoalPipeListener
//{
//	enum EInputPorts
//	{
//		EIP_Start,
//		EIP_Cancel,
//		EIP_Pos,
//		EIP_StopDistance,
//		EIP_Run,
//		EIP_Stance
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Done,
//		EOP_Succeed,
//		EOP_Failed,
//	};
//
//public:
//
//	//IGoalPipeListener
//	void OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId) override
//	{
//		if (!pPipeUser)
//			return;
//
//		if (m_goalPipeId == goalPipeId)
//		{
//			switch (event)
//			{
//			case ePN_Deselected:
//			case ePN_OwnerRemoved:
//			{
//				if (m_started)
//					ActivateOutput(&m_actInfo, EOP_Failed, 1);
//			}
//			break;
//			case ePN_Suspended:
//				break;
//			case ePN_Resumed:
//				break;
//			case ePN_Finished:
//			case ePN_Removed:
//			{
//				if (m_started)
//					ActivateOutput(&m_actInfo, EOP_Succeed, 1);
//			}
//			break;
//			case ePN_AnimStarted:
//				break;
//			case ePN_RefPointMoved:
//				break;
//			default:
//				break;
//			}
//		}
//	}
//	//~IGoalPipeListener
//
//	////////////////////////////////////////////////////
//	CFlowNode_AIGotoSpeedStanceNew(SActivationInfo* pActInfo)
//	{
//		m_goalPipeId = 0;
//		m_started = false;
//		m_pUser = nullptr;
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_AIGotoSpeedStanceNew()
//	{
//		if (m_pUser)
//			m_pUser->UnRegisterGoalPipeListener(this, m_goalPipeId);
//	}
//	////////////////////////////////////////////////////
//	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetConfiguration(SFlowNodeConfig& config)
//	{
//		const char* ui_config1 = "enum_int:Idle=0,Alerted=1,Combat=2";
//		const char* ui_config2 = "enum_int:None=0,DisableCombatDuringAction=1,PausingActionWhenCombat=2";
//
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Start",false, _HELP("")),
//			InputPortConfig_AnyType("Cancel",false, _HELP("")),
//			InputPortConfig<Vec3>("Pos",Vec3(0), _HELP("")),
//			InputPortConfig<float>("StopDistance",0.0f, _HELP("")),
//			InputPortConfig<int>("run",0, _HELP("")),
//			//InputPortConfig<bool>("IgnoreCombat",false, _HELP("AI ignores combat when performing an action")),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig_AnyType("Started",_HELP("When activated Start port")),
//			OutputPortConfig_AnyType("Finished",_HELP("When ai success finish action")),
//			OutputPortConfig_AnyType("Failed",_HELP("When ai die")),
//			OutputPortConfig_AnyType("Stopped",_HELP("When activated Stop port")),
//			OutputPortConfig_AnyType("Paused",_HELP("")),
//			OutputPortConfig_AnyType("Unpaused",_HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("Causes the AI to perform an action based on input. This node is tracked by the AI Action Tracker (tos_debug_draw_ai...). Attention: the old AI:AIExecute node is not relevant");
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	////////////////////////////////////////////////////
//	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
//	{
//		switch (event)
//		{
//		case eFE_Activate:
//		{
//			auto pEntity = pActInfo->pEntity;
//			if (!pEntity)
//				return;
//
//			auto pAI = pEntity->GetAI();
//			if (!pAI)
//				return;
//
//			if (IsPortActive(pActInfo, EIP_Start))
//			{
//				auto pObject = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo, EIP_ObjectId));
//				if (!pObject)
//					return;
//
//				const char* actionName = GetPortString(pActInfo, EIP_Name);
//				const bool highPriority = GetPortBool(pActInfo, EIP_HighPriority);
//				const auto executingFlag = EAAEFlag(GetPortInt(pActInfo, EIP_ExecutingFlag));
//
//				int alertness = GetPortInt(pActInfo, EIP_MaxAlertness);
//				if (highPriority)
//					alertness += 100;
//
//				m_goalPipeId = GetPortInt(pActInfo, EIP_GoalPipeId);
//				m_goalPipeId = TOS_AI::ExecuteAIAction(pAI, pObject, actionName, alertness, m_goalPipeId, executingFlag, "CFlowNode_AIExecuteAction::ProcessEvent");
//
//				if (m_goalPipeId != -1)
//				{
//					m_started = true;
//					ActivateOutput(pActInfo, EOP_Started, 1);
//				}
//			}
//			else if (IsPortActive(pActInfo, EIP_Stop))
//			{
//				TOS_AI::AbortAIAction(pEntity->GetAI(), m_goalPipeId, "CFlowNode_AIExecuteAction::ProcessEvent");
//				ActivateOutput(pActInfo, EOP_Stopped, 1);
//			}
//		}
//		break;
//		case IFlowNode::eFE_Update:
//			break;
//		case IFlowNode::eFE_FinalActivate:
//			break;
//		case IFlowNode::eFE_Initialize:
//			m_actInfo = *pActInfo;
//			break;
//		case IFlowNode::eFE_FinalInitialize:
//			break;
//		case IFlowNode::eFE_SetEntityId:
//			break;
//		case IFlowNode::eFE_Suspend:
//			break;
//		case IFlowNode::eFE_Resume:
//			break;
//		case IFlowNode::eFE_ConnectInputPort:
//			break;
//		case IFlowNode::eFE_DisconnectInputPort:
//			break;
//		case IFlowNode::eFE_ConnectOutputPort:
//			break;
//		case IFlowNode::eFE_DisconnectOutputPort:
//			break;
//		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
//			break;
//		default:
//			break;
//		}
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetMemoryStatistics(ICrySizer* s)
//	{
//		s->Add(*this);
//	}
//
//	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
//	{
//		return new CFlowNode_AIGotoSpeedStanceNew(pActInfo);
//	}
//
//	bool m_started;
//	SActivationInfo m_actInfo;
//	int m_goalPipeId;
//	IPipeUser* m_pUser;
//};

class CFlowNode_IsInVehicle : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
	};

	enum EOutputPorts
	{
		EOP_IsDriver,
		EOP_IsGunner,
		EOP_IsPassenger,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_IsInVehicle(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_IsInVehicle(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("VehicleId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("IsGunner", _HELP("")),
			OutputPortConfig_AnyType("IsDriver", _HELP("")),
			OutputPortConfig_AnyType("IsPassenger", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Put the actor in vehicle fast");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
				return;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				const auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));
				if (pVehicle)
				{
					TOS_Vehicle::ActorIsDriver(pActor) ? ActivateOutput(pActInfo, EOP_IsDriver, 1) : void();
					TOS_Vehicle::ActorIsGunner(pActor) ? ActivateOutput(pActInfo, EOP_IsGunner, 1) : void();
					TOS_Vehicle::ActorIsPassenger(pActor) ? ActivateOutput(pActInfo, EOP_IsPassenger, 1) : void();
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
		return new CFlowNode_IsInVehicle(pActInfo);
	}
};

class CFlowNode_GetInterface : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync = 0,
	};

	enum EOutputPorts
	{
		EOP_IsVehicle,
		EOP_IsActor,
		EOP_IsItem,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetInterface(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetInterface(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("IsVehicle", _HELP("")),
			OutputPortConfig<EntityId>("IsActor", _HELP("")),
			OutputPortConfig<EntityId>("IsItem", _HELP("")),
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
			if (IsPortActive(pActInfo, EIP_Sync && pActInfo->pEntity))
			{
				const IEntity* pFlowEntity = pActInfo->pEntity;
				if (pFlowEntity)
				{
					const auto id = pFlowEntity->GetId();

					const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
					const auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
					const auto pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(id);

					if (pActor)
						ActivateOutput(pActInfo, EOP_IsActor, id);
					else if (pVehicle)
						ActivateOutput(pActInfo, EOP_IsVehicle, id);
					else if (pItem)
						ActivateOutput(pActInfo, EOP_IsItem, id);
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
		return new CFlowNode_GetInterface(pActInfo);
	}
};

class CFlowNode_VehicleExitNew : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_TransitionEnabled,
		EIP_Force,
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Fail,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleExitNew(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleExitNew(void)
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<bool>("TransitionEnabled", _HELP("")),
			InputPortConfig<bool>("Force", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Fail", _HELP("")),
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
			if (!pActInfo->pEntity)
				return;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				const auto pVehicle = TOS_Vehicle::GetVehicle(pActor);
				if (pVehicle)
				{
					const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
					if (pSeat)
					{
						const auto transEnabled = GetPortBool(pActInfo, EIP_TransitionEnabled);
						const auto force = GetPortBool(pActInfo, EIP_Force);

						if (pSeat->Exit(transEnabled, force))
						{
							ActivateOutput(pActInfo, EOP_Success, 1);
						}
						else
						{
							ActivateOutput(pActInfo, EOP_Fail, 1);
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleExitNew(pActInfo);
	}
};

//class CFlowNode_ActorGetCurrentItemAmmo : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Sync
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Ammo,
//	};
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_ActorGetCurrentItemAmmo(SActivationInfo* pActInfo)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_ActorGetCurrentItemAmmo()
//	{
//	}
//	////////////////////////////////////////////////////
//	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetConfiguration(SFlowNodeConfig& config)
//	{
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Sync", _HELP("")),
//
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<string>("Ammo",_HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("");
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	////////////////////////////////////////////////////
//	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
//	{
//		switch (event)
//		{
//		case eFE_Activate:
//		{
//			if (IsPortActive(pActInfo, EIP_Sync))
//			{
//				auto pEntity = pActInfo->pEntity;
//				if (pEntity)
//				{
//					const auto pEntityId = pEntity->GetId();
//					if (pEntityId)
//					{
//						auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
//						if (pActor)
//						{
//							auto pCurrentItem = pActor->GetCurrentItem();
//							if (pCurrentItem)
//							{
//								auto pWeapon = pCurrentItem->GetIWeapon();
//								if (pWeapon)
//								{
//									pWeapon->GetAmmoCount()
//								}
//							}
//
//							ActivateOutput(pActInfo, EOP_ItemId, pCurrentItem ? pCurrentItem->GetEntityId() : 0);
//						}
//					}
//				}
//			}
//		}
//		break;
//		}
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetMemoryStatistics(ICrySizer* s)
//	{
//		s->Add(*this);
//	}
//
//	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
//	{
//		return new CFlowNode_ActorGetCurrentItemAmmo(pActInfo);
//	}
//};


//REGISTER_FLOW_NODE("AI:AIReset", CFlowNode_AIReset);
//REGISTER_FLOW_NODE("AI:AIScoutDropBomb", CFlowNode_ScoutDropBomb);
//REGISTER_FLOW_NODE("AI:AIGetPerc", CFlowNode_AIGetPerc);
//REGISTER_FLOW_NODE("AI:AIGotoSpeedStanceNew", CFlowNode_AIGotoSpeedStanceNew);
REGISTER_FLOW_NODE("Animations:PlayAction", CFlowNode_PlayAction);
REGISTER_FLOW_NODE("AI:AIEvent", CFlowNode_AIEvent);
//REGISTER_FLOW_NODE("AI:AIGetTarget", CFlowNode_AIGetTarget);
REGISTER_FLOW_NODE("AI:AIEnableCombat", CFlowNode_AIEnableCombat);
REGISTER_FLOW_NODE("AI:AIExecuteActionNew", CFlowNode_AIExecuteAction);
REGISTER_FLOW_NODE("AI:AIPauseAction", CFlowNode_PauseAIAction);
REGISTER_FLOW_NODE("AI:AIAbortAction", CFlowNode_AbortAIAction);
REGISTER_FLOW_NODE("AI:AIGetActionFlag", CFlowNode_GetActionFlag);
//REGISTER_FLOW_NODE("AI:AISetRefPos", CFlowNode_SetRefPoint);
REGISTER_FLOW_NODE("AI:AISelectGoalPipe", CFlowNode_AISelectGoalPipe);
REGISTER_FLOW_NODE("AI:AIIsUsingPipe", CFlowNode_AIIsUsingPipe);
//REGISTER_FLOW_NODE("AI:AIFollowPathNew", CFlowNode_NewFollowPath);
REGISTER_FLOW_NODE("AI:AIGetSpecies", CFlowNode_AIGetSpecies);

REGISTER_FLOW_NODE("Entity:SetMaterial", CFlowNode_SetEntityMaterial);
REGISTER_FLOW_NODE("Entity:GetEntityByName", CFlowNode_GetEntityIdByName); //
REGISTER_FLOW_NODE("Entity:EnableDisable", CFlowNode_EntityEnableDisable); //
REGISTER_FLOW_NODE("Entity:CheckName", CFlowNode_EntityCheckName); //
REGISTER_FLOW_NODE("Entity:RemoveEntity", CFlowNode_RemoveEntity);
REGISTER_FLOW_NODE("Entity:CheckDistance", CFlowNode_EntityCheckDistance);
REGISTER_FLOW_NODE("Entity:CallScriptFunction", CFlowNode_CallScriptFunction);
REGISTER_FLOW_NODE("Entity:SetScriptValue", CFlowNode_SetScriptValue);
REGISTER_FLOW_NODE("Entity:GetScriptValue", CFlowNode_GetScriptValue);
//REGISTER_FLOW_NODE("Entity:SwitchView", CFlowNode_SwitchView);//
REGISTER_FLOW_NODE("Entity:GetBounds", CFlowNode_GetEntityBounds);
REGISTER_FLOW_NODE("Entity:GetInterface", CFlowNode_GetInterface);

REGISTER_FLOW_NODE("Debug:DrawBox", CFlowNode_DrawBox);
REGISTER_FLOW_NODE("Debug:DrawRay", CFlowNode_DrawRay);
//REGISTER_FLOW_NODE("Entity:GetPosFromMap", CFlowNode_GetPosFromMap);

//Inventory
REGISTER_FLOW_NODE("Inventory:ActorRemoveAllItems", CFlowNode_ActorRemoveAllItems);
REGISTER_FLOW_NODE("Inventory:ActorRemoveItem", CFlowNode_ActorRemoveItem);
REGISTER_FLOW_NODE("Inventory:ActorAddItem", CFlowNode_ActorAddItem);
REGISTER_FLOW_NODE("Inventory:ActorPickupItem", CFlowNode_ActorPickupItem);
REGISTER_FLOW_NODE("Inventory:ActorGetCurrentItem", CFlowNode_ActorGetCurrentItem);
//REGISTER_FLOW_NODE("Inventory:ActorGetCurrentItemAmmo", CFlowNode_ActorGetCurrentItemAmmo);

REGISTER_FLOW_NODE("Game:ActorLowerWeapon", CFlowNode_ActorLowerWeapon);
REGISTER_FLOW_NODE("Game:ActorGetMaxHealth", CFlowNode_GetMaxHealth); //
REGISTER_FLOW_NODE("Game:ActorGetItemId", CFlowNode_GetActorItemId);
REGISTER_FLOW_NODE("Game:ActorSetModel", CFlowNode_ActorSetModel);
REGISTER_FLOW_NODE("Game:ActorHumanMode", CFlowNode_ActorHumanMode);
REGISTER_FLOW_NODE("Game:ActorOnlyTPMode", CFlowNode_ActorOnlyTPMode);
REGISTER_FLOW_NODE("Game:ActorKill", CFlowNode_ActorKill);

REGISTER_FLOW_NODE("Sound:PlayAISoundEvent", CFlowNode_PlayAISoundEvent);
REGISTER_FLOW_NODE("System:GetEnvironment", CFlowNode_GetSystemEnvironment);
REGISTER_FLOW_NODE("String:CheckStrings", CFlowNode_StringCheck);

REGISTER_FLOW_NODE("Material:GetMaterial", CFlowNode_GetMaterial);
REGISTER_FLOW_NODE("Math:SetInteger", CFlowNode_SetInteger);
REGISTER_FLOW_NODE("Math:Round", CFlowNode_Round);
REGISTER_FLOW_NODE("Math:Calculate", CFlowNode_Calculate);
REGISTER_FLOW_NODE("Math:InRange", CFlowNode_InRange);

//REGISTER_FLOW_NODE("Vehicle:GetMaxDamage", CFlowNode_VehicleGetMaxDamage);
REGISTER_FLOW_NODE("Vehicle:GetFreeSeat", CFlowNode_VehicleGetFreeSeat);
//REGISTER_FLOW_NODE("Vehicle:TurnLights", CFlowNode_VehicleLightsOn);
//REGISTER_FLOW_NODE("Vehicle:DebugDraw", CFlowNode_VehicleDebugDraw);
REGISTER_FLOW_NODE("Vehicle:EnterNew", CFlowNode_VehicleEnterNew);
REGISTER_FLOW_NODE("Vehicle:GetSpeed", CFlowNode_VehicleStatus);
REGISTER_FLOW_NODE("Vehicle:ExitNew", CFlowNode_VehicleExitNew);
REGISTER_FLOW_NODE("Vehicle:ChangeSeat", CFlowNode_VehicleChangeSeat);
REGISTER_FLOW_NODE("Vehicle:IsInVehicle", CFlowNode_IsInVehicle);

REGISTER_FLOW_NODE("Render:VisionParams", CFlowNode_VisionParams);