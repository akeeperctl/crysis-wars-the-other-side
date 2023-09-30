#include "StdAfx.h"

#include "GenericSynchronizer.h"
#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

CTOSGenericSynchronizer::CTOSGenericSynchronizer()
{
}

CTOSGenericSynchronizer::~CTOSGenericSynchronizer()
{
}

bool CTOSGenericSynchronizer::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetGameObject()->EnablePostUpdates(this);

	return true;
}

void CTOSGenericSynchronizer::PostInit(IGameObject* pGameObject)
{
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_WithoutAI);
	pGameObject->EnablePostUpdates(this);
}

void CTOSGenericSynchronizer::InitClient(int channelId)
{

}

void CTOSGenericSynchronizer::PostInitClient(int channelId)
{
	//At this moment can Invoke RMI
}

void CTOSGenericSynchronizer::Release()
{
	delete this;
}

void CTOSGenericSynchronizer::FullSerialize(TSerialize ser)
{
}

bool CTOSGenericSynchronizer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

void CTOSGenericSynchronizer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CTOSGenericSynchronizer::HandleEvent(const SGameObjectEvent& event)
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

void CTOSGenericSynchronizer::ProcessEvent(SEntityEvent& event)
{

}

void CTOSGenericSynchronizer::SetAuthority(bool auth)
{

}

void CTOSGenericSynchronizer::GetMemoryStatistics(ICrySizer* s)
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
IMPLEMENT_RMI(CTOSGenericSynchronizer, SvRequestPintest)
{
	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestPintest] from %s",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSGenericSynchronizer, ClPintest)
{
	if (gEnv->bClient)
	{
		CryLogAlways("[C++][%s][%s][ClPintest] from %s",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary);
	}

	return true;
}

