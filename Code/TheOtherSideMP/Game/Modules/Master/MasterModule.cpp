#include "StdAfx.h"
#include "MasterModule.h"

#include "Game.h"
#include "IEntitySystem.h"
#include "MasterSynchronizer.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"
#include "../../../Actors/Player/TOSPlayer.h"
#include "../../../Helpers/TOS_Debug.h"

#include "TheOtherSideMP/Game/TOSGameCvars.h"
#include "TheOtherSideMP/Helpers/TOS_Cache.h"
#include "TheOtherSideMP/Helpers/TOS_Entity.h"


CTOSMasterModule::CTOSMasterModule()
{
	m_masters.clear();
}

CTOSMasterModule::~CTOSMasterModule()
{
}

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
	case eEGE_GamerulesStartGame:
	{
		CreateSynchonizer<CTOSMasterSynchronizer>("MasterSynchronizer", "TOSMasterSynchronizer");
		break;
	}
	//case eEGE_ActorPostInit: no ok on client
	case eEGE_SynchronizerCreated:	
	{
		if (pEntity && gEnv->bClient)
		{
			const int joinAsAlien = gEnv->pConsole->GetCVar("tos_cl_JoinAsMaster")->GetIVal();
			if (joinAsAlien > 0)
			{
				const auto clientEntityId = g_pGame->GetIGameFramework()->GetClientActorId();
				const auto pSlaveEntClsCvar = gEnv->pConsole->GetCVar("tos_cl_SlaveEntityClass");
				assert(pSlaveEntClsCvar);

				const auto params = MasterAddingParams(clientEntityId, pSlaveEntClsCvar->GetString());

				assert(m_pSynchonizer);
				m_pSynchonizer->RMISend(CTOSMasterSynchronizer::SvRequestMasterAdd(), params, eRMI_ToServer);
			}
		}

		break;
	}
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

				TOS_Entity::SpawnDelay(params);
			}

		}

		break;
	}
	case eEGE_SlaveReadyToObey:
	{
		if (gEnv->bServer && pEntity)
		{
			const auto masterChannelId = event.int_value;

			//TODO:
			//1) Создать RMI на клиенте для начала управления рабом
			//2) Отправить RMI на клиент мастера, чтобы мастер начал управлять рабом
			MasterStartControlParams params;
			params.slaveId = pEntity->GetId();

			assert(m_pSynchonizer);
			m_pSynchonizer->RMISend(
				CTOSMasterSynchronizer::ClMasterStartControl(), 
				params, 
				eRMI_ToClientChannel, 
				masterChannelId);
		}

		break;
	}
	case eEGE_PlayerJoinedSpectator:
	{
		if (gEnv->bServer)
		{
			if (IsMaster(pEntity))
			{
				const auto pSlave = GetSlave(pEntity);
				if (pSlave)
					TOS_Entity::RemoveEntityForced(pSlave->GetId());
			}
		}

		break;
	}
	case eEGE_ActorRelease:
	{
		if (pEntity && gEnv->bServer)
		{
			const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
			if (!pPlayer)
				break;

			if (IsMaster(pPlayer->GetEntity()))
			{
				const auto pSlave = GetSlave(pEntity);
				if (pSlave)
					TOS_Entity::RemoveEntityForced(pSlave->GetId());
			}

			MasterRemove(pPlayer->GetEntity());
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
}

void CTOSMasterModule::Update(float frametime)
{
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
