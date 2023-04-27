#include "StdAfx.h"

#include "IEntity.h"
#include "IActorSystem.h"

#include "Game.h"
#include "GameCVars.h"
#include "GameRules.h"

#include "StrategicArea.h"
#include "../Control/ControlSystem.h"
#include "ConquerorSystem.h"
#include "../AI Files/AIActionTracker.h"

#include "AreaVehicleSpawnPoint.h"

#include "../Helpers/TOS_Script.h"
#include "../Helpers/TOS_Debug.h"

CAreaVehicleSpawnPoint::CAreaVehicleSpawnPoint()
{
	m_speciesVehicleArchetypesMap.clear();
	m_vehicleAutoDestroyScheduler.clear();
	m_vehiclePassengers.clear();
	m_vehiclePlayingAlarm.clear();
	m_vehicleRespawnScheduler.clear();

	m_bIsEnabled = false;

	m_spawnedVehicleId = 0;
	m_canRespawn = false;
	m_canAbandon = false;
	m_abandonTimer = 0;
	m_respawnTimer = 0;

	//m_lastTimeVehicleDestroyed = 0;

	m_species = eST_NEUTRAL;
}

bool CAreaVehicleSpawnPoint::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (!Reset())
		return false;

	if (!GetGameObject()->BindToNetwork())
		return false;

	return true;
}

void CAreaVehicleSpawnPoint::PostInit(IGameObject* pGameObject)
{
	GetGameObject()->EnableUpdateSlot(this, 0);
	g_pControlSystem->GetConquerorSystem()->AddVehicleSpawner(this);
	GetLuaValues();
}

void CAreaVehicleSpawnPoint::Release()
{
	g_pControlSystem->GetConquerorSystem()->RemoveVehicleSpawner(this);

	delete this;
}

void CAreaVehicleSpawnPoint::FullSerialize(TSerialize ser)
{

}

bool CAreaVehicleSpawnPoint::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

void CAreaVehicleSpawnPoint::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	//if (m_oldSpawnedVehicles.size() != 0)
		//UpdateOldVehicleScheduler(ctx.fFrameTime);

	UpdateSpawnedVehicleScheduler(ctx.fFrameTime);
}

void CAreaVehicleSpawnPoint::HandleEvent(const SGameObjectEvent& event)
{

}

void CAreaVehicleSpawnPoint::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_RESET:
	{
		Reset();
		GetLuaValues();
		break;
	}
	case ENTITY_EVENT_START_GAME:
	{
		//GameStart();
		break;
	}
	}
}

void CAreaVehicleSpawnPoint::SetAuthority(bool auth)
{

}

void CAreaVehicleSpawnPoint::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CAreaVehicleSpawnPoint::OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params)
{
	auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
	if (!pVehicle)
		return;

	if (event == eVE_Destroyed)
	{
		//m_lastTimeVehicleDestroyed = ;

		g_pControlSystem->GetConquerorSystem()->OnVehicleDestroyed(pVehicle);
		g_pControlSystem->GetAIActionTracker()->OnVehicleDestroyed(pVehicle);

		m_vehiclePassengers.clear();
		m_vehiclePlayingAlarm.clear();
		m_vehicleAutoDestroyScheduler.clear();
		m_vehicleRespawnScheduler[m_spawnedVehicleId] = gEnv->pTimer->GetFrameStartTime().GetSeconds();

		//auto iter1 = m_vehiclePassengers.find(pVehicle->GetEntityId());
		//if (iter1 != m_vehiclePassengers.end())
		//	m_vehiclePassengers.erase(iter1);

		//auto iter2 = m_vehiclePlayingAlarm.find(pVehicle->GetEntityId());
		//if (iter2 != m_vehiclePlayingAlarm.end())
		//	m_vehiclePlayingAlarm.erase(iter2);

		//auto iter3 = m_vehicleAutoDestroyScheduler.find(pVehicle->GetEntityId());
		//if (iter3 != m_vehicleAutoDestroyScheduler.end())
		//	m_vehicleAutoDestroyScheduler.erase(iter3);

		//CryLogAlways("%s[C++][Vehicle Destroyed %i]", STR_PURPLE, pVehicle->GetEntityId());

		pVehicle->UnregisterVehicleEventListener(this);
	}

	//CryLogAlways("[C++][On Vehicle Event][Event Num %i][Params: bParam %i, entityId %i, fParam %1.f, fParam2 %1.f, iParam %i, vParam (%1.f,%1.f,%1.f)", 
	//	event, params.bParam, params.entityId, params.fParam, params.fParam2, params.iParam, params.vParam.x, params.vParam.y, params.vParam.z);
}

void CAreaVehicleSpawnPoint::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
	if (!IsEnabled())
		return;

	if (!pVehicle)
		return;

	if (!pActor)
		return;

	const auto vehId = pVehicle->GetEntityId();
	const auto actId = pActor->GetEntityId();

	if (m_spawnedVehicleId != vehId)
		return;

	//if (!stl::find(m_spawnedVehicles, vehId) && !stl::find(m_oldSpawnedVehicles, vehId))
	//	return;

	m_vehiclePassengers[vehId].push_back(actId);

	auto iter = m_vehicleAutoDestroyScheduler.find(vehId);
	if (iter != m_vehicleAutoDestroyScheduler.end())
	{
		CancelVehicleAutoDestroy(vehId);
		m_vehiclePlayingAlarm[vehId] = 0;
	}
}

void CAreaVehicleSpawnPoint::OnExitVehicle(IActor* pActor)
{
	if (!m_pArea)
	{
		return;
	}

	if (!IsEnabled())
		return;

	if (!pActor)
		return;

	if (m_vehiclePassengers.size() == 0)
		return;

	const auto passId = pActor->GetEntityId();

	auto it = m_vehiclePassengers.begin();
	auto end = m_vehiclePassengers.end();

	for (; it != end; it++)
	{
		const auto vehicleId = it->first;
		auto& passengers = it->second;

		if (m_spawnedVehicleId != vehicleId)
			continue;

		//if (!stl::find(m_spawnedVehicles, vehicleId) && !stl::find(m_oldSpawnedVehicles, vehicleId))
		//	continue;

		const auto pit = std::find(passengers.begin(), passengers.end(), passId);

		if (pit == passengers.end())
			continue;

		passengers.erase(pit);

		if (passengers.size() > 0)
			continue;

		auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(vehicleId);
		if (!pVehicle)
			continue;

		if (m_canAbandon && !pVehicle->IsDestroyed())
		{
			ScheduleVehicleAutoDestroy(vehicleId, m_abandonTimer);
		}		
		else
		{
			CryLogAlways("$8[C++][Vehicle Spawner %s][Vehicle %i CANNOT AUTO DESTRUCT]",
				GetEntity()->GetName(), pVehicle->GetEntityId());
		}
	}
}

void CAreaVehicleSpawnPoint::OnStrategicAreaSpeciesChanged(ESpeciesType species)
{
	m_species = species;
	//DefineOldVehicles(m_spawnedVehicles);

	auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
	if (!pVehicle)
		SpawnRandomArchetype(species);
}

void CAreaVehicleSpawnPoint::AttachStrategicArea(CStrategicArea* pArea)
{
	if (!pArea)
		return;

	if (m_species != eST_NEUTRAL)
		return;

	m_species = pArea->GetSpecies();

	auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
	if (!pVehicle)
		SpawnRandomArchetype(m_species);

	m_pArea = pArea;
}

EntityId CAreaVehicleSpawnPoint::SpawnVehicleArchetype(const char* archetype)
{
	if (!gEnv->bServer)
		return 0;

	if (!IsEnabled())
		return 0;

	auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archetype);
	if (!pArchetype)
	{
		CryLogAlways("%s[C++][ERROR][Vehicle Spawner %s][Can not load entity archetype %s]",
			STR_RED, GetEntity()->GetName(), archetype);

		return 0;
	}

	SEntitySpawnParams spawnParams;
	spawnParams.nFlags = ENTITY_FLAG_CASTSHADOW | ENTITY_FLAG_ON_RADAR;
	spawnParams.pArchetype = pArchetype;
	spawnParams.vScale = Vec3(1);
	spawnParams.vPosition = GetEntity()->GetWorldPos();
	spawnParams.qRotation = GetEntity()->GetRotation();
	spawnParams.bStaticEntityId = false;

	IEntity* pEntity = nullptr;

	if (m_spawnedVehicleId == 0)
	{
		pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams, false);
	}

	if (!pEntity)
		return 0;

	if (pEntity && gEnv->pEntitySystem->InitEntity(pEntity, spawnParams))
	{
		m_spawnedVehicleId = pEntity->GetId();
	}

	char buffer[256];
	sprintf(buffer, "%i", m_spawnedVehicleId);
	const string nameCombination = string(archetype) + buffer;
	pEntity->SetName(nameCombination);

	auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
	if (pVehicle)
	{
		m_vehiclePassengers.insert(std::make_pair(pVehicle->GetEntityId(), 0));

		TOS_Script::GetEntityProperty(pVehicle->GetEntity(), "Conquest", "bVehRespawn", m_canRespawn);
		TOS_Script::GetEntityProperty(pVehicle->GetEntity(), "Conquest", "nVehRespawnTimer", m_respawnTimer);
		TOS_Script::GetEntityProperty(pVehicle->GetEntity(), "Conquest", "bVehAbandon", m_canAbandon);
		TOS_Script::GetEntityProperty(pVehicle->GetEntity(), "Conquest", "nVehAbandonTimer", m_abandonTimer);

		if (g_pGameCVars->conq_debug_log_vehiclespawner)
			CryLogAlways("%s[C++][Vehicle Spawner %s][Spawn Archetype][%i][Respawn %i][Abandon %i][AbandonTimer %1.f][RespawnTimer %1.f]", 
				STR_YELLOW, GetEntity()->GetName(), pVehicle->GetEntity()->GetId(), 
				m_canRespawn, m_canAbandon, m_abandonTimer, m_respawnTimer);

		pVehicle->RegisterVehicleEventListener(this, GetEntity()->GetName());
	}

	//auto params = SVehicleEventParams();
	//params.bParam = true;
	//pVehicle->BroadcastVehicleEvent(eVE_BlockDoors, params);

	return pVehicle->GetEntityId();
}

void CAreaVehicleSpawnPoint::DeleteSpawnedVehicle()
{
	auto pEntity = gEnv->pEntitySystem->GetEntity(m_spawnedVehicleId);
	if (pEntity)
		gEnv->pEntitySystem->RemoveEntity(m_spawnedVehicleId, false);

	m_vehicleRespawnScheduler.erase(m_spawnedVehicleId);
	m_spawnedVehicleId = 0;
	m_abandonTimer = 0;
	m_canAbandon = false;
	m_canRespawn = false;
}

IVehicle* CAreaVehicleSpawnPoint::GetSpawnedVehicle()
{
	return g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
}

ESpeciesType CAreaVehicleSpawnPoint::GetSpecies() const
{
	return m_species;
}

bool CAreaVehicleSpawnPoint::Reset()
{
	DeleteSpawnedVehicle();

	//m_lastTimeVehicleDestroyed = 0;
	m_vehicleRespawnScheduler.clear();
	m_speciesVehicleArchetypesMap.clear();
	m_vehicleAutoDestroyScheduler.clear();
	m_vehiclePassengers.clear();
	m_vehiclePlayingAlarm.clear();
	m_species = eST_NEUTRAL;

	return true;
}

bool CAreaVehicleSpawnPoint::IsEnabled()
{
	auto pTable = GetEntity()->GetScriptTable();
	if (pTable)
	{
		SmartScriptTable props;
		if (pTable->GetValue("Properties", props))
			props->GetValue("bEnable", m_bIsEnabled);
	}

	return m_bIsEnabled;
}

void CAreaVehicleSpawnPoint::UpdateSpawnedVehicleScheduler(float frametime)
{
	auto pGameRules = g_pGame->GetGameRules();
	if (pGameRules)
	{
		auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_spawnedVehicleId);
		if (!pVehicle)
			return;

		auto destrIter = m_vehicleAutoDestroyScheduler.find(m_spawnedVehicleId);
		if (destrIter != m_vehicleAutoDestroyScheduler.end())
		{
			float& timer = destrIter->second;
			timer -= frametime;

			if ((m_vehiclePlayingAlarm[m_spawnedVehicleId] == 0) && (timer > 4.5 && timer < 5.5))
			{
				auto zero = Vec3Constants<float>::fVec3_Zero;
				auto oneY = Vec3Constants<float>::fVec3_OneY;
				auto name = "sounds/interface:multiplayer_interface:mp_vehicle_alarm";

				auto pSoundProxy = (IEntitySoundProxy*)pVehicle->GetEntity()->GetProxy(ENTITY_PROXY_SOUND);
				if (pSoundProxy)
					m_vehiclePlayingAlarm[m_spawnedVehicleId] = pSoundProxy->PlaySound(name, zero, oneY, FLAG_SOUND_3D, eSoundSemantic_Vehicle);
			}

			if (timer <= 0)
			{
				const auto& pos = pVehicle->GetEntity()->GetWorldPos();
				pVehicle->OnHit(m_spawnedVehicleId, m_spawnedVehicleId, 18000, pos, 1, "normal", false);

				m_vehiclePlayingAlarm[m_spawnedVehicleId] = 0;
				//m_vehicleAutoDestroyScheduler.erase(destrIter);
			}
		}


		auto respIter = m_vehicleRespawnScheduler.find(m_spawnedVehicleId);
		if (respIter != m_vehicleRespawnScheduler.end())
		{
			const float lastDestroyedTime = gEnv->pTimer->GetFrameStartTime().GetSeconds() - respIter->second;

			if (lastDestroyedTime > m_respawnTimer)
			{
				//CryLogAlways("%s[C++][Vehicle Respawn %i]", STR_PURPLE, pVehicle->GetEntityId());
				DeleteSpawnedVehicle();
				SpawnRandomArchetype(m_species);
			}
		}
	}
}

void CAreaVehicleSpawnPoint::ScheduleVehicleAutoDestroy(EntityId entityId, float timer)
{
	if (!gEnv->bServer)
		return;

	auto iter = m_vehicleAutoDestroyScheduler.find(entityId);
	if (iter == m_vehicleAutoDestroyScheduler.end())
		m_vehicleAutoDestroyScheduler.insert(std::make_pair(entityId, timer));
}

void CAreaVehicleSpawnPoint::CancelVehicleAutoDestroy(EntityId entityId)
{
	auto it = m_vehicleAutoDestroyScheduler.find(entityId);
	if (it != m_vehicleAutoDestroyScheduler.end())
		m_vehicleAutoDestroyScheduler.erase(it);
}

void CAreaVehicleSpawnPoint::GetLuaValues()
{
	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	m_speciesVehicleArchetypesMap.clear();

	const int maxSpeciesCount = ESpeciesType::eST_LastPlayableSpecies;
	SmartScriptTable props;

	auto pScriptTable = GetEntity()->GetScriptTable();
	if (pScriptTable && pScriptTable->GetValue("Properties", props))
	{
		props->GetValue("bEnable", m_bIsEnabled);

		for (int i = 0; i <= maxSpeciesCount; i++)
		{
			const ESpeciesType currentSpecies = ESpeciesType(i);

			SmartScriptTable speciesTable;			

			char elementIndex[256];
			sprintf(elementIndex, "%i", i);

			string speciesElementIndex = elementIndex;
			string speciesElementName = "Species_" + speciesElementIndex;
			const char* name = speciesElementName;		

			if (props->GetValue(name, speciesTable))
			{
				const int vehicleVariationCount = 4;
				const char* vehicleArchetype = "";

				std::vector<const char*> archetypes;			

				for (int y = 0; y < vehicleVariationCount; y++)
				{
					char indexChar[256];
					sprintf(indexChar, "%i", y);

					string paramIndex = indexChar;
					string paramName = "vehicle_" + paramIndex;

					//CryLogAlways("archetypeName %s", paramName);

					if (speciesTable->GetValue(paramName.c_str(), vehicleArchetype))
					{
						if (strcmp(vehicleArchetype, "") != 0)
						{
							//CryLogAlways("[C++][Vehicle Spawner %s][Species %i][Add Archetype %s]",
								//GetEntity()->GetName(), currentSpecies, vehicleArchetype);

							archetypes.push_back(vehicleArchetype);
						}
					}
				}

				m_speciesVehicleArchetypesMap.insert(std::make_pair(currentSpecies, archetypes));
			}
		}
	}
}

EntityId CAreaVehicleSpawnPoint::SpawnRandomArchetype(ESpeciesType species)
{
	auto iter = m_speciesVehicleArchetypesMap.cbegin();
	auto end = m_speciesVehicleArchetypesMap.cend();

	for (; iter != end; iter++)
	{
		if (iter->first == species)
		{
			auto& archetypes = iter->second;

			if (archetypes.size() == 0)
				return 0;

			const int max = archetypes.size();
			const int random = Random(0, max);
			return SpawnVehicleArchetype(archetypes[random]);
		}
	}

	return 0;
}