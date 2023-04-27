#pragma once

enum ERequestState
{
	eRQ_AfterReset,
	eRQ_Created,
	eRQ_Assigned,
	eRQ_Executing,
	eRQ_Completed,
	eRQ_NotAssigned,
	eRQ_Failed,
	eRQ_FailedByExecutorKilled,
	eRQ_Cancelled,
	eRQ_Last,
};

enum ERequestType
{
	//Commented = not implemented
	eRT_NotDefined,
	//eRT_HumanTaxsee,
	eRT_AlienTaxsee,
	//eRT_EliminateTarget,
	//eRT_FollowMe2min,
	//eRT_FollowMe5min,
	//eRT_FollowMe10min,
	//eRT_GuardPoint2min,
	//eRT_GuardPoint5min,
	//eRT_GuardPoint10min,
};

class CRARSRequester
{
public:
	friend class CRequestsAndResponses;

	CRARSRequester()
	{
		Init();
	}
	CRARSRequester(EntityId id)
	{
		Init();
		m_entityId = id;
	}

	const inline uint GetLastRequestId() const { return m_lastRequestId; };
	const inline uint GetRequestId() const { return m_requestId; };
	const inline EntityId GetEntityId() const { return m_entityId; };

	const float GetLastTime(ERequestState state) const 
	{
		auto iter = m_lastTimes.find(state);
		if (iter != m_lastTimes.end())
			return iter->second;

		return 0.0f;
	};

	void UpdateStateTime(ERequestState state)
	{
		m_lastTimes[state] = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	}

	bool operator == (const CRARSRequester* pRequester)
	{
		if (!pRequester)
			return false;

		return pRequester->GetEntityId() == this->GetEntityId();
	}

	bool operator == (const CRARSRequester& requester)
	{
		return (requester.GetEntityId() == this->GetEntityId());
	}

private:
	void Init()
	{
		m_requestId = 0;
		m_lastRequestId = 0;
		m_entityId = 0;

		for (int i = 0; i < eRQ_Last; i++)
			m_lastTimes[ERequestState(i)] = 0.0f;
	}

	void SetRequestId(uint id)
	{
		if (m_requestId != id)
		{
			m_lastRequestId = m_requestId;
			m_requestId = id;
		}
	}

	uint m_requestId;
	uint m_lastRequestId;
	EntityId m_entityId;
	std::map<ERequestState, float> m_lastTimes;
};

class CRARSRequest
{
public:
	friend class CRequestsAndResponses;

	CRARSRequest()
	{
		reset();
	}

	void reset()
	{
		state = eRQ_AfterReset;
		type = eRT_NotDefined;
		goalEntityId = 0;
		requesterId = 0;
		id = 0;

		lastTimeAssigned = 0;
		lastTimeCompleted = 0;
		lastTimeCreated = 0;
		lastTimeFailed = 0;
	}

	ERequestState state;
	ERequestType type;
	EntityId goalEntityId;
	EntityId requesterId;

	float lastTimeCreated;
	float lastTimeAssigned;
	float lastTimeCompleted;
	float lastTimeFailed;

	const uint getId() const
	{
		return id;
	}

	bool operator == (CRARSRequest* pRequest)
	{
		if (!pRequest)
			return false;

		return (pRequest->getId() == this->getId());
	}

	bool operator == (CRARSRequest& request)
	{
		return (request.getId() == this->getId());
	}


private:
	uint id;
};