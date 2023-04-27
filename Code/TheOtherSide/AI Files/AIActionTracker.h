#pragma once

#include "IActorSystem.h"

struct IControlSystemChild;
struct IGoalPipeListener;
struct IAIActionTrackerListener;
struct IActor;

enum EAIActionExecutingFlag
{
	//if you change this also change the CAIActionTracker::GetString(EAAEFlag flag) func

	eAAEF_None,
	eAAEF_IgnoreCombatDuringAction,//AI will exit combat if it is in combat and will not participate in it until it completes the ai action
	eAAEF_JoinCombatPauseAction,//When performing an action, the AI will enter combat and pause the action, after the combat, it will return to performing the action
};

typedef EAIActionExecutingFlag EAAEFlag;

struct SAIActionInfo
{
	SAIActionInfo()
	{
		name = 0;
		desiredGoalPipe = 0;
		goalPipeId = -1;
		flag = eAAEF_None;
		paused = false;
		objectId = -1;
		maxAlertness = 0;
	}

	bool operator == (const SAIActionInfo& info) { return strcmp(info.name, this->name) == 0; }
	bool operator == (const SAIActionInfo* info) { return strcmp(info->name, this->name) == 0; }
	void operator = (const SAIActionInfo& info) 
	{ 
		this->desiredGoalPipe = info.desiredGoalPipe;
		this->flag = info.flag;
		this->goalPipeId = info.goalPipeId;
		this->maxAlertness = info.maxAlertness;
		this->name = info.name;
		this->objectId = info.objectId;
		this->paused = info.paused;
	}
	void operator = (const SAIActionInfo* info) 
	{
		this->desiredGoalPipe = info->desiredGoalPipe;
		this->flag = info->flag;
		this->goalPipeId = info->goalPipeId;
		this->maxAlertness = info->maxAlertness;
		this->name = info->name;
		this->objectId = info->objectId;
		this->paused = info->paused;
	}

	EntityId objectId;
	bool paused;
	const char* name;
	const char* desiredGoalPipe;
	int goalPipeId;
	int maxAlertness;
	EAAEFlag flag;
};

struct SVoidHolder
{
	SVoidHolder()
	{
		id = 0;
	}

	SVoidHolder(const EntityId _id, const SAIActionInfo& _actionInfo)
	{
		id = _id;
		actionInfo.desiredGoalPipe = _actionInfo.desiredGoalPipe;
		actionInfo.flag =			 _actionInfo.flag;
		actionInfo.goalPipeId =		 _actionInfo.goalPipeId;
		actionInfo.maxAlertness =	 _actionInfo.maxAlertness;
		actionInfo.name =			 _actionInfo.name;
		actionInfo.objectId =		 _actionInfo.objectId;
		actionInfo.paused =			 _actionInfo.paused;
	}

	//const bool operator == (EntityId _id) const {return this->id == _id;}
	//const bool operator != (EntityId _id) const {return this->id != _id; }
	//const bool operator < (EntityId _id) const { return this->id < _id;}
	//const bool operator <= (EntityId _id) const { return this->id <= _id; }
	//const bool operator > (EntityId _id) const { return this->id > _id; }
	//const bool operator >= (EntityId _id) const { return this->id >= _id; }

	//fix C2678 
	bool operator < (const SVoidHolder& void2) const { return this->id < void2.id; }

	EntityId id;
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

	float lastTimeFinished;
	float lastTimeAborted;
	float lastTimeFailed;
	float lastTimeStarted;
	float lastTimeVoidDetect;
	string actionName;
};

class CAIActionTracker : public IControlSystemChild, IGoalPipeListener
{
public:
	CAIActionTracker();
	~CAIActionTracker();

	bool	IsExecuting(const IAIObject* pUserAI, int actionGoalPipeId);
	bool	IsExecuting(const IAIObject* pUserAI, const char* actionName);
	bool	IsExecuting(const IAIObject* pUserAI, const char* actionName, const char* desiredGoalPipe);
	bool	IsExecuting(const IAIObject* pUserAI, const char* actionName, IEntity* pObject);

	void		OnVehicleDestroyed(IVehicle* pVeh);

	//IControlSystemChild
	void		OnMainMenuEnter() override;
	void		OnGameRulesReset() override;
	void		OnActorDeath(IActor* pActor) override;
	void		OnActorGrabbed(IActor* pActor, EntityId grabberId) override;
	void		OnActorDropped(IActor* pActor, EntityId droppedId) override;
	void		OnActorGrab(IActor* pActor, EntityId grabId) override;
	void		OnActorDrop(IActor* pActor, EntityId dropId) override;
	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) override;
	void		OnExitVehicle(IActor* pActor) override;
	virtual bool OnInputEvent(const SInputEvent& event) override;
	void		Init() override; //Add this struct as child in the function
	void		Update(float frametime) override;
	void		Serialize(TSerialize ser) override;
	void		GetMemoryStatistics(ICrySizer* s) override;
	const char* GetChildName() { return "CAIActionTracker"; };
	//~IControlSystemChild

	//IGoalPipeListener
	virtual void OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId);
	//~IGoalPipeListener


	string	GetString(EGoalPipeEvent event) const;
	string	GetString(EAAEFlag flag) const;

	bool	IsTracking(IAIObject* pAI) const;
	bool	IsTracking(EntityId Id) const;
	bool	StartTracking(IAIObject* pAI, const SAIActionInfo& actionInfo);
	void	StopTracking(EntityId Id);
	void	StopTracking(IAIObject* pAI);

	bool	IsPaused(IAIObject* pAI);
	void	SetPausedAction(IAIObject* pAI, bool paused);

	bool	IsFinished(IAIObject* pAI, const char* actionName);

	void	GetActionInfo(EntityId id, SAIActionInfo& actionInfo) const;
	void	GetActionInfo(const IAIObject* pAI, SAIActionInfo& actionInfo) const;
	bool	GetActionStats(const IAIObject* pAI, SAIActionStats& actionStats) const;

	//Listeners
	void AddListener(IAIActionTrackerListener* pListener);
	void RemoveListener(IAIActionTrackerListener* pListener);
	//~Listeners

	//Events
	void OnActionPaused(IAIObject* pAIObject);
	void OnActionUnpaused(IAIObject* pAIObject);
	void OnActionAborted(IAIObject* pAIObject);
	//~Events

protected:
private:

	void	Reset();
	void	SetActionInfo(const IAIObject* pAI, const SAIActionInfo& actionInfo);

	//Stats created when StartTracking
	//Stats not be cleared when StopTracking
	std::map<EntityId, SAIActionStats> m_entitiesStats;

	//who is being tracked?
	//what is he doing at the moment?
	std::map<EntityId, SAIActionInfo> m_entitiesActions;

	std::map<SVoidHolder, float> m_voidHolders;
	std::vector<IAIActionTrackerListener*> m_listeners;
};