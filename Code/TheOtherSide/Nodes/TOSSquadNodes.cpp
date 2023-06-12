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
#include "TheOtherSide/Squad/SquadSystem.h"

namespace CtrlSys
{
	SSquadSystem* GetSquadSystem()
	{
		if (g_pControlSystem->GetControlClient())
		{
			SSquadSystem* pSquadSystem = g_pControlSystem->pSquadSystem;
			return pSquadSystem;
		}
		return 0;
	}
}

//class CFlowNode_SquadSystemToggle : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Enable,
//		EIP_Disable,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Enabled,
//		EOP_Disabled,
//	};
//
//	bool toggle;
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_SquadSystemToggle(SActivationInfo* pActInfo)
//	{
//		toggle = false;
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_SquadSystemToggle()
//	{
//		toggle = 0;
//	}
//	////////////////////////////////////////////////////
//	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
//	{
//		SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
//		if (pSquadSystem)
//		{
//			ser.BeginGroup("SSquadSystemToggleFG");
//			if (ser.IsWriting())
//			{
//				//toggle = pSquadSystem->IsEnabled();
//				ser.Value("m_bEnabled", toggle);
//			}
//			else
//			{
//				ser.Value("m_bEnabled", toggle);
//				//pSquadSystem->ToggleSystem(toggle);
//			}
//			ser.EndGroup();
//		}
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetConfiguration(SFlowNodeConfig& config)
//	{
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Enable", _HELP("")),
//			InputPortConfig_AnyType("Disable", _HELP("")),
//
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig_AnyType("Enable",_HELP("")),
//			OutputPortConfig_AnyType("Disable",_HELP("")),
//			{0}
//		};
//
//		//config.nFlags |= EFLN_TARGET_ENTITY;
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
//		case eFE_Initialize:
//		{
//			SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
//			if (pSquadSystem)
//				pSquadSystem->ToggleSystem(false);
//		}
//		break;
//
//		case eFE_Activate:
//		{
//			SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
//			if (pSquadSystem)
//			{
//				if (IsPortActive(pActInfo, EIP_Enable))
//				{
//					if (!pSquadSystem->IsEnabled())
//					{
//						pSquadSystem->ToggleSystem(true);
//						//ActivateOutput(pActInfo, EOP_Enabled, true);
//
//						if (pSquadSystem->IsEnabled())
//							ActivateOutput(pActInfo, EOP_Enabled, true);
//					}
//				}
//				else if (IsPortActive(pActInfo, EIP_Disable))
//				{
//					if (pSquadSystem->IsEnabled())
//					{
//						pSquadSystem->ToggleSystem(false);
//
//						if (!pSquadSystem->IsEnabled())
//							ActivateOutput(pActInfo, EOP_Disabled, true);
//					}
//				}
//			}
//		}
//		break;
//
//		case eFE_Update:
//		{
//			//Use this case to display the health and energy info of each member in the player squad
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
//		return new CFlowNode_SquadSystemToggle(pActInfo);
//	}
//};

class CFlowNode_MemberExecuteOrder : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_RefPoint,
		EIP_Order,
	};

	enum EOutputPorts
	{
		EOP_Executed,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_MemberExecuteOrder(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_MemberExecuteOrder()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config_orders = "enum_int:Goto=0,Search=1,Follow=2,None=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Trigger", _HELP("")),
			InputPortConfig<Vec3>("RefPoint", _HELP("")),
			InputPortConfig<int>("Order", 0, _HELP(""),0,ui_config_orders),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Executed",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;//The squad member entity
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
			{
				ActivateOutput(pActInfo, EOP_Executed, false);
				return;
			}

			IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pMember)
			{
				ActivateOutput(pActInfo, EOP_Executed, false);
				return;
			}

			SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
			if (pSquadSystem)
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					ESquadOrders eSquadOrder = ESquadOrders(GetPortInt(pActInfo, EIP_Order));
					Vec3 vRefPoint = GetPortVec3(pActInfo, EIP_RefPoint);

					SSquad squad = pSquadSystem->GetSquadFromMember(pMember, 1);
					if (squad.GetLeader() != 0)
					{
						SMember member = squad.GetMember(pMember);
						bool result = squad.ExecuteOrderFG(eSquadOrder, member, vRefPoint);

						ActivateOutput(pActInfo, EOP_Executed, result);
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
		return new CFlowNode_MemberExecuteOrder(pActInfo);
	}
};

class CFlowNode_MemberGetInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Index,
	};

	enum EOutputPorts
	{
		EOP_EntityId,
		EOP_Order,
		EOP_SearchPos,
		EOP_GuardPos
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_MemberGetInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_MemberGetInfo()
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
			InputPortConfig<int>("Slot",0, _HELP(""),"Index"),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("EntityId",_HELP("")),
			OutputPortConfig<int>("Order",_HELP("Gets the member current order, Goto=0,Search=1,Follow=2,None=3")),
			OutputPortConfig<Vec3>("SearchPos",_HELP("")),
			OutputPortConfig<Vec3>("GuardPos",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;//The squad leader entity
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Gets the member info by its index from input leader entity squad");
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
			{
				//ActivateOutput(pActInfo, EOP_Executed, false);
				return;
			}

			IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pLeader)
			{
				//ActivateOutput(pActInfo, EOP_Executed, false);
				return;
			}

			if (IsPortActive(pActInfo, EIP_Trigger))
			{
				if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
				{
					SSquad& squad = pSquadSystem->GetSquadFromLeader(pLeader);
					if (squad.GetLeader() != 0)
					{
						int index = GetPortInt(pActInfo, EIP_Index);

						SMember& member = squad.GetMemberFromIndex(index);
						if (!squad.GetActor(member.actorId))
							return;

						ActivateOutput(pActInfo, EOP_EntityId, squad.GetActor(member.actorId)->GetEntityId());
						ActivateOutput(pActInfo, EOP_Order, int(member.currentOrder));
						ActivateOutput(pActInfo, EOP_SearchPos, member.searchPos);
						ActivateOutput(pActInfo, EOP_GuardPos, member.guardPos);
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
		return new CFlowNode_MemberGetInfo(pActInfo);
	}
};

class CFlowNode_SquadSetLeader : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_LeaderId,
		EIP_SquadId,
	};

	enum EOutputPorts
	{
		EOP_EntityId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadSetLeader(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadSetLeader()
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
			InputPortConfig<EntityId>("LeaderId", 0, _HELP("")),
			InputPortConfig<int>("SquadId", 0, _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("LeaderId",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Sets the leader of the squad which are getting form SquadId");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					int squadId = GetPortInt(pActInfo, EIP_SquadId);

					SSquad squad = pSquadSystem->GetSquadFromId(squadId);

					IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetPortEntityId(pActInfo, EIP_LeaderId));

					if (squad.GetLeader() != 0)
					{
						squad.SetLeader(pLeader);

						ActivateOutput(pActInfo, EOP_EntityId, squad.GetLeader()->GetEntityId());
					}

					/*if (pSquadSystem->IsEnabled())
					{
						if (IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(leaderId))
						{
							pSquadSystem->SetLeader(pLeader);

							if (pSquadSystem->GetLeader() == pLeader)
								ActivateOutput(pActInfo, EOP_EntityId, leaderId);
						}
					}*/
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
		return new CFlowNode_SquadSetLeader(pActInfo);
	}
};

class CFlowNode_SquadGetLeader : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_SquadId
	};

	enum EOutputPorts
	{
		EOP_EntityId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadGetLeader(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadGetLeader()
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
			OutputPortConfig<EntityId>("LeaderId",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Gets the current leader of the squad");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					int squadId = GetPortInt(pActInfo, EIP_SquadId);

					SSquad squad = pSquadSystem->GetSquadFromId(squadId);

					if (squad.GetLeader())
						ActivateOutput(pActInfo, EOP_EntityId, squad.GetLeader()->GetEntityId());

					/*if (pSquadSystem->IsEnabled())
						ActivateOutput(pActInfo, EOP_EntityId, pSquadSystem->GetLeader()->GetEntityId());*/
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
		return new CFlowNode_SquadGetLeader(pActInfo);
	}
};

class CFlowNode_SquadGetMembersCount : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_IncludeLeader,
		EIP_SquadId
	};

	enum EOutputPorts
	{
		EOP_Count,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadGetMembersCount(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadGetMembersCount()
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
			InputPortConfig<bool>("IncludeLeader",0 , _HELP("")),
			InputPortConfig<int>("SquadId",0 , _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Count",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
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
			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					int squadId = GetPortInt(pActInfo, EIP_SquadId);
					SSquad squad = pSquadSystem->GetSquadFromId(squadId);
					if (squad.GetLeader() != 0)
					{
						int count = squad.GetMembersCount();

						if (GetPortBool(pActInfo, EIP_IncludeLeader))
							count++;

						ActivateOutput(pActInfo, EOP_Count, count);
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
		return new CFlowNode_SquadGetMembersCount(pActInfo);
	}
};

class CFlowNode_SquadGetFreeIndex : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_SquadId,
	};

	enum EOutputPorts
	{
		EOP_Slot,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadGetFreeIndex(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadGetFreeIndex()
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
			InputPortConfig<int>("SquadId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Slot",_HELP("Return -1 if free slot not found"),"Index"),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
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
			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					int squadId = GetPortInt(pActInfo, EIP_SquadId);
					SSquad squad = pSquadSystem->GetSquadFromId(squadId);
					if (squad.GetLeader() != 0)
					{
						ActivateOutput(pActInfo, EOP_Slot, squad.GetFreeMemberIndex());
					}

					/*if (pSquadSystem->IsEnabled())
					{
						int slot = pSquadSystem->GetFreeSlot();
						ActivateOutput(pActInfo, EOP_Slot, slot);
					}*/
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
		return new CFlowNode_SquadGetFreeIndex(pActInfo);
	}
};

//class CFlowNode_SquadGetEnabled : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Trigger,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Enabled,
//		EOP_Disabled,
//	};
//
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_SquadGetEnabled(SActivationInfo* pActInfo)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_SquadGetEnabled()
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
//			InputPortConfig_Void("Trigger", _HELP("")),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<bool>("Enabled",_HELP("")),
//			OutputPortConfig<bool>("Disabled",_HELP("")),
//			{0}
//		};
//
//		//config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("Get enabled/disabled state from the Squad System");
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
//			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
//			{
//				if (IsPortActive(pActInfo, EIP_Trigger))
//					ActivateOutput(pActInfo, pSquadSystem->IsEnabled() ? EOP_Enabled : EOP_Disabled, 1);
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
//	/*virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
//	{
//		return new CFlowNode_SquadGetEnabled(pActInfo);
//	}*/
//};

class CFlowNode_SquadCreate : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Create,
	};

	enum EOutputPorts
	{
		EOP_Created,
		EOP_LeaderId,
		EOP_SquadId,
	};

	SSquad m_squad = SSquad();

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadCreate(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadCreate()
	{
		if (SSquadSystem* pSS = CtrlSys::GetSquadSystem())
		{
			pSS->RemoveSquad(m_squad);
		}
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
			InputPortConfig_Void("Create", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Created",_HELP("")),
			OutputPortConfig<int>("LeaderId",_HELP("")),
			OutputPortConfig<int>("SquadId",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Input entity - leader actor");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			if (SSquadSystem* pSS = CtrlSys::GetSquadSystem())
			{
				pSS->RemoveSquad(m_squad);
			}
		}
		break;
		case eFE_Activate:
		{
			if (!pActInfo->pEntity)
			{
				ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pLeader)
			{
				ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Create))
				{
					SSquad squad;
					squad.SetLeader(pLeader);
					bool created = pSquadSystem->CreateSquad(squad);

					ActivateOutput(pActInfo, EOP_Created, created);
					ActivateOutput(pActInfo, EOP_LeaderId, squad.GetLeader()->GetEntityId());
					ActivateOutput(pActInfo, EOP_SquadId, squad.GetId());

					m_squad = squad;
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
		return new CFlowNode_SquadCreate(pActInfo);
	}
};

class CFlowNode_SquadGetInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
	};

	enum EOutputPorts
	{
		EOP_LeaderId,
		EOP_SquadId,
		EOP_MemberCount,
		EOP_MemberIndex,
		EOP_LeaderOrder,
		EOP_IsPlayerLeader,
		EOP_IsPlayerMember,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadGetInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadGetInfo()
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
			InputPortConfig_Void("Get", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("LeaderId",_HELP("")),
			OutputPortConfig<int>("SquadId",_HELP("")),
			OutputPortConfig<int>("MemberCount",_HELP("")),
			OutputPortConfig<int>("MemberIndex",_HELP("Gets a free member index"),"MemberIndex"),
			OutputPortConfig<int>("LeaderOrder",_HELP("Gets the squad leader's current order, 0 - goto, 1 - search, 2 - follow, 3 - none")),
			OutputPortConfig<bool>("IsPlayerLeader",_HELP("")),
			OutputPortConfig<bool>("IsPlayerMember",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;//Leader entity
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Input entity - leader actor");
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
			{
				//ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pLeader)
			{
				//ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			if (SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					SSquad& squad = pSquadSystem->GetSquadFromLeader(pLeader);
					if (squad.GetLeader() != 0)
					{
						ActivateOutput(pActInfo, EOP_LeaderId, squad.GetLeader()->GetEntityId());
						ActivateOutput(pActInfo, EOP_SquadId, squad.GetId());
						ActivateOutput(pActInfo, EOP_MemberCount, squad.GetMembersCount());
						ActivateOutput(pActInfo, EOP_MemberIndex, squad.GetFreeMemberIndex());
						ActivateOutput(pActInfo, EOP_LeaderOrder, int(squad.GetOrderLeader()));
						ActivateOutput(pActInfo, EOP_IsPlayerLeader, squad.isPlayerLeader());
						ActivateOutput(pActInfo, EOP_IsPlayerMember, squad.isPlayerMember());
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
		return new CFlowNode_SquadGetInfo(pActInfo);
	}
};

class CFlowNode_SquadMembersManage : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_AddMember,
		EIP_RemoveMember,
		EIP_SquadId,
	};

	enum EOutputPorts
	{
		EOP_Added,
		EOP_Removed,
		EOP_Index,
		EOP_EntityId,
		EOP_SquadId,
	};

	//std::vector<EntityId> m_members;

public:
	////////////////////////////////////////////////////
	CFlowNode_SquadMembersManage(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SquadMembersManage()
	{
		//m_members.clear();
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		/*SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
		if (pSquadSystem && pSquadSystem->IsEnabled())
		{
			ser.BeginGroup("SquadMembersFG");
			if (ser.IsWriting())
			{
				m_members = pSquadSystem->GetMembersArray();
				ser.Value("m_iSquadSlotMembers", m_members);
			}
			else
			{
				ser.Value("m_iSquadSlotMembers", m_members);
				pSquadSystem->SetMembersArray(m_members);
			}
			ser.EndGroup();
		}*/
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:0=0,1=1,2=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Add", _HELP("Add actor to the squad")),
			InputPortConfig_AnyType("Remove", _HELP("Remove actor from the squad")),
			InputPortConfig<int>("SquadId",0 , _HELP("Write a squad number"),"SquadId"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Added",_HELP("")),
			OutputPortConfig_AnyType("Removed",_HELP("")),
			OutputPortConfig<int>("Slot",_HELP(""),"Index"),
			OutputPortConfig<EntityId>("EntityId",_HELP("")),
			OutputPortConfig<int>("SquadId",_HELP("")),

			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Add member to the squad. Squad we getting from the SquadId input port");
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
			{
				//ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pMember)
			{
				//ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			SSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
			if (pSquadSystem)
			{
				int squadId = GetPortInt(pActInfo, EIP_SquadId);
				if (squadId < 0)
					return;

				SSquad& squad = pSquadSystem->GetSquadFromId(squadId);
				if (squad.GetLeader() != 0)
				{
					SMember member;
					member.actorId = pMember->GetEntityId();

					if (IsPortActive(pActInfo, EIP_AddMember))
					{
						//Исправить недобавляемость в массив
						//Исправлено: Нужно добавить & к типу возврата у функций получения отряда/члена
						//CryLogAlways("squad.AddMember(member)");
						bool result = squad.AddMember(member);
						ActivateOutput(pActInfo, EOP_Added, result);
						ActivateOutput(pActInfo, EOP_Index, squad.GetIndexFromMember(pMember));
						ActivateOutput(pActInfo, EOP_SquadId, squad.GetId());
					}
					else if (IsPortActive(pActInfo, EIP_RemoveMember))
					{
						bool result = squad.RemoveMember(member);
						ActivateOutput(pActInfo, EOP_Removed, result);
					}

					ActivateOutput(pActInfo, EOP_EntityId, pMember->GetEntityId());
				}

				/*if (pSquadSystem->IsEnabled())
				{
					if (IsPortActive(pActInfo, EIP_AddMember))
					{
						if (IEntity* pEntity = pActInfo->pEntity)
						{
							EntityId id = pEntity->GetId();

							if (IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
							{
								pSquadSystem->AddMember(pMember, GetPortInt(pActInfo, EIP_Slot));

								if (pSquadSystem->IsMemberFromActor(pMember))
								{
									ActivateOutput(pActInfo, EOP_Added, true);
									ActivateOutput(pActInfo, EOP_Slot, GetPortInt(pActInfo, EIP_Slot));
									ActivateOutput(pActInfo, EOP_EntityId, id);
								}
							}
						}
					}
					else if (IsPortActive(pActInfo, EIP_RemoveMember))
					{
						if (IEntity* pEntity = pActInfo->pEntity)
						{
							EntityId id = pEntity->GetId();

							if (IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
							{
								pSquadSystem->RemoveMember(pMember);

								if (!pSquadSystem->IsMemberFromActor(pMember))
								{
									ActivateOutput(pActInfo, EOP_Removed, true);
									ActivateOutput(pActInfo, EOP_Slot, GetPortInt(pActInfo, EIP_Slot));
									ActivateOutput(pActInfo, EOP_EntityId, id);
								}
							}
						}
					}
				}*/
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
		return new CFlowNode_SquadMembersManage(pActInfo);
	}
};

//Squad System
//REGISTER_FLOW_NODE("Squad:Toggle", CFlowNode_SquadSystemToggle);
//REGISTER_FLOW_NODE_SINGLETON("Squad:GetEnabled", CFlowNode_SquadGetEnabled);

//New Squad System

//Create Squad
REGISTER_FLOW_NODE("SquadSystem:SquadCreate", CFlowNode_SquadCreate);

//Get Squad info
REGISTER_FLOW_NODE("SquadSystem:SquadGetInfo", CFlowNode_SquadGetInfo);

// Add/Remove members to squad
REGISTER_FLOW_NODE("SquadSystem:SquadMembers", CFlowNode_SquadMembersManage);

//Set Squad Leader
REGISTER_FLOW_NODE("SquadSystem:SquadSetLeader", CFlowNode_SquadSetLeader);

//Get Squad Leader
REGISTER_FLOW_NODE("SquadSystem:SquadGetLeader", CFlowNode_SquadGetLeader);

//Get Squad Free Member index
REGISTER_FLOW_NODE("SquadSystem:SquadGetMemberFreeIndex", CFlowNode_SquadGetFreeIndex);

//Get Squad members count
REGISTER_FLOW_NODE("SquadSystem:SquadGetMembersCount", CFlowNode_SquadGetMembersCount);

//Member nodes
REGISTER_FLOW_NODE("SquadSystem:MemberExecuteOrder", CFlowNode_MemberExecuteOrder);
REGISTER_FLOW_NODE("SquadSystem:MemberGetInfo", CFlowNode_MemberGetInfo);