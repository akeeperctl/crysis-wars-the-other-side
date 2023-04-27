/*************************************************************************
Copyright (C), CryTechLab, 2021.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 17:03:2021   12.28: Created by Akeeper

*************************************************************************/

#include "StdAfx.h"

#include "TOSNodesIncludes.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"


class CFlowNode_ManageAbility : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_AddAbility,
		EIP_RemoveAbility,
		EIP_Name,
	};

	enum EOutputPorts
	{
		EOP_Added,
		EOP_Removed,
		EOP_Name,
		EOP_Index,
		EOP_EntityId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_ManageAbility(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ManageAbility()
	{
		
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:0=0,1=1,2=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Add", _HELP("Add ability to the actor")),
			InputPortConfig_AnyType("Remove", "Remove ability from the actor"),
			InputPortConfig<string>("Name",0 , _HELP(""),"Name"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Added",_HELP("")),
			OutputPortConfig_AnyType("Removed",_HELP("")),
			OutputPortConfig<string>("Name",_HELP(""),"Name"),
			OutputPortConfig<int>("Index",_HELP(""),"Index"),
			OutputPortConfig<EntityId>("EntityId",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = 
			_HELP("Add a ability to the actor. The actor must be ability owner, if he is'n then the ability system will create them. And also this node will reset the ability system in the level load moment");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			//Moved to CGame::EditorResetGame
			//if (g_pControlSystem->GetAbilitiesSystem())
			//	g_pControlSystem->GetAbilitiesSystem()->Reset();
		}
			break;
		case eFE_Activate:
		{
			CAbilitiesSystem* pAbSys = g_pControlSystem->GetAbilitiesSystem();
			if (pAbSys)
			{
				if (!pActInfo->pEntity)
					return;

				const CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId()));
				if (pActor)
				{
					if (IsPortActive(pActInfo,EIP_AddAbility))
					{
						if (!pAbSys->IsAbilityOwner(pActor))
							pAbSys->AddAbilityOwner(pActor);

						if (pAbSys->IsAbilityOwner(pActor))
						{
							const string abilityName = GetPortString(pActInfo, EIP_Name);

							const auto pOwner = pAbSys->GetAbilityOwner(pActor);
							if (pOwner)
							{
								pOwner->AddAbility(abilityName);

								const auto pAbility = pOwner->GetAbility(abilityName);
								if (pAbility)
								{
									ActivateOutput(pActInfo, EOP_EntityId, pOwner->GetEntityId());
									ActivateOutput(pActInfo, EOP_Index, pAbility->index);
									ActivateOutput(pActInfo, EOP_Name, pAbility->name);
									ActivateOutput(pActInfo, EOP_Added, 1);
								}
							}
						}
					}
					else if (IsPortActive(pActInfo,EIP_RemoveAbility))
					{
						if (!pAbSys->IsAbilityOwner(pActor))
							return;

						const auto pOwner = pAbSys->GetAbilityOwner(pActor);
						if (pOwner)
						{
							const string abilityName = GetPortString(pActInfo, EIP_Name);

							const auto pAbility = pOwner->GetAbility(abilityName);
							if (pAbility)
							{
								const int abilityIndex = pAbility->index;

								ActivateOutput(pActInfo, EOP_EntityId, pOwner->GetEntityId());
								ActivateOutput(pActInfo, EOP_Index, abilityIndex);
								ActivateOutput(pActInfo, EOP_Name, abilityName);

								pOwner->RemoveAbility(abilityName);

								if (!pOwner->IsHaveAbility(abilityName))
									ActivateOutput(pActInfo, EOP_Removed, 1);
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
		return new CFlowNode_ManageAbility(pActInfo);
	}
};

class CFlowNode_GetAbilityInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Name,
		EIP_Index,
	};

	enum EOutputPorts
	{
		EOP_Name,
		EOP_Index,
		EOP_State,
		EOP_VisibleMode,
		EOP_EntityId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetAbilityInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetAbilityInfo()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:0=0,1=1,2=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<string>("Name",string("") , _HELP("Write the name of a ability"),"Name"),
			InputPortConfig<int>("Index",0 , _HELP("Write the index of a ability"),"Index"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<string>("Name",_HELP(""),"Name"),
			OutputPortConfig<int>("Index",_HELP(""),"Index"),
			OutputPortConfig<int>("State",_HELP("1=ReadyToActivate, 2=Activated, 4=Cooldown, 8=DisabledByCondition"),"State"),
			OutputPortConfig<int>("VisibleMode",_HELP("1=Hide, 2=Show, 4=Disabled, 8=Activated"),"Mode"),
			OutputPortConfig<EntityId>("OwnerId",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = 
			_HELP("Add a ability to the actor. The actor must be ability owner!");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			//if (g_pControlSystem->GetAbilitiesSystem())
			//	g_pControlSystem->GetAbilitiesSystem()->Reset();
		}
		break;
		case eFE_Activate:
		{
			CAbilitiesSystem* pAbSys = g_pControlSystem->GetAbilitiesSystem();
			if (pAbSys)
			{
				if (!pActInfo->pEntity)
					return;

				const CActor* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId()));
				if (pActor)
				{
					if (IsPortActive(pActInfo, EIP_Sync))
					{
						if (!pAbSys->IsAbilityOwner(pActor))
							return;

						const auto pOwner = pAbSys->GetAbilityOwner(pActor);
						if (pOwner)
						{
							const string abilityName = GetPortString(pActInfo, EIP_Name);
							const int abilityIndex = GetPortInt(pActInfo, EIP_Index);
							bool isFoundByName = false;

							auto pAbility = pOwner->GetAbility(abilityName);

							if (pAbility)
							{
								ActivateOutput(pActInfo, EOP_EntityId, pAbility->abilityOwnerId);
								ActivateOutput(pActInfo, EOP_Index, pAbility->index);
								ActivateOutput(pActInfo, EOP_Name, pAbility->name);
								ActivateOutput(pActInfo, EOP_State, (int)pAbility->state);
								ActivateOutput(pActInfo, EOP_VisibleMode, (int)pAbility->visMode);
								isFoundByName = true;
							}

							pAbility = pOwner->GetAbility(abilityIndex);

							if (!isFoundByName && pAbility)
							{
								ActivateOutput(pActInfo, EOP_EntityId, pAbility->abilityOwnerId);
								ActivateOutput(pActInfo, EOP_Index, pAbility->index);
								ActivateOutput(pActInfo, EOP_Name, pAbility->name);
								ActivateOutput(pActInfo, EOP_State, (int)pAbility->state);
								ActivateOutput(pActInfo, EOP_VisibleMode, (int)pAbility->visMode);
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
		return new CFlowNode_GetAbilityInfo(pActInfo);
	}
};

//Create Squad
REGISTER_FLOW_NODE("AbilitySystem:ManageAbility", CFlowNode_ManageAbility);
REGISTER_FLOW_NODE("AbilitySystem:GetAbilityInfo", CFlowNode_GetAbilityInfo);