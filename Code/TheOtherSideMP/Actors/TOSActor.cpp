#include "StdAfx.h"
#include "TOSActor.h"
#include "Actor.h"

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
	CActor::PostInit(pGameObject);

	CryLogAlways("[C++][CallFunc][CTOSActor::PostInit] Actor: %s", 
		GetEntity()->GetName());
}

bool CTOSActor::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	CActor::NetSerialize(ser,aspect,profile,flags);
	return false;
}

void CTOSActor::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CActor::Update(ctx, updateSlot);
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
