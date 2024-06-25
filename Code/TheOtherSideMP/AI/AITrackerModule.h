// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyClangDiagnosticInvalidSourceEncoding
#pragma once

#include "IActorSystem.h"

#include "TheOtherSideMP/Game/Modules/GenericModule.h"
#include "TheOtherSideMP/Game/Modules/ITOSGameModule.h"

struct IControlSystemChild;
struct IGoalPipeListener;
struct IAIActionTrackerListener;
struct IActor;

enum EAIActionExecutingFlag
{
	//if you change this also change the CAIActionTracker::GetString(EAAEFlag flag) func

	eAAEF_None,
	eAAEF_IgnoreCombatDuringAction,
	//AI will exit combat if it is in combat and will not participate in it until it completes the ai action
	eAAEF_JoinCombatPauseAction,
	//When performing an action, the AI will enter combat and pause the action, after the combat, it will return to performing the action
};

typedef EAIActionExecutingFlag EAAEFlag;

struct SAIActionInfo
{
	SAIActionInfo()
	{
		name = nullptr;
		desiredGoalPipe = nullptr;
		goalPipeId = -1;
		flag = eAAEF_None;
		paused = false;
		objectId = -1;
		maxAlertness = 0;
	}

	bool operator ==(const SAIActionInfo& info) const { return strcmp(info.name, this->name) == 0; }
	bool operator ==(const SAIActionInfo* info) const { return strcmp(info->name, this->name) == 0; }

	SAIActionInfo& operator =(const SAIActionInfo& info)
	{
		if (this == &info)
			return *this;

		this->desiredGoalPipe = info.desiredGoalPipe;
		this->flag = info.flag;
		this->goalPipeId = info.goalPipeId;
		this->maxAlertness = info.maxAlertness;
		this->name = info.name;
		this->objectId = info.objectId;
		this->paused = info.paused;

		return *this;
	}

	SAIActionInfo& operator =(const SAIActionInfo* info)
	{
		this->desiredGoalPipe = info->desiredGoalPipe;
		this->flag = info->flag;
		this->goalPipeId = info->goalPipeId;
		this->maxAlertness = info->maxAlertness;
		this->name = info->name;
		this->objectId = info->objectId;
		this->paused = info->paused;

		return *this;
	}

	EntityId    objectId;
	bool        paused;
	const char* name;
	const char* desiredGoalPipe;
	int         goalPipeId;
	int         maxAlertness;
	EAAEFlag    flag;
};

struct SVoidHolder
{
	SVoidHolder() { id = 0; }

	SVoidHolder(const EntityId _id, const SAIActionInfo& _actionInfo)
	{
		id = _id;
		actionInfo.desiredGoalPipe = _actionInfo.desiredGoalPipe;
		actionInfo.flag = _actionInfo.flag;
		actionInfo.goalPipeId = _actionInfo.goalPipeId;
		actionInfo.maxAlertness = _actionInfo.maxAlertness;
		actionInfo.name = _actionInfo.name;
		actionInfo.objectId = _actionInfo.objectId;
		actionInfo.paused = _actionInfo.paused;
	}

	//const bool operator == (EntityId _id) const {return this->id == _id;}
	//const bool operator != (EntityId _id) const {return this->id != _id; }
	//const bool operator < (EntityId _id) const { return this->id < _id;}
	//const bool operator <= (EntityId _id) const { return this->id <= _id; }
	//const bool operator > (EntityId _id) const { return this->id > _id; }
	//const bool operator >= (EntityId _id) const { return this->id >= _id; }

	//fix C2678 
	bool operator <(const SVoidHolder& void2) const { return this->id < void2.id; }

	EntityId      id;
	SAIActionInfo actionInfo;
};

struct SAIActionStats
{
	SAIActionStats()
	{
		lastTimeFinished = 0;
		lastTimeAborted = 0;
		lastTimeFailed = 0;
		lastTimeStarted = 0;
		lastTimeVoidDetect = 0;
		actionName = "NullActionName";
	}

	float  lastTimeFinished;
	float  lastTimeAborted;
	float  lastTimeFailed;
	float  lastTimeStarted;
	float  lastTimeVoidDetect;
	string actionName;
};

/**
 * \brief Модуль отслеживает действия ИИ во время игры.
	\note Указатель на модуль существует только на сервере.
	\note Указатель на модуль на клиенте равен nullptr.
 */
class СTOSAIModule  : public CTOSGenericModule, IGoalPipeListener
{
public:
	СTOSAIModule();
	~СTOSAIModule() ;

	bool IsExecuting(const IAIObject* pUserAI, int actionGoalPipeId);
	bool IsExecuting(const IAIObject* pUserAI, const char* actionName);
	bool IsExecuting(const IAIObject* pUserAI, const char* actionName, const char* desiredGoalPipe);
	bool IsExecuting(const IAIObject* pUserAI, const char* actionName, const IEntity* pObject);

	//void OnVehicleDestroyed(const IVehicle* pVeh);

	//ITOSGameModule
	bool OnInputEvent(const SInputEvent& event)  { return true; };
	bool OnInputEventUI(const SInputEvent& event)  { return false; };

	void        OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) ;
	void        GetMemoryStatistics(ICrySizer* s) ;
	const char* GetName()  { return "СTOSAIModule"; }
	void        Init() ;
	void        Update(float frametime) ;
	void        Serialize(TSerialize ser) ;

	void InitCVars(IConsole* pConsole) ;
	void InitCCommands(IConsole* pConsole) ;
	void ReleaseCVars() ;
	void ReleaseCCommands() ;
	//~ITOSGameModule

	//IGoalPipeListener
	void OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId) ;
	//~IGoalPipeListener

	bool IsTracking(const IAIObject* pAI) const;
	bool IsTracking(EntityId Id) const;
	bool StartTracking(IAIObject* pAI, const SAIActionInfo& actionInfo);
	void StopTracking(EntityId Id);
	void StopTracking(IAIObject* pAI);

	bool IsPaused(IAIObject* pAI) const;
	void SetPausedAction(IAIObject* pAI, bool paused);

	bool IsFinished(const IAIObject* pAI, const char* actionName) const;

	void GetActionInfo(EntityId id, SAIActionInfo& actionInfo) const;
	void GetActionInfo(const IAIObject* pAI, SAIActionInfo& actionInfo) const;
	bool GetActionStats(const IAIObject* pAI, SAIActionStats& actionStats) const;

	//Listeners
	//void AddListener(IAIActionTrackerListener* pListener);
	//void RemoveListener(IAIActionTrackerListener* pListener);
	//~Listeners

	//Events
	void OnActionPaused(IAIObject* pAIObject) const;
	void OnActionUnpaused(IAIObject* pAIObject) const;
	void OnActionAborted(const IAIObject* pAIObject);
	//~Events

protected:

	string GetString(const EGoalPipeEvent event) const
	{
		switch (event)
		{
		case ePN_Removed:
			return "Removed";
		case ePN_OwnerRemoved:
			return "OwnerRemoved";
		case ePN_Finished:
			return "Finished";
		case ePN_Deselected:
			return "Deselected";
		case ePN_Suspended:
			return "Suspended";
		case ePN_Resumed:
			return "Resumed";
		case ePN_AnimStarted:
			return "AnimStarted";
		case ePN_RefPointMoved:
			return "RefPointMoved";
		}

		return "<undefined>";
	}

	string GetString(const EAAEFlag flag) const
	{
		switch (flag)
		{
		case eAAEF_None:
			return "None";
		case eAAEF_IgnoreCombatDuringAction:
			return "IgnoreCombatDuringAction";
		case eAAEF_JoinCombatPauseAction:
			return "JoinCombatPauseAction";
		}

		return "<undefined>";
	}

private:
	void Reset();
	void SetActionInfo(const IAIObject* pAI, const SAIActionInfo& actionInfo);

	//Stats created when StartTracking
	//Stats not be cleared when StopTracking
	std::map<EntityId, SAIActionStats> m_entitiesStats;

	//who is being tracked?
	//what is he doing at the moment?
	std::map<EntityId, SAIActionInfo> m_entitiesActions;

	std::map<SVoidHolder, float>           m_voidHolders;
	//std::vector<IAIActionTrackerListener*> m_listeners;
};
