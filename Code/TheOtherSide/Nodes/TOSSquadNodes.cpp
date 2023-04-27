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
#include "TheOtherSide/Helpers/TOS_Vehicle.h"

const char* g_ui_config_orders = "enum_int:Guard=0,SearchEnemy=1,FollowLeader=2,ShootPrim=3,ShootSec=4,EnterVeh=5,ExitVeh=6,PrimPickup=7,SecPickup=8,UseVehTurret=9,SearchCover=18";

namespace CtrlSys
{
	CSquadSystem* GetSquadSystem()
	{
		if (g_pControlSystem->GetLocalControlClient())
		{
			CSquadSystem* pSquadSystem = g_pControlSystem->GetSquadSystem();
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

//class CFlowNode_MemberExecuteOrder : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Trigger,
//		EIP_RefPoint,
//		EIP_Order,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Executed,
//	};
//
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_MemberExecuteOrder(SActivationInfo* pActInfo)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_MemberExecuteOrder()
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
//		const char* ui_config_orders = "enum_int:Goto=0,Search=1,Follow=2,None=3";
//
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Trigger", _HELP("")),
//			InputPortConfig<Vec3>("RefPoint", _HELP("")),
//			InputPortConfig<int>("Order", 0, _HELP(""),0,ui_config_orders),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<bool>("Executed",_HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;//The squad member entity
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
//			if (!pActInfo->pEntity)
//			{
//				ActivateOutput(pActInfo, EOP_Executed, false);
//				return;
//			}
//
//			IActor* pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
//			if (!pMember)
//			{
//				ActivateOutput(pActInfo, EOP_Executed, false);
//				return;
//			}
//
//			CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
//			if (pSquadSystem)
//			{
//				if (IsPortActive(pActInfo, EIP_Trigger))
//				{
//					ESquadOrders eSquadOrder = ESquadOrders(GetPortInt(pActInfo, EIP_Order));
//					Vec3 vRefPoint = GetPortVec3(pActInfo, EIP_RefPoint);
//
//					auto pSquad = pSquadSystem->GetSquadFromMember(pMember, 1);
//					if (pSquad->GetLeader() != 0)
//					{
//						auto* member = pSquad->GetMemberInstance(pMember);
//						bool result = pSquad->ExecuteOrderFG(eSquadOrder, *member, vRefPoint);
//
//						ActivateOutput(pActInfo, EOP_Executed, result);
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
//		return new CFlowNode_MemberExecuteOrder(pActInfo);
//	}
//};

//class CFlowNode_GetMemberInfo : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Trigger,
//		EIP_Index,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_EntityId,
//		EOP_Order,
//		EOP_SearchPos,
//		EOP_GuardPos
//	};
//
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_GetMemberInfo(SActivationInfo* pActInfo)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_GetMemberInfo()
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
//			InputPortConfig_AnyType("Get", _HELP("")),
//			InputPortConfig<int>("Slot",0, _HELP("Squad member index"), "MemberIndex"),
//
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<EntityId>("EntityId",_HELP("")),
//			OutputPortConfig<int>("Order",_HELP("Gets the member current order, Goto=0,Search=1,Follow=2,None=3")),
//			OutputPortConfig<Vec3>("SearchPos",_HELP("")),
//			OutputPortConfig<Vec3>("GuardPos",_HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;//The squad leader entity
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("Gets the member info by its index from input leader entity squad");
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
//			if (!pActInfo->pEntity)
//			{
//				//ActivateOutput(pActInfo, EOP_Executed, false);
//				return;
//			}
//
//			IActor* pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
//			if (!pLeader)
//			{
//				//ActivateOutput(pActInfo, EOP_Executed, false);
//				return;
//			}
//
//			if (IsPortActive(pActInfo, EIP_Trigger))
//			{
//				if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
//				{
//					auto* pSquad = pSquadSystem->GetSquadFromLeader(pLeader);
//					if (pSquad)
//					{
//						int index = GetPortInt(pActInfo, EIP_Index);
//
//						auto* pMember = pSquad->GetMemberFromIndex(index);
//						if (!pMember || !pSquad->GetActor(pMember->GetId()))
//							return;
//
//						if (!pSquad->GetActor(pMember->GetId()))
//							return;
//
//						ActivateOutput(pActInfo, EOP_EntityId, pSquad->GetActor(pMember->GetId())->GetEntityId());
//						ActivateOutput(pActInfo, EOP_Order, int(pMember->GetCurrentOrder()));
//						ActivateOutput(pActInfo, EOP_SearchPos, pMember->GetSearchPos());
//						ActivateOutput(pActInfo, EOP_GuardPos, pMember->GetGuardPos());
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
//		return new CFlowNode_GetMemberInfo(pActInfo);
//	}
//};

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
			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					const auto pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetPortEntityId(pActInfo, EIP_LeaderId));
					const int squadId = GetPortInt(pActInfo, EIP_SquadId);
					const auto pSquad = pSquadSystem->GetSquadFromId(squadId);

					if (pSquad && pLeader)
					{
						pSquad->SetLeader(pLeader, false);

						ActivateOutput(pActInfo, EOP_EntityId, pSquad->GetLeader()->GetEntityId());
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
			InputPortConfig<int>("SquadId", _HELP("")),

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
			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					const int squadId = GetPortInt(pActInfo, EIP_SquadId);

					const auto pSquad = pSquadSystem->GetSquadFromId(squadId);

					if (pSquad->GetLeader())
						ActivateOutput(pActInfo, EOP_EntityId, pSquad->GetLeader()->GetEntityId());

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
			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					const int squadId = GetPortInt(pActInfo, EIP_SquadId);
					const auto pSquad = pSquadSystem->GetSquadFromId(squadId);
					if (pSquad)
					{
						int count = pSquad->GetMembersCount();

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
			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Trigger))
				{
					const int squadId = GetPortInt(pActInfo, EIP_SquadId);
					const auto pSquad = pSquadSystem->GetSquadFromId(squadId);
					if (pSquad->GetLeader() != 0)
					{
						ActivateOutput(pActInfo, EOP_Slot, pSquad->GetFreeMemberIndex());
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

class CFlowNode_CreateSquad : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Create,
		EIP_NewLeaderAfterDead,
		EIP_NewLeaderAfterLeft,
	};

	enum EOutputPorts
	{
		EOP_Created,
		EOP_LeaderId,
		EOP_SquadId,
	};

	//CSquad m_squad = CSquad();

public:
	////////////////////////////////////////////////////
	CFlowNode_CreateSquad(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_CreateSquad()
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
			InputPortConfig_Void("Create", _HELP("")),
			InputPortConfig<bool>("GetLeaderAfterDead", false, "Automatically appoints a new leader from the current squad members if the previous one has died.","GetLeaderAfterDead"),
			InputPortConfig<bool>("GetLeaderAfterLeave", false, "Automatically appoints a new leader from the current squad members if the previous one has leave from squad.", "GetLeaderAfterLeave"),
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
		config.sDescription = _HELP("Creates an empty squad with a defined leader");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
		}
		break;
		case eFE_Activate:
		{
			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Create))
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

					const bool newLeaderAfterDead = GetPortBool(pActInfo, EIP_NewLeaderAfterDead);
					const bool newLeaderAfterLeave = GetPortBool(pActInfo, EIP_NewLeaderAfterLeft);
					uint flags = 0;

					if (newLeaderAfterDead)
						flags |= eSCF_NewLeaderWhenOldDead;
					if (newLeaderAfterLeave)
						flags |= eSCF_NewLeaderWhenOldLeave;

					CSquad* pSquad = nullptr;

					if (!pSquadSystem->GetSquadFromLeader(pLeader))
					{
						pSquad = pSquadSystem->CreateSquad();
						if (pSquad)
						{
							pSquad->SetFlags(flags);
							pSquad->SetLeader(pLeader, false);
						}
					}
					
					ActivateOutput(pActInfo, EOP_Created, pSquad != nullptr);
					if (pSquad)
					{
						ActivateOutput(pActInfo, EOP_LeaderId, pSquad->GetLeader()->GetEntityId());
						ActivateOutput(pActInfo, EOP_SquadId, pSquad->GetId());
						//m_squad = squad;
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
		return new CFlowNode_CreateSquad(pActInfo);
	}
};

class CFlowNode_GetSquadInfo : public CFlowBaseNode
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
		//EOP_LeaderOrder,
		EOP_IsPlayerLeader,
		EOP_IsPlayerMember,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetSquadInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetSquadInfo()
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
			OutputPortConfig<EntityId>("LeaderId",_HELP("")),
			OutputPortConfig<int>("SquadId",_HELP("")),
			OutputPortConfig<int>("MemberCount",_HELP("Returns the number of all members in the squad")),
			OutputPortConfig<int>("MemberIndex",_HELP("Gets a free member index"),"MemberIndex"),
			//OutputPortConfig<int>("LeaderOrder",_HELP("Gets the squad leader's current order, 0 - goto, 1 - search, 2 - follow, 3 - none")),
			OutputPortConfig<bool>("IsPlayerLeader",_HELP("")),
			OutputPortConfig<bool>("IsPlayerMember",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;//Leader entity
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Returns some information about the squad");
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

			if (CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem())
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					const auto* pSquad = pSquadSystem->GetSquadFromLeader(pLeader);
					if (pSquad)
					{
						if (pSquad->GetLeader())
							ActivateOutput(pActInfo, EOP_LeaderId, pSquad->GetLeader()->GetEntityId());

						ActivateOutput(pActInfo, EOP_SquadId, pSquad->GetId());
						ActivateOutput(pActInfo, EOP_MemberCount, pSquad->GetMembersCount());
						ActivateOutput(pActInfo, EOP_MemberIndex, pSquad->GetFreeMemberIndex());
						//ActivateOutput(pActInfo, EOP_LeaderOrder, int(pSquad->GetLeaderOrder()));
						ActivateOutput(pActInfo, EOP_IsPlayerLeader, pSquad->HasClientLeader());
						ActivateOutput(pActInfo, EOP_IsPlayerMember, pSquad->HasClientMember());
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
		return new CFlowNode_GetSquadInfo(pActInfo);
	}
};

class CFlowNode_ManageSquad : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_AddMember,
		EIP_RemoveMember,
		EIP_SquadId,
		EIP_Detached,
		EIP_ExecuteOrder,
		EIP_SafeFly,
		EIP_DefaultOrder,
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
	CFlowNode_ManageSquad(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ManageSquad()
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
			InputPortConfig<bool>("Detached",false , _HELP("Detached squad member can be programmed using FlowGraph")),
			InputPortConfig<bool>("Execute",false , _HELP("")),
			InputPortConfig<bool>("SafeFly",true , _HELP("")),
			InputPortConfig<int>("DefaultOrder",0 , _HELP(""),"DefaultOrder", g_ui_config_orders),
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
		config.sDescription = _HELP("Add member to the pSquad->Squad we getting from the SquadId input port");
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

			const IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
			if (!pActor)
			{
				//ActivateOutput(pActInfo, EOP_Created, false);
				return;
			}

			CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
			if (pSquadSystem)
			{
				const int squadId = GetPortInt(pActInfo, EIP_SquadId);
				if (squadId < 0)
					return;

				const auto pSquad = pSquadSystem->GetSquadFromId(squadId);
				if (pSquad)
				{
					CMember member(pActor->GetEntityId());

					if (IsPortActive(pActInfo, EIP_AddMember))
					{
						//Исправить недобавляемость в массив
						//Исправлено: Нужно добавить & к типу возврата у функций получения отряда/члена
						//CryLogAlways("pSquad->AddMember(member)");
						const bool result = pSquad->AddMember(member);
						const bool markDetached = GetPortBool(pActInfo, EIP_Detached);
						const bool execute = GetPortBool(pActInfo, EIP_ExecuteOrder);
						const bool safeFly = GetPortBool(pActInfo, EIP_SafeFly);

						if (result)
						{
							if (markDetached)
							{
								SDetachedMemberData data;
								data.enableUpdate = false;

								//member.SetDetachedData(data);
								pSquad->MarkDetached(member.GetId(), data);
							}

							if (execute)
							{
								const auto pInstance = pSquad->GetMemberInstance(pActor);
								if (!pInstance)
									return;

								const auto pRef = pSquad->GetActionRef(pInstance, pActor->GetEntity()->GetWorldPos());
								if (!pRef)
									return;

								SOrderInfo order;
								order.type = (ESquadOrders)GetPortInt(pActInfo, EIP_DefaultOrder);

								pSquadSystem->ClientApplyExecution(pInstance, order, eEOF_ExecutedByFG, 0, pRef->GetWorldPos(), safeFly);
							}
						}

						ActivateOutput(pActInfo, EOP_Added, result);
						ActivateOutput(pActInfo, EOP_Index, pSquad->GetIndexFromMember(pActor));
						ActivateOutput(pActInfo, EOP_SquadId, pSquad->GetId());
					}
					else if (IsPortActive(pActInfo, EIP_RemoveMember))
					{
						const bool result = pSquad->RemoveMember(&member);
						ActivateOutput(pActInfo, EOP_Removed, result);
					}

					ActivateOutput(pActInfo, EOP_EntityId, pActor->GetEntityId());
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
		return new CFlowNode_ManageSquad(pActInfo);
	}
};

class CFlowNode_ExecuteOrder : public CFlowBaseNode, public ISquadEventListener
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Order,
		EIP_Flag,
		EIP_SafeFly,
		EIP_TargetId,
		EIP_TargetPos,
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed,
	};

	SActivationInfo m_actInfo;
	SOrderInfo m_order;
	CSquad* m_pSquad;
public:
	////////////////////////////////////////////////////
	CFlowNode_ExecuteOrder(SActivationInfo* pActInfo)
	{
		m_pSquad = nullptr;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ExecuteOrder()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	void OnOrderExecuted(CMember* pMember, const SOrderInfo& order) override
	{
		if (!pMember)
		{
			ActivateOutput(&m_actInfo, EOP_Failed, 1);
			return;
		}

		if (order.type == m_order.type)
		{
			ActivateOutput(&m_actInfo, EOP_Success, 1);
		}
	}

	void OnOrderExecuteFailed(CMember* pMember, const SOrderInfo& order) override 
	{
		if (!pMember)
		{
			ActivateOutput(&m_actInfo, EOP_Failed, 1);
			return;
		}

		if (order.type == m_order.type)
		{
			ActivateOutput(&m_actInfo, EOP_Failed, 1);
		}
	}

	void OnOrderPerfomingFailed(CMember* pMember, const SOrderInfo& order) override
	{
		//if (!pMember)
		//{
		//	ActivateOutput(&m_actInfo, EOP_Failed, 1);
		//	return;
		//}

		//if (order.type == m_order.type)
		//{
		//	ActivateOutput(&m_actInfo, EOP_Failed, 1);
		//}
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:PauseActionWhenCombat=0,WhenGotoTarget=1,WhenPerformingAction=2,Always=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<int>("Order", 0, _HELP(""), "Order", g_ui_config_orders),
			InputPortConfig<int>("IgnoreFlag", 0, _HELP("Ignore Combat Flag. Adjusts AI entry into combat while executing order steps(GotAnOrder->GotoTarget->PerformingAction)"), "IgnoreFlag", ui_config),
			InputPortConfig<bool>("SafeFly", true, _HELP("")),
			InputPortConfig<EntityId>("TargetId", EntityId(0), _HELP("")),
			InputPortConfig<Vec3>("TargetPos", Vec3(0), _HELP("")),
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
		config.sDescription = _HELP("Forces the AI to execute an order. The AI must be a squad member!");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			m_actInfo = *pActInfo;

			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
			{
				ActivateOutput(pActInfo, EOP_Failed, 1);
				return;
			}

			const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
			if (!pActor)
			{
				ActivateOutput(pActInfo, EOP_Failed, 1);
				return;
			}

			CSquadSystem* pSquadSystem = CtrlSys::GetSquadSystem();
			if (pSquadSystem)
			{
				m_pSquad = pSquadSystem->GetSquadFromMember(pActor, true);
				if (m_pSquad)
				{
					const auto pInstance = m_pSquad->GetMemberInstance(pActor);
					if (!pInstance)
					{
						ActivateOutput(pActInfo, EOP_Failed, 1);
						return;
					}

					const auto pLeader = m_pSquad->GetLeader();
					if (!pLeader)
					{
						ActivateOutput(pActInfo, EOP_Failed, 1);
						return;
					}

					if (IsPortActive(pActInfo, EIP_Sync))
					{
						m_pSquad->AddListener(this);

						DEFINE_STEPS;

						m_order = SOrderInfo();
						m_order.type = ESquadOrders(GetPortInt(pActInfo, EIP_Order));
						auto ignoreFlag = GetPortInt(pActInfo, EIP_Flag);
						
						const auto isAlien = pActor->IsAlien();
						const auto pVehicle = TOS_Vehicle::GetVehicle(pActor);

						if (!pVehicle)
						{
							const string memClass = pEntity->GetClass()->GetName();

							if (isAlien)
							{
								APPLY_ALIEN_EXECUTION(m_order);
							}
							else
							{
								APPLY_HUMAN_EXECUTION(m_order);
							}
						}
						else
						{
							APPLY_VEHICLE_EXECUTION(m_order);
						}

						m_order.targetId = GetPortInt(pActInfo, EIP_TargetId);
						m_order.targetPos = GetPortVec3(pActInfo, EIP_TargetPos);
						m_order.ignoreFlag = ignoreFlag;
						m_order.safeFly = GetPortBool(pActInfo, EIP_SafeFly);
						m_pSquad->ExecuteOrder(pInstance, m_order, eEOF_ExecutedByFG);
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
			if (m_pSquad)
				m_pSquad->RemoveListener(this);
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
		return new CFlowNode_ExecuteOrder(pActInfo);
	}
};
REGISTER_FLOW_NODE("SquadSystem:CreateSquad", CFlowNode_CreateSquad);
REGISTER_FLOW_NODE("SquadSystem:GetSquadInfo", CFlowNode_GetSquadInfo);
REGISTER_FLOW_NODE("SquadSystem:ManageSquad", CFlowNode_ManageSquad);
REGISTER_FLOW_NODE("SquadSystem:SetSquadLeader", CFlowNode_SquadSetLeader);
REGISTER_FLOW_NODE("SquadSystem:ExecuteOrder", CFlowNode_ExecuteOrder);
//REGISTER_FLOW_NODE("SquadSystem:GetSquadLeader", CFlowNode_SquadGetLeader);
//REGISTER_FLOW_NODE("SquadSystem:GetFreeIndex", CFlowNode_SquadGetFreeIndex);
//REGISTER_FLOW_NODE("SquadSystem:GetMembersCount", CFlowNode_SquadGetMembersCount);
//REGISTER_FLOW_NODE("SquadSystem:MemberExecuteOrder", CFlowNode_MemberExecuteOrder);
//REGISTER_FLOW_NODE("SquadSystem:GetMemberInfo", CFlowNode_GetMemberInfo);