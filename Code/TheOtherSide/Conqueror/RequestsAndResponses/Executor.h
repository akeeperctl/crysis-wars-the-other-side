#pragma once

enum EExecutionState
{
	eES_NotAssigned,
	eES_Assigned,
	eES_MoveToRequester,
	eES_PerformingRequest,
	eES_FinishRequest,
	eES_Last
};

class CRARSExecutor
{
public:
	CRARSExecutor();
	CRARSExecutor(EntityId id);

	void AddAllowedRequestType(ERequestType type);
	void GetAllowedRequestTypes(std::vector<ERequestType>& requests);

	const inline EExecutionState GetCurrentState() const { return m_executionState; };
	const inline uint GetCurrentRequestId() const { return m_requestId; };
	void SetCurrentRequest(CRARSRequest* pRequest);
	void	SetCurrentState(EExecutionState state);
	const inline EntityId GetEntityId() const { return m_entityId; };

	float m_lastTimeRequestChange;
	float m_lastTimeStateChange;

private:
	void Init();

	EExecutionState m_executionState;
	uint m_requestId;
	EntityId m_entityId;
	std::vector<ERequestType> m_allowedRequestTypes;
};