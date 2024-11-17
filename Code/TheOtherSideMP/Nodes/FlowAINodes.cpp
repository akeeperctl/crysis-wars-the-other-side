#include "StdAfx.h"

#include "Game.h"
#include "Nodes/G2FlowBaseNode.h"

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
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AISelectGoalPipe(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

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
		config.sDescription = _HELP("Change AI's current goalpipe");
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

REGISTER_FLOW_NODE("AI:AISelectGoalPipe", CFlowNode_AISelectGoalPipe);