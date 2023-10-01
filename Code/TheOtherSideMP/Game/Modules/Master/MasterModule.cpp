#include "StdAfx.h"
#include "MasterModule.h"

#include "Game.h"
#include "IEntitySystem.h"
#include "MasterSynchronizer.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"
#include "../../../Actors/Player/TOSPlayer.h"
#include "../../../Helpers/TOS_Debug.h"


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
		//Create Synchronizer entity
		CreateSynchonizer<CTOSMasterSynchronizer>("MasterSynchronizer", "TOSMasterSynchronizer");


		//auto pSynchronizer = gEnv->pEntitySystem->FindEntityByName("MasterSynchronizer");
		//if (pSynchronizer)
		//{
		//	IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchronizer->GetId());
		//	if (pGO)
		//	{
		//		m_pSynchronizer = dynamic_cast<CTOSMasterSynchronizer*>(pGO->AcquireExtension("TOSMasterSynchronizer"));
		//		assert(m_pSynchronizer);
		//	}

		//	return;
		//}

		////auto pSynchronizerCls = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TOSMasterSynchronizer");
		////assert(pSynchronizerCls);

		////if (!pSynchronizerCls)
		////	return;

		//SEntitySpawnParams params;
		//params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
		//params.bStaticEntityId = true;
		//params.sName = "MasterSynchronizer";
		////params.nFlags |= ENTITY_FLAG_NO_PROXIMITY;
		////Флаг ENTITY_FLAG_UNREMOVABLE не работает при sv_restart
		//params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;
		////params.id = 2;

		//pSynchronizer = gEnv->pEntitySystem->SpawnEntity(params);
		//assert(pSynchronizer);

		//if (!pSynchronizer)
		//	return;

		////IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchronizer->GetId());
		//IGameObject* pGO = g_pGame->GetIGameFramework()->GetIGameObjectSystem()->CreateGameObjectForEntity(pSynchronizer->GetId());
		//if (pGO)
		//{
		//	m_pSynchronizer = dynamic_cast<CTOSMasterSynchronizer*>(pGO->AcquireExtension("TOSMasterSynchronizer"));
		//	assert(m_pSynchronizer);

		//	pGO->ForceUpdate(true);
		//}
		break;
	}
	case eGE_GameReset:
	{
		//Delete Synchronizer entity

		//if (m_pSynchronizer)
		//{
		//	m_pSynchronizer->GetEntity()->ClearFlags(ENTITY_FLAG_UNREMOVABLE);

		//	gEnv->pEntitySystem->RemoveEntity(m_pSynchronizer->GetEntityId());
		//	m_pSynchronizer = nullptr;
		//}
		//break;
	}
	case eEGE_ActorPostInit:
	{
		if (pEntity && gEnv->bServer)
		{
			auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
			if (!pPlayer)
				break;

			MasterAdd(pPlayer->GetEntity());
		}
		break;
	}
	case eEGE_ActorReleased:
	{
		if (pEntity && gEnv->bServer)
		{
			auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
			if (!pPlayer)
				break;

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
}

void CTOSMasterModule::Update(float frametime)
{
}

void CTOSMasterModule::Serialize(TSerialize ser)
{
}

void CTOSMasterModule::MasterAdd(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (!IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters[id] = 0;

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
		auto it = m_masters.find(pMasterEntity->GetId());
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
			for (auto& MasterSlavePair : m_masters)
			{
				if (MasterSlavePair.first == pMasterEntity->GetId())
					return gEnv->pEntitySystem->GetEntity(MasterSlavePair.second);
			}				
		}
	}

	return nullptr;
}

void CTOSMasterModule::DebugDrawMasters(const Vec2& screenPos, float fontSize, float interval, int maxElemNum)
{
	//Header
	TOS_Debug::Draw2dText(
		screenPos.x, 
		screenPos.y - interval * 2,
		fontSize + 0.2f,
		"--- TOS Master System (MasterName:SlaveName) ---");

	//Body
	for (auto& masterInfoPair : m_masters)
	{
		auto pMasterActor = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(masterInfoPair.first));
		if (!pMasterActor)
			continue;

		auto pMasterEnt = pMasterActor->GetEntity();
		if (!pMasterEnt)
			continue;

		auto pSlaveEntity = GetSlave(pMasterEnt);

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

void CTOSMasterModule::GetMasters(std::map<EntityId, EntityId>& masters)
{
	masters = m_masters;
}
