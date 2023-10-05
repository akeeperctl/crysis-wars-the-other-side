#include "StdAfx.h"

#include "MasterSynchronizer.h"
#include "../../TOSGameEventRecorder.h"

CTOSMasterSynchronizer::CTOSMasterSynchronizer()
{
}

CTOSMasterSynchronizer::~CTOSMasterSynchronizer()
{
}

void CTOSMasterSynchronizer::PostInit(IGameObject* pGameObject)
{
	CTOSGenericSynchronizer::PostInit(pGameObject);
}

void CTOSMasterSynchronizer::Release()
{
	CTOSGenericSynchronizer::Release();
}

void CTOSMasterSynchronizer::FullSerialize(TSerialize ser)
{
	CTOSGenericSynchronizer::FullSerialize(ser);
}

bool CTOSMasterSynchronizer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CTOSGenericSynchronizer::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSMasterSynchronizer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CTOSMasterSynchronizer::HandleEvent(const SGameObjectEvent& event)
{
	CTOSGenericSynchronizer::HandleEvent(event);
}

void CTOSMasterSynchronizer::ProcessEvent(SEntityEvent& event)
{
	CTOSGenericSynchronizer::ProcessEvent(event);
}

void CTOSMasterSynchronizer::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

// Пример
// IMPLEMENT_RMI(CTOSGenericSynchronizer, SvRequestPintest)

////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestMasterAdd)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestMasterAdd]", TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
		assert(pEntity);

		g_pTOSGame->GetMasterModule()->MasterAdd(pEntity, params.desiredSlaveClassName);
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestSetDesiredSlaveCls)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestSetDesiredSlaveCls]", TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));
		// Указатель на класс уже проверен на этапе перед отправкой RMI
		const auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
		assert(pEntity);

		if (pEntity)
		{
			g_pTOSGame->GetMasterModule()->SetMasterDesiredSlaveCls(pEntity, params.desiredSlaveClassName);
		}

	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, ClMasterStartControl)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		const auto localPlayerNick = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetName();

		CryLogAlways("[C++][%s][%s][ClMasterStartControl] LocalPlayerNick: %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), localPlayerNick);

		const auto pSlaveEntity = gEnv->pEntitySystem->GetEntity(params.slaveId);
		assert(pSlaveEntity);


	}

	return true;
}
//Not actual any more
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