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
#include "GameUtils.h"
#include "TheOtherSide/Helpers/TOS_AI.h"

std::map<int, int> g_controlClients;

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
						pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor && pActor->IsAlien())
						{
							pAlien = dynamic_cast<CAlien*>(pActor);

							const float energy = GetPortFloat(pActInfo, EIP_Energy);

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
		config.SetCategory(EFLN_APPROVED);
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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor && pActor->IsAlien())
						{
							pAlien = dynamic_cast<CAlien*>(pActor);

							const float energy = pAlien->GetAlienEnergy();
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
				const IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					const EntityId pEntityId = pEntity->GetId();
					if (pEntityId)
					{
						pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntityId));
						if (pActor)
						{
							const EntityId pPlayerId = pActor->GetOwnerId();
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
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			if (g_pControlSystem)
				g_pControlSystem->StopLocal(false);
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
		}
		break;
		case eFE_Update:
		{
			if (g_pControlSystem->GetLocalControlClient() &&
				g_pControlSystem->GetLocalControlClient()->GetControlledActor())
			{
				ActivateOutput(pActInfo, EOP_GetControlId, g_pControlSystem->GetLocalControlClient()->GetControlledActor()->GetEntityId());
				//ActivateOutput(pActInfo, EOP_DebugControlled, g_pControlSystem->m_Timer->GetCurrTime());
			}
			else
			{
				ActivateOutput(pActInfo, EOP_GetControlId, NULL);
			}

			if (g_pGame->GetIGameFramework()->GetClientActor() &&
				g_pGame->GetIGameFramework()->GetClientActor()->GetLinkedVehicle())
			{
				g_pControlSystem->StopLocal(false);
				g_pControlSystem->GetLocalControlClient()->ToggleDudeHide(false);
				g_pControlSystem->GetLocalControlClient()->ToggleDudeBeam(false);
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

			m_pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_EntityId));
			if (!m_pActor)
				return;

			if (m_pStoredActor == m_pActor)
				return;

			m_pStoredActor = m_pActor;

			if (g_pControlSystem->GetLocalEnabled() && m_pStoredActor)
			{
				//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(m_pStoredActor->GetEntity()->GetRotation());
				//SetPlayerRotation(m_pStoredActor);
				g_pControlSystem->GetLocalControlClient()->SetActor(m_pStoredActor);
			}
		}
		break;
		case eFE_Activate:
		{
			//prevents a crash in the editor when pressing CTRL+P
			if (gEnv->pSystem->IsEditorMode())
				return;

			const bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
			const bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

			CPlayer* pNomadPlayer = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (pNomadPlayer)
			{
				if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetLocalEnabled() && m_pStoredActor)
				{
					if (!pNomadPlayer->GetLinkedVehicle() && m_pStoredActor->GetHealth() > 0.1f)
					{
						StoreInventoryItems(pNomadPlayer);
						CleanInventory(pNomadPlayer);

						g_pControlSystem->StartLocal(m_pStoredActor, bHidePlayer, bBeamPlayer);

						//SetPlayerRotation(m_pStoredActor);

						//g_pControlSystem->GetLocalControlClient()->SetActor(m_pStoredActor);

						//g_pControlSystem->GetLocalControlClient()->ToggleDudeHide(bHidePlayer);
						//g_pControlSystem->GetLocalControlClient()->ToggleDudeBeam(bBeamPlayer);

						if (!pNomadPlayer->IsThirdPerson() && !bHidePlayer)
							pNomadPlayer->ToggleThirdPerson();

						ActivateOutput(pActInfo, EOP_Started, 1);

						pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					}
				}
				else if (IsPortActive(pActInfo, EIP_Cancel) && g_pControlSystem->GetLocalEnabled())
				{
					g_pControlSystem->StopLocal(false);

					g_pControlSystem->GetLocalControlClient()->ToggleDudeHide(false);
					g_pControlSystem->GetLocalControlClient()->ToggleDudeBeam(false);

					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);

					if (pNomadPlayer->IsThirdPerson() && !bHidePlayer)
						pNomadPlayer->ToggleThirdPerson();

					RestoreInventoryItems(pNomadPlayer);

					ActivateOutput(pActInfo, EOP_Stopped, 1);
				}
				else if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetLocalEnabled() && !m_pStoredActor)
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
					const string itemClassName = "Binoculars";

					pClassRegistry->IteratorMoveFirst();
					const IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

					if (pEntityClass)
						//pInventory->AddItem()
						g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemClassName, false, false, false);
				}

				return true;
			}
		}
		return false;
	}

	bool StoreInventoryItems(const IActor* pActor)
	{
		m_itemsNum = 0;
		if (pActor)
		{
			const IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				m_itemsNum = pInventory->GetCount();

				//Clean massive
				m_inventoryItemsMap.clear();

				//Push items id values to massive
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					const EntityId itemId = pInventory->GetItem(slot);
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
					const EntityId itemId = m_inventoryItemsMap[slot];
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
			//if (g_pControlSystem)
			//	g_pControlSystem->Start();
			//g_pControlSystem->StopLocal();

			bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
			bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

			CGameRules* pGR = g_pGame->GetGameRules();

			CPlayer* pNomadPlayer = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (pNomadPlayer && pGR)
			{
				if (g_controlClients[pNomadPlayer->GetChannelId()])
				{
					if (!pNomadPlayer->GetLinkedVehicle())
					{
						StoreInventoryItems(pNomadPlayer);
						CleanInventory(pNomadPlayer);

						if (gEnv->bClient)
						{
							pGR->StartControl(g_controlClients[pNomadPlayer->GetChannelId()], pNomadPlayer->GetEntityId(), false);
							//pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer), eRMI_ToServer);
							//CryLogAlways("[eFE_Activate][Start][Client][SvRequestTakeControl]");
						}
					}
				}
			}

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
						//g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(g_pGame->GetIGameFramework()->GetClientActor()->GetEntityId(), 0, false, false), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
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

			m_pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_EntityId));
			if (!m_pActor)
				return;

			m_pStoredActor = m_pActor;

			if (g_pControlSystem->GetLocalEnabled())
			{
				const CPlayer* pNomadPlayer = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
				const CGameRules* pGR = g_pGame->GetGameRules();
				if (pGR && pNomadPlayer)
				{
					bool bHidePlayer = GetPortBool(pActInfo, EIP_HidePlayer);
					bool bBeamPlayer = GetPortBool(pActInfo, EIP_BeamPlayer);

					if (gEnv->bClient)
					{	
						//pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer), eRMI_ToServer);
						//CryLogAlways("[eFE_SetEntityId][Client][SvRequestTakeControl]");
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

			CPlayer* pNomadPlayer = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (pNomadPlayer && pGR)
			{
				if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetLocalEnabled() && m_pStoredActor)
				{
					if (!pNomadPlayer->GetLinkedVehicle() && m_pStoredActor->GetHealth() > 0.1f)
					{
						StoreInventoryItems(pNomadPlayer);
						CleanInventory(pNomadPlayer);

						if (gEnv->bClient)
						{
							pGR->StartControl(m_pStoredActor->GetEntityId(), pNomadPlayer->GetEntityId(), false);
							g_controlClients[pNomadPlayer->GetChannelId()] = m_pStoredActor->GetEntityId();
							//pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), m_pStoredActor->GetEntityId(), bHidePlayer, bBeamPlayer), eRMI_ToServer);
							//CryLogAlways("[eFE_Activate][Start][Client][SvRequestTakeControl]");
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
				else if (IsPortActive(pActInfo, EIP_Cancel) && g_pControlSystem->GetLocalEnabled())
				{
					if (gEnv->bClient)
					{
						pGR->StopControl(pNomadPlayer->GetEntityId());
						g_controlClients[pNomadPlayer->GetChannelId()] = 0;
						//pGR->StartControl(0, pNomadPlayer->GetEntityId());
						//pGR->GetGameObject()->InvokeRMI(CGameRules::SvRequestTakeControl(), CGameRules::ControlParams(pNomadPlayer->GetEntityId(), 0, false, false), eRMI_ToServer); //eRMI_ToServer //eRMI_ToOwnClient
						//CryLogAlways("[eFE_Activate][Cancel][Client][SvRequestTakeControl]");
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
				else if (IsPortActive(pActInfo, EIP_Start) && !g_pControlSystem->GetLocalEnabled() && !m_pStoredActor)
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
					const string binoClass = "Binoculars";

					pClassRegistry->IteratorMoveFirst();
					const IEntityClass* pEntityClass = pClassRegistry->FindClass(binoClass);

					if (pEntityClass)
						g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, binoClass, false, false, false);
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
			const IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				m_itemsNum = pInventory->GetCount();

				//Clean massive
				m_inventoryItemsMap.clear();

				//Push items id values to massive
				for (int slot = 0; slot <= m_itemsNum; slot++)
				{
					const EntityId itemId = pInventory->GetItem(slot);
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
					const EntityId itemId = m_inventoryItemsMap[slot];
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
			if (g_pControlSystem->GetLocalEnabled())
			{
				if (IsPortActive(pActInfo, EIP_Enable))
				{
					const bool bAbility = g_pControlSystem->GetLocalControlClient()->GetGenericParams().canUseCenterAbility;

					if (bAbility == false)
					{
						g_pControlSystem->GetLocalControlClient()->GetGenericParams().canUseCenterAbility = true;
						ActivateOutput(pActInfo, EOP_Enabled, true);
					}
				}
				else if (IsPortActive(pActInfo, EIP_Disable))
				{
					const bool bAbility = g_pControlSystem->GetLocalControlClient()->GetGenericParams().canUseCenterAbility;

					if (bAbility == true)
					{
						g_pControlSystem->GetLocalControlClient()->GetGenericParams().canUseCenterAbility = false;
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
	CControlClient* m_pCC = 0;
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
			CPlayer* pDude = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity ? pActInfo->pEntity->GetId() : 0));
			if (pDude)
			{
				m_pCC = pDude->GetControlClient();
				if (m_pCC)
				{
					const CActor* pCdActor = m_pCC->GetControlledActor();
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
				if (g_pControlSystem->GetLocalEnabled())
				{
					const CActor* pActor = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
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
			if (CControlClient* pCC = g_pControlSystem->GetLocalControlClient())
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


//class CFlowNode_ActorRecruitToSquad : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Trigger,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Done
//	};
//
//	IEntity* pEntity = 0;
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_ActorRecruitToSquad(SActivationInfo* pActInfo)
//	{
//	}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_ActorRecruitToSquad()
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
//			InputPortConfig_AnyType("Trigger", _HELP("")),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<EntityId>("Done",_HELP("")),
//			//OutputPortConfig<bool>("Disabled",_HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("Allows you to recruit an input actor's entity to the squad");
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
//				if (pEntity = pActInfo->pEntity)
//				{
//					SmartScriptTable props;
//					IScriptTable* pTable = pEntity->GetScriptTable();
//					if (pTable && pTable->GetValue("Properties", props))
//					{
//						props->SetValue("UseText", "@recruit");
//						Script::CallMethod(pTable, "Event_EnableUsable");
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
//		return new CFlowNode_ActorRecruitToSquad(pActInfo);
//	}
//};


class CFlowNode_BlendSpace : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_Stop,
		EIP_Type,
		EIP_Value
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

	EMotionParamID m_paramId;
	float m_turnSpeed;
	bool m_bToStop;
	bool m_bToStart;
public:
	////////////////////////////////////////////////////
	CFlowNode_BlendSpace(SActivationInfo* pActInfo)
	{
		m_paramId = eMotionParamID_TurnAngle;
		m_turnSpeed = 0.5f;
		m_bToStop = false;
		m_bToStart = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_BlendSpace()
	{
	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:TurnAngle=1, TurnSpeed=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig_AnyType("Stop", _HELP("")),
			InputPortConfig<int>("Type", 1, _HELP(""),"Type", ui_config),
			InputPortConfig<float>("Value",0.5f, _HELP(""), "Value"),
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
			if (const IEntity* pEntity = pActInfo->pEntity)
			{
				auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
				if (IsPortActive(pActInfo, EIP_Start))
				{
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					m_bToStart = true;
				}
				else if (IsPortActive(pActInfo, EIP_Stop))
					m_bToStop = true;
			}
		}
		break;
		case eFE_Update:

			if (m_bToStop)
			{
				if (const IEntity* pEntity = pActInfo->pEntity)
				{
					const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
					if (pActor)
					{
						const auto pChar = pActor->GetEntity()->GetCharacter(0);
						if (pChar)
						{
							Interpolate(m_turnSpeed, 0.5f, 2.5f, gEnv->pTimer->GetFrameTime());
							pChar->GetISkeletonAnim()->SetBlendSpaceOverride(m_paramId, m_turnSpeed, true);
						}
					}
				}

				if (abs(m_turnSpeed - 0.5f) == 0.05f)
				{
					m_bToStop = false;
					m_bToStart = false;
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				}
			}
			else if (m_bToStart)
			{
				if (const IEntity* pEntity = pActInfo->pEntity)
				{
					const auto pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
					if (pActor)
					{
						const auto pChar = pActor->GetEntity()->GetCharacter(0);
						if (pChar)
						{
							const int type = GetPortInt(pActInfo, EIP_Type);
							const float goal = GetPortFloat(pActInfo, EIP_Value);
							m_paramId = type == 1 ? eMotionParamID_TurnAngle : eMotionParamID_TurnSpeed;

							Interpolate(m_turnSpeed, goal, 5.5f, gEnv->pTimer->GetFrameTime());
							pChar->GetISkeletonAnim()->SetBlendSpaceOverride(m_paramId, m_turnSpeed, true);
						}
					}
				}
			}


			break;
		case IFlowNode::eFE_Initialize:
		{
			m_bToStart = false;
			m_bToStop = false;
			m_turnSpeed = 0.5f;
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
		return new CFlowNode_BlendSpace(pActInfo);
	}
};

class CFlowNode_OnStartControl : public CFlowBaseNode, IControlSystemChild
{
	enum EInputPorts
	{
		
	};

	enum EOutputPorts
	{
		EOP_ControlledClassName,
		EOP_ControlledId,
	};
	CActor* pActor;
	SActivationInfo m_actInfo;

public:
	////////////////////////////////////////////////////
	CFlowNode_OnStartControl(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_OnStartControl()
	{
		g_pControlSystem->RemoveChild(this, true);
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
			//InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<string>("ClassName",_HELP("")),
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
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_Activate:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_Initialize:
			m_actInfo = *pActInfo;
			g_pControlSystem->AddChild(this, true);
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
		return new CFlowNode_OnStartControl(pActInfo);
	}

	virtual void OnStartControl(const IActor* pActor) override
	{ 
		if (!pActor)
			return;

		ActivateOutput(&m_actInfo, EOP_ControlledClassName, 
			string(pActor->GetEntity()->GetClass()->GetName()));

		ActivateOutput(&m_actInfo, EOP_ControlledId,
			pActor->GetEntityId());
	}
	virtual void OnStopControl(const IActor* pActor) override
	{
		if (!pActor)
			return;
	}

	virtual void OnMainMenuEnter() override
	{

	}
	virtual void OnGameRulesReset() override
	{

	}
	virtual void OnActorDeath(IActor* pActor) override
	{

	}
	virtual void OnActorGrabbed(IActor* pActor, EntityId grabberId) override
	{

	}
	virtual void OnActorDropped(IActor* pActor, EntityId droppedId) override
	{

	}
	virtual void OnActorGrab(IActor* pActor, EntityId grabId) override
	{

	}
	virtual void OnActorDrop(IActor* pActor, EntityId dropId) override
	{

	}
	virtual void OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) override
	{

	}
	virtual void OnExitVehicle(IActor* pActor) override
	{

	}
	virtual bool OnInputEvent(const SInputEvent& event) override
	{
		return false;
	}
	virtual bool OnInputEventUI(const SInputEvent& event) { return false; }
	virtual void Init() override
	{

	}
	virtual void Update(float frametime) override
	{

	}
	virtual void Serialize(TSerialize ser) override
	{

	}
	virtual const char* GetChildName() override
	{
		return "CFlowNode_OnStartControl";
	}
};


//TheOtherSide
//REGISTER_FLOW_NODE("Control:ControlEntityMP", CFlowNode_ControlEntityMP);
//REGISTER_FLOW_NODE("Control:BlendSpace", CFlowNode_BlendSpace);
REGISTER_FLOW_NODE("Control:ControlEntity", CFlowNode_ControlEntity);
REGISTER_FLOW_NODE("Control:CheckControl", CFlowNode_CheckControl);
REGISTER_FLOW_NODE("Control:OnStartControl", CFlowNode_OnStartControl);
REGISTER_FLOW_NODE("Control:GetAlienEnergy", CFlowNode_GetAlienEnergy);
REGISTER_FLOW_NODE("Control:SetAlienEnergy", CFlowNode_SetAlienEnergy);
//REGISTER_FLOW_NODE("Control:ToggleAbility", CFlowNode_ToggleAbility);
REGISTER_FLOW_NODE("Control:GetControlledId", CFlowNode_GetControlledId);
REGISTER_FLOW_NODE("Control:SetTutorialMode", CFlowNode_TutorialMode);
//REGISTER_FLOW_NODE("Control:Actions",			CFlowNode_Actions);

//Actor
//REGISTER_FLOW_NODE("Game:ActorRecruit", CFlowNode_ActorRecruitToSquad);