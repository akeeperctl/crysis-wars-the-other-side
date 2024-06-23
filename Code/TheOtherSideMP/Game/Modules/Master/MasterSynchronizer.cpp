#include "StdAfx.h"

#include "MasterSynchronizer.h"

#include "MasterClient.h"

#include "../../TOSGameEventRecorder.h"

#include "TheOtherSideMP/Helpers/TOS_AI.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

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

	g_pTOSGame->GetMasterModule()->NetSerialize(ser,aspect, profile, flags);

	return true;
}

void CTOSMasterSynchronizer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CTOSGenericSynchronizer::Update(ctx,updateSlot);
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
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestMasterRemove)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestMasterRemove]", TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pEntity = gEnv->pEntitySystem->GetEntity(params.entityId);
		assert(pEntity);

		g_pTOSGame->GetMasterModule()->MasterRemove(pEntity);
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
IMPLEMENT_RMI(CTOSMasterSynchronizer, ClMasterClientStartControl)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		const auto pSlaveActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(params.slaveId);
		assert(pSlaveActor);

		const auto slaveIsPlayer = pSlaveActor->IsPlayer();
		CRY_ASSERT_MESSAGE(!slaveIsPlayer, "[ClMasterClientStartControl] by design at 21/10/2023 the real player cannot be a slave");

		if (slaveIsPlayer)
			return true;

		constexpr uint flags = 
			TOS_DUDE_FLAG_BEAM_MODEL | 
			TOS_DUDE_FLAG_DISABLE_SUIT | 
			TOS_DUDE_FLAG_ENABLE_ACTION_FILTER | 
			TOS_DUDE_FLAG_HIDE_MODEL;

		// В данном случае params.masterId равен 0, т.к. мы уже на локальной машине,
		// который имеет мастер-клиент и локального игрока
		g_pTOSGame->GetMasterModule()->GetMasterClient()->StartControl(pSlaveActor->GetEntity(), flags);

		const auto localPlayerNick = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetName();

		CryLogAlways("[C++][%s][%s][ClMasterClientStartControl] LocalPlayerNick: %s",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), localPlayerNick);
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestMasterClientStartControl)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		const auto pSlaveActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(params.slaveId);
		const auto pMasterActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(params.masterId);

		CRY_ASSERT_MESSAGE(pSlaveActor, "Slave actor pointer is NULL when try start controlling");
		CRY_ASSERT_MESSAGE(pMasterActor, "Master actor pointer is NULL when try start controlling");

		if (!pSlaveActor || !pMasterActor)
			return true;

		// Защита от дурака :)
		const auto slaveIsPlayer = pSlaveActor->IsPlayer();
		const auto masterIsPlayer = pMasterActor->IsPlayer();

		CRY_ASSERT_MESSAGE(!slaveIsPlayer, "[SvRequestMasterClientStartControl] by design at 21/10/2023 the real player cannot be a slave");
		CRY_ASSERT_MESSAGE(masterIsPlayer, "[SvRequestMasterClientStartControl] by design at 21/10/2023 the master only can be a real player");

		if (slaveIsPlayer || !masterIsPlayer)
			return true;

		// В данном случае сервер не знает какому мастеру нужно прописать полученного раба.
		// Поэтому мы передаём серверу информацию как о рабе, так и о мастере.
		g_pTOSGame->GetMasterModule()->SetCurrentSlave(pMasterActor->GetEntity(), pSlaveActor->GetEntity(), params.masterFlags);

		CryLogAlways("[C++][%s][%s][SvRequestMasterClientStartControl]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestMasterClientStopControl)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestMasterClientStopControl]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pMasterEntity = gEnv->pEntitySystem->GetEntity(params.masterId);
		assert(pMasterEntity);

		g_pTOSGame->GetMasterModule()->ClearCurrentSlave(pMasterEntity);
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestDelegateAuthority)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestDelegateAuthority] ChannelId: %i, SlaveId: %i",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), params.masterChannelId, params.slaveId);

		const auto pSlaveEntity = gEnv->pEntitySystem->GetEntity(params.slaveId);
		assert(pSlaveEntity);

		const auto pPlayer = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActorByChannelId(params.masterChannelId);
		assert(pPlayer);

		TOS_NET::DelegateAuthority(pPlayer->GetGameObject(), params.slaveId);
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, ClMasterClientStopControl)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		CryLogAlways("[C++][%s][%s][ClMasterClientStopControl]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		g_pTOSGame->GetMasterModule()->GetMasterClient()->StopControl();
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestSaveMCParams)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestSaveMCParams]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pMasterEntity = gEnv->pEntitySystem->GetEntity(params.masterId);
		assert(pMasterEntity);

		const auto pMM = g_pTOSGame->GetMasterModule();
		assert(pMM);

		pMM->SaveMasterClientParams(pMasterEntity);
	}

	return true;
}

////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestApplyMCSavedParams)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestApplyMCSavedParams]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pMasterEntity = gEnv->pEntitySystem->GetEntity(params.masterId);
		assert(pMasterEntity);

		const auto pMM = g_pTOSGame->GetMasterModule();
		assert(pMM);

		pMM->ApplyMasterClientParams(pMasterEntity);
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