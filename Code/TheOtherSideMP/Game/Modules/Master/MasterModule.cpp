#include "StdAfx.h"
#include "MasterModule.h"

#include "Game.h"
#include "GameRules.h"
#include "IEntitySystem.h"
#include "MasterClient.h"
#include "MasterSynchronizer.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

#include "../../../Actors/Player/TOSPlayer.h"

#include "../../../Helpers/TOS_Debug.h"

#include "TheOtherSideMP/Helpers/TOS_AI.h"
#include "TheOtherSideMP/Helpers/TOS_Cache.h"
#include "TheOtherSideMP/Helpers/TOS_Entity.h"

CTOSMasterModule::CTOSMasterModule()
	: tos_sv_mc_LookDebugDraw(0),
	tos_cl_JoinAsMaster(0),
	tos_cl_SlaveEntityClass(nullptr),
	tos_sv_SlaveSpawnDelay(0),
	tos_sv_mc_StartControlDelay(0),
	tos_sv_pl_inputAccel(0),
	m_pLocalMasterClient(nullptr)
{
	m_masters.clear();
	m_scheduledTakeControls.clear();
}

CTOSMasterModule::~CTOSMasterModule() = default;

void CTOSMasterModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	// Обнаружен баг
	//	При вводе sv_restart у Synchronizer не вызывается Release(), но при этом он всё же куда-то пропадает. 
	//	Флаг у Synchronizer на неудаляемость присутствует.
	// Исправление: 
	//	- удаление вручную при событии eGE_GameReset (неактуально)
	//	- оставить как есть и создавать синхронизатор при событии eEGE_GamerulesStartGame.
	//		это гарантирует его наличие при sv_restart'е

	switch (event.event)
	{
	//case eEGE_GamerulesStartGame: is ok
	//case eEGE_GamerulesPostInit: not ok
	//case eEGE_GameModuleInit: not ok
	//case eEGE_EntitiesPostReset: not ok
	case eEGE_GamerulesStartGame: //not ok long time still ok
	{
		//10/10/2023, 18:43 Теперь синхронизаторы нужно спавнить через редактор (заебался я с ними возиться)
		//CreateSynchonizer<CTOSMasterSynchronizer>("MasterSynchronizer", "TOSMasterSynchronizer");
		break;
	}
	//case eEGE_ActorPostInit: no ok on client
	case eEGE_SynchronizerCreated:
	{
		if (pGO)
		{
			m_pSynchonizer = dynamic_cast<CTOSMasterSynchronizer*>(pGO->AcquireExtension("TOSMasterSynchronizer"));
			assert(m_pSynchonizer);
		}

		TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_SynchronizerRegistered, "For Master Module", true));

		break;
	}
	//case eEGE_SynchronizerRegistered:
	case eEGE_ClientEnteredGame:
	{
		if (pEntity)
		{
			if (gEnv->bServer)
			{
				const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entId));
				assert(pPlayer);

				const auto masterNeedSlave = pPlayer->GetSpectatorMode() == 0 && 
					IsMaster(pPlayer->GetEntity()) && 
					!GetSlave(pPlayer->GetEntity());

				if (masterNeedSlave)
				{
					TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_PlayerJoinedGame, "after sv_restart", true));
				}
			}
			else if(gEnv->bClient)
			{
				const int joinAsAlien = gEnv->pConsole->GetCVar("tos_cl_JoinAsMaster")->GetIVal();
				if (joinAsAlien > 0)
				{
					const auto clientEntityId = g_pGame->GetIGameFramework()->GetClientActorId();
					const auto pSlaveEntClsCvar = gEnv->pConsole->GetCVar("tos_cl_SlaveEntityClass");
					assert(pSlaveEntClsCvar);

					const auto params = NetMasterAddingParams(clientEntityId, pSlaveEntClsCvar->GetString());

					assert(m_pSynchonizer);
					m_pSynchonizer->RMISend(CTOSMasterSynchronizer::SvRequestMasterAdd(), params, eRMI_ToServer);
				}
			}
		}

		break;
	}
	//case eEGE_PlayerJoinedGame:
	case eEGE_PlayerJoinedGame:
	{
		if (gEnv->bServer)
		{
			if (IsMaster(pEntity))
			{
				STOSMasterInfo info;
				GetMasterInfo(pEntity, info);

				const string slaveClsName = info.desiredSlaveClassName;
				const string slaveName = entName + "(Slave)";
				const auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(slaveClsName.c_str());

				assert(pClass);
				if (!pClass)
				{
					CryLogAlways("[C++][%s][%s] Class %s not found", 
						TOS_Debug::GetEnv(), 
						TOS_Debug::GetAct(1),
						slaveClsName.c_str());
					break;
				}

				STOSEntityDelaySpawnParams params;
				params.authorityPlayerName = entName;
				params.savedName = slaveName;
				params.scheduledTimeStamp = gEnv->pTimer->GetFrameStartTime().GetSeconds();
				params.spawnDelay = tos_sv_SlaveSpawnDelay;
				params.tosFlags |= TOS_ENTITY_FLAG_MUST_RECREATED;
				params.vanilla.bStaticEntityId = true;
				params.vanilla.nFlags |= ENTITY_FLAG_NEVER_NETWORK_STATIC | ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_CASTSHADOW;
				params.vanilla.pClass = pClass;
				params.vanilla.qRotation = pEntity->GetWorldRotation();
				params.vanilla.vPosition = pEntity->GetWorldPos();
				params.willBeSlave = true;

				bool scheduled = TOS_Entity::SpawnDelay(params);
				if (!scheduled)
				{
					auto pSavedSlave = g_pTOSGame->GetEntitySpawnModule()->GetSavedSlaveByAuthName(entName);
					CRY_ASSERT_MESSAGE(!pSavedSlave, "The slave must not be created or saved before the player joins the game");
				}
			}

		}

		break;
	}
	case eEGE_SlaveReadyToObey:
	{
		if (gEnv->bServer && pEntity)
		{
			const auto masterChannelId = event.int_value;

			//Обнаружен баг
			//При sv_restart на сервере синхронизатор появляется раньше и раб тоже
			//а на клиенте много позже, так что при попытке отправить RMI'шку на клиент
			//во время его загрузки вызывает дисконнект клиента. И пишет что у синхронизатора
			//не найден GameObjectExtension
			// Исправление: запланировать передачу контроля после того как сущность готова
			// подчиняться и когда клиент полностью инициализирован

			const auto inGame = g_pGame->GetGameRules()->IsChannelInGame(masterChannelId);
			if (!inGame)
			{
				//раб, клиент мастера
				STOSStartControlInfo info;
				info.masterChannelId = masterChannelId;
				info.slaveId = entId;
				info.startDelay = gEnv->pConsole->GetCVar("tos_sv_MasterStartControlDelay")->GetFVal();

				ScheduleMasterStartControl(info);

				break;
			}

			NetMasterStartControlParams params;
			params.slaveId = entId;

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::ClMasterClientStartControl(),
				params,
				eRMI_ToClientChannel,
				masterChannelId
			);
		}

		break;
	}
	case eEGE_MasterClientStartControl:
	{
		// Излишняя проверка на клиента
		if (gEnv->bClient)
		{
			NetMasterStartControlParams params;
			params.slaveId = pEntity->GetId();
			params.masterId = g_pGame->GetIGameFramework()->GetClientActorId();

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::SvRequestMasterClientStartControl(),
				params,
				eRMI_ToServer
			);
		}
		break;
	}
	case eEGE_MasterClientStopControl:
	{
		// Излишняя проверка на клиента
		if (gEnv->bClient)
		{
			NetMasterStopControlParams params;
			params.masterId = g_pGame->GetIGameFramework()->GetClientActorId();

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::SvRequestMasterClientStopControl(),
				params,
				eRMI_ToServer
			);
		}
		break;
	}
	case eEGE_PlayerJoinedSpectator:
	{
		const auto pPlayer = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entId);
		assert(pPlayer);

		if (gEnv->bServer)
		{
			const int playerChannelId = pPlayer->GetChannelId();

			if (IsMaster(pEntity))
			{
				const auto pSlave = GetSlave(pEntity);
				if (pSlave)
				{
					assert(m_pSynchonizer);
					m_pSynchonizer->RMISend(
						CTOSMasterSynchronizer::ClMasterClientStopControl(),
						NetGenericNoParams(),
						eRMI_ToClientChannel,
						playerChannelId
					);

					TOS_Entity::RemoveEntityForced(pSlave->GetId());
				}
			}
		}

		if (gEnv->bClient && pPlayer->IsClient())
		{
			const auto pLocalMS = g_pTOSGame->GetMasterModule()->GetMasterClient();
			assert(pLocalMS);

			if (pLocalMS->GetSlaveEntity())
			{
				pLocalMS->StopControl();
			}
		}

		break;
	}
	//case eEGE_ActorRelease:
	case eEGE_ClientDisconnect:
	{
		if (pEntity && gEnv->bServer)
		{
			const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entId));
			if (!pPlayer)
				break;

			if (IsMaster(pPlayer->GetEntity()))
			{
				const auto pSlave = GetSlave(pPlayer->GetEntity());
				if (pSlave)
				{
					//Вызывало баг, когда в какой-то момент раб перестал появляться после sv_restart
					//Вернул, чтобы сущность удалялась после отключения клиента, а не когда актёр клиента вызвал Release
					TOS_Entity::RemoveEntityForced(pSlave->GetId());
				}

				const auto pSavedEnt = g_pTOSGame->GetEntitySpawnModule()->GetSavedSlaveByAuthName(pPlayer->GetEntity()->GetName());
				if (pSavedEnt)
				{
					TOS_Entity::RemoveEntityForced(pSavedEnt->GetId());
				}
			}

			MasterRemove(pPlayer->GetEntity());
		}
		break;
	}
	case eEGE_EntityOnRemove:
	{
		if (gEnv->bServer && IsSlave(pEntity))
		{
			TOS_RECORD_EVENT(pEntity->GetId(), STOSGameEvent(eEGE_SlaveEntityOnRemove), "", true);
		}
		break;
	}
	default:
		break;
	}
}

void CTOSMasterModule::Init()
{
	CTOSGenericModule::Init();

	//Кешируем модели объектов, чтобы при их спавне не было подгрузок/подлагиваний
	TOS_Cache::CacheObject("Objects/Characters/Alien/trooper/Trooper.chr");
	TOS_Cache::CacheObject("Objects/Characters/Alien/trooper/trooper_leader.chr");
	TOS_Cache::CacheObject("Objects/Characters/Alien/scout/scout_base.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/scout/scout_leader.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/hunter/Hunter.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/AlienBase/AlienBase.cdf");

	m_scheduledTakeControls.clear();
}

void CTOSMasterModule::Update(float frametime)
{
	for (auto &schedPair : m_scheduledTakeControls)
	{
		const auto slaveId = schedPair.first;
		const auto masterChannelId = schedPair.second.masterChannelId;
		const auto inGame = g_pGame->GetGameRules()->IsChannelInGame(masterChannelId);

		auto& delay = schedPair.second.inGameDelay;

		if (inGame && delay < 0.0f)
		{
			NetMasterStartControlParams params;
			params.slaveId = slaveId;

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::ClMasterClientStartControl(),
				params,
				eRMI_ToClientChannel,
				masterChannelId
			);

			m_scheduledTakeControls.erase(slaveId);
			break;
		}

		delay -= gEnv->pTimer->GetFrameStartTime().GetSeconds();
	}

	if (gEnv->bServer)
	{
		// Очистим неактуальных мастеров.
		for (const auto& masterPair : m_masters)
		{
			const EntityId id = masterPair.first;

			const auto pMasterEntity = TOS_GET_ENTITY(id);
			if (!pMasterEntity)
			{
				m_masters.erase(id);
				break;
			}
		}
	}

	const auto pMC = GetMasterClient();
	if (pMC)
	{
		pMC->Update(frametime);
	}
}

void CTOSMasterModule::Serialize(TSerialize ser)
{
}

void CTOSMasterModule::MasterAdd(const IEntity* pMasterEntity, const char* slaveDesiredClass)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (!IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			auto info = STOSMasterInfo();
			info.desiredSlaveClassName = slaveDesiredClass;

			m_masters[id] = info;

			TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_MasterAdd, "", true));
		}
	}
}

void CTOSMasterModule::MasterRemove(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters.erase(id);

			TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_MasterRemove, "", true));
		}
	}
}

bool CTOSMasterModule::IsMaster(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		const auto it = m_masters.find(pMasterEntity->GetId());
		return it != m_masters.end();
	}

	return false;
}

IEntity* CTOSMasterModule::GetSlave(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (IsMaster(pMasterEntity))
		{
			for (const auto& masterSlavePair : m_masters)
			{
				if (masterSlavePair.first == pMasterEntity->GetId())
					return gEnv->pEntitySystem->GetEntity(masterSlavePair.second.slaveId);
			}				
		}
	}

	return nullptr;
}

void CTOSMasterModule::SetSlave(const IEntity* pMasterEntity, const IEntity* pSlaveEntity)
{
	assert(pMasterEntity);
	assert(pSlaveEntity);

	if (!IsMaster(pMasterEntity))
		return;

	m_masters[pMasterEntity->GetId()].slaveId = pSlaveEntity->GetId();
}

void CTOSMasterModule::ClearSlave(const IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	if (!IsMaster(pMasterEntity))
		return;

	m_masters[pMasterEntity->GetId()].slaveId = 0;
}

bool CTOSMasterModule::IsSlave(const IEntity* pEntity) const
{
	if (!pEntity)
		return false;

	for (auto &masterPair : m_masters)
	{
		if (masterPair.second.slaveId == pEntity->GetId())
			return true;
	}

	return false;
}

void CTOSMasterModule::DebugDraw(const Vec2& screenPos, float fontSize, float interval, int maxElemNum)
{
	//Header
	TOS_Debug::Draw2dText(
		screenPos.x, 
		screenPos.y - interval * 2,
		fontSize + 0.2f,
		"--- TOS Master System (MasterName:SlaveName) ---");

	//Body
	for (const auto& masterInfoPair : m_masters)
	{
		const auto pMasterActor = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(masterInfoPair.first));
		if (!pMasterActor)
			continue;

		const auto pMasterEnt = pMasterActor->GetEntity();
		if (!pMasterEnt)
			continue;

		const auto pSlaveEntity = GetSlave(pMasterEnt);

		const char* masterName = pMasterEnt->GetName();
		const char* slaveName = pSlaveEntity != nullptr ? pSlaveEntity->GetName() : "NULL";

		const int channelId = pMasterActor->GetChannelId();

		TOS_Debug::Draw2dText(
			screenPos.x,
			screenPos.y + channelId * interval, 
			fontSize, 
			"%i) %s:%s", 
			channelId, masterName, slaveName);
	}
}

bool CTOSMasterModule::GetMasterInfo(const IEntity* pMasterEntity, STOSMasterInfo& info)
{
	if (!IsMaster(pMasterEntity))
		return false;

	info = m_masters[pMasterEntity->GetId()];

	return true;
}

void CTOSMasterModule::SaveMasterClientParams(IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	if (IsMaster(pMasterEntity))
	{
		auto &params = m_masters[pMasterEntity->GetId()].mcSavedParams;

		params.pos = pMasterEntity->GetWorldPos();
		params.rot = static_cast<Quat>(pMasterEntity->GetWorldAngles());

		IAIObject* pAI = pMasterEntity->GetAI();
		if (pAI)
		{
			pAI->Event(AIEVENT_DISABLE, nullptr);
			params.species = TOS_AI::GetSpecies(pAI, false);
		}

		const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMasterEntity->GetId()));
		assert(pPlayer);

		const auto pSuit = pPlayer->GetNanoSuit();
		assert(pSuit);

		params.suitEnergy = pSuit->GetSuitEnergy();
		params.suitMode = pSuit->GetMode();

		pSuit->SetMode(NANOMODE_DEFENSE);

		// Сохраняем инвентарь и очищаем
		IInventory* pInventory = pPlayer->GetInventory();
		if (pInventory)
		{
			params.inventoryItems.clear();

			const auto itemsNum = pInventory->GetCount();

			//Push items id values to massive
			for (int slot = itemsNum; slot >= 0; slot--)
			{
				const EntityId itemId = pInventory->GetItem(slot);
				const auto     pItemEnt = TOS_GET_ENTITY(itemId);
				if (pItemEnt)
				{
					const char* name = pItemEnt->GetClass()->GetName();

					params.inventoryItems[slot] = name;
					//CryLogAlways("SAVE ITEM %s", name);
				}

			}

			const auto curItemId = pInventory->GetCurrentItem();
			const auto pCurEnt = TOS_GET_ENTITY(curItemId);
			if (pCurEnt)
			{
				params.currentItemClass = pCurEnt->GetClass()->GetName();
			}

			pInventory->RemoveAllItems();
		}

		//pSuit->SetModeDefect(NANOMODE_CLOAK, true);
		//pSuit->SetModeDefect(NANOMODE_SPEED, true);
		//pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
	}
}

void CTOSMasterModule::ApplyMasterClientParams(IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	if (IsMaster(pMasterEntity))
	{
		STOSMasterInfo info;
		GetMasterInfo(pMasterEntity, info);

		const auto& saved = info.mcSavedParams;

		const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMasterEntity->GetId()));
		assert(pPlayer);

		const auto pSuit = pPlayer->GetNanoSuit();
		assert(pSuit);

		const Vec3  pos = saved.pos;
		const Quat  rot = saved.rot;
		const int   species = saved.species;
		const float suitEnergy = saved.suitEnergy;
		uint        suitMode = saved.suitMode;

		if (pPlayer->GetHealth() > 0)
		{
			pMasterEntity->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), rot, pos));

			pSuit->SetSuitEnergy(suitEnergy);
			pSuit->SetMode(static_cast<ENanoMode>(suitMode));
		}

		IAIObject* pAI = pMasterEntity->GetAI();
		if (pAI)
		{
			pAI->Event(AIEVENT_ENABLE, nullptr);
			TOS_AI::SetSpecies(pAI, species);
		}

		const IInventory* pInventory = pPlayer->GetInventory();
		if (!pInventory)
			return;

		//Загружаем инвентарь
		auto& items = saved.inventoryItems;

		for (auto& itemPair : items)
		{
			string itemClass = itemPair.second;

			const bool isItem = g_pGame->GetIGameFramework()->GetIItemSystem()->IsItemClass(itemClass.c_str());
			if (isItem)
			{
				if (itemClass != saved.currentItemClass)
				{
					const auto itemId = g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pPlayer, itemClass, false, false, false);
					//CryLogAlways("LOAD SEC ITEM %s:%i", itemClass, itemId);
				}
				else
				{
					const auto itemId = g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pPlayer, itemClass, false, true, false);
					//CryLogAlways("LOAD PRIMARY ITEM %s:%i", itemClass, itemId);
				}

			}
		}
	}
}

void CTOSMasterModule::ScheduleMasterStartControl(const STOSStartControlInfo& info)
{
	const auto it = m_scheduledTakeControls.find(info.slaveId);
	if (it == m_scheduledTakeControls.end())
	{
		m_scheduledTakeControls[info.slaveId].masterChannelId = info.masterChannelId;
		m_scheduledTakeControls[info.slaveId].inGameDelay = info.startDelay;
	}
}

void CTOSMasterModule::GetMasters(std::map<EntityId, STOSMasterInfo>& masters) const
{
	masters = m_masters;
}

bool CTOSMasterModule::SetMasterDesiredSlaveCls(const IEntity* pEntity, const char* slaveDesiredClass)
{
	assert(pEntity);
	if (!pEntity)
		return false;

	if (!IsMaster(pEntity))
		return false;

	m_masters[pEntity->GetId()].desiredSlaveClassName = slaveDesiredClass;

	return true;
}
