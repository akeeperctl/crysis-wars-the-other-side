#include "StdAfx.h"

#include "RMISender.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

CTOSMasterRMISender::CTOSMasterRMISender()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_RMISenderCreated, "MasterModule", true));
}

CTOSMasterRMISender::~CTOSMasterRMISender()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_RMISenderDestroyed, "MasterModule", true));
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

//Not actual any more
////------------------------------------------------------------------------
//IMPLEMENT_RMI(CTOSMasterRMISender, SvRequestMasterAdd)
//{
//	if (gEnv->bServer)
//	{
//		auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
//		assert(pEntity);
//
//		CryLogAlways(" ");
//		CryLogAlways("[C++][SERVER][RMI RECEIVED][SvRequestMasterAdd] MasterEntity: %s",
//			 pEntity->GetName());
//		//[RMI RECEIVED][SERVER][SvRequestMasterAdd] NetChannel: lmlicenses.wip4.adobe.com:50632, MasterEntity: Akeeper
//
//		g_pTOSGame->GetMasterModule()->MasterAdd(pEntity);
//	}
//
//	return true;
//}
//
////------------------------------------------------------------------------
//IMPLEMENT_RMI(CTOSMasterRMISender, SvRequestMasterRemove)
//{
//	if (gEnv->bServer)
//	{
//		auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
//		assert(pEntity);
//
//		CryLogAlways(" ");
//		CryLogAlways("[C++][%s][%s][SvRequestMasterRemove] MasterEntity: %s",
//			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), pEntity->GetName());
//		//[RMI RECEIVED][SERVER][SvRequestMasterAdd] NetChannel: lmlicenses.wip4.adobe.com:50632, MasterEntity: Akeeper
//
//		g_pTOSGame->GetMasterModule()->MasterRemove(pEntity);
//	}
//
//	return true;
//}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterRMISender, SvRequestPintest)
{
	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestPintest] from %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3),params.commentary);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterRMISender, ClPintest)
{
	if (gEnv->bClient)
	{
		CryLogAlways("[C++][%s][%s][ClPintest] from %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary);
	}

	return true;
}

