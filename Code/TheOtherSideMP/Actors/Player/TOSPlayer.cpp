#include "StdAfx.h"
#include "TOSPlayer.h"
#include "TheOtherSideMP\Game\Modules\MasterSystem\MasterClient.h"

CTOSPlayer::CTOSPlayer():
	m_pMasterClient(0)
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

void CTOSPlayer::InitClient(int channelId)
{
	CPlayer::InitClient(channelId);

	if (gEnv->bServer)
	{
		CryLogAlways(" ");
		CryLogAlways("[C++][SERVER][FUNC CALL][CPlayer::InitClient] channelId: %i, ThisPlayer: %s", channelId, GetEntity()->GetName());
	}
	else if(gEnv->bClient)
	{
		CryLogAlways(" ");
		CryLogAlways("[C++][CLIENT][FUNC CALL][CPlayer::InitClient] channelId: %i, ThisPlayer: %s", channelId, GetEntity()->GetName());
	}

	//Case 3
	//if (!m_pMasterClient)
	//{
	//	m_pMasterClient = new CTOSMasterClient(this);
	//}
}

void CTOSPlayer::InitLocalPlayer()
{
	CPlayer::InitLocalPlayer();

	if (gEnv->bServer)
	{
		CryLogAlways(" ");
		CryLogAlways("[C++][SERVER][FUNC CALL][CPlayer::InitLocalPlayer] Player: %s", GetEntity()->GetName());
	}
	else if (this->GetEntityId() == g_pGame->GetIGameFramework()->GetClientActorId())
	{
		CryLogAlways(" ");
		CryLogAlways("[C++][LOCAL][FUNC CALL][CPlayer::InitLocalPlayer] Player: %s", GetEntity()->GetName());
	}
	else if (gEnv->bClient)
	{
		CryLogAlways(" ");
		CryLogAlways("[C++][CLIENT][FUNC CALL][CPlayer::InitLocalPlayer] Player: %s", GetEntity()->GetName());
	}

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

bool CTOSPlayer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CPlayer::NetSerialize(ser,aspect,profile,flags))
		return false;

	return true;
}

void CTOSPlayer::Release()
{
	SAFE_DELETE(m_pMasterClient);
	CActor::Release();
}

CTOSMasterClient* CTOSPlayer::GetMasterClient()
{
	return m_pMasterClient;
}
