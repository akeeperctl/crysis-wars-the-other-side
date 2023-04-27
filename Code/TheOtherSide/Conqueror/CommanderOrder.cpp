#include "StdAfx.h"

#include "Actor.h"

#include <ISerialize.h>
#include <SerializeFwd.h>

#include "CommanderOrder.h"


void CCommanderOrder::Update(float frametime)
{
	
}

void CCommanderOrder::SetState(EOrderExecuteState state)
{
	if (m_oldState != state)
	{
		m_oldState = m_state;
		m_state = state;
		m_lastStateChange = gEnv->pTimer->GetFrameStartTime();

		//CryLogAlways("[C++][Order %i][Set State][%i]", m_type, m_state);

		//m_vehicleRequested = false;
	}
};

void CCommanderOrder::SetType(ECommanderOrders type)
{
	if (m_oldType != type)
	{
		m_oldType = m_type;
		m_type = type;
		m_lastTypeChange = gEnv->pTimer->GetFrameStartTime();

		//m_vehicleRequested = false;
		//m_vehicleId = 0;
	}
};

void CCommanderOrder::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		ser.Value("target", m_targetId);
		ser.Value("pos", m_pos);
		//ser.Value("vehicleRequested", m_vehicleRequested);
		//ser.Value("vehicleId", m_vehicleId);
		ser.EnumValue("order", m_type, eCO_FirstOrder, eCO_LastOrder);
		ser.EnumValue("oldorder", m_oldType, eCO_FirstOrder, eCO_LastOrder);
		ser.EnumValue("state", m_state, eOES_NotStarted, eOES_Last);
		ser.EnumValue("oldstate", m_oldState, eOES_NotStarted, eOES_Last);
	}
};