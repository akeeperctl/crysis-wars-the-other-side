#include "StdAfx.h"
#include "ControlSystem.h"
#include "Player.h"
#include "GameRules.h"

CNetControlClient::CNetControlClient(CPlayer* pActor)
{
	m_pLocalActor = pActor;
	m_pDesiredActor = nullptr;
	m_IsDesiredDudeBeam = false;
	m_IsDesiredDudeHide = false;

	//CryLogAlways("[CNetControlClient][Constructor][Player Name] %s", m_pLocalActor->GetEntity()->GetName());
	//CryLogAlways("[CNetControlClient][Constructor]");
}

CNetControlClient::~CNetControlClient()
{

}

void CNetControlClient::SetDesiredActor(IActor* pActor, bool reset)
{
	m_pDesiredActor = pActor;

	if (reset && m_pLocalActor && m_pLocalActor->GetControlClient())
	{

		auto* pDesActor = dynamic_cast<CActor*>(m_pDesiredActor);

		g_pControlSystem->StartLocal(pDesActor);

	}
		//g_pGame->GetGameRules()->StartControl(m_pDesiredActor->GetEntityId(), m_pLocalActor->GetEntityId(), false);

	if (pActor)
		CryLogAlways("[C++][CNetControlClient][SetDesiredActor] %s",pActor->GetEntity()->GetName());
}

void CNetControlClient::UpdateLocal(float frametime)
{
	if (m_pDesiredActor)
	{
		if (m_pLocalActor)
		{
			m_pLocalActor->GetControlClient()->m_mpLastControlledId = m_pDesiredActor->GetEntityId();
			ResetLocalDesired();
			CryLogAlways("[C++][CLIENT][NetControlClient][UpdateLocal][Channel %i][Set m_mpLastControlledId %i]", m_pLocalActor->GetChannelId(), m_pLocalActor->GetControlClient()->m_mpLastControlledId);
		}
		else
		{
			CryLogAlways("[C++][CLIENT][NetControlClient][UpdateLocal][local player NULL]");
		}
	}
}

void CNetControlClient::StartLocalDesired(IActor* pActor, bool dudeHide /*= 1*/, bool dudeBeam /*= 1*/)
{
	/*if (!pActor)
		return;

	if (!m_pDesiredActor)
	{
		m_pDesiredActor = pActor;
		m_IsDesiredDudeBeam = dudeBeam;
		m_IsDesiredDudeHide = dudeHide;
	}*/
}

void CNetControlClient::ResetLocalDesired()
{
	m_IsDesiredDudeBeam = false;
	m_IsDesiredDudeHide = false;
	m_pDesiredActor = nullptr;
}