/*************************************************************************
Copyright (C), CryTechLab, 2020.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 30:06:2020   11.58: Created by Akeeper

*************************************************************************/

#include "StdAfx.h"
#include "TOSNodesIncludes.h"

class CFlowNode_SetAlienEnergy : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
		EIP_Energy
	};

	enum EOutputPorts
	{
		EOP_Energy,
	};
	CActor* pActor;
	CAlien* pAlien;
public:
	////////////////////////////////////////////////////
	CFlowNode_SetAlienEnergy(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SetAlienEnergy()
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
			InputPortConfig<float>("Energy", 0, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<float>("Energy",_HELP("")),
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
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor && pActor->IsAlien())
						{
							pAlien = static_cast<CAlien*>(pActor);

							float energy = GetPortFloat(pActInfo, EIP_Energy);

							pAlien->SetAlienEnergy(energy);

							ActivateOutput(pActInfo, EOP_Energy, pAlien->GetAlienEnergy());
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
		return new CFlowNode_SetAlienEnergy(pActInfo);
	}
};

class CFlowNode_GetAlienEnergy : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
		EIP_Stop,
	};

	enum EOutputPorts
	{
		EOP_Energy,
	};
	CActor* pActor;
	CAlien* pAlien;
public:
	////////////////////////////////////////////////////
	CFlowNode_GetAlienEnergy(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetAlienEnergy()
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
			OutputPortConfig<float>("Energy",_HELP("")),
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
			if (IsPortActive(pActInfo, EIP_Get))
			{
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor && pActor->IsAlien())
						{
							pAlien = static_cast<CAlien*>(pActor);

							float energy = pAlien->GetAlienEnergy();
							ActivateOutput(pActInfo, EOP_Energy, energy);
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
		return new CFlowNode_GetAlienEnergy(pActInfo);
	}
};

class CFlowNode_CheckControl : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_OwnerId,
		EOP_LocalOwnerId,
		EOP_Controlled,
		EOP_LocalControlled,
	};
	CActor* pActor;
public:
	////////////////////////////////////////////////////
	CFlowNode_CheckControl(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_CheckControl()
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
			OutputPortConfig<EntityId>("LocalOwnerId",_HELP("")),
			OutputPortConfig<EntityId>("OwnerId",_HELP("")),
			OutputPortConfig<bool>("LocalControlled",_HELP("")),
			OutputPortConfig<bool>("Controlled",_HELP("")),
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
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							EntityId pPlayerId = pActor->GetOwnerId();
							if (pPlayerId)
							{
								ActivateOutput(pActInfo, EOP_OwnerId, pPlayerId);
								ActivateOutput(pActInfo, EOP_Controlled, true);
								if (pActor->IsLocalOwner())
								{
									ActivateOutput(pActInfo, EOP_LocalOwnerId, pPlayerId);
									ActivateOutput(pActInfo, EOP_LocalControlled, true);
								}
							}
							else
							{
								ActivateOutput(pActInfo, EOP_OwnerId, 0);
								ActivateOutput(pActInfo, EOP_LocalOwnerId, 0);
								ActivateOutput(pActInfo, EOP_Controlled, false);
								ActivateOutput(pActInfo, EOP_LocalControlled, false);
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
		return new CFlowNode_CheckControl(pActInfo);
	}
};

class CFlowNode_ControlEntity : public CFlowBaseNode, public IGameFrameworkListener
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Cancel,
		EIP_HidePlayer,
		EIP_BeamPlayer,
	};

	enum EOutputPorts
	{
		EOP_Started,
		EOP_Stopped,
		EOP_GetControlId,
	};

	IEntity* m_pEntity;
	EntityId m_EntityId;

	CActor* m_pActor;
	CActor* m_pStoredActor;

	SActivationInfo* m_pActInfo;

	int m_itemsNum;
	EntityId m_currentItemId;
	std::map<unsigned int, EntityId> m_inventoryItemsMap;

public:
	////////////////////////////////////////////////////
	CFlowNode_ControlEntity(SActivationInfo* pActInfo)
	{
		//g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_ControlActor", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		m_pActInfo = pActInfo;

		m_pEntity = 0;
		m_EntityId = 0;

		m_pActor = 0;
		m_pStoredActor = 0;

		m_itemsNum = 0;
		m_currentItemId = 0;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ControlEntity()
	{
		//g_pGame->GetIGameFramework()->UnregisterListener(this);
		m_inventoryItemsMap.clear();

		m_pEntity = 0;
		m_EntityId = 0;

		m_pActor = 0;
		m_pStoredActor = 0;

		m_itemsNum = 0;
		m_currentItemId = 0;
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.BeginGroup("CFlowNode_ControlEntity");
		//ser.Value("entityId", m_EntityId, 'eid');
		ser.Value("m_inventoryItemsMap", m_inventoryItemsMap);
		ser.EndGroup();
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("It's port starts the control system and if an input entity is defined and it is an actor, then the local player will be reassigned to this entity."),"Start"),
			InputPortConfig_AnyType("Cancel", _HELP("It's port stops the control system and changes the local player to default (Nomad)"),"Stop"),
			InputPortConfig<bool>("HidePlayer", true, _HELP("It's port hides the default player(Nomad)")),
			InputPortConfig<bool>("BeamPlayer", true, _HELP("Beam the Nomad player to the controlled actor")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Started",_HELP("ControlSystem is started and local player changed")),
			OutputPortConfig_AnyType("Stopped",_HELP("ControlSystem is stopped and local player changed to default (Nomad)")),
			OutputPortConfig<EntityId>("EntityId",_HELP("Id of input entity")),
		{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Changes the local player to an input entity. The entity must be an actor.");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			if (g_pControlSystem)
				g_pControlSystem->Stop();
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
		}
		break;
		case eFE_Update:
		{
			if (g_pControlSystem->GetControlClient() &&
				g_pControlSystem->GetControlClient()->GetControlledActor())
			{
				ActivateOutput(pActInfo, EOP_GetControlId, g_pControlSystem->GetControlClient()->GetControlledActor()->GetEntityId());
				//ActivateOutput(pActInfo, EOP_DebugControlled, g_pControlSystem->m_Timer->GetCurrTime());
			}
			else
			{
				ActivateOutput(pActInfo, EOP_GetControlId, NULL);
			}

			if (g_pGame->GetIGameFramework()->GetClientActor() &&
				g_pGame->GetIGameFramework()->GetClientActor()->GetLinkedVehicle())
			{
				g_pControlSystem->Stop();
				g_pControlSystem->GetControlClient()->ToggleDudeHide(false);
				g_pControlSystem->GetControlClient()->ToggleDudeBeam(false);
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case eFE_SetEntityId:
		{
			m_pEntity = pActInfo->pEntity;
			if (!m_pEntity)
			{
				m_pStoredActor = 0;
				return;
			}

			m_EntityId = m_pEntity->GetId();
			if (!m_EntityId)
				return;

			m_pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_EntityId));
			if (!m_pActor)
				return;

			if (m_pStoredActor == m_pActor)
				return;

			m_pStoredActor = m_pActor;

			if (g_pControlSystem->GetEnabled() && m_pStoredActor)
			{
				//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(m_pStoredActor->GetEntity()->GetRotation());
				//SetPlayerRotation(m_pStoredActor);
				g_pControlSystem->GetControlClient()->SetActor(m_pStoredActor);
			}
		}
		break;
		case eFE_Activate:
		{
			bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
			bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

			CPlayer* pNomadPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (pNomadPlayer)
			{
				if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetEnabled() && m_pStoredActor)
				{
					if (!pNomadPlayer->GetLinkedVehicle() && m_pStoredActor->GetHealth() > 0.1f)
					{
						StoreInventoryItems(pNomadPlayer);
						CleanInventory(pNomadPlayer);

						g_pControlSystem->Start();

						//SetPlayerRotation(m_pStoredActor);

						g_pControlSystem->GetControlClient()->SetActor(m_pStoredActor);

						g_pControlSystem->GetControlClient()->ToggleDudeHide(bHidePlayer);
						g_pControlSystem->GetControlClient()->ToggleDudeBeam(bBeamPlayer);

						if (!pNomadPlayer->IsThirdPerson() && !bHidePlayer)
							pNomadPlayer->ToggleThirdPerson();

						ActivateOutput(pActInfo, EOP_Started, 1);

						pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					}
				}
				else if (IsPortActive(pActInfo, EIP_Cancel) && g_pControlSystem->GetEnabled())
				{
					g_pControlSystem->Stop();

					g_pControlSystem->GetControlClient()->ToggleDudeHide(false);
					g_pControlSystem->GetControlClient()->ToggleDudeBeam(false);

					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);

					if (pNomadPlayer->IsThirdPerson() && !bHidePlayer)
						pNomadPlayer->ToggleThirdPerson();

					RestoreInventoryItems(pNomadPlayer);

					ActivateOutput(pActInfo, EOP_Stopped, 1);
				}
				else if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetEnabled() && !m_pStoredActor)
				{
					//CryLogAlways("ControlSystem can not be enabled without actor");
				}
			}
		}
		break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_FinalInitialize:
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

	/*void SetPlayerRotation(IActor* pActor)
	{
		IActor* pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
		if (pActor && pPlayer)
			pPlayer->SetViewRotation(pActor->GetViewRotation());
	}*/

	bool CleanInventory(IActor* pActor)
	{
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				pInventory->HolsterItem(true);
				pInventory->RemoveAllItems();

				if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
				{
					string itemClassName = "Binoculars";

					pClassRegistry->IteratorMoveFirst();
					IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

					if (pEntityClass)
						//pInventory->AddItem()
						g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemClassName, false, false, false);
				}

				return true;
			}
		}
		return false;
	}

	bool StoreInventoryItems(IActor* pActor)
	{
		m_itemsNum = 0;
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				m_itemsNum = pInventory->GetCount();

				//Clean massive
				m_inventoryItemsMap.clear();

				//Push items id values to massive
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					EntityId itemId = pInventory->GetItem(slot);
					m_inventoryItemsMap[slot] = itemId;
				}

				m_currentItemId = pInventory->GetCurrentItem();

				return true;
			}
		}
		return false;
	}

	void RestoreInventoryItems(IActor* pActor)
	{
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					EntityId itemId = m_inventoryItemsMap[slot];
					pInventory->AddItem(itemId);
				}

				CActor* pPlayer = (CActor*)pActor;

				pPlayer->SelectItem(m_currentItemId, true);
			}
		}
	}

	//IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime) {}
	virtual void OnSaveGame(ISaveGame* pSaveGame)
	{
		/*int state = g_pControlSystem->GetEnabled();
		pSaveGame->AddMetadata("ControlSaveState", state);*/
	}
	virtual void OnLoadGame(ILoadGame* pLoadGame)
	{
		//int state = 0;
		//pLoadGame->GetMetadata("ControlSaveState", state);
		//if (state == 0)
		//{
		//	g_pControlSystem->Stop();
		//}
		//else
		//	g_pControlSystem->Start();
	}
	virtual void OnLevelEnd(const char* nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	//~IGameFrameworkListener

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	/*virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ControlEntity(pActInfo);
	}*/
};

class CFlowNode_ControlEntityMP : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Cancel,
		EIP_HidePlayer,
		EIP_BeamPlayer,
	};

	enum EOutputPorts
	{
		EOP_Started,
		EOP_Stopped,
		EOP_GetControlId,
	};

	IEntity* m_pEntity;
	EntityId m_EntityId;

	CActor* m_pActor;
	CActor* m_pStoredActor;

	SActivationInfo* m_pActInfo;

	int m_itemsNum;
	EntityId m_currentItemId;
	std::map<unsigned int, EntityId> m_inventoryItemsMap;

public:
	////////////////////////////////////////////////////
	CFlowNode_ControlEntityMP(SActivationInfo* pActInfo)
	{
		m_pActInfo = pActInfo;

		m_pEntity = 0;
		m_EntityId = 0;

		m_pActor = 0;
		m_pStoredActor = 0;

		m_itemsNum = 0;
		m_currentItemId = 0;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ControlEntityMP()
	{
		m_inventoryItemsMap.clear();

		m_pEntity = 0;
		m_EntityId = 0;

		m_pActor = 0;
		m_pStoredActor = 0;

		m_itemsNum = 0;
		m_currentItemId = 0;
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		/*ser.BeginGroup("g_pControlSystem");
		ser.Value("entityId", m_EntityId, 'eid');
		ser.EndGroup();*/
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("It's port starts the control system and if an input entity is defined and it is an actor, then the local player will be reassigned to this entity."),"Start"),
			InputPortConfig_AnyType("Cancel", _HELP("It's port stops the control system and changes the local player to default (Nomad)"),"Stop"),
			InputPortConfig<bool>("HidePlayer", true, _HELP("It's port hides the default player(Nomad)")),
			InputPortConfig<bool>("BeamPlayer", true, _HELP("Beam the Nomad player to the controlled actor")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Started",_HELP("ControlSystem is started and local player changed")),
			OutputPortConfig_AnyType("Stopped",_HELP("ControlSystem is stopped and local player changed to default (Nomad)")),
			OutputPortConfig<EntityId>("EntityId",_HELP("Id of input entity")),
		{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Changes the local player to an input entity. The entity must be an actor.");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			/*if (g_pControlSystem)
				g_pControlSystem->Stop();*/
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
		}
		break;
		case eFE_Update:
		{
			//if (g_pControlSystem->GetControlClient() &&
			//	g_pControlSystem->GetControlClient()->GetControlledActor())
			//{
			//	ActivateOutput(pActInfo, EOP_GetControlId, g_pControlSystem->GetControlClient()->GetControlledActor()->GetEntityId());
			//	//ActivateOutput(pActInfo, EOP_DebugControlled, g_pControlSystem->m_Timer->GetCurrTime());
			//}
			//else
			//{
			//	ActivateOutput(pActInfo, EOP_GetControlId, NULL);
			//}

			if (g_pGame->GetIGameFramework()->GetClientActor() &&
				g_pGame->GetIGameFramework()->GetClientActor()->GetLinkedVehicle())
			{
				if (gEnv->bClient)
				{
					if (g_pGame->GetGameRules())
					{
						g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(g_pGame->GetIGameFramework()->GetClientActor()->GetEntityId(), 0, false, false), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
						//CryLogAlways("Client %s send request to take control to server", g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetName());
					}
				}
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case eFE_SetEntityId:
		{
			m_pEntity = pActInfo->pEntity;
			if (!m_pEntity)
			{
				m_pStoredActor = 0;
				return;
			}

			m_EntityId = m_pEntity->GetId();
			if (!m_EntityId)
				return;

			m_pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_EntityId));
			if (!m_pActor)
				return;

			m_pStoredActor = m_pActor;

			if (g_pControlSystem->GetEnabled())
			{
				CPlayer* pNomadPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
				CGameRules* pGR = g_pGame->GetGameRules();
				if (pGR && pNomadPlayer)
				{
					bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
					bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

					if (gEnv->bClient)
					{
						pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
						//CryLogAlways("Client %s send request to take control to server", pNomadPlayer->GetEntity()->GetName());
					}
					//pGR->TakeControl(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer);
				}
				//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(m_pStoredActor->GetEntity()->GetRotation());
			}
		}
		break;
		case eFE_Activate:
		{
			bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
			bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

			CGameRules* pGR = g_pGame->GetGameRules();

			CPlayer* pNomadPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (pNomadPlayer && pGR)
			{
				if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetEnabled() && m_pStoredActor)
				{
					if (!pNomadPlayer->GetLinkedVehicle() && m_pStoredActor->GetHealth() > 0.1f)
					{
						StoreInventoryItems(pNomadPlayer);
						CleanInventory(pNomadPlayer);

						if (gEnv->bClient)
						{
							pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
							//CryLogAlways("Client %s send request to take control to server", pNomadPlayer->GetEntity()->GetName());
						}

						/*g_pControlSystem->Start();
						g_pControlSystem->GetControlClient()->SetActor(m_pStoredActor);

						g_pControlSystem->GetControlClient()->SetHidePlayer(bHidePlayer);
						g_pControlSystem->GetControlClient()->SetBeamPlayer(bBeamPlayer);

						if (!pNomadPlayer->IsThirdPerson() && !bHidePlayer)
							pNomadPlayer->ToggleThirdPerson();*/

						ActivateOutput(pActInfo, EOP_Started, 1);

						pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					}
				}
				else if (IsPortActive(pActInfo, EIP_Cancel) && g_pControlSystem->GetEnabled())
				{
					if (gEnv->bClient)
					{
						pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), 0, false, false), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
						//CryLogAlways("Client %s send request to take control to server", pNomadPlayer->GetEntity()->GetName());
					}

					/*g_pControlSystem->Stop();

					g_pControlSystem->GetControlClient()->SetHidePlayer(false);
					g_pControlSystem->GetControlClient()->SetBeamPlayer(false);

					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);

					if (pNomadPlayer->IsThirdPerson() && !bHidePlayer)
						pNomadPlayer->ToggleThirdPerson();*/

					RestoreInventoryItems(pNomadPlayer);

					ActivateOutput(pActInfo, EOP_Stopped, 1);
				}
				else if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetEnabled() && !m_pStoredActor)
				{
					//CryLogAlways("ControlSystem can not be enabled without actor");
				}
			}
		}
		break;
		}
	}

	bool CleanInventory(IActor* pActor)
	{
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				pInventory->HolsterItem(true);
				pInventory->RemoveAllItems();

				if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
				{
					string itemClassName = "Binoculars";

					pClassRegistry->IteratorMoveFirst();
					IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

					if (pEntityClass)
						//pInventory->AddItem()
						g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemClassName, false, false, false);
				}

				return true;
			}
		}
		return false;
	}

	bool StoreInventoryItems(IActor* pActor)
	{
		m_itemsNum = 0;
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				m_itemsNum = pInventory->GetCount();

				//Clean massive
				m_inventoryItemsMap.clear();

				//Push items id values to massive
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					EntityId itemId = pInventory->GetItem(slot);
					m_inventoryItemsMap[slot] = itemId;
				}

				m_currentItemId = pInventory->GetCurrentItem();

				return true;
			}
		}
		return false;
	}

	void RestoreInventoryItems(IActor* pActor)
	{
		if (pActor)
		{
			IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					EntityId itemId = m_inventoryItemsMap[slot];
					pInventory->AddItem(itemId);
				}

				CActor* pPlayer = (CActor*)pActor;

				pPlayer->SelectItem(m_currentItemId, true);
			}
		}
	}
	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	/*virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ControlEntity(pActInfo);
	}*/
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
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
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
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							if (IInventory* pInventory = pActor->GetInventory())
							{
								if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
								{
									pClassRegistry->IteratorMoveFirst();
									IEntityClass* pEntityClass = pClassRegistry->FindClass(GetPortString(pActInfo, EIP_ItemClass));

									if (pEntityClass)
									{
										EntityId itemId = pInventory->GetItemByClass(pEntityClass);
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
				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							if (IInventory* pInventory = pActor->GetInventory())
							{
								if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
								{
									string itemClassName = GetPortString(pActInfo, EIP_ItemClass);

									pClassRegistry->IteratorMoveFirst();
									IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

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

class CFlowNode_ToggleAbility : public CFlowBaseNode
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
	};
public:
	////////////////////////////////////////////////////
	CFlowNode_ToggleAbility(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ToggleAbility()
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
			InputPortConfig_AnyType("Enable", _HELP("")),
			InputPortConfig_AnyType("Disable", _HELP("")),

			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Enabled",_HELP("")),
			OutputPortConfig<bool>("Disabled",_HELP("")),
			{0}
		};

		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Its node enables/disables the ability (L key) of controlled actor.");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (g_pControlSystem->GetEnabled())
			{
				if (IsPortActive(pActInfo, EIP_Enable))
				{
					bool bAbility = g_pControlSystem->GetControlClient()->GetGenericParams().canUseCenterAbility;

					if (bAbility == false)
					{
						g_pControlSystem->GetControlClient()->GetGenericParams().canUseCenterAbility = true;
						ActivateOutput(pActInfo, EOP_Enabled, true);
					}
				}
				else if (IsPortActive(pActInfo, EIP_Disable))
				{
					bool bAbility = g_pControlSystem->GetControlClient()->GetGenericParams().canUseCenterAbility;

					if (bAbility == true)
					{
						g_pControlSystem->GetControlClient()->GetGenericParams().canUseCenterAbility = false;
						ActivateOutput(pActInfo, EOP_Disabled, true);
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
		return new CFlowNode_ToggleAbility(pActInfo);
	}
};

class CFlowNode_TutorialMode : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Enable,
		EIP_Disable,
		EIP_Get,
	};

	enum EOutputPorts
	{
		EOP_Enabled,
		EOP_Disabled,
	};
	int m_iCanSelfDestruct = -1;
	SControlClient* m_pCC = 0;
	IScriptTable* m_pTable = 0;
	SmartScriptTable m_props;
	bool bTutMode = false;
public:
	////////////////////////////////////////////////////
	CFlowNode_TutorialMode(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_TutorialMode()
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
			InputPortConfig_AnyType("Enable", _HELP("")),
			InputPortConfig_AnyType("Disable", _HELP("")),
			InputPortConfig_AnyType("Get", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Enabled",_HELP("Returns 1 if tutorial mode is active in the Control System")),
			OutputPortConfig<bool>("Disabled",_HELP("Returns 1 if tutorial mode is unactive in the Control System")),
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
			CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity ? pActInfo->pEntity->GetId() : 0));
			if (pDude)
			{
				m_pCC = pDude->GetControlClient();
				if (m_pCC)
				{
					CActor* pCdActor = m_pCC->GetControlledActor();
					if (pCdActor)
					{
						bTutMode = m_pCC->GetTutorialMode();

						if (IsPortActive(pActInfo, EIP_Enable))
						{
							m_pCC->ToggleTutorialMode(true);
							ActivateOutput(pActInfo, EOP_Enabled, bTutMode);
						}

						if (IsPortActive(pActInfo, EIP_Disable))
						{
							m_pCC->ToggleTutorialMode(false);
							ActivateOutput(pActInfo, EOP_Disabled, !bTutMode);
						}

						if (IsPortActive(pActInfo, EIP_Get))
							ActivateOutput(pActInfo, bTutMode ? EOP_Enabled : EOP_Disabled, true);
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
			if (m_pCC)
				m_pCC->ToggleTutorialMode(false);

			//if (m_iCanSelfDestruct != -1)
			//{
			//	if (m_pCC)
			//	{
			//		CActor* pCdActor = m_pCC->GetControlledActor();

			//		m_pTable = pCdActor->GetEntity()->GetScriptTable();
			//		if (m_pTable && m_pTable->GetValue("Properties", m_props))
			//		{
			//			if (bTutMode)
			//			{
			//			}

			//			m_props->GetValue("bCanSelfDestruct", m_iCanSelfDestruct);
			//			m_props->SetValue("bCanSelfDestruct", value);
			//		}
			//	}
			//}
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			/*CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity ? pActInfo->pEntity->GetId() : 0));
			if (pDude)
				m_pCC = pDude->GetControlClient();*/
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

	void SetSelfDestruct(int value)
	{
		if (m_iCanSelfDestruct != -1)
		{
			if (m_pCC)
			{
			}
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_TutorialMode(pActInfo);
	}
};

class CFlowNode_GetControlledId : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_EntityId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetControlledId(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetControlledId()
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
			OutputPortConfig<EntityId>("EntityId",_HELP("")),
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
			if (IsPortActive(pActInfo, EIP_Trigger))
			{
				if (g_pControlSystem->GetEnabled())
				{
					CActor* pActor = g_pControlSystem->GetControlClient()->GetControlledActor();
					if (pActor)
						ActivateOutput(pActInfo, EOP_EntityId, pActor->GetEntityId());
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
		return new CFlowNode_GetControlledId(pActInfo);
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
			IAISystem* pAI = gEnv->pAISystem;
			if (pAI)
			{
				IEntity* pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					if (IAIObject* pGraphAI = pGraphEntity->GetAI())
					{
						int iEvent = GetPortInt(pActInfo, EIP_Event);

						pGraphAI->Event(GetPortInt(pActInfo, EIP_Event), 0);

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
		string sTypes = "0 - AITARGET_NONE, \
						1 - AITARGET_SOUND, \
						2 - AITARGET_MEMORY, \
						3 - AITARGET_VISUAL, \
						4 - AITARGET_ENEMY, \
						5 - AITARGET_FRIENDLY, \
						6 - AITARGET_BEACON, \
						7 - AITARGET_GRENADE, \
						8 - AITARGET_RPG";
		string sThreat = "0 - AITHREAT_NONE, \
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
			IAISystem* pAI = gEnv->pAISystem;
			if (pAI)
			{
				IEntity* pGraphEntity = pActInfo->pEntity;
				if (pGraphEntity)
				{
					if (IAIObject* pGraphAI = pGraphEntity->GetAI())
					{
						if (IPipeUser* pPU = pGraphAI->CastToIPipeUser())
						{
							int type = pPU->GetAttentionTargetType();
							int threat = pPU->GetAttentionTargetThreat();

							IAIObject* pTarget = pPU->GetAttentionTarget();
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
	bool m_bRestorePerc, m_bZeroPerc, m_bIgnore, m_bResetIgnore, m_bAcquireTarget = false;
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

			if (IsPortActive(pActInfo, EIP_Trigger))
			{
				if (IAISystem* pAI = gEnv->pAISystem)
				{
					if (m_pGraphEntity = pActInfo->pEntity)
					{
						string sEntityClass = m_pGraphEntity->GetClass()->GetName();
						if (sEntityClass == "Trooper" || sEntityClass == "PlayerTrooper")
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

					if (IEntity* pTarget = gEnv->pEntitySystem->GetEntity(m_eRefTarget))
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
							AgentParameters agentParams = pGraphAI->CastToIAIActor()->GetParameters();
							float m_oldVisualScale = agentParams.m_PerceptionParams.perceptionScale.visual;
							float m_oldAudioScale = agentParams.m_PerceptionParams.perceptionScale.audio;

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

class CFlowNode_Actions : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Enable,
		EIP_Disable,
		EIP_Action,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_OnHold,
		EOP_OnPressed,
		EOP_OnReleased,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_Actions(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_Actions()
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
			InputPortConfig_Void("Enable", _HELP("")),
			InputPortConfig_Void("Disable", _HELP("")),
			InputPortConfig<string>("Action", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP("")),
			OutputPortConfig_AnyType("OnHold",_HELP("")),
			OutputPortConfig_AnyType("OnPressed",_HELP("")),
			OutputPortConfig_AnyType("OnReleased",_HELP("")),
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
			if (IsPortActive(pActInfo, EIP_Enable))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
			}
			else if (IsPortActive(pActInfo, EIP_Disable))
			{
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
		}
		break;
		case eFE_Initialize:
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
		}
		break;
		case eFE_Update:
		{
			if (SControlClient* pCC = g_pControlSystem->GetControlClient())
			{
				if (pCC->GetControlledActor())
				{
					ActionId(action) = GetPortString(pActInfo, EIP_Action);
					if (pCC->GetActions()[action] == EActionActivationMode::eAAM_OnPress)
						ActivateOutput(pActInfo, EOP_OnPressed, true);
					else if (pCC->GetActions()[action] == EActionActivationMode::eAAM_OnRelease)
						ActivateOutput(pActInfo, EOP_OnReleased, true);
					else if (pCC->GetActions()[action] == EActionActivationMode::eAAM_OnHold)
						ActivateOutput(pActInfo, EOP_OnHold, true);
				}
				else
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			}
			else
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
		return new CFlowNode_Actions(pActInfo);
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
				if (IEntity* pGraphEntity = pActInfo->pEntity)
				{
					CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pGraphEntity->GetId()));
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
					CScout* pGraphScout = static_cast<CScout*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
					if (pGraphScout)
					{
						float fForce = GetPortFloat(pActInfo, EIP_Force);
						Vec3 vTarget = GetPortVec3(pActInfo, EIP_Target);

						if (IEntity* pBomb = DropHealBomb(fForce, vTarget))
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

class CFlowNode_ActorRecruitToSquad : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Trigger,
	};

	enum EOutputPorts
	{
		EOP_Done
	};

	IEntity* pEntity = 0;
public:
	////////////////////////////////////////////////////
	CFlowNode_ActorRecruitToSquad(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorRecruitToSquad()
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
			OutputPortConfig<EntityId>("Done",_HELP("")),
			//OutputPortConfig<bool>("Disabled",_HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Allows you to recruit an input actor's entity to the squad");
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
					SmartScriptTable props;
					IScriptTable* pTable = pEntity->GetScriptTable();
					if (pTable && pTable->GetValue("Properties", props))
					{
						props->SetValue("UseText", "@recruit");
						Script::CallMethod(pTable, "Event_EnableUsable");
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
		return new CFlowNode_ActorRecruitToSquad(pActInfo);
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
			if (IEntity* pEntity = pActInfo->pEntity)
			{
				m_pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
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
//Entity
REGISTER_FLOW_NODE("Entity:SetMaterial", CFlowNode_SetEntityMaterial);

//System
REGISTER_FLOW_NODE("System:GetEnvironment", CFlowNode_GetSystemEnvironment);

//Animations
REGISTER_FLOW_NODE("Animations:PlayAction", CFlowNode_PlayAction);

//TheOtherSide
REGISTER_FLOW_NODE("Control:ControlEntityMP", CFlowNode_ControlEntityMP);
REGISTER_FLOW_NODE("Control:ControlEntity", CFlowNode_ControlEntity);
REGISTER_FLOW_NODE("Control:CheckControl", CFlowNode_CheckControl);
REGISTER_FLOW_NODE("Control:GetAlienEnergy", CFlowNode_GetAlienEnergy);
REGISTER_FLOW_NODE("Control:SetAlienEnergy", CFlowNode_SetAlienEnergy);
REGISTER_FLOW_NODE("Control:ToggleAbility", CFlowNode_ToggleAbility);
REGISTER_FLOW_NODE("Control:GetControlledId", CFlowNode_GetControlledId);
REGISTER_FLOW_NODE("Control:SetTutorialMode", CFlowNode_TutorialMode);
//REGISTER_FLOW_NODE("Control:Actions",			CFlowNode_Actions);

//Inventory
REGISTER_FLOW_NODE("Inventory:ActorRemoveAllItems", CFlowNode_ActorRemoveAllItems);
REGISTER_FLOW_NODE("Inventory:ActorRemoveItem", CFlowNode_ActorRemoveItem);
REGISTER_FLOW_NODE("Inventory:ActorAddItem", CFlowNode_ActorAddItem);

//Actor
REGISTER_FLOW_NODE("Game:ActorRecruit", CFlowNode_ActorRecruitToSquad);

//AI
REGISTER_FLOW_NODE("AI:AIGetTarget", CFlowNode_AIGetTarget);
REGISTER_FLOW_NODE("AI:Event", CFlowNode_AIEvent);
REGISTER_FLOW_NODE("AI:Reset", CFlowNode_AIReset);
REGISTER_FLOW_NODE("AI:ScoutDropBomb", CFlowNode_ScoutDropBomb);
REGISTER_FLOW_NODE("AI:GetPerc", CFlowNode_AIGetPerc);