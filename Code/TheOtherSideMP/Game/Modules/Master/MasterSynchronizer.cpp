#include "StdAfx.h"

#include "MasterSynchronizer.h"

#include "MasterClient.h"

#include "../../TOSGameEventRecorder.h"

#include "TheOtherSideMP/Helpers/TOS_AI.h"

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
IMPLEMENT_RMI(CTOSMasterSynchronizer, ClMasterClientStartControl)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		const auto localPlayerNick = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetName();

		CryLogAlways("[C++][%s][%s][ClMasterClientStartControl] LocalPlayerNick: %s", 
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3), localPlayerNick);

		const auto pSlaveEntity = gEnv->pEntitySystem->GetEntity(params.slaveId);
		assert(pSlaveEntity);

		// В данном случае params.masterId равен 0, т.к. мы уже на локальном клиенте,
		// который имеет мастер-клиент и локального игрока

		constexpr uint flags = TOS_DUDE_FLAG_BEAM_MODEL | 
			TOS_DUDE_FLAG_DISABLE_SUIT | 
			TOS_DUDE_FLAG_ENABLE_ACTION_FILTER | 
			TOS_DUDE_FLAG_HIDE_MODEL;

		g_pTOSGame->GetMasterModule()->GetMasterClient()->StartControl(pSlaveEntity, flags);
	}

	return true;
}
////------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSMasterSynchronizer, SvRequestMasterClientStartControl)
{
	// Здесь пишем всё, что должно выполниться на сервере

	if (gEnv->bServer)
	{
		CryLogAlways("[C++][%s][%s][SvRequestMasterClientStartControl]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		const auto pSlaveEntity = gEnv->pEntitySystem->GetEntity(params.slaveId);
		const auto pMasterEntity = gEnv->pEntitySystem->GetEntity(params.masterId);
		assert(pSlaveEntity);
		assert(pMasterEntity);

		// В данном случае сервер не знает какому мастеру нужно прописать полученного раба.
		// Поэтому мы передаём серверу информацию как о рабе, так и о мастере.
		g_pTOSGame->GetMasterModule()->SetSlave(pMasterEntity, pSlaveEntity);
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

		g_pTOSGame->GetMasterModule()->ClearSlave(pMasterEntity);
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