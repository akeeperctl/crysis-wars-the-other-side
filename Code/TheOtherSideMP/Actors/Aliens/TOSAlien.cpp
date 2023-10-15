#include "StdAfx.h"
#include "TOSAlien.h"

#include "Player.h"

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

	if (aspect == SERVER_ASPECT_HEALTH)
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
	}

	if (aspect == CLIENT_ASPECT_INPUT)
	{
		//m_input.Serialize(ser);
		ser.Value("deltaMovement", m_input.deltaMovement, 'pMov'); //ok
		//ser.Value("movementVector", m_input.movementVector, 'pMov');
		//ser.Value("viewDir", m_input.viewDir, 'dir0'); //не протестировано
		//ser.Value("viewVector", m_input.viewVector, 'dir0'); //не протестировано
		//ser.Value("deltaRotation", m_input.deltaRotation); //не протестировано
		//ser.Value("posTarget", m_input.posTarget, 'wrld'); //не протестировано
		//ser.Value("upTarget", m_input.upTarget, 'wrld'); //не протестировано

		if (ser.IsWriting())
		{
			CryLogAlways("[%s] WRITE INPUT (%1.f, %1.f, %1.f)", GetEntity()->GetName(),m_input.deltaMovement.x, m_input.deltaMovement.y, m_input.deltaMovement.z );
		}
		else
		{
			CryLogAlways("[%s] READ INPUT (%1.f, %1.f, %1.f)", GetEntity()->GetName(), m_input.deltaMovement.x, m_input.deltaMovement.y, m_input.deltaMovement.z);
		}
	}


	return true;
}

void CTOSAlien::ProcessEvent(SEntityEvent& event)
{
	CAlien::ProcessEvent(event);

	if (event.event == ENTITY_EVENT_PREPHYSICSUPDATE)
	{
		if (gEnv->bClient)
		{
			//GetGameObject()->ChangedNetworkState(CLIENT_ASPECT_INPUT);
			//m_input.ResetDeltas();
		}
	}
}

void CTOSAlien::SetHealth(const int health)
{
	CAlien::SetHealth(health);

	if (gEnv->bServer)
	{
		GetGameObject()->ChangedNetworkState(SERVER_ASPECT_HEALTH);
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

void CTOSAlien::ApplyMasterMovement(const Vec3& delta)
{
	m_input.deltaMovement.x = clamp_tpl(m_input.deltaMovement.x + delta.x, -1.0f, 1.0f);
	m_input.deltaMovement.y = clamp_tpl(m_input.deltaMovement.y + delta.y, -1.0f, 1.0f);
	m_input.deltaMovement.z = clamp_tpl(m_input.deltaMovement.z + delta.z, -1.0f, 1.0f);

	m_input.deltaMovement.x = (delta.x < 0.0f || delta.x > 0.0f) ? m_input.deltaMovement.x : 0;
	m_input.deltaMovement.y = (delta.y < 0.0f || delta.y > 0.0f) ? m_input.deltaMovement.y : 0;
	m_input.deltaMovement.z = (delta.z < 0.0f || delta.z > 0.0f) ? m_input.deltaMovement.z : 0;
}
