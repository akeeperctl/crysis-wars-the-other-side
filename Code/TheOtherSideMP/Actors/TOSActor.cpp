#include "StdAfx.h"
#include "TOSActor.h"
#include "Actor.h"
#include "../Game/TOSGameEventRecorder.h"

CTOSActor::CTOSActor() : 
	m_masterEntityId(0),
	m_slaveEntityId(0)
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
}

void CTOSActor::InitClient(int channelId)
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::InitClient] Actor: %s|%i|ch:%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId(), channelId);

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorInitClient, "", true, false, 0.0f, channelId));

	CActor::InitClient(channelId);
}

bool CTOSActor::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CActor::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSActor::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CActor::Update(ctx, updateSlot);
}

void CTOSActor::Release()
{
	//CryLogAlways("[C++][%s][%s][CTOSActor::Release] Actor: %s|%i",
	//	TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName(), GetEntity()->GetId());

	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_ActorReleased, "", true));

	CActor::Release();
}

void CTOSActor::SetMasterEntityId(EntityId id)
{
	//gEnv->pRenderer->GetFrameID();
	m_masterEntityId = id;
}

void CTOSActor::SetSlaveEntityId(EntityId id)
{
	m_slaveEntityId = id;
}
