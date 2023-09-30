#include "StdAfx.h"

#include "Synchronizer.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

CTOSMasterSynchronizer::CTOSMasterSynchronizer()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_SynchronizerCreated, "MasterModule", true));
}

CTOSMasterSynchronizer::~CTOSMasterSynchronizer()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_SynchronizerDestroyed, "MasterModule", true));
}

bool CTOSMasterSynchronizer::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetGameObject()->EnablePostUpdates(this);

	return true;
}

void CTOSMasterSynchronizer::PostInit(IGameObject* pGameObject)
{
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_WithoutAI);
	pGameObject->EnablePostUpdates(this);
}

void CTOSMasterSynchronizer::InitClient(int channelId)
{

}

void CTOSMasterSynchronizer::PostInitClient(int channelId)
{
	//At this moment can Invoke RMI
}

void CTOSMasterSynchronizer::Release()
{
	delete this;
}

void CTOSMasterSynchronizer::FullSerialize(TSerialize ser)
{
}

bool CTOSMasterSynchronizer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

void CTOSMasterSynchronizer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CTOSMasterSynchronizer::HandleEvent(const SGameObjectEvent& event)
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

void CTOSMasterSynchronizer::ProcessEvent(SEntityEvent& event)
{

}

void CTOSMasterSynchronizer::SetAuthority(bool auth)
{

}

void CTOSMasterSynchronizer::GetMemoryStatistics(ICrySizer* s)
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
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestPintest)
{
	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestPintest] from %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3),params.commentary);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, ClPintest)
{
	if (gEnv->bClient)
	{
		CryLogAlways("[C++][%s][%s][ClPintest] from %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary);
	}

	return true;
}

