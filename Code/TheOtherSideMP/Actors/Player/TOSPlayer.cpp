#include "StdAfx.h"
#include "TOSPlayer.h"

#include "../../Game/TOSGame.h"
#include "../../Game/Modules/Master/MasterClient.h"
#include "../../Game/Modules/Master/MasterSynchronizer.h"

CTOSPlayer::CTOSPlayer():
	m_pMasterClient(nullptr)
{
}

CTOSPlayer::~CTOSPlayer()
{
	
}

void CTOSPlayer::PostInit(IGameObject* pGameObject)
{
	CPlayer::PostInit(pGameObject);

	//Case 1 - Master Client was created only on local machine
	// Not working at dedicated server
	//if (GetEntityId() == g_pGame->GetIGameFramework()->GetClientActorId())
	//{
	//	m_pLocalMasterClient = new CTOSMasterClient(this);
	//}

	//Case 2 - Master Client was created on client
	// It is ok on dedicated but calling two cases on not dedicated
	//if (!m_pMasterClient)
	//{
	//	m_pMasterClient = new CTOSMasterClient(this);
	//}
}

void CTOSPlayer::InitClient(const int channelId)
{
	CPlayer::InitClient(channelId);

	//if (gEnv->bServer)
	//{
	//	CryLogAlways(" ");
	//	CryLogAlways("[C++][SERVER][FUNC CALL][CPlayer::InitClient] channelId: %i, ThisPlayer: %s", channelId, GetEntity()->GetName());
	//}
	//else if(gEnv->bClient)
	//{
	//	CryLogAlways(" ");
	//	CryLogAlways("[C++][CLIENT][FUNC CALL][CPlayer::InitClient] channelId: %i, ThisPlayer: %s", channelId, GetEntity()->GetName());
	//}

	//Case 3
	//if (!m_pMasterClient)
	//{
	//	m_pMasterClient = new CTOSMasterClient(this);
	//}
}

void CTOSPlayer::InitLocalPlayer()
{
	CPlayer::InitLocalPlayer();

	//if (gEnv->bServer)
	//{
	//	CryLogAlways(" ");
	//	CryLogAlways("[C++][%s][%s][CTOSPlayer::InitLocalPlayer] Player: %s", 
	//		TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName());
	//}
	//else if (this->GetEntityId() == g_pGame->GetIGameFramework()->GetClientActorId())
	//{
	//	CryLogAlways(" ");
	//	CryLogAlways("[C++][%s][%s][CTOSPlayer::InitLocalPlayer] Player: %s", 
	//		TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName());
	//}
	//else if (gEnv->bClient)
	//{
	//	CryLogAlways(" ");
	//	CryLogAlways("[C++][%s][%s][CTOSPlayer::InitLocalPlayer] Player: %s", 
	//		TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName());
	//}

	CryLogAlways("[C++][%s][%s][CTOSPlayer::InitLocalPlayer] Player: %s",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), GetEntity()->GetName());

	//Case 4 perfect - Master Client was created only on local machine
	if (!m_pMasterClient)
	{
		m_pMasterClient = new CTOSMasterClient(this);
	}
}

void CTOSPlayer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CPlayer::Update(ctx,updateSlot);
}

// ReSharper disable once CppParameterMayBeConst
bool CTOSPlayer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CPlayer::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSPlayer::Release()
{
	//if (gEnv->bServer)
	//	CryLogAlways("[C++][SERVER][FUNC CALL][CTOSPlayer::Release] PlayerId: %i", GetEntityId());
	//else if (gEnv->bClient)
	//	CryLogAlways("[C++][CLIENT][FUNC CALL][CTOSPlayer::Release] PlayerId: %i", GetEntityId());

	//Case 2 not work
	/*if (gEnv->bClient)
	{
		auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
		assert(pSender);

		MasterAddingParams params;
		params.entityId = 3323;

		pSender->RMISend(CTOSMasterRMISender::SvRequestMasterAdd(), params, eRMI_ToServer);
	}*/

	//Case 3 not work
	//auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
	//assert(pSender);
	//
	//PintestParams params;
	//params.commentary = "[CTOSPlayer::Release]";

	//if (gEnv->bClient)
	//	pSender->RMISend(CTOSMasterRMISender::SvRequestPintest(), params, eRMI_ToServer);

	SAFE_DELETE(m_pMasterClient);
	CPlayer::Release();
}

CTOSMasterClient* CTOSPlayer::GetMasterClient()
{
	return m_pMasterClient;
}
