#include "StdAfx.h"
#include "TOSAlien.h"

#include "Player.h"

#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"

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

	if (aspect == TOS_NET::SERVER_ASPECT_HEALTH)
	{
		ser.Value("health", m_health);

		//if (ser.IsWriting())
		//{
		//	CryLogAlways("WRITE HEALTH %1.f", m_health);
		//}
		//else
		//{
		//	CryLogAlways("READ HEALTH %1.f", m_health);
		//}
	}

	if (aspect == TOS_NET::CLIENT_ASPECT_INPUT)
	{
		//m_input.Serialize(ser);
		ser.Value("deltaMovement", m_input.deltaMovement, 'pMov'); //tr ok
		//ser.Value("movementVector", m_input.movementVector, 'pMov');
		ser.Value("viewDir", m_input.viewDir, 'dir0'); //tr ok 
		//ser.Value("viewVector", m_input.viewVector, 'dir0'); //не протестировано
		//ser.Value("deltaRotation", m_input.deltaRotation); //не протестировано
		//ser.Value("posTarget", m_input.posTarget, 'wrld'); //не протестировано
		//ser.Value("upTarget", m_input.upTarget, 'wrld'); //не протестировано
		//ser.Value("basemtx", (Quat)m_baseMtx); //не протестировано

		if (ser.IsWriting())
		{
			CryLogAlways("[%s] WRITE INPUT (%1.f, %1.f, %1.f)", GetEntity()->GetName(), m_input.viewDir.x, m_input.viewDir.y, m_input.viewDir.z);
		}
		else
		{
			CryLogAlways("[%s] READ INPUT (%1.f, %1.f, %1.f)", GetEntity()->GetName(), m_input.viewDir.x, m_input.viewDir.y, m_input.viewDir.z);
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
			float color[] = { 1,1,1,1 };
			gEnv->pRenderer->Draw2dLabel(100, 160, 1.3f, color, false, "Slave: m_input.deltaMovement = (%f,%f,%f)", m_input.deltaMovement.x, m_input.deltaMovement.y, m_input.deltaMovement.z);
			gEnv->pRenderer->Draw2dLabel(100, 100, 1.3f, color, false, "Slave: m_filteredDeltaMovement = (%f,%f,%f)", m_filteredDeltaMovement.x, m_filteredDeltaMovement.y, m_filteredDeltaMovement.z);

		}
	}
}

void CTOSAlien::SetHealth(const int health)
{
	CAlien::SetHealth(health);

	if (gEnv->bServer)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_HEALTH);
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
	//m_input.deltaMovement = FilterDeltaMovement(delta);

	m_input.deltaMovement.x = clamp_tpl(m_input.deltaMovement.x + delta.x, -1.0f, 1.0f);
	m_input.deltaMovement.y = clamp_tpl(m_input.deltaMovement.y + delta.y, -1.0f, 1.0f);
	m_input.deltaMovement.z = clamp_tpl(m_input.deltaMovement.z + delta.z, -1.0f, 1.0f);

	m_input.deltaMovement.x = (delta.x < 0.0f || delta.x > 0.0f) ? m_input.deltaMovement.x : 0;
	m_input.deltaMovement.y = (delta.y < 0.0f || delta.y > 0.0f) ? m_input.deltaMovement.y : 0;
	m_input.deltaMovement.z = (delta.z < 0.0f || delta.z > 0.0f) ? m_input.deltaMovement.z : 0;
}
