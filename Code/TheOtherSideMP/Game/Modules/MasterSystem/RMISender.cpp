#include "StdAfx.h"

#include "RMISender.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

CTOSMasterRMISender::CTOSMasterRMISender()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_RMISenderCreated, "By Master Module", true));
}

CTOSMasterRMISender::~CTOSMasterRMISender()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_RMISenderDestroyed, "By Master Module", true));
}

bool CTOSMasterRMISender::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetGameObject()->EnablePostUpdates(this);

	return true;
}

void CTOSMasterRMISender::PostInit(IGameObject* pGameObject)
{
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_WithoutAI);
	pGameObject->EnablePostUpdates(this);
}

void CTOSMasterRMISender::InitClient(int channelId)
{

}

void CTOSMasterRMISender::PostInitClient(int channelId)
{
	//At this moment can Invoke RMI
}

void CTOSMasterRMISender::Release()
{
	delete this;
}

void CTOSMasterRMISender::FullSerialize(TSerialize ser)
{
}

bool CTOSMasterRMISender::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

void CTOSMasterRMISender::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CTOSMasterRMISender::HandleEvent(const SGameObjectEvent& event)
{
	//switch (event.event)
	//{
	//case ENTITY_EVENT_RESET:
	//	break;
	//case ENTITY_EVENT_ENTERAREA:
	//	break;
	//case ENTITY_EVENT_LEAVEAREA:
	//	break;
	//}
}

void CTOSMasterRMISender::ProcessEvent(SEntityEvent& event)
{

}

void CTOSMasterRMISender::SetAuthority(bool auth)
{

}

void CTOSMasterRMISender::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterRMISender, SvRequestMasterAdd)
{
	if (gEnv->bServer)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
		assert(pEntity);

		CryLogAlways(" ");
		CryLogAlways("[C++][SERVER][RMI RECEIVED][SvRequestMasterAdd] MasterEntity: %s",
			 pEntity->GetName());
		//[RMI RECEIVED][SERVER][SvRequestMasterAdd] NetChannel: lmlicenses.wip4.adobe.com:50632, MasterEntity: Akeeper

		g_pTOSGame->GetMasterModule()->MasterAdd(pEntity);
	}

	return true;
}