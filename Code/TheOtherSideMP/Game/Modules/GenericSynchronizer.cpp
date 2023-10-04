#include "StdAfx.h"

//#include "GenericModule.h"
#include "GenericSynchronizer.h"

#include "../TOSGameEventRecorder.h"

TSynches CTOSGenericSynchronizer::s_synchronizers;

CTOSGenericSynchronizer::CTOSGenericSynchronizer()
{
	//m_pModule = nullptr;
	
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

	//stl::push_back_unique(s_synchronizers, GetEntityId());
	s_synchronizers[GetEntity()->GetName()] = GetEntityId();
	//TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_SynchronizerCreated, "", true));
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
	TOS_RECORD_EVENT(GetEntityId(), STOSGameEvent(eEGE_SynchronizerDestroyed, "", true));

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
	//case ENTITY_EVENT_START_GAME:
	//	break;
	//case ENTITY_EVENT_RESET:
	//	break;	
	//case ENTITY_EVENT_INIT:
	//	break;
	//default: 
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

void CTOSGenericSynchronizer::GetSynchonizers(TSynches& synches)
{
	synches = s_synchronizers;
}

//CTOSGenericModule* CTOSGenericSynchronizer::GetModule()
//{
//	return m_pModule;
//}
//
//void CTOSGenericSynchronizer::SetModule(CTOSGenericModule* pModule)
//{
//	m_pModule = pModule;
//	assert(m_pModule);
//}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSGenericSynchronizer, SvRequestPintest)
{
	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestPintest] from %s",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary.c_str());
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSGenericSynchronizer, ClPintest)
{
	if (gEnv->bClient)
	{
		CryLogAlways("[C++][%s][%s][ClPintest] from %s",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.commentary.c_str());
	}

	return true;
}

