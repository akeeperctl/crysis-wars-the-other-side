/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "MasterModule.h"

#include "Item.h"

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
#include <stdexcept>
#include <TheOtherSideMP/Helpers/TOS_NET.h>

CTOSMasterModule::CTOSMasterModule()
	: tos_cl_JoinAsMaster(0),
	tos_cl_playerFeedbackSoundsVersion(0),
	tos_cl_nanosuitSoundsVersion(0),
	tos_sv_mc_LookDebugDraw(0),
	//tos_cl_JoinAsMaster(0),
	//tos_cl_SlaveEntityClass(nullptr),
	tos_sv_SlaveSpawnDelay(0),
	tos_sv_mc_StartControlDelay(0),
	tos_sv_pl_inputAccel(0),
	tos_tr_double_jump_energy_cost(0),
	tos_tr_double_jump_melee_energy_cost(0),
	tos_tr_double_jump_melee_rest_seconds(0),
	tos_tr_melee_energy_costs(0),
	tos_tr_regen_energy_start_delay_sp(0),
	tos_tr_regen_energy_start_delay_mp(0),
	tos_tr_regen_energy_start_delay_20boundary(0),
	tos_tr_regen_energy_recharge_time_sp(0),
	tos_tr_regen_energy_recharge_time_mp(0),
	m_pLocalMasterClient(nullptr)
{
	m_masters.clear();
	m_scheduledTakeControls.clear();
}

CTOSMasterModule::~CTOSMasterModule()
{
	if (g_pGame->GetGameRules())
	{
		g_pGame->GetGameRules()->RemoveHitListener(this);
	}
};

void CTOSMasterModule::Reset()
{
	m_masters.clear();
	m_scheduledTakeControls.clear();
	m_pLocalMasterClient = nullptr;
}

void CTOSMasterModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	if (!pEntity)
	{
		//CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "OnExtraGameplayEvent Entity is NULL when");
		return;
	}

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
			// 10/10/2023, 18:43 Akeeper: Теперь синхронизаторы нужно спавнить через редактор (заебался я с ними возиться)
			//CreateSynchonizer<CTOSMasterSynchronizer>("MasterSynchronizer", "TOSMasterSynchronizer");

			if (g_pGame->GetGameRules())
			{
				g_pGame->GetGameRules()->AddHitListener(this);
			}

			break;
		}
		//case eEGE_ActorPostInit: no ok on client
		case eEGE_SynchronizerCreated:
		{
			if (pGO)
			{
				RegisterSynchronizer(static_cast<CTOSMasterSynchronizer*>(pGO->AcquireExtension("TOSMasterSynchronizer")));
				assert(GetSynchronizer() != nullptr);
			}

			TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_SynchronizerRegistered, "For Master Module", true));

			break;
		}
		case eEGE_ClientEnteredGame:
		{
			// В одиночной игре мастер будет задаваться по случаю
			// Т.е в момент когда идёт запрос на взятие раба под контроль 
			if (!gEnv->bMultiplayer)
				break;

			if (pEntity)
			{
				if (gEnv->bServer)
				{
					const auto pPlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entId));
					assert(pPlayer);

					const auto masterNeedSlave = pPlayer->GetSpectatorMode() == 0 &&
						IsMaster(pPlayer->GetEntity()) &&
						!GetCurrentSlave(pPlayer->GetEntity());

					if (masterNeedSlave)
					{
						TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_PlayerJoinedGame, "after sv_restart", true));
					}
				}

				//if(gEnv->bClient)
				//{
					//const int joinAsAlien = gEnv->pConsole->GetCVar("tos_cl_JoinAsMaster")->GetIVal();
					//if (joinAsAlien > 0)
					//{
						//const auto clientEntityId = g_pGame->GetIGameFramework()->GetClientActorId();
						//const auto pSlaveEntClsCvar = gEnv->pConsole->GetCVar("tos_cl_SlaveEntityClass");
						//assert(pSlaveEntClsCvar);

						//const auto params = NetMasterAddingParams(clientEntityId, pSlaveEntClsCvar->GetString());

						//assert(m_pSynchonizer);
						//m_pSynchonizer->RMISend(CTOSMasterSynchronizer::SvRequestMasterAdd(), params, eRMI_ToServer);
					//}
				//}
			}

			break;
		}
		//case eEGE_PlayerJoinedGame:
		case eEGE_PlayerJoinedGame:
		{
			if (gEnv->bServer)
			{
				const int    teamId = g_pGame->GetGameRules()->GetTeam(pEntity->GetId());
				const string teamName = g_pGame->GetGameRules()->GetTeamName(teamId);

				if (!IsMaster(pEntity) && teamName == "aliens")
				{
					MasterAdd(pEntity, "Trooper");
				}

				if (IsMaster(pEntity))
				{
					STOSMasterInfo info;
					GetMasterInfo(pEntity, &info);

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

					auto pSavedSlave = g_pTOSGame->GetEntitySpawnModule()->GetSavedSlaveByAuthName(entName);
					if (!pSavedSlave)
					{
						STOSEntityDelaySpawnParams params;
						params.authorityPlayerName = entName;
						params.savedName = slaveName;
						params.scheduledTimeStamp = gEnv->pTimer->GetFrameStartTime().GetSeconds();
						params.spawnDelay = tos_sv_SlaveSpawnDelay;
						params.tosFlags |= ENTITY_MUST_RECREATED;
						params.vanilla.bStaticEntityId = true;
						params.vanilla.nFlags |= ENTITY_FLAG_NEVER_NETWORK_STATIC | ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_CASTSHADOW;
						params.vanilla.pClass = pClass;
						params.vanilla.qRotation = pEntity->GetWorldRotation();
						params.vanilla.vPosition = pEntity->GetWorldPos();
						params.forceStartControl = true;

						TOS_Entity::SpawnDelay(params, true);
					}
				}
			}

			break;
		}
		case eEGE_ForceStartControl:
		{
			if (gEnv->bServer && pEntity)
			{
				const auto masterChannelId = event.int_value;
				auto pPlayer = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActorByChannelId(masterChannelId);
				assert(pPlayer);

				const int    teamId = g_pGame->GetGameRules()->GetTeam(pPlayer->GetEntityId());
				const string teamName = g_pGame->GetGameRules()->GetTeamName(teamId);

				// После sv_restart игрок начинает контролировать раба, но игрок не является мастером
				// Поэтому нужно игрока добавить в ряды мастеров
				if (!IsMaster(pPlayer->GetEntity()) && teamName == "aliens")
				{
					MasterAdd(pPlayer->GetEntity(), "Trooper");
				}

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
					auto pCvar = gEnv->pConsole->GetCVar("tos_sv_mc_StartControlDelay");

					//раб, клиент мастера
					STOSStartControlInfo info;
					info.masterChannelId = masterChannelId;
					info.slaveId = entId;
					info.startDelay = pCvar ? pCvar->GetFVal() : 0.0f;

					ScheduleMasterStartControl(info);

					break;
				}

				NetMasterStartControlParams params;
				params.slaveId = entId;
				params.factionPriority = eFP_Master;

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
		case eEGE_MasterClientOnStartControl:
		{
			// Излишняя проверка на клиента
			if (gEnv->bClient)
			{
				NetMasterStartControlParams params;
				params.slaveId = pEntity->GetId();
				params.masterId = g_pGame->GetIGameFramework()->GetClientActorId();

				auto pFlags = static_cast<uint*>(event.extra_data);
				if (pFlags)
				{
					params.masterFlags = *pFlags;
					delete pFlags;
				}

				assert(m_pSynchonizer);
				m_pSynchonizer->RMISend(
					CTOSMasterSynchronizer::SvRequestMasterClientStartControl(),
					params,
					eRMI_ToServer
				);
			}
			break;
		}
		case eEGE_MasterClientOnStopControl:
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
					const auto pSlave = GetCurrentSlave(pEntity);
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

					MasterRemove(pPlayer->GetEntity());
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
		case eEGE_ActorDead:
		{
			if (gEnv->bServer)
			{
				// Раб погиб -> убиваем Мастера
				IEntity* pSlaveEntity = pEntity;
				IEntity* pMasterEntity = GetMaster(pSlaveEntity);

				if (pSlaveEntity && pMasterEntity)
				{
					//IActor* const pMasterActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMasterEntity->GetId());
					//if (pMasterActor)
					//{
					//	const EntityId id = pMasterActor->GetEntityId();

					//	HitInfo info;
					//	info.SetDamage(9999);
					//	info.shooterId = id;
					//	info.targetId = id;

					//	//g_pGame->GetGameRules()->ServerHit(info);
					//}
				}
				else
				{
					// Мастер погиб -> убиваем Раба
					pMasterEntity = pEntity;
					pSlaveEntity = GetCurrentSlave(pMasterEntity);

					if (pMasterEntity && pSlaveEntity)
					{
						IActor* const pSlaveActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pSlaveEntity->GetId());
						if (pSlaveActor)
						{
							const EntityId id = pSlaveActor->GetEntityId();

							HitInfo info;
							info.SetDamage(9999);
							info.shooterId = id;
							info.targetId = id;

							g_pGame->GetGameRules()->ServerHit(info);
						}
					}
				}
			}

			break;
		}
		case eEGE_ClientDisconnect:
		{
			if (pEntity && gEnv->bServer)
			{
				const auto pPlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entId));
				if (!pPlayer)
					break;

				if (IsMaster(pPlayer->GetEntity()))
				{
					const auto pSlave = GetCurrentSlave(pPlayer->GetEntity());
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

	m_scheduledTakeControls.clear();
}

void CTOSMasterModule::Update(float frametime)
{
	for (auto it = m_scheduledTakeControls.begin(); it != m_scheduledTakeControls.end();)
	{
		const auto slaveId = it->first;
		const auto masterChannelId = it->second->masterChannelId;
		const bool inGame = g_pGame->GetGameRules()->IsChannelInGame(masterChannelId);

		float& delay = it->second->inGameDelay;

		if (inGame && delay < 0.0f)
		{
			NetMasterStartControlParams params;
			params.slaveId = slaveId;
			params.factionPriority = eFP_Master;

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::ClMasterClientStartControl(),
				params,
				eRMI_ToClientChannel,
				masterChannelId
			);

			it = m_scheduledTakeControls.erase(it);
			break;
		}
		else
		{
			delay -= gEnv->pTimer->GetFrameStartTime().GetSeconds();
			++it;
		}
	}


	if (gEnv->bServer)
	{
		// Очистим неактуальных мастеров.
		for (auto it = m_masters.begin(); it != m_masters.end(); it++)
		{
			const EntityId id = it->first;

			const auto pMasterEntity = TOS_GET_ENTITY(id);
			if (!pMasterEntity)
			{
				m_masters.erase(id);
				break;
			}
		}
	}

	//for (auto it = m_masters.begin(); it != m_masters.end(); it++)
	//{
	//	const EntityId id = it->first;
	//	auto pActor = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
	//	if (pActor)
	//	{
	//		auto pCharacter = pActor->GetAnimatedCharacter();

	//		if (pCharacter->GetPhysicalColliderMode() != eColliderMode_Spectator)
	//		{
	//			pCharacter->ForceRefreshPhysicalColliderMode();
	//			pCharacter->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "MasterModule::Update");
	//		}
	//	}
	//}

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
		const EntityId id = pMasterEntity->GetId();
		auto pActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));

		if (!IsMaster(pMasterEntity))
		{
			auto info = new STOSMasterInfo();
			info->desiredSlaveClassName = slaveDesiredClass;

			m_masters[id] = info;

			if (pActor)
			{
				pActor->SetMeMaster(true);
				pActor->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
			}

			TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_MasterAdd, "", true));
		}

		// Почему то в одиночной игре в редакторе IsMaster срабатывает не так как нужно
		if (!gEnv->bMultiplayer)
		{
			if (pActor)
			{
				pActor->SetMeMaster(true);
				pActor->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
			}
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

			auto pActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
			if (pActor)
			{
				pActor->SetMeMaster(false);
				pActor->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
			}


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

IEntity* CTOSMasterModule::GetCurrentSlave(const IEntity* pMasterEntity)
{
	if (pMasterEntity->GetClass() == g_pGame->GetGameRules()->GetEntity()->GetClass())
	{
		throw std::logic_error("GameRules не может быть мастером");
		return nullptr;
	}

	if (gEnv->bServer && pMasterEntity)
	{
		if (IsMaster(pMasterEntity))
		{
			auto it = m_masters.begin();
			for (; it != m_masters.end(); it++)
			{
				if (it->first == pMasterEntity->GetId())
					return gEnv->pEntitySystem->GetEntity(it->second->slaveId);
			}
		}
	}

	return nullptr;
}

void CTOSMasterModule::SetCurrentSlave(const IEntity* pMasterEntity, const IEntity* pSlaveEntity, uint masterFlags)
{
	assert(pMasterEntity);
	assert(pSlaveEntity);

	if (!IsMaster(pMasterEntity))
		return;

	m_masters[pMasterEntity->GetId()]->slaveId = pSlaveEntity->GetId();
	m_masters[pMasterEntity->GetId()]->flags = masterFlags;

	auto pActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pSlaveEntity->GetId()));
	if (pActor)
	{
		pActor->SetMeSlave(true);
		pActor->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}
}

void CTOSMasterModule::ClearCurrentSlave(const IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	if (!IsMaster(pMasterEntity))
		return;

	EntityId slaveId = m_masters[pMasterEntity->GetId()]->slaveId;

	auto pActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(slaveId));
	if (pActor)
	{
		pActor->SetMeSlave(false);
		pActor->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}

	m_masters[pMasterEntity->GetId()]->slaveId = 0;
	m_masters[pMasterEntity->GetId()]->flags = 0;
}

bool CTOSMasterModule::IsSlave(const IEntity* pPotentialSlave) const
{
	if (!pPotentialSlave)
		return false;

	auto it = m_masters.begin();
	for (; it != m_masters.end(); it++)
	{
		if (it->second->slaveId == pPotentialSlave->GetId())
			return true;
	}

	return false;
}

bool CTOSMasterModule::ReviveSlave(const IEntity* pSlaveEntity, const Vec3& revivePos, const Ang3& angles, const int teamId, const bool resetWeapons) const
{
	if (!pSlaveEntity)
		return false;

	auto pMasterActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetMaster(pSlaveEntity)->GetId());
	if (!pMasterActor)
		return false;

	auto pSlaveActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pSlaveEntity->GetId()));
	if (!pSlaveActor)
		return false;

	auto pGameRules = g_pGame->GetGameRules();

	// get out of vehicles before reviving
	if (IVehicle* pVehicle = pSlaveActor->GetLinkedVehicle())
		if (IVehicleSeat* pSeat = pVehicle->GetSeatForPassenger(pSlaveActor->GetEntityId()))
			pSeat->Exit(false);

	// stop using any mounted weapons before reviving
	if (auto pItem = static_cast<CItem*>(pSlaveActor->GetCurrentItem()))
		if (pItem->IsMounted())
			pItem->StopUse(pSlaveActor->GetEntityId());

	if (pGameRules->IsFrozen(pSlaveActor->GetEntityId()))
		pGameRules->FreezeEntity(pSlaveActor->GetEntityId(), false, false);

	//int health = 100;
	//if(!gEnv->bMultiplayer && pActor->IsClient())
	//health = g_pGameCVars->g_playerHealthValue;
	//pSlaveActor->SetMaxHealth(health);

	//if (!g_pGame->GetIGameFramework()->IsChannelOnHold(pMasterActor->GetChannelId()))
	pSlaveActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);

	//Matrix34 tm(pSlaveActor->GetEntity()->GetWorldTM());
	//tm.SetTranslation(revivePos);

	//pSlaveActor->GetEntity()->SetWorldTM(tm);
	//pSlaveActor->SetAngles(angles);

	if (resetWeapons)
	{
		//pActor->GetGameObject()->InvokeRMI(CActor::ClClearInventory(), CActor::NoParams(), eRMI_ToAllClients | eRMI_NoLocalCalls);

		//IInventory* pInventory = pActor->GetInventory();
		//pInventory->Destroy();
		//pInventory->Clear();

		pSlaveActor->ResetActorWeapons(1000);
	}

	pSlaveActor->NetReviveAt(revivePos, Quat(angles), teamId);
	pSlaveActor->GetGameObject()->InvokeRMI(CActor::ClRevive(), CActor::ReviveParams(revivePos, angles, teamId), eRMI_ToAllClients | eRMI_NoLocalCalls);

	return true;
}

void CTOSMasterModule::DebugDraw(const Vec2& screenPos, float fontSize, float interval, int maxElemNum)
{
	//Header
	DRAW_2D_TEXT(
		screenPos.x,
		screenPos.y - interval * 2,
		fontSize + 0.2f,
		"--- TOS Master System (MasterName:SlaveName) ---");

	//Body
	auto it = m_masters.begin();
	for (; it != m_masters.end(); it++)
	{
		const auto pMasterActor = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(it->first));
		if (!pMasterActor)
			continue;

		const auto pMasterEnt = pMasterActor->GetEntity();
		if (!pMasterEnt)
			continue;

		const auto pSlaveEntity = GetCurrentSlave(pMasterEnt);

		const char* masterName = pMasterEnt->GetName();
		const char* slaveName = pSlaveEntity != nullptr ? pSlaveEntity->GetName() : "NULL";

		const int channelId = pMasterActor->GetChannelId();

		DRAW_2D_TEXT(
			screenPos.x,
			screenPos.y + channelId * interval,
			fontSize,
			"%i) %s:%s",
			channelId, masterName, slaveName);
	}
}

bool CTOSMasterModule::GetMasterInfo(const IEntity* pMasterEntity, STOSMasterInfo* info)
{
	if (!IsMaster(pMasterEntity))
		return false;

	info = m_masters[pMasterEntity->GetId()].get();

	return true;
}

void CTOSMasterModule::SaveMasterClientParams(IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	IAIObject* pAI = pMasterEntity->GetAI();
	if (pAI)
		pAI->Event(AIEVENT_DISABLE, nullptr);

	auto& params = m_masters[pMasterEntity->GetId()]->mcSavedParams;
	params.dirty = true;

	params.pos = pMasterEntity->GetWorldPos();
	params.rot = static_cast<Quat>(pMasterEntity->GetWorldAngles());

	if (pAI)
		params.species = TOS_AI::GetSpecies(pAI, false);

	const auto pPlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMasterEntity->GetId()));
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
	CryLog("<c++> [SaveMasterClientParams] pos(%1.f,%1.f,%1.f), rot(%1.f,%1.f,%1.f), species '%i', energy '%1.f', suitMode '%i'",
		   params.pos.x, params.pos.y, params.pos.z, params.rot.v.x, params.rot.v.y, params.rot.v.z, params.species, params.suitEnergy, params.suitMode);
}

void CTOSMasterModule::ApplyMasterClientParams(IEntity* pMasterEntity)
{
	assert(pMasterEntity);

	if (IsMaster(pMasterEntity))
	{
		STOSMasterInfo info;
		GetMasterInfo(pMasterEntity, &info);

		const auto& saved = info.mcSavedParams;
		if (!saved.dirty)
			return;

		const auto pPlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pMasterEntity->GetId()));
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
			TOS_AI::SetSpecies(pAI, species);

		const IInventory* pInventory = pPlayer->GetInventory();
		if (!pInventory)
			return;

		//Загружаем инвентарь
		auto& items = saved.inventoryItems;
		auto it = items.begin();
		auto end = items.end();

		for (; it != end; it++)
		{
			string itemClass = it->second;

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

		CryLog("<c++> [ApplyMCSavedParams] pos(%1.f,%1.f,%1.f), rot(%1.f,%1.f,%1.f), species '%i', energy '%1.f', suitMode '%i'",
			   pos.x, pos.y, pos.z, rot.v.x, rot.v.y, rot.v.z, species, suitEnergy, suitMode);
	}
}

void CTOSMasterModule::ScheduleMasterStartControl(const STOSStartControlInfo& info)
{
	const auto it = m_scheduledTakeControls.find(info.slaveId);
	if (it == m_scheduledTakeControls.end())
	{
		m_scheduledTakeControls[info.slaveId]->masterChannelId = info.masterChannelId;
		m_scheduledTakeControls[info.slaveId]->inGameDelay = info.startDelay;
	}
}

void CTOSMasterModule::GetMasters(std::map<EntityId, _smart_ptr<STOSMasterInfo>>& masters) const
{
	masters = m_masters;
}

IEntity* CTOSMasterModule::GetMaster(const IEntity* pSlaveEntity) const
{
	if (!pSlaveEntity)
		return nullptr;

	auto it = m_masters.begin();
	auto end = m_masters.end();

	for (; it != end; it++)
	{
		if (it->second->slaveId == pSlaveEntity->GetId())
		{
			return TOS_GET_ENTITY(it->first);
		}
	}

	return nullptr;
}

bool CTOSMasterModule::SetMasterDesiredSlaveCls(const IEntity* pEntity, const char* slaveDesiredClass)
{
	assert(pEntity);
	if (!pEntity)
		return false;

	if (!IsMaster(pEntity))
		return false;

	m_masters[pEntity->GetId()]->desiredSlaveClassName = slaveDesiredClass;

	return true;
}
