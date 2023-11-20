// ReSharper disable CppInconsistentNaming
#include "StdAfx.h"
#include "TOSActor.h"

#include "Actor.h"
//#include "OffHand.h"

//#include "Aliens/TOSTrooper.h"

#include "TheOtherSideMP/Extensions/EnergyСonsumer.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Helpers/TOS_Inventory.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

CTOSActor::CTOSActor()
	:
	//m_filteredDeltaMovement(ZERO),
	m_isSlave(false),
	m_pEnergyConsumer(nullptr)
{
	
}

CTOSActor::~CTOSActor() = default;

bool CTOSActor::Init(IGameObject* pGameObject)
{
	if (!CActor::Init(pGameObject))
		return false;

	m_pEnergyConsumer = dynamic_cast<CTOSEnergyConsumer*>(GetGameObject()->AcquireExtension("TOSEnergyConsumer"));
	assert(m_pEnergyConsumer);

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
	if (gEnv->bServer && gEnv->bMultiplayer && !IsPlayer())
		GetEntity()->SetTimer(eMPTIMER_GIVEWEAPONDELAY, 1000);
}

void CTOSActor::InitClient(const int channelId)
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::InitClient] Actor: %s|%i|ch:%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId(), channelId);

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorInitClient, "", true, false, nullptr, 0.0f, channelId));

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
	case ENTITY_EVENT_TIMER:
	{
		if (event.nParam[0] == eMPTIMER_GIVEWEAPONDELAY)
		{
			string       equipName;
			const string actorClass = GetEntity()->GetClass()->GetName();

			if (actorClass == "Trooper")
			{
				equipName = gEnv->pConsole->GetCVar("tos_sv_TrooperMPEquipPack")->GetString();
			}
			else if (actorClass == "Scout")
			{
				equipName = gEnv->pConsole->GetCVar("tos_sv_ScoutMPEquipPack")->GetString();
			}
			else if (actorClass == "Alien")
			{
				equipName = gEnv->pConsole->GetCVar("tos_sv_AlienMPEquipPack")->GetString();
			}
			else if (actorClass == "Hunter")
			{
				equipName = gEnv->pConsole->GetCVar("tos_sv_HunterMPEquipPack")->GetString();
			}

			TOS_Inventory::GiveEquipmentPack(this, equipName.c_str());
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

	//if (m_pEnergyConsumer->NetSerialize(ser,aspect, profile, flags))
	//{
	//}

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
			const float energy = m_pEnergyConsumer->GetEnergy();
			const float maxEnergy = m_pEnergyConsumer->GetMaxEnergy();
			const float drain = m_pEnergyConsumer->GetDrainValue();
			const bool updating = m_pEnergyConsumer->IsUpdating();

			DRAW_2D_TEXT(40, 200, 1.3f, "--- Energy Consumer (%s) ---", pDebugEntity->GetName());
			DRAW_2D_TEXT(40, 215, 1.3f, "Updating:   %i", updating);
			DRAW_2D_TEXT(40, 230, 1.3f, "Energy:     %1.f", energy);
			DRAW_2D_TEXT(40, 245, 1.3f, "MaxEnergy:  %1.f", maxEnergy);
			DRAW_2D_TEXT(40, 260, 1.3f, "DrainValue: %1.f", drain);
		}
	}
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
}

void CTOSActor::Kill()
{
	CActor::Kill();

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

void CTOSActor::NetMarkMeSlave(const bool slave) const
{
	CRY_ASSERT_MESSAGE(!IsPlayer(), "[MarkMeSlave] by design at 21/10/2023 the player cannot be a slave");
	if (IsPlayer())
		return;

	if (gEnv->bClient)
	{
		NetMarkMeAsSlaveParams params;
		params.slave = true;

		GetGameObject()->InvokeRMI(SvRequestMarkMeAsSlave(), params, eRMI_ToServer);
	}
}

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

IMPLEMENT_RMI(CTOSActor, SvRequestMarkMeAsSlave)
{
	// Описываем здесь всё, что будет выполняться на сервере

	m_isSlave = params.slave;
	GetGameObject()->InvokeRMI(ClMarkMeAsSlave(), params, eRMI_ToAllClients);

	CryLogAlways("[C++][%s][%s][%s] mark as slave = %i",
		TOS_Debug::GetEnv(),
		TOS_Debug::GetAct(3),
		__FUNCTION__,
		params.slave);

	return true;
}

IMPLEMENT_RMI(CTOSActor, ClMarkMeAsSlave)
{
	// Описываем здесь всё, что будет выполняться на клиенте

	m_isSlave = params.slave;

	CryLogAlways("[C++][%s][%s][%s] mark as slave = %i", 
		TOS_Debug::GetEnv(), 
		TOS_Debug::GetAct(3), 
		__FUNCTION__, 
		params.slave);

	return true;
}