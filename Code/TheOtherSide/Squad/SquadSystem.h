#pragma once

#include "HUD/GameFlashAnimation.h"

struct SControlClient;
struct SSquadSystem;
struct SSquad;
struct SMember;

//class CGameFlashAnimation;

typedef std::vector<EntityId> TSMembers;
typedef std::vector<SMember> TMembers;
typedef std::vector<SSquad> TSquads;

enum ESquadOrders
{
	eSquadOrders_GoTo = 0,
	eSquadOrders_SearchEnemy = 1,
	eSquadOrders_FollowLeader = 2,
	eSquadOrders_None = 3,
	eSquadOrders_ForSync
};

enum EGotoUpdateState
{
	eGotoUpdateState_CleanAI = 0,//Set when the player execute the command "Goto"
	eGotoUpdateState_GoTo,//Set when AI is cleaned and select ord_goto pipe in update cycle
	eGotoUpdateState_Guard,//Set when AI complete goalpipe ord_goto and select ord_guard
	eGotoUpdateState_Combat,//Set when AI is on guard and see enemy
	eGotoUpdateState_ForSync
};

struct SMember
{
	friend struct SSquadSystem;
	friend struct SSquad;

	SMember();
	SMember(IActor* _pAct);
	SMember(EntityId _id);

	~SMember();

	void Reset();

	void Serialize(TSerialize ser);
	void GetMemoryStatistics(ICrySizer* s);

	int index;
	int aiCleanDuration; // 300 - 3 sec
	bool isLeader;
	Vec3 guardPos;
	Vec3 searchPos;
	//IActor* pActor;
	EntityId actorId;
	ESquadOrders currentOrder;
	EGotoUpdateState currentGotoState;
	SSquadSystem* m_pSquadSystem;
};

struct SSquad
{
	friend struct SSquadSystem;
	friend struct SMember;

	SSquad();
	SSquad(IActor* _Leader, uint _squadId);
	~SSquad();
public:
	bool AddMember(SMember& member);
	bool AddMember(IActor* pActor);
	bool AddMember(EntityId id);

	bool RemoveMember(SMember& member);
	bool RemoveMember(IActor* pActor);
	bool RemoveMember(EntityId id);

	SMember& GetMember(const EntityId id);
	SMember& GetMember(const IActor* pActor);
	SMember& GetMemberAlive();
	SMember& GetMemberFromIndex(const int index);

	//Summary
	//Get the count of existing members in the squad but also leader not included
	int GetMembersCount() const { return m_squadMembers.size() + 0; };
	//int GetTestMembersCount() const { return m_testMembers.size() + 0; };

	//Set/Get members array methods
	void	 SetAllMembers(TMembers members) { m_squadMembers = members; };
	TMembers& GetAllMembers() { return m_squadMembers; };

	void	SetLeader(IActor* pLeader);
	IActor* GetLeader() const { return m_pLeader; };// The leader determines if the squad exists

	//Order methods
	void ExecuteOrder(ESquadOrders order, SMember& Member);
	bool ExecuteOrderFG(ESquadOrders order, SMember& Member, Vec3& refPoint = Vec3(0, 0, 0));

	int GetOrder(const SMember& Member);
	int GetOrder(const EntityId id);
	int GetOrder(const IActor* act);
	int	GetOrder(const int index);

	ESquadOrders GetOrderLeader() const { return m_eLeaderCurrentOrder; };
	void		 SetOrderLeader(ESquadOrders order) { m_eLeaderCurrentOrder = order; };

	//Getting a squad member from an EntityId and returning bool True if it in squad. If not, then return False.
	bool IsMember(const SMember& Member) const;
	bool IsMember(const IActor* pActor) const;
	bool IsMember(const EntityId id) const;

	bool IsMemberSelected(const SMember& Member);
	bool IsMemberSelected(const IActor* pActor);
	//bool IsMemberSelected(EntityId id);
	bool IsMemberSelected(const int index);

	int	GetIndexFromMember(const SMember& Member);
	int	GetIndexFromMember(const IActor* pActor);
	int	GetIndexFromMember(const EntityId id);
	int	GetFreeMemberIndex() const;

	//Summary
	//Get the minimal distance between the leader and squad members
	float GetMinDistance() const;

	void RemoveMemberFromSelected(const SMember& member);
	void RemoveMemberFromSelected(const IActor* pActor);
	//void RemoveMemberFromSelected(EntityId id);
	void RemoveMemberFromSelected(const int index);

	void AddMemberToSelected(const SMember& member);
	void AddMemberToSelected(IActor* pActor);
	//void AddMemberToSelected(EntityId id);
	void AddMemberToSelected(const int index);

	bool isPlayerMember() const;
	bool isPlayerLeader() const;

	void OnPlayerAdded();//Called when the local player join in any squad
	void PlayerRemoved();//Called when the local player left from any squad

	void Update();
	void UpdateMembersHUD();

	IActor* GetActor(EntityId id) { return g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id); };

	void Reset()
	{
		TMembers::iterator it = m_squadMembers.begin();
		TMembers::iterator end = m_squadMembers.end();
		for (; it != end; it++)
		{
			it->Reset();
		}
		m_searchRadius = 20;
		m_squadId = -1;
		m_pLeader = 0;
		m_squadMembers.clear();
		m_squadSelectedMembers.clear();
	};

	void Serialize(TSerialize ser);

	void SetId(const int id) { m_squadId = id; };
	int GetId() const { return m_squadId; };

	void GetMemoryStatistics(ICrySizer* s);
protected:
	int m_squadId;
	IActor* m_pLeader;
	//TMembers m_testMembers;
	TMembers m_squadMembers;
	TSMembers m_squadSelectedMembers;
	ESquadOrders m_eLeaderCurrentOrder;
	int m_searchRadius;
	SSquadSystem* m_pSquadSystem;
};

struct SSquadSystem
{
	// enums and typedefs and nested classes
public:
	friend struct SControlClient;
	friend struct SSquad;
	friend struct SMember;

	SSquadSystem();
	~SSquadSystem();

	//Methods
private:
	//void		UpdateOrders();
	void		UpdateHUD();
	void		UpdateSelectedHUD();//Update selected members hud

	void		ResetSystem();

	void		ShowPlayerOrder();
	void		ShowAllSquadControlsRed(bool active);
	void		ShowAllSquadControls(bool active);
	void		ShowSquadMember(const bool active, const int slot);
	void		ShowDeadSquadMember(int slot); //now we need show as dead only last member

	void		ShowSquadControl(int index, bool show);//show the '1' or '2' squad control hud

	bool		OnActionCommandMode(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionSwitchControlled(EntityId entityId, const ActionId& actionId, int activationMode, float value);//Not implemented
	bool		OnActionOrderFollow(EntityId entityId, const ActionId& actionId, int activationMode, float value);//Z
	bool		OnActionSwitchOrder(EntityId entityId, const ActionId& actionId, int activationMode, float value);//T
	bool		OnActionExecuteOrder(EntityId entityId, const ActionId& actionId, int activationMode, float value);//R
	bool		OnActionSelectOne(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionSelectTwo(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionSelectThree(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionSelectAll(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionSelectNone(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool		OnActionTest(EntityId entityId, const ActionId& actionId, int activationMode, float value);

public:
	bool		OnAction(const ActionId& actionId, int activationMode, float value);
	void		InitHUD(bool active);
	void		RemoveHUD();//Called when the local player left from any squad

	//Squad system methods
	void Update();
	void FullSerialize(TSerialize ser);
	SSquad& GetSquadFromMember(IActor* pActor, bool includeLeader);
	SSquad& GetSquadFromLeader(IActor* pLeader);
	SSquad& GetSquadFromId(int squadId);
	bool CreateSquad(SSquad& squad);//Return true when Squad with its id is created
	bool RemoveSquad(SSquad& squad);//Return true when Squad is removed
	int GetFreeSquadIndex() const;

	void GetMemoryStatistics(ICrySizer* s);

	//Members
public:
	//SSquad m_dudeSquad;
	TSquads m_allSquads;
	bool m_isCommandMode;

protected:
	CGameFlashAnimation m_animSquadMembers;
private:
	bool m_mustShowSquadControls;
	static TActionHandler<SSquadSystem> s_actionHandler;
};