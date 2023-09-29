#include "StdAfx.h"
#include "IEntitySystem.h"

#include "Game.h"

#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/TOSGame.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Helpers/TOS_Debug.h"

#include "MasterModule.h"
#include "RMISender.h"

CTOSMasterModule::CTOSMasterModule():
	m_pRMISender(nullptr)
{
	m_masters.clear();
	g_pTOSGame->ModuleAdd(this, false);
}

CTOSMasterModule::~CTOSMasterModule()
{
	g_pTOSGame->ModuleRemove(this, false);
}

void CTOSMasterModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	switch (event.event)
	{
	case eEGE_GamerulesPostInit:
	{
		//Spawn the RMI Sender entity

		auto pRMISender = gEnv->pEntitySystem->FindEntityByName("MasterSystem_RMISender");
		if (pRMISender)
		{
			IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pRMISender->GetId());
			if (pGO)
			{
				m_pRMISender = dynamic_cast<CTOSMasterRMISender*>(pGO->AcquireExtension("TOSMasterRMISender"));
				assert(m_pRMISender);
			}

			return;
		}

		auto pRMISenderCls = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TOSMasterRMISender");
		assert(pRMISenderCls);

		if (!pRMISenderCls)
			return;

		
		SEntitySpawnParams params;
		params.pClass = pRMISenderCls;
		//params.bStaticEntityId = true;
		params.sName = "MasterSystem_RMISender";
		params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;
		params.id = 2;

		pRMISender = gEnv->pEntitySystem->SpawnEntity(params);
		assert(pRMISender);

		if (!pRMISender)
			return;

		IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pRMISender->GetId());
		if (pGO)
		{
			m_pRMISender = dynamic_cast<CTOSMasterRMISender*>(pGO->AcquireExtension("TOSMasterRMISender"));
			assert(m_pRMISender);

			pGO->ForceUpdate(true);
		}

		break;
	}
	case eGE_Connected:
	{
		if (pEntity && gEnv->bServer)
		{
			MasterAdd(pEntity);
		}
		break;
	}
	case eGE_Disconnected:
	{
		if (pEntity && gEnv->bServer)
		{
			MasterRemove(pEntity);
		}
		break;
	}
	default:
		break;
	}
}

void CTOSMasterModule::GetMemoryStatistics(ICrySizer* s)
{
}

void CTOSMasterModule::Init()
{
	
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

CTOSMasterRMISender* CTOSMasterModule::GetRMISender() const
{
	return m_pRMISender;
}
