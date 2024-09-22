/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"

#include "Game.h"
#include "Nodes/G2FlowBaseNode.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"

class CFlowNode_MCStartControl  : public CFlowBaseNode
{
	int m_savedItemsCount;
	EntityId m_currentDudeItemId;
	std::map<unsigned int, EntityId> m_savedItems;

public:
	explicit CFlowNode_MCStartControl(SActivationInfo* pActInfo)
	{
		m_savedItemsCount = 0;
		m_currentDudeItemId = 0;
		m_savedItems.clear();
	}

	~CFlowNode_MCStartControl() {};

	//IFlowNodePtr Clone(SActivationInfo* pActInfo) 
	//{
	//	return new CFlowNode_MCStartControl(pActInfo);
	//}

	enum EInputPorts
	{
		EIP_Start = 0,
		EIP_Cancel,
		EIP_PriorityFaction,
		EIP_BeamDude,
		EIP_DisableSuit,
		EIP_DisableActions,
		EIP_SaveLoadParams,
		EIP_SaveItems,
	};

	enum EOutputPorts
	{
		EOP_Started = 0,
		EOP_Done,
	};

	bool StoreInventoryItems(const IActor* pActor)
	{
		m_savedItemsCount = 0;
		if (pActor)
		{
			const IInventory* pInventory = pActor->GetInventory();
			if (pInventory)
			{
				m_savedItemsCount = pInventory->GetCount();

				//Clean massive
				m_savedItems.clear();

				//Push items id values to massive
				for (int slot = 0; slot <= m_savedItemsCount; slot++)
				{
					const EntityId itemId = pInventory->GetItem(slot);
					m_savedItems[slot] = itemId;
				}

				m_currentDudeItemId = pInventory->GetCurrentItem();

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
				for (int slot = 0; slot <= m_savedItemsCount; slot++)
				{
					const EntityId itemId = m_savedItems[slot];
					pInventory->AddItem(itemId);
				}

				const auto pPlayer = static_cast<CTOSActor*>(pActor);
				pPlayer->SelectItem(m_currentDudeItemId, true);
			}
		}
	}

	void GetConfiguration(SFlowNodeConfig& config) 
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Start", _HELP("Trigger to start controlling input actor")),
			InputPortConfig_Void("Cancel", _HELP("Trigger to cancel controlling of input actor")),
			InputPortConfig<int>("Priority", 0, _HELP("Master - the faction is taken from the master and applied to the slave."), "Faction", _UICONFIG("enum_int:Master=0,Slave=1")),
			InputPortConfig<bool>("BeamDude", true,_HELP("Beam dude to input actor position")),
			InputPortConfig<bool>("DisableSuit", true,_HELP("Disable dude's nanosuit")),
			InputPortConfig<bool>("DisableActions", true,_HELP("Disable dude's human actions")),
			InputPortConfig<bool>("SaveLoadParams", true,_HELP("Save/Load pos, rot, species, inventory, suit energy and mode")),
			InputPortConfig<bool>("SaveItems", true,_HELP("Save/load dude's inventory items before/after start control")),
			{nullptr}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void("Started", _HELP("")),
			OutputPortConfig_Void("Done", _HELP("")),
			{nullptr}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("*Work only in singleplayer gamerules* Master Client Start/Stop control slave");
		config.SetCategory(EFLN_DEBUG);
	}

	void ProcessEvent(const EFlowEvent event, SActivationInfo* pActInfo) 
	{
		if (gEnv->bMultiplayer)
			return;

		switch (event)
		{
		case eFE_Initialize:
		{
			const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
			if (!pMC)
				return;

			if (pMC->GetSlaveEntity())
				pMC->StopControl(true);
		}
		break;
		case eFE_SetEntityId:
		{
			
		}
		break;
		case eFE_Activate:
		{
			const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
			if (!pMC)
				return;

			const auto pInputEntity = pActInfo->pEntity;
			if (!pInputEntity)
				return;

			const auto pDudePlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
			if (!pDudePlayer)
				return;

			if (pDudePlayer->GetLinkedVehicle())
			{
				CryLog("%s[%s] [%s] cannot start control Slave %s while in a vehicle", 
					TOS_COLOR_YELLOW, 
					TOS_Debug::GetEnv(), 
					pDudePlayer->GetEntity()->GetName(),
					pInputEntity->GetName());

				return;
			}

			if (IsPortActive(pActInfo, EIP_Start))
			{
				if (pMC->GetSlaveEntity())
					pMC->StopControl(true);

				if (GetPortBool(pActInfo, EIP_SaveItems))
					StoreInventoryItems(pDudePlayer);

				uint flags = 0;
				if (GetPortBool(pActInfo, EIP_BeamDude))
					flags |= CTOSMasterClient::TOS_DUDE_FLAG_BEAM_MODEL;

				if (GetPortBool(pActInfo, EIP_DisableSuit))
					flags |= CTOSMasterClient::TOS_DUDE_FLAG_DISABLE_SUIT;

				if (GetPortBool(pActInfo, EIP_DisableActions))
					flags |= CTOSMasterClient::TOS_DUDE_FLAG_ENABLE_ACTION_FILTER;

				if (GetPortBool(pActInfo, EIP_SaveLoadParams))
					flags |= CTOSMasterClient::TOS_DUDE_FLAG_SAVELOAD_PARAMS;

				const auto factionPriority = EFactionPriority(GetPortInt(pActInfo, EIP_PriorityFaction));				
				pMC->StartControl(pInputEntity, flags, true, factionPriority);
				ActivateOutput(pActInfo, EOP_Started, 1);
			}
			else if (IsPortActive(pActInfo, EIP_Cancel))
			{
				if (pMC->GetSlaveEntity())
					pMC->StopControl(true);

				if (GetPortBool(pActInfo, EIP_SaveItems))
					RestoreInventoryItems(pDudePlayer);

				ActivateOutput(pActInfo, EOP_Done, 1);
			}

		}
		break;
		}
	}

	void GetMemoryStatistics(ICrySizer* s) 
	{
		s->Add(*this);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser) 
	{
		ser.BeginGroup("CFlowNode_MCStartControl");
		//ser.Value("entityId", m_EntityId, 'eid');
		ser.Value("m_savedItems", m_savedItems);
		ser.EndGroup();
	}
	SActivationInfo m_actInfo;
	//~INanoSuitListener
};

REGISTER_FLOW_NODE("TOSMasterClient:StartControl", CFlowNode_MCStartControl);