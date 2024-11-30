/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

// ReSharper disable CppInconsistentNaming
#include "StdAfx.h"
#include "TOSActor.h"

#include "Actor.h"
//#include "OffHand.h"

//#include "Aliens/TOSTrooper.h"

#include "Fists.h"
#include "GameRules.h"
#include "NetInputChainDebug.h"

#include "TheOtherSideMP/Extensions/EnergyСonsumer.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Helpers/TOS_Inventory.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDTagNames.h"

#include "TheOtherSideMP/Control/ControlSystem.h"
#include "DummyTargetPointVerifier.h"
#include <stdexcept>

CDummyTargetPointVerifier CDummyTargetPointVerifier::Instance = CDummyTargetPointVerifier();

CTOSActor::CTOSActor()
	:
	//m_filteredDeltaMovement(ZERO),
	m_isSlave(false),
	m_isMaster(false),
	m_isZeus(false),
	m_chargingJump(false),
	m_lastShooterId(0),
	m_pEnergyConsumer(nullptr)
{
	
}

CTOSActor::~CTOSActor() {};

bool CTOSActor::Init(IGameObject* pGameObject)
{
	if (!CActor::Init(pGameObject))
		return false;

	m_pEnergyConsumer = static_cast<CTOSEnergyConsumer*>(GetGameObject()->AcquireExtension("TOSEnergyConsumer"));
	assert(m_pEnergyConsumer);
	m_pEnergyConsumer->Reset();

	m_debugName = GetEntity()->GetName();

	return true;
}

void CTOSActor::PostInit(IGameObject* pGameObject)
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::PostInit] Actor: %s|%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId());

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorPostInit, "", true));

	CActor::PostInit(pGameObject);

	m_netBodyInfo.Reset();
	m_slaveStats = STOSSlaveStats();

	// Факт: если оружие выдаётся на сервере, оно выдаётся и на всех клиентах тоже.
	ResetActorWeapons(1000);

	// 30.11.2023 Akeeper: Это я оставлю здесь на всякий случай.
	// Но к сожалению это не позволяет включить PrePhysicsUpdate в одиночной игре 
	// отравки запроса на движение в MasterClient. 
	GetGameObject()->EnablePrePhysicsUpdate(ePPU_Always);
	m_debugName = GetEntity()->GetName();

	if (ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0))
		pCharacter->SetFlags(pCharacter->GetFlags() | CS_FLAG_UPDATE_ALWAYS);

	IEntityRenderProxy* pRenderProxy = (IEntityRenderProxy*)(GetEntity()->GetProxy(ENTITY_PROXY_RENDER));
	if (pRenderProxy)
	{
		pRenderProxy->UpdateCharactersBeforePhysics(true);
	}
}

void CTOSActor::InitClient(const int channelId)
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::InitClient] Actor: %s|%i|ch:%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId(), channelId);

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_InitClient, "", true, false, nullptr, 0.0f, channelId));

	CActor::InitClient(channelId);
}

void CTOSActor::ProcessEvent(SEntityEvent& event)
{
	CActor::ProcessEvent(event);

	const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	if (pMC)
		pMC->OnEntityEvent(GetEntity(), event);

	switch (event.event)
	{
	//case ENTITY_EVENT_XFORM:
	//{
	//	auto flag = event.nParam[0];

	//	if (gEnv->bServer && IsPlayer() && (flag & ENTITY_XFORM_POS))
	//	{
	//		auto pSlaveEntity = g_pTOSGame->GetMasterModule()->GetCurrentSlave(GetEntity());
	//		if (pSlaveEntity)
	//		{
	//			const Vec3 slavePos = pSlaveEntity->GetWorldPos();
	//			const Quat slaveRot = pSlaveEntity->GetWorldRotation();

	//			if ((this->GetEntity()->GetWorldPos() - slavePos).len() > 1.0f)
	//			{
	//				this->GetEntity()->SetWorldTM(Matrix34::CreateTranslationMat(slavePos), 0);
	//			}
	//		}
	//	}
	//	
	//	break;
	//}
	case ENTITY_EVENT_TIMER:
	{
		// Фикс бага #29
		if (event.nParam[0] == eMPTIMER_REMOVEWEAPONSDELAY)
		{
			const auto pInventory = GetInventory();
			if (pInventory)
			{
				pInventory->HolsterItem(true);
				pInventory->RemoveAllItems();
				pInventory->Clear();

				if (gEnv->bServer && gEnv->bMultiplayer && !IsPlayer())
				{
					GetEntity()->SetTimer(eMPTIMER_GIVEWEAPONDELAY, 1000);
				}
			}
		}
		else if (event.nParam[0] == eMPTIMER_GIVEWEAPONDELAY)
		{
			string       equipName;
			const string actorClass = GetEntity()->GetClass()->GetName();

			if (actorClass == "Trooper")
			{
				equipName = (string)gEnv->pConsole->GetCVar("tos_sv_TrooperMPEquipPack")->GetString();
			}
			else if (actorClass == "Scout")
			{
				equipName = (string)gEnv->pConsole->GetCVar("tos_sv_ScoutMPEquipPack")->GetString();
			}
			else if (actorClass == "Alien")
			{
				equipName = (string)gEnv->pConsole->GetCVar("tos_sv_AlienMPEquipPack")->GetString();
			}
			else if (actorClass == "Hunter")
			{
				equipName = (string)gEnv->pConsole->GetCVar("tos_sv_HunterMPEquipPack")->GetString();
			}
			else if (actorClass == "Grunt")
			{
				equipName = (string)gEnv->pConsole->GetCVar("tos_sv_HumanGruntMPEquipPack")->GetString();
			}

			TOS_Inventory::GiveEquipmentPack(this, equipName, false);
		}
	}
	default: 
		break;
	}
}

// ReSharper disable once CppParameterMayBeConst
bool CTOSActor::NetSerialize(TSerialize ser, const EEntityAspects aspect, const uint8 profile, const int flags)
{
	if (!CActor::NetSerialize(ser,aspect,profile,flags))
		return false;

	if (aspect == TOS_NET::SERVER_ASPECT_STATIC)
	{
		// Персонаж мастера всегда должен быть невидим
		ser.Value("is_master", m_isMaster, 'bool');
		ser.Value("is_slave", m_isSlave, 'bool');
		ser.Value("is_zeus", m_isZeus, 'bool');

		if (ser.IsReading())
		{
			HideMe(m_isMaster);
		}

	}


	return true;
}

void CTOSActor::SelectNextItem(const int direction, const bool keepHistory, const char* category)
{
	CActor::SelectNextItem(direction, keepHistory, category);

	if (gEnv->bClient)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_STATIC);
	}
}

void CTOSActor::HolsterItem(const bool holster)
{
	CActor::HolsterItem(holster);

	if (gEnv->bClient)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_STATIC);
	}
}

void CTOSActor::SelectLastItem(const bool keepHistory, const bool forceNext /* = false */)
{
	CActor::SelectLastItem(keepHistory, forceNext);

	if (gEnv->bClient)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_STATIC);
	}
}

void CTOSActor::SelectItemByName(const char* name, const bool keepHistory)
{
	CActor::SelectItemByName(name, keepHistory);

	if (gEnv->bClient)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_STATIC);
	}
}

void CTOSActor::SelectItem(const EntityId itemId, const bool keepHistory)
{
	CActor::SelectItem(itemId, keepHistory);

	if (gEnv->bClient)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_STATIC);
	}
}


void CTOSActor::Update(SEntityUpdateContext& ctx, const int updateSlot)
{
	CActor::Update(ctx, updateSlot);

	//Отладка потребителя энергии в виде вывода инф. на экран
	if (gEnv->bClient && IsClient())
	{
		const auto pDebugEntity = gEnv->pEntitySystem->FindEntityByName(CTOSEnergyConsumer::s_debugEntityName);
		if (pDebugEntity)
		{
			const auto pDebugActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pDebugEntity->GetId()));
			if (pDebugActor)
			{
				const float energy    = pDebugActor->m_pEnergyConsumer->GetEnergy();
				const float maxEnergy = pDebugActor->m_pEnergyConsumer->GetMaxEnergy();
				const float drain     = pDebugActor->m_pEnergyConsumer->GetDrainValue();
				const bool  updating  = pDebugActor->m_pEnergyConsumer->IsUpdating();

				DRAW_2D_TEXT(40, 200, 1.3f, "--- Energy Consumer (%s) ---", pDebugEntity->GetName());
				DRAW_2D_TEXT(40, 215, 1.3f, "Updating:   %i", updating);
				DRAW_2D_TEXT(40, 230, 1.3f, "Energy:     %1.f", energy);
				DRAW_2D_TEXT(40, 245, 1.3f, "MaxEnergy:  %1.f", maxEnergy);
				DRAW_2D_TEXT(40, 260, 1.3f, "DrainValue: %1.f", drain);
			}
		}
	}

	//int clTeamId = -1;
	//int svTeamId = -1;

	//if (gEnv->bClient)
	//{
	//	clTeamId = g_pGame->GetGameRules()->GetTeam(GetEntityId());
	//}
	//if (gEnv->bServer)
	//{
	//	svTeamId = g_pGame->GetGameRules()->GetTeam(GetEntityId());
	//}

	NETINPUT_TRACE(GetEntityId(), m_isMaster);
	NETINPUT_TRACE(GetEntityId(), m_isSlave);
	NETINPUT_TRACE(GetEntityId(), m_isZeus);
}

void CTOSActor::Release()
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::Release] Actor: %s|%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId());

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorRelease, "", true));

	CActor::Release();
}

void CTOSActor::Revive(const bool fromInit)
{
	CActor::Revive(fromInit);

	if (m_isMaster)
	{
		// Скрыть актера, если он мастер

		if (gEnv->bClient)
		{
			GetGameObject()->InvokeRMI(SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
		}
		else if (gEnv->bServer)
		{
			const auto* pFists = static_cast<CFists*>(GetItemByClass(CItem::sFistsClass));
			if (pFists)
				g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(this, pFists->GetEntityId());

			GetGameObject()->InvokeRMI(ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients);
		}

		if (IsClient())
		{
			SetAngles(Ang3(m_lastSpawnPointRotation));
		}

		// Не уверен что это на что-то влияет
		SAnimatedCharacterParams params = m_pAnimatedCharacter->GetParams();
		params.flags &= ~eACF_EnableMovementProcessing;
		params.flags |= eACF_NoLMErrorCorrection;

		m_pAnimatedCharacter->SetParams(params);

		// Фиксит "физичность" в режиме зрителя после контроля раба
		if (ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0))
			pCharacter->GetISkeletonPose()->DestroyCharacterPhysics(1);

		m_pAnimatedCharacter->ForceRefreshPhysicalColliderMode();
		m_pAnimatedCharacter->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "Actor::SetAspectProfile");
	}

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorRevived, "", true));
}

void CTOSActor::Kill()
{
	CActor::Kill();

	// Вызывается только на сервере
	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorDead, "", true));
}

void CTOSActor::PlayAction(const char* action, const char* extension, const bool looping)
{
	CActor::PlayAction(action, extension, looping);

	NetPlayAnimationParams params;
	params.animation = action;
	params.mode = looping ? AIANIM_ACTION : AIANIM_SIGNAL;

	if (gEnv->bClient)
	{
		GetGameObject()->InvokeRMI(SvRequestPlayAnimation(), params, eRMI_ToServer);
	}
	else
	{
		GetGameObject()->InvokeRMI(ClPlayAnimation(), params, eRMI_ToRemoteClients);
	}
}

void CTOSActor::AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event)
{
	if (!pCharacter)
		return;

	const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	if (pMC)
		pMC->AnimationEvent(GetEntity(), pCharacter, event);

	CActor::AnimationEvent(pCharacter, event);
}

void CTOSActor::NetKill(EntityId shooterId, uint16 weaponClassId, int damage, int material, int hit_type, int killerHealthOnKill)
{
	static char weaponClassName[129] = { 0 };
	m_pGameFramework->GetNetworkSafeClassName(weaponClassName, 128, weaponClassId);

	g_pGame->GetGameRules()->OnKill(this, shooterId, weaponClassName, damage, material, hit_type);

	m_netLastSelectablePickedUp = 0;

	if (GetHealth() > 0)
		SetHealth(0);

	Kill();

	g_pGame->GetGameRules()->OnKillMessage(GetEntityId(), shooterId, weaponClassName, damage, material, hit_type);

	CHUD* pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	bool ranked = pHUD->GetPlayerRank(shooterId) != 0 || pHUD->GetPlayerRank(GetEntityId()) != 0;

	if (IsClient() && gEnv->bMultiplayer && shooterId != GetEntityId() && g_pGameCVars->g_deathCam != 0)
	{
		// use the spectator target to store who killed us (used for the MP death cam - not quite spectator mode but similar...).
		if (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(shooterId))
		{
			SetSpectatorTarget(shooterId);
			SetSpectatorHealth(killerHealthOnKill);

			// Also display the name of the enemy who shot you...
			if (g_pGame->GetGameRules()->GetTeam(shooterId) != g_pGame->GetGameRules()->GetTeam(GetEntityId()) || g_pGame->GetGameRules()->GetTeamCount() <= 1)
				SAFE_HUD_FUNC(GetTagNames()->AddEnemyTagName(shooterId));

			// ensure full body is displayed (otherwise player is headless)
			if (!IsThirdPerson())
				ToggleThirdPerson();
		}
	}
}

void CTOSActor::NetReviveAt(const Vec3& pos, const Quat& rot, int teamId)
{
	if (IVehicle* pVehicle = GetLinkedVehicle())
	{
		if (IVehicleSeat* pSeat = pVehicle->GetSeatForPassenger(GetEntityId()))
			pSeat->Exit(false);
	}

	// stop using any mounted weapons before reviving
	CItem* pItem = static_cast<CItem*>(GetCurrentItem());
	if (pItem)
	{
		if (pItem->IsMounted())
		{
			pItem->StopUse(GetEntityId());
			pItem = 0;
		}
	}

	SetHealth(GetMaxHealth());

	m_teamId = teamId;
	g_pGame->GetGameRules()->OnRevive(this, pos, rot, m_teamId);

	Revive();

	// Мастер привязан координатно к рабу, поэтому менять позицию ему не требуется
	if (!m_isMaster)
	{
		GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), rot, pos));
	}
	// Фиксит положение камеры игрока после воскрешения после смерти
	GetEntity()->SetRotation(rot);

	// This will cover the case when the ClPickup RMI comes in before we're revived
	{
		if (m_netLastSelectablePickedUp)
			pItem = static_cast<CItem*>(m_pItemSystem->GetItem(m_netLastSelectablePickedUp));
		
		m_netLastSelectablePickedUp = 0;

		if (pItem)
		{
			bool soundEnabled = pItem->IsSoundEnabled();
			pItem->EnableSound(false);
			pItem->Select(false);
			pItem->EnableSound(soundEnabled);

			m_pItemSystem->SetActorItem(this, (EntityId)0);
			SelectItem(pItem->GetEntityId(), true);
		}
	}

	//TheOtherSide IsLocalSlave
	if (IsClient() || IsLocalSlave())
	{
		SupressViewBlending(); // no view blending when respawning // CActor::Revive resets it.
		if (g_pGame->GetHUD())
			g_pGame->GetHUD()->GetRadar()->Reset();
	}

	UpdateLastMPSpawnPointRotation(rot);
	//~TheOtherSide
}

void CTOSActor::NetReviveInVehicle(EntityId vehicleId, int seatId, int teamId)
{
	// stop using any mounted weapons before reviving
	CItem* pItem = static_cast<CItem*>(GetCurrentItem());
	if (pItem)
	{
		if (pItem->IsMounted())
		{
			pItem->StopUse(GetEntityId());
			pItem = 0;
		}
	}

	SetHealth(GetMaxHealth());

	m_teamId = teamId;
	g_pGame->GetGameRules()->OnReviveInVehicle(this, vehicleId, seatId, m_teamId);

	Revive();

	// fix our physicalization, since it's need for some vehicle stuff, and it will be set correctly before the end of the frame
	// make sure we are alive, for when we transition from ragdoll to linked...
	if (!GetEntity()->GetPhysics() || GetEntity()->GetPhysics()->GetType() != PE_LIVING)
		Physicalize();

	IVehicle* pVehicle = m_pGameFramework->GetIVehicleSystem()->GetVehicle(vehicleId);
	assert(pVehicle);
	if (pVehicle)
	{
		IVehicleSeat* pSeat = pVehicle->GetSeatById(seatId);
		if (pSeat && (!pSeat->GetPassenger() || pSeat->GetPassenger() == GetEntityId()))
			pSeat->Enter(GetEntityId(), false);
	}

	// This will cover the case when the ClPickup RMI comes in before we're revived
	if (m_netLastSelectablePickedUp)
		pItem = static_cast<CItem*>(m_pItemSystem->GetItem(m_netLastSelectablePickedUp));
	m_netLastSelectablePickedUp = 0;

	if (pItem)
	{
		bool soundEnabled = pItem->IsSoundEnabled();
		pItem->EnableSound(false);
		pItem->Select(false);
		pItem->EnableSound(soundEnabled);

		m_pItemSystem->SetActorItem(this, (EntityId)0);
		SelectItem(pItem->GetEntityId(), true);
	}

	if (IsClient())
	{
		SupressViewBlending(); // no view bleding when respawning // CActor::Revive resets it.
		if (g_pGame->GetHUD())
			g_pGame->GetHUD()->GetRadar()->Reset();
	}
}

void CTOSActor::NetSimpleKill()
{
	if (GetHealth() > 0)
		SetHealth(0);

	Kill();
}

bool CTOSActor::ResetActorWeapons(int delayMilliseconds)
{
	if (gEnv->bServer && gEnv->bMultiplayer && !IsPlayer())
	{
		GetEntity()->SetTimer(eMPTIMER_REMOVEWEAPONSDELAY, 1000);
		return true;
	}

	return false;
}

bool CTOSActor::ShouldUsePhysicsMovement()
{
	//swimming use physics, for everyone
	//if (m_stats.inWaterTimer > 0.01f)
	//	return true;

	if (GetActorStats()->inAir > 0.01f || InZeroG())
		return true;

	//players
	if (IsPlayer() || IsSlave())
	{
		//the client will be use physics always but when in thirdperson
		if (IsClient())
		{
			if (!IsThirdPerson()/* || m_stats.inAir>0.01f*/)
				return true;
			else
				return false;
		}

		//other clients will use physics always
		return true;
	}


	//in demo playback - use physics for recorded entities
	if (IsDemoPlayback())
		return true;

	//AIs in normal conditions are supposed to use LM
	return false;
}

bool CTOSActor::ApplyActions(int actions)
{
	throw std::logic_error("функция должна быть переопределена в дочерних классах");
	return false;
}

//void CTOSActor::QueueAnimationEvent(const SQueuedAnimEvent& sEvent)
//{
//	if (!gEnv->bServer || gEnv->bEditor)
//		return;
//
//	//if (CCoopSystem::GetInstance()->GetDebugLog() > 1)
//	CryLogAlways("[%s] Animation Event Queued %s", __FUNCTION__, sEvent.sAnimEventName.c_str());
//
//	m_AnimEventQueue.push_back(sEvent);
//}

//void CTOSActor::UpdateAnimEvents(const float fFrameTime)
//{
//	if (!gEnv->bServer)
//		return;
//
//	for (auto iterator = m_AnimEventQueue.begin(); iterator != m_AnimEventQueue.end(); ++iterator)
//	{
//		SQueuedAnimEvent& animEvent = (*iterator);
//
//		animEvent.fElapsed += fFrameTime;
//
//		if (animEvent.fElapsed > animEvent.fEventTime)
//		{
//			//this->CreateScriptEvent("animationevent", 0.f, animEvent.sAnimEventName);
//
//			AnimEventInstance sEvent;
//			sEvent.m_EventName = animEvent.sAnimEventName;
//
//			this->AnimationEvent(GetEntity()->GetCharacter(0), sEvent);
//
//			m_AnimEventQueue.erase(iterator);
//
//			//if (CCoopSystem::GetInstance()->GetDebugLog() > 1)
//			CryLogAlways("[%s] Animation Event Played %s", __FUNCTION__, animEvent.sAnimEventName.c_str());
//
//			break;
//		}
//	}
//}

void CTOSActor::OnAGSetInput(bool bSucceeded, IAnimationGraphState::InputID id, float value, TAnimationGraphQueryID* pQueryID)
{
	
}

void CTOSActor::OnAGSetInput(bool bSucceeded, IAnimationGraphState::InputID id, int value, TAnimationGraphQueryID* pQueryID)
{
	
}

void CTOSActor::OnAGSetInput(const bool bSucceeded, const IAnimationGraphState::InputID id, const char* value, TAnimationGraphQueryID* pQueryID)
{
	//TODO:
	//20/10/2023 Не уверен, что это вообще работает
	//21/10/2023 Брейкпоинты так и не вызывались

	IAnimationGraphState* pState = m_pAnimatedCharacter ? m_pAnimatedCharacter->GetAnimationGraphState() : nullptr;
	if (bSucceeded && gEnv->bServer && !this->IsPlayer())
	{
		if (strcmp(value, m_sLastNetworkedAnim) != 0)
		{
			if (pState->GetInputId("Action") == id)
				GetGameObject()->InvokeRMI(ClPlayAnimation(), NetPlayAnimationParams(AIANIM_ACTION, value), eRMI_ToRemoteClients);
			if (pState->GetInputId("Signal") == id)
				GetGameObject()->InvokeRMI(ClPlayAnimation(), NetPlayAnimationParams(AIANIM_SIGNAL, value), eRMI_ToRemoteClients);

			m_sLastNetworkedAnim = value;
		}
		//SQueuedAnimEvent sAnimEvent = SQueuedAnimEvent();

		//if (this->IsAnimEvent(value, &sAnimEvent.sAnimEventName, &sAnimEvent.fEventTime))
		//{
		//	QueueAnimationEvent(sAnimEvent);
		//}

		//GetAnimationGraphState()->GetInputName(id);
	}

}

bool CTOSActor::IsLocalSlave() const
{
	const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	if (!pMC)
		return false;

	return pMC->GetSlaveEntity() == GetEntity();
}

CTOSEnergyConsumer* CTOSActor::GetEnergyConsumer() const
{
	assert(m_pEnergyConsumer);
	return m_pEnergyConsumer;
}

bool CTOSActor::UpdateLastMPSpawnPointRotation(const Quat& rotation)
{
	m_lastSpawnPointRotation = rotation;

	return true;
}

bool CTOSActor::UpdateLastShooterId(const EntityId id)
{
	m_lastShooterId = id;

	return true;
}

bool CTOSActor::HideMe(bool value)
{
	SActorStats* pActorStats = GetActorStats();
	if (pActorStats)
	{
		pActorStats->isHidden = value;

		uint32 slotFlags = GetEntity()->GetSlotFlags(0);

		if (value)
			slotFlags &= ~ENTITY_SLOT_RENDER;
		else
			slotFlags |= ENTITY_SLOT_RENDER;

		GetEntity()->SetSlotFlags(0, slotFlags);
		return true;
	}

	return false;
}

bool CTOSActor::SetMeSlave(bool value)
{
	m_isSlave = value;
	return true;
}

bool CTOSActor::SetMeMaster(bool value)
{
	m_isMaster = value;
	return true;
}

bool CTOSActor::SetMeZeus(bool value)
{
	m_isZeus = value;
	return true;
}

//void CTOSActor::NetMarkMeSlave(const bool slave) const
//{
//	CRY_ASSERT_MESSAGE(!IsPlayer(), "[MarkMeSlave] by design at 21/10/2023 the player cannot be a slave");
//	if (IsPlayer())
//		return;
//
//	if (gEnv->bClient)
//	{
//		NetMarkMeParams params;
//		params.value = slave;
//
//		GetGameObject()->InvokeRMI(SvRequestMarkMeAsSlave(), params, eRMI_ToServer);
//	}
//}
//
//void CTOSActor::NetMarkMeMaster(const bool master) const
//{
//	if (gEnv->bClient)
//	{
//		NetMarkMeParams params;
//		params.value = master;
//
//		GetGameObject()->InvokeRMI(SvRequestMarkMeAsMaster(), params, eRMI_ToServer);
//	}
//}

//const Vec3& CTOSActor::FilterDeltaMovement(const Vec3& deltaMov)
//{
//	//Скопировано из PlayerInput.cpp
//
//	const float frameTimeCap(min(gEnv->pTimer->GetFrameTime(), 0.033f));
//	const float inputAccel(gEnv->pConsole->GetCVar("tos_sv_pl_inputAccel")->GetFVal());
//
//	//const Vec3 oldFilteredMovement = m_filteredDeltaMovement;
//
//	if (deltaMov.len2() < 0.01f)
//	{
//		m_filteredDeltaMovement = {0,0,0};
//	}
//	else if (inputAccel < 0.1f)
//	{
//		m_filteredDeltaMovement = deltaMov;
//	}
//	else
//	{
//		Vec3 delta(deltaMov - m_filteredDeltaMovement);
//
//		const float len(delta.len());
//		if (len <= 1.0f)
//			delta = delta * (1.0f - len * 0.55f);
//
//		m_filteredDeltaMovement += delta * min(frameTimeCap * inputAccel, 1.0f);
//	}
//
//	//if (oldFilteredMovement.GetDistance(m_filteredDeltaMovement) > 0.001f)
//	//	GetGameObject()->ChangedNetworkState(TOS_NET::CLIENT_ASPECT_INPUT);
//
//	return m_filteredDeltaMovement;
//}

IMPLEMENT_RMI(CTOSActor, SvRequestPlayAnimation)
{
	// Описываем здесь всё, что будет выполняться на сервере

	GetGameObject()->InvokeRMI(ClPlayAnimation(), params, eRMI_ToRemoteClients);

	return true;
}

IMPLEMENT_RMI(CTOSActor, ClPlayAnimation)
{
	// Описываем здесь всё, что будет выполняться на клиенте

	IAnimationGraphState* pGraphState = (GetAnimatedCharacter() ? GetAnimatedCharacter()->GetAnimationGraphState() : nullptr);
	string mode;

	if (pGraphState)
	{
		if (params.mode == AIANIM_SIGNAL)
		{
			mode = "Signal";
		}
		else if (params.mode == AIANIM_ACTION)
		{
			mode = "Action";
		}

		pGraphState->SetInput(mode.c_str(), params.animation.c_str());
	}

	//CryLogAlways("[C++][%s][%s][%s] mode = %s, animation = %s", 
	//	TOS_Debug::GetEnv(), 
	//	TOS_Debug::GetAct(3), 
	//	__FUNCTION__, 
	//	mode.c_str(), params.animation.c_str());

	return true;
}

//IMPLEMENT_RMI(CTOSActor, SvRequestMarkMeAsMaster)
//{
//	// Описываем здесь всё, что будет выполняться на сервере
//
//	m_isMaster = params.value;
//	GetGameObject()->InvokeRMI(ClMarkMeAsMaster(), params, eRMI_ToAllClients);
//
//	GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
//
//	CryLog("[C++][%s][%s][%s][%s] mark as master = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, ClMarkMeAsMaster)
//{
//	// Описываем здесь всё, что будет выполняться на клиенте
//
//	m_isMaster = params.value;
//
//	CryLog("[C++][%s][%s][%s][%s] mark as master = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, SvRequestMarkMeAsSlave)
//{
//	// Описываем здесь всё, что будет выполняться на сервере
//
//	m_isSlave = params.value;
//
//	GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
//
//	CryLog("[C++][%s][%s][%s][%s] mark as slave = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//	
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, ClMarkMeAsSlave)
//{
//	// Описываем здесь всё, что будет выполняться на клиенте
//
//	m_isSlave = params.value;
//
//	CryLog("[C++][%s][%s][%s][%s] mark as slave = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//	return true;
//}

IMPLEMENT_RMI(CTOSActor, SvRequestHideMe)
{
	// Описываем здесь всё, что будет выполняться на сервере

	// 13.01.2024 Akeeper: Не уверен на счёт этих строк, но пусть они тут будут (519-521)
	const auto* pFists = static_cast<CFists*>(GetItemByClass(CItem::sFistsClass));
	if (pFists)
		g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(this, pFists->GetEntityId());

	GetGameObject()->SetAspectProfile(eEA_Physics, GetSpectatorMode() != 0 || params.hide ? eAP_Spectator : eAP_Alive);
	GetGameObject()->InvokeRMI(ClMarkHideMe(), params, eRMI_ToAllClients);	

	return true;
}

IMPLEMENT_RMI(CTOSActor, ClMarkHideMe)
{
	// Описываем здесь всё, что будет выполняться на клиенте

	HideMe(params.hide);
	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, ClTOSJump)
{
	CMovementRequest request;
	request.SetJump();
	GetMovementController()->RequestMovement(request);

	CryLog("[%s] Received jump", m_debugName);
	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, SvRequestTOSJump)
{
	auto channelId = g_pGame->GetIGameFramework()->GetGameChannelId(pNetChannel);
	GetGameObject()->InvokeRMI(ClTOSJump(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, channelId);
	GetGameObject()->Pulse('bang');

	if ((IsSlave() && !IsLocalSlave()) || IsClient())
	{
		CMovementRequest request;
		request.SetJump();
		GetMovementController()->RequestMovement(request);

		CryLog("[%s] Requesting jump to channel %i", m_debugName, channelId);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, ClAttachChild)
{
	IEntity* pChild = TOS_GET_ENTITY(params.id);
	if(pChild)
	{
		GetEntity()->AttachChild(pChild, params.flags);
		CryLog("[%s] Received attach child '%s'", m_debugName, pChild->GetName());
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, SvRequestAttachChild)
{
	IEntity* pChild = TOS_GET_ENTITY(params.id);
	if(pChild)
	{
		CryLog("[%s] Requesting attach child '%s'", m_debugName, pChild->GetName());
		GetEntity()->AttachChild(pChild, params.flags);
	}

	GetGameObject()->InvokeRMI(ClAttachChild(), params, eRMI_ToRemoteClients);

	return true;
}