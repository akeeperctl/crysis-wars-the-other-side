#include "StdAfx.h"

#include "IAgent.h"
#include "IMovieSystem.h"
#include "ILevelSystem.h"
#include "IVehicleSystem.h"

#include "Scout.h"
#include "Actor.h"

#include "Menus/FlashMenuObject.h"
#include "GameActions.h"

#include "HUD/HUD.h"
#include "HUD/HUDScore.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDCrosshair.h"
#include "HUD/HUDPowerStruggle.h"

#include "TheOtherSide/Conqueror/AreaVehicleSpawnPoint.h"
#include "TheOtherSide/Conqueror/StrategicArea.h"
#include "TheOtherSide/Conqueror/ConquerorStrategy.h"
#include "TheOtherSide/Conqueror/ConquerorCommander.h"
#include "TheOtherSide/Conqueror/ConquerorShop.h"
#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Conqueror/SpawnPoint.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Conqueror/ConquerorChannel.h"
#include "TheOtherSide/Conqueror/ConquerorSpeciesClass.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"

#include "TheOtherSide/Helpers/TOS_AI.h"
#include "TheOtherSide/Helpers/TOS_Script.h"
#include "TheOtherSide/Helpers/TOS_Debug.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "TheOtherSide/Helpers/TOS_Inventory.h"

#define UPDATE_POWERSTRUGGLE_BUY_INTERFACE \
auto pHud = g_pGame->GetHUD();\
if (pHud)\
{\
	auto pPSHUD = pHud->GetPowerStruggleHUD();\
	if (pPSHUD)\
	{\
		pPSHUD->UpdateBuyList();\
		pPSHUD->UpdateBuyZone(true, 0);\
		pPSHUD->UpdateServiceZone(true, 0);\
	}\
}\

CConquerorSystem::CConquerorSystem()
{
	m_pRARSystem = new CRequestsAndResponses();
	m_pShop = new CConquerorShop(this);

	m_vehiclesCanBookUnloadSpots.clear();
	m_strategicAreas.clear();
	m_conquerorChannels.clear();
	m_strategies.clear();
	m_preRespawns.clear();

	m_speciesCommanders.clear();
	m_speciesClassesMap.clear();
	m_speciesDefaultClassesMap.clear();
	m_speciesFlagIndexMap.clear();
	m_speciesCharNameMap.clear();
	m_speciesAutoDestroyTime.clear();
	m_gameAllowedSpecies.clear();
	m_lobbyAllowedSpecies.clear();
	m_vehicleSpawners.clear();
	m_vehicleClasses.clear();

	//m_areaSpawnedLeadersCountMap.clear();

	m_gameStatus = eGS_GameStart;
	m_pClientArea = nullptr;
	m_pSelectedClientAreaEntity = nullptr;

	m_pAnimScoreBoard = nullptr;
	//m_pAnimBuyMenu = nullptr;
	//m_pAnimRespawnTimer = nullptr;
	m_pAnimPlayerPP = nullptr;
	m_pAnimPlayerPPChange = nullptr;
	m_pAnimMPMessages = nullptr;
	m_pAnimBattleLog = nullptr;
	m_pAnimKillLog = nullptr;

	m_oldLobbySpeciesIndex = -1;
	m_oldLobbyClassIndex = 0;

	m_lobbyConfirmedSpeciesIndex = -1;
	m_lobbyConfirmedClassIndex = 0;

	m_lobbySelectedClassIndex = 0;
	m_lobbySelectedSpeciesIndex = 0;

	m_usedAlienId = 0;
	m_isDebugLog = false;
	m_haveBotsSpawned = false;
	m_botsSpawnedTime = 0;
	m_gameOver = false;

	m_friendPointsCount = 0;
	m_enemyPointsCount = 0;
	//m_playerSpawnedCount = 0;

	m_CVarBotsJoinBeforePlayer = false;
	m_CVarRespawnTime = 3;
	m_CVarTimeLimit = 0.0f;
	m_CVarSpeciesChangeLimit = 0;
	m_speciesSwitchCount = 0;

	m_lastClientPointsSet = 0;
	m_lastTimeAIReset = 0;

	m_pXMLAICountInfo = new CConquerAICountInfo();

	for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; ++i)
		m_speciesLeadersCountMap[ESpeciesType(i)] = 0;

	//for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; ++i)
	//	m_speciesFlagIndexMap[ESpeciesType(i)] = -1;
}

CConquerorSystem::~CConquerorSystem()
{
	if (g_pControlSystem)
		g_pControlSystem->RemoveChild(this, false);

	SAFE_DELETE(m_pRARSystem);
	SAFE_DELETE(m_pShop);
	SAFE_DELETE(m_pXMLAICountInfo);

	RemoveAllChannels();
}

void CConquerorSystem::OnMainMenuEnter()
{
	m_LobbyInfo = SConquerLobbyInfo();
	m_vehiclesCanBookUnloadSpots.clear();
	m_strategicAreas.clear();
	m_conquerorChannels.clear();
	m_strategies.clear();
	m_preRespawns.clear();

	m_speciesCommanders.clear();
	m_speciesClassesMap.clear();
	m_speciesDefaultClassesMap.clear();
	m_speciesFlagIndexMap.clear();
	m_speciesCharNameMap.clear();
	m_speciesAutoDestroyTime.clear();
	m_gameAllowedSpecies.clear();
	m_lobbyAllowedSpecies.clear();
	m_vehicleSpawners.clear();
	m_vehicleClasses.clear();

	m_gameStatus = eGS_GameStart;
	m_pClientArea = nullptr;
	m_pSelectedClientAreaEntity = nullptr;

	m_pAnimScoreBoard = nullptr;
	m_pAnimPlayerPP = nullptr;
	m_pAnimPlayerPPChange = nullptr;
	m_pAnimMPMessages = nullptr;
	m_pAnimBattleLog = nullptr;
	m_pAnimKillLog = nullptr;

	m_oldLobbySpeciesIndex = -1;
	m_oldLobbyClassIndex = 0;

	m_lobbyConfirmedSpeciesIndex = -1;
	m_lobbyConfirmedClassIndex = 0;

	m_lobbySelectedClassIndex = 0;
	m_lobbySelectedSpeciesIndex = 0;

	m_usedAlienId = 0;
	m_isDebugLog = false;
	m_haveBotsSpawned = false;
	m_botsSpawnedTime = 0;
	m_gameOver = false;

	m_friendPointsCount = 0;
	m_enemyPointsCount = 0;
	//m_playerSpawnedCount = 0;

	m_CVarBotsJoinBeforePlayer = false;
	m_CVarRespawnTime = 3;
	m_CVarTimeLimit = 0.0f;
	m_CVarSpeciesChangeLimit = 0;
	m_speciesSwitchCount = 0;

	m_lastClientPointsSet = 0;
	m_lastTimeAIReset = 0;

	if (m_pRARSystem)
	{
		m_pRARSystem->Reset();
	}

	for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; ++i)
		m_speciesLeadersCountMap[ESpeciesType(i)] = 0;
}

bool CConquerorSystem::OnInputEvent(const SInputEvent& rInputEvent)
{
	const auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return false;

	if (!IsGamemode())
		return false;

	const auto pDudeChannel = GetClientConquerorChannel();
	if (!pDudeChannel)
		return false;

	const bool inLobby = m_LobbyInfo.state == EConquerLobbyState::IN_LOBBY;
	const bool isKeyboard = rInputEvent.deviceId == eDI_Keyboard;
	const bool isMouse = rInputEvent.deviceId == eDI_Mouse;

	bool inMenu = g_pGame->GetMenu()->IsActive();
	const bool inCutscene = g_pGame->GetHUD()->IsInCinematic();
	const bool inConsole = gEnv->pConsole->IsOpened();

	if (inConsole || (inLobby && inCutscene))
		return false;

	if (isKeyboard)
	{
		if (m_gameOver)
			return false;
		else if (m_gameOver && rInputEvent.keyId == eKI_Escape)
		{
			CryLogAlways("[C++][Conqueror][Game Over, open inGame main menu]");
			g_pGame->GetMenu()->ShowInGameMenu(true);
			return false;
		}

		if (inLobby || pDudeChannel->GetState() == eCCS_Dead)
		{
			//M - MAP
			if (rInputEvent.keyId == eKI_M && rInputEvent.state == eIS_Pressed)
			{	

				//if (pDudeChannel->GetState() == eCCS_Dead)
				//{
				//	const bool isVisible = pHUD->m_animPDA.GetVisible();
				//	if (isVisible)
				//	{
				//		pHUD->ShowPDA(false);
				//		//pHUD->m_animPDA.SetVisible(false);
				//		//pHUD->GetRadar()->SetRenderMapOverlay(false);
				//		//pHUD->SetModalHUD(nullptr);
				//		//pHUD->CursorDecrementCounter();
				//	}
				//	else
				//	{
				//		pHUD->ShowPDA(true);
				//		//pHUD->m_animPDA.SetVisible(true);
				//		//pHUD->GetRadar()->SetRenderMapOverlay(true);
				//		//pHUD->SetModalHUD(&pHUD->m_animPDA);
				//		//pHUD->CursorIncrementCounter();
				//	}
				//}
				//else
				//{
				//	const bool isVisible = pHUD->m_animPDA.GetVisible();
				//	const bool show = isVisible ? false : true;
				//	pHUD->ShowPDA(show, false);
				//	//pHUD->GetRadar()->SetRenderMapOverlay(show);
				//}
			
				//const bool isVisible = pHUD->m_animPDA.GetVisible();
				//const bool show = isVisible ? false : true;

				//if (show)
					pHUD->SetModalHUD(nullptr);

				pHUD->ShowPDA(true, false);

				return true;
			}
		}
		else
		{
			if (rInputEvent.keyId == eKI_P && rInputEvent.state == eIS_Pressed)
			{
				const auto pArea = GetClientArea();
				if (pArea && pArea->IsBuyZoneActived(GetClientSpecies()))
				{
					//const bool isVisible = m_pAnimBuyMenu->GetVisible();
					//const bool show = isVisible ? false : true;
					//m_pAnimBuyMenu->SetVisible(show);

					pHUD->ShowBuyMenu(true);
				}
				else
				{
					pHUD->ShowBuyMenu(false);
				}
			}
		}
	}
	else if (isMouse)
	{
		if (m_gameOver)
			return false;

		if (inLobby)
		{
			if (pHUD->GetModalHUD() == &pHUD->m_animPDA)
			{
				if (rInputEvent.state == eIS_Pressed)
				{
					if (rInputEvent.keyId == eKI_MouseWheelUp)
					{
						pHUD->GetRadar()->ZoomPDA(true);
					}
					else if (rInputEvent.keyId == eKI_MouseWheelDown)
					{
						pHUD->GetRadar()->ZoomPDA(false);
					}

					if (rInputEvent.keyId == eKI_Mouse2)
					{
						pHUD->GetRadar()->SetDrag(true);
					}
				}
				else if (rInputEvent.state == eIS_Released)
				{
					if (rInputEvent.keyId == eKI_Mouse2)
					{
						pHUD->GetRadar()->SetDrag(false);
					}
				}
			}
		}
	}

	return false;
}

void CConquerorSystem::Init()
{
	g_pControlSystem->AddChild(this, false);
	InitVehicleClasses(true);
}

void CConquerorSystem::OnActorRespawned(IActor* pActor, ERespawnEvent event)
{
	if (!pActor)
		return;

	const auto pCActor = dynamic_cast<CActor*>(pActor);

	const auto pChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(pActor->GetEntityId());
	if (!pChannel)
		return;

	if (GetLobbyInfo().state == EConquerLobbyState::IN_LOBBY)
	{
		if (pActor == g_pGame->GetIGameFramework()->GetClientActor())
		{
			//if (pChannel->m_pLastStrategicArea)
				//m_areaSpawnedLeadersCountMap[pChannel->m_pLastStrategicArea->GetEntityId()] += 1;

			SConquerLobbyInfo info;
			info = GetLobbyInfo();
			info.state = EConquerLobbyState::IN_GAME;
			SetLobbyInfo(info);// Spawn AI Bots

			const auto species = pChannel->GetSpecies();
			const auto squadId = GetRandomSquadId(species);

			pActor = g_pControlSystem->GetClientActor();

			if (g_pGameCVars->conq_debug_log_aichannel)
				CryLogAlways("[C++][Player %s Respawn and Get Random Squad Id %i]",
					pActor->GetEntity()->GetName(), squadId);

			const auto pNewRandomSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromId(squadId);
			if (pNewRandomSquad)
			{
				pNewRandomSquad->SetOldAILeader(pNewRandomSquad->GetLeader());
				pNewRandomSquad->SetLeader(pActor, true);

				const auto pOldLeader = pNewRandomSquad->GetOldAILeader();
				if (pOldLeader)
				{
					//the old squad leader should not be in the vehicle
					//when his squad is selected for the player

					const auto pVehicle = TOS_Vehicle::GetVehicle(pOldLeader);
					if (pVehicle)
					{
						//TODO NEED FIX THIS SHIT
						TOS_Vehicle::Exit(pOldLeader, false, true);

						for (auto& member : pNewRandomSquad->GetAllMembers())
						{
							const auto pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
							if (pMember)
							{
								const auto pMemberVeh = TOS_Vehicle::GetVehicle(pMember);
								if (pMemberVeh)
									TOS_Vehicle::Exit(pMember, false, true);
							}
						}

						if (species != eST_Aliens)
						{
							const int idx = TOS_Vehicle::RequestFreeSeatIndex(pVehicle);

							const auto pSeat = pVehicle->GetSeatById(idx);
							if (pSeat)
								pSeat->Enter(pActor->GetEntityId(), false);
						}
					}

					const auto pOldLeaderChannel = GetConquerorChannel(pOldLeader->GetEntity());
					if (pOldLeaderChannel)
					{
						pOldLeaderChannel->OnSpectator(eSC_PlayerSpawned);

						if (g_pGameCVars->conq_debug_log_aichannel)
							CryLogAlways("[C++][New Random Squad %i Setup New Leader (%s) and Hide Old Leader (%s)]",
								pNewRandomSquad->GetId(), pActor->GetEntity()->GetName(), pOldLeader->GetEntity()->GetName());
					}
				}
				
				//respawn squad members at leader area position
				if (m_haveBotsSpawned)
				{
					for (auto& member : pNewRandomSquad->GetAllMembers())
					{
						const auto pEntity = gEnv->pEntitySystem->GetEntity(member.GetId());
						if (!pEntity)
							continue;

						const auto pMemChannel = GetConquerorChannel(pEntity);
						if (!pMemChannel)
							continue;

						const auto pArea = pMemChannel->GetLeaderStrategicArea();
						if (!pArea)
							continue;

						if (!pArea->IsQueueCreated())
							pArea->CreateQueue(1.0f);

						pArea->AddToQueue(pEntity->GetId(), eRC_OnGameStartSpawn);
						//pChannel->SpawnActor(eRC_OnGameStartSpawn, pArea->GetSpawnPoint(eSGSF_NotHaveRecentlySpawned));
					}
				}

				CHUDCrosshair* pCrosshair = g_pGame->GetHUD()->GetCrosshair();
				if (pCrosshair)
				{
					pCrosshair->SetOpacity(1.0f);
					pCrosshair->SetCrosshair(g_pGameCVars->hud_crosshair);
				}
			}

			if (m_gameOver)
			{
				g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_you_win", 8.0f, ColorF(0.0f, 1.0f, 0.0f));
			}
		}
	}

	if (pActor == g_pControlSystem->GetClientActor())
	{
		auto pPlayerSquad = g_pControlSystem->GetSquadSystem()->GetClientSquad();
		
		//respawn squad members at leader area position
		if (m_haveBotsSpawned && pPlayerSquad)
		{
			const auto pArea = GetNearestStrategicArea(
				pActor->GetEntity()->GetWorldPos(),
				OWNED,
				eAGSF_EnabledAndCapturable,
				GetClientSpecies(),
				EAreaBusyFlags::eABF_NoMatter,
				EAreaFlag::ControlPoint);
			
			for (auto& member : pPlayerSquad->GetAllMembers())
			{
				const auto pEntity = gEnv->pEntitySystem->GetEntity(member.GetId());
				if (!pEntity)
					continue;
				
				if (!pArea)
					continue;

				if (!pArea->IsQueueCreated())
					pArea->CreateQueue(1.0f);

				pArea->AddToQueue(pEntity->GetId(), eRC_OnGameStartSpawn);
				//pChannel->SpawnActor(eRC_OnGameStartSpawn, pArea->GetSpawnPoint(eSGSF_NotHaveRecentlySpawned));
			}
		}

		if (strcmp(pActor->GetEntity()->GetClass()->GetName(), "Trooper") == 0)
		{
			std::vector<EntityId> iteratedExecutors;
			
			const auto it = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
			while (const auto pIteratedActor = static_cast<CActor*>(it->Next()))
			{
				if (m_pRARSystem->IsAssignedExecutor(pIteratedActor->GetEntityId()))
					continue;

				if (TOS_AI::GetSpecies(pIteratedActor->GetEntity()->GetAI(), false) !=
					TOS_AI::GetSpecies(pActor->GetEntity()->GetAI(), false))
				{
					continue;
				}

				if (strcmp(pIteratedActor->GetEntity()->GetClass()->GetName(), "Scout") != 0)
					continue;

				if (pIteratedActor->GetGrabStats()->grabbedIds.empty() == false)
					continue;

				iteratedExecutors.push_back(pIteratedActor->GetEntityId());
			}

			float minDistance = 0.0f;
			EntityId scoutId = 0;

			for (auto id : iteratedExecutors)
			{
				const auto pEntity = gEnv->pEntitySystem->GetEntity(iteratedExecutors[0]);
				if (pEntity != nullptr)
				{
					const auto pos = pEntity->GetWorldPos();
					const auto dist = (pos - pActor->GetEntity()->GetWorldPos()).GetLength();

					if (minDistance == 0.0f && scoutId == 0)
					{
						minDistance = (pEntity->GetWorldPos() - pos).GetLength();
						scoutId = pEntity->GetId();
					}
					else if (dist < minDistance)
					{
						minDistance = dist;
						scoutId = pEntity->GetId();
					}
				}
			}

			const auto pScoutEntity = GET_ENTITY(scoutId);
			if (pScoutEntity)
			{
				Script::CallMethod(pScoutEntity->GetScriptTable(), "GrabEntityInTentacle", ScriptHandle(pActor->GetEntityId()));

				for (auto& members : pPlayerSquad->GetAllMembers())
				{
					auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(members.GetId());
					if (!pMemberActor)
						continue;

					if (pMemberActor->GetHealth() < 0)
						continue;

					Script::CallMethod(pScoutEntity->GetScriptTable(), "GrabEntityInTentacle", ScriptHandle(pMemberActor->GetEntityId()));
				}

				const auto pos = pActor->GetEntity()->GetWorldPos();
				const auto pGoalArea = g_pControlSystem->GetConquerorSystem()->GetNearestStrategicArea(
					pos,
					HOSTILE,
					eAGSF_EnabledAndCapturable,
					GetClientSpecies(),
					eABF_NoMatter,
					EAreaFlag::ControlPoint);
			
				if (pGoalArea)
				{
					IEntity* pEntity = nullptr;

					EntityId id = pGoalArea->GetBookedUnloadSpot(pScoutEntity);
					if (id == 0)
						id = pGoalArea->BookFreeUnloadSpot(pScoutEntity);

					if (id)
						pEntity = GET_ENTITY(id);

					const char* desiredGoalName = "";
					const int goalPipeId = -1;
					const char* actionName = "conqueror_goto_and_drop";
					const char* solution = "";
					const auto maxAlertness = 102.0f; //high prioritry
					const auto flag = eAAEF_IgnoreCombatDuringAction;
				
					if (!TOS_AI::IsExecuting(pScoutEntity->GetAI(), actionName))
						TOS_AI::ExecuteAIAction(pScoutEntity->GetAI(), pEntity, actionName, maxAlertness, goalPipeId, flag, desiredGoalName, solution);
				}
			}
		}
	}
	
	const auto actorSpecies = (ESpeciesType)GetSpeciesFromEntity(pActor->GetEntity());
	if (event == eRC_OnKilledRespawn)
		AddSpeciesReinforcements(actorSpecies, 1, -1);

	//if (pActor->IsPlayer())
		//m_playerSpawnedCount++;

	pCActor->GetActorStats()->lastTimeRespawned = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pActor);
	if (pSquad)
	{
		auto pLeader = static_cast<CActor*>(pSquad->GetLeader());
		//pSquad->m_leadersStats[pSquad->GetLeader()->GetEntityId()].lastTimeConquerorSpawned = gEnv->pTimer->GetFrameStartTime().GetSeconds();

		const auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(actorSpecies);
		if (pCommander)
			pCommander->OnSquadLeaderSpawned(pSquad);
	}

	pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, false);
	if (pSquad)
	{
		const auto pMember = pSquad->GetMemberInstance(pActor);
		if (pMember)
		{
			pMember->GetStats()->lastTimeConquerorSpawned = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			const auto pCommander = g_pControlSystem->GetConquerorSystem()->GetSpeciesCommander(actorSpecies);
			if (pCommander)
				pCommander->OnSquadMemberSpawned(pSquad, pMember);
		}
	}

	//CryLogAlways("%s[C++][Actor %s: respawned]", STR_PURPLE, pActor->GetEntity()->GetName());
}

void CConquerorSystem::OnActorAddedInQueue(IActor* pActor, CStrategicArea* pArea)
{
	if (!pActor || !pArea)
		return;

	if (pActor == g_pGame->GetIGameFramework()->GetClientActor())
	{
		ShowRespawnCycle(true);
	}

	if (g_pGameCVars->conq_debug_log_area)
	{
		CryLogAlways("%s[C++][Actor %s added to Queue in %s]",
			STR_GREEN, pActor->GetEntity()->GetName(), pArea->GetEntity()->GetName());
	}
}

void CConquerorSystem::OnActorRemovedFromQueue(IActor* pActor)
{
	if (!pActor)
		return;

	if (pActor == g_pGame->GetIGameFramework()->GetClientActor())
	{
		ShowRespawnCycle(false);
		SetRespawnCycleRemainingTime((int)GetRespawnTime(), 0);
	}

	//CryLogAlways("%s[C++][Actor %s removed from Queue]",
	//	STR_GREEN, pActor->GetEntity()->GetName());
}

void CConquerorSystem::AddVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner)
{
	stl::push_back_unique(m_vehicleSpawners, pSpawner);
}

CAreaVehicleSpawnPoint* CConquerorSystem::GetVehicleSpawner(EntityId id) const
{
	for (const auto pSpawner : m_vehicleSpawners)
	{
		if (pSpawner->GetEntityId() == id)
			return pSpawner;
	}

	CryLogAlways("RETURN NULL VEHICLE SPAWNER");

	return nullptr;
}

void CConquerorSystem::RemoveVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner)
{
	stl::find_and_erase(m_vehicleSpawners, pSpawner);
}

bool CConquerorSystem::IsExistVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner) const
{
	return stl::find(m_vehicleSpawners, pSpawner);
}

SConquerLobbyInfo::SConquerLobbyInfo()
{
	state = EConquerLobbyState::PRE_GAME;
	modelEntityId = 0;
	modelPos = Vec3(ZERO);
	modelScale = Vec3(1);
	isConquest = 0;
}

void CConquerorSystem::AddStrategicArea(CStrategicArea* area)
{
	if (area)
	{
		if (!IsExistStrategicArea(area))
			m_strategicAreas.push_back(area);
	}
}

void CConquerorSystem::RemoveStrategicArea(CStrategicArea* area)
{
	if (area)
		stl::find_and_erase(m_strategicAreas, area);	
}

bool CConquerorSystem::IsExistStrategicArea(CStrategicArea* area)
{
	if (area)
		return stl::find(m_strategicAreas, area);
	
	return false;
}

void CConquerorSystem::GetStrategicAreas(std::vector<CStrategicArea*>& areas, ESpeciesType targetSpecies, EAreaGameStatusFlag gameStatus, ESpeciesType owner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	for (auto pArea : m_strategicAreas)
	{
		if (pArea->GetSpecies() != targetSpecies)
			continue;

		if (gameStatus == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}
		else if (gameStatus == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameStatus == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}

		const auto& flags = pArea->GetFlags();
		if (!stl::find(flags, areaFlag))
			continue;

		const auto pCommander = GetSpeciesCommander(owner);
		if (pCommander)
		{
			if (busyFlags == eABF_AreaIsSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) <= 0)
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) > 0)
					continue;
			}
		}

		areas.push_back(pArea);
	}
}

CStrategicArea* CConquerorSystem::GetNearestStrategicArea(const Vec3& pos, const string& areaStatus, EAreaGameStatusFlag gameStatus, ESpeciesType desiredOwner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	std::vector<CStrategicArea*> allAreasVector;
	for (auto pArea : m_strategicAreas)
	{
		if (areaStatus == OWNED)
		{
			if (pArea->GetSpecies() != desiredOwner)
				continue;
		}
		else if (areaStatus == HOSTILE)
		{
			if (pArea->GetSpecies() == desiredOwner)
				continue;
		}

		if (gameStatus == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}
		else if (gameStatus == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameStatus == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}


		const auto& flags = pArea->GetFlags();

		if (!stl::find(flags, areaFlag))
			continue;

		const auto pCommander = GetSpeciesCommander(desiredOwner);
		if (pCommander)
		{
			if (busyFlags == eABF_AreaIsSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) <= 0)
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) > 0)
					continue;
			}
		}


		allAreasVector.push_back(pArea);
	}

	if (allAreasVector.size() == 0)
		return nullptr;

	const auto pFirstArea = allAreasVector[0];
	if (!pFirstArea)
	{
		CryLogAlways("$4[C++][ERROR][Conqueror System][Get Nearest Strategic Area][NULL FIRST AREA]");
		return nullptr;
	}

	//CryLogAlways("[C++][Species %s][Enemy Areas Count %i]", m_pConqueror->GetSpeciesName(m_species), enemyAreasVector.size());

	const Vec3 firstAreaPos = pFirstArea->GetEntity()->GetWorldPos();
	float minDistance = (firstAreaPos - pos).GetLength();

	//CryLogAlways("[C++][Species %s][Distance to first area %1.f]", m_pConqueror->GetSpeciesName(m_species), minDistance);

	CStrategicArea* pNearestArea = nullptr;

	for (const auto pArea : allAreasVector)
	{
		auto& areaPos = pArea->GetEntity()->GetWorldPos();
		const float distance = (areaPos - pos).GetLength();

		if (distance < minDistance)
		{
			minDistance = distance;
			pNearestArea = pArea;
		}
	}

	if (pNearestArea == nullptr)
		pNearestArea = pFirstArea;

	return pNearestArea;
}

CStrategicArea* CConquerorSystem::GetNearestStrategicArea(const Vec3& pos, ESpeciesType targetSpecies, EAreaGameStatusFlag gameStatus, ESpeciesType desiredOwner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag)
{
	std::vector<CStrategicArea*> allAreasVector;
	for (auto pArea : m_strategicAreas)
	{
		if (pArea->GetSpecies() != targetSpecies)
			continue;

		if (gameStatus == eAGSF_Enabled)
		{
			if (!pArea->IsEnabled())
				continue;
		}
		else if (gameStatus == eAGSF_Capturable)
		{
			if (!pArea->IsCapturable())
				continue;
		}
		else if (gameStatus == eAGSF_EnabledAndCapturable)
		{
			if (!pArea->IsEnabled())
				continue;

			if (!pArea->IsCapturable())
				continue;
		}


		const auto& flags = pArea->GetFlags();

		if (!stl::find(flags, areaFlag))
			continue;

		const auto pCommander = GetSpeciesCommander(desiredOwner);
		if (pCommander)
		{
			if (busyFlags == eABF_AreaIsSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) <= 0)
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTSquadTarget)
			{
				const auto pSquads = pCommander->GetAssignedSquadsForArea(pArea);
				if (!pSquads || (pSquads && pSquads->size() == 0))
					continue;
			}
			else if (busyFlags == eABF_AreaIsNOTHaveEnemyGuards)
			{
				if (pCommander->GetEnemyCountAroundArea(pArea, false) > 0)
					continue;
			}
		}


		allAreasVector.push_back(pArea);
	}

	if (allAreasVector.size() == 0)
		return nullptr;

	const auto pFirstArea = allAreasVector[0];
	if (!pFirstArea)
	{
		CryLogAlways("$4[C++][ERROR][Conqueror System][Get Nearest Strategic Area][NULL FIRST AREA]");
		return nullptr;
	}

	//CryLogAlways("[C++][Species %s][Enemy Areas Count %i]", m_pConqueror->GetSpeciesName(m_species), enemyAreasVector.size());

	const Vec3 firstAreaPos = pFirstArea->GetEntity()->GetWorldPos();
	float minDistance = (firstAreaPos - pos).GetLength();

	//CryLogAlways("[C++][Species %s][Distance to first area %1.f]", m_pConqueror->GetSpeciesName(m_species), minDistance);

	CStrategicArea* pNearestArea = nullptr;

	for (const auto pArea : allAreasVector)
	{
		auto& areaPos = pArea->GetEntity()->GetWorldPos();
		const float distance = (areaPos - pos).GetLength();

		if (distance < minDistance)
		{
			minDistance = distance;
			pNearestArea = pArea;
		}
	}

	if (pNearestArea == nullptr)
		pNearestArea = pFirstArea;

	return pNearestArea;
}

CStrategicArea* CConquerorSystem::GetBaseStrategicArea(ESpeciesType species)
{
	for (const auto pArea : m_strategicAreas)
	{
		if (pArea->GetSpecies() != species)
			continue;

		if (pArea->IsHaveFlag(EAreaFlag::Base))
			return pArea;
	}

	return nullptr;
}

CStrategicArea* CConquerorSystem::GetStrategicArea(EntityId id, std::vector<EntityId>* excludeAreaIds, bool getOnlyEnabled /*= true*/)
{
	for (const auto pArea : m_strategicAreas)
	{
		if ((pArea && excludeAreaIds && !stl::find(excludeAreaIds, pArea->GetEntityId())) || (pArea && !excludeAreaIds))
		{
			if (pArea->GetEntityId() == id && (pArea->IsEnabled() && getOnlyEnabled))
				return pArea;
		}

	}

	return nullptr;
}

CStrategicArea* CConquerorSystem::GetStrategicArea(ESpeciesType species, std::vector<EntityId>* excludeAreaIds, bool getOnlyEnabled /*= true*/)
{
	for (const auto pArea : m_strategicAreas)
	{
		if (pArea && excludeAreaIds && !stl::find(excludeAreaIds, pArea->GetEntityId()) || (pArea && !excludeAreaIds))
		{
			if (pArea->GetSpecies() == species)
			{
				if (getOnlyEnabled && !pArea->IsEnabled())
					continue;

				return pArea;
			}
				
		}
	}

	return nullptr;
}

int CConquerorSystem::GetHostileAreasCount(ESpeciesType myspecies, EAreaGameStatusFlag gameStatus) const
{
	int count = 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (!pArea)
			continue;

		if (pArea->GetSpecies() != myspecies)
		{
			if (gameStatus == eAGSF_Capturable)
			{
				if (!pArea->IsCapturable())
					continue;
			}
			else if (gameStatus == eAGSF_Enabled)
			{
				if (!pArea->IsEnabled())
					continue;
			}
			else if (gameStatus == eAGSF_EnabledAndCapturable)
			{
				if (!pArea->IsCapturable())
					continue;

				if (!pArea->IsEnabled())
					continue;
			}

			++count;
		}
	}

	return count;
}

int CConquerorSystem::GetHostileAreasCount(ESpeciesType myspecies, const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const
{
	//int count = 0;
	std::vector<EntityId> countedIds;
	const bool needCompareFlags = flags.size() > 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea)
		{
			if (stl::find(countedIds, pArea->GetEntityId()))
				continue;

			if (pArea->GetSpecies() == myspecies)
				continue;

			if (gameStatus == eAGSF_Capturable)
			{
				if (!pArea->IsCapturable())
					continue;
			}
			else if (gameStatus == eAGSF_Enabled)
			{
				if (!pArea->IsEnabled())
					continue;
			}
			else if (gameStatus == eAGSF_EnabledAndCapturable)
			{
				if (!pArea->IsCapturable())
					continue;

				if (!pArea->IsEnabled())
					continue;
			}

			if (needCompareFlags)
			{
				bool notFound = false;

				for (const auto flag : flags)
				{
					if (!pArea->IsHaveFlag(flag))
						notFound = true;
				}

				if (notFound)
					continue;
			}

			countedIds.push_back(pArea->GetEntityId());
			//++count;
		}
	}

	return countedIds.size();
}

int CConquerorSystem::GetStrategicAreaCount(ESpeciesType species, EAreaGameStatusFlag gameStatus) const
{
	int count = 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea)
		{
			if (pArea->GetSpecies() == species)
			{
				if (gameStatus == eAGSF_Capturable)
				{
					if (!pArea->IsCapturable())
						continue;
				}
				else if (gameStatus == eAGSF_Enabled)
				{
					if (!pArea->IsEnabled())
						continue;
				}
				else if (gameStatus == eAGSF_EnabledAndCapturable)
				{
					if (!pArea->IsCapturable())
						continue;

					if (!pArea->IsEnabled())
						continue;
				}

				++count;
			}
		}
	}

	//if (count == 0)
	//{
	//	CryLogAlways("[C++][Count %i of Area with species %i enabled %i capturable %i]",count, species, enabled, capturable);
	//}

	return count;
}

int CConquerorSystem::GetStrategicAreaCount(EAreaGameStatusFlag gameStatus) const
{
	int count = 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea)
		{
			if (gameStatus == eAGSF_Capturable)
			{
				if (!pArea->IsCapturable())
					continue;
			}
			else if (gameStatus == eAGSF_Enabled)
			{
				if (!pArea->IsEnabled())
					continue;
			}
			else if (gameStatus == eAGSF_EnabledAndCapturable)
			{
				if (!pArea->IsCapturable())
					continue;

				if (!pArea->IsEnabled())
					continue;
			}

			++count;
		}
	}

	if (count == 0)
	{
		//CryLogAlways("[C++][Count %i of Area with params: enabled %i capturable %i]", count, enabled, capturable);
	}

	return count;
}

int CConquerorSystem::GetStrategicAreaCount(ESpeciesType species, const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const
{
	//int count = 0;
	std::vector<EntityId> countedIds;
	const bool needCompareFlags = flags.size() > 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea)
		{
			if (stl::find(countedIds, pArea->GetEntityId()))
				continue;

			if (pArea->GetSpecies() != species)
				continue;

			if (gameStatus == eAGSF_Capturable)
			{
				if (!pArea->IsCapturable())
					continue;
			}
			else if (gameStatus == eAGSF_Enabled)
			{
				if (!pArea->IsEnabled())
					continue;
			}
			else if (gameStatus == eAGSF_EnabledAndCapturable)
			{
				if (!pArea->IsCapturable())
					continue;

				if (!pArea->IsEnabled())
					continue;
			}

			if (needCompareFlags)
			{
				bool notFound = false;

				for (const auto flag : flags)
				{
					if (!pArea->IsHaveFlag(flag))
						notFound = true;
				}

				if (notFound)
					continue;
			}

			countedIds.push_back(pArea->GetEntityId());
			//++count;
		}
	}

	return countedIds.size();
}

int CConquerorSystem::GetStrategicAreaCount(const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const
{
	int count = 0;
	std::vector<EntityId> countedIds;
	const bool needCompareFlags = flags.size() > 0;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea)
		{
			if (stl::find(countedIds, pArea->GetEntityId()))
				continue;

			if (gameStatus == eAGSF_Capturable)
			{
				if (!pArea->IsCapturable())
					continue;
			}
			else if (gameStatus == eAGSF_Enabled)
			{
				if (!pArea->IsEnabled())
					continue;
			}
			else if (gameStatus == eAGSF_EnabledAndCapturable)
			{
				if (!pArea->IsCapturable())
					continue;

				if (!pArea->IsEnabled())
					continue;
			}

			if (needCompareFlags)
			{
				bool notFound = false;

				for (const auto flag : flags)
				{
					if (!pArea->IsHaveFlag(flag))
						notFound = true;
				}

				if (notFound)
					continue;
			}

			countedIds.push_back(pArea->GetEntityId());
			++count;
		}
	}

	return count;
}

void CConquerorSystem::OnAreaCaptured(CStrategicArea* pArea, ESpeciesType ownerSpecies)
{
	if (!pArea)
		return;

	if (pArea->CanUnlockClasses(true))
	{
		for (CSpeciesClass* pClass : m_speciesClassesMap[ownerSpecies])
		{
			if (!(pClass->CheckFlag(eSCF_UnlockedByArea)))
				continue;

			pClass->PushFlag(eSCF_UnlockedForPlayer);
		}
	}

	if (pArea->CanUnlockClasses(false))
	{
		for (CSpeciesClass* pClass : m_speciesClassesMap[ownerSpecies])
		{
			if (!(pClass->CheckFlag(eSCF_UnlockedByArea)))
				continue;

			pClass->PushFlag(eSCF_UnlockedForAI);
		}
	}


	if (m_haveBotsSpawned)
	{
		const auto pCommander = GetSpeciesCommander(ownerSpecies);
		if (pCommander)
			pCommander->OnAreaCaptured(pArea);

		if (m_gameStatus != eGS_Battle)
			m_gameStatus = eGS_Battle;
	}

	if (m_LobbyInfo.state == EConquerLobbyState::IN_LOBBY)
	{
		//TClasses classes;
		//GetSpeciesClasses(ownerSpecies, classes);

		//for (auto& i : classes)
		//	HUDLobbyManageClass(ownerSpecies, i.m_name, !ReadClassConditions(ownerSpecies, &i));

		HUDLobbyUpdateSpeciesMenu(true, true);
	}

	if (ownerSpecies == GetClientSpecies())
	{
		if (pArea == GetClientArea())
		{
			UPDATE_POWERSTRUGGLE_BUY_INTERFACE;
			m_animBuyZoneIndicator.SetVisible(true);
		}

		if (m_haveBotsSpawned)
			Script::CallMethod(g_pGame->GetGameRules()->GetEntity()->GetScriptTable(), "BLAlert", 2, "@conq_we_capture_area", ScriptHandle(pArea->GetEntityId()));
	}
}

void CConquerorSystem::OnAreaLost(CStrategicArea* pArea, ESpeciesType oldSpecies)
{
	if (!pArea)
		return;

	const auto pCommander = GetSpeciesCommander(oldSpecies);
	if (!pCommander)
		return;

	pCommander->OnAreaLost(pArea);

	if (pArea->CanUnlockClasses(true))
	{
		for (CSpeciesClass* pClass : m_speciesClassesMap[oldSpecies])
		{
			if (!(pClass->CheckFlag(eSCF_UnlockedByArea)))
				continue;

			pClass->CleanFlag(eSCF_UnlockedForPlayer);
		}
	}

	if (pArea->CanUnlockClasses(false))
	{
		for (CSpeciesClass* pClass : m_speciesClassesMap[oldSpecies])
		{
			if (!(pClass->CheckFlag(eSCF_UnlockedByArea)))
				continue;

			pClass->CleanFlag(eSCF_UnlockedForAI);
		}
	}

	if (pArea->IsQueueCreated() || pArea->GetQueueSize())
	{
		bool copied = false;
		TAreas areas;
		GetStrategicAreas(areas, oldSpecies, eAGSF_Enabled, oldSpecies, eABF_NoMatter, EAreaFlag::SoldierSpawner);
		
		for (const auto pStrategicArea : areas)
		{
			if (pStrategicArea == pArea)
				continue;

			const auto queueSize = pArea->GetQueueSize() + pStrategicArea->GetQueueSize();

			if (pStrategicArea->GetSpawnPointCount() < queueSize)
				continue;

			if (CopyQueue(pArea, pStrategicArea))
			{
				if (!pStrategicArea->IsQueueCreated())
					pStrategicArea->CreateQueue(GetRespawnTime());

				pArea->DeleteQueue();
				copied = true;
				
				CryLogAlways("%s[C++][Copy Queue From %s to %s]",
					STR_RED, pArea->GetEntity()->GetName(), pStrategicArea->GetEntity()->GetName());

				break;
			}
		}

		if (!copied)
		{
			CryLogAlways("%s[C++][ERROR][Your level haven't strategic area who can respawn all soldiers of species!!!]",
				STR_RED);
		}
	}

	if (oldSpecies == GetClientSpecies())
	{
		if (pArea == GetClientArea())
		{
			UPDATE_POWERSTRUGGLE_BUY_INTERFACE;
			m_animBuyZoneIndicator.SetVisible(false);
		}

		if (m_haveBotsSpawned)
			Script::CallMethod(g_pGame->GetGameRules()->GetEntity()->GetScriptTable(), "BLAlert", 2, "@conq_we_lost_area", ScriptHandle(pArea->GetEntityId()));
	}

	if (m_pSelectedClientAreaEntity && m_pSelectedClientAreaEntity->GetId() == pArea->GetEntityId())
		m_pSelectedClientAreaEntity = nullptr;
}

CConquerorChannel* CConquerorSystem::CreateConquerorChannel(IEntity* pEntity, CSpeciesClass& classInfo)
{
	if (!pEntity)
		return nullptr;

	if (IsExistConquerorChannel(pEntity))
		return nullptr;

	const auto pNewChannel = new CConquerorChannel(pEntity, classInfo.GetName());
	if (pNewChannel)
	{
		m_conquerorChannels.push_back(pNewChannel);
		pNewChannel->m_id = m_conquerorChannels.size();

		if (m_isDebugLog)
			CryLogAlways("$3[C++][Conqueror System][Create Conqueror Channel %i][Entity %s][Class %s]",pNewChannel->m_id, pEntity->GetName(), classInfo.GetName());
	}

	return pNewChannel;
}

void CConquerorSystem::RemoveConquerorChannel(IEntity* pEntity)
{
	if (!pEntity)
		return;

	auto it = m_conquerorChannels.begin();
	const auto end = m_conquerorChannels.end();
	for (; it != end; it++)
	{
		auto* pChannel = *it;
		if (pChannel && pChannel->GetEntity() == pEntity)
		{
			m_conquerorChannels.erase(it);
			SAFE_DELETE(*it);

			if (m_isDebugLog)
			{
				CryLogAlways("[CConquerorSystem][CreateAIChannel][Size-1] %i", m_conquerorChannels.size());
				CryLogAlways("[CConquerorSystem][CreateAIChannel][Pointer] %i", (int)pChannel);
			}

			break;
		}
			
	}
}

void CConquerorSystem::RemoveAllChannels()
{
	auto it = m_conquerorChannels.begin();
	const auto end = m_conquerorChannels.end();
	for (; it != end; it++)
	{
		const auto iter = *it;
		const auto pEntity = iter->GetEntity();
		if (pEntity && pEntity->GetId() != g_pGame->GetIGameFramework()->GetClientActorId())
		{
			const auto* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (!pActor)
				continue;

			auto* pInventory = pActor->GetInventory();
			if (pInventory)
				pInventory->RemoveAllItems();

			gEnv->pEntitySystem->RemoveEntity(pEntity->GetId(), false);
		}

		SAFE_DELETE(*it);
	}	

	m_conquerorChannels.clear();
}

bool CConquerorSystem::IsExistConquerorChannel(IEntity* pEntity)
{
	if (!pEntity)
		return false;

	if (GetConquerorChannel(pEntity))
		return true;

	return false;
}

CConquerorChannel* CConquerorSystem::GetClientConquerorChannel()
{
	const auto EntityID = g_pGame->GetIGameFramework()->GetClientActorId();

	return GetConquerorChannel(EntityID);
}

CConquerorChannel* CConquerorSystem::GetConquerorChannel(IEntity* pEntity)
{
	if (!pEntity)
		return nullptr;

	for (const auto pChannel : m_conquerorChannels)
	{
		if (pChannel && pChannel->GetEntity() == pEntity)
			return pChannel;
	}

	return nullptr;
}

CConquerorChannel* CConquerorSystem::GetConquerorChannel(int idx)
{
	return m_conquerorChannels.at(idx);
}

CConquerorChannel* CConquerorSystem::GetConquerorChannel(EntityId entityId)
{
	if (entityId == 0)
		return nullptr;

	for (const auto pChannel : m_conquerorChannels)
	{
		if (pChannel && pChannel->GetEntity() && pChannel->GetEntity()->GetId() == entityId)
			return pChannel;
	}

	return nullptr;
}

CConquerorChannel* CConquerorSystem::GetConquerorChannelById(int id)
{
	for (const auto pChannel : m_conquerorChannels)
	{
		if (pChannel->m_id == id)
			return pChannel;
	}

	return nullptr;
}

ESpeciesType CConquerorSystem::GetSpeciesLobby()
{
	return (ESpeciesType)m_lobbyConfirmedSpeciesIndex;
}

void CConquerorSystem::InitAllowedSpecies()
{
	for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; i++)
	{
		auto species = ESpeciesType(i);

		const auto pArea = GetStrategicArea(species, 0, true);
		if (pArea && pArea->IsEnabled())
		{
			stl::push_back_unique(m_gameAllowedSpecies, species);
			stl::push_back_unique(m_lobbyAllowedSpecies, species);
		}		
	}
}

void CConquerorSystem::InitPlayerClasses(bool forceReload)
{
	if (forceReload)
		RemoveAllClasses();

	const string filePath = "Game/Libs/Conqueror/ConquerorPlayerClasses.xml";

	bool failed = false;

	auto rootNode = GetISystem()->LoadXmlFile(filePath.c_str());
	if (!rootNode)
		failed = true;

	//Factions Node
	auto factionsNode = rootNode->findChild("Factions");
	if (!factionsNode)
		failed = true;
		
	if (failed)
	{
		CryLogAlways("$4[C++][ERROR][Init Player Classes from xml Failed]");
		return;
	}

	for (int i = 0; i < factionsNode->getChildCount(); i++)
	{
		auto factionNode = factionsNode->getChild(i);
		if (!factionNode)
			continue;

		//faction attributes
		int speciesIndex = -1;
		XmlString factionName;
		bool factionEnabled = true;

		factionNode->getAttr("speciesIndex", speciesIndex);
		factionNode->getAttr("name", factionName);
		//factionNode->getAttr("enabled", enabled);

		if (!factionEnabled)
			continue;

		auto speciesType = ESpeciesType(speciesIndex);
		//~faction attributes

		m_speciesCharNameMap[speciesType] = factionName;
		//CryLogAlways("[C++][Add Species %i Name %s]", speciesIndex, name);

		//faction childs
		for (int y = 0; y < factionNode->getChildCount(); y++)
		{
			auto classRootNode = factionNode->getChild(y);
			if (!classRootNode)
				continue;

			CSpeciesClassPtr conquerorClass = new CSpeciesClass;
			//conquerorClass->PushFlag(eSCF_UnlockedForAll);
			conquerorClass->PushFlag(eSCF_UnlockedForAI);
			conquerorClass->PushFlag(eSCF_UnlockedForPlayer);
			//SClassModel classModel;
			//SClassEquipment classEquip;
			//SClassAI classAI;
			//std::set<string> classAbilities;

			//class attributes		
			
			XmlString className;
			auto onlyTP = false;
			auto isHumanMode = false;
			auto isLeader = false;
			auto isAir = false;
			auto classEnabled = true;

			//Be sure to use a XmlString, otherwise crash

			classRootNode->getAttr("name", className);
			classRootNode->getAttr("onlyThirdPerson", onlyTP);
			classRootNode->getAttr("isHumanMode", isHumanMode);
			classRootNode->getAttr("isLeader", isLeader);
			classRootNode->getAttr("isAir", isAir);
			classRootNode->getAttr("enabled", classEnabled);
			//~class attributes

			//class childs
			//for (int x = 0; x < classRootNode->getChildCount(); ++x)
			//{
				auto classModelNode = classRootNode->findChild("Model");
				if (classModelNode)
				{
					for (int z = 0; z < classModelNode->getChildCount(); ++z)
					{
						auto modelParam = classModelNode->getChild(z);
						if (modelParam)
						{
							const string paramName = modelParam->getAttr("name");
							const string paramValue = modelParam->getAttr("value");

							if (paramName == "lobbyAnim")
							{
								conquerorClass->m_model.m_lobbyAnim = paramValue;
							}
							else if (paramName == "char")
							{
								conquerorClass->m_model.m_character = paramValue;
							}
							else if (paramName == "fp3p")
							{
								conquerorClass->m_model.m_fp3p = paramValue;
							}
							else if (paramName == "arms")
							{
								conquerorClass->m_model.m_arms = paramValue;
							}
							else if (paramName == "mat")
							{
								conquerorClass->m_model.m_mat = paramValue;
							}
							else if (paramName == "mat_helmet")
							{
								conquerorClass->m_model.m_helmetMat = paramValue;
							}
							else if (paramName == "mat_arms")
							{
								conquerorClass->m_model.m_armsMat = paramValue;
							}
							else if (paramName == "worldOffset")
							{
								auto worldOffset = Vec3(0, 0, 0);
								if (modelParam->getAttr("value", worldOffset))
								{
									//CryLogAlways("[C++][Species %s][Class %s worldOffset (%1.f,%1.f,%1.f)]",
										/*GetSpeciesName(speciesType),
										name,
										worldOffset.x,
										worldOffset.y,
										worldOffset.z);*/
								}


								conquerorClass->m_model.m_worldOffset = worldOffset;
							}
							else if (paramName == "scale")
							{
								float scale = 0.0f;
								if (modelParam->getAttr("value", scale))
								{
									//CryLogAlways("[C++][Species %s][Class %s Scale %f]",
									//	GetSpeciesName(speciesType), name, classModel.m_scale);

								}
								conquerorClass->m_model.m_scale = scale;
							}
						}
					}				
				}

				auto classEquipmentNode = classRootNode->findChild("Equipment");
				if (classEquipmentNode)
				{
					for (int z = 0; z < classEquipmentNode->getChildCount(); ++z)
					{
						auto modelParam = classEquipmentNode->getChild(z);
						if (modelParam)
						{
							const string paramName = modelParam->getAttr("name");
							const string paramValue = modelParam->getAttr("value");

							//if (paramName == "primaryWeapon")
							//{
							//	classEquip.m_primaryWeapon = paramValue;
							//}
							/*else*/ 
							if (paramName == "equipPack")
							{
								conquerorClass->m_equipment.m_equipPack = paramValue;
							}
						}
					}
				}

				auto classAINode = classRootNode->findChild("AI");
				if (classAINode)
				{
					for (int z = 0; z < classAINode->getChildCount(); ++z)
					{
						auto modelParam = classAINode->getChild(z);
						if (modelParam)
						{
							const string paramName = modelParam->getAttr("name");
							const string paramValue = modelParam->getAttr("value");

							if (paramName == "archetype")
							{
								conquerorClass->m_ai.m_archetype = paramValue;
							}
							else if (paramName == "character")
							{
								conquerorClass->m_ai.m_character = paramValue;
							}
							else if (paramName == "behaviour")
							{
								conquerorClass->m_ai.m_behaviour = paramValue;
							}
						}
					}
				}

				auto classAbilitiesNode = classRootNode->findChild("Abilities");
				if (classAbilitiesNode)
				{
					for (int z = 0; z < classAbilitiesNode->getChildCount(); ++z)
					{
						auto modelParam = classAbilitiesNode->getChild(z);
						if (modelParam)
						{
							const string abilityName = modelParam->getAttr("name");

							if (!abilityName.empty())
							{
								conquerorClass->m_abilities.insert(abilityName);
							}
						}
					}
				}

				auto classConditionsNode = classRootNode->findChild("Conditions");
				if (classConditionsNode)
				{
					for (int z = 0; z < classConditionsNode->getChildCount(); ++z)
					{
						auto conditionNode = classConditionsNode->getChild(z);
						if (conditionNode)
						{
							const string type = conditionNode->getAttr("type");
							const string name = conditionNode->getAttr("name");
							const string relationship = conditionNode->getAttr("relationship");
							const string conditional1 = conditionNode->getAttr("conditional1");
							const string conditional2 = conditionNode->getAttr("conditional2");
							const string oper = conditionNode->getAttr("operator");
							const string svalue = conditionNode->getAttr("strvalue");
							float fvalue = 0; conditionNode->getAttr("fvalue", fvalue);

							SGenericCondition condition;
							condition.m_type = type;
							condition.m_name = name;
							condition.m_relationship = relationship;
							condition.m_conditional1 = conditional1;
							condition.m_conditional2 = conditional2;
							condition.m_operator = oper;
							condition.m_fvalue = fvalue;
							condition.m_svalue = svalue;

							conquerorClass->m_conditions.push_back(condition);

							if (type == "UnlockedByArea")
							{
								conquerorClass->CleanFlag(eSCF_UnlockedForAI);
								conquerorClass->CleanFlag(eSCF_UnlockedForPlayer);
								conquerorClass->PushFlag(eSCF_UnlockedByArea);
								//conquerorClass->m_flags |= eSCF_UnlockedByArea;
								//conquerorClass->m_flags &= ~eSCF_UnlockedForAll;
							}
							else if (type == "UnlockedForPlayer")
							{
								conquerorClass->CleanFlag(eSCF_UnlockedForAI);
								conquerorClass->CleanFlag(eSCF_UnlockedByArea);
								conquerorClass->PushFlag(eSCF_UnlockedForPlayer);
							}
							else if (type == "UnlockedForAI")
							{
								conquerorClass->CleanFlag(eSCF_UnlockedByArea);
								conquerorClass->CleanFlag(eSCF_UnlockedForPlayer);
								conquerorClass->PushFlag(eSCF_UnlockedForAI);
							}
						}
					}
				}
			//}
			//~class childs

			

			conquerorClass->SetName(className.c_str());
			//conquerorClass->SetOnlyThirdPerson(onlyTP);
			//conquerorClass->SetNeedHumanMode(isHumanMode);
			//conquerorClass->SetIsLeader(isLeader);

			if (onlyTP)
				conquerorClass->PushFlag(eSCF_OnlyThirdPerson);
				//conquerorClass->m_flags |= eSCF_OnlyThirdPerson;

			if (isHumanMode)
				conquerorClass->PushFlag(eSCF_NeedHumanMode);
				//conquerorClass->m_flags |= eSCF_NeedHumanMode;

			if (isLeader)
				conquerorClass->PushFlag(eSCF_LeaderClass);
				//conquerorClass->m_flags |= eSCF_LeaderClass;
			//else
			//	conquerorClass->m_flags |= eSCF_NonLeaderClass;

			if (isAir)
				conquerorClass->PushFlag(eSCF_IsAir);
				//conquerorClass->m_flags |= eSCF_IsAir;

			if (className == "Default")
			{
				//The default class should always be derived!!!
				
				//All required lines must be filled in the default class
				//See ConquerorClasses.xml
			
				//conquerorClass->m_abilities = classAbilities;
				//conquerorClass->m_ai = classAI;
				//conquerorClass->m_equipment = classEquip;
				//conquerorClass->m_model = classModel;

				//if (enabled)
					AddConquerorClass(speciesType, conquerorClass, true);
			}
			else
			{
				CSpeciesClass* pDefaultClass = m_speciesDefaultClassesMap[speciesType];

				const string defaultClassName = pDefaultClass->GetName();
				if (!defaultClassName.empty())
				{
					//////////////////////////////////////////////////////////////////////////////
					conquerorClass->m_abilities = !conquerorClass->m_abilities.empty() ? 
						conquerorClass->m_abilities : pDefaultClass->m_abilities;

					//////////////////////////////////////////////////////////////////////////////
					conquerorClass->m_ai.m_archetype = !conquerorClass->m_ai.m_archetype.empty() ?
						conquerorClass->m_ai.m_archetype : pDefaultClass->m_ai.m_archetype;

					conquerorClass->m_ai.m_behaviour = !conquerorClass->m_ai.m_behaviour.empty() ?
						conquerorClass->m_ai.m_behaviour : pDefaultClass->m_ai.m_behaviour;

					conquerorClass->m_ai.m_character = !conquerorClass->m_ai.m_character.empty() ?
						conquerorClass->m_ai.m_character : pDefaultClass->m_ai.m_character;

					//////////////////////////////////////////////////////////////////////////////
					conquerorClass->m_equipment.m_equipPack = !conquerorClass->m_equipment.m_equipPack.empty() ? 
						conquerorClass->m_equipment.m_equipPack : pDefaultClass->m_equipment.m_equipPack;

					//conquerorClass->m_equipment.m_primaryWeapon = !classEquip.m_primaryWeapon.empty() ?
					//	classEquip.m_primaryWeapon : defClass.m_equipment.m_primaryWeapon;

					//////////////////////////////////////////////////////////////////////////////
					conquerorClass->m_model.m_arms = !conquerorClass->m_model.m_arms.empty() ? 
						conquerorClass->m_model.m_arms : pDefaultClass->m_model.m_arms;

					conquerorClass->m_model.m_armsMat = !conquerorClass->m_model.m_armsMat.empty() ?
						conquerorClass->m_model.m_armsMat : pDefaultClass->m_model.m_armsMat;

					conquerorClass->m_model.m_character = !conquerorClass->m_model.m_character.empty() ?
						conquerorClass->m_model.m_character : pDefaultClass->m_model.m_character;

					conquerorClass->m_model.m_fp3p = !conquerorClass->m_model.m_fp3p.empty() ?
						conquerorClass->m_model.m_fp3p : pDefaultClass->m_model.m_fp3p;

					conquerorClass->m_model.m_helmetMat = !conquerorClass->m_model.m_helmetMat.empty() ?
						conquerorClass->m_model.m_helmetMat : pDefaultClass->m_model.m_helmetMat;

					conquerorClass->m_model.m_lobbyAnim = !conquerorClass->m_model.m_lobbyAnim.empty() ?
						conquerorClass->m_model.m_lobbyAnim : pDefaultClass->m_model.m_lobbyAnim;

					conquerorClass->m_model.m_mat = !conquerorClass->m_model.m_mat.empty() ?
						conquerorClass->m_model.m_mat : pDefaultClass->m_model.m_mat;

					conquerorClass->m_model.m_worldOffset = !conquerorClass->m_model.m_worldOffset.IsZero() ?
						conquerorClass->m_model.m_worldOffset : pDefaultClass->m_model.m_worldOffset;

					conquerorClass->m_model.m_scale = conquerorClass->m_model.m_scale > 0.0f ?
						conquerorClass->m_model.m_scale : pDefaultClass->m_model.m_scale;

					//////////////////////////////////////////////////////////////////////////////
					conquerorClass->m_conditions = !conquerorClass->m_conditions.empty() ?
						conquerorClass->m_conditions : pDefaultClass->m_conditions;

					if (classEnabled)
						AddConquerorClass(speciesType, conquerorClass);
				}
			}
		}
		//~faction childs
	}

	//Map Settings Node
	//auto factionsNode = rootNode->findChild("Factions");
	//if (!factionsNode)
	//	return;
}

void CConquerorSystem::InitVehicleClasses(bool forceReload)
{
	if (forceReload)
		m_vehicleClasses.clear();

	const string filePath = "Game/Libs/Conqueror/ConquerorVehicleClasses.xml";

	bool failed = false;

	const auto rootNode = GetISystem()->LoadXmlFile(filePath.c_str());
	if (!rootNode)
		failed = true;

	if (failed)
	{
		CryLogAlways("$4[C++][ERROR][Init Vehicle Classes from xml Failed]");
		return;
	}

	//Air Node
	const auto airNode = rootNode->findChild("Air");
	if (airNode)
	{
		for (int i = 0; i < airNode->getChildCount(); i++)
		{
			auto vehNode = airNode->getChild(i);
			if (!vehNode)
				continue;

			//vehicle attributes
			XmlString name;
			vehNode->getAttr("className", name);

			SVehicleClass vehClass;
			vehClass.name = name;
			vehClass.type = eVCT_Air;

			m_vehicleClasses.push_back(vehClass);

			//CryLogAlways("%s[C++][Define Vehicle Move Class][Vehicle (%s) = Move (%s)]",
				//STR_GREEN, name, "Air");
		}
	}

	//Cars Node
	const auto carsNode = rootNode->findChild("Cars");
	if (carsNode)
	{
		for (int i = 0; i < carsNode->getChildCount(); i++)
		{
			auto vehNode = carsNode->getChild(i);
			if (!vehNode)
				continue;

			//vehicle attributes
			XmlString name;
			vehNode->getAttr("className", name);

			SVehicleClass vehClass;
			vehClass.name = name;
			vehClass.type = eVCT_Cars;

			m_vehicleClasses.push_back(vehClass);

			//CryLogAlways("%s[C++][Define Vehicle Move Class][Vehicle (%s) = Move (%s)]",
				//STR_GREEN, name, "Cars");
		}
	}

	//Paratroopers Land Vehicle Node
	const auto plvNode = rootNode->findChild("ParatroopersLandVehicle");
	if (plvNode)
	{
		for (int i = 0; i < plvNode->getChildCount(); i++)
		{
			auto vehNode = plvNode->getChild(i);
			if (!vehNode)
				continue;

			//vehicle attributes
			XmlString name;
			vehNode->getAttr("className", name);

			SVehicleClass vehClass;
			vehClass.name = name;
			vehClass.type = eVCT_PLV;

			m_vehicleClasses.push_back(vehClass);

			//CryLogAlways("%s[C++][Define Vehicle Move Class][Vehicle (%s) = Move (%s)]",
				//STR_GREEN, name, "PLV");
		}
	}

	//Tanks Node
	const auto tanksNode = rootNode->findChild("Tanks");
	if (tanksNode)
	{
		for (int i = 0; i < tanksNode->getChildCount(); i++)
		{
			auto vehNode = tanksNode->getChild(i);
			if (!vehNode)
				continue;

			//vehicle attributes
			XmlString name;
			vehNode->getAttr("className", name);

			SVehicleClass vehClass;
			vehClass.name = name;
			vehClass.type = eVCT_Tanks;

			m_vehicleClasses.push_back(vehClass);

			//CryLogAlways("%s[C++][Define Vehicle Move Class][Vehicle (%s) = Move (%s)]",
				//STR_GREEN, name, "Tanks");
		}
	}
}

void CConquerorSystem::InitHUD(bool load)
{
	//const auto pHUD = g_pGame->GetHUD();

	if (load)
	{
		m_animConquerorProgress.Load("Libs/UI/HUD_ConquerorProgress.swf", eFD_Center);
		m_animConquerorProgress.SetVisible(false);

		//if (pHUD && !pHUD->m_animBuyMenu.IsLoaded())
			//pHUD->m_animBuyMenu.Load("Libs/UI/HUD_PDA_Buy.gfx", eFD_Right, eFAF_ThisHandler);

		m_animConquerorLobby.Load("Libs/UI/HUD_ConquerorLobby.swf", eFD_Left);
		m_animConquerorLobby.SetVisible(false);
	}
	else
	{
		m_animConquerorProgress.Unload();
		m_animConquerorLobby.Unload();

		//if (pHUD)
			//pHUD->m_animBuyMenu.Unload();
	}


	InitPlayerClasses(true);
}

void CConquerorSystem::InitPowerStruggleHUD()
{
	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		if (!pHUD->m_pHUDScore)
		{
			pHUD->m_pHUDScore = new CHUDScore;
			pHUD->m_hudObjectsList.push_back(pHUD->m_pHUDScore);
		}

		m_pAnimBuyMenu = &pHUD->m_animBuyMenu;
		if (m_pAnimBuyMenu)
		{
			m_pAnimBuyMenu->Load("Libs/UI/HUD_PDA_Buy.gfx", eFD_Right);
			pHUD->SetFlashColor(m_pAnimBuyMenu);
		}

		m_pAnimScoreBoard = &pHUD->m_animScoreBoard;
		if (m_pAnimScoreBoard)
		{
			m_pAnimScoreBoard->Load("Libs/UI/HUD_MultiplayerScoreboard_TDM.swf");
			pHUD->SetFlashColor(m_pAnimScoreBoard);
		}

		m_pAnimPlayerPP = &pHUD->m_animPlayerPP;
		if (m_pAnimPlayerPP)
		{
			m_pAnimPlayerPP->Load("Libs/UI/HUD_MP_PPoints.swf", eFD_Right);
			pHUD->SetFlashColor(m_pAnimPlayerPP);
		}

		m_pAnimPlayerPPChange = &pHUD->m_animPlayerPPChange;
		if (m_pAnimPlayerPPChange)
		{
			m_pAnimPlayerPPChange->Load("Libs/UI/HUD_MP_PPoints_Log.swf", eFD_Right, eFAF_Visible);
			pHUD->SetFlashColor(m_pAnimPlayerPPChange);
		}

		m_pAnimMPMessages = &pHUD->m_animMPMessages;
		if (m_pAnimMPMessages)
		{
			m_pAnimMPMessages->Load("Libs/UI/HUD_MP_Messages.gfx", eFD_Center, eFAF_Visible);
			pHUD->SetFlashColor(m_pAnimMPMessages);
		}

		m_pAnimBattleLog = &pHUD->m_animBattleLog;
		if (m_pAnimBattleLog)
		{
			m_pAnimBattleLog->Load("Libs/UI/HUD_MP_Log.gfx", eFD_Right);
			pHUD->SetFlashColor(m_pAnimBattleLog);
		}

		m_pAnimKillLog = &pHUD->m_animKillLog;
		if (m_pAnimKillLog)
		{
			m_pAnimKillLog->Load("Libs/UI/HUD_KillLog.swf", eFD_Right, eFAF_Visible);
			pHUD->SetFlashColor(m_pAnimKillLog);
		}

		m_animSwingOMeter.Load("Libs/UI/HUD_ConquerorSwingOMeter.swf", eFD_Center, eFAF_Visible);
		m_animSwingOMeter.GetFlashPlayer()->SetVisible(true);
		pHUD->SetFlashColor(&m_animSwingOMeter);

		m_animRespawmTimer.Load("Libs/UI/HUD_MP_SpawnCircle.swf", eFD_Center, eFAF_Visible);
		pHUD->SetFlashColor(&m_animRespawmTimer);

		m_animBuyZoneIndicator.Load("Libs/UI/HUD_BuyZoneIndicator.swf", eFD_Left, eFAF_Visible);
		m_animBuyZoneIndicator.SetVisible(false);
		pHUD->SetFlashColor(&m_animBuyZoneIndicator);
	}
}

void CConquerorSystem::InitGamemodeFromFG(SConquerLobbyInfo& info)
{
	if (!info.isConquest || info.state != EConquerLobbyState::IN_LOBBY)
		return;

	const auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	if (g_pGame->GetIGameFramework()->IsEditing())
		return;

	//if (g_pGame->GetHUD()->m_pHUDPowerStruggle)
	//{
		//g_pGame->GetHUD()->m_pHUDPowerStruggle = new CHUDPowerStruggle(g_pGame->GetHUD(), &g_pGame->GetHUD()->m_animBuyMenu, &g_pGame->GetHUD()->m_animHexIcons);
		//g_pGame->GetHUD()->m_hudObjectsList.push_back(g_pGame->GetHUD()->m_pHUDPowerStruggle);
	//}

	InitVehicleClasses(true);
	InitAllowedSpecies();

	if (m_lobbyAllowedSpecies.size() == 0)
	{
		CryLogAlways("%s[C++][Conqueror Start Error][Cause: Not Defined Allowed Species]", STR_RED);
		return;
	}

	GameOver(false);

	if (gEnv->bEditor)
	{
		char* levelPathName = "";
		g_pGame->GetIGameFramework()->GetEditorLevel(&levelPathName, 0);

		string mapName = levelPathName;
		int slashPos = mapName.rfind('\\');
		if (slashPos == -1)
			slashPos = mapName.rfind('/');
		mapName = mapName.substr(slashPos + 1, mapName.length() - slashPos);

		InitConquerorAICountInfo(true, mapName);
		InitConquerorStrategies(true, mapName);
	}
	else
	{
		const auto pLevel = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel();
		if (pLevel)
		{
			const auto pInfo = pLevel->GetLevelInfo();
			if (pInfo)
			{
				const string rootName = pInfo->GetPath();
				string mapName = rootName;
				int slashPos = mapName.rfind('\\');
				if (slashPos == -1)
					slashPos = mapName.rfind('/');
				mapName = mapName.substr(slashPos + 1, mapName.length() - slashPos);

				InitConquerorAICountInfo(true, mapName.c_str());
				InitConquerorStrategies(true, mapName.c_str());

			}
		}
	}

	//Remove species with 0 count of reinforcements
	for (auto species : m_gameAllowedSpecies)
	{
		const auto reinforcements = GetSpeciesReinforcements(species);
		if (reinforcements <= 0)
		{
			if (stl::find_and_erase(m_lobbyAllowedSpecies, species))
			{
				if (g_pGameCVars->conq_debug_log)
					CryLogAlways("$8[C++][WARNING][%s Species has Removed from lobby because its reinforcements equal zero]",
						GetSpeciesName(species));
 			}
		}
	}

	InitPowerStruggleHUD();

	//Setup gameplay CVars
	m_CVarBotsJoinBeforePlayer = max(0, g_pGameCVars->conq_botsJoinBeforePlayer);//1
	m_CVarRespawnTime = max(0, g_pGameCVars->conq_respawnTime);//15
	m_CVarTimeLimit = max(0.0f, g_pGameCVars->conq_limitTime);
	SetSpeciesChangeLimit(max(0, g_pGameCVars->conq_teamChangeLimit));
	//m_CVarSpeciesChangeLimit = max(0, g_pGameCVars->conq_teamChangeLimit);
	pGameRules->ResetGameTime();

	//Create client player channel
	const auto* pLocalActor = g_pGame->GetIGameFramework()->GetClientActor();
	if (pLocalActor)
		CreateConquerorChannel(pLocalActor->GetEntity(), *GetDefaultClass());

	//Init Lobby Classes and Factions
	HUDLobbyUpdateSpeciesMenu(false, false);

	//Humans, unlike aliens, should have a random class
	TClassesPtr randomClasses;

	//Init AI channels and squads
	for (auto allowedSpeciesType : m_gameAllowedSpecies)
	{
		int squadsCount = m_pXMLAICountInfo->GetSquadsCount(allowedSpeciesType);
		const int unitsCount = m_pXMLAICountInfo->GetUnitsCount(allowedSpeciesType);

		if (squadsCount == 0 && unitsCount == 0)
			continue;

		if (squadsCount == 0 && unitsCount > 0)
			squadsCount = 1;

		//Create squads
		for (int i = 0; i < squadsCount; i++)
			CreateSquadFromSpecies(allowedSpeciesType);

		//Create ai squad leaders
		for (int i = 0; i < squadsCount; i++)
		{
			//GetSpeciesRandomLeaderClasses(allowedSpeciesType, squadsCount, randomClasses);
			const uint flags = eSCF_LeaderClass | eSCF_UnlockedForAI;
			GetSpeciesRandomClasses(allowedSpeciesType, flags, squadsCount, randomClasses);

			CSpeciesClass* aiClassInfo = randomClasses[i];

			const string soldierSuffix = " Leader";
			const string aiName = GetSpeciesName(allowedSpeciesType) + soldierSuffix;

			if (CreateAIEntity(aiClassInfo, aiName, allowedSpeciesType))
				++m_speciesLeadersCountMap[allowedSpeciesType];
		}

		//Create ai squad members
		for (int i = 0; i < unitsCount - m_speciesLeadersCountMap[allowedSpeciesType]; i++)
		{
			//GetSpeciesRandomClasses(allowedSpeciesType, unitsCount - m_speciesLeadersCountMap[allowedSpeciesType], true, randomClasses);
			//const uint flags = eSCF_NonLeaderClass | eSCF_UnlockedForAI;
			const uint flags = eSCF_UnlockedForAI;
			GetSpeciesRandomClasses(allowedSpeciesType, flags, unitsCount - m_speciesLeadersCountMap[allowedSpeciesType], randomClasses);

			if (randomClasses.size() > 0)
			{
				CSpeciesClass* aiClassInfo = randomClasses[i];

				const string soldierSuffix = " Soldier";
				const string aiName = GetSpeciesName(allowedSpeciesType) + soldierSuffix;

				CreateAIEntity(aiClassInfo, aiName, allowedSpeciesType);
			}
			else
			{
				CryLogAlways("$4[C++][ERROR][Can't Create AI Entity][Cause: Classes not found (size = 0)]");
			}
		}
	}

	for (const auto allowedSpeciesType : m_gameAllowedSpecies)
	{
		//Create ai commanders must be after creating a squads
		CreateAICommander(allowedSpeciesType);
	}

	SetLobbyInfo(info);
	OnLobbySetTeam(m_lobbyAllowedSpecies[0], 0);

	const auto soldiers = GetSpeciesReinforcements(GetClientSpecies());
	if (soldiers > 0)
		g_pGame->GetHUD()->DisplayBigOverlayFlashMessage("Press M to Select Spawn Point", 15.0f, 400, 450);

	//Swing O Meter Initialization
	//UpdateHUDSOM(true);
}

void CConquerorSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_speciesClassesMap);
	s->AddContainer(m_speciesLeadersCountMap);
	//s->AddContainer(m_areaSpawnedLeadersCountMap);
	s->AddContainer(m_speciesCharNameMap);
	s->AddContainer(m_gameAllowedSpecies);
	s->AddContainer(m_lobbyAllowedSpecies);
	s->AddContainer(m_speciesDefaultClassesMap);
	s->AddContainer(m_conquerorChannels);
	s->AddContainer(m_strategicAreas);
	s->AddContainer(m_vehicleSpawners);
	s->AddContainer(m_vehicleClasses);
	s->AddContainer(m_speciesAutoDestroyTime);

	for (auto iter = m_speciesClassesMap.begin(); iter != m_speciesClassesMap.end(); ++iter)
		s->AddContainer(iter->second);

	for (auto iter = m_speciesCharNameMap.begin(); iter != m_speciesCharNameMap.end(); ++iter)
		s->Add(iter->second);

	for (auto iter = m_conquerorChannels.begin(); iter != m_conquerorChannels.end(); ++iter)
	{
		const auto ite = *iter; 
		ite->GetMemoryStatistics(s);
	}

}

void CConquerorSystem::Update(float frametime)
{
	m_isDebugLog = g_pGameCVars->conq_debug_log == 1;

	const auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	if (gEnv->pSystem->IsEditorMode())
		return;

	if (m_pRARSystem)
		m_pRARSystem->Update(frametime);

	if (!IsGamemode())
		return;
	
	for (auto& preRespawnPair : m_preRespawns)
	{
		if (preRespawnPair.second > 0)
			preRespawnPair.second -= frametime;

		if (preRespawnPair.second <= 0)
		{
			preRespawnPair.second = 0;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(preRespawnPair.first.actorId);
			const auto pArea = GetStrategicArea(preRespawnPair.first.areaId, 0, true);

			if (!pActor || !pArea)
				continue;

			const auto pChannel = GetConquerorChannel(pActor->GetEntity());
			if (!pChannel)
				continue;

			if (!pArea->GetBookedSpawnPoint(pActor))
			{
				const auto isAir = pChannel->GetClass()->IsAirClass();
				pArea->BookFreeSpawnPoint(pActor, isAir);
			}

			pChannel->SetSelectedStrategicArea(pArea);

			if (!pArea->IsQueueCreated())
				pArea->CreateQueue(GetRespawnTime());

			pArea->AddToQueue(pActor->GetEntityId(), preRespawnPair.first.event);
			m_preRespawns.erase(preRespawnPair.first);
			break;
		}
	}

	for (const auto pChannel : m_conquerorChannels)
	{
		const auto pActor = static_cast<CActor*>(pChannel->GetActor());
		if (pActor && pActor->GetHealth() > 0)
		{
			const auto pActAI = pActor->GetEntity()->GetAI();
			const auto pVehicle = TOS_Vehicle::GetVehicle(pActor);
			const char* actorBehav = TOS_AI::GetCurrentAIBehaviour(pActAI);

			if (!pVehicle && actorBehav)
			{
				if (strcmp(actorBehav, "InVehicle") == 0 || 
					strcmp(actorBehav, "InVehicleAlerted") == 0)
				{
					CryLogAlways("%s[C++][Detect and fix InVehicle behaviour without vehicle][Victim: %s]",
						STR_RED, pActAI->GetName());

					TOS_AI::SendSignal(pActAI, SIGNALFILTER_SENDER, "TO_IDLE", pActor->GetEntity(), 0);
				}
			}
			else if (pVehicle)
			{
				const auto pVehAI = pVehicle->GetEntity()->GetAI();
				const char* vehBehav = TOS_AI::GetCurrentAIBehaviour(pVehAI);

				if (vehBehav && strcmp(vehBehav, "VtolFlyTOS") == 0)
				{
					int heliTimer4 = 0;
					TOS_Script::GetEntityScriptValue(pVehAI->GetEntity(), "AI", "heliTimer4", heliTimer4);

					if (heliTimer4 == 0 && !TOS_Vehicle::IsHavePassengers(pVehicle))
					{
						CryLogAlways("%s[C++][Detect and fix VtolFlyTOS behaviour without fly target and passengers][Victim: %s]",
							STR_RED, pVehAI->GetName());

						TOS_AI::SendSignal(pVehAI, SIGNALFILTER_SENDER, "TO_HELI_IDLE", pVehicle->GetEntity(), 0);
					}
				}
			}
		}
	}

	const auto clientSpecies = GetClientSpecies();
	const auto pClientChannel = GetClientConquerorChannel();

	for (auto species : m_gameAllowedSpecies)
	{
		const int reinforcements = GetSpeciesReinforcements(species);

		if (species != clientSpecies)
		{
			//CryLogAlways("%s: %i, %i", GetSpeciesName(species), reinforcements, teammates.size());
			
			if (IsSpeciesAllowed(species, true))
			{
				if (m_haveBotsSpawned)
				{
					const auto count = GetStrategicAreaCount(species, eAGSF_EnabledAndCapturable);
					if (reinforcements <= 0 && count == 0)
						m_speciesAutoDestroyTime[species] += frametime;
					else
						m_speciesAutoDestroyTime[species] = 0;

					if (m_speciesAutoDestroyTime[species] >= 3)
						ForceKillSpecies(species);
				}

				std::vector<EntityId> teammates;
				GetSpeciesTeammates(species, teammates, true);

				if (reinforcements == 0 && (teammates.size() == 0))
				{
					OnSpeciesDestroyed(species);

					auto iter = m_speciesAutoDestroyTime.find(species);
					if (iter != m_speciesAutoDestroyTime.end())
						m_speciesAutoDestroyTime.erase(iter);
				}
			}
		}
		else
		{
			if (reinforcements == 0 && pClientChannel->GetState() != eCCS_Alive)
			{
				if (IsSpeciesAllowed(species, true))
					OnSpeciesDestroyed(species);
			}
		}
	}

	SetFriendAreaPointsCount(GetStrategicAreaCount(GetClientSpecies(), eAGSF_EnabledAndCapturable));

	for (const auto pChannel : m_conquerorChannels)
	{
		if (pChannel)
		{
			pChannel->Update(frametime);

			if (!pChannel->GetClass())
				continue;

			//Debug
			if (g_pGameCVars->conq_debug_draw_aichannel > 0)
			{
				static float color[] = { 1,1,1,1 };
				const auto size = 1.1f;
				const auto scale = 20;
				const auto xoffset = TOS_Debug::XOFFSET_COMMON;
				const auto yoffset = TOS_Debug::YOFFSET_CONQ_CHANNELS;

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, 1.3f, color, false,
					"Conqueror channels: ");

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + pChannel->m_id * scale, size, color, false,
					"Channel %i, Class %s, isLeader %i, Species %i, State %i",
					pChannel->m_id, pChannel->GetClass()->GetName(), pChannel->GetClass()->IsLeaderClass(), (int)pChannel->GetSpecies(), (int)pChannel->m_state);
			}
		}	
	}

	const auto pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
	if (pClientActor)
	{
		const int clientPoints = GetActorPoints(pClientActor->GetEntity());
		if (m_lastClientPointsSet != clientPoints)
		{
			const int diff = clientPoints - m_lastClientPointsSet;

			m_pAnimPlayerPPChange->Invoke("addLog", diff);
			m_pAnimPlayerPP->Invoke("setPPoints", clientPoints);
			m_lastClientPointsSet = clientPoints;
		}

		if (m_pClientArea && m_pClientArea->IsCapturable() && m_pClientArea->IsEnabled())
		{
			if (g_pGameCVars->conq_debug_draw_client_area)
			{
				static float color[] = { 1,1,1,1 };
				const auto size = TOS_Debug::SIZE_COMMON;
				const auto scale = 20;
				const auto xoffset = TOS_Debug::XOFFSET_COMMON+300;
				const auto yoffset = TOS_Debug::YOFFSET_CLIENTAREA;

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - scale, TOS_Debug::SIZE_HEADER, color, false,
					"Client area: ");

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + 0 * scale, TOS_Debug::SIZE_HEADER, color, false,
					"%s: exclusive: %i, progress: %1.f, units: %i", GetSpeciesName(eST_USA), m_pClientArea->IsSpeciesExclusive(eST_USA), m_pClientArea->GetCaptureProgress(eST_USA), m_pClientArea->GetAgentCount(eST_USA));
				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + 1 * scale, TOS_Debug::SIZE_HEADER, color, false,
					"%s: exclusive: %i, progress: %1.f, units: %i", GetSpeciesName(eST_Aliens), m_pClientArea->IsSpeciesExclusive(eST_Aliens), m_pClientArea->GetCaptureProgress(eST_Aliens), m_pClientArea->GetAgentCount(eST_Aliens));
				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + 2 * scale, TOS_Debug::SIZE_HEADER, color, false,
					"%s: exclusive: %i, progress: %1.f, units: %i", GetSpeciesName(eST_NK), m_pClientArea->IsSpeciesExclusive(eST_NK), m_pClientArea->GetCaptureProgress(eST_NK), m_pClientArea->GetAgentCount(eST_NK));
				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + 3 * scale, TOS_Debug::SIZE_HEADER, color, false,
					"%s: exclusive: %i, progress: %1.f, units: %i", GetSpeciesName(eST_CELL), m_pClientArea->IsSpeciesExclusive(eST_CELL), m_pClientArea->GetCaptureProgress(eST_CELL), m_pClientArea->GetAgentCount(eST_CELL));

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + 4 * scale, size, color, false,
					"Capture State: %i, Actors Inside: %i, Capturable: %i, Enable: %i, Capturing Species: %s",
					m_pClientArea->m_CaptureState, m_pClientArea->m_ActorsInside.size(), m_pClientArea->m_bIsCapturable, m_pClientArea->m_bIsEnabled, GetSpeciesName(m_pClientArea->GetCapturingSpecies()));
			}

			auto playerSpecies = eST_NEUTRAL;
			const auto areaSpecies = m_pClientArea->GetSpecies();
			const auto areaCapturingSpecies = m_pClientArea->GetCapturingSpecies();

			const auto pClientActor = g_pControlSystem->GetClientActor();

			if (const auto pAI = pClientActor->GetEntity()->GetAI())
			{
				auto& playerParams = pAI->CastToIAIActor()->GetParameters();
				playerSpecies = (ESpeciesType)playerParams.m_nSpecies;
			}

			if (playerSpecies != eST_NEUTRAL)
			{
				const auto percentConverver = 100.0f / m_pClientArea->GetCaptureTime();
				const auto playerCaptureProgress = m_pClientArea->GetCaptureProgress(playerSpecies) * percentConverver;
				const auto enemyCaptureProgress = m_pClientArea->GetEnemyCaptureProgress(playerSpecies) * percentConverver;
				auto currentProgress = 0.0f;

				const auto state = m_pClientArea->GetCaptureState();

				if (state == ECaptureState::CAPTURED || state == ECaptureState::UNCAPTURING)
				{
					if (playerSpecies != areaSpecies)
					{
						m_animConquerorProgress.Invoke("applyEnemyColor");
						currentProgress = enemyCaptureProgress;
					}
					else
					{
						m_animConquerorProgress.Invoke("applyFriendColor");
						currentProgress = playerCaptureProgress;
					}
				}
				else if (state == ECaptureState::NOTCAPTURED || state == ECaptureState::CAPTURING)
				{
					if (areaSpecies == eST_NEUTRAL)
					{
						m_animConquerorProgress.Invoke("applyFriendColor");
						currentProgress = playerCaptureProgress;
					}
				}
				else if (state == ECaptureState::CONTESTED)
				{
					m_animConquerorProgress.Invoke("applyNeutralColor");

					float maxProgress = 0.0f;
					ESpeciesType maxSpecies = eST_NEUTRAL;

					if (areaSpecies == playerSpecies)
						currentProgress = playerCaptureProgress;
					else
					{
						m_pClientArea->GetMaxCaptureProgress(maxProgress, maxSpecies);
						if (maxSpecies == playerSpecies)
							currentProgress = playerCaptureProgress;
						else
							currentProgress = maxProgress * percentConverver;
					}
				}

				const auto finalProgress = currentProgress;
				m_animConquerorProgress.Invoke("setCaptureProgress", (int)finalProgress);
			}
		}

	}

	for (int i = 0; i < m_speciesCommanders.size(); i++)
	{
		CConquerorCommander* pCommander = m_speciesCommanders[i];
		if (!pCommander)
			continue;

		std::vector<EntityId> teammates; 
		GetSpeciesTeammates(pCommander->GetSpecies(), teammates, true);

		const float curTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

		if (m_haveBotsSpawned && /*teammates.size() > 0 &&*/ curTime - m_botsSpawnedTime > 0.5f)
			pCommander->Update(frametime);

		//Debug
		if (g_pGameCVars->conq_debug_draw_commanders > 0)
		{
			static float color[] = { 1,1,1,1 };
			const auto size = 1.1f;
			const auto scale = 15;
			const auto xoffset = TOS_Debug::XOFFSET_COMMON;
			const auto yoffset = TOS_Debug::YOFFSET_CONQ_CMDRS;

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset - 20, 1.3f, color, false,
				"Conqueror commanders: ");

			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset-40, size, color, false, 
				"Game Status %i", m_gameStatus);

			const float lastTimeStrategyChange = gEnv->pTimer->GetFrameStartTime().GetSeconds() - pCommander->m_lastTimeStrategyChange;

			if (pCommander->GetCurrentStrategy())
			{
				//const char* lastStrategyName = pCommander.m_pLastStrategy->GetName();

				const auto curStrategyGoals = pCommander->GetCurrentStrategyGoals(eSGT_CapturedAreasCount);
				const char* curStrategyName = pCommander->GetCurrentStrategy()->m_name.c_str();
				const float curStrategyTimelimit = pCommander->GetCurrentStrategy()->GetSettings().m_timeLimit;
				const float timelimit = pCommander->m_currentStrategyTimeLimit;

				//CryLogAlways("Commander Species %s, Squads Count %i, Timelimit %1.f (%1.f), Current Strategy %s",
				//	GetSpeciesName(pCommander.m_species), pCommander.m_subordinateSquadIds.size(), curStrategyTimelimit, timelimit, curStrategyName);

				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + i * scale, size, color, false,
					"Commander Species %s, Squads Count %i, Timelimit %1.f (%1.f), Goals %i, Current Strategy %s, Last Time Strategy Change %1.f",
					GetSpeciesName(pCommander->m_species), pCommander->m_subordinateSquadIds.size(), curStrategyTimelimit, timelimit, curStrategyGoals, curStrategyName, lastTimeStrategyChange);
			}
			else
			{
				gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + i * scale, size, color, false,
					"Commander Species %s, Squads Count %i, Timelimit NULL (NULL), Current Strategy NULL, Last Time Strategy Change %1.f",
					GetSpeciesName(pCommander->m_species), pCommander->m_subordinateSquadIds.size(), lastTimeStrategyChange);
			}
		}
	}

	//for (int i = 0; i < m_strategicAreas.size(); i++)
	//{
	//	auto pArea = m_strategicAreas[i];
	//	if (!pArea)
	//		continue;

	//	const float dist = (pArea->GetEntity()->GetWorldPos() - g_pControlSystem->GetClientActor()->GetEntity()->GetWorldPos()).GetLength();
	//	if (dist > 15)
	//		continue;

	//	//Debug
	//	if (true)
	//	{
	//		static float color[] = { 1,1,1,1 };
	//		const auto size = 1.2f;
	//		const auto scale = 15;
	//		const auto xoffset = 300;
	//		const auto yoffset = 150;

	//		for (int k = 0; k < pArea->m_queue.respawns.size(); k++)
	//		{
	//			auto& info = pArea->m_queue.respawns[k];
	//			auto pEntity = gEnv->pEntitySystem->GetEntity(info.entityId);
	//			if (!pEntity)
	//				continue;

	//			gEnv->pRenderer->Draw2dLabel(xoffset, yoffset + k * scale, size, color, false,
	//				"Area %s, Queue: element: %i, name: %s",
	//				pArea->GetEntity()->GetName(), k, pEntity->GetName());
	//		}
	//	}
	//}

	//Reset disabled buttoms when change the faction

	//Update class enabling
	//Check class enable condition
	//if condition is true then enable buttom of the class
	//else condition is false then disable buttom of the class
}

void CConquerorSystem::Serialize(TSerialize ser)
{

}

//void CConquerorSystem::OnPointsAdded(CConquerorChannel* pChannel, int value)
//{
//	if (pChannel && pChannel->IsClient())
//	{
//
//	}
//}

float CConquerorSystem::GetRespawnTime() const
{
	return m_haveBotsSpawned ? m_CVarRespawnTime : 0.1f;
	//return 3.0f;
}

void CConquerorSystem::ShowRespawnCycle(bool show)
{
	if (m_animRespawmTimer.IsLoaded())
	{
		m_animRespawmTimer.SetVisible(show);
	}	
}

void CConquerorSystem::SetRespawnCycleRemainingTime(int respawnTime, float remainingSeconds)
{
	if (m_animRespawmTimer.IsLoaded())
	{
		const SFlashVarValue args[2] = { respawnTime, remainingSeconds };
		m_animRespawmTimer.Invoke("setSeconds", args, 2);
	}
}

ESpeciesType CConquerorSystem::GetClientSpecies()
{
	//const auto pLocalCC = g_pControlSystem->GetLocalControlClient();

	//auto pClientActor = pLocalCC ? pLocalCC->GetControlledActor();
	//if (!pClientActor)
	//	pClientActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	const auto pClientActor = g_pControlSystem->GetClientActor();

	auto playerSpecies = eST_NEUTRAL;

	if (pClientActor)
	{
		if (const auto pAI = pClientActor->GetEntity()->GetAI())
		{
			const auto& playerParams = pAI->CastToIAIActor()->GetParameters();
			playerSpecies = (ESpeciesType)playerParams.m_nSpecies;
		}
	}

	return playerSpecies;
}

IEntity* CConquerorSystem::GetClientAreaEntity()
{
	return m_pSelectedClientAreaEntity;
}

void CConquerorSystem::SetClientAreaEntity(IEntity* pEntity)
{
	const auto pDude = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pDude)
		return;

	auto wasInQueue = false;
	const auto nowIsDead = pDude->GetHealth() < 0;

	m_pSelectedClientAreaEntity = pEntity;
	//CryLogAlways("[C++][Player Select %s]", pEntity->GetName());

	if (IsInQueue(pDude))
	{
		RemoveFromQueue(pDude);
		wasInQueue = true;
	}

	const auto pArea = GetStrategicArea(m_pSelectedClientAreaEntity->GetId(), 0);
	if (pArea && (wasInQueue || nowIsDead))
	{
		RegisterToRespawn(pDude, pArea, ERespawnEvent::eRC_OnKilledRespawn);
	}
}

SConquerLobbyInfo& CConquerorSystem::GetLobbyInfo()
{
	return m_LobbyInfo;
}

void CConquerorSystem::SetLobbyInfo(SConquerLobbyInfo& info)
{
	m_LobbyInfo = info;

	OnLobbySetInfo(info);
}

void CConquerorSystem::OnCmdJoinGame(IActor* pDude)
{
	if (!pDude)
		return;

	if (m_LobbyInfo.state != EConquerLobbyState::IN_LOBBY)
		return;

	auto* pChannel = GetConquerorChannel(pDude->GetEntity());
	if (pChannel)
	{
		if (!CanJoinGame())
		{
			g_pGame->GetHUD()->DisplayOverlayFlashMessage("@conq_not_enougth_reinforcements", ColorF(1.0f,0.0f,0.0f));
			return;
		}

		if (m_lobbyConfirmedSpeciesIndex != m_lobbySelectedSpeciesIndex)
		{
			if (m_lobbyConfirmedSpeciesIndex != -1 && m_haveBotsSpawned)
			{
				OnChangeConfirmedLobbySpecies();
				//CryLogAlways("[C++][Change confirmed lobby species from %s to %s]", GetSpeciesName(ESpeciesType(m_lobbyConfirmedSpeciesIndex)), GetSpeciesName(ESpeciesType(m_lobbySelectedSpeciesIndex)));
			}
				

			m_lobbyConfirmedSpeciesIndex = m_lobbySelectedSpeciesIndex;
		}

		if (m_lobbyConfirmedClassIndex != m_lobbySelectedClassIndex)
			m_lobbyConfirmedClassIndex = m_lobbySelectedClassIndex;

		pChannel->SetClass(GetClientSelectedClass());

		//Send actorId to strategic area's queue here
		if (m_pSelectedClientAreaEntity == nullptr)
		{
			//Select random friendly area
			CStrategicArea* pType = nullptr;
			TAreas areas;
			GetStrategicAreas(areas, GetClientSpecies(), eAGSF_Enabled, GetClientSpecies(), eABF_NoMatter, EAreaFlag::SoldierSpawner);

			const auto pArea = TOS_STL::GetRandomFromSTL<TAreas, CStrategicArea*>(areas);
			if (pArea)
				m_pSelectedClientAreaEntity = pArea->GetEntity();
		}

		if (!m_pSelectedClientAreaEntity)
		{
			CryLogAlways("%s[C++][ERROR][On Join Game: Can not define player area to spawn]");
		}

		if (const auto pArea = GetStrategicArea(m_pSelectedClientAreaEntity->GetId(), 0))
		{
			if (!pArea->IsBookedSpawnPoint(pDude->GetEntityId()))
				pArea->BookFreeSpawnPoint(pDude, pChannel->GetClass()->IsAirClass());
			
			pChannel->SetSelectedStrategicArea(pArea);

			if (IsInQueue(pDude))
				RemoveFromQueue(pDude);

			if (!pArea->IsQueueCreated())
				pArea->CreateQueue(GetRespawnTime());

			pArea->AddToQueue(pDude->GetEntityId(), eRC_OnClassChangedRespawn);
		}
	}
}

void CConquerorSystem::OnCmdSpectator(IActor* pActor)
{
	SConquerLobbyInfo info = m_LobbyInfo;
	info.state = EConquerLobbyState::IN_LOBBY;

	SetLobbyInfo(info);
}

void CConquerorSystem::OnGameRulesReset()
{
	RemoveAllChannels();

	m_LobbyInfo = SConquerLobbyInfo();

	m_pSelectedClientAreaEntity = nullptr;
	m_gameStatus = eGS_GameStart;
	m_haveBotsSpawned = 0;
	m_usedAlienId = 0;
	m_speciesSwitchCount = 0;
	//m_playerSpawnedCount = 0;

	GameOver(false);

	m_winnerSpecies = eST_NEUTRAL;
	m_vehiclesCanBookUnloadSpots.clear();
	m_strategies.clear();
	m_preRespawns.clear();
	m_speciesCommanders.clear();
	m_speciesFlagIndexMap.clear();
	//m_areaSpawnedLeadersCountMap.clear();
	m_gameAllowedSpecies.clear();
	m_lobbyAllowedSpecies.clear();

	if (m_pRARSystem)
		m_pRARSystem->Reset();

	for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; ++i)
		m_speciesLeadersCountMap[ESpeciesType(i)] = 0;

	for (const auto pArea : m_strategicAreas)
		pArea->Reset();

	//if (g_pGame->GetHUD() && g_pGame->GetHUD()->m_pHUDPowerStruggle)
	//{
		//stl::find_and_erase(g_pGame->GetHUD()->m_hudObjectsList, g_pGame->GetHUD()->m_pHUDPowerStruggle);
		//SAFE_DELETE(g_pGame->GetHUD()->m_pHUDPowerStruggle);
	//}

	//for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; ++i)
	//	m_speciesFlagIndexMap[ESpeciesType(i)] = -1;

	//CryLogAlways("[C++][CConquerorSystem][OnGameRulesReset]");
}

void CConquerorSystem::SetPlayerModel(IActor* pActor, const SClassModel& info)
{
	if (!pActor)
		return;

	CPlayer* pPlayer = static_cast<CPlayer*>(pActor);
	pPlayer->TurnOFFHumanMode();
	pPlayer->TurnOFFOnlyThirdPerson();

	const auto pEntity = pPlayer->GetEntity();
	if (pEntity)
	{
		const auto pScriptProxy = static_cast<IEntityScriptProxy*>(pEntity->GetProxy(ENTITY_PROXY_SCRIPT));

		const auto pEntityTable = pScriptProxy->GetScriptTable();
		if (pEntityTable)
		{
			SmartScriptTable pEntityProperties;
			if (pEntityTable->GetValue("Properties", pEntityProperties))
			{
				pEntityProperties->SetValue("fileModel", (ScriptAnyValue)info.m_character);
				pEntityProperties->SetValue("clientFileModel", (ScriptAnyValue)info.m_fp3p);
				pEntityProperties->SetValue("fpItemHandsModel", (ScriptAnyValue)info.m_arms);


			//	auto pPlayer = static_cast<CPlayer*>(pPlayer);
				//if (pPlayer)
				{
					const auto isClient = pPlayer->IsClient();
					//if (isClient)
					//{
					//	pPlayer->SupressViewBlending(); // no view bleding when respawning // CActor::Revive resets it.
					//	if (g_pGame->GetHUD())
					//		g_pGame->GetHUD()->GetRadar()->Reset();
					//}

					if (isClient)
					{
						//pEntity->FreeSlot(3);
						//pEntity->FreeSlot(4);
					}

					//Script::CallMethod(pEntityTable, "SetActorModel", isClient);

					if (!isClient)
					{
						const auto pItem = pPlayer->GetCurrentItem();
						if (pItem)
						{
							const bool soundEnabled = pItem->IsSoundEnabled();
							pItem->EnableSound(false);
							pItem->Select(false);
							pItem->EnableSound(soundEnabled);

							g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(pPlayer, (EntityId)0);
							pPlayer->SelectItem(pItem->GetEntityId(), true);
						}

						pPlayer->Physicalize();

						if (gEnv->bServer)
							pPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);

						pPlayer->Freeze(false);

						if (const auto pPlayerPhysics = pEntity->GetPhysics())
						{
							pe_action_move actionMove;
							actionMove.dir.zero();
							actionMove.iJump = 1;

							pe_action_set_velocity actionVel;
							actionVel.v.zero();
							actionVel.w.zero();

							pPlayerPhysics->Action(&actionMove);
							pPlayerPhysics->Action(&actionVel);
						}
					}
				}

				//auto flags = pEntity->GetSlotFlags(3);
				//flags &= ~ENTITY_SLOT_RENDER;
				//flags &= ~ENTITY_SLOT_RENDER_NEAREST;
				//pEntity->SetSlotFlags(3, flags);
			}
		}
	}
}

void CConquerorSystem::SetPlayerMaterial(IActor* pActor, const SClassModel& info)
{
	const auto pPlayer = static_cast<CPlayer*>(pActor);

	const bool modelIsNanosuit = std::strstr(info.m_character, "nano");

	//CNanosuit will load all the necessary textures from Nanosuit.cpp
	if (!modelIsNanosuit)
	{
		const auto pBodyMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(info.m_mat.c_str());
		const auto pHandsMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(info.m_armsMat.c_str());
		const auto pHelmetMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(info.m_helmetMat.c_str());

		const auto pCharacter = pPlayer->GetEntity()->GetCharacter(0);
		if (pCharacter)
		{
			if (pBodyMat)
				pCharacter->SetMaterial(pBodyMat);

			// this should be the legs of the character
			const auto pAttachmentMananager = pCharacter->GetIAttachmentManager();
			if (pAttachmentMananager)
			{
				auto pAttachment = pAttachmentMananager->GetInterfaceByName("upper_body");
				if (pAttachment)
				{
					const auto pAttachmentObj = pAttachment->GetIAttachmentObject();
					if (pAttachmentObj)
					{
						const auto pCharInstance = pAttachmentObj->GetICharacterInstance();
						if (pCharInstance)
						{
							// needed to support "fp3p"
							if (pPlayer->IsClient() && pHandsMat)
								pCharInstance->SetMaterial(pHandsMat);
							else
								pCharInstance->SetMaterial(pBodyMat);
						}
					}
				}

				pAttachment = pAttachmentMananager->GetInterfaceByName("helmet");
				if (pAttachment && pHelmetMat)
				{
					const auto pAttachmentObj = pAttachment->GetIAttachmentObject();
					if (pAttachmentObj)
					{
						// TODO: maybe reduce just to pAttachmentObj->SetMaterial...
						const auto pCharInstance = pAttachmentObj->GetICharacterInstance();
						if (pCharInstance)
							pCharInstance->SetMaterial(pHelmetMat);
						else
							pAttachmentObj->SetMaterial(pHelmetMat);
					}
				}

				if (pHandsMat)
				{
					const auto pArms = pPlayer->GetEntity()->GetCharacter(3);
					const auto pDualSocomArms = pPlayer->GetEntity()->GetCharacter(4);

					// arms ... these indices are a bit workaround
					if (pArms)
						pArms->SetMaterial(pHandsMat);

					// second set of arms for dual socom
					if (pDualSocomArms)
						pDualSocomArms->SetMaterial(pHandsMat);
				}
			}
		}
	}

	//It is necessary for the correct application of the nanosuit material after changing the model
	const auto pCurrentNanosuit = pPlayer->GetNanoSuit();
	if (pCurrentNanosuit && modelIsNanosuit)
	{
		pCurrentNanosuit->Reset(pPlayer);
	}
}

CConquerorCommander* CConquerorSystem::GetSpeciesCommander(ESpeciesType species) const
{
	for (CConquerorCommander* pCmdr : m_speciesCommanders)
	{
		if (pCmdr->GetSpecies() == species)
			return pCmdr;
	}

	return nullptr;
}

bool CConquerorSystem::CreateAIEntity(CSpeciesClass* classInfo, string entityName, ESpeciesType species)
{
	if (!classInfo)
	{
		CryLogAlways("$4[C++][ERROR][Failed Create AI ENTITY][Cause: Unknown ClassInfo]");
		return false;
	}

	const auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(classInfo->GetAI().m_archetype.c_str());
	if (pArchetype)
	{
		SEntitySpawnParams params;
		params.pArchetype = pArchetype;
		params.vPosition = Vec3(1, 1, 1);
		params.nFlags = ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_CASTSHADOW; //| ENTITY_FLAG_UNREMOVABLE;
		params.sName = entityName;
		params.bStaticEntityId = true;

		//if (gEnv->bEditor)
		//	params.nFlags &= ~ENTITY_FLAG_UNREMOVABLE;

		const auto pEntity = gEnv->pEntitySystem->SpawnEntity(params);
		if (pEntity)
		{
			char idBuffer[256];
			char squadBuffer[256];
			int squadIdname = 0;

			TOS_AI::SetSpecies(pEntity->GetAI(), (int)species);

			CreateConquerorChannel(pEntity, *classInfo);

			const auto speciesType = ESpeciesType(GetSpeciesFromEntity(pEntity));
			int squadsCount = m_pXMLAICountInfo->GetSquadsCount(speciesType);
			const int unitsCount = m_pXMLAICountInfo->GetUnitsCount(speciesType) - m_speciesLeadersCountMap[speciesType];

			if (squadsCount == 0 && unitsCount == 0)
			{
				CryLogAlways("$4[C++][ERROR][%s][Failed Create (%s)][Cause: unitsCount and squadsCount == 0]",
					GetSpeciesName(speciesType), entityName);

				return false;
			}

			if (unitsCount > 0 && squadsCount == 0)
				squadsCount = 1;

			const int remain = unitsCount % squadsCount;
			int count = unitsCount / squadsCount;
			
			if (remain != 0)
				count += 1;

			const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			const auto& squadIds = g_pControlSystem->GetSquadSystem()->GetSquadIdsFromSpecies(speciesType, true);

			for (const auto squadId : squadIds)
			{
				const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromId(squadId);
				if (pSquad)
				{
					squadIdname = squadId;

					if (classInfo->IsLeaderClass())
					{
						const auto pLeader = pSquad->GetLeader();
						if (!pLeader)
						{
							pSquad->SetLeader(pActor, true);
							break;
						}
					}
					else
					{
						const int memberCount = pSquad->GetMembersCount();
						if (memberCount < count)
						{
							if (pSquad->AddMember(pActor))
							{
								//CryLogAlways("[C++][Search Criteria %i][Squad Members %i][Max Members %i][AI Actor %s][Join Squad %i]", memberCount < count, memberCount, count, pEntity->GetName(), squadId);
								break;
							}
						}
					}
				}
			}

			sprintf(idBuffer, "%i", pEntity->GetId());
			sprintf(squadBuffer, "Sqd:%i ", squadIdname);
			const string newName = string(squadBuffer) + entityName + string(idBuffer);
			pEntity->SetName(newName);

			return true;
		}
	}
}

void CConquerorSystem::CreateAICommander(ESpeciesType species)
{
	for (const auto& commander : m_speciesCommanders)
	{
		if (commander->GetSpecies() == species)
			return;
	}

	m_speciesCommanders.push_back(new CConquerorCommander(species));

	//m_speciesCommanders.push_back(new CConquerorCommander(species);
}

IEntity* CConquerorSystem::EmergencyCreateAIEntity(CSpeciesClass* classInfo, const char* name)
{
	if (!classInfo)
	{
		CryLogAlways("$4[C++][ERROR][Failed Create AI ENTITY][Cause: Unknown ClassInfo]");
		return false;
	}

	const auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(classInfo->GetAI().m_archetype.c_str());
	if (pArchetype)
	{
		SEntitySpawnParams params;
		params.pArchetype = pArchetype;
		params.vPosition = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldPos();
		params.nFlags = ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_CASTSHADOW; //| ENTITY_FLAG_UNREMOVABLE;
		params.sName = name;
		params.bStaticEntityId = true;

		const auto pEntity = gEnv->pEntitySystem->SpawnEntity(params);
		if (pEntity)
		{
			char idBuffer[256];
			sprintf(idBuffer, "%i", pEntity->GetId());
			const string newName = string(name) + string(idBuffer);
			pEntity->SetName(newName);
		}

		return pEntity;
	}
	else
	{
		CryLogAlways("$4[C++][ERROR][Failed Create AI ENTITY][Cause: Unknown Archetype]");
	}

	return nullptr;
}

void CConquerorSystem::InitConquerorAICountInfo(bool forceReload, const char* levelName /*= ""*/)
{
	const string levelNameStr = levelName;
	const string defaultFilePath = "Game/Libs/Conqueror/ConquerorUnits_default.xml";
	const string levelFilePath = "Game/Libs/Conqueror/ConquerorUnits_" + levelNameStr + string(".xml");

	if (forceReload)
		m_pXMLAICountInfo->Clear();

	auto rootNode = GetISystem()->LoadXmlFile(levelFilePath.c_str());

	if (!rootNode)
		rootNode = GetISystem()->LoadXmlFile(defaultFilePath.c_str());

	if (!rootNode)
	{
		CryLogAlways("$4[C++][ERROR][Init Units from xml Failed]");
		return;
	}

	for (int i = 0; i < rootNode->getChildCount(); i++)
	{
		auto factionNode = rootNode->getChild(i);
		if (!factionNode)
			continue;

		//faction attributes
		//XmlString name;
		int speciesIndex = -1;
		int unitsCount = 0;
		int squadsCount = 0;
		int reinforcements = 0;

		//factionNode->getAttr("name", name);
		factionNode->getAttr("speciesIndex", speciesIndex);
		factionNode->getAttr("unitsCount", unitsCount);
		factionNode->getAttr("squadsCount", squadsCount);
		factionNode->getAttr("reinforcements", reinforcements);

		ESpeciesType speciesType = ESpeciesType(speciesIndex);

		m_pXMLAICountInfo->speciesSquadsCountMap[speciesType] = squadsCount;
		m_pXMLAICountInfo->speciesUnitsCountMap[speciesType] = unitsCount;
		m_pXMLAICountInfo->speciesReinforcementCountMap[speciesType] = reinforcements;
		m_pXMLAICountInfo->speciesReinforcementCountConstMap[speciesType] = reinforcements;

		//CryLogAlways("[C++][Species %i][Squads Count %i][Units Count %i]", speciesIndex, squadsCount, unitsCount);
	}
}

void CConquerorSystem::InitConquerorStrategies(bool forceReload, const char* levelName /*= ""*/)
{
	const string levelNameStr = levelName;
	const string defaultFilePath = "Game/Libs/Conqueror/ConquerorStrategies_default.xml";
	const string levelFilePath = "Game/Libs/Conqueror/ConquerorStrategies_" + levelNameStr + string(".xml");

	if (forceReload)
		m_strategies.clear();

	auto rootNode = GetISystem()->LoadXmlFile(levelFilePath.c_str());

	if (!rootNode)
		rootNode = GetISystem()->LoadXmlFile(defaultFilePath.c_str());

	if (!rootNode)
	{
		CryLogAlways("$4[C++][ERROR][Init Strategies from xml Failed]");
		return;
	}
		

	for (int i = 0; i < rootNode->getChildCount(); i++)
	{
		auto strategyNode = rootNode->getChild(i);
		if (!strategyNode)
			continue;

		SConquerorStrategyPtr pStrategy = new SConquerorStrategy;

		//strategy attributes
		const string name = strategyNode->getAttr("name");
		pStrategy->m_name = name.c_str();

		//strategy childs
		auto selectConditionsNode = strategyNode->findChild("SelectConditions");
		if (selectConditionsNode)
		{
			for (int z = 0; z < selectConditionsNode->getChildCount(); ++z)
			{
				auto conditionNode = selectConditionsNode->getChild(z);
				if (conditionNode)
				{
					const string type = conditionNode->getAttr("type");
					const string name = conditionNode->getAttr("name");
					const string relationship = conditionNode->getAttr("relationship");
					const string conditional1 = conditionNode->getAttr("conditional1");
					const string conditional2 = conditionNode->getAttr("conditional2");
					const string oper = conditionNode->getAttr("operator");
					const string svalue = conditionNode->getAttr("strvalue");
					float fvalue = 0; conditionNode->getAttr("fvalue", fvalue);

					SGenericCondition condition;
					condition.m_type = type;
					condition.m_name = name;
					condition.m_relationship = relationship;
					condition.m_conditional1 = conditional1;
					condition.m_conditional2 = conditional2;
					condition.m_operator = oper;
					condition.m_fvalue = fvalue;
					condition.m_svalue = svalue;

					pStrategy->m_conditions.push_back(condition);
				}
			}
		}

		auto settingsNode = strategyNode->findChild("Settings");
		if (settingsNode)
		{
			for (int z = 0; z < settingsNode->getChildCount(); ++z)
			{
				auto paramNode = settingsNode->getChild(z);
				if (paramNode)
				{
					const char* paramName = paramNode->getAttr("name");
					int paramValue = 0; paramNode->getAttr("value", paramValue);

					if (strcmp(paramName, "aggression") == 0)
						pStrategy->m_settings.m_aggression = paramValue;
					else if (strcmp(paramName, "numberOfAttacks") == 0)
						pStrategy->m_settings.m_numberOfAttacks = paramValue;
					else if (strcmp(paramName, "numberOfDefences") == 0)
						pStrategy->m_settings.m_numberOfDefences = paramValue;
					else if (strcmp(paramName, "timeLimit") == 0)
						pStrategy->m_settings.m_timeLimit = paramValue;
					else if (strcmp(paramName, "uncapturableSelect") == 0)
						pStrategy->m_settings.m_uncapturableSelect = paramValue;
					else if (strcmp(paramName, "targetSpecies") == 0)
						pStrategy->m_settings.m_targetSpecies = paramNode->getAttr("value");
					else if (strcmp(paramName, "vehicleUseDistance") == 0)
						pStrategy->m_settings.m_vehicleUseDistance = paramValue;
				}
			}
		}

		auto goalsNode = strategyNode->findChild("Goals");
		if (goalsNode)
		{
			for (int z = 0; z < goalsNode->getChildCount(); ++z)
			{
				auto goalNode = goalsNode->getChild(z);
				if (goalNode)
				{
					const string type = goalNode->getAttr("type");
					const string name = goalNode->getAttr("name");
					const string relationship = goalNode->getAttr("relationship");
					const string conditional1 = goalNode->getAttr("conditional1");
					const string conditional2 = goalNode->getAttr("conditional2");
					const string oper = goalNode->getAttr("operator");
					const string svalue = goalNode->getAttr("strvalue");
					float fvalue = 0; goalNode->getAttr("fvalue", fvalue);

					SGenericCondition goal;
					goal.m_type = type;
					goal.m_name = name;
					goal.m_relationship = relationship;
					goal.m_conditional1 = conditional1;
					goal.m_conditional2 = conditional2;
					goal.m_operator = oper;
					goal.m_fvalue = fvalue;
					goal.m_svalue = svalue;

					pStrategy->m_goals.push_back(goal);
				}
			}
		}


		auto prioritiesNode = strategyNode->findChild("AreaStrategyPriorities");
		if (prioritiesNode)
		{
			for (int z = 0; z < prioritiesNode->getChildCount(); ++z)
			{
				auto areaNode = prioritiesNode->getChild(z);
				if (areaNode)
				{
					float priorityValue = 0; areaNode->getAttr("priority", priorityValue);
					const char* flag = areaNode->getAttr("flag");
					const char* status = areaNode->getAttr("status");
					const char* targetSpecies = areaNode->getAttr("targetSpecies");

					SStrategyPriority priority;
					priority.m_priority = priorityValue;
					priority.m_status = status;
					priority.m_areaFlag = flag;
					priority.m_targetSpecies = targetSpecies;

					pStrategy->m_priorities.push_back(priority);
				}
			}
		}

		m_strategies.push_back(pStrategy);
		pStrategy->m_index = m_strategies.size() - 1;

		if (g_pGameCVars->conq_debug_log_strategies)
		{
			CryLogAlways("$3[C++][Init Strategy Success][Name %s][Index %d][timeLimit %1.f][aggression %1.f]",
				pStrategy->m_name.c_str(), pStrategy->m_index, pStrategy->GetSettings().m_timeLimit, pStrategy->GetSettings().m_aggression);
		}
	}
}

bool CConquerorSystem::ReadStrategyCondition(const CConquerorCommander* pDesiredOwner, const SGenericCondition& condition, bool goal) const
{
	if (!pDesiredOwner)
		return false;

	const auto& oper = condition.m_operator;
	const auto& type = condition.m_type;
	const auto& cond1 = condition.m_conditional1;
	const auto& cond2 = condition.m_conditional2;
	const auto& svalue = condition.m_svalue;
	const auto& fvalue = condition.m_fvalue;

	//CryLogAlways("[C++][Read Strategy Condition][Type %s][Name %s][Conditional1 %s][Conditional2 %s][Operator %s][strvalue %s][fvalue %1.f]",
	//	type, condition.m_name, cond1, cond2, oper, svalue, fvalue);

	if (type == "Game")
	{
		if (cond1 == "GameStatus")
		{
			auto check = eGS_GameStart;
			if (svalue == "InBattle")
				check = eGS_Battle;
			else if (svalue == "GameStart")
				check = eGS_GameStart;
			else if (svalue == "GameOver")
				check = eGS_GameOver;

			if (oper == "Equal")
				return m_gameStatus == check;
			else if (oper == "Less")
				return m_gameStatus < check;
			else if (oper == "More")
				return m_gameStatus > check; 
			else if (oper == "NotEqual")
				return m_gameStatus != check;
		}
	}
	else if (type == "Count")
	{
		auto checkValue = 0;
		auto countedValue = 0;

		if (cond1 == "Areas")
		{
			auto getAreaFlag = [](const string& str)
			{
				auto areaFlag = EAreaFlag::FirstType;

				if (strstr(str, "Centre"))
					areaFlag = EAreaFlag::Centre;
				else if (strstr(str, "AirSpawner"))
					areaFlag = EAreaFlag::AirSpawner;
				else if (strstr(str, "LandSpawner"))
					areaFlag = EAreaFlag::LandSpawner;
				else if (strstr(str, "SeaSpawner"))
					areaFlag = EAreaFlag::SeaSpawner;
				else if (strstr(str, "SoldierSpawner"))
					areaFlag = EAreaFlag::SoldierSpawner;
				else if (strstr(str, "Bridge"))
					areaFlag = EAreaFlag::Bridge;
				else if (strstr(str, "Base"))
					areaFlag = EAreaFlag::Base;
				else if (strstr(str, "AirField"))
					areaFlag = (EAreaFlag::AirField);
				else if (strstr(str, "SupplyPoint"))
					areaFlag = (EAreaFlag::SupplyPoint);
				else if (strstr(str, "ControlPoint"))
					areaFlag = (EAreaFlag::ControlPoint);
				else if (strstr(str, "North"))
					areaFlag = (EAreaFlag::North);
				else if (strstr(str, "West"))
					areaFlag = (EAreaFlag::West);
				else if (strstr(str, "South"))
					areaFlag = (EAreaFlag::South);
				else if (strstr(str, "East"))
					areaFlag = (EAreaFlag::East);
				else if (strstr(str, "Safe"))
					areaFlag = (EAreaFlag::Safe);
				else if (strstr(str, "Neutral"))
					areaFlag = (EAreaFlag::Neutral);
				else if (strstr(str, "Front"))
					areaFlag = (EAreaFlag::Front);

				return areaFlag;
			};

			std::vector<EAreaFlag> flags;
			if (cond2 != "")
				flags.push_back(getAreaFlag(cond2));

			const auto& relationship = condition.m_relationship;
			const int check = condition.m_fvalue;
			int areasCount = 0;

			if (!goal)
			{
				if (relationship == "Friendly")
					areasCount = GetStrategicAreaCount(pDesiredOwner->GetSpecies(), flags, eAGSF_Enabled);
				else if (relationship == "Hostile")
					areasCount = GetHostileAreasCount(pDesiredOwner->GetSpecies(), flags, eAGSF_Enabled);
				else if (relationship == "Any")
					areasCount = GetStrategicAreaCount(flags, eAGSF_Enabled);
			}
			else
			{
				if (relationship == "Friendly")
					areasCount = pDesiredOwner->GetCurrentStrategyGoals(eSGT_CapturedAreasCount);
					//areasCount = GetStrategicAreaCount(pDesiredOwner->GetSpecies(), flags, eAGSF_Enabled);
				else if (relationship == "Hostile")
					areasCount = GetHostileAreasCount(pDesiredOwner->GetSpecies(), flags, eAGSF_Enabled);
				else if (relationship == "Any")
					areasCount = GetStrategicAreaCount(flags, eAGSF_Enabled);
			}

			checkValue = check;
			countedValue = areasCount;
		}
		else if (cond1 == "Reinforcements")
		{
			const auto cond2SpeciesType = GetSpeciesTypeFromString(cond2);
			const int check = condition.m_fvalue;
			const int reinfCount = GetSpeciesReinforcements(cond2SpeciesType);

			checkValue = check;
			countedValue = reinfCount;
		}
		else if (cond1 == "Squads")
		{
			const auto cond2SpeciesType = GetSpeciesTypeFromString(cond2);
			const int check = condition.m_fvalue;
			const auto& squads = g_pControlSystem->GetSquadSystem()->GetSquadIdsFromSpecies(cond2SpeciesType, true);
			const int squadsCount = squads.size();

			checkValue = check;
			countedValue = squadsCount;
		}
		
		if (oper == "Equal")
			return countedValue == checkValue;
		else if (oper == "Less")
			return countedValue < checkValue;
		else if (oper == "More")
			return countedValue > checkValue;
		else if (oper == "NotEqual")
			return countedValue != checkValue;
	}
	else if (type == "Strategies")
	{
		const auto checkCount = condition.m_fvalue;
		const auto& check = condition.m_svalue;
		const auto& names = pDesiredOwner->m_strategiesNamesHistory;

		if (cond1 == "Required")
		{
			if (oper == "AnyOf")
			{
				for (auto& name : names)
				{
					const bool isUsed = strstr(check, name);
					if (isUsed)
						return true;
				}
			}
			else if (oper == "AllOf")
			{
				auto countedValue = 0;

				for (auto& name : names)
				{
					const bool isUsed = strstr(check, name);
					if (isUsed)
						countedValue++;
				}

				if (countedValue >= checkCount)
					return true;
			}			
		}
		else if(cond1 == "Blocking")
		{
			if (oper == "AnyOf")
			{
				bool isUsed = false;

				for (auto& name : names)
				{
					isUsed = strstr(check, name);
					if (isUsed)
						return false;
				}

				if (!isUsed)
					return true;
			}
			else if (oper == "AllOf")
			{
				auto countedValue = 0;
				auto isUsed = false;

				for (auto& name : names)
				{
					isUsed = strstr(check, name);
					if (isUsed)
						countedValue++;
				}

				if (countedValue != checkCount)
					return true;
			}
		}
	}
	else if (type == "OurSpecies")
	{
		const string countedValue = GetSpeciesName(pDesiredOwner->GetSpecies());
		const string checkValue = cond1;

		if (oper == "Equal")
			return countedValue == checkValue;
		else if (oper == "NotEqual")
			return countedValue != checkValue;
	}

	return false;
}

bool CConquerorSystem::ReadStrategyGoals(const CConquerorCommander* pStrategyOwner) const
{
	if (!pStrategyOwner)
		return false;

	const auto pCurrent = pStrategyOwner->GetCurrentStrategy();
	if (!pCurrent)
		return false;

	const auto& goals = pCurrent->GetGoals();
	const int count = goals.size();

	int readed = 0;

	for (auto& goal : goals)
	{
		if (ReadStrategyCondition(pStrategyOwner, goal, true))
			readed++;
	}

	return readed == count;
}

bool CConquerorSystem::ReadClassConditions(ESpeciesType species, const CSpeciesClass* pClass, bool AI) 
{
	const auto pCommander = GetSpeciesCommander(species);
	if (!pCommander)
		return true;

	if (!pClass)
		return true;

	//const auto flags = pClass->GetFlags();
	//const bool unlockedByArea = (flags & eSCF_UnlockedByArea);
	const bool unlockedByArea = pClass->CheckFlag(eSCF_UnlockedByArea);
	const int count = pClass->m_conditions.size();

	auto readed = 0;

	for (auto& cond : pClass->m_conditions)
	{
		if (ReadStrategyCondition(pCommander, cond, false))
			readed++;
	}

	if (unlockedByArea)
	{
		return pClass->CheckFlag(AI ? eSCF_UnlockedForAI : eSCF_UnlockedForPlayer);
	}
	else if (pClass->CheckFlag(eSCF_UnlockedForAI) || pClass->CheckFlag(eSCF_UnlockedForPlayer))
	{
		return pClass->CheckFlag(AI ? eSCF_UnlockedForAI : eSCF_UnlockedForPlayer);
	}
	else if (readed < count)
	{
		CryLogAlways("$8[C++][WARNING][Can not read conditions of %s class][Readed %i, Count %i]",
			pClass->m_name, readed, count);
		return false;
	}

	return true;
}

//bool CConquerorSystem::ApplyStrategy(CConquerorCommander* pDesiredOwner, const SConquerorStrategy& strategy)
//{
//	if (!pDesiredOwner)
//		return false;
//	pDesiredOwner->SetCurrentStrategy(&strategy);
//
//	return pDesiredOwner->GetCurrentStrategy()->m_name == strategy.m_name;
//}

SConquerorStrategy* CConquerorSystem::GetStrategy(const char* name) const
{
	for (SConquerorStrategy* pStrategy : m_strategies)
	{
		if (strcmp(pStrategy->m_name, name) == 0)
			return pStrategy;
	}

	return nullptr;
}

SConquerorStrategy* CConquerorSystem::GetStrategy(uint index) const
{
	for (SConquerorStrategy* pStrategy : m_strategies)
	{
		if (pStrategy->m_index == index)
			return pStrategy;
	}

	return nullptr;
}

std::vector<int> CConquerorSystem::GetAllowableStrategies(CConquerorCommander* pDesiredOwner, bool includeCurrentStrategy) const
{
	std::vector<int> strategies;

	if (!pDesiredOwner)
		return strategies;

	for (const SConquerorStrategy* pStrategy : m_strategies)
	{
		const auto pCmdrStrategy = pDesiredOwner->GetCurrentStrategy();
		if (pCmdrStrategy)
		{
			const bool isCurrent = pStrategy->GetIndex() == pCmdrStrategy->GetIndex();
			if (isCurrent && !includeCurrentStrategy)
				continue;
		}

		const auto conditionsCount = pStrategy->m_conditions.size();
		auto trueCondCount = 0;

		for (auto& condition : pStrategy->m_conditions)
		{
			if (ReadStrategyCondition(pDesiredOwner, condition, false))
				trueCondCount++;
		}

		if (trueCondCount == conditionsCount)
		{
			if (g_pGameCVars->conq_debug_log_strategies)
				CryLogAlways("[C++][%s Commander][Get Allowable Strategies [Index %i](%s)]", 
					GetSpeciesName(pDesiredOwner->GetSpecies()), pStrategy->GetIndex(), pStrategy->GetName());

			strategies.push_back(pStrategy->GetIndex());
		}
	}

	return strategies;
}

CSpeciesClass* CConquerorSystem::GetClass(ESpeciesType species, const char* name) const
{
	const auto iter = m_speciesClassesMap.find(species);
	if (iter == m_speciesClassesMap.end())
		return nullptr;

	for (CSpeciesClass* pSpeciesClass : iter->second)
	{
		if (strcmp(pSpeciesClass->GetName(), name) == 0)
			return pSpeciesClass;
	}

	return nullptr;
}

EVehicleClassTypes CConquerorSystem::GetVehicleClassType(const IVehicle* pVehicle)
{
	if (!pVehicle)
		return eVCT_Default;

	for (auto& vehClass : m_vehicleClasses)
	{
		if (vehClass.name == pVehicle->GetEntity()->GetClass()->GetName())
			return vehClass.type;
	}

	return eVCT_Default;
}

IEntity* CConquerorSystem::GetNearestLandingSpot(IEntity* pAirEntity)
{
	if (!pAirEntity)
		return nullptr;

	AABB bounds; 
	pAirEntity->GetWorldBounds(bounds);

	primitives::box obb;
	obb.Basis = Matrix33(pAirEntity->GetWorldTM());
	obb.size = bounds.GetSize();
	obb.bOriented = true;

	const auto pIter = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIter->IsEnd())
	{
		const auto pEnt = pIter->Next();
		if (!pEnt)
			continue;

		const char* type = "";
		TOS_Script::GetEntityProperty(pAirEntity, "Properties", "aianchor_AnchorType", type);

		if (strcmp(type, "TOS_SA_AIRVEHICLE_LANDING_SPOT") == 0)
		{
			const auto pos = pEnt->GetWorldPos();
			obb.center = pos + bounds.GetCenter();

			const auto test = g_pGame->GetGameRules()->TestEntitySpawnPosition(pAirEntity->GetId(), pos, obb);

			if (test)
				return pEnt;
		}
	}

	return nullptr;
}

IEntity* CConquerorSystem::GetNearestLandingSpot(IEntity* pAirEntity, IEntity* pAreaEntity, float radius)
{
	if (!pAirEntity || !pAreaEntity)
		return nullptr;

	AABB bounds;
	pAirEntity->GetWorldBounds(bounds);

	primitives::box obb;
	obb.Basis = Matrix33(pAirEntity->GetWorldTM());
	obb.size = bounds.GetSize();
	obb.bOriented = true;

	const auto pIter = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIter->IsEnd())
	{
		const auto pEnt = pIter->Next();
		if (!pEnt)
			continue;

		const auto pos1 = pEnt->GetWorldPos();
		const auto pos2 = pAreaEntity->GetWorldPos();
		const float dist = (pos1-pos2).GetLength();

		if (dist < radius)
		{
			const char* type = "";
			TOS_Script::GetEntityProperty(pAirEntity, "Properties", "aianchor_AnchorType", type);

			if (strcmp(type, "TOS_SA_AIRVEHICLE_LANDING_SPOT") == 0)
			{
				const auto pos = pEnt->GetWorldPos();
				obb.center = pos + bounds.GetCenter();

				const auto test = g_pGame->GetGameRules()->TestEntitySpawnPosition(pAirEntity->GetId(), pos, obb);

				if (test)
					return pEnt;
			}
		}
	}

	return nullptr;

}

//IEntity* CConquerorSystem::GetNewNearestUnloadSpot(IVehicle* pVehicle, CStrategicArea* pArea)
//{
//	if (!pVehicle || !pArea)
//		return nullptr;
//
//	auto pBookedSpot = pArea->GetBookedUnloadSpot(pVehicle);
//	if (pBookedSpot)
//	{
//		return pBookedSpot;
//	}
//	else
//	{
//		//Remove the vehicle's booked spot on other area
//		if (IsHaveUnloadSpot(pVehicle))
//			UnbookUnloadSpot(pVehicle);
//
//		//Book the spot for the vehicle at the current area
//		pBookedSpot = pArea->BookFreeUnloadSpot(pVehicle);
//	}
//
//	return pBookedSpot;
//}

bool CConquerorSystem::IsHaveUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return false;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea->GetBookedUnloadSpot(pEntity))
			return true;
	}

	return false;
}

void CConquerorSystem::UnbookUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea->GetBookedUnloadSpot(pEntity))
		{
			pArea->UnbookUnloadSpot(pEntity);
			//break;
		}
	}
}

void CConquerorSystem::SetCanBookSpot(IVehicle* pVehicle, bool can, const char* cause)
{
	if (!pVehicle)
		return;

	m_vehiclesCanBookUnloadSpots[pVehicle->GetEntityId()] = can;

	if (g_pGameCVars->conq_debug_log)
	{
		CryLogAlways("[C++][Set Can Book Spot To: %i][Vehicle: %s][Cause: %s]",
			can, pVehicle->GetEntity()->GetName(), cause);
	}
}

bool CConquerorSystem::CanBookUnloadSpot(IEntity* pEntity)
{
	if (!pEntity)
		return false;

	const auto iter = m_vehiclesCanBookUnloadSpots.find(pEntity->GetId());
	if (iter != m_vehiclesCanBookUnloadSpots.end())
		return iter->second;

	return true;
}

//SUnloadSpotInfo* CConquerorSystem::GetNearestFreeUnloadSpot(IVehicle* pVehicle, IEntity* pAreaEntity)
//{
//	if (!pVehicle || !pAreaEntity)
//		return nullptr;
//
//	//Get vehicle information here
//	const auto pVehEnt = pVehicle->GetEntity();
//	const auto vehPos = pVehEnt->GetWorldPos();
//	const auto isAir = TOS_Vehicle::IsAir(pVehicle);
//
//	AABB bounds;
//	pVehEnt->GetWorldBounds(bounds);
//
//	primitives::box obb;
//	obb.Basis = Matrix33(pVehEnt->GetWorldTM());
//	obb.center = vehPos + bounds.GetCenter();
//	obb.size = bounds.GetSize();
//	obb.bOriented = true;
//
//	auto pArea = GetStrategicArea(pAreaEntity->GetId(), 0);
//	if (!pArea)
//		return nullptr;
//
//	//Get nearest here
//	std::vector<SUnloadSpotInfo> spots;
//	pArea->GetVehicleUnloadSpots(isAir ? "AIR" : "LAND", spots);
//
//	if (spots.size() > 0)
//	{
//		auto pNearestSpotEnt = gEnv->pEntitySystem->GetEntity(spots.at(0).entityId);
//		if (!pNearestSpotEnt)
//			return nullptr;
//
//		float minDist = (pNearestSpotEnt->GetWorldPos() - vehPos).GetLength();
//
//		for (auto& spot : spots)
//		{
//			auto pSpotEnt = gEnv->pEntitySystem->GetEntity(spot.entityId);
//			if (!pSpotEnt)
//				continue;
//	
//			auto pos = pSpotEnt->GetWorldPos();
//			const auto radius = spot.radius;
//
//			auto xTest = false;
//			auto yTest = false;
//
//			for (int i = pos.x - radius; i <= pos.x + radius; i++)
//			{
//				if (g_pGame->GetGameRules()->TestEntitySpawnPosition(pVehEnt->GetId(), pos, obb))
//				{
//					xTest = true;
//					pos.x = i;
//					CryLogAlways("%s[C++][Unload Spot %s: success xTest, pos.x = %1.f]", STR_BLUE, pSpotEnt->GetName(), pos.x);
//					break;
//				}
//			}
//
//			//If xTest failed start yTest here
//			if (!xTest)
//			{
//				for (int i = pos.y - radius; i <= pos.y + radius; i++)
//				{
//					if (g_pGame->GetGameRules()->TestEntitySpawnPosition(pVehEnt->GetId(), pos, obb))
//					{
//						yTest = true;
//						pos.y = i;
//						CryLogAlways("%s[C++][Unload Spot %s: success yTest, pos.y = %1.f]", STR_BLUE, pSpotEnt->GetName(), pos.y);
//						break;
//					}
//				}
//			}
//
//			const auto dist = (pos - vehPos).GetLength();
//
//			//If both tests is failed then select new spot here
//			if (!xTest && !yTest)
//			{
//				CryLogAlways("%s[C++][Unload Spot %s: failed xTest and yTest]", STR_RED, pSpotEnt->GetName());
//				continue;
//			}
//			else
//			{
//				if (dist < minDist)
//				{
//					minDist = dist;
//					pNearestSpotEnt = pSpotEnt;
//				}
//			}
//		}
//
//		return pArea->GetVehicleUnloadSpot(pNearestSpotEnt->GetId());
//	}
//
//	return nullptr;
//}

int CConquerorSystem::GetConquerorChannelsCount()
{
	return m_conquerorChannels.size();
}

void CConquerorSystem::GameOver(bool isOver)
{
	m_gameOver = isOver;

	if (isOver)
		m_gameStatus = eGS_GameOver;

	//if (!gEnv->bEditor)
	//{
	//	if (m_gameOver && !g_pGame->GetIGameFramework()->IsGamePaused())
	//		g_pGame->GetIGameFramework()->PauseGame(true, true);
	//	else if (!m_gameOver && g_pGame->GetIGameFramework()->IsGamePaused())
	//		g_pGame->GetIGameFramework()->PauseGame(false, true);
	//}
}

void CConquerorSystem::OnChangeConfirmedLobbySpecies()
{
	m_speciesSwitchCount++;
	//CryLogAlways("[C++][Confirm Lobby Species][Now confirmed count is %i]", m_speciesSwitchCount);
}

void CConquerorSystem::OnSpeciesDestroyed(ESpeciesType species)
{
	HUDLobbyEnableSpecies(species, false);

	const auto pChannel = GetClientConquerorChannel();
	if (pChannel && pChannel->GetSpecies() == species)
	{
		//если наша фракция уничтожена и менять её больше нельзя, то это гг дамы и господа (Вывести сообщение о поражении)

		const bool canSwitchSpecies = CanSwitchSpecies(pChannel->GetEntity());
		const bool inLobby = (pChannel->GetState() == eCCS_Spectator) && (m_LobbyInfo.state == EConquerLobbyState::IN_LOBBY);
		const bool isDead = pChannel->GetState() == eCCS_Dead;

		if (inLobby)
		{
			OnLobbySetTeam(m_lobbyAllowedSpecies[0], 0);
			const auto soldiers = GetSpeciesReinforcements(species);

			if (canSwitchSpecies)
			{
				//Вывести сообщение о поражении текущей фракции
				//Насильно сменить фракцию				

				if (m_haveBotsSpawned)
				{
					g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_select_new", 6.0f, ColorF(1.0f, 1.0f, 0.0f));
				}
				else
				{
					if (soldiers >= 1)
					{
						char times[4];
						sprintf(times, "%i", m_CVarSpeciesChangeLimit - m_speciesSwitchCount);

						g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_you_can_switch_team_count", 6.0f, ColorF(0.0f, 1.0f, 0.0f), true, times);
					}
					else
					{
						HUDLobbyMenuShow(false);
						g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
						GameOver(true);
					}

				}
			}
			else
			{
				if (!m_haveBotsSpawned)
				{
					if (soldiers >= 1)
					{
						char times[4];
						sprintf(times, "%i", m_CVarSpeciesChangeLimit - m_speciesSwitchCount);

						g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_you_can_switch_team_count", 5.0f, ColorF(0.0f, 1.0f, 0.0f), true, times);
					}
					else
					{
						HUDLobbyMenuShow(false);
						g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
						GameOver(true);
					}

				}
				else
				{
					HUDLobbyMenuShow(false);
					g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
					GameOver(true);
				}
			}			
		}
		else if (isDead)
		{	
			if (canSwitchSpecies)
			{
				g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_select_new", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
				
				auto info = GetLobbyInfo();
				info.state = EConquerLobbyState::IN_LOBBY;
				SetLobbyInfo(info);
				//OnLobbySetTeam(m_lobbyAllowedSpecies[0], 0);
			}
			else
			{
				GameOver(true);
				g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
			}

			//Вы погибли и ваша фракция уничтожена и менять её больше нельзя
			//Вывести сообщение о поражении
			//Вывести сообщение "Нажмите ESC, чтобы перейти в меню"
		}
	}

	const auto pCommander = GetSpeciesCommander(species);
	if (pCommander)
	{
		const auto pSquadSystem = g_pControlSystem->GetSquadSystem();

		const auto& squads = pSquadSystem->GetSquadIdsFromSpecies(pCommander->GetSpecies(), true);
		for (const auto id : squads)
			pSquadSystem->RemoveSquad(id);

		stl::find_and_erase(m_speciesCommanders, pCommander);
	}

	if (m_speciesCommanders.size() == 1)
	{
		const CConquerorCommander* pCommander = m_speciesCommanders.at(0);
		if (pCommander)
			m_winnerSpecies = pCommander->GetSpecies();
	}

	if (pChannel && pChannel->GetSpecies() == m_winnerSpecies)
	{
		g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_you_win", 8.0f, ColorF(0.0f, 1.0f, 0.0f));
		GameOver(true);

		const auto reinfCount = GetSpeciesReinforcements(m_winnerSpecies);

		if (reinfCount <= 0)
		{
			const auto currentCount = GetSpeciesReinforcements(m_winnerSpecies);
			const auto newCount = currentCount + (1 * 1);

			SetSpeciesReinforcements(m_winnerSpecies, newCount);
		}
	}

	//if (g_pGameCVars->conq_debug_log)
		CryLogAlways("$8[C++][WARNING][%s Species has destroyed]", 
			GetSpeciesName(species));
}

bool CConquerorSystem::CanJoinGame()
{
	const auto backups = GetSpeciesReinforcements(GetClientSpecies()) > 0;
	const auto speciesCount = m_lobbyAllowedSpecies.size() > 0;
	const auto speciesAllowed = IsSpeciesAllowed(GetClientSpecies(), true);

	return backups && speciesCount && speciesAllowed;
}

bool CConquerorSystem::CanSwitchSpecies(IEntity* pEntity)
{
	const auto pChannel = GetConquerorChannel(pEntity);
	if (!pChannel)
		return false;

	const auto count = m_lobbyAllowedSpecies.size();
	if (count <= 1)
		return false;

	const auto can = GetSpeciesReinforcements(pChannel->GetSpecies()) > 10;

	return can && pChannel->IsPlayer() &&
		(m_speciesSwitchCount < m_CVarSpeciesChangeLimit) && 
		(m_CVarSpeciesChangeLimit > 0);
}

void CConquerorSystem::SetFriendAreaPointsCount(int count)
{
	const auto newCount = max(0, count);

	HUDSOMSetPointsCount(true, newCount);

	const auto areasCount = GetStrategicAreaCount(eAGSF_EnabledAndCapturable);
	const auto neutralCount = GetStrategicAreaCount(eST_NEUTRAL, eAGSF_EnabledAndCapturable);

	const auto enemyCount = areasCount - neutralCount - count;
	const auto newEnemyCount = max(0, enemyCount);

	HUDSOMSetPointsCount(false, newEnemyCount);

	m_friendPointsCount = newCount;
	m_enemyPointsCount = newEnemyCount;
}

void CConquerorSystem::HUDLobbyManageClass(ESpeciesType species, const char* className, bool remove)
{
	const char* menuName = "Classes";

	const SFlashVarValue args[2] = { className, (int)species };

	if (remove)
		m_animConquerorLobby.CheckedInvoke("RemoveDesiredClass", args, 2);
	else
		m_animConquerorLobby.CheckedInvoke("AddDesiredClass", args, 2);
	//}
	//if (enable)
	//	stl::push_back_unique(m_lobbyAllowedSpecies, species);
	//else
	//	stl::find_and_erase(m_lobbyAllowedSpecies, species);
}

void CConquerorSystem::HUDLobbyEnableSpecies(ESpeciesType species, bool enable)
{
	const char* menuName = "Factions";
	const char* factionName = GetSpeciesName(species);

	const SFlashVarValue args[3] = { menuName, factionName, enable };
	m_animConquerorLobby.CheckedInvoke("EnableMenuButtomByName", args, 3);
	
	if (enable)
		stl::push_back_unique(m_lobbyAllowedSpecies, species);
	else
		stl::find_and_erase(m_lobbyAllowedSpecies, species);
}

//void CConquerorSystem::HUDLobbyShowSpeciesClasses(ESpeciesType species)
//{
//	const char* menuName = "Factions";
//
//	//menu name, faction index (species)
//	SFlashVarValue args[2] = { menuName, (int)species };
//	m_animConquerorLobby.CheckedInvoke("OnFactionSelected", args, 2);
//}

void CConquerorSystem::HUDLobbyUpdateSpeciesMenu(bool lobbySpecies, bool saveSelection)
{
	SFlashVarValue selectedFaction = (int)0;
	SFlashVarValue selectedClass = (int)0;

	if (saveSelection)
	{
		m_animConquerorLobby.GetFlashPlayer()->GetVariable("g_selectedFaction", &selectedFaction);
		m_animConquerorLobby.GetFlashPlayer()->GetVariable("g_selectedClass", &selectedClass);
	}

	m_animConquerorLobby.Reload(true);

	SetSpeciesChangeLimit(m_CVarSpeciesChangeLimit);
	//m_animConquerorLobby.Invoke("SetSwitchSpeciesCount", m_CVarSpeciesChangeLimit - m_speciesSwitchCount);

	auto it = m_speciesClassesMap.begin();
	const auto end = m_speciesClassesMap.end();
	for (; it != end; it++)
	{
		const auto species = it->first;
		const auto speciesName = GetSpeciesName(species);
		m_animConquerorLobby.CheckedInvoke("AddAvailableFaction", speciesName);

		//Disable buttons of a disallowed factions
		if (!IsSpeciesAllowed(species, lobbySpecies))
			HUDLobbyEnableSpecies(species, false);

		auto& classesVector = it->second;
		for (const CSpeciesClass* i : classesVector)
		{
			if (ReadClassConditions(species, i, false))
			{
				const char* className = i->GetName();
				const auto removeClass = false;

				HUDLobbyManageClass(species, className, removeClass);
			}
		}
	}

	if (saveSelection)
	{
		const auto species = (ESpeciesType)m_lobbySelectedSpeciesIndex;
		const auto classIdx = m_lobbySelectedClassIndex;
		const CSpeciesClass* pClientClass = m_speciesClassesMap[species][classIdx];

		const bool needFirst= !ReadClassConditions(species, pClientClass, false);

		SFlashVarValue args[2] = { "Factions", selectedFaction };
		m_animConquerorLobby.CheckedInvoke("OnFactionSelected", args, 2);

		args[0] = "Classes";
		args[1] = needFirst ? 0 : selectedClass;
		m_animConquerorLobby.CheckedInvoke("OnClassSelected", args, 2);
	}
}

void CConquerorSystem::HUDLobbyMenuShow(bool show)
{
	m_animConquerorLobby.SetVisible(show);
}

void CConquerorSystem::UpdateHUDSOM(bool reset)
{
	auto allowedSpecies = m_gameAllowedSpecies;

	const auto playerDefaultFlagSpecies = GetClientSpecies();
	const int playerDefaultFlagIndex = 0;
	const int allowedFlagsCount = m_gameAllowedSpecies.size();
	const int maxFlagsCount = 4;

	//Reset the Swing O Meter//
	HUDSOMReset();

	m_speciesFlagIndexMap.clear();

	for (int i = 0; i < allowedFlagsCount; i++)
		HUDSOMSetFlagEnabled(i, true);

	for (int i = 0; i < maxFlagsCount; i++)
		HUDSOMSetFlagSpecies(i, eST_NEUTRAL);

	//Set player default flag//
	HUDSOMSetFlagSpecies(playerDefaultFlagIndex, playerDefaultFlagSpecies);
	HUDSOMSetFlagRelationship(playerDefaultFlagIndex, "friend");
	stl::find_and_erase(allowedSpecies, playerDefaultFlagSpecies);

	//Set enemy flags//
	for (int flagIndex = playerDefaultFlagIndex + 1; flagIndex < allowedFlagsCount; flagIndex++)
	{
		for (auto species : allowedSpecies)
		{
			if (GetFlagIndexSpecies(flagIndex) == eST_NEUTRAL)
			{
				HUDSOMSetFlagSpecies(flagIndex, species);
				stl::find_and_erase(allowedSpecies, species);

				break;
			}

		}

		HUDSOMSetFlagRelationship(flagIndex, "enemy");
	}

	//Set species reinforcements//
	for (const auto species : m_gameAllowedSpecies)
	{
		const auto flagIndex = GetSpeciesFlagIndex(species);
		const auto flagUnits = GetSpeciesReinforcements(species);

		//if (GetFlagIndexSpecies(flagIndex) != species)
		//	continue;

		HUDSOMSetFlagUnitsCount(flagIndex, flagUnits);

		const auto pCommander = GetSpeciesCommander(species);
		if (pCommander)
			HUDSOMSetFlagLoseAdvantage(flagIndex, pCommander->m_loosingAdvantage);
	}
}

void CConquerorSystem::HUDSOMReset()
{
	if (m_animSwingOMeter.IsLoaded())
		m_animSwingOMeter.Invoke("resetSwingOMeter");
}

void CConquerorSystem::HUDSOMSetFlagSpecies(int flagIndex, ESpeciesType species)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const auto name = GetSpeciesName(species);

		const SFlashVarValue args[2] = { flagIndex, name };
		m_animSwingOMeter.Invoke("setFlagSpecies", args, 2);

		m_speciesFlagIndexMap[species] = flagIndex;

		if (g_pGameCVars->conq_debug_log_ometer)
			CryLogAlways("[C++][Flag Index %i Set Species To %s]", flagIndex, name);
	}
}

void CConquerorSystem::HUDSOMSetFlagUnitsCount(int flagIndex, int count)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const SFlashVarValue args[2] = { flagIndex, count };
		m_animSwingOMeter.Invoke("setFlagUnitsCount", args, 2);

		if (g_pGameCVars->conq_debug_log_ometer)
			CryLogAlways("[C++][Flag Index %i Set Units Count To %i]", flagIndex, count);
	}
}

void CConquerorSystem::HUDSOMSetFlagLoseAdvantage(int flagIndex, bool lose)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const SFlashVarValue args[2] = { flagIndex, lose };
		m_animSwingOMeter.Invoke("setFlagLoseAdvantage", args, 2);

		if (g_pGameCVars->conq_debug_log_ometer)
			CryLogAlways("[C++][Flag Index %i Set Lose Advantage To %i]", flagIndex, lose);
	}
}

void CConquerorSystem::HUDSOMSetFlagEnabled(int flagIndex, bool enabled)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const SFlashVarValue args[2] = { flagIndex, (int)enabled };
		m_animSwingOMeter.Invoke("setFlagEnabled", args, 2);

		if (g_pGameCVars->conq_debug_log_ometer)
			CryLogAlways("[C++][Flag Index %i Set Enabled To %i]", flagIndex, enabled);
	}
}

void CConquerorSystem::HUDSOMSetFlagRelationship(int flagIndex, const char* relationship)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const SFlashVarValue args[2] = { flagIndex, relationship };
		m_animSwingOMeter.Invoke("setFlagRelationship", args, 2);

		if (g_pGameCVars->conq_debug_log_ometer)
			CryLogAlways("[C++][Flag Index %i Set Relationship To %s]", flagIndex, relationship);
	}
}

ESpeciesType CConquerorSystem::GetFlagIndexSpecies(int index)
{
	if (m_speciesFlagIndexMap.size() != 0)
	{
		auto it = m_speciesFlagIndexMap.begin();
		const auto end = m_speciesFlagIndexMap.end();

		for (; it != end; it++)
		{
			if (it->second == index)
				return it->first;
		}

	}

	return eST_NEUTRAL;
}

void CConquerorSystem::SetSpeciesChangeLimit(int value)
{
	m_CVarSpeciesChangeLimit = value;

	if (m_animConquerorLobby.IsLoaded())
		m_animConquerorLobby.Invoke("SetSwitchSpeciesCount", m_CVarSpeciesChangeLimit - m_speciesSwitchCount);
}

int CConquerorSystem::GetSpeciesFlagIndex(ESpeciesType species)
{
	return	m_speciesFlagIndexMap[species];
}

void CConquerorSystem::GetSpeciesClasses(ESpeciesType species, TClassesPtr& classes)
{
	classes = m_speciesClassesMap[species];
}

void CConquerorSystem::HUDSOMSetPointsCount(bool friendPoints, int count)
{
	if (m_animSwingOMeter.IsLoaded())
	{
		const SFlashVarValue args[2] = { (int)friendPoints, count };
		m_animSwingOMeter.Invoke("setPointsCount", args, 2);
	}
}

int CConquerorSystem::GetSpeciesFromEntity(IEntity* pEntity)
{
	if (!pEntity)
		return -1;

	if (const auto pAI = pEntity->GetAI())
	{
		const auto& playerParams = pAI->CastToIAIActor()->GetParameters();
		return playerParams.m_nSpecies;
	}
	else
	{
		auto species = -1;
		TOS_Script::GetEntityProperty(pEntity, "species", species);
		return species;
	}

	return -1;
}

void CConquerorSystem::HandleFSCommand(const char* command, const char* arguments)
{
	if (IsGamemode())
	{
		if (!strcmp(command, "WarningBox"))
			HandleWarningAnswer(arguments);

		if (!strcmp(command, "LobbyUpdateFaction"))
		{
			if (arguments)
			{
				const int speciesIndex = atoi(arguments);

				if (m_lobbySelectedSpeciesIndex != speciesIndex)
					m_lobbySelectedSpeciesIndex = speciesIndex;
			}				
		}

		if (!strcmp(command, "LobbySelectClass"))
		{
			if (arguments)
			{
				const int classIndex = atoi(arguments);

				if (m_lobbySelectedClassIndex != classIndex)
					m_lobbySelectedClassIndex = classIndex;

				OnLobbySetTeam(ESpeciesType(m_lobbySelectedSpeciesIndex), m_lobbySelectedClassIndex);
			}
		}

		if (!strcmp(command, "MPMap_SelectSpawnPoint") || !strcmp(command, "MPMap_SelectObjective"))
		{
			EntityId id = 0;
			if (arguments)
				id = EntityId(atoi(arguments));

			const ESpeciesType playerSpecies = GetClientSpecies();

			CStrategicArea* pArea = GetStrategicArea(id, 0);
			if (pArea)
			{
				const auto spawner = pArea->IsHaveFlag(EAreaFlag::SoldierSpawner);
				if (pArea->GetSpecies() == playerSpecies && spawner)
				{
					if (GetClientAreaEntity() != pArea->GetEntity())
						SetClientAreaEntity(pArea->GetEntity());

					g_pGame->GetHUD()->SetOnScreenObjective(pArea->GetEntityId());
				}
				else
				{
					g_pGame->GetHUD()->DisplayOverlayFlashMessage("This area can not be selected", ColorF(1.0f, 0.0f, 0));
				}
			}
		}
	}
}

void CConquerorSystem::HandleWarningAnswer(const char* warning)
{
	//CryLogAlways("[C++][Conqueror][Handle Warning Answer %s]", warning);
}

const char* CConquerorSystem::GetSpeciesName(ESpeciesType type) const
{
	const char* name = stl::find_in_map(m_speciesCharNameMap, type, "(Undefined Species Name)");

	return name;
}

ESpeciesType CConquerorSystem::GetSpeciesTypeFromString(const char* speciesName) const
{
	auto type = eST_NEUTRAL;

	for (auto& pair : m_speciesCharNameMap)
	{
		if (strcmp(pair.second, speciesName) == 0)
			type = pair.first;
	}

	return type;
}

CSpeciesClass* CConquerorSystem::GetClientSelectedClass()
{
	const auto playerSpecies = GetClientSpecies();
	const auto lobbyClassIdx = m_lobbyConfirmedClassIndex;

	return m_speciesClassesMap[playerSpecies][lobbyClassIdx];
}

CSpeciesClass* CConquerorSystem::GetDefaultClass()
{
	const auto playerSpecies = GetClientSpecies();

	return m_speciesDefaultClassesMap[playerSpecies];
}

//TClasses& CConquerorSystem::GetSpeciesClasses(ESpeciesType species)
//{
//	return m_speciesClassesMap[species];
//}

std::vector<EntityId> CConquerorSystem::GetSpeciesCapturableAreas(ESpeciesType species)
{
	std::vector<EntityId> areas;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea && (pArea->GetSpecies() == species))
			areas.push_back(pArea->GetEntityId());
	}

	return areas;
}

int CConquerorSystem::GetActorPoints(IEntity* pEntity)
{
	const auto pChannel = GetConquerorChannel(pEntity);
	if (pChannel)
		return pChannel->GetPoints();
}

void CConquerorSystem::AddPointsToActor(IEntity* pEntity, int value)
{
	const auto pChannel = GetConquerorChannel(pEntity);
	if (pChannel)
		pChannel->AddPoints(value);
}

bool CConquerorSystem::IsGamemode()
{
	return m_LobbyInfo.isConquest;
}

//void CConquerorSystem::GetSpeciesRandomClasses(ESpeciesType species, int needCount, bool excludeLeaderClass, TClasses& classes)
//{
//	if (!classes.empty())
//		classes.clear();
//
//	TClasses nonLeaderClasses;
//
//	for (auto& classInfo : m_speciesClassesMap[species])
//	{
//		if (!classInfo.IsLeaderClass() && excludeLeaderClass)
//			nonLeaderClasses.push_back(classInfo);
//	}
//
//	auto nonLeaderClassesCount = nonLeaderClasses.size();
//	if (nonLeaderClassesCount > 1)
//	{
//		for (auto i = 0; i < needCount; i++)
//		{
//			int randomInt = Random(0, nonLeaderClassesCount);
//			auto& randomNonLeaderClass = nonLeaderClasses[randomInt];
//
//			classes.push_back(randomNonLeaderClass);
//		}
//	}
//	else if (nonLeaderClassesCount == 1)
//	{
//		for (auto i = 0; i < needCount; i++)
//		{
//			for (auto& nonLeaderClass : nonLeaderClasses)
//			{
//				classes.push_back(nonLeaderClass);
//			}
//		}
//	}
//}

void CConquerorSystem::GetSpeciesRandomClasses(ESpeciesType species, uint flags, int classCount, TClassesPtr& classes)
{
	if (!classes.empty())
		classes.clear();

	TClassesPtr randomClasses;

	//static string debug = "[C++][GetSpeciesRandomClasses][Species %s][Class %s][check the flag:";

	for (CSpeciesClass* classInfo : m_speciesClassesMap[species])
	{
		//const auto classFlags = classInfo->GetFlags();

		//if (!(classFlags & flags))
		//	continue;

		if (flags & eSCF_LeaderClass)
		{
			//CryLogAlways(debug + "eSCF_LeaderClass][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_LeaderClass);

			if (!(classInfo->CheckFlag(eSCF_LeaderClass)))
				continue;
		}
		else// if(flags & eSCF_NonLeaderClass)
		{
			//CryLogAlways(debug + "eSCF_NonLeaderClass][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_NonLeaderClass);

			//if (!(classFlags & eSCF_NonLeaderClass))
			if ((classInfo->CheckFlag(eSCF_LeaderClass)))
				continue;
		}

		if (flags & eSCF_NeedHumanMode)
		{
			//CryLogAlways(debug + "eSCF_NeedHumanMode][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_NeedHumanMode);

			//if (!(classFlags & eSCF_NeedHumanMode))
			if (!(classInfo->CheckFlag(eSCF_NeedHumanMode)))
				continue;
		}

		if (flags & eSCF_OnlyThirdPerson)
		{
			//CryLogAlways(debug + "eSCF_OnlyThirdPerson][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_OnlyThirdPerson);

			//if (!(classFlags & eSCF_OnlyThirdPerson))
			if (!(classInfo->CheckFlag(eSCF_OnlyThirdPerson)))
				continue;
		}

		if (flags & eSCF_UnlockedByArea)
		{
			//CryLogAlways(debug + "eSCF_UnlockedByArea][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_UnlockedByArea);

			//if (!(classFlags & eSCF_UnlockedByArea))
			if (!(classInfo->CheckFlag(eSCF_UnlockedByArea)))
				continue;
		}

		if (flags & eSCF_UnlockedForPlayer)
		{
			//CryLogAlways(debug + "eSCF_PlayerCanChoose][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_UnlockedForPlayer);

			//if (!(classFlags & eSCF_UnlockedForPlayer))
			if (!(classInfo->CheckFlag(eSCF_UnlockedForPlayer)))
				continue;
		}

		if (flags & eSCF_UnlockedForAI)
		{
			//CryLogAlways(debug + "eSCF_AICanChoose][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_UnlockedForAI);

			//if (!(classFlags & eSCF_UnlockedForAI))
			if (!(classInfo->CheckFlag(eSCF_UnlockedForAI)))
				continue;
		}

		//if (flags & eSCF_UnlockedForAll)
		//{
		//	//CryLogAlways(debug + "eSCF_AllCanChoose][%i]", GetSpeciesName(species), classInfo.GetName(), classFlags & eSCF_UnlockedForAll);

		//	//if (!(classFlags & eSCF_UnlockedForAll))
		//	if (!(classInfo->CheckFlag(eSCF_UnlockedForAll)))
		//		continue;
		//}

		randomClasses.push_back(classInfo);
	}

	const auto count = randomClasses.size();
	if (count > 1)
	{
		for (auto i = 0; i < classCount; i++)
		{
			const int randomInt = Random(0, count);
			auto& randomClassInst = randomClasses[randomInt];

			classes.push_back(randomClassInst);
			//CryLogAlways("[C++][GetSpeciesRandomClasses][count > 1][push %s]", randomClassInst.GetName());
		}
	}
	else if (count == 1)
	{
		for (auto i = 0; i < classCount; i++)
		{
			for (auto& classInst : randomClasses)
			{
				classes.push_back(classInst);
				//CryLogAlways("[C++][GetSpeciesRandomClasses][count == 1][push %s]", classInst.GetName());
			}
		}
	}
	else
	{
		CryLogAlways("$4[C++][GetSpeciesRandomClasses][count == 0][push ERROR]");
	}
}

//void CConquerorSystem::GetSpeciesRandomLeaderClasses(ESpeciesType species, int leadersCount, TClasses& classes)
//{
//	if (!classes.empty())
//		classes.clear();
//
//	TClasses leaderClasses;
//
//	for (auto& classInfo : m_speciesClassesMap[species])
//	{
//		if (classInfo.IsLeaderClass())
//			leaderClasses.push_back(classInfo);
//	}
//
//	auto leaderClassesCount = leaderClasses.size();
//
//	if (leaderClassesCount > 1)
//	{
//		for (auto i = 0; i < leadersCount; i++)
//		{
//			int randomInt = Random(0, leaderClassesCount);
//			auto& randomLeaderClass = leaderClasses[randomInt];
//
//			classes.push_back(randomLeaderClass);
//			//CryLogAlways("[C++][Push Random Leader Class %s]", randomLeaderClass.GetName());
//		}
//	}
//	else if (leaderClassesCount == 1)
//	{
//		for (auto i = 0; i < leadersCount; i++)
//		{
//			for (auto& leaderClass : leaderClasses)
//			{
//				classes.push_back(leaderClass);
//				//CryLogAlways("[C++][Push Leader Class %s]", leaderClass.GetName());
//			}
//		}
//	}
//}

void CConquerorSystem::GetSpeciesTeammates(ESpeciesType species, std::vector<EntityId>& teammates, bool onlyAlive /*= false*/)
{
	auto it = m_conquerorChannels.begin();
	auto end = m_conquerorChannels.end();

	for (const auto pChannel : m_conquerorChannels)
	{
		if (pChannel->GetSpecies() != species)
			continue;

		const auto pActor = pChannel->GetActor();
		if (pActor)
		{
			if (onlyAlive && ((pActor->GetHealth() <= 0) || (pChannel->GetState() != eCCS_Alive)))
				continue;

			auto id = pActor->GetEntity()->GetId();
			teammates.push_back(id);
		}
	}
}

//int CConquerorSystem::GetLeadersCount(ESpeciesType species)
//{
//	return m_xmlAICountInfo.GetSquadsCount(species);
//}

void CConquerorSystem::SetSpeciesReinforcements(ESpeciesType species, int count)
{
	m_pXMLAICountInfo->speciesReinforcementCountMap[species] = max(0, count);

	const auto flagIndex = GetSpeciesFlagIndex(species);
	const auto flagUnits = GetSpeciesReinforcements(species);

	HUDSOMSetFlagUnitsCount(flagIndex, flagUnits);
}

void CConquerorSystem::AddSpeciesReinforcements(ESpeciesType species, int count, int multiplyer)
{
	if (m_gameOver)
		return;

	const auto currentCount = GetSpeciesReinforcements(species);
	const auto newCount = currentCount + (count * multiplyer);

	SetSpeciesReinforcements(species, newCount);
}

int CConquerorSystem::GetSpeciesReinforcements(ESpeciesType species) const
{
	return m_pXMLAICountInfo->GetReinforcementsCount(species);
}

int CConquerorSystem::GetMaxSpeciesReinforcements(ESpeciesType species) const
{
	return m_pXMLAICountInfo->GetMaxReinforcementsCount(species);
}

void CConquerorSystem::OnActorDeath(IActor* pActor)
{
	if (!pActor)
		return;

	auto* pChannel = GetConquerorChannel(pActor->GetEntity());
	if (!pChannel)
		return;

	pChannel->OnKilled();

	if (m_gameStatus != eGS_Battle)
		m_gameStatus = eGS_Battle;

	for (CConquerorCommander* pCommander : m_speciesCommanders)
		pCommander->OnActorDeath(pActor);

	const auto pExecutor = GetRAR()->GetExecutorInstance(pActor->GetEntityId());
	if (pExecutor)
		GetRAR()->CancelRequest(pExecutor->GetCurrentRequestId());

	if (pActor->GetEntityId() == g_pControlSystem->GetClientActor()->GetEntityId())
	{
		const auto count = g_pControlSystem->GetConquerorSystem()->GetSpeciesReinforcements(pChannel->GetSpecies());
		if (count > 0 && !pChannel->m_disableRespawnRequest)
		{
			const auto pEntity = GetClientAreaEntity();
			if (pEntity)
			{
				const auto pArea = GetStrategicArea(pEntity->GetId(), 0);
				if (pArea)
				{
					if (!IsInQueue(pActor))
					{
						auto pSpawnPoint = pArea->GetBookedSpawnPoint(pActor);
						if (!pSpawnPoint)
						{
							pSpawnPoint = pArea->BookFreeSpawnPoint(pActor, pChannel->GetClass()->IsAirClass());
						}

						if (pSpawnPoint)
						{
							const auto respawnTime = g_pControlSystem->GetConquerorSystem()->GetRespawnTime();

							if (!pArea->IsQueueCreated())
								pArea->CreateQueue(respawnTime);

							pArea->AddToQueue(pActor->GetEntityId(), eRC_OnKilledRespawn);
						}
					}
				}
			}
		}


		const auto currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		if ((currentTime - m_lastTimeAIReset) > 450) //7.5 min
		{
			ResetAllAI();
		}
	}
	else
	{
		const int count = g_pControlSystem->GetConquerorSystem()->GetSpeciesReinforcements(pChannel->GetSpecies());
		
		if ((count > 1 && pChannel->GetSpecies() == GetClientSpecies()) || (count > 0 && pChannel->GetSpecies() != GetClientSpecies()))
		{
			//const bool clientIsDead = GetClientConquerorChannel()->GetState() == eCCS_Dead || GetClientConquerorChannel()->GetState() == eCCS_Spectator;

			if (!pChannel->m_disableRespawnRequest)
			{
				if (pChannel->m_pForcedStrategicArea)
				{
					pChannel->SetSelectedStrategicArea(pChannel->m_pForcedStrategicArea);
				}
				else
				{
					if (pChannel->GetClass()->IsLeaderClass())
					{
						//Leader select random strategic area here
						auto randomVar = Random(0, 100);
						auto isTrue = randomVar < 90;

						if (isTrue)
						{
							randomVar = Random(0, 100);
							isTrue = randomVar < 20;

							if (isTrue)
							{
								pChannel->SetSelectedStrategicArea(GetBaseStrategicArea(pChannel->GetSpecies()));
							}
							else
							{
								std::vector<CStrategicArea*> areas;
								GetStrategicAreas(areas, pChannel->GetSpecies(), eAGSF_Enabled, pChannel->GetSpecies(), eABF_NoMatter, EAreaFlag::SoldierSpawner);
								pChannel->SetSelectedStrategicArea(TOS_STL::GetRandomFromSTL<TAreas, CStrategicArea*>(areas));
							}
						}
						else
						{
							pChannel->SetSelectedStrategicArea(GetNearestStrategicArea(pChannel->m_lastDeathPos, OWNED, eAGSF_Enabled, pChannel->GetSpecies(), eABF_NoMatter, EAreaFlag::SoldierSpawner));
						}
					}
					else
					{
						if (pChannel->GetLeaderStrategicArea())
						{
							//Members select last strategic area which selected by leader here
							pChannel->SetSelectedStrategicArea(pChannel->GetLeaderStrategicArea());
						}
						else
						{
							pChannel->SetSelectedStrategicArea(GetNearestStrategicArea(pChannel->m_lastDeathPos, OWNED, eAGSF_Enabled, pChannel->GetSpecies(), eABF_NoMatter, EAreaFlag::SoldierSpawner));
						}
					}
				}

				//Add actor to respawn queue here
				if (pChannel->GetSelectedArea())
				{
					RegisterToRespawn(pActor, pChannel->GetSelectedArea(), eRC_OnKilledRespawn);
				}
			}
		}
	}
}

void CConquerorSystem::OnActorGrabbed(IActor* pActor, EntityId grabberId)
{

}

void CConquerorSystem::OnActorDropped(IActor* pActor, EntityId grabberId)
{

}

void CConquerorSystem::OnActorGrab(IActor* pActor, EntityId grabId)
{
	if (m_pRARSystem)
		m_pRARSystem->OnActorGrab(pActor, grabId);
}

void CConquerorSystem::OnActorDrop(IActor* pActor, EntityId dropId)
{
	if (m_pRARSystem)
		m_pRARSystem->OnActorDrop(pActor, dropId);
}

void CConquerorSystem::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
	for (const auto pArea : m_strategicAreas)
		pArea->OnEnterVehicle(pActor, pVehicle);

	for (CConquerorCommander* pCommander : m_speciesCommanders)
		pCommander->OnEnterVehicle(pActor, pVehicle);

	const auto todHour = gEnv->p3DEngine->GetTimeOfDay()->GetTime();
	const auto value = (todHour > 18 || todHour < 6) ? 1 : 0;
		
	pVehicle->OnAction(eVAI_ToggleLights, eAAM_OnPress, value, pActor->GetEntityId());
}

void CConquerorSystem::OnExitVehicle(IActor* pActor)
{
	for (const auto pArea : m_strategicAreas)
		pArea->OnExitVehicle(pActor);

	for (CConquerorCommander* pCommander : m_speciesCommanders)
		pCommander->OnExitVehicle(pActor);
}

void CConquerorSystem::OnVehicleDestroyed(IVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	for (const auto pArea : m_strategicAreas)
		pArea->OnVehicleDestroyed(pVehicle);

	for (CConquerorCommander* pCmdr : m_speciesCommanders)
		pCmdr->OnVehicleDestroyed(pVehicle);
}

void CConquerorSystem::OnVehicleStuck(IVehicle* pVehicle, bool stuck)
{
	if (!pVehicle)
		return;

	for (CConquerorCommander* pCmdr : m_speciesCommanders)
		pCmdr->OnVehicleStuck(pVehicle, stuck);

	if (g_pGameCVars->conq_debug_log)
	{
		CryLogAlways("%s[C++][OnVehicleStuck][Vehicle: %s, value: %i]",
			STR_YELLOW, pVehicle->GetEntity()->GetName(), stuck);
	}
}

void CConquerorSystem::OnGoalPipeEvent(IEntity* pUserEntity, IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId)
{
	if (!pUserEntity || !pPipeUser)
		return;

	switch (event)
	{
	case ePN_OwnerRemoved:
		break;
	case ePN_Deselected:
		break;
	case ePN_Finished:
		break;
	case ePN_Suspended:
		break;
	case ePN_Resumed:
		break;
	case ePN_Removed:
		break;
	case ePN_AnimStarted:
		break;
	case ePN_RefPointMoved:
		break;
	}
}

void CConquerorSystem::OnLobbySetInfo(SConquerLobbyInfo& info)
{
	auto* pDude = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return;

	auto* pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	int showPlayerHUD = 0;

	if (info.state == EConquerLobbyState::IN_GAME)
	{
		if (!m_CVarBotsJoinBeforePlayer && !m_haveBotsSpawned)
		{
			//ResetAllAI();
			GameStartSpawnAI();

			m_haveBotsSpawned = true;
			m_botsSpawnedTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			for (CConquerorCommander* pCommander : m_speciesCommanders)
				pCommander->OnAIJoinGame();

			for (const auto pArea : m_strategicAreas)
				pArea->OnAIJoinGame();
		}
	}

	if (info.state == EConquerLobbyState::PRE_GAME ||
		info.state == EConquerLobbyState::IN_GAME)
	{
		if (gEnv->pMovieSystem)
			gEnv->pMovieSystem->StopSequence("Lobby_Cine");

		pHUD->FadeOutBigOverlayFlashMessage();

		showPlayerHUD = 1;
		m_animConquerorLobby.Reload();//need check this
		m_animConquerorLobby.SetVisible(false);
	}
	else
	{
		//IN LOBBY

		if (!info.isConquest)
			return;

		if (m_CVarBotsJoinBeforePlayer && !m_haveBotsSpawned)
		{
			//ResetAllAI();
			GameStartSpawnAI();

			m_haveBotsSpawned = true;
			m_botsSpawnedTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			for (CConquerorCommander* pCommander : m_speciesCommanders)
				pCommander->OnAIJoinGame();

			for (const auto pArea : m_strategicAreas)
				pArea->OnAIJoinGame();
		}

		if (const auto pEntity = gEnv->pEntitySystem->FindEntityByName("Lobby_playerPos"))
		{
			const auto pClientVehicle = TOS_Vehicle::GetVehicle(pDude);
			if (pClientVehicle)
				TOS_Vehicle::Exit(pDude, false, true);

			const auto desiredPlayerPos = pEntity->GetWorldPos();

			pDude->SetHealth(pDude->GetMaxHealth());
			//pDude->Revive(false);

			const auto pInventory = pDude->GetInventory();
			if (pInventory)
			{
				//FIX: fix double hands problem
				g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(pDude, EntityId(0), false);
				//pInventory->SetCurrentItem(0);
				pInventory->RemoveAllItems();
				//pInventory->ResetAmmo();
			}

			Matrix34 plMat;
			plMat = pDude->GetEntity()->GetWorldTM();
			plMat.SetTranslation(desiredPlayerPos);

			pDude->GetEntity()->SetWorldTM(plMat);

			auto* pChannel = GetConquerorChannel(pDude->GetEntity());
			if (pChannel)
				pChannel->OnSpectator(eSC_EnterLobby);

			if (IsInQueue(pDude))
				RemoveFromQueue(pDude);

			auto pClientActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_usedAlienId));
			if (!pClientActor)
				pClientActor = pDude;

			bool oldSpawned = false;
			CActor* pOldLeader = nullptr;

			const auto pOldRandomSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pClientActor);
			if (pOldRandomSquad)
			{
				pOldLeader = static_cast<CActor*>(pOldRandomSquad->RequestNewLeader(false, true));
				if (pOldLeader)
				{
					const auto pChannel = GetConquerorChannel(pOldLeader->GetEntity());
					if (pChannel)
					{
						const auto pSpawnPoint = GetPerfectFreeSpawnPoint(pOldLeader);
						if (pSpawnPoint)
						{
							pOldRandomSquad->SetLeader(pOldLeader, true);

							oldSpawned = pChannel->SpawnActor(eRC_ForcedRespawn, pSpawnPoint);
							pOldLeader->GetActorStats()->lastTimeRespawned = gEnv->pTimer->GetFrameStartTime().GetSeconds();

							if (pClientVehicle)
							{
								const int idx = TOS_Vehicle::RequestFreeSeatIndex(pClientVehicle);
								const auto pSeat = pClientVehicle->GetSeatById(idx);
								if (pSeat)
									pSeat->Enter(pOldLeader->GetEntityId(), false);
							}

							//pOldRandomSquad->ExecuteOrderAllMembers(eSO_FollowLeader, eEOF_ExecutedByAI);

							if (m_isDebugLog)
								CryLogAlways("[C++][Old Random Squad %i Setup New Leader %s]", pOldRandomSquad->GetId(), pOldLeader->GetEntity()->GetName());
						}
					}
				}
			}

			if (!oldSpawned && pOldLeader && pOldLeader->GetEntity())
			{
				CryLogAlways("[C++][ERROR][Old Random Squad %i New Leader %s can not spawn]", 
					pOldRandomSquad->GetId(), pOldLeader->GetEntity()->GetName());
			}

			if (m_usedAlienId)
			{
				const auto pAlienEntity = gEnv->pEntitySystem->GetEntity(m_usedAlienId);

				const auto pAbilityOwner = g_pControlSystem->GetAbilitiesSystem()->GetAbilityOwner(m_usedAlienId);
				if (pAbilityOwner)
				{
					for (int i = 0; i <= pAbilityOwner->GetAbilityCount(); i++)
					{
						const auto pAbility = pAbilityOwner->GetAbility(i);
						if (pAbility)
							pAbilityOwner->DeactivateAbility(pAbility);
					}
				}

				auto it = m_strategicAreas.begin();
				const auto end = m_strategicAreas.end();
				for (; it != end; it++)
				{
					const auto pArea = *it;
					if (pArea->IsActorInside(pAlienEntity->GetId()))
						pArea->RemoveFromArea(pAlienEntity->GetId());
				}

				g_pControlSystem->StopLocal(false);

				Matrix34 conMat;
				conMat = pAlienEntity->GetWorldTM();
				conMat.SetTranslation(Vec3(ZERO));

				pAlienEntity->SetWorldTM(conMat);

				//m_pSpawnedAlienPlayer = nullptr;
				m_usedAlienId = 0;
			}
		}

		if (gEnv->pMovieSystem)
			gEnv->pMovieSystem->PlaySequence("Lobby_Cine", false);

		//The Lobby character update
		m_lobbySelectedClassIndex = m_lobbyConfirmedClassIndex = 0;
		m_lobbySelectedSpeciesIndex = m_lobbyConfirmedSpeciesIndex = GetClientSpecies();
		OnLobbySetTeam((ESpeciesType)m_lobbyConfirmedSpeciesIndex, m_lobbyConfirmedClassIndex);
	
		//HUD
		HUDLobbyUpdateSpeciesMenu(true, false);
		showPlayerHUD = 0;

		bool disableSpecies = false;
		//bool showHints = false;

		gEnv->p3DEngine->ResetPostEffects();

		if (m_CVarSpeciesChangeLimit <= 0)
		{
			HUDLobbyMenuShow(true);
			disableSpecies = true;
			//showHints = true;
		}
		else if (!CanSwitchSpecies(pDude->GetEntity()) && m_haveBotsSpawned)
		{
			const auto isClientWinner = GetClientSpecies() == m_winnerSpecies;

			if (!isClientWinner && !m_gameOver)
			{
				pHUD->DisplayOverlayFlashMessage("@conq_team_change_limit", ColorF(1.0f, 0.0f, 0));
			}
			else if (isClientWinner)
			{
				pHUD->DisplayOverlayFlashMessage("@conq_you_win", ColorF(0.0f, 1.0f, 0));
			}
			else if (!isClientWinner && m_gameOver)
			{
				g_pGame->GetHUD()->DisplayTimedOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", 8.0f, ColorF(1.0f, 1.0f, 0.0f));
			}

			if (!m_gameOver)
			{
				if (GetSpeciesReinforcements(GetClientSpecies()) <= 10)
				{
					SetSpeciesChangeLimit(0);
					pHUD->DisplayBigOverlayFlashMessage("@conq_reinf_less_ten", 8.0f);
				}
			}

			//HUDLobbyMenuShow(false);
			disableSpecies = true;
		}
		else
		{
			char times[4];
			sprintf(times, "%i", m_CVarSpeciesChangeLimit - m_speciesSwitchCount);

			pHUD->DisplayOverlayFlashMessage("@conq_you_can_switch_team_count", ColorF(0.0f, 1.0f, 0), true, times);

			HUDLobbyMenuShow(true);

			//showHints = true;
		}

		if (disableSpecies && m_haveBotsSpawned)
		{
			std::vector<ESpeciesType> speciesToRemove;
			for (auto species : m_lobbyAllowedSpecies)
			{
				if (species == GetClientSpecies())
					continue;

				stl::push_back_unique(speciesToRemove, species);
			}

			for (const auto species : speciesToRemove)
				HUDLobbyEnableSpecies(species, false);
		}

		//if (showHints && !m_gameOver)
			//pHUD->DisplayBigOverlayFlashMessage("Press M to Select Spawn Point", 15.0f, 400, 450);
	}

	pHUD->m_animRadarCompassStealth.SetVisible(showPlayerHUD);
	pHUD->m_animPlayerStats.SetVisible(showPlayerHUD);
	pHUD->m_pHUDCrosshair->SetOpacity(showPlayerHUD);
}

void CConquerorSystem::OnLobbySetTeam(ESpeciesType speciesIndex, int classIndex)
{
	IEntity* pAnimEntity = gEnv->pEntitySystem->GetEntity(m_LobbyInfo.modelEntityId);
	if (!pAnimEntity)
		return;

	IScriptTable* pAnimTable = pAnimEntity->GetScriptTable();
	if (!pAnimTable)
		return;

	pAnimEntity->Hide(false);

	const CGameFlashAnimation* pAnim = &m_animConquerorLobby;
	if (!pAnim->IsLoaded())
		return;

	SmartScriptTable props;
	SmartScriptTable animation;

	//Now stored in m_lobbyInfo when FG init
	//static const Vec3 humanAnimPos = pAnimEntity->GetWorldPos();

	const float nakedAlienZOffset = 2.0f;

	if (pAnimTable->GetValue("Properties", props) && props->GetValue("Animation", animation))
	{
		//Idle Animations
		//string humanIdleAnim = "mpHUD_koreanSoldierSelectionIdle_04";
		//string alienTrooperIdleAnim = "trooper_idleVert_02";
		//string alienNakedIdleAnim = "alien_flyProneIdle_fast_01";
		//string alienScoutIdleAnim = "fly_idle";

		Matrix34 animMat34;
		animMat34 = pAnimEntity->GetWorldTM();
		animMat34.SetTranslation(m_LobbyInfo.modelPos);

		pAnimEntity->SetWorldTM(animMat34);
		pAnimEntity->SetScale(m_LobbyInfo.modelScale);

		const CSpeciesClass* classInfo = m_speciesClassesMap[speciesIndex][classIndex];
		const auto& classModel = classInfo->m_model;

		props->SetValue("object_Model", classModel.m_character.c_str());

		animation->SetValue("Animation", classModel.m_lobbyAnim.c_str());
		animation->SetValue("bPlaying", 1);
		animation->SetValue("bLoop", 1);

		const auto scale = classModel.m_scale;
		const auto offset = classModel.m_worldOffset;

		const auto newPos = m_LobbyInfo.modelPos + offset;
		const auto newScale = Vec3(1, 1, 1) * scale;

		animMat34.SetTranslation(newPos);
		pAnimEntity->SetWorldTM(animMat34);
		pAnimEntity->SetScale(newScale);

		Script::CallMethod(pAnimTable, "SetFromProperties");
	}

	auto* pDude = g_pGame->GetIGameFramework()->GetClientActor();
	if (pDude)
	{
		TOS_AI::SetSpecies(pDude->GetEntity()->GetAI(), speciesIndex);

		if (auto* pChannel = GetConquerorChannel(pDude->GetEntity()))
			pChannel->SetSpecies(speciesIndex);
		
		if (IsInQueue(pDude))
			RemoveFromQueue(pDude);
	}

	if (m_pSelectedClientAreaEntity)
	{
		const ESpeciesType playerSpecies = GetClientSpecies();

		const CStrategicArea* pPlayerArea = GetStrategicArea(m_pSelectedClientAreaEntity->GetId(), 0);
		if (pPlayerArea && pPlayerArea->GetSpecies() != playerSpecies)
		{
			m_pSelectedClientAreaEntity = nullptr;
			SAFE_HUD_FUNC(SetOnScreenObjective(0));
		}
	}

	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		const bool isVisible = pHUD->m_animPDA.GetVisible();
		if (isVisible)
		{
			pHUD->ShowPDA(false);
			pHUD->ShowPDA(true);
		}

	}

	UpdateHUDSOM(true);
}

//void CConquerorSystem::SpawnAIBotsOld()
//{
//	std::vector<EntityId> restrictedAreas;
//	std::vector<EntityId> aiLeadersIds;
//
//	//spawn leaders first
//	auto it = m_gameAllowedSpecies.begin();
//	auto end = m_gameAllowedSpecies.end();

	//for (; it != end; it++)
	//{
	//	auto speciesType = *it;

	//	const int squadsCount = m_xmlAICountInfo.GetSquadsCount(speciesType);
	//	const int areasCount = GetStrategicAreaCount(speciesType, eAGSF_Enabled);
	//	const int remain = squadsCount % areasCount;

	//	int count = (squadsCount / areasCount);
	//	int unitsCount = m_xmlAICountInfo.GetUnitsCount(speciesType);

	//	if (remain != 0)
	//		count++;

	//	if (GetClientSpecies() == speciesType)
	//		count++;

	//	auto it = m_conquerorChannels.begin();
	//	auto end = m_conquerorChannels.end();

	//	for (; it != end; it++)
	//	{
	//		auto pChannel = *it;

	//		if (pChannel->GetSpecies() != speciesType)
	//			continue;

	//		auto pClass = pChannel->GetClass();
	//		if (!pClass)
	//			continue;

	//		const auto isLeader = pClass->IsLeaderClass();

	//		if (!pChannel->IsPlayer() && isLeader)
	//		{
	//			//auto pSpawnArea = pChannel->RequestStrategicArea(false, &restrictedAreas);
	//			if (pSpawnArea)
	//			{
	//				const auto spawnsCount = pSpawnArea->GetSpawnPointCount();

	//				if (spawnsCount <= unitsCount)
	//				{
	//					if (m_isDebugLog)
	//						CryLogAlways("%s[C++][Strategic Area][ERROR][Species %i][Can not spawn %s][Cause: Count of strategic area spawn points <= Count of units to spawn]", 
	//							STR_RED, speciesType, pChannel->GetName());
	//					unitsCount--;

	//					continue;
	//				}

	//				auto pSpawnPoint = pSpawnArea->GetSpawnPointAt(unitsCount);
	//				if (pSpawnPoint)
	//				{
	//					if (pChannel->SpawnActor(eRC_OnGameStartSpawn, pSpawnPoint))
	//					{
	//						aiLeadersIds.push_back(pChannel->GetEntity()->GetId());
	//					}

	//					if (areasCount > 1)
	//					{
	//						++m_areaSpawnedLeadersCountMap[pSpawnArea->GetEntityId()];

	//						if (m_areaSpawnedLeadersCountMap[pSpawnArea->GetEntityId()] >= count)
	//							restrictedAreas.push_back(pSpawnArea->GetEntityId());
	//					}
	//					else
	//					{
	//						if (m_isDebugLog)
	//							CryLogAlways("[C++][Species %i][Leaders spawned at one SpawnArea]", speciesType);
	//					}

	//				}
	//				else
	//				{
	//					if (m_isDebugLog)
	//						CryLogAlways("[C++][Channel %i][FAIL FIND SpawnPoint to spawning]", pChannel->m_id);
	//				}
	//			}
	//			else
	//			{
	//				if (m_isDebugLog)
	//					CryLogAlways("[C++][Channel %i][FAIL FIND SpawnArea to spawning]", pChannel->m_id);
	//			}

	//			unitsCount--;
	//		}
	//	}
	//}

	////spawn squad members on leader's spawn area
	//for (auto id : aiLeadersIds)
	//{
	//	auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	//	if (pActor)
	//	{
	//		auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pActor);
	//		if (pSquad)
	//		{
	//			for (auto& member : pSquad->GetAllMembers())
	//			{
	//				auto pEntity = gEnv->pEntitySystem->GetEntity(member.GetId());
	//				if (!pEntity)
	//					continue;

	//				auto pChannel = GetConquerorChannel(pEntity);
	//				if (!pChannel)
	//					continue;

	//				auto pArea = pChannel->GetLeaderStrategicArea();
	//				if (pArea)
	//					pChannel->SpawnActor(eRC_OnGameStartSpawn, pArea->GetSpawnPoint(eSGSF_NotHaveRecentlySpawned));

	//				//member.SetCurrentOrder(eSO_FollowLeader);
	//				//pSquad->ExecuteOrder(eSO_FollowLeader, &member, eEOF_ExecutedByAI);
	//			}
	//		}
	//	}
	//}
//}

void CConquerorSystem::GameStartSpawnAI()
{
	//std::vector<EntityId> restrictedAreas;
	std::vector<EntityId> aiLeadersIds;
	std::map<CStrategicArea*, int> spawnedSquadsCount;


	//spawn leaders first here
	auto it = m_gameAllowedSpecies.begin();
	const auto end = m_gameAllowedSpecies.end();

	for (; it != end; it++)
	{
		const auto speciesType = *it;

		int squadsCount = m_pXMLAICountInfo->GetSquadsCount(speciesType);
		const int unitsCount = m_pXMLAICountInfo->GetUnitsCount(speciesType);

		if (GetClientSpecies() == speciesType)
			squadsCount--;

		//const int areasCount = GetStrategicAreaCount(speciesType, eAGSF_Enabled);
		//const int remain = squadsCount % areasCount;

		//int count = (squadsCount / areasCount);
		//int unitsCount = m_pXMLAICountInfo->GetUnitsCount(speciesType);

		//if (remain != 0)
			//count++;

		//if (GetClientSpecies() == speciesType)
			//count++;

		auto it = m_conquerorChannels.begin();
		auto end = m_conquerorChannels.end();

		for (; it != end; it++)
		{
			const auto pChannel = *it;

			const auto pActor = pChannel->GetActor();
			if (!pActor)
				continue;

			if (pChannel->GetSpecies() != speciesType)
				continue;

			const auto pClass = pChannel->GetClass();
			if (!pClass)
				continue;

			const auto isLeader = pClass->IsLeaderClass();
			auto isSuccessSpawned = false;

			if (!pChannel->IsPlayer() && isLeader)
			{
				TAreas areas;
				GetStrategicAreas(areas, speciesType, eAGSF_Enabled, speciesType, eABF_NoMatter, EAreaFlag::SoldierSpawner);
				
				TAreas filteredAreas;

				//Find all areas who can spawn all units at game start
				for (auto pArea : areas)
				{
					//const auto queueIsCreated = pArea->IsQueueCreated();
					//const auto queueSize = pArea->GetQueueSize();
					const auto spawnsCount = pArea->GetSpawnPointCount();

					//Here unitsCount is primary
					if (unitsCount >= spawnsCount)
					{
						CryLogAlways("%s[C++][Strategic Area][ERROR][Species %i][Can not spawn %s][Cause: count of spawn points <= respawn queue size]",
							STR_RED, speciesType, pChannel->GetName());

						continue;
					}

					filteredAreas.push_back(pArea);
				}

				const int areasCount = filteredAreas.size();
				const float squadsPerArea = round(squadsCount / areasCount);

				for (auto pArea : filteredAreas)
				{
					if (spawnedSquadsCount[pArea] < squadsPerArea)
						continue;

					auto pSpawnPoint = pArea->GetBookedSpawnPoint(pActor);
					if (!pSpawnPoint)
						pSpawnPoint = pArea->BookFreeSpawnPoint(pActor, pClass->IsAirClass());

					pChannel->SetSelectedStrategicArea(pArea);
					const auto spawned = pChannel->SpawnActor(eRC_OnGameStartSpawn, pSpawnPoint);

					if (pSpawnPoint && spawned)
					{
						aiLeadersIds.push_back(pChannel->GetEntity()->GetId());
						spawnedSquadsCount[pArea]++;
						isSuccessSpawned = true;
					}
					else
					{
						if (!pSpawnPoint)
						{
							CryLogAlways("%s[C++][ERROR][Channel %i][FAIL FIND SpawnPoint to spawning]",
								STR_RED, pChannel->m_id);
						}
						else if (!spawned)
						{
							CryLogAlways("%s[C++][ERROR][Channel %i][FAIL Channel Spawning][See CConquerorChannel::SpawnActor for more info]",
								STR_RED, pChannel->m_id);
						}
					}
				}

				//If exist not spawned leader
				//We need to spawn it on random filtered area
				if (!isSuccessSpawned)
				{
					CStrategicArea* pArea = TOS_STL::GetRandomFromSTL<TAreas, CStrategicArea*>(filteredAreas);
					if (pArea)
					{
						auto pSpawnPoint = pArea->GetBookedSpawnPoint(pActor);
						if (!pSpawnPoint)
							pSpawnPoint = pArea->BookFreeSpawnPoint(pActor, pClass->IsAirClass());

						pChannel->SetSelectedStrategicArea(pArea);
						const auto spawned = pChannel->SpawnActor(eRC_OnGameStartSpawn, pSpawnPoint);

						if (pSpawnPoint && spawned)
						{
							aiLeadersIds.push_back(pChannel->GetEntity()->GetId());
							isSuccessSpawned = true;
						}
						else
						{
							if (!pSpawnPoint)
							{
								CryLogAlways("%s[C++][ERROR][Channel %i][FAIL FIND SpawnPoint to spawning]",
									STR_RED, pChannel->m_id);
							}
							else if (!spawned)
							{
								CryLogAlways("%s[C++][ERROR][Channel %i][FAIL Channel Spawning][See CConquerorChannel::SpawnActor for more info]",
									STR_RED, pChannel->m_id);
							}
						}
					}
					else
					{
						CryLogAlways("%s[C++][ERROR][Channel %i][FAIL FIND Area to spawning]",
							STR_RED, pChannel->m_id);
					}
				}
			}
		}
	}

	//spawn squad members on leader's spawn area here
	for (const auto id : aiLeadersIds)
	{
		const auto pLeader = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
		if (pLeader)
		{
			const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pLeader);
			if (pSquad)
			{
				for (auto& member : pSquad->GetAllMembers())
				{
					const auto pMemberActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
					if (!pMemberActor)
						continue;

					const auto pChannel = GetConquerorChannel(pMemberActor->GetEntity());
					if (!pChannel)
						continue;

					const auto pArea = pChannel->GetLeaderStrategicArea();
					if (!pArea)
						continue;

					auto pSpawnPoint = pArea->GetBookedSpawnPoint(pMemberActor);
					if (!pSpawnPoint)
						pSpawnPoint = pArea->BookFreeSpawnPoint(pMemberActor, pChannel->GetClass()->IsAirClass());

					if (pSpawnPoint)
					{
						pChannel->SpawnActor(eRC_OnGameStartSpawn, pSpawnPoint);
					}
					else
					{
						CryLogAlways("%s[C++][ERROR][Channel %i][FAIL FIND SpawnPoint to spawning]",
							STR_RED, pChannel->m_id);
					}
				}
			}
		}
	}
}

void CConquerorSystem::ResetAllAI()
{
	const auto pIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while (const auto pActor = static_cast<CActor*>(pIt->Next()))
	{
		const auto pAI = pActor->GetEntity()->GetAI();
		if (!pAI)
			continue;

		TOS_AI::ReturnToFirstSimple(pAI, false);

		if (!TOS_Vehicle::ActorInVehicle(pActor))
		{
			const auto pAnimChar = pActor->GetAnimatedCharacter();
			if (pAnimChar)
			{
				pAnimChar->ResetState();
				//pAnimChar->GetAnimationGraphState()->SetInput("Action", "idle");
				//pAnimChar->GetAnimationGraphState()->SetInput("waterLevel", 0);
				//pAnimChar->SetParams(pAnimChar->GetParams().ModifyFlags(eACF_EnableMovementProcessing, 0));
			}

			g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(pActor, (EntityId)0, false);
			pActor->SelectItem(0, false);

			const auto pInventory = pActor->GetInventory();
			if (pInventory)
			{
				pInventory->SetCurrentItem(0);
				pInventory->Destroy();
				pInventory->Clear();
			}

			//g_pGame->GetIGameFramework()->GetIItemSystem()->GetIEquipmentManager()->GiveEquipmentPack(pActor, pActor->GetEquipmentPackName(), false, true);

			const auto pChannel = GetConquerorChannel(pActor->GetEntity());
			if (pChannel)
			{
				TOS_Inventory::GiveEquipmentPack(pActor, pChannel->GetClass()->GetEquipment().m_equipPack);
				TOS_Inventory::SelectPrimary(pActor);
			}

			if (pActor->IsAlien())
			{
				const auto pScout = static_cast<CScout*>(pActor);
				pScout->InitSearchBeam();
			}
		}
	}

	const auto pVehIt = g_pGame->GetIGameFramework()->GetIVehicleSystem()->CreateVehicleIterator();
	while (const auto pVeh = pVehIt->Next())
	{
		const auto pAI = pVeh->GetEntity()->GetAI();
		if (!pAI)
			continue;

		TOS_AI::ReturnToFirstSimple(pAI, false);
		TOS_AI::CancelSubpipe(pAI, 0);
	}


	m_lastTimeAIReset = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	//if (m_isDebugLog)
		CryLogAlways("%s[C++][Conqueror System][Reset all ai agents]", STR_YELLOW);
}

CRequestsAndResponses* CConquerorSystem::GetRAR()
{
	return m_pRARSystem;
}

CConquerorShop* CConquerorSystem::GetShop()
{
	return m_pShop;
}

CSpawnPoint* CConquerorSystem::GetPerfectFreeSpawnPoint(IActor* pActor)
{
	if (!pActor)
		return nullptr;

	const auto pChannel = GetConquerorChannel(pActor->GetEntity());
	if (!pChannel)
		return nullptr;

	const auto isAir = pChannel->GetClass()->IsAirClass();
	const auto actorSpecies = ESpeciesType(GetSpeciesFromEntity(pActor->GetEntity()));

	TAreas areas;
	GetStrategicAreas(areas, actorSpecies, eAGSF_Enabled, actorSpecies, eABF_NoMatter, EAreaFlag::SoldierSpawner);
	for (const auto pArea : areas)
	{
		//std::vector<CSpawnPoint> spawnpoints = isAir ? pArea->m_airSpawnPoints : pArea->m_spawnPoints;

		for (const auto pSpawnpoint : pArea->m_spawnPoints)
		{
			if (isAir)
			{
				if (!pSpawnpoint->IsForAir())
					continue;
			}

			if (pArea->IsBookedSpawnPoint(pSpawnpoint->GetEntityId()))
				continue;

			if (pSpawnpoint->IsRecentlySpawned())
				continue;

			return pSpawnpoint;
		}
	}

	return nullptr;
}

bool CConquerorSystem::RegisterToRespawn(IActor* pActor, CStrategicArea* pArea, ERespawnEvent event)
{
	if (!pArea || !pActor)
		return false;

	auto isRegistered = false;

	for (const auto& preResPair : m_preRespawns)
	{
		if (preResPair.first.actorId == pActor->GetEntityId())
		{
			isRegistered = true;
			break;
		}
	}

	if (!isRegistered)
		m_preRespawns[SPreRespawnData(pActor->GetEntityId(), pArea->GetEntityId(), event)] = 3.0f;

	//auto pChannel = GetConquerorChannel(pActor->GetEntity());
	//if (!pChannel)
	//	return false;

	//if (!pArea->GetBookedSpawnPoint(pActor))
	//{
	//	const auto isAir = pChannel->GetClass()->IsAirClass();
	//	pArea->BookFreeSpawnPoint(pActor, isAir);
	//}	

	//pChannel->SetSelectedStrategicArea(pArea);

	//if (!pArea->IsQueueCreated())
	//	pArea->CreateQueue(GetRespawnTime());

	//pArea->AddToQueue(pActor->GetEntityId(), event);

	//return true;
}

bool CConquerorSystem::IsInQueue(IActor* pActor)
{
	if (!pActor)
		return false;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea->IsInQueue(pActor->GetEntityId()))
			return true;
	}

	return false;
}

bool CConquerorSystem::RemoveFromQueue(const IActor* pActor)
{
	if (!pActor)
		return false;

	for (const auto pArea : m_strategicAreas)
	{
		if (pArea->IsInQueue(pActor->GetEntityId()))
			return pArea->EraseFromQueue(pActor->GetEntityId());
	}

	return false;
}

bool CConquerorSystem::CopyQueue(CStrategicArea* pAreaSource, CStrategicArea* pAreaDestination)
{
	if (!pAreaSource || !pAreaDestination)
		return false;

	if (pAreaSource->GetQueueSize() > 0)
	{
		//Cause crash
		//std::copy(pAreaSource->GetQueueBeginIt(), pAreaSource->GetQueueEndIt(), pAreaDestination->GetQueueBeginIt());
			
		auto it = pAreaSource->GetQueueBeginIt();
		const auto end = pAreaSource->GetQueueEndIt();
		for (; it != end; it++)
			pAreaDestination->AddToQueue(it->entityId, it->event);

		return true;
	}

	return false;
}

bool CConquerorSystem::IsBookedVehicle(IVehicle* pVehicle) const
{
	for (CConquerorCommander* pCmdr : m_speciesCommanders)
	{
		if (pCmdr->IsBookedVehicle(pVehicle))
			return true;
	}

	return false;
}

//ESpeciesType CConquerorSystem::GetSpeciesFromSquad(CSquad* pSquad)
//{
//	/*auto it = m_speciesSquadsMap.begin();
//	auto end = m_speciesSquadsMap.end();
//
//	for (; it!=end;it++)
//	{
//		auto it2 = it->second.begin();
//		auto end2 = it->second.end();
//
//		for (;it2!=end2;it2++)
//		{
//			if (it2->GetId() == pSquad->GetId())
//				return it->first;
//		}
//	}*/
//}

//uint CConquerorSystem::GetSpeciesAllowed(bool onInit)
//{
//	if (onInit)
//	{
//		uint allowedOnInit = 0;
//
//		for (int i = eST_FirstPlayableSpecies; i <= eST_LastPlayableSpecies; i++)
//		{
//			//auto it = m_CapturableAreas.begin();
//			//auto end = m_CapturableAreas.end();
//
//			auto species = ESpeciesType(i);
//
//			auto pArea = GetCapturableArea(species, true);
//			if (pArea /*&& pArea->IsCapturable() == false*/)
//			{
//				if (species == ESpeciesType::eST_Aliens)
//				{
//					allowedOnInit |= EAllowedSpecies::eAS_Aliens;
//				}
//				if (species == ESpeciesType::eST_USA)
//				{
//					allowedOnInit |= EAllowedSpecies::eAS_USA;
//				}
//				if (species == ESpeciesType::eST_NK)
//				{
//					allowedOnInit |= EAllowedSpecies::eAS_NK;
//				}
//				if (species == ESpeciesType::eST_CELL)
//				{
//					allowedOnInit |= EAllowedSpecies::eAS_CELL;
//				}
//			}
//		}
//
//		return allowedOnInit;
//	}
//
//	return 0;
//}

bool CConquerorSystem::IsSpeciesAllowed(ESpeciesType species, bool inLobby)
{
	if (!inLobby)
		return stl::find(m_gameAllowedSpecies, species);
	else
		return stl::find(m_lobbyAllowedSpecies, species);
}

void CConquerorSystem::AddConquerorClass(ESpeciesType species, CSpeciesClass* pConquerorClass, bool isDefaultClass /*= false*/)
{
	if (isDefaultClass)
	{
		m_speciesDefaultClassesMap[species] = pConquerorClass;
	}
	else
	{
		m_speciesClassesMap[species].push_back(pConquerorClass);
	}

	if (g_pGameCVars->conq_debug_log_classes)
	{
		CryLogAlways("$3[Conqueror System][Add Class][Name %s][Species %s]", 
			pConquerorClass->GetName(), GetSpeciesName(species));
		CryLogAlways("	$3[SIZE] %i", sizeof(pConquerorClass));
		CryLogAlways("	$3[Only ThirdPerson] %i", pConquerorClass->IsOnlyThirdPerson());
		CryLogAlways("	$3[Is Leader] %i", pConquerorClass->IsLeaderClass());
		CryLogAlways("	$3[Is Air] %i", pConquerorClass->IsAirClass());
		CryLogAlways("	$3[Abilities Count] %i", pConquerorClass->GetAbilities().size());
		CryLogAlways("	$3[Model Arms] %s", pConquerorClass->GetModel().m_arms);
		CryLogAlways("	$3[Model ArmsMat] %s", pConquerorClass->GetModel().m_armsMat);
		CryLogAlways("	$3[Model Character] %s", pConquerorClass->GetModel().m_character);
		CryLogAlways("	$3[Model FP3P] %s", pConquerorClass->GetModel().m_fp3p);
		CryLogAlways("	$3[Model HelmetMat] %s", pConquerorClass->GetModel().m_helmetMat);
		CryLogAlways("	$3[Model Anim] %s", pConquerorClass->GetModel().m_lobbyAnim);
		CryLogAlways("	$3[Model Mat] %s", pConquerorClass->m_model.m_mat);
		CryLogAlways("	$3[Model Offset] (%1.f,%1.f,%1.f)", 
			pConquerorClass->m_model.m_worldOffset.x,
			pConquerorClass->m_model.m_worldOffset.y,
			pConquerorClass->m_model.m_worldOffset.z);
		CryLogAlways("	$3[Model Scale] (%f)", pConquerorClass->m_model.m_scale);
		CryLogAlways("	$3[Equip Pack] %s", pConquerorClass->GetEquipment().m_equipPack);
		//CryLogAlways("	[Equip Primary] %s", pConquerorClass->GetEquipment().m_primaryWeapon);
		CryLogAlways("	$3[AI Archetype] %s", pConquerorClass->GetAI().m_archetype);
		CryLogAlways("	$3[AI Behaviour] %s", pConquerorClass->GetAI().m_behaviour);
		CryLogAlways("	$3[AI Character] %s", pConquerorClass->GetAI().m_character);
		CryLogAlways("//////////////////////////////////////////////////////////////");
	}
}

void CConquerorSystem::RemoveAllClasses()
{
	//smart ptr crash fix
	if (m_speciesDefaultClassesMap.size())
	{
		m_speciesDefaultClassesMap.clear();
	}

	if (m_speciesClassesMap.size())
	{
		m_speciesClassesMap.clear();
	}
}

int CConquerorSystem::GetRandomSquadId(ESpeciesType species)
{
	const std::vector<int> squadIds = g_pControlSystem->GetSquadSystem()->GetSquadIdsFromSpecies(species, false);
	if (squadIds.size() > 0)
		return TOS_STL::GetRandomFromSTL<std::vector<int>, int>(squadIds);

	//Причина комментирования: Иногда отряд могло выдать из другой фракции
	//if (squadIds.size() != 0)
	//{
	//	auto minimumId = squadIds[0];
	//	auto maximumId = squadIds[0];

	//	for (auto id : squadIds)
	//	{
	//		if (id < minimumId)
	//			minimumId = id;

	//		if (id > maximumId)
	//			maximumId = id;
	//	}

	//	if (m_isDebugLog)
	//		CryLogAlways("[C++][Get Random Squad Id][Min %i][Max %i]", minimumId, maximumId);

	//	int value = Random(minimumId, maximumId);
	//	return value;
	//}
	//
	//if (m_isDebugLog)
	//	CryLogAlways("[C++][Get Random Squad Id FAILED][Cause: No have squads with this species]");

	return -1;
}

//void CConquerorSystem::RemoveAllSpeciesSquads()
//{
//	m_speciesSquadsMap.clear();
//}

CSquad* CConquerorSystem::CreateSquadFromSpecies(ESpeciesType species)
{
	const auto pCreated = g_pControlSystem->GetSquadSystem()->CreateSquad();
	if (!pCreated)
		return nullptr;

	pCreated->SetSpecies(species);

	return pCreated;
}

void CConquerorSystem::SetClientArea(CStrategicArea* area)
{
	m_pClientArea = area;
}

CStrategicArea* CConquerorSystem::GetClientArea()
{
	return m_pClientArea;
}

int CConquerorSystem::GetCommandersCount() const
{
	return m_speciesCommanders.size();
}

int CConquerorSystem::GetSpeciesChangeLimit() const
{
	return m_CVarSpeciesChangeLimit;
}

int CConquerorSystem::GetSpeciesChangeCount() const
{
	return m_speciesSwitchCount;
}

void CConquerorSystem::ForceKillSpecies(ESpeciesType species)
{
	SetSpeciesReinforcements(species, 0);

	TAreas areas;
	GetStrategicAreas(areas, species, EAreaGameStatusFlag::eAGSF_NoMatter, species, EAreaBusyFlags::eABF_NoMatter, EAreaFlag::SoldierSpawner);
	for (const auto pArea : areas)
		pArea->DeleteQueue();

	const auto count = GetConquerorChannelsCount();
	for (int i = 0; i < count; ++i)
	{
		const auto pChannel = GetConquerorChannel(i);
		if (pChannel)
		{
			if (pChannel->GetSpecies() != species)
				continue;

			const auto pActor = pChannel->GetActor();
			if (pActor)
			{
				if (!pActor->IsPlayer())
				{
					IScriptTable* pTable = pChannel->GetEntity()->GetScriptTable();
					if (pTable)
						Script::CallMethod(pTable, "Event_Kill");
				}
				else
				{
					const auto pGameRules = g_pGame->GetGameRules();
					if (pGameRules)
					{
						HitInfo suicideInfo(pActor->GetEntityId(), pActor->GetEntityId(), pActor->GetEntityId(),
							-1, 0, 0, -1, 0, ZERO, ZERO, ZERO);
						suicideInfo.SetDamage(10000);
						pGameRules->ClientHit(suicideInfo);
					}
				}
			}
		}
	}
}

void CConquerorSystem::OnClientAreaEnter(CStrategicArea* area, bool vehicle)
{
	if (area->IsCapturable())
	{
		m_animConquerorProgress.Reload(true);
		m_animConquerorProgress.SetVisible(true);

		m_animConquerorProgress.Invoke("setInVehicle", (int)vehicle);
		m_animConquerorProgress.Invoke("setCaptureProgress", 0);
	}

	SetClientArea(area);
	UPDATE_POWERSTRUGGLE_BUY_INTERFACE;

	if (area->IsBuyZoneActived(GetClientSpecies()))
		m_animBuyZoneIndicator.SetVisible(true);
}

void CConquerorSystem::OnClientAreaExit()
{
	SetClientArea(nullptr);
	UPDATE_POWERSTRUGGLE_BUY_INTERFACE;

	m_animConquerorProgress.SetVisible(false);
	m_animBuyZoneIndicator.SetVisible(false);
}