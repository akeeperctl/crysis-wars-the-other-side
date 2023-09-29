#include "StdAfx.h"
#include "IEntitySystem.h"

#include "Game.h"

#include "../Actor Files/Player/TOSPlayer.h"
#include "../Game Files/TOSGame.h"
#include "../Game Files/TOSGameEventRecorder.h"
#include "../Helpers/TOS_Debug.h"

#include "MasterSystemClientServer.h"

CTOSMasterSystem::CTOSMasterSystem()
{
	m_masters.clear();
}

CTOSMasterSystem::~CTOSMasterSystem()
{
	g_pTOSGame->ModuleRemove(this, false);
}

void CTOSMasterSystem::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
}

void CTOSMasterSystem::GetMemoryStatistics(ICrySizer* s)
{
}

void CTOSMasterSystem::Init()
{
	g_pTOSGame->ModuleAdd(this, false);
}

void CTOSMasterSystem::Update(float frametime)
{
}

void CTOSMasterSystem::Serialize(TSerialize ser)
{
}

void CTOSMasterSystem::AddMaster(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (!IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters[id] = 0;
			g_pTOSGame->GetEventRecorder()->RecordEvent(id, STOSGameEvent(eEGE_MasterAdd, "SERVER", true));
		}
	}
}

void CTOSMasterSystem::RemoveMaster(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		if (IsMaster(pMasterEntity))
		{
			const EntityId id = pMasterEntity->GetId();

			m_masters.erase(id);
			g_pTOSGame->GetEventRecorder()->RecordEvent(id, STOSGameEvent(eEGE_MasterRemove, "SERVER", true));
		}
	}
}

bool CTOSMasterSystem::IsMaster(const IEntity* pMasterEntity)
{
	if (gEnv->bServer && pMasterEntity)
	{
		return stl::find_in_map(m_masters, pMasterEntity->GetId(), 0) != 0;
	}

	return false;
}

IEntity* CTOSMasterSystem::GetSlave(const IEntity* pMasterEntity)
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

void CTOSMasterSystem::DebugDrawMasters(const Vec2& screenPos, float fontSize, float interval, int maxElemNum)
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