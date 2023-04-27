#pragma once


enum EOrderCause
{
	eOC_DefendOurTerritory,
	eOC_ExpandOurTerritory,
	eOC_HelpInDefending,
	eOC_HelpInExpanding,

	eOC_Last
};

enum EOrderExecuteState
{
	eOES_NotStarted,
	eOES_Starting,
	eOES_InMovingToTarget,
	//eOES_InMovingToTargetAfterRespawn,
	eOES_PerformingAction,

	eOES_Last
};

enum ECommanderOrders
{
	eCO_FirstOrder,
	//eCO_GoTo = 0,
	eCO_Attack,
	eCO_Defend,
	eCO_LastOrder
};

class CCommanderOrder
{
public:

	friend class CConquerorCommander;

	CCommanderOrder()
	{
		m_currentOrderTime = 0.f;
		m_lastStateChange = 0.f;
		m_lastTypeChange = 0.f;

		SetType(eCO_Defend);
		SetState(eOES_NotStarted);
		m_pos = Vec3(0, 0, 0);
		m_targetId = 0;

		m_oldState = eOES_NotStarted;
		m_oldType = eCO_Defend;

		//m_vehicleRequested = false;
		//m_vehicleId = 0;
	};

	CCommanderOrder(ECommanderOrders _orderType, Vec3& orderPos)
	{
		m_currentOrderTime = 0.f;
		m_lastStateChange = 0.f;
		m_lastTypeChange = 0.f;

		SetType(_orderType);
		SetState(eOES_NotStarted);
		m_pos = orderPos;
		m_targetId = 0;

		m_oldState = eOES_NotStarted;
		m_oldType = eCO_FirstOrder;

		//m_vehicleRequested = false;
		//m_vehicleId = 0;
	};

	CCommanderOrder(ECommanderOrders _orderType, Vec3& orderPos, EOrderExecuteState _state)
	{
		m_currentOrderTime = 0.f;
		m_lastStateChange = 0.f;
		m_lastTypeChange = 0.f;

		SetType(_orderType);
		SetState(_state);		
		m_pos = orderPos;
		m_targetId = 0;

		m_oldState = eOES_NotStarted;
		m_oldType = eCO_FirstOrder;

		//m_vehicleRequested = false;
		//m_vehicleId = 0;
	};

	CCommanderOrder(ECommanderOrders _orderType, Vec3& orderPos, EOrderExecuteState _state, EntityId _entityId)
	{
		m_currentOrderTime = 0.f;
		m_lastStateChange = 0.f;
		m_lastTypeChange = 0.f;

		SetType(_orderType);
		SetState(_state);
		m_pos = orderPos;
		m_targetId = _entityId;

		m_oldState = eOES_NotStarted;
		m_oldType = eCO_FirstOrder;

		//m_vehicleRequested = false;
		//m_vehicleId = 0;
	};

	void Update(float frametime);
	void SetState(EOrderExecuteState state);
	void SetType(ECommanderOrders type);
	void Serialize(TSerialize ser);

private:
	EOrderExecuteState m_state;
	EOrderExecuteState m_oldState;
	ECommanderOrders m_type;
	ECommanderOrders m_oldType;
	Vec3 m_pos;
	EntityId m_targetId;
	CTimeValue m_currentOrderTime;
	CTimeValue m_lastStateChange;
	CTimeValue m_lastTypeChange;
	
	//bool m_vehicleRequested;
	//EntityId m_vehicleId;
};