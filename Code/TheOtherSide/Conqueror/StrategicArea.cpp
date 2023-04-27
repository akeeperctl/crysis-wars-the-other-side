#include "StdAfx.h"

#include "SpawnPoint.h"
#include "ConquerorSystem.h"
#include "ConquerorSpeciesClass.h"
#include "AreaVehicleSpawnPoint.h"
#include "StrategicArea.h"
#include "ConquerorCommander.h"
#include "ConquerorChannel.h"
#include "../Control/ControlSystem.h"
#include "../Squad/SquadSystem.h"
#include "../Helpers/TOS_Debug.h"
#include "../Helpers/TOS_Script.h"
#include "../Helpers/TOS_STL.h"
#include "../Helpers/TOS_Vehicle.h"

#include "GameRules.h"
#include "GameCVars.h"

#include "IEntity.h"
#include "IVehicleSystem.h"

constexpr auto LANDSPOT = "LAND";
constexpr auto AIRSPOT = "AIR";

AABB GetAABBFromBOX(const primitives::box& obb)
{
	OBB box;
	box.c = obb.center;
	box.m33 = obb.Basis;
	box.h = obb.size * 0.5f;

	AABB bounds;
	bounds.SetAABBfromOBB(box);

	return bounds;
};

primitives::box& GetBoxFromAABB(const IEntity* pEntity, const AABB& bounds)
{
	primitives::box obb;
	obb.Basis = Matrix33(pEntity->GetWorldTM());
	obb.center = pEntity->GetWorldPos() + bounds.GetCenter();
	obb.size = bounds.GetSize();
	obb.bOriented = true;

	return obb;
};

CStrategicArea::CStrategicArea()
{
	m_DefinedMultiplayerSide = EMultiplayerSide::SPECIES;
	//m_OldOwnerSpecies = ESpeciesType::eST_NEUTRAL;
	m_Species = ESpeciesType::eST_NEUTRAL;
	m_LuaSpecies = ESpeciesType::eST_NEUTRAL;
	m_CapturingSpecies = ESpeciesType::eST_NEUTRAL;
	m_CaptureState = ECaptureState::NOTCAPTURED;

	m_buyZone.reset();
	m_flagEntityId = 0;
	m_centreAnchorId = 0;
	m_ShapeEntityId = 0; 
	m_Team = 0;
	m_LuaTeam = 0;

	m_CaptureStep = 1.0f;
	m_CaptureTime = 15.0f;
	m_CaptureRequirement = 1;
	m_FinalCaptureProgress = 0.0f;

	m_bIsEnabled = false;
	m_bIsCapturable = false;
	m_bCanUnlockClassesForAI = false;
	m_bCanUnlockClassesForPlayer = false;
	m_bSpawnSquadFromClasses = false;
	m_SpawnedMembersCount = 0;
	
	m_bookedUnloadSpots.clear();
	m_bookedSpawnPoints.clear();
	m_forcedspawnedSquadId.clear();
	m_unlockedClasses.clear();
	m_AreaFlags.clear();
	m_SpeciesMembers.clear();
	m_CaptureProgress.clear();

	m_pQueue = nullptr;

	for (int i = eST_NEUTRAL; i < eST_LAST; i++)
	{
		m_SpeciesMembers.insert(std::make_pair(ESpeciesType(i), 0));
		m_CaptureProgress.insert(std::make_pair(ESpeciesType(i), 0));
	}

	m_spawnPoints.clear();
	//m_airSpawnPoints.clear();
	m_vehicleSpawners.clear();

	ResetVehicleUnloadSpots();
}

CStrategicArea::~CStrategicArea()
{
	SAFE_DELETE(m_pQueue);
}

//bool CCapturableArea::IsPointWithinArea(Vec3& min, Vec3& max, Vec3& point)
//{
//	if (min <= point && point <= max);
//		return true;
//
//	return false;
//}

bool CStrategicArea::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	m_pQueue = new SAreaQueueInfo();

	if (!Reset())
		return false;

	if (!GetGameObject()->BindToNetwork())
		return false;

	return true;
}

void CStrategicArea::PostInit(IGameObject* pGameObject)
{
	GetLuaValues();
	GetGameObject()->EnableUpdateSlot(this, 0);

	g_pControlSystem->GetConquerorSystem()->AddStrategicArea(this);
}

void CStrategicArea::ChangeAgentCount(EntityId entityId, const char* operation)
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (m_DefinedMultiplayerSide == EMultiplayerSide::SPECIES)
	{
		IAIObject* pEntityAI = pEntity->GetAI();
		ESpeciesType speciesType = (ESpeciesType)pEntityAI->CastToIAIActor()->GetParameters().m_nSpecies;

		if (!m_SpeciesMembers.empty())
		{
			if (string(operation) == "+")
			{
				m_SpeciesMembers.at(speciesType)++;
			}
			else if (string(operation) == "-")
			{
				m_SpeciesMembers.at(speciesType)--;

				if (m_SpeciesMembers.at(speciesType) <= 0)
					m_SpeciesMembers.at(speciesType) = 0;
			}
		}
	}
}

const int CStrategicArea::GetAgentCount(ESpeciesType speciesType)
{
	return m_SpeciesMembers.at(speciesType);
}

void CStrategicArea::GetMaxAgentInfo(int& agentCount, ESpeciesType& agentSpecies)
{
	int max = 0;
	ESpeciesType maxAgentSpecies = ESpeciesType::eST_NEUTRAL;

	for (int i = ESpeciesType::eST_FirstPlayableSpecies; i < ESpeciesType::eST_LastPlayableSpecies; i++)
	{
		int currentAgentCount = m_SpeciesMembers.at(ESpeciesType(i));
		ESpeciesType currentAgentSpecies= ESpeciesType(i);

		//max = currentAgentCount;
		//maxAgentSpecies = currentAgentSpecies;
		//static int max = currentAgentCount;
		//static ESpeciesType maxAgentSpecies = currentAgentSpecies;

		if (max < currentAgentCount)
		{
			max = currentAgentCount;
			maxAgentSpecies = currentAgentSpecies;
		}	
		
		if (i == ESpeciesType::eST_LastPlayableSpecies - 1)
		{
			agentCount = max;
			agentSpecies = maxAgentSpecies;
		}		
	}	
}

ESpeciesType CStrategicArea::GetSpecies() const
{
	return m_Species;
}

int CStrategicArea::GetTeam()
{
	return m_Team;
}

int CStrategicArea::GetEnemiesCount(ESpeciesType species)
{
	int finalAgentCount = 0;

	for (int i = ESpeciesType::eST_FirstPlayableSpecies; i < ESpeciesType::eST_LastPlayableSpecies; i++)
	{
		ESpeciesType currentAgentSpecies = ESpeciesType(i);
	
		if (species != currentAgentSpecies)
		{
			int currentAgentCount = m_SpeciesMembers.at(ESpeciesType(i));
			finalAgentCount += currentAgentCount;
		}
	}

	return finalAgentCount;
}

void CStrategicArea::StartCapturing(ESpeciesType capturingSpecies)
{
	if (m_CapturingSpecies != capturingSpecies)
		m_CapturingSpecies = capturingSpecies;

	if (m_CaptureState!=ECaptureState::CAPTURING)
		m_CaptureState = ECaptureState::CAPTURING;
}

void CStrategicArea::StartUncapturing(ESpeciesType capturingSpecies)
{
	if (m_CapturingSpecies != capturingSpecies)
		m_CapturingSpecies = capturingSpecies;

	if (m_CaptureState != ECaptureState::UNCAPTURING)
		m_CaptureState = ECaptureState::UNCAPTURING;
}

bool CStrategicArea::GetContested(ESpeciesType capturingSpecies)
{
//TODO: function work bugged but not cretical!!! 12.05.2022

	for (int i = (int)eST_NEUTRAL; i < eST_LastPlayableSpecies; i++)
	{
		const auto species = ESpeciesType(i);

		if (species != capturingSpecies)
		{
			if (GetAgentCount(species) > 0) //== GetAgentCount(capturingSpecies))
			{
				//if (g_pGameCVars->conq_debug_log_area)
				//	CryLogAlways("[C++][Strategic Area][Get Contested] TRUE");
				return true;
			}
				
		}
	}

	//if (g_pGameCVars->conq_debug_log_area)
	//	CryLogAlways("[CCapturableArea]::[GetContested] FALSE");
	return false;
}

void CStrategicArea::SetCaptured(ESpeciesType& capturingSpecies)
{
	SetCaptureProgress(capturingSpecies, m_CaptureTime);

	m_Species = capturingSpecies;
	capturingSpecies = ESpeciesType::eST_NEUTRAL;
	
	OnCaptured(m_Species);
}

void CStrategicArea::SetNeutral(bool reset)
{
	if (!reset && (m_Species != eST_NEUTRAL))
	{
		auto pConqueror = g_pControlSystem->GetConquerorSystem();
		if (pConqueror)
		{
			//Disable respawn for old spawned squad
			if (m_forcedspawnedSquadId[m_Species] > 0)
			{
				auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromId(m_forcedspawnedSquadId[m_Species]);
				if (pSquad)
				{
					for (auto& member : pSquad->GetAllMembers())
					{
						auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
						if (!pActor)
							continue;

						auto pChannel = pConqueror->GetConquerorChannel(pActor->GetEntity());
						if (!pChannel)
							continue;

						pChannel->DisableRespawnRequest(true);
					}
				}
			}

			pConqueror->OnAreaLost(this, m_Species);

			for (auto spawnerId : m_vehicleSpawners)
			{
				auto pSpawner = pConqueror->GetVehicleSpawner(spawnerId);
				if (!pSpawner)
					continue;

				if (pSpawner->GetSpecies() != eST_NEUTRAL)
					pSpawner->OnStrategicAreaSpeciesChanged(eST_NEUTRAL);
			}
		}
	}

	//m_OldOwnerSpecies = m_Species;
	m_Species = ESpeciesType::eST_NEUTRAL;
	m_CapturingSpecies = ESpeciesType::eST_NEUTRAL;
	m_CaptureState = ECaptureState::NOTCAPTURED;

	auto pFlagEntity = gEnv->pEntitySystem->GetEntity(m_flagEntityId);
	if (pFlagEntity)
		Script::CallMethod(pFlagEntity->GetScriptTable(), "SetSpecies", m_Species);
	
	for (int i = ESpeciesType::eST_NEUTRAL; i < ESpeciesType::eST_LastPlayableSpecies; i++)
		SetCaptureProgress(ESpeciesType(i), 0.0f);

	CreateScriptEvent("OnNeutral", 1);
}

void CStrategicArea::OnCaptured(ESpeciesType species)
{
	if (species != eST_NEUTRAL)
	{
		auto pConqueror = g_pControlSystem->GetConquerorSystem();

		//First of all, you need to respawn vehicles, and then do everything else
		for (auto spawnerId : m_vehicleSpawners)
		{
			auto pSpawner = pConqueror->GetVehicleSpawner(spawnerId);
			if (!pSpawner)
				continue;

			if (pSpawner->GetSpecies() != species)
				pSpawner->OnStrategicAreaSpeciesChanged(species);
		}

		if (pConqueror)
			pConqueror->OnAreaCaptured(this, species);

		if (CanSpawnSquadFromClasses())
		{
			std::vector<const char*> classes;
			GetUnlockedClasses(species, classes);
			SpawnSquadForced(species, classes);
		}

		auto pFlagEntity = gEnv->pEntitySystem->GetEntity(m_flagEntityId);
		if (pFlagEntity)
			Script::CallMethod(pFlagEntity->GetScriptTable(), "SetSpecies", species);
	}

	m_CaptureState = ECaptureState::CAPTURED;
	CreateScriptEvent("OnCaptured", (float)species);
}

void CStrategicArea::OnContested(ESpeciesType species)
{
	m_CaptureState = ECaptureState::CONTESTED;
	CreateScriptEvent("OnContested", 1);
}

void CStrategicArea::OnActorDeath(IActor* pActor)
{
	if (!pActor)
		return;

	if (IsActorInside(pActor))
		stl::find_and_erase(m_ActorsInside, pActor->GetEntityId());
}

void CStrategicArea::OnClientEnter(bool vehicle)
{
	if (g_pGameCVars->conq_debug_log_area)
		CryLogAlways("[C++][Strategic Area %s][On Client Enter]", GetEntity()->GetName());

	if (IsEnabled())
		g_pControlSystem->GetConquerorSystem()->OnClientAreaEnter(this, vehicle);
}

void CStrategicArea::OnClientExit()
{
	if (g_pGameCVars->conq_debug_log_area)
		CryLogAlways("[C++][Strategic Area %s][On Client Exit]", GetEntity()->GetName());

	if (IsEnabled())
		g_pControlSystem->GetConquerorSystem()->OnClientAreaExit();
}

void CStrategicArea::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
	if (!pActor || !pVehicle)
		return;

	//Vehicle spawner OnEnterVehicle
	for (auto spawnerId : m_vehicleSpawners)
	{
		auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(spawnerId);
		if (!pSpawner)
			continue;

		pSpawner->OnEnterVehicle(pActor, pVehicle);
	}
}

void CStrategicArea::OnExitVehicle(IActor* pActor)
{
	if (!pActor)
		return;

	//Vehicle spawner OnExitVehicle

	for (auto spawnerId : m_vehicleSpawners)
	{
		auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(spawnerId);
		if (!pSpawner)
			continue;

		pSpawner->OnExitVehicle(pActor);
	}

	auto pNewActor = static_cast<CActor*>(pActor);
	auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pNewActor->m_vehicleStats.lastOperatedVehicleId);
	if (pVehicle && TOS_Vehicle::ActorIsDriver(pActor))
		UnbookUnloadSpot(pVehicle->GetEntity());	
}

void CStrategicArea::OnVehicleDestroyed(IVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	UnbookUnloadSpot(pVehicle->GetEntity());
}

float CStrategicArea::GetCaptureTime() const
{
	return m_CaptureTime;
}

ESpeciesType CStrategicArea::GetCapturingSpecies() const
{
	return m_CapturingSpecies;
}

float CStrategicArea::GetEnemyCaptureProgress(ESpeciesType species) const
{
	for (int i = ESpeciesType::eST_FirstPlayableSpecies; i < ESpeciesType::eST_LastPlayableSpecies; i++)
	{
		ESpeciesType currentAgentSpecies = ESpeciesType(i);

		if (species != currentAgentSpecies)
		{
			float currentProgress = GetCaptureProgress(currentAgentSpecies);

			if (currentProgress != 0.0f)
				return currentProgress;
		}
	}

	return 0.0f;
}

bool CStrategicArea::IsNeutral() const
{
	if (m_Species == ESpeciesType::eST_NEUTRAL)
		return true;

	return false;
}

void CStrategicArea::CreateScriptEvent(const char* event, float value, const char* str /*= NULL*/)
{
	IEntity* pEntity = GetEntity();
	IScriptTable* pScriptTable = pEntity ? pEntity->GetScriptTable() : 0;

	if (pScriptTable)
	{
		HSCRIPTFUNCTION scriptEvent(NULL);
		pScriptTable->GetValue("ScriptEvent", scriptEvent);

		if (scriptEvent)
			Script::Call(gEnv->pScriptSystem, scriptEvent, pScriptTable, event, value, str);

		gEnv->pScriptSystem->ReleaseFunc(scriptEvent);
	}

}

void CStrategicArea::AddSpawnPoint(EntityId id)
{
	auto pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (pEntity)
	{
		if (!FindSpawnPoint(pEntity->GetId()))
		{
			bool air = false;
			TOS_Script::GetEntityProperty(pEntity, "bForAirUnits", air);

			const int index = m_spawnPoints.size();

			CSpawnPoint* spawnPoint = new CSpawnPoint(pEntity, this);
			spawnPoint->SetIndex(index);
			spawnPoint->m_isForAirUnits = air;

			m_spawnPoints.push_back(spawnPoint);

			//if (pSpawnpoint->m_isForAirUnits)
				//CryLogAlways("$3[C++][Strategic Area %s][Add AIR Spawn Location %i][Entity Id %i]", GetEntity()->GetName(), m_spawnPoints.size(), id);
		}

		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$3[C++][Strategic Area %s][Add Spawn Location][Entity Id %i]", GetEntity()->GetName(), id);
	}
}

bool CStrategicArea::FindSpawnPoint(EntityId id)
{
	for (auto pSpawnpoint : m_spawnPoints)
	{
		if (pSpawnpoint->GetEntityId() == id)
			return true;
	}

	return false;
}

void CStrategicArea::RemoveSpawnPoint(EntityId id)
{
	auto it = m_spawnPoints.begin();
	auto end = m_spawnPoints.end();

	for (; it != end; it++)
	{
		CSpawnPoint* pSpawnPoint = *it;
		if (pSpawnPoint->GetEntityId() == id)
		{
			m_spawnPoints.erase(it);
			return;
		}	
	}
}

void CStrategicArea::AddVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner)
{
	if (!pSpawner)
	{
		CryLogAlways("NOt find spawner");
	}

	if (pSpawner)
	{
		if (!stl::find(m_vehicleSpawners, pSpawner->GetEntityId()))
		{
			if (g_pGameCVars->conq_debug_log_vehiclespawner)
				CryLogAlways("$3[C++][Strategic Area %s][Add Vehicle Spawner][Entity Id %i]",
					GetEntity()->GetName(), pSpawner->GetEntityId());

			m_vehicleSpawners.push_back(pSpawner->GetEntityId());

			pSpawner->AttachStrategicArea(this);
		}	
	}
}

bool CStrategicArea::FindVehicleSpawner(EntityId id) const
{
	for (auto spwId : m_vehicleSpawners)
	{
		if (spwId == id)
			return true;
	}
}

CAreaVehicleSpawnPoint* CStrategicArea::GetVehicleSpawner(EntityId id) const
{
	auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(id);
	if (!pSpawner)
		return nullptr;

	for (auto entId : m_vehicleSpawners)
	{
		if (pSpawner->GetEntityId() == id)
			return pSpawner;
	}

	return nullptr;
}

void CStrategicArea::RemoveVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner)
{
	if (!pSpawner)
		return;

	/*auto it = m_vehicleSpawners.begin();
	auto end = m_vehicleSpawners.end();

	for (; it != end; it++)
	{
		auto pSpawner = *it;
		if (pSpawner->GetEntityId() == pSpawner->GetEntityId())
		{
			m_vehicleSpawners.erase(it);
			return;
		}
	}*/

	stl::find_and_erase(m_vehicleSpawners, pSpawner->GetEntityId());
}

CSpawnPoint* CStrategicArea::GetSpawnPointInstance(EntityId entityId)
{
	for (auto pSpawnpoint : m_spawnPoints)
	{
		if (pSpawnpoint->m_entityId == entityId)
			return pSpawnpoint;
	}

	return nullptr;
}

CSpawnPoint* CStrategicArea::GetSpawnPoint(ESpawnpointGameStatusFlag flag)
{
	for (auto pSpawnPoint : m_spawnPoints)
	{
		//Get only spawn point which have no recently spawned
		if (flag == eSGSF_NotHaveRecentlySpawned)
		{
			if (pSpawnPoint->IsRecentlySpawned())
				continue;
		}

		return pSpawnPoint;
	}

	CryLogAlways("%s[C++][ERROR][Area %s][GetSpawnPoint][SpawnPoints %i][Flag %i][Cannot Find spawn point]", 
		TOS_Debug::GetLogColor(ELogColor::red), GetEntity()->GetName(), m_spawnPoints.size(), flag);

	return nullptr;
}


CSpawnPoint* CStrategicArea::GetSpawnPoint(ESpawnpointGameStatusFlag flag, uint speciesClassFlags)
{
	for (auto pSpawnpoint : m_spawnPoints)
	{
		//Get only spawn point which have no recently spawned
		if (flag == eSGSF_NotHaveRecentlySpawned)
		{
			if (pSpawnpoint->IsRecentlySpawned())
				continue;
		}

		if (speciesClassFlags & eSCF_IsAir)
		{
			if (pSpawnpoint->IsForAir())
				continue;
		}

		return pSpawnpoint;
	}

	CryLogAlways("%s[C++][ERROR][Area %s][GetSpawnPoint][SpawnPoints Count %i][Flag %i][Cannot Find spawn point with species class flag %i]",
		STR_RED, GetEntity()->GetName(), int(m_spawnPoints.size()), flag, speciesClassFlags);

	return nullptr;
}
CSpawnPoint* CStrategicArea::GetSpawnPointAt(int index, bool air)
{
	for (auto pSpawnpoint : m_spawnPoints)
	{
		if (pSpawnpoint->IsForAir() != air)
			continue;

		if (pSpawnpoint->GetIndex() == index)
			return pSpawnpoint;
	}

	return nullptr;
}

bool CStrategicArea::IsHaveFlag(EAreaFlag flag)
{
	return stl::find(m_AreaFlags, flag);
}

const TAreaFlags& CStrategicArea::GetFlags()
{
	return m_AreaFlags;
}

void CStrategicArea::GetUnlockedClasses(ESpeciesType species, std::vector<const char*>& classes)
{
	const auto iter = m_unlockedClasses.find(species);
	if (iter == m_unlockedClasses.end())
		return;

	for (auto unlockedClassName : m_unlockedClasses[species])
		classes.push_back(unlockedClassName);
}

void CStrategicArea::SpawnSquadForced(ESpeciesType species, const std::vector<const char*>& classes)
{
	auto pSquadSystem = g_pControlSystem->GetSquadSystem();
	if (!pSquadSystem)
		return;

	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	if (!pConqueror->IsGamemode())
		return;

	if (g_pGameCVars->conq_botsJoinBeforePlayer == 1)
	{
		if (!pConqueror->m_haveBotsSpawned)
			return;
	}

	if (classes.size() == 0)
		return;

	auto pSquad = pConqueror->CreateSquadFromSpecies(species);
	if (!pSquad)
	{
		CryLogAlways("%s[C++][Forced Spawn Squad][FAILED][Cause: Can not create the squad]", STR_RED);
		return;
	}

	if (m_forcedspawnedSquadId[species] > 0)
	{
		auto pSquad = pSquadSystem->GetSquadFromId(m_forcedspawnedSquadId[m_Species]);
		if (!pSquad)
			return;

		auto pLeader = pSquad->GetLeader();
		if (pLeader)
		{
			auto pChannel = pConqueror->GetConquerorChannel(pLeader->GetEntity());
			if (pChannel)
			{
				//const auto classFlags = pChannel->GetClass()->GetFlags();
				//const auto pointFlags = eSGSF_NotHaveRecentlySpawned;

				pChannel->DisableRespawnRequest(false);
			}
		}

		for (auto& member : pSquad->GetAllMembers())
		{
			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
			if (!pActor)
				continue;

			auto pChannel = pConqueror->GetConquerorChannel(pActor->GetEntity());
			if (!pChannel)
				continue;

			//const auto classFlags = pChannel->GetClass()->GetFlags();
			//const auto pointFlags = eSGSF_NotHaveRecentlySpawned;

			pChannel->DisableRespawnRequest(false);
			//pChannel->SpawnActor(eRC_ForcedRespawn, GetSpawnPoint(pointFlags, classFlags));
		}
	}
	else
	{
		//Define Leader Class
		CSpeciesClass* pLeaderClass = nullptr;

		//Get Classes
		std::vector<const char*> memberClasses;

		auto it = classes.begin();
		auto end = classes.end();

		for (; it != end; it++)
		{
			const auto pClass = pConqueror->GetClass(species, *it);

			if (!pClass)
				continue;

			if (pClass->IsNonLeaderClass())
			{
				memberClasses.push_back(pClass->GetName());
				//CryLogAlways("iter member class %s", pClass->GetName());
				continue;
			}

			if (!pSquad->GetLeader() && pClass->IsLeaderClass() && !pLeaderClass)
			{
				//CryLogAlways("iter leader class %s", className);
				pLeaderClass = pClass;
				continue;
			}
		}

		if (memberClasses.size() == 0)
			return;

		string randomClassName = TOS_STL::GetRandomFromSTL<std::vector<const char*>, const char*>(memberClasses);

		//Spawn Leader AI
		const string soldierSuffix = " FS";
		const string aiName = pConqueror->GetSpeciesName(species) + string(" ") + randomClassName + soldierSuffix;

		bool leaderSpawned = false;

		CSpeciesClass* pSelectedClass = pLeaderClass != nullptr ?
			pLeaderClass : pConqueror->GetClass(species, randomClassName);

		if (pSelectedClass)
		{
			auto pEntity = pConqueror->EmergencyCreateAIEntity(pSelectedClass, aiName.c_str());
			if (pEntity)
			{
				auto pChannel = pConqueror->CreateConquerorChannel(pEntity, *pSelectedClass);
				if (pChannel)
				{
					pChannel->SetForcedStrategicArea(this);

					auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
					if (pActor)
					{
						if (!pSquad->GetLeader())
							pSquad->SetLeader(pActor, true);

						//Only one squad can be forced spawned per species
						//m_forcedspawnedSquadIds.push_back(pSquad->GetId());
						m_forcedspawnedSquadId[m_Species] = pSquad->GetId();

						auto pSpawnPoint = GetBookedSpawnPoint(pActor);
						if (!pSpawnPoint)
							pSpawnPoint = BookFreeSpawnPoint(pActor, pChannel->GetClass()->IsAirClass());

						if (pSpawnPoint)
						{
							leaderSpawned = pChannel->SpawnActor(eRC_ForcedRespawn, pSpawnPoint);

							if (strcmp(pActor->GetEntity()->GetClass()->GetName(), "Scout") == 0)
							{
								SDetachedMemberData data;
								data.enableUpdate = true;
								data.routineType = eDRT_AirPathPatrol;
								data.targetId = pActor->GetEntityId();

								pSquad->MarkDetached(pActor->GetEntityId(), data);
							}


							//CryLogAlways("[C++][SpawnSquadForced][Squad %i Count %i][Set Leader %s]",
							//	pSquad->GetId(), pSquad->GetMembersCount(), pActor->GetEntity()->GetName());
						}
						else
						{
							CryLogAlways("$4[C++][ERROR][Failed Force AI Spawn][Cause: Unknown Leader SpawnPoint]");
						}
					
					}
				}
				else
				{
					CryLogAlways("$4[C++][ERROR][Failed Force AI Spawn][Cause: Unknown pChannel]");
				}
			}
			else
			{
				CryLogAlways("$4[C++][ERROR][Failed Force AI Spawn][Cause: Unknown pEntity]");
			}

			//need adding to xml?
			//m_xmlAICountInfo
		}
		else
		{
			CryLogAlways("$4[C++][ERROR][Failed Force AI Spawn][Cause: Unknown pSelectedClass]");
		}

		if (!leaderSpawned)
			return;

		//Create Members
		const int membersCount = GetMembersCount(); //pConqueror->m_xmlAICountInfo.GetUnitsCount(species);
		int createdCount = 0;

		for (; createdCount < membersCount;)
		{
			const string soldierSuffix = " FS";
			const string aiName = pConqueror->GetSpeciesName(species) + string(" ") + randomClassName + soldierSuffix;

			pSelectedClass = pConqueror->GetClass(species, randomClassName);
			if (!pSelectedClass)
			{
				CryLogAlways("%s[C++][Cannot Forced Spawn][Invalid Class]", STR_RED);
				break;
			}

			auto pEntity = pConqueror->EmergencyCreateAIEntity(pSelectedClass, aiName);
			if (!pEntity)
				continue;

			auto pChannel = pConqueror->CreateConquerorChannel(pEntity, *pSelectedClass);
			if (!pChannel)
				continue;

			pChannel->SetForcedStrategicArea(this);

			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (!pActor)
				continue;

			pSquad->AddMember(pActor);
			createdCount++;

			auto pSpawnPoint = GetBookedSpawnPoint(pActor);
			if (!pSpawnPoint)
				pSpawnPoint = BookFreeSpawnPoint(pActor, pChannel->GetClass()->IsAirClass());

			if (pSpawnPoint)
			{
				pChannel->SpawnActor(eRC_ForcedRespawn, pSpawnPoint);

				SDetachedMemberData data;
				data.enableUpdate = true;
				data.routineType = eDRT_AirPathPatrol;
				data.targetId = pActor->GetEntityId();

				pSquad->MarkDetached(pActor->GetEntityId(), data);
			}
			else
			{
				CryLogAlways("$4[C++][ERROR][Failed Force AI Spawn][Cause: Unknown Member SpawnPoint]");
			}
		}
	}
}

const int CStrategicArea::GetSpawnPointCount() const
{
	return m_spawnPoints.size();
}

bool CStrategicArea::CreateQueue(float respawnTimer)
{
	if (!m_pQueue->respawns.size())
	{
		m_pQueue->lastTimeCreated = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		m_pQueue->respawnTimer = respawnTimer;

		return true;
	}

	return false;
}

bool CStrategicArea::IsQueueCreated()
{
	return m_pQueue->respawnTimer > 0;
}

bool CStrategicArea::IsInQueue(EntityId entityId)
{
	for (auto& queueInfo : m_pQueue->respawns)
	{
		if (queueInfo.entityId == entityId)
			return true;
	}

	return false;
}

void CStrategicArea::AddToQueue(EntityId entityId, ERespawnEvent event)
{
	auto found = false;
	for (auto& queueInfo : m_pQueue->respawns)
	{
		if (queueInfo.entityId == entityId)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		SQueueRespawnInfo info;
		info.entityId = entityId;
		info.event = event;

		m_pQueue->respawns.push_back(info);

		const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
		g_pControlSystem->GetConquerorSystem()->OnActorAddedInQueue(pActor, this);
	}
}

bool CStrategicArea::EraseFromQueue(EntityId entityId)
{
	auto iter = std::find(m_pQueue->respawns.begin(), m_pQueue->respawns.end(), entityId);
	if (iter != m_pQueue->respawns.end())
	{
		m_pQueue->respawns.erase(iter);

		const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
		g_pControlSystem->GetConquerorSystem()->OnActorRemovedFromQueue(pActor);
	}

	if (m_pQueue->respawns.size() == 0)
		DeleteQueue();

	iter = std::find(m_pQueue->respawns.begin(), m_pQueue->respawns.end(), entityId);
	return iter == m_pQueue->respawns.end();
}

void CStrategicArea::EraseFirstFromQueue()
{
	const auto id = m_pQueue->respawns.front().entityId;
	const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);

	m_pQueue->respawns.pop_front();
	g_pControlSystem->GetConquerorSystem()->OnActorRemovedFromQueue(pActor);

	if (m_pQueue->respawns.size() == 0)
		DeleteQueue();
}

int CStrategicArea::GetQueueSize() const
{
	return m_pQueue->respawns.size();
}

float CStrategicArea::GetQueueTimer() const
{
	return m_pQueue->respawnTimer;
}

void CStrategicArea::GetFirstInQueue(SQueueRespawnInfo& info)
{
	info = m_pQueue->respawns.front();
}

SQueueRespawnInfo* CStrategicArea::GetLastInQueue()
{
	return &m_pQueue->respawns.back();
}

void CStrategicArea::DeleteQueue()
{
	m_pQueue->lastTimeRemoved = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	m_pQueue->respawnTimer = 0;
	m_pQueue->respawns.clear();
}

void CStrategicArea::UpdateQueue(float frametime)
{
	const auto pDude = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pDude)
		return;

	//Update booked spawnpoints here
	auto it = m_bookedSpawnPoints.begin();
	auto end = m_bookedSpawnPoints.end();

	for (; it!=end;it++)
	{
		auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(it->first));
		if (!pActor)
			continue;

		auto pChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(pActor->GetEntity());
		if (!pChannel)
			continue;

		if (pChannel->GetState() != eCCS_Alive)
			continue;

		if (pChannel->GetStateDuration(eCCS_Alive) > 5.0f)
		{
			m_bookedSpawnPoints.erase(it);
			//CryLogAlways("%s[C++][Area %s: unbook spawn point %i for actor %s]",
				//STR_YELLOW, GetEntity()->GetName(), it->second, pActor->GetEntity()->GetName());

			break;
		}
	}

	//Update queue respawn here
	if (m_pQueue->respawns.size() > 0)
	{
		if (IsInQueue(pDude->GetEntityId()))
		{
			const float respawnTime = g_pControlSystem->GetConquerorSystem()->GetRespawnTime();
			g_pControlSystem->GetConquerorSystem()->SetRespawnCycleRemainingTime(respawnTime, m_pQueue->respawnTimer);
		}

		if (m_pQueue->respawnTimer > 0)
			m_pQueue->respawnTimer -= frametime;

		if (m_pQueue->respawnTimer < 0)
			m_pQueue->respawnTimer = 0;

		if (m_pQueue->respawnTimer == 0)
		{
			SQueueRespawnInfo info;
			GetFirstInQueue(info);
			
			//auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pRespawnInfo->entityId));
			//if (!pActor)
			//{
			//	CryLogAlways("%s[C++][ERROR][Can't respawn %i in %s][Cause: actor not defined]",
			//		STR_RED, pRespawnInfo->entityId, GetEntity()->GetName());

			//	return;
			//}

			auto pChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(info.entityId);
			if (!pChannel)
			{
				CryLogAlways("%s[C++][ERROR][Can't respawn %i in %s][Cause: channel not defined]",
					STR_RED, info.entityId, GetEntity()->GetName());
				EraseFirstFromQueue();
				return;
			}

			const auto isHaveReinf = g_pControlSystem->GetConquerorSystem()->GetSpeciesReinforcements(pChannel->GetSpecies()) > 0;

			auto pActor = pChannel->GetActor();
			auto spawned = false;

			//Booked spawnpoint must be defined for actor before adding them into queue
			auto pSpawnpoint = GetBookedSpawnPoint(pActor);
			if (pSpawnpoint)
			{
				AABB bounds;
				pSpawnpoint->GetEntity()->GetWorldBounds(bounds);

				primitives::box obb;
				obb.Basis = Matrix33(pSpawnpoint->GetEntity()->GetWorldTM());
				obb.size = bounds.GetSize();
				obb.bOriented = true;

				const auto test = g_pGame->GetGameRules()->TestEntitySpawnPosition(pSpawnpoint->GetEntityId(), pSpawnpoint->GetEntity()->GetWorldPos(), obb);

				if (test && isHaveReinf)
				{
					pChannel->SpawnActor(info.event, pSpawnpoint);
					EraseFirstFromQueue();
					spawned = true;
					return;
				}
			}
			
			if (!pSpawnpoint || !spawned)
			{
				pSpawnpoint = g_pControlSystem->GetConquerorSystem()->GetPerfectFreeSpawnPoint(pActor);
				if (pSpawnpoint)
				{
					AABB bounds;
					pSpawnpoint->GetEntity()->GetWorldBounds(bounds);

					primitives::box obb;
					obb.Basis = Matrix33(pSpawnpoint->GetEntity()->GetWorldTM());
					obb.size = bounds.GetSize();
					obb.bOriented = true;

					const auto test = g_pGame->GetGameRules()->TestEntitySpawnPosition(pSpawnpoint->GetEntityId(), pSpawnpoint->GetEntity()->GetWorldPos(), obb);
					if (test && isHaveReinf)
					{
						pChannel->SetSelectedStrategicArea(pSpawnpoint->GetArea());
						pChannel->SpawnActor(info.event, pSpawnpoint);
						EraseFirstFromQueue();
						spawned = true;
						return;
					}
				}
			}

			if (!spawned)
			{
				CryLogAlways("%s[C++][ERROR][Can't respawn %s in %s][Cause: booked spawnpoint not defined or not pass the test]",
					STR_RED, pActor->GetEntity()->GetName(), GetEntity()->GetName());

				EraseFirstFromQueue();
			}

			//auto it = m_queue.respawns.begin();
			//auto end = m_queue.respawns.end();

			//for (; it != end; it++)
			//{
			//	auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(it->entityId));
			//	if (!pActor)
			//		continue;

			//	auto pChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(pActor->GetEntity());
			//	if (!pChannel)
			//		continue;

			//	//Booked spawnpoint must be defined for actor before adding them into queue
			//	auto pSpawnpoint = GetBookedSpawnPoint(pActor);
			//	if (pSpawnpoint)
			//	{
			//		pChannel->SpawnActor(it->event, pSpawnpoint);

			//	}
			//	else
			//	{
			//		CryLogAlways("%s[C++][ERROR][Can't respawn %s in %s][Cause: booked spawnpoint not defined]", 
			//			STR_RED, pActor->GetEntity()->GetName(), GetEntity()->GetName());
			//	}
			//}
		}
	}
}

bool CStrategicArea::IsBookedSpawnPoint(EntityId spawnpointId)
{
	if (spawnpointId == 0)
		return false;

	for (auto& bookedPair : m_bookedSpawnPoints)
	{
		if (bookedPair.second == spawnpointId)
			return true;
	}

	return false;
}

CSpawnPoint* CStrategicArea::GetBookedSpawnPoint(IActor* pActor)
{
	if (!pActor)
		return nullptr;

	for (auto& bookedPair : m_bookedSpawnPoints)
	{
		if (bookedPair.first == pActor->GetEntityId())
		{
			for (auto pSpawnpoint : m_spawnPoints)
			{
				if (pSpawnpoint->m_entityId == bookedPair.second)
					return pSpawnpoint;
			}
		}
	}

	return nullptr;
}

CSpawnPoint* CStrategicArea::BookFreeSpawnPoint(IActor* pActor, bool forAIR)
{
	if (!pActor)
		return nullptr;

	std::vector<EntityId> freeSpawnpoints;
	//std::vector<CSpawnPoint> spawnpoints = forAIR ? m_airSpawnPoints : m_spawnPoints;

	for (auto pSpawnpoint : m_spawnPoints)
	{
		if (IsBookedSpawnPoint(pSpawnpoint->m_entityId))
			continue;

		//if (pSpawnpoint->IsRecentlySpawned())
		//	continue;

		//if (pSpawnpoint->IsForAir())
		//{
		//	if (!forAIR)
		//		continue;
		//}

		if (forAIR)
		{
			if (!pSpawnpoint->IsForAir())
				continue;
		}

		freeSpawnpoints.push_back(pSpawnpoint->m_entityId);

		//m_bookedSpawnPoints[pActor->GetEntityId()] = pSpawnpoint->m_entityId;
		//return &spawnPoint;
	}

	if (freeSpawnpoints.size() > 0)
	{
		const EntityId entityIdType = 0;
		const auto id = TOS_STL::GetRandomFromSTL<std::vector<EntityId>, EntityId>(freeSpawnpoints);

		const auto pInstance = GetSpawnPointInstance(id);
		if (pInstance)
		{
			m_bookedSpawnPoints[pActor->GetEntityId()] = pInstance->m_entityId;
			return pInstance;
		}
	}

	return nullptr;
}

bool CStrategicArea::UnbookSpawnPoint(IActor* pActor)
{
	if (!pActor)
		return false;

	auto iter = m_bookedSpawnPoints.find(pActor->GetEntityId());
	if (iter != m_bookedSpawnPoints.end())
	{
		m_bookedSpawnPoints.erase(iter);
		return true;
	}

	return false;
}

bool CStrategicArea::UnbookSpawnPoint(EntityId spawnpointId)
{
	if (spawnpointId == 0)
		return false;

	IActor* pActor = nullptr;

	for (auto &bookedPair : m_bookedSpawnPoints)
	{
		if (bookedPair.second == spawnpointId)
		{
			pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(bookedPair.first);
			break;
		}
	}

	return UnbookSpawnPoint(pActor);
}

bool CStrategicArea::IsMyUnloadSpot(EntityId spotId)
{
	for (auto& spotPair : m_unloadSpots)
	{
		if (stl::find(spotPair.second, spotId))
		{
			return true;
		}
	}

	return false;
}

void CStrategicArea::GetSpawnedVehicles(std::vector<EntityId>& vehicles) const
{
	for (auto spawnerId : m_vehicleSpawners)
	{
		auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(spawnerId);
		if (!pSpawner)
			continue;

		auto pVehicle = pSpawner->GetSpawnedVehicle();
		if (!pVehicle)
			continue;

		vehicles.push_back(pVehicle->GetEntityId());
	}
}

void CStrategicArea::GetSpawnedVehicles(std::vector<EntityId>& vehicles, uint flags, int minSeatCount) const
{
	for (auto spawnerId : m_vehicleSpawners)
	{
		auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(spawnerId);
		if (!pSpawner)
			continue;

		auto pVehicle = pSpawner->GetSpawnedVehicle();
		if (!pVehicle)
			continue;

		if (pVehicle->GetEntity()->IsHidden())
			continue;

		if (pVehicle->IsDestroyed())
			continue;

		if (g_pControlSystem->GetConquerorSystem()->IsBookedVehicle(pVehicle))
			continue;

		if (pVehicle->GetStatus().passengerCount != 0)
			continue;

		if ((minSeatCount != -1 && minSeatCount > 0) && (pVehicle->GetSeatCount() < minSeatCount))
			continue;

		auto movType = pVehicle->GetMovement()->GetMovementType();

		if (flags & eVGF_Air)
		{
			if (movType != IVehicleMovement::eVMT_Air)
				continue;
		}
		else if (flags & eVGF_Land)
		{
			if (movType != IVehicleMovement::eVMT_Land)
				continue;
		}
		else if (flags & eVGF_Amphibious)
		{
			if (movType != IVehicleMovement::eVMT_Amphibious)
				continue;
		}
		else if (flags & eVGF_Sea)
		{
			if (movType != IVehicleMovement::eVMT_Sea)
				continue;
		}

		if (flags & eVGF_MustHaveGun)
		{
			const auto seatIdx = TOS_Vehicle::RequestGunnerSeatIndex(pVehicle);
			if (seatIdx == -1)
				continue;
		}

		vehicles.push_back(pVehicle->GetEntityId());
	}
}

void CStrategicArea::GetUnloadSpots(const char* spotType, std::vector<EntityId>& spots)
{
	auto iter = m_unloadSpots.find(spotType);
	if (iter == m_unloadSpots.end())
		return;

	for (auto spot : m_unloadSpots[spotType])
		spots.push_back(spot);
}

//SUnloadSpotInfo* CStrategicArea::GetVehicleUnloadSpot(EntityId entityId)
//{
//	auto it = m_unloadSpots.begin();
//	auto end = m_unloadSpots.end();
//
//	for (; it != end; it++)
//	{
//		for (auto& info : it->second)
//		{
//			if (info.entityId == entityId)
//				return &info;
//		}
//	}
//
//	return nullptr;
//}

//bool CStrategicArea::IsBookedUnloadBounds(const AABB& inputAABB)
//{
//	for (auto& spotPair : m_unloadSpots)
//	{
//		for (auto& spot : spotPair.second)
//		{
//			if (IsBookedUnloadBounds(inputAABB, &spot))
//				return true;
//		}		
//	}
//
//	return false;
//}
//
//bool CStrategicArea::IsBookedUnloadBounds(const AABB& inputBounds, SUnloadSpotInfo* pInfo)
//{
//	if (!pInfo)
//		return false;
//
//	for (auto& spotBookedPoses : pInfo->bookedPositions)
//	{
//		const auto& box = spotBookedPoses.second;
//		const auto storedBounds = GetAABBFromBOX(box);
//
//		if (Overlap::AABB_AABB(inputBounds, storedBounds))
//			return true;
//	}
//
//	return false;
//}

void CStrategicArea::OnAIJoinGame()
{
	if (CanSpawnSquadFromClasses())
	{
		std::vector<const char*> classes;
		GetUnlockedClasses(GetSpecies(), classes);
		SpawnSquadForced(GetSpecies(), classes);
	}
}

Vec3 CStrategicArea::GetWorldPos()
{
	auto worldPos = GetEntity()->GetWorldPos();

	auto pCenter = gEnv->pEntitySystem->GetEntity(m_centreAnchorId);
	if (pCenter)
		worldPos = pCenter->GetWorldPos();

	return worldPos;
}

void CStrategicArea::GetBuyZoneInfo(SBuyZoneInfo& info)
{
	info = m_buyZone;
}

bool CStrategicArea::IsBuyZoneActived(ESpeciesType species) const
{
	return m_Species == species && m_buyZone.enabled;
}

EntityId CStrategicArea::GetBookedUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return 0;

	auto iter = m_bookedUnloadSpots.find(pEntity->GetId());
	if (iter != m_bookedUnloadSpots.end())
		return iter->second;

	return 0;
}

EntityId CStrategicArea::BookFreeUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return 0;

	if (!g_pControlSystem->GetConquerorSystem()->CanBookUnloadSpot(pEntity))
		return 0;

	const string className = pEntity->GetClass()->GetName();
	const auto entId = pEntity->GetId();

	auto pVehicle = TOS_Vehicle::GetVehicle(pEntity);
	const auto isAir = (pVehicle && TOS_Vehicle::IsAir(pVehicle)) || (className == "Scout" || className == "Drone");

	std::vector<EntityId> allspots;
	std::vector<EntityId> freespots;

	GetUnloadSpots(isAir ? AIRSPOT : LANDSPOT, allspots);
	
	//Get all free spots
	if (allspots.size())
	{
		for (auto spotId : allspots)
		{
			if (IsBookedUnloadSpot(spotId))
				continue;

			freespots.push_back(spotId);
		}
	}

	//Get the nearest spot to the area's center
	if (freespots.size())
	{
		auto pNearestSpot = gEnv->pEntitySystem->GetEntity(freespots[0]);
		if (!pNearestSpot)
			return 0;

		float minDistToAreaCenter = (pNearestSpot->GetWorldPos() - GetEntity()->GetWorldPos()).GetLength();

		for (auto spotId : freespots)
		{
			auto pSpotEntity = gEnv->pEntitySystem->GetEntity(spotId);
			if (!pSpotEntity)
				continue;

			const float dist = (pSpotEntity->GetWorldPos() - GetEntity()->GetWorldPos()).GetLength();
			if (dist < minDistToAreaCenter)
			{
				pNearestSpot = pSpotEntity;
				minDistToAreaCenter = dist;
			}
		}

		if (pNearestSpot)
		{
			m_bookedUnloadSpots[entId] = pNearestSpot->GetId();

			if (g_pGameCVars->conq_debug_log_area)
			{
				CryLogAlways("%s[C++][Book Unload Spot: %s][Area: %s][Booker: %s]",
					STR_YELLOW, pNearestSpot->GetName(), GetEntity()->GetName(), pEntity->GetName());
			}

			return pNearestSpot->GetId();
		}
	}

	return 0;
}

bool CStrategicArea::IsBookedUnloadSpot(EntityId spotId) const
{
	for (auto& spotPair : m_bookedUnloadSpots)
	{
		if (spotPair.second == spotId)
			return true;
	}

	return false;
}

bool CStrategicArea::UnbookUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return false;

	const auto entId = pEntity->GetId();

	auto iter = m_bookedUnloadSpots.find(entId);
	if (iter != m_bookedUnloadSpots.end())
	{
		m_bookedUnloadSpots.erase(entId);

		if (g_pGameCVars->conq_debug_log_area)
		{
			CryLogAlways("%s[C++][UnBook Unload Spot][Area: %s][Entity: %s]",
				STR_YELLOW, GetEntity()->GetName(), pEntity->GetName());
		}

		return true;
	}

	if (g_pGameCVars->conq_debug_log_area)
	{
		CryLogAlways("%s[C++][UnBook Unload Spot][FAILED][Area: %s][Entity: %s][Cause: already unbooked]",
			STR_RED, GetEntity()->GetName(), pEntity->GetName());
	}
	return false;
}

//Vec3 CStrategicArea::BookFreeUnloadSpotPosition(IEntity* pEntity)
//{
//	if (!pEntity)
//	{
//		return Vec3(0);
//	}
//
//	//Get free spot here
//	auto getFailed = false;
//	const auto entPos = pEntity->GetWorldPos();
//
//	auto isAir = TOS_Vehicle::IsAir(TOS_Vehicle::GetVehicle(pEntity));
//	if (!isAir)
//		isAir = strcmp(pEntity->GetClass()->GetName(), "Scout") == 0;
//
//	//Check intersection with booked bounds and with entities
//	auto CheckGenericIntersection = [&](const IEntity* pEntity, const AABB& entityBounds)
//	{
//		auto& box = GetBoxFromAABB(pEntity, entityBounds);
//
//		//Checking in real space for empty bounds
//		auto test1 = g_pGame->GetGameRules()->TestEntitySpawnPosition2(box);
//
//		//Checking bounds for crossing with booked bounds
//		auto test2 = IsBookedUnloadBounds(entityBounds);
//
//		if (!test1)
//		{
//			CryLogAlways("%s[C++][Entity %s: box fail test1]",
//				STR_RED, pEntity->GetName());
//		}
//
//		if (test2)
//		{
//			CryLogAlways("%s[C++][Entity %s: bounds fail test2]",
//				STR_RED, pEntity->GetName());
//		}
//
//		return test1 && !test2;
//	};
//
//	AABB notUseEntityBounds;
//	pEntity->GetWorldBounds(notUseEntityBounds);
//	auto& entityBox = GetBoxFromAABB(pEntity, notUseEntityBounds);
//
//	CryLogAlways("[C++][Entity %s][Box: center (%1.f,%1.f,%1.f)]", 
//		pEntity->GetName(), entityBox.center.x, entityBox.center.y, entityBox.center.z);
//
//	std::vector<SUnloadSpotInfo> spots;
//	GetUnloadSpots(isAir ? "AIR" : "LAND", spots);
//
//	if (spots.size() > 0)
//	{
//		//auto pNearestSpotEnt = gEnv->pEntitySystem->GetEntity(spots.at(0).entityId);
//		//if (!pNearestSpotEnt)
//		//{
//		//	return Vec3(0);
//		//}
//
//		auto checkXFail = false;
//		auto checkYFail = false;
//		auto checkZFail = false;
//
//		const int XAxis = 0;
//		const int YAxis = 1;
//		const int ZAxis = 2;
//
//		//float minDist = (pNearestSpotEnt->GetWorldPos() - entPos).GetLength();
//
//		for (auto& spot : spots)
//		{
//			auto pSpotEnt = gEnv->pEntitySystem->GetEntity(spot.entityId);
//			if (!pSpotEnt)
//			{
//				continue;
//			}
//
//			entityBox.Basis.SetRotationVDir(pSpotEnt->GetWorldTM().GetColumn1().GetNormalizedSafe());
//
//			checkXFail = false;
//			checkYFail = false;
//			checkZFail = false;
//
//			const auto spotPos = pSpotEnt->GetWorldPos();
//			const auto radius = spot.radius;
//			const auto diameter = radius * 2;
//
//			for (int currentAxis = XAxis; currentAxis <= ZAxis; currentAxis++)
//			{
//				if (!isAir && currentAxis == ZAxis)
//					break;
//
//				auto startCheckingNewAxis = true;
//
//				// At start set entity box center to the spot's radius start position
//				if (startCheckingNewAxis)
//				{
//					auto spotOffsetPos = spotPos;
//
//					if (currentAxis == XAxis)
//					{
//						spotOffsetPos.x -= radius;
//					}
//					else if (currentAxis == YAxis)
//					{
//						spotOffsetPos.y -= radius;
//					}
//					else if (currentAxis == ZAxis)
//					{
//						spotOffsetPos.z -= radius;
//					}
//
//					entityBox.center = spotOffsetPos;
//					startCheckingNewAxis = false;
//				}
//
//				//Check here
//				for (int i = 0; i <= diameter; i++)
//				{
//					const bool check = CheckGenericIntersection(pEntity, GetAABBFromBOX(entityBox));
//					if (check)
//					{
//						CryLogAlways("[C++][Entity %s][Book Pos][Box: center (%1.f,%1.f,%1.f)]",
//							pEntity->GetName(), entityBox.center.x, entityBox.center.y, entityBox.center.z);
//
//						spot.bookedPositions[pEntity->GetId()] = entityBox;
//						return entityBox.center;
//					}
//					else
//					{
//						if (isAir)
//						{
//							if (currentAxis == XAxis)
//							{
//								checkXFail = true;
//							}
//							else if (currentAxis == YAxis)
//							{
//								checkYFail = true;
//							}
//							else if (currentAxis == ZAxis)
//							{
//								checkZFail = true;
//							}
//						}
//						else
//						{
//							if (currentAxis == XAxis)
//							{
//								checkXFail = true;
//							}
//							else if (currentAxis == YAxis)
//							{
//								checkYFail = true;
//							}
//						}
//
//						//Set Check Value here and check again in the next iteration
//						if (currentAxis == XAxis)
//						{
//							entityBox.center.x += i;
//						}
//						else if (currentAxis == YAxis)
//						{
//							entityBox.center.y += i;
//						}
//						else if (currentAxis == ZAxis)
//						{
//							entityBox.center.z += i;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	CryLogAlways("%s[C++][ERROR][Fail Find free point, return zero vector]", STR_RED);
//	return Vec3(0);
//}
//
//Vec3 CStrategicArea::GetBookedUnloadSpotPosition(IEntity* pEntity)
//{
//	if (!pEntity)
//	{
//		return Vec3(0);
//	}
//
//	//Забронированные точки не сохраняются
//
//	for (auto& pairs : m_unloadSpots)
//	{
//		auto& spots = pairs.second;
//		for (auto& spot : spots)
//		{
//			auto iter = spot.bookedPositions.find(pEntity->GetId());
//			if (iter == spot.bookedPositions.end())
//				continue;
//			else
//				return iter->second.center;
//		}
//	}
//
//	return Vec3(0);
//}
//
//void CStrategicArea::UnbookUnloadSpotPosition(IEntity* pEntity)
//{
//	if (!pEntity)
//		return;
//
//	auto isAir = TOS_Vehicle::IsAir(TOS_Vehicle::GetVehicle(pEntity));
//	if (!isAir)
//		isAir = strcmp(pEntity->GetClass()->GetName(), "Scout") == 0;
//
//	std::vector<SUnloadSpotInfo> spots;
//	GetUnloadSpots(isAir ? "AIR" : "LAND", spots);
//
//	for (auto& spot : spots)
//	{
//		auto iter = spot.bookedPositions.find(pEntity->GetId());
//		if (iter == spot.bookedPositions.end())
//			continue;
//		else
//			spot.bookedPositions.erase(iter);
//	}
//}

void CStrategicArea::AddVehicleUnloadSpot(const char* spotType, EntityId spotId)
{
	auto iter = m_unloadSpots.find(spotType);
	if (iter == m_unloadSpots.end())
		return;

	stl::push_back_unique(m_unloadSpots[spotType], spotId);

	//for (auto id : m_unloadSpots[spotType])
	//{
	//	if (id == spotId)
	//		return;
	//}

	////SUnloadSpotInfo info;
	////info.entityId = spotId;
	////info.radius = radius;

	//m_unloadSpots[spotType].push_back(spotId);
}

void CStrategicArea::ResetVehicleUnloadSpots()
{
	m_unloadSpots.clear();
	m_unloadSpots[AIRSPOT].clear();
	m_unloadSpots[LANDSPOT].clear();
	m_bookedUnloadSpots.clear();
}

std::deque<SQueueRespawnInfo>::iterator CStrategicArea::GetQueueBeginIt()
{
	return m_pQueue->respawns.begin();
}

std::deque<SQueueRespawnInfo>::iterator CStrategicArea::GetQueueEndIt()
{
	return m_pQueue->respawns.end();
}

bool CStrategicArea::IsSpeciesExclusive(ESpeciesType species)
{
	int otherAgentCount = 0;

	for (int i = eST_FirstPlayableSpecies; i < eST_LastPlayableSpecies; i++)
	{
		const auto itSpecies = ESpeciesType(i);

		if (itSpecies != species)
			otherAgentCount += GetAgentCount(itSpecies);
	}

	if (otherAgentCount == 0)
		return true;

	return false;
}

const float CStrategicArea::GetCaptureProgress(ESpeciesType species) const
{
	return m_CaptureProgress.at(species);
}

void CStrategicArea::GetMaxCaptureProgress(float& value, ESpeciesType& species)
{
	float maxCaptureProgress = 0;
	ESpeciesType maxCaptureSpecies = eST_FirstPlayableSpecies;

	for (int i = ESpeciesType::eST_FirstPlayableSpecies; i < ESpeciesType::eST_LastPlayableSpecies; i++)
	{
		const float currentCaptureProgress = m_CaptureProgress[ESpeciesType(i)];

		if (maxCaptureProgress < currentCaptureProgress)
		{
			maxCaptureSpecies = ESpeciesType(i);
			maxCaptureProgress = currentCaptureProgress;

			//CryLogAlways("New Max Progress is %1.f", maxCaptureProgress);
		}

		value = maxCaptureProgress;
		species = maxCaptureSpecies;
	}
}

ECaptureState CStrategicArea::GetCaptureState()
{
	return m_CaptureState;
}

bool CStrategicArea::IsCapturable()
{
	//const auto isConquest = g_pControlSystem->GetConquerorSystem()->IsGamemode();
	if (GetEntity() /*&& !isConquest*/)
	{
		auto pTable = GetEntity()->GetScriptTable();
		if (pTable)
		{
			SmartScriptTable props;
			if (pTable->GetValue("Properties", props))
				props->GetValue("bCapturable", m_bIsCapturable);
		}
	}

	return m_bIsCapturable;
}

bool CStrategicArea::IsEnabled()
{
	//const auto isConquest = g_pControlSystem->GetConquerorSystem()->IsGamemode();

	auto pTable = GetEntity()->GetScriptTable();
	if (pTable /*&& !isConquest*/)
	{
		SmartScriptTable props;
		if (pTable->GetValue("Properties", props))
			props->GetValue("bEnable", m_bIsEnabled);
	}

	return m_bIsEnabled;
}

bool CStrategicArea::CanUnlockClasses(bool forPlayer)
{
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorSquad", "bCanUnlockClassesForPlayer", m_bCanUnlockClassesForPlayer);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorSquad", "bCanUnlockClassesForAI", m_bCanUnlockClassesForAI);
	
	return forPlayer ? m_bCanUnlockClassesForPlayer : m_bCanUnlockClassesForAI;
}

int CStrategicArea::GetMembersCount()
{
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorSquad", "membersCount", m_SpawnedMembersCount);
	return m_SpawnedMembersCount;
}

bool CStrategicArea::CanSpawnSquadFromClasses()
{
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorSquad", "bCanSpawnSquadFromClasses", m_bSpawnSquadFromClasses);
	return m_bSpawnSquadFromClasses;
}

bool CStrategicArea::IsUnlockedClass(ESpeciesType species, const char* name) const
{
	auto iter = m_unlockedClasses.find(species);
	if (iter == m_unlockedClasses.end())
		return false;

	for (auto className : iter->second)
	{
		if (strcmp(className, name) == 0 && (m_Species == species))
			return true;
	}

	return false;
}

IEntity* CStrategicArea::GetAIAnchor()
{
	auto pAnchor = GET_ENTITY(m_centreAnchorId);
	if (!pAnchor)
		pAnchor = this->GetEntity();

	return pAnchor;
}

void CStrategicArea::SetCaptureProgress(ESpeciesType species, float progress)
{
	m_CaptureProgress.at(species) = progress;
}

void CStrategicArea::Release()
{
	g_pControlSystem->GetConquerorSystem()->RemoveStrategicArea(this);
	delete this;
}

void CStrategicArea::FullSerialize(TSerialize ser)
{

}

bool CStrategicArea::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return false;
}

void CStrategicArea::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	const float frametime = ctx.fFrameTime;
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	//debug values
	bool bDebugPointIsWithin = false;
	float debugCaptureProgress, debugMaxCount = 0;
	ESpeciesType debugMaxAgentSpecies = ESpeciesType::eST_NEUTRAL;
	//~debug values

	if (gEnv->pSystem->IsEditorMode())
		return;

	IEntity* pShape = gEnv->pEntitySystem->GetEntity(m_ShapeEntityId);
	if (!pShape)
	{
		CryLogAlways("%s[C++][Area Update Fail][Victim: %s][Cause: pShape is null]",
			STR_RED, GetEntity()->GetName());
		return;
	}

	IEntityAreaProxy* pAreaProxy = (IEntityAreaProxy*)pShape->GetProxy(ENTITY_PROXY_AREA);
	if (!pAreaProxy)
	{
		CryLogAlways("%s[C++][Area Update Fail][Victim: %s][Cause: pAreaProxy is null]",
			STR_RED, GetEntity()->GetName());
		return;
	}

	UpdateQueue(frametime);

	Vec3 min, max, worldPos(pShape->GetWorldPos());
	min.Set(0.f, 0.f, 0.f);
	max.Set(0.f, 0.f, 0.f);
	EEntityAreaType areaType = pAreaProxy->GetAreaType();

	// Construct bounding space around area
	switch (areaType)
	{
	case ENTITY_AREA_TYPE_BOX:
	{
		pAreaProxy->GetBox(min, max);
		min += worldPos;
		max += worldPos;
	}
	break;
	case ENTITY_AREA_TYPE_SPHERE:
	{
		Vec3 center;
		float radius = 0.f;
		pAreaProxy->GetSphere(center, radius);

		min.Set(center.x - radius, center.y - radius, center.z - radius);
		max.Set(center.x + radius, center.y + radius, center.z + radius);
	}
	break;
	case ENTITY_AREA_TYPE_SHAPE:
	{
		const Vec3* points = pAreaProxy->GetPoints();
		const int count = pAreaProxy->GetPointsCount();
		if (count > 0)
		{
			Vec3 p = worldPos + points[0];
			min = p;
			max = p;
			for (int i = 1; i < count; ++i)
			{
				p = worldPos + points[i];
				if (p.x < min.x) min.x = p.x;
				if (p.y < min.y) min.y = p.y;
				if (p.z < min.z) min.z = p.z;
				if (p.x > max.x) max.x = p.x;
				if (p.y > max.y) max.y = p.y;
				if (p.z > max.z) max.z = p.z;
			}
		}
	}
	break;
	}

	EntityId physicalEntId = 0;
	EEntityType entityType = EEntityType::UNKNOWN;

	IPhysicalWorld* pWorld = gEnv->pPhysicalWorld;
	IPhysicalEntity** ppList = nullptr;
	int	numEnts = pWorld->GetEntitiesInBox(min, max, ppList, ent_living);
	for (int i = 0; i < numEnts; ++i)
	{
		physicalEntId = pWorld->GetPhysicalEntityId(ppList[i]);
		entityType = GetEntityType(physicalEntId);
		const EEntityType actorType = EEntityType::ACTOR;
		const EEntityType vehicleType = EEntityType::VEHICLE;

		if (IsValidEntityType(actorType, entityType))
		{
			IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(physicalEntId);

			const auto pVehicle = pActor->GetLinkedVehicle();
			bool isVehicle = pVehicle && !pVehicle->IsDestroyed();

			if (pActor->GetHealth() > 0.1f && pAreaProxy->IsPointWithin(pActor->GetEntity()->GetWorldPos(), pAreaProxy->GetHeight() == 0))
			{
				AddToArea(pActor->GetEntityId(), isVehicle);
			}
		}
	}

	for (auto id : m_ActorsInside)
	{
		auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id));
		if (pActor)
		{
			const auto entityPos = pActor->GetEntity()->GetWorldPos();

			bool bIsInsideArea = false;
			bDebugPointIsWithin = bIsInsideArea = pAreaProxy->IsPointWithin(entityPos, pAreaProxy->GetHeight() == 0);

			if (!bIsInsideArea || pActor->GetHealth() < 0.1f /*|| (pVehicle && pVehicle->IsDestroyed())*/)
				RemoveFromArea(pActor);
		}
	}

	if (m_bIsCapturable && m_bIsEnabled)
	{
		int allAgentsCount = 0;

		for (int i = eST_FirstPlayableSpecies; i < eST_LastPlayableSpecies; i++)
		{
			const auto speciesType = ESpeciesType(i);

			const int agentCount = GetAgentCount(speciesType);
			const int captureProgress = GetCaptureProgress(speciesType);

			allAgentsCount += agentCount;

			if (!IsAreaOwner(speciesType))
			{
				if (agentCount == 0 && captureProgress > 0.0f)
					SetCaptureProgress(speciesType, 0.0f);
			}
		}


	if (currentTime - m_lastUpdateTime > g_pGameCVars->conq_area_update_delay)
	{
		m_lastUpdateTime = currentTime;

		static bool bIsAnyAgentInside = 0; bIsAnyAgentInside = allAgentsCount > 0;

		switch (m_CaptureState)
		{
		case ECaptureState::NOTCAPTURED:
		{
			//if (g_pGameCVars->conq_debug_log_area)
			//	CryLogAlways("$6[C++][Strategic Area %s now is NOTCAPTURED", GetEntity()->GetName());

			if (bIsAnyAgentInside)
			{
				int maxCount = 0;
				ESpeciesType maxAgentSpecies(eST_NEUTRAL);

				GetMaxAgentInfo(maxCount, maxAgentSpecies);

				debugMaxCount = maxCount;
				debugMaxAgentSpecies = maxAgentSpecies;

				if (maxCount >= m_CaptureRequirement)
				{
					if (m_CapturingSpecies != maxAgentSpecies)
						m_CapturingSpecies = maxAgentSpecies;

					if (m_CaptureState != ECaptureState::CAPTURING)
						m_CaptureState = ECaptureState::CAPTURING;
				}
			}
			break;
		}
		case ECaptureState::CAPTURED:
		{
			//if (g_pGameCVars->conq_debug_log_area)
			//	CryLogAlways("$6[C++][Strategic Area %s now is CAPTURED", GetEntity()->GetName());

			if (bIsAnyAgentInside)
			{
				const int attackersCount = GetEnemiesCount(m_Species);
				if (attackersCount > 0 && attackersCount >= m_CaptureRequirement)
				{
					int capturersCount = 0;
					ESpeciesType capturersSpecies = eST_NEUTRAL;
					GetMaxAgentInfo(capturersCount, capturersSpecies);

					debugMaxCount = capturersCount;
					debugMaxAgentSpecies = capturersSpecies;

					m_CapturingSpecies = capturersSpecies;

					if (!IsSpeciesExclusive(capturersSpecies))
					{
						if (GetContested(capturersSpecies))
							OnContested(capturersSpecies);
					}

					const int defendersCount = GetAgentCount(m_Species);
					if (defendersCount == 0)
						m_CaptureState = ECaptureState::UNCAPTURING;
				}
			}

			float currentProgress = GetCaptureProgress(m_Species);

			if (currentProgress < m_CaptureTime)
				currentProgress += g_pGameCVars->conq_area_update_delay * m_CaptureStep * 0.25f;
				//currentProgress += frametime * m_CaptureStep * 0.25f;

			SetCaptureProgress(m_Species, currentProgress);
			break;
		}
		case ECaptureState::CONTESTED:
		{
			//if (g_pGameCVars->conq_debug_log_area)
			//	CryLogAlways("$6[C++][Strategic Area %s now is CONTESTED", GetEntity()->GetName());

			int agentCount = 0;
			ESpeciesType agentSpecies = eST_NEUTRAL;
			GetMaxAgentInfo(agentCount, agentSpecies);

			debugMaxCount = agentCount;
			debugMaxAgentSpecies = agentSpecies;

			//Test

			if (IsSpeciesExclusive(agentSpecies))
			{
				if (!IsAreaOwner(eST_NEUTRAL))
				{
					if (IsAreaOwner(agentSpecies))
					{
						//if (g_pGameCVars->conq_debug_log_area)
						//	CryLogAlways("[CapturableArea %s][CONTESTED]->[CAPTURED]", GetEntity()->GetName());

						m_CaptureState = ECaptureState::CAPTURED;
						m_CapturingSpecies = eST_NEUTRAL;
					}

					if (!IsAreaOwner(agentSpecies))
					{
						//if (g_pGameCVars->conq_debug_log_area)
						//	CryLogAlways("[CapturableArea %s][CONTESTED]->[UNCAPTURING]", GetEntity()->GetName());
						m_CaptureState = ECaptureState::UNCAPTURING;
					}
				}
				else
				{
					//if (g_pGameCVars->conq_debug_log_area)
					//	CryLogAlways("[CapturableArea %s][CONTESTED]->[NOTCAPTURED]", GetEntity()->GetName());

					//m_CaptureState = ECaptureState::NOTCAPTURED;
					m_CaptureState = ECaptureState::CAPTURING;
				}
			}

			break;
		}
		case ECaptureState::CAPTURING:
		{
			//if (g_pGameCVars->conq_debug_log_area)
			//	CryLogAlways("$6[C++][Strategic Area %s now is CAPTURING", GetEntity()->GetName());

			if (!IsSpeciesExclusive(m_CapturingSpecies))
			{
				if (GetContested(m_CapturingSpecies))
					OnContested(m_CapturingSpecies);
			}

			if (GetAgentCount(m_CapturingSpecies) != 0 &&
				GetAgentCount(m_CapturingSpecies) >= m_CaptureRequirement)
			{
				//if (m_Species != m_CapturingSpecies)
				if (!IsAreaOwner(m_CapturingSpecies))
				{
					if (IsAreaOwner(ESpeciesType::eST_NEUTRAL))
					{
						int capturingAgentCount = GetAgentCount(m_CapturingSpecies);
						capturingAgentCount = ::min(capturingAgentCount, 3);

						float& capturingProgress = m_CaptureProgress.at(m_CapturingSpecies);

						if (capturingProgress != m_CaptureTime)
							capturingProgress += g_pGameCVars->conq_area_update_delay * m_CaptureStep * capturingAgentCount;
							//capturingProgress += frametime * m_CaptureStep * capturingAgentCount;

						if (capturingProgress >= m_CaptureTime)
						{
							capturingProgress = m_CaptureTime;
							SetCaptured(m_CapturingSpecies);
							//CryLogAlways("CAPTURING->CAPTURED");
						}
					}

					if (!IsAreaOwner(ESpeciesType::eST_NEUTRAL))
					{
						if (m_CaptureState != ECaptureState::UNCAPTURING)
						{
							m_CaptureState = ECaptureState::UNCAPTURING;
							//CryLogAlways("CAPTURING->UNCAPTURING");
						}
					}

				}
			}
			else
				m_CaptureState = ECaptureState::NOTCAPTURED;

			break;
		}
		case ECaptureState::UNCAPTURING:
		{
			//if (g_pGameCVars->conq_debug_log_area)
			//	CryLogAlways("$6[C++][Strategic Area %s now is UNCAPTURING", GetEntity()->GetName());

			//CryLogAlways("[ECaptureState][UNCAPTURING]");

			if (!IsSpeciesExclusive(m_CapturingSpecies))
			{
				if (GetContested(m_CapturingSpecies))
					OnContested(m_CapturingSpecies);
			}

			if (GetAgentCount(m_CapturingSpecies) != 0 &&
				GetAgentCount(m_CapturingSpecies) >= m_CaptureRequirement)
			{
				if (!IsAreaOwner(m_CapturingSpecies))
				{
					if (!IsAreaOwner(ESpeciesType::eST_NEUTRAL))
					{
						int capturingAgentCount = GetAgentCount(m_CapturingSpecies);
						capturingAgentCount = ::min(capturingAgentCount, 3);

						float& capturingProgress = m_CaptureProgress.at(m_Species);

						if (capturingProgress > 0.0f)
							capturingProgress -= g_pGameCVars->conq_area_update_delay * m_CaptureStep * capturingAgentCount;
							//capturingProgress -= frametime * m_CaptureStep * capturingAgentCount;

						if (capturingProgress <= 0.0f)
						{
							CreateScriptEvent("OnUncaptured", (float)m_CapturingSpecies);

							capturingProgress = 0.0f;
							SetNeutral(false);
						}

						debugCaptureProgress = m_CaptureProgress.at(m_CapturingSpecies);
					}
				}
			}
			else
				m_CaptureState = ECaptureState::CAPTURED;

			break;
		}

		default:
			break;
		}
		}
	}

	for (auto pSpawnpoint : m_spawnPoints)
		pSpawnpoint->Update(frametime);

	//for (auto& point : m_airSpawnPoints)
	//	point.Update(frametime);


	//Debug
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//const string name = GetEntity()->GetName();
	//if (name == "test" || name == "Test" || name == "TEST")
	//{
	//	for (auto pSpawnpoint : m_spawnPoints)
	//	{
	//		//Debug

	//		static float color[] = { 1,1,1,1 };
	//		const auto size = 1.15f;
	//		const auto scale = 20;
	//		const auto xoffset = 700;
	//		const auto yoffset = 50;

	//		gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + pSpawnpoint->m_index * scale, size, color, false,
	//			"SpawnPoint %i, EntityId %i, RecentlySpawned %i",
	//			pSpawnpoint->m_index, pSpawnpoint->m_entityId, pSpawnpoint->m_recentlySpawned);
	//	}
	//}

	//UpdateVehicleScheduler(frametime);

	//bool bDebugDraw = false;

	//SmartScriptTable properties;
	//IScriptTable* pTable = GetEntity()->GetScriptTable();
	//if (pTable->GetValue("Properties", properties))
	//{
	//	properties->GetValue("bDrawDebugLog", bDebugDraw);
	//}

	//if (bDebugDraw)
	//{
	//	string entityName;
	//	IEntity* pEntity = nullptr;//gEnv->pEntitySystem->GetEntity(physicalEntId);
	//	if (pEntity)
	//	{
	//		entityName = pEntity->GetName();
	//	}


	//	static float color[] = { 1,1,1,1 };
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 20.0f, 1.15f, color, false, "[CapturableArea][numEnts]: %i", numEnts);
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 40.0f, 1.15f, color, false, "[CapturableArea][physicalEntId %i", physicalEntId);
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 60.0f, 1.15f, color, false, "[CapturableArea][bIsInsideArea]: %i", int(bDebugPointIsWithin));
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 80.0f, 1.15f, color, false, "[CapturableArea][entityName]: %s" , entityName);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 100.0f, 1.15f, color, false, "[CapturableArea][m_Species]: %i", (int)m_Species);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 120.0f, 1.15f, color, false, "[CapturableArea][Actors Count]: %i", GetActorCount());
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 140.0f, 1.15f, color, false, "[CapturableArea][USA Count]: %i", GetAgentCount(eST_USA));
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 160.0f, 1.15f, color, false, "[CapturableArea][NK Count]: %i", GetAgentCount(eST_NK));
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 180.0f, 1.15f, color, false, "[CapturableArea][ALIENS Count]: %i", GetAgentCount(eST_Aliens));
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 200.0f, 1.15f, color, false, "[CapturableArea][CELL Count]: %i", GetAgentCount(eST_CELL));
	//	//gEnv->pRenderer->Draw2dLabel(20.0f, 220.0f, 1.15f, color, false, "[CapturableArea][m_SpeciesMembers.size]: %i", m_SpeciesMembers.size());
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 240.0f, 1.15f, color, false, 
	//		"[CapturableArea][m_CaptureState]: %i (0 - Not Captured, 1 - Captured, 2 - Contested, 4 - Capturing, 8 - Uncapturing)", m_CaptureState);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 260.0f, 1.15f, color, false, "[CapturableArea][m_bIsCapturable]: %i", (int)m_bIsCapturable);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 280.0f, 1.15f, color, false, "[CapturableArea][debugCaptureProgress]: %1.f", debugCaptureProgress);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 300.0f, 1.15f, color, false, "[CapturableArea][m_CaptureTime]: %1.f", m_CaptureTime);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 320.0f, 1.15f, color, false, "[CapturableArea][m_CapturingSpecies]: %i", (int)m_CapturingSpecies);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 340.0f, 1.15f, color, false, "[CapturableArea][debugMaxCount]: %i", (int)debugMaxCount);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 360.0f, 1.15f, color, false, "[CapturableArea][debugMaxAgentSpecies]: %i", (int)debugMaxAgentSpecies);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 380.0f, 1.15f, color, false, "[CapturableArea][m_FinalCaptureProgress]: %1.f", m_FinalCaptureProgress);
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 400.0f, 1.15f, color, false, "[CapturableArea][USA Progress]: %1.f", GetCaptureProgress(eST_USA));
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 420.0f, 1.15f, color, false, "[CapturableArea][NK Progress]: %1.f", GetCaptureProgress(eST_NK));
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 440.0f, 1.15f, color, false, "[CapturableArea][ALIENS Progress]: %1.f", GetCaptureProgress(eST_Aliens));
	//	gEnv->pRenderer->Draw2dLabel(20.0f, 460.0f, 1.15f, color, false, "[CapturableArea][CELL Progress]: %1.f", GetCaptureProgress(eST_CELL));
	//}
}

void CStrategicArea::HandleEvent(const SGameObjectEvent& event)
{
	
}

void CStrategicArea::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_RESET:
	{
		Reset();
		break;
	}
	case ENTITY_EVENT_START_GAME:
	{
		OnGameStart();
		break;
	}
	}
}

void CStrategicArea::OnGameStart()
{
	if (g_pGameCVars->conq_debug_log_area)
		CryLogAlways("[C++][Strategic Area %s][On Game Start][Add Entities from Links]", GetEntity()->GetName());

	for (IEntityLink* pLink = GetEntity()->GetEntityLinks(); pLink; pLink = pLink->next)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(pLink->entityId);
		if (!pEntity)
			continue;

		const char* type = 0;
		TOS_Script::GetEntityScriptValue(pEntity, "type", type);

		const string stype = type;

		if (stype == "ConquerorFlag")
		{
			m_flagEntityId = pLink->entityId;

			auto pFlagEntity = gEnv->pEntitySystem->GetEntity(m_flagEntityId);
			if (pFlagEntity)
				Script::CallMethod(pFlagEntity->GetScriptTable(), "SetSpecies", m_Species);
		}

		if (stype == "ConquerorSpawnPoint")
			AddSpawnPoint(pLink->entityId);

		if (stype == "AreaVehicleSpawnPoint")
		{
			auto pSpawner = g_pControlSystem->GetConquerorSystem()->GetVehicleSpawner(pLink->entityId);
			AddVehicleSpawner(pSpawner);		
		}
		
		const auto isShape = 
			strcmp(pEntity->GetClass()->GetName(), "AreaShape") == 0 || 
			strcmp(pEntity->GetClass()->GetName(), "AreaBox") == 0 ||
			strcmp(pEntity->GetClass()->GetName(), "AreaSphere") == 0;

		if (isShape)
		{
			//if (m_ShapeEntityId == 0)
				m_ShapeEntityId = pLink->entityId;
		}

		//if (strcmp(pLink->name, "CapturableArea") == 0)
		//{
		//	//CryLogAlways("SHAPE TYPE %s", pEntity->GetClass()->GetName());

		//	if (m_ShapeEntityId == 0)
		//		m_ShapeEntityId = pLink->entityId;			
		//}
		//else if (strcmp(pLink->name, "StrategicArea") == 0)
		//{
		//	//CryLogAlways("SHAPE TYPE %s", pEntity->GetClass()->GetName());

		//	if (m_ShapeEntityId == 0)
		//		m_ShapeEntityId = pLink->entityId;
		//}
		//else if (strcmp(pLink->name, "Area") == 0)
		//{
		//	//CryLogAlways("SHAPE TYPE %s", pEntity->GetClass()->GetName());

		//	if (m_ShapeEntityId == 0)
		//		m_ShapeEntityId = pLink->entityId;
		//}
		//else if (strcmp(pLink->name, "Shape") == 0)
		//{
		//	//CryLogAlways("SHAPE TYPE %s", pEntity->GetClass()->GetName());

		//	if (m_ShapeEntityId == 0)
		//		m_ShapeEntityId = pLink->entityId;
		//}

		//if (strcmp(pLink->name, "Center") == 0)
		//{
		//	if (m_centreAnchorId == 0)
		//		m_centreAnchorId = pLink->entityId;
		//}
		//else if (strcmp(pLink->name, "AIAnchor") == 0)
		//{
		//	//for backward compatibility

		//	if (m_centreAnchorId == 0)
		//		m_centreAnchorId = pLink->entityId;
		//}

		if (stype == "AIAnchor")
		{
			bool isEnabled = 0;
			//float radius = 0;
			const char* anchorType = 0;
			TOS_Script::GetEntityProperty(pEntity, "aianchor_AnchorType", anchorType);
			//TOS_Script::GetEntityProperty(pEntity, "radius", radius);
			TOS_Script::GetEntityProperty(pEntity, "bEnabled", isEnabled);

			if (isEnabled)
			{
				if (strcmp(anchorType, "TOS_SA_CENTRE_SPOT") == 0)
				{
					m_centreAnchorId = pEntity->GetId();
				}
				else if (strcmp(anchorType, "TOS_SA_LANDVEHICLE_UNLOAD_SPOT") == 0)
				{
					AddVehicleUnloadSpot(LANDSPOT, pEntity->GetId());
				}
				else if (strcmp(anchorType, "TOS_SA_AIRVEHICLE_UNLOAD_SPOT") == 0)
				{
					AddVehicleUnloadSpot(AIRSPOT, pEntity->GetId());
				}
			}
		}
	}

	if (m_LuaSpecies != eST_NEUTRAL)
		SetCaptured(m_LuaSpecies);
}

void CStrategicArea::SetAuthority(bool auth)
{

}

void CStrategicArea::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_ActorsInside);
	s->AddContainer(m_spawnPoints);
	//s->AddContainer(m_airSpawnPoints);
	s->AddContainer(m_SpeciesMembers);
	s->AddContainer(m_CaptureProgress);

	for (auto pSpawnpoint : m_spawnPoints)
		pSpawnpoint->GetMemoryStatistics(s);

	//for (auto iter = m_airSpawnPoints.begin(); iter != m_airSpawnPoints.end(); ++iter)
	//{
	//	iter->GetMemoryStatistics(s);
	//}
}

bool CStrategicArea::Reset()
{
	ResetVehicleUnloadSpots();
	if (m_pQueue)
		m_pQueue->reset();

	m_buyZone.reset();
	m_spawnPoints.clear();
	//m_airSpawnPoints.clear();
	m_bookedSpawnPoints.clear();
	m_forcedspawnedSquadId.clear();
	m_unlockedClasses.clear();
	m_ActorsInside.clear();
	m_SpeciesMembers.clear();
	m_CaptureProgress.clear();
	m_vehicleSpawners.clear();
	m_AreaFlags.clear();

	for (int i = eST_NEUTRAL; i < eST_LAST; i++)
	{
		m_SpeciesMembers.insert(std::make_pair(ESpeciesType(i), 0));
		m_CaptureProgress.insert(std::make_pair(ESpeciesType(i), 0));
	}

	m_ShapeEntityId = 0;
	m_CaptureState = ECaptureState::NOTCAPTURED;
	m_FinalCaptureProgress = 0.0f;
	m_lastUpdateTime = 0.0f;

	GetLuaValues();	

	SetNeutral(true);

	return true;
}

void CStrategicArea::GetLuaValues()
{
	bool bVehicles = false;
	bool bWeapons = false;
	bool bEquipment = false;
	bool bPrototypes = false;
	bool bAmmo = false;

	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bEnabled", m_buyZone.enabled);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bVehicles", bVehicles);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bWeapons", bWeapons);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bEquipment", bEquipment);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bPrototypes", bPrototypes);
	TOS_Script::GetEntityProperty(GetEntity(), "ConquerorBuyOptions", "bAmmo", bAmmo);

	if (bVehicles)
		m_buyZone.flags |= eBZO_Vehicles;
	if (bWeapons)
		m_buyZone.flags |= eBZO_Weapons;
	if (bEquipment)
		m_buyZone.flags |= eBZO_Equipment;
	if (bPrototypes)
		m_buyZone.flags |= eBZO_Prototypes;
	if (bAmmo)
		m_buyZone.flags |= eBZO_Ammo;

	SmartScriptTable props;
	IScriptTable* pScriptTable = GetEntity()->GetScriptTable();
	if (pScriptTable && pScriptTable->GetValue("Properties", props))
	{
		int outSpecies = -1;

		m_DefinedMultiplayerSide = EMultiplayerSide::SPECIES;

		props->GetValue("species", outSpecies);
		m_LuaSpecies = ESpeciesType(outSpecies);

		props->GetValue("captureTime", m_CaptureTime);
		props->GetValue("captureRequirement", m_CaptureRequirement);
		props->GetValue("bCapturable", m_bIsCapturable);
		props->GetValue("bEnable", m_bIsEnabled);

		props->GetValue("bCanUnlockClassesForPlayer", m_bCanUnlockClassesForPlayer);
		props->GetValue("bCanUnlockClassesForAI", m_bCanUnlockClassesForAI);
		props->GetValue("bSpawnSquadFromClasses", m_bSpawnSquadFromClasses);

		const char* flags;
		props->GetValue("soclasses_areaFlags", flags);

		if (strstr(flags, "Centre"))
			m_AreaFlags.push_back(EAreaFlag::Centre);
		if (strstr(flags, "AirSpawner"))
			m_AreaFlags.push_back(EAreaFlag::AirSpawner);
		if (strstr(flags, "LandSpawner"))
			m_AreaFlags.push_back(EAreaFlag::LandSpawner);
		if (strstr(flags, "SeaSpawner"))
			m_AreaFlags.push_back(EAreaFlag::SeaSpawner);
		if (strstr(flags, "SoldierSpawner"))
			m_AreaFlags.push_back(EAreaFlag::SoldierSpawner);
		if (strstr(flags, "Bridge"))
			m_AreaFlags.push_back(EAreaFlag::Bridge);
		if (strstr(flags, "Base"))
			m_AreaFlags.push_back(EAreaFlag::Base);
		if (strstr(flags, "AirField"))
			m_AreaFlags.push_back(EAreaFlag::AirField);
		if (strstr(flags, "SupplyPoint"))
			m_AreaFlags.push_back(EAreaFlag::SupplyPoint);
		if (strstr(flags, "ControlPoint"))
			m_AreaFlags.push_back(EAreaFlag::ControlPoint);
		if (strstr(flags, "North"))
			m_AreaFlags.push_back(EAreaFlag::North);
		if (strstr(flags, "West"))
			m_AreaFlags.push_back(EAreaFlag::West);
		if (strstr(flags, "South"))
			m_AreaFlags.push_back(EAreaFlag::South);
		if (strstr(flags, "East"))
			m_AreaFlags.push_back(EAreaFlag::East);
		if (strstr(flags, "Safe"))
			m_AreaFlags.push_back(EAreaFlag::Safe);
		if (strstr(flags, "Neutral"))
			m_AreaFlags.push_back(EAreaFlag::Neutral);
		if (strstr(flags, "Front"))
			m_AreaFlags.push_back(EAreaFlag::Front);

		const int maxSpeciesCount = ESpeciesType::eST_LastPlayableSpecies;
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
				const char* unlockedClass = "";

				std::vector<const char*> classes;

				for (int y = 0; y < vehicleVariationCount; y++)
				{
					char indexChar[256];
					sprintf(indexChar, "%i", y);

					string paramIndex = indexChar;
					string paramName = "unlockedClass_" + paramIndex;

					//CryLogAlways("archetypeName %s", paramName);

					if (speciesTable->GetValue(paramName.c_str(), unlockedClass))
					{
						if (strcmp(unlockedClass, "") != 0)
						{
							//CryLogAlways("[C++][Vehicle Spawner %s][Species %i][Add Archetype %s]",
								//GetEntity()->GetName(), currentSpecies, vehicleArchetype);

							classes.push_back(unlockedClass);
						}
					}
				}

				m_unlockedClasses.insert(std::make_pair(currentSpecies, classes));
			}
		}

		//for (auto flag : m_AreaFlags)
		//{
			//CryLogAlways("[C++][Strategic Area %s][Add Flag %i]", GetEntity()->GetName(), flag);
		//}
	}
	else
	{
		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$2[C++][ERROR][Strategic Area %s][Fail Get Lua Values]", GetEntity()->GetName());
	}
}

//void CStrategicArea::UpdateVehicleScheduler(float frametime)
//{
//	
//}

//void CStrategicArea::CreateVehicleAbandonData(EntityId entityId, float timer)
//{
//}

//void CStrategicArea::CancelVehicleAutoDestroy(EntityId entityId, std::map<EntityId, int>& soundPlaying)
//{
//	auto it = m_vehiclesAutoDestroyScheduler.find(entityId);
//	if (it != m_vehiclesAutoDestroyScheduler.end())
//	{
//		soundPlaying[entityId] = 0;
//		m_vehiclesAutoDestroyScheduler.erase(it);
//	}
//}

//void CStrategicArea::InitVehicleReset(EntityId id)
//{
//}
//
//void CStrategicArea::TriggerVehicleReset(EntityId id)
//{
//}

void CStrategicArea::AddToArea(EntityId entityId, bool vehicle)
{
	if (!IsActorInside(entityId))
	{
		m_ActorsInside.push_back(entityId);

		IEntity* pEntity = g_pControlSystem->GetLocalControlClient()->GetControlledEntity();
		if (pEntity)
		{
			if (entityId == pEntity->GetId())
				OnClientEnter(vehicle);
		}
		else
		{
			if (entityId == g_pGame->GetIGameFramework()->GetClientActorId())
				OnClientEnter(vehicle);
		}	

		ChangeAgentCount(entityId, "+");

		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$3[C++][Strategic Area %s][Add Actor Id] %i", GetEntity()->GetName(), entityId);
	}
		
}

void CStrategicArea::AddToArea(IActor* pActor, bool vehicle)
{
	if (!IsActorInside(pActor))
	{
		m_ActorsInside.push_back(pActor->GetEntityId());

		IEntity* pEntity = g_pControlSystem->GetLocalControlClient()->GetControlledEntity();
		if (pEntity)
		{
			if (pActor->GetEntityId() == pEntity->GetId())
				OnClientEnter(vehicle);
		}
		else
		{
			if (pActor->GetEntityId() == g_pGame->GetIGameFramework()->GetClientActorId())
				OnClientEnter(vehicle);
		}

		ChangeAgentCount(pActor->GetEntityId(), "+");

		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$3[C++][Strategic Area %s][Add Actor Id] %i", GetEntity()->GetName(), pActor->GetEntityId());
	}
}

void CStrategicArea::RemoveFromArea(IActor* pActor)
{
	if (pActor)
	{
		RemoveFromArea(pActor->GetEntityId());

		//stl::find_and_erase(m_ActorsInside, pActor->GetEntityId());

		//IEntity* pEntity = g_pControlSystem->GetLocalControlClient()->GetControlledEntity();
		//if (pEntity)
		//{
		//	if (pActor->GetEntityId() == pEntity->GetId())
		//		OnClientExit();
		//}
		//else
		//{
		//	if (pActor->GetEntityId() == g_pGame->GetIGameFramework()->GetClientActorId())
		//		OnClientExit();
		//}

		//ChangeAgentCount(pActor->GetEntityId(), "-");
		//if (g_pGameCVars->conq_debug_log_area)
		//	CryLogAlways("[CCapturableArea]::[RemoveActor pActor] entityId %i", pActor->GetEntityId());
	}
}

void CStrategicArea::RemoveFromArea(EntityId entityId)
{
	if (gEnv->pEntitySystem->GetEntity(entityId))
	{
		stl::find_and_erase(m_ActorsInside, entityId);

		IEntity* pEntity = g_pControlSystem->GetLocalControlClient()->GetControlledEntity();
		if (pEntity)
		{
			if (entityId == pEntity->GetId())
				OnClientExit();
		}
		else
		{
			if (entityId == g_pGame->GetIGameFramework()->GetClientActorId())
				OnClientExit();
		}

		ChangeAgentCount(entityId, "-");

		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$3[Strategic Area %s][Remove Actor Id] %i", GetEntity()->GetName(), entityId);
	}
}

void CStrategicArea::RemoveActorAt(int entityIndex)
{
	const EntityId id = m_ActorsInside.at(entityIndex);
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);

	if (pActor)
	{
		stl::find_and_erase(m_ActorsInside, id);
		ChangeAgentCount(pActor->GetEntityId(), "-");

		if (g_pGameCVars->conq_debug_log_area)
			CryLogAlways("$3[Strategic Area %s][Remove Actor Id] %i", GetEntity()->GetName(), id);
	}
}

int CStrategicArea::GetActorCount()
{
	return m_ActorsInside.size();
}

EntityId CStrategicArea::GetActorIdAt(int entityIndex)
{
	return m_ActorsInside.at(entityIndex);
}

IActor* CStrategicArea::GetActorAt(int entityIndex)
{
	const EntityId entityId = m_ActorsInside.at(entityIndex);
	return g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
}

bool CStrategicArea::IsActorInside(IActor* actor)
{
	if (actor)
		return stl::find(m_ActorsInside, actor->GetEntityId());

	return false;
}

bool CStrategicArea::IsActorInside(EntityId entityId)
{
	return stl::find(m_ActorsInside, entityId);
}

bool CStrategicArea::IsValidEntityType(const EEntityType& requestedType, const EEntityType& type)
{
	bool bValid = false;

	if (requestedType == EEntityType::VALID)
	{
		bValid = (type & EEntityType::VALID) == EEntityType::VALID;
	}
	else if (requestedType == EEntityType::AI)
	{
		bValid = (type & EEntityType::AI) == EEntityType::AI;
	}
	else if (requestedType == EEntityType::ACTOR)
	{
		bValid = (type & EEntityType::ACTOR) == EEntityType::ACTOR;
	}
	else if (requestedType == EEntityType::VEHICLE)
	{
		bValid = (type & EEntityType::VEHICLE) == EEntityType::VEHICLE;
	}
	else if (requestedType == EEntityType::ITEM)
	{
		bValid = (type & EEntityType::ITEM) == EEntityType::ITEM;
	}

	return bValid;
}

bool CStrategicArea::IsAreaOwner(const ESpeciesType& species)
{
	return species == m_Species;
}

EEntityType CStrategicArea::GetEntityType(EntityId id)
{
	int type = EEntityType::UNKNOWN;

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	if (pEntitySystem)
	{
		IEntity* pEntity = pEntitySystem->GetEntity(id);
		if (pEntity)
		{
			type = EEntityType::VALID;

			IEntityClass* pClass = pEntity->GetClass();
			if (pClass)
			{
				const char* className = pClass->GetName();

				// Check AI
				if (pEntity->GetAI())
				{
					type |= EEntityType::AI;
				}

				// Check actor
				IActorSystem* pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();
				if (pActorSystem)
				{
					IActor* pActor = pActorSystem->GetActor(id);
					if (pActor)
					{
						type |= EEntityType::ACTOR;
					}
				}

				// Check vehicle
				IVehicleSystem* pVehicleSystem = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem();
				if (pVehicleSystem)
				{
					if (pVehicleSystem->IsVehicleClass(className))
					{
						type |= EEntityType::VEHICLE;
					}
				}

				// Check item
				IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
				if (pItemSystem)
				{
					if (pItemSystem->IsItemClass(className))
					{
						type |= EEntityType::ITEM;
					}
				}
			}
		}
	}

	return (EEntityType)type;
}

int CStrategicArea::GetActorCountBySpecies(int _species)
{
	int aiEntitiesNum = 0;
	int entitiesCount = GetActorCount();

	for (int i = 0; i < entitiesCount; i++)
	{
		EntityId entityId = m_ActorsInside.at(i);
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);

		if (IAIObject* pEntityAI = pEntity->GetAI())
		{
			if (_species == pEntityAI->CastToIAIActor()->GetParameters().m_nSpecies)
				++aiEntitiesNum;
		}
	}

	return aiEntitiesNum;
}