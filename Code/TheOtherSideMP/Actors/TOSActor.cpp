#include "StdAfx.h"
#include "TOSActor.h"

#include "Actor.h"

#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

CTOSActor::CTOSActor() : 
	//m_filteredDeltaMovement(ZERO),
	m_slaveEntityId(0),
	m_masterEntityId(0)
{
}

CTOSActor::~CTOSActor()
{

}

void CTOSActor::PostInit(IGameObject* pGameObject)
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::PostInit] Actor: %s|%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId());

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorPostInit, "", true));

	CActor::PostInit(pGameObject);

	m_netBodyInfo.Reset();
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
	{
		pMC->OnEntityEvent(GetEntity(), event);
	}
}

// ReSharper disable once CppParameterMayBeConst
bool CTOSActor::NetSerialize(TSerialize ser, const EEntityAspects aspect, const uint8 profile, const int flags)
{
	if (!CActor::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSActor::Update(SEntityUpdateContext& ctx, const int updateSlot)
{
	CActor::Update(ctx, updateSlot);

	//const auto pMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	//if (pMC)
	//{
	//	pMC->Update(GetEntity());
	//}
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

void CTOSActor::SetMasterEntityId(const EntityId id)
{
	//gEnv->pRenderer->GetFrameID();
	m_masterEntityId = id;
}

void CTOSActor::SetSlaveEntityId(const EntityId id)
{
	m_slaveEntityId = id;
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
