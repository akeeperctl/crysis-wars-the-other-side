#include "StdAfx.h"

#include "IVehicleSystem.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"

#include "Actor.h"
#include "Trooper.h"
#include "Player.h"
#include "Game.h"
#include "GameRules.h"
#include "GameCVars.h"
#include "GameUtils.h"

#include "SpawnPoint.h"
#include "ConquerorSystem.h"
#include "StrategicArea.h"
#include "ConquerorCommander.h"
#include "RequestsAndResponses/RARSystem.h"

#include "../Abilities/AbilitiesSystem.h"
#include "../Abilities/AbilityOwner.h"

#include "../Control/ControlSystem.h"
#include "../Squad/SquadSystem.h"

#include "../Helpers/TOS_AI.h"
#include "../Helpers/TOS_Vehicle.h"

#include "ConquerorChannel.h"
#include "Cry_Math.h"

CConquerorChannel::CConquerorChannel(IEntity* pEntity, const char* className)
{
	m_entityId = pEntity->GetId();
	m_controlledEntityId = 0;
	m_squadId = -1;
	m_id = 0;
	m_pLastStrategicArea = nullptr;
	m_pForcedStrategicArea = nullptr;
	m_pSelectedStrategicArea = nullptr;
	m_points = 0;
	m_lastDeathPos = Vec3(0, 0, 0);
	m_disableRespawnRequest = false;

	SetSpecies(pEntity);
	SetState(EConquerorChannelState::eCCS_Spectator);	
	SetClass(g_pControlSystem->GetConquerorSystem()->GetClass(GetSpecies(), className));

	RemoveInventoryItems(pEntity);

	m_stateDurationMap.clear();

	if (g_pGameCVars->conq_debug_log_aichannel)
		CryLogAlways("[C++][Channel %s: Create success", pEntity->GetName());
}

CConquerorChannel::~CConquerorChannel()
{
}

//void CConquerorChannel::OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId)
//{
//	auto pEntity = gEnv->pEntitySystem->GetEntity(m_entityId);
//	if (pEntity && pEntity->GetAI())
//	{
//		auto pMemberPipeUser = pEntity->GetAI()->CastToIPipeUser();
//		if (pMemberPipeUser)
//		{
//			if (pPipeUser == pMemberPipeUser)
//				g_pControlSystem->GetConquerorSystem()->OnGoalPipeEvent(pEntity, pPipeUser, event, goalPipeId);
//		}
//	}
//}

void CConquerorChannel::OnKilled()
{
	m_lastDeathPos = GetEntity()->GetWorldPos();

	SetState(eCCS_Dead);
}

void CConquerorChannel::OnRespawned(ERespawnEvent respawnCase)
{
	SetState(eCCS_Alive);

	if (m_pLastStrategicArea != m_pSelectedStrategicArea)
	{
		m_pLastStrategicArea = m_pSelectedStrategicArea;
		//CryLogAlways("%s[C++][Channel %s: Set Last Strategic Area %s]", 
		//	STR_PURPLE, GetEntity()->GetName(), m_pLastStrategicArea->GetEntity()->GetName());
		
		m_pSelectedStrategicArea = nullptr;
	}

	g_pControlSystem->GetConquerorSystem()->OnActorRespawned(GetActor(), respawnCase);
}

void CConquerorChannel::OnSpectator(ESpectatorEvent event)
{
	SetState(eCCS_Spectator);

	if (IsPlayer() && (event == eSC_EnterLobby) && g_pControlSystem->GetConquerorSystem()->m_haveBotsSpawned)
		g_pControlSystem->GetConquerorSystem()->AddSpeciesReinforcements(GetSpecies(), 1, -1);
}

void CConquerorChannel::SetState(EConquerorChannelState state)
{
	m_state = state;
	OnSetState(state);
}

void CConquerorChannel::SetControlledEntity(EntityId entityId)
{
	if (!m_entityId)
		return;

	m_controlledEntityId = entityId;
	OnSetControlled(entityId);
}


void CConquerorChannel::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_stateDurationMap);
	m_pClass->GetMemoryStatistics(s);
}

float CConquerorChannel::GetRemainingRespawnTime()
{
	//return max(0.0f, (m_respawnCycleEndTime - g_pGame->GetIGameFramework()->GetServerTime()).GetSeconds());
	return 0;
}

float CConquerorChannel::GetStateDuration(EConquerorChannelState state)
{
	return m_stateDurationMap[state];
}

EConquerorChannelState CConquerorChannel::GetState() const
{
	return m_state;
}

void CConquerorChannel::OnSetControlled(EntityId entityId)
{
	if (IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entityId))
		SetSpecies(pEntity);
	else
		SetSpecies(GetEntity());
}

void CConquerorChannel::AddStateDuration(EConquerorChannelState state, float duration)
{
	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	m_stateDurationMap[state] += duration;

	//if (state == eCCS_Dead)
	//{
	//	const float curDuration = m_stateDurationMap[state];
	//	const float respawnTime = pConqueror->GetRespawnTime();

	//	auto count = g_pControlSystem->GetConquerorSystem()->GetSpeciesReinforcements(GetSpecies());

	//	if (count > 0)
	//	{
	//		if (IsClient())
	//		{
	//			const float remainingTime = max(0.f, respawnTime - curDuration);
	//			pConqueror->SetRespawnCycleRemainingTime((int)respawnTime, remainingTime);
	//		}

	//		if (curDuration >= respawnTime)
	//		{
	//			const bool isAI = !IsPlayer();

	//			if (isAI)
	//			{
	//				const bool oneReinfLeft = count == 1;
	//				const bool reservedForClient = true;
	//				const bool clientIsDead = pConqueror->GetClientConquerorChannel()->GetState() == eCCS_Dead;

	//				if (!(clientIsDead && oneReinfLeft && reservedForClient) || !m_disableRespawnRequest)
	//					RequestRespawn(eRC_OnKilledRespawn);
	//			}
	//			else
	//			{
	//				if (!m_disableRespawnRequest)
	//				{
	//					RequestRespawn(eRC_OnKilledRespawn);
	//				}
	//			}
	//		}
	//	}
	//	else
	//	{
	//		if (IsClient())
	//		{
	//			pConqueror->SetRespawnCycleRemainingTime((int)respawnTime, 0);
	//			pConqueror->ShowRespawnCycle(false);
	//			//pConqueror->GameOver(true);
	//			//g_pGame->GetHUD()->DisplayOverlayFlashMessage("@conq_your_species_is_destroyed_go_menu", ColorF(1.0f, 1.0f, 0.0f));
	//		}
	//	}
	//}
}

void CConquerorChannel::OnSetState(EConquerorChannelState state)
{
	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return;

	auto* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_entityId));
	if (!pActor)
		return;

	if (!m_pClass)
		return;

	if (IsPlayer())
	{
		if (state == EConquerorChannelState::eCCS_Spectator)
		{
			//Hide revive timer
			pConqueror->ShowRespawnCycle(false);

			//if (g_pGameCVars->conq_debug_log_aichannel)
			//	CryLogAlways("[CAIChannel][State][SPECTATOR]");
		}
		else if (state == EConquerorChannelState::eCCS_Alive)
		{
			//Hide revive timer
			pConqueror->ShowRespawnCycle(false);

			//if (g_pGameCVars->conq_debug_log_aichannel)
			//	CryLogAlways("[CAIChannel][State][ALIVE]");
		}
		else if (state == EConquerorChannelState::eCCS_Dead)
		{

			//Register to respawn
			///if (g_pGameCVars->conq_debug_log_aichannel)
			//	CryLogAlways("[CAIChannel][State][Dead]");
		}
	}
	else
	{
		if (state == EConquerorChannelState::eCCS_Spectator)
		{
			if (!GetEntity()->IsHidden())
				GetEntity()->Hide(true);

			//if (g_pGameCVars->conq_debug_log_aichannel)
			//	CryLogAlways("[CAIChannel][State][SPECTATOR]");
		}
		else if (state == EConquerorChannelState::eCCS_Alive)
		{
			if (GetEntity()->IsHidden())
				GetEntity()->Hide(false);

			//if (g_pGameCVars->conq_debug_log_aichannel)
			//	CryLogAlways("[CAIChannel][State][ALIVE]");
		}
		else if (state == EConquerorChannelState::eCCS_Dead)
		{
		}
	}
}

ESpeciesType CConquerorChannel::GetSpecies()
{
	return ESpeciesType(m_species);
}

IEntity* CConquerorChannel::GetEntity()
{
	return gEnv->pEntitySystem->GetEntity(m_entityId);
}

IEntity* CConquerorChannel::GetControlledEntity()
{
	return gEnv->pEntitySystem->GetEntity(m_controlledEntityId);
}

IActor* CConquerorChannel::GetActor()
{
	return g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_entityId);
}

CSpeciesClass* CConquerorChannel::GetClass()
{
	return m_pClass;
}

const char* CConquerorChannel::GetName()
{
	IEntity* pEntity = GetEntity();
	if (pEntity)
		return pEntity->GetName();

	return 0;
}

void CConquerorChannel::SetName(const char* name)
{
	IEntity* pEntity = GetEntity();
	if (pEntity)
		pEntity->SetName(name);
}

//CSpawnPoint* CConquerorChannel::RequestSpawnPoint(bool requestForClient, CStrategicArea* pCapturableArea/*=nullptr*/)
//{
//	if (pCapturableArea)
//		return pCapturableArea->GetSpawnPoint(eSGSF_NotHaveRecentlySpawned);
//
//	auto pSpawnArea = RequestStrategicArea(requestForClient, 0);
//		
//	if (!pSpawnArea)
//		return nullptr;
//
// 	return pSpawnArea->GetSpawnPoint(eSGSF_NotHaveRecentlySpawned);
//}
//
//CStrategicArea* CConquerorChannel::RequestStrategicArea(bool requestForClient, std::vector<EntityId>* excludeAreaIds)
//{
//	auto pSpawnArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea((ESpeciesType)m_species, excludeAreaIds, true);
//
//	if (requestForClient)
//	{
//		auto pAreaEntity = g_pControlSystem->GetConquerorSystem()->m_pSelectedClientAreaEntity;
//		if (pAreaEntity)
//			pSpawnArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pAreaEntity->GetId(), excludeAreaIds, true);
//	}
//
//	//if (pSpawnArea /*&& pSpawnArea->IsEnabled()*/)
//		return pSpawnArea;
//
//	return nullptr;
//}

//void CConquerorChannel::RequestRespawn(ERespawnEvent respawnCase)
//{
//	SpawnActor(respawnCase, nullptr);
//}

CStrategicArea* CConquerorChannel::GetLeaderStrategicArea()
{
	auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(GetActor(), false);
	if (pSquad)
	{
		auto pLeader = static_cast<CActor*>(pSquad->GetLeader());
		if (!pLeader)
			return nullptr;

		//Controlled actors must not have own channels
		if (pLeader->IsHaveOwner())
			pLeader = static_cast<CActor*>(pLeader->GetOwnerActor());

		auto pLeaderChannel = g_pControlSystem->GetConquerorSystem()->GetConquerorChannel(pLeader->GetEntity());
		if (!pLeaderChannel)
			return nullptr;

		CStrategicArea* pArea = nullptr;

		if (pLeader->GetHealth() > 0)
		{
			const auto pos = pLeader->GetEntity()->GetWorldPos();
			const auto species = pLeaderChannel->GetSpecies();

			pArea = g_pControlSystem->GetConquerorSystem()->GetNearestStrategicArea(pos, species, eAGSF_Enabled, species, eABF_NoMatter, EAreaFlag::SoldierSpawner);
			
			return pArea;
		}
		else
		{
			pArea = pLeaderChannel->m_pLastStrategicArea;
			if (pArea && (pArea->GetSpecies() == GetSpecies() && pArea->IsEnabled()))
				return pArea;
			else
				return g_pControlSystem->GetConquerorSystem()->GetStrategicArea(GetSpecies(), 0, true);
		}		
	}

	return nullptr;
}

void CConquerorChannel::SetSpecies(int species)
{
	m_species = species;
}

void CConquerorChannel::SetSpecies(IEntity* pEntity)
{
	if (IAIObject* pAI = pEntity->GetAI())
	{
		AgentParameters playerParams = pAI->CastToIAIActor()->GetParameters();
		m_species = playerParams.m_nSpecies;
	}
}

void CConquerorChannel::SetSquadId(int id)
{
	m_squadId = id;
}

void CConquerorChannel::SetClass(CSpeciesClass* classInfo)
{
	m_pClass = classInfo;
}

void CConquerorChannel::RemoveInventoryItems(IEntity* pEntity)
{
	if (!pEntity)
		return;

	auto* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
	if (!pActor)
		return;

	auto* pInventory = pActor->GetInventory();
	if (pInventory)
		pInventory->RemoveAllItems();
}

void CConquerorChannel::ResetStateDuration(EConquerorChannelState state)
{
	m_stateDurationMap[state] = 0.0f;
}

bool CConquerorChannel::SpawnActor(ERespawnEvent respawnCase, CSpawnPoint* pNewSpawnPoint)
{
	if (!m_pClass)
		return false;

	auto pConqueror = g_pControlSystem->GetConquerorSystem();
	if (!pConqueror)
		return false;

	auto pAbilitiesSystem = g_pControlSystem->GetAbilitiesSystem();
	if (!pAbilitiesSystem)
		return false;

	auto pEntity = GetEntity();
	if (!pEntity)
		return false;

	bool isSpawned = false;

	auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (pActor)
	{
		auto& classModel = m_pClass->GetModel();
		auto& classEquip = m_pClass->GetEquipment();
		auto& classAI = m_pClass->GetAI();
		auto& classAbilities = m_pClass->GetAbilities();

		const bool isClient = pActor->IsPlayer();
		bool isLeader = m_pClass->IsLeaderClass();

		if (!isClient)
		{
			auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, true);

			if (respawnCase != eRC_ForcedRespawn && (!pSquad || pSquad && !pSquad->GetLeader()))
			{
				if (!pSquad)
				{
					CryLogAlways("%s[C++][Spawn Actor Fail][Victim: %s][Cause1: pSquad not defined]",
						STR_RED, pActor->GetEntity()->GetName());
				}
				else if (pSquad && !pSquad->GetLeader())
				{
					CryLogAlways("%s[C++][Spawn Actor Fail][Victim: %s][Cause1: Squad Leader not defined]",
						STR_RED, pActor->GetEntity()->GetName());
				}

				if (respawnCase != eRC_ForcedRespawn)
				{
					CryLogAlways("%s[C++][Spawn Actor Fail][Victim: %s][Cause2: respawnCase is not ForcedRespawn]",
						STR_RED, pActor->GetEntity()->GetName());
				}

				return false;
			}

			if (pSquad)
				isLeader = pSquad->GetLeader() == pActor;

			if (pNewSpawnPoint)
			{
				if (respawnCase == eRC_OnKilledRespawn || respawnCase == eRC_OnClientSpectatorRespawn)
				{
					// get out of vehicles before reviving
					TOS_Vehicle::Exit(pActor, false, false);

					// stop using any mounted weapons before reviving
					if (auto pItem = static_cast<CItem*>(pActor->GetCurrentItem()))
					{
						if (pItem->IsMounted())
							pItem->StopUse(pActor->GetEntityId());
					}

					if (g_pGame->GetGameRules()->IsFrozen(pActor->GetEntityId()))
						g_pGame->GetGameRules()->FreezeEntity(pActor->GetEntityId(), false, false);

					pActor->SetHealth(pActor->GetMaxHealth());
					pActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);
					pActor->Revive();
				}

				auto pTable = pActor->GetEntity()->GetScriptTable();
				if (pTable)
				{
					SmartScriptTable propertiesInstance;
					SmartScriptTable properties;

					if (pTable->GetValue("Properties", properties) &&
						pTable->GetValue("PropertiesInstance", propertiesInstance))
					{
						propertiesInstance->SetValue("bAutoDisable", 0);

						if (classAI.m_behaviour != "")
						{
							propertiesInstance->SetValue("aibehavior_behaviour", classAI.m_behaviour.c_str());
						}

						if (classAI.m_character != "")
							properties->SetValue("aicharacter_character", classAI.m_character.c_str());
					}
				}

				TOS_AI::RegisterAI(pEntity, false);
				//TOS_AI::EnableCombat(pActor, false, true);
				//TOS_AI::EnableCombat(pActor, true, false);
				Script::CallMethod(pTable, "OnReset");

				pNewSpawnPoint->SpawnActor(pActor);
				g_pGame->GetIGameFramework()->GetIItemSystem()->GetIEquipmentManager()->GiveEquipmentPack(pActor, classEquip.m_equipPack.c_str(), true, true);
				//pActor->SetStance(STANCE_STAND);

				if (pSquad)
				{
					auto pMember = pSquad->GetMemberInstance(pActor);
					if (pMember)
					{
						const auto groupId = pMember->GetGroupId();
						const auto newId = g_pControlSystem->GetSquadSystem()->RequestGroupId(groupId, pActor->GetEntity());
						pMember->SetCurrentGroupId(newId);
					}
					else if (pSquad->GetLeader() == pActor)
					{
						const auto groupId = pActor->GetEntity()->GetAI()->GetGroupId();
						const auto newId = g_pControlSystem->GetSquadSystem()->RequestGroupId(groupId, pActor->GetEntity());
						pActor->GetEntity()->GetAI()->SetGroupId(newId);
					}
				}

				if (g_pGameCVars->conq_debug_log_aichannel)
					CryLogAlways("$3[C++][Channel %i: Spawn the actor %s", pActor->GetEntity()->GetName());

				isSpawned = true;
			}
			
		}
		else
		{
			if (pNewSpawnPoint)
			{
				if (!GetClass()->IsAirClass() && pNewSpawnPoint->IsForAir())
				{
					CryLogAlways("PIZDEC TOVARISHI");
				}

				auto pArea = pNewSpawnPoint->GetArea();

				/*if (auto pVehicle = pActor->GetLinkedVehicle())
				{
					if (auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId()))
						pSeat->Exit(false);
				}*/
				TOS_Vehicle::Exit(pActor, false, false);

				// stop using any mounted weapons before reviving
				auto pItem = static_cast<CItem*>(pActor->GetCurrentItem());
				if (pItem)
				{
					if (pItem->IsMounted())
					{
						pItem->StopUse(pActor->GetEntityId());
						pItem = 0;
					}
				}

				if (g_pGame->GetHUD())
					g_pGame->GetHUD()->ActorRevive(pActor);

				if (pArea->GetSpecies() != eST_Aliens)
					pConqueror->SetPlayerModel(pActor, classModel);

				pActor->SetHealth(pActor->GetMaxHealth());
				pActor->Revive();

				auto pCharacter = pActor->GetEntity()->GetCharacter(0);
				if (pCharacter)
					pCharacter->SetFlags(pCharacter->GetFlags() | CS_FLAG_UPDATE_ALWAYS);

				if (pArea->GetSpecies() != eST_Aliens)
					pConqueror->SetPlayerMaterial(pActor, classModel);

				if (pActor->IsClient())
				{
					// no view bleding when respawning // CActor::Revive resets it.
					pActor->SupressViewBlending(); 

					if (g_pGame->GetHUD())
						g_pGame->GetHUD()->GetRadar()->Reset();
				}

				if (pArea->GetSpecies() != eST_Aliens)
				{
					pNewSpawnPoint->SpawnActor(pActor);

					auto* pPlayer = static_cast<CPlayer*>(pActor);
					m_pClass->IsOnlyThirdPerson() ? pPlayer->TurnOnOnlyThirdPerson() : pPlayer->TurnOFFOnlyThirdPerson();
					m_pClass->IsNeedHumanMode() ? pPlayer->TurnOnHumanMode() : pPlayer->TurnOFFHumanMode();

					TOS_AI::RegisterAI(pActor->GetEntity(), true);
					TOS_AI::SetSpecies(pActor->GetEntity()->GetAI(), GetSpecies());

					string equipName = classEquip.m_equipPack;

					if (pActor->IsPlayer())
						equipName.append("_Player");

					g_pGame->GetIGameFramework()->GetIItemSystem()->GetIEquipmentManager()->GiveEquipmentPack(pActor, equipName, true, true);

					isSpawned = true;
				}					
				else
				{					
					CSquad* pOldSquad = nullptr;

					auto pOldAlien = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pConqueror->m_usedAlienId);
					if (pOldAlien)
					{
						pOldSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pOldAlien);
					}

					auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(m_pClass->GetAI().m_archetype.c_str());
					if (pArchetype)
					{
						const string playerName = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetName();

						SEntitySpawnParams params;
						params.pArchetype = pArchetype;
						params.vPosition = Vec3(1, 1, 1);
						params.nFlags = ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_CASTSHADOW /*| ENTITY_FLAG_UNREMOVABLE*/;
						params.sName = playerName + " Alien";
						params.bStaticEntityId = true;

						//if (gEnv->bEditor)
						//	params.nFlags &= ~ENTITY_FLAG_UNREMOVABLE;

						auto pEntity = gEnv->pEntitySystem->SpawnEntity(params);
						if (pEntity)
						{
							pConqueror->m_usedAlienId = pEntity->GetId();

							/*if (m_pClass.GetName() == "Scout")
							{
								const float scoutZOffset = 10.0f;
								finalSpawnPos.z += scoutZOffset;
							}*/

							if (pEntity->IsHidden())
								pEntity->Hide(false);
									
							auto* pAlienActor = static_cast<CAlien*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pConqueror->m_usedAlienId));
							if (pAlienActor)
							{
								pActor = pAlienActor;

								pNewSpawnPoint->SpawnActor(pActor);
								g_pControlSystem->StartLocal(pActor, true, true);

								if (pOldAlien)
								{
									pOldAlien->SetHealth(0);
									gEnv->pEntitySystem->RemoveEntity(pOldAlien->GetEntityId(), true);
									pOldAlien = nullptr;
								}

								if (pOldSquad)
								{
									bool changeOldConquerorLeader = false;
									pOldSquad->SetLeader(pActor, changeOldConquerorLeader);
								}

								auto pDude = GetActor();
								if (pDude == g_pGame->GetIGameFramework()->GetClientActor())
								{
									auto pAI = pDude->GetEntity()->GetAI();
									if (pAI)
										pAI->Event(AIEVENT_DISABLE, nullptr);
								}

								//Use equip from classes file
								string equipName = classEquip.m_equipPack;
								g_pGame->GetIGameFramework()->GetIItemSystem()->GetIEquipmentManager()->GiveEquipmentPack(pActor, equipName, false, true);

								if (strcmp(m_pClass->GetName(), "Trooper Guardian") == 0)
									pAlienActor->CastToCTrooper()->ApplyGuardianType();
								else if (strcmp(m_pClass->GetName(), "Trooper Leader") == 0)
									pAlienActor->CastToCTrooper()->ApplyLeaderType();

								isSpawned = true;
							}
						}
					}
				}

				if (!classAbilities.empty())
				{
					pAbilitiesSystem->AddAbilityOwner(pActor);

					auto pAbilityOwner = pAbilitiesSystem->GetAbilityOwner(pActor->GetEntityId());
					if (pAbilityOwner)
					{
						for (auto& abilityName : classAbilities)
							pAbilityOwner->AddAbility(abilityName);
					}
				}
			}
		}
	}

	if (isSpawned)
		OnRespawned(respawnCase);

	return isSpawned;
}

bool CConquerorChannel::IsPlayer()
{
	auto* pActor = GetActor();
	if (!pActor)
		return false;

	return pActor->GetChannelId() != 0;
}

bool CConquerorChannel::IsClient() noexcept
{
	auto* pActor = GetActor();
	if (!pActor)
		return false;

	return pActor == g_pGame->GetIGameFramework()->GetClientActor();
}

void CConquerorChannel::Update(float frametime)
{
	//const string name = GetActor()->GetEntity()->GetName();
	bool drawDebug = false;
	if (drawDebug && GetActor() == g_pGame->GetIGameFramework()->GetClientActor())
	{
		float color[] = { 1,1,1,1 };
		gEnv->pRenderer->Draw2dLabel(60, 20, 1.2f, color, false, "channel owner %s", GetEntity()->GetName());
		gEnv->pRenderer->Draw2dLabel(60, 40, 1.2f, color, false, "m_stateDurationMap[Alive] %f", m_stateDurationMap[eCCS_Alive]);
		gEnv->pRenderer->Draw2dLabel(60, 60, 1.2f, color, false, "m_stateDurationMap[Dead] %f", m_stateDurationMap[eCCS_Dead]);
		gEnv->pRenderer->Draw2dLabel(60, 80, 1.2f, color, false, "m_stateDurationMap[Spectator] %f", m_stateDurationMap[eCCS_Spectator]);
		gEnv->pRenderer->Draw2dLabel(60, 100, 1.2f, color, false, "m_respawnTime %1.f", g_pControlSystem->GetConquerorSystem()->GetRespawnTime());
		gEnv->pRenderer->Draw2dLabel(60, 120, 1.2f, color, false, "m_pLastCapturableArea %s", m_pLastStrategicArea ? m_pLastStrategicArea->GetEntity()->GetName() : "NULL");
	}

	switch (m_state)
	{
	case eCCS_Alive:
		AddStateDuration(m_state, frametime);

		m_stateDurationMap[eCCS_Dead] = 0;
		m_stateDurationMap[eCCS_Spectator] = 0;
		break;
	case eCCS_Spectator:
		AddStateDuration(m_state, frametime);

		m_stateDurationMap[eCCS_Dead] = 0;
		m_stateDurationMap[eCCS_Alive] = 0;
		break;
	case eCCS_Dead:
		AddStateDuration(m_state, frametime);

		m_stateDurationMap[eCCS_Spectator] = 0;
		m_stateDurationMap[eCCS_Alive] = 0;
		break;
	}
}

int CConquerorChannel::GetPoints() noexcept
{
	return m_points;
}

void CConquerorChannel::AddPoints(int value) noexcept
{
	m_points += value;
}

void CConquerorChannel::DisableRespawnRequest(bool disable)
{
	m_disableRespawnRequest = disable;

	if (g_pGameCVars->conq_debug_log_aichannel)
		CryLogAlways("$6[C++][WARNING][AI Channel %s][Respawn Request Disabled %i]", GetName(), disable);
}

void CConquerorChannel::SetForcedStrategicArea(CStrategicArea* pArea)
{
	m_pForcedStrategicArea = pArea;
}

void CConquerorChannel::SetSelectedStrategicArea(CStrategicArea* pArea)
{
	m_pSelectedStrategicArea = pArea;
}

const CStrategicArea* CConquerorChannel::GetLastArea()
{
	return m_pLastStrategicArea;
}

CStrategicArea* CConquerorChannel::GetSelectedArea()
{
	return m_pSelectedStrategicArea;
}
