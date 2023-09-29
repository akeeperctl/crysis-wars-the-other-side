#include "StdAfx.h"
#include "IEntitySystem.h"

#include "Game.h"

#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/TOSGame.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Helpers/TOS_Debug.h"

#include "MasterSystemClientServer.h"
#include "RMISender.h"

CTOSModuleMasterSystem::CTOSModuleMasterSystem():
	m_pRMISender(nullptr)
{
	m_masters.clear();
	g_pTOSGame->ModuleAdd(this, false);
}

CTOSModuleMasterSystem::~CTOSModuleMasterSystem()
{
	g_pTOSGame->ModuleRemove(this, false);
}

void CTOSModuleMasterSystem::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
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
		params.bStaticEntityId = true;
		params.sName = "MasterSystem_RMISender";
		params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;
		//params.id = 2;

		pRMISender = gEnv->pEntitySystem->SpawnEntity(params);
		assert(pRMISender);

		IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pRMISender->GetId());
		if (pGO)
		{
			m_pRMISender = dynamic_cast<CTOSMasterRMISender*>(pGO->AcquireExtension("TOSMasterRMISender"));
			assert(m_pRMISender);

			pGO->ForceUpdate(true);
		}

		break;
	}

	default:
		break;
	}
}

void CTOSModuleMasterSystem::GetMemoryStatistics(ICrySizer* s)
{
}

void CTOSModuleMasterSystem::Init()
{
	
}

void CTOSModuleMasterSystem::Update(float frametime)
{
}

void CTOSModuleMasterSystem::Serialize(TSerialize ser)
{
}

void CTOSModuleMasterSystem::MasterAdd(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (!IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters[id] = 0;

			TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_MasterAdd, "SERVER", true));
		}
	}
}

void CTOSModuleMasterSystem::MasterRemove(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters.erase(id);

			TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_MasterRemove, "SERVER", true));
		}
	}
}

bool CTOSModuleMasterSystem::IsMaster(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		return stl::find_in_map(m_masters, pMasterEntity->GetId(), 0) != 0;
	}

	return false;
}

IEntity* CTOSModuleMasterSystem::GetSlave(const IEntity* pMasterEntity)
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

void CTOSModuleMasterSystem::DebugDrawMasters(const Vec2& screenPos, float fontSize, float interval, int maxElemNum)
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

CTOSMasterRMISender* CTOSModuleMasterSystem::GetRMISender() const
{
	return m_pRMISender;
}
