#pragma once

#include "IGameObjectSystem.h"

class CRARSRequest;

class CRARSExecutor;
class CConquerorSystem;
class CConquerorCommander;
enum ESpeciesType;

#include "Request.h"
#include "Executor.h"

enum EExecutorGetFlag
{
	eEGF_Any = BIT(0),
	eEGF_IsPerformingRequest = BIT(1),
	eEGF_IsFree = BIT(2),
	eEGF_IsInCombat = BIT(3),
	eEGF_IsNotInCombat = BIT(4),
	eEGF_IsNearest = BIT(5)
};

class CRequestsAndResponses
{
public:
	friend class CRARSExecutor;
	friend class CConquerorSystem;

	CRequestsAndResponses();
	~CRequestsAndResponses();

	CRARSRequest* CreateRequest(EntityId requesterId, EntityId goalEntityId, ERequestType type);
	CRARSRequest* GetRequestById(uint id);
	CRARSRequest* GetRequestFromEntity(EntityId requesterId);
	void CompleteRequest(uint id);
	void CancelRequest(uint id);
	void RemoveRequest(uint id);
	void FailRequest(uint id, int cause);// 0 - executor is dead, 1 - executor not assigned

	EntityId AssignExecutor(CRARSRequest* pRequest);
	void AddExecutor(ESpeciesType species, EntityId id, const std::vector<ERequestType>& requestTypes);

	CRARSExecutor* GetExecutorInstance(EntityId id);
	CRARSRequester* GetRequesterInstance(EntityId id);


	EntityId GetExecutorIdByFlags(ESpeciesType species, const Vec3& pos, uint flags);
	EntityId GetExecutorIdByFlags(ESpeciesType species, const Vec3& pos, uint flags, ERequestType allowedRequestType);
	EntityId GetAssignedExecutorId(CRARSRequest* pRequest);

	bool IsAssignedExecutor(EntityId id);
	bool IsExecutor(EntityId id);
	bool IsExecutor(ESpeciesType species, EntityId id);

	void RemoveExecutor(EntityId id);
	void RemoveRequester(EntityId id);

	void Reset();

	void Update(float frametime);
	void UpdateExecutors(float frametime);
	void UpdateRequests(float frametime);

	void OnActorGrab(IActor* pActor, EntityId grabId);
	void OnActorDrop(IActor* pActor, EntityId dropId);

	bool SetRequestGoal(EntityId ExecutorId, EntityId goalId);

	//Enum to String conversion functions

	string GetString(ERequestType type)
	{
		if (type == eRT_AlienTaxsee)
			return "AlienTaxsee";
		else if (type == eRT_NotDefined)
			return "NotDefined";

		return "NullString";
	}

	string GetString(ERequestState state)
	{
		switch (state)
		{
		case eRQ_AfterReset:
			return "AfterReset";
			break;
		case eRQ_Created:
			return "Created";
			break;
		case eRQ_Assigned:
			return "Assigned";
			break;
		case eRQ_Executing:
			return "Executing";
			break;
		case eRQ_Completed:
			return "Completed";
			break;
		case eRQ_NotAssigned:
			return "NotAssigned";
			break;
		case eRQ_FailedByExecutorKilled:
			return "FailedByExecutorKilled";
			break;
		case eRQ_Cancelled:
			return "Cancelled";
			break;
		case eRQ_Last:
			return "Last";
			break;
		case eRQ_Failed:
			return "Failed";
			break;
		}

		return "NullString";
	}

private:
	//const CRARSRequest GetDefaultRequest() const;

	std::map<ESpeciesType, std::vector<CRARSExecutor>> m_speciesExecutors;
	std::vector<CRARSRequest> m_requests;
	std::vector<CRARSRequester> m_requesters;
	float m_lastUpdateTime;
};