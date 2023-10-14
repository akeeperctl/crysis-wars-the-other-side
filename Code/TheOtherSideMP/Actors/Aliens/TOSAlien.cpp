#include "StdAfx.h"
#include "TOSAlien.h"

CTOSAlien::CTOSAlien()
{
}

CTOSAlien::~CTOSAlien()
{
}

void CTOSAlien::PostInit(IGameObject* pGameObject)
{
	CAlien::PostInit(pGameObject);
}

void CTOSAlien::Update(SEntityUpdateContext& ctx, const int updateSlot)
{
	CAlien::Update(ctx, updateSlot);
}

// ReSharper disable once CppParameterMayBeConst
bool CTOSAlien::NetSerialize(TSerialize ser, const EEntityAspects aspect, const uint8 profile, const int flags)
{
	if (!CAlien::NetSerialize(ser,aspect,profile,flags))
		return false;

	if (aspect == ASPECT_HEALTH)
	{
		ser.Value("health", m_health);

		if (ser.IsWriting())
		{
			CryLogAlways("WRITE HEALTH %1.f", m_health);
		}
		else
		{
			CryLogAlways("READ HEALTH %1.f", m_health);
		}

		//if (ser.IsWriting())
		//{
		//	//float convertedHp = (m_health / m_maxHealth) * 100.0f;
		//	ser.Value("health", convertedHp, 'hlth');
		//	//CryLogAlways("WRITE HEALTH %1.f", convertedHp);
		//}
		//else
		//{
		//	//float convertedHp = 0.0f;
		//	ser.Value("health", convertedHp, 'hlth');
		//	//m_health = convertedHp * m_maxHealth;
		//	//CryLogAlways("READ HEALTH %1.f", m_health);

		//}
	}


	return true;
}

void CTOSAlien::SetHealth(const int health)
{
	CAlien::SetHealth(health);

	if (gEnv->bServer)
	{
		GetGameObject()->ChangedNetworkState(ASPECT_HEALTH);
	}
}

Matrix33 CTOSAlien::GetViewMtx()
{
	return m_viewMtx;
}

Matrix33 CTOSAlien::GetBaseMtx()
{
	return m_baseMtx;
}
Matrix33 CTOSAlien::GetEyeMtx()
{
	return m_eyeMtx;
}
