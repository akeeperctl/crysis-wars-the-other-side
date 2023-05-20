#pragma once

#include "../Control/IControlSystemChild.h"
#include "Actor.h"
#include "HUD/GameFlashAnimation.h"

enum ESpeciesType;

struct ISound;
struct SOrderInfo;

class CSquad;
class CMember;
class CControlClient;
class CSquadSystem;

enum EDetachedRoutineType
{
	eDRT_FirstType = 0,

	eDRT_OnFootPathPatrol = 0,

	eDRT_LandPathPatrol,
	eDRT_LandPointGuard,

	eDRT_AirPathPatrol,
	eDRT_AirPointSearch,

	eDRT_WaterPathPatrol,
	eDRT_WaterPointPatrol,

	eDRT_LastType
};

enum ESquadCreateFlag
{
	eSCF_NewLeaderWhenOldDead = (1 << 0),
	eSCF_NewLeaderWhenOldLeave = (1 << 1),
};

enum EExecuteOrderFlag
{
	eEOF_ExecutedByKeyboard = (1 << 0),
	eEOF_ExecutedByMouse = (1 << 1),
	eEOF_ExecutedByPlayer = eEOF_ExecutedByKeyboard | eEOF_ExecutedByMouse,
	eEOF_ExecutedByAI = (1 << 2),
	eEOF_ExecutedByCommander = (1 << 3),
	eEOF_ExecutedByFG = (1 << 4),
	eEOF_PlayAIReadability = (1 << 5),
};

enum ESquadOrders
{
	//если ты меняешь здесь, то добавить и в CSquadSystem::CSquadSystem()

	eSO_Guard = 0, //attack enemy on sight disabled
	eSO_SearchEnemy = 1, //attack enemy on sight enabled
	eSO_FollowLeader = 2,
	eSO_SubPrimaryShootAt = 3,
	eSO_SubSecondaryShootAt,
	eSO_SubEnterVehicle,
	eSO_SubExitVehicle,
	eSO_SubPrimaryPickupItem,
	eSO_SubSecondaryPickupItem,
	eSO_SubUseVehicleTurret,//processedId is vehicle with gunner seat
	eSO_DebugEnableCombat,
	eSO_DebugDisableCombat,
	eSO_DebugStanceRelaxed,
	eSO_DebugStanceStanding,
	eSO_DebugStanceStealth,
	eSO_DebugStanceCrouch,
	eSO_ConqSearchCoverAroundArea,
	eSO_ConqGoTo,//attack enemy on sight enabled
	eSO_ConqBlankAction,//attack enemy on sight enabled
	eSO_SearchCoverAroundPoint,
	//eSO_SubAlienGrabPlayerSquad,
	//eSO_SubAlienDropPlayerSquad,
	//eSO_AlienSelfDestructHere,
	//eSO_AlienGrabThis,
	eSO_ScoutGrabMe,
	eSO_None,
	eSO_Last
};

//enum EGotoUpdateState
//{
//	eGUS_CleanAI = 0,//Set when the player execute the command "Goto"
//	eGUS_GoTo,//Set when AI is cleaned and select ord_goto pipe in update cycle
//	eGUS_Guard,//Set when AI complete goalpipe ord_goto and select ord_guard
//	eGUS_Combat,//Set when AI is on guard and see enemy
//	eGUS_ForSync
//};

enum class EOrderExecutingStep
{
	NotHaveOrder,
	GotAnOrder,
	GotoTarget,
	PerformingAction,
	Last,
};

enum EOrderIgnoreCombatFlag
{
	eOICF_IgnoreCombatWhenGotoTarget = BIT(0),
	eOICF_IgnoreCombatWhenPerfomingAction = BIT(1),

	eOICF_IgnoreEnemyAlways =
	eOICF_IgnoreCombatWhenGotoTarget |
	eOICF_IgnoreCombatWhenPerfomingAction
};

typedef EOrderIgnoreCombatFlag EOIEFlag;
typedef std::vector<EntityId> TSMembers;
typedef std::vector<CMember> TMembers;
typedef std::vector<CSquad> TSquads;
typedef _smart_ptr<CSquad> CSquadPtr;
typedef std::vector<CSquadPtr> TSquadsPtr;
typedef std::vector<ESquadOrders> TOrders;


//goalpipe ids of orders
//#define GOALPIPEID_ORDER_SEARCH 770
//#define GOALPIPEID_ORDER_COOLDOWN 771
//#define GOALPIPEID_ORDER_GOTO 772
//#define GOALPIPEID_ORDER_GOTO_GUARD 773
//#define GOALPIPEID_ORDER_FOLLOW 774
//#define GOALPIPEID_ORDER_FOLLOW_BACKOFF 412
//#define GOALPIPEID_ORDER_ENTER_VEHICLE 776
//#define GOALPIPEID_ORDER_EXIT_VEHICLE 777
//#define GOALPIPEID_ORDER_SHOOT_AT 778
//#define GOALPIPEID_ORDER_SEARCH_COVER 780
//
//#define GOALPIPEID_ORDER_CMDR_SEARCH_COVER 779
//#define GOALPIPEID_ORDER_CMDR_GOTO 781

#define APPLY_VEHICLE_EXECUTION(order)\
	switch (order.type)\
	{\
	case eSO_Guard:\
	case eSO_FollowLeader:\
		ignoreFlag |= eOICF_IgnoreCombatWhenGotoTarget;\
		break;\
	case eSO_SubPrimaryShootAt:\
	case eSO_SubSecondaryShootAt:\
	case eSO_SubEnterVehicle:\
	case eSO_SubExitVehicle:\
	case eSO_SubUseVehicleTurret:\
	ignoreFlag |= eOICF_IgnoreEnemyAlways;\
		break;\
	case eSO_SubPrimaryPickupItem:\
	case eSO_SubSecondaryPickupItem:\
	case eSO_SearchEnemy:\
	case eSO_DebugEnableCombat:\
	case eSO_DebugDisableCombat:\
	case eSO_DebugStanceRelaxed:\
	case eSO_DebugStanceStanding:\
	case eSO_DebugStanceStealth:\
	case eSO_DebugStanceCrouch:\
	case eSO_ConqSearchCoverAroundArea:\
	case eSO_ConqGoTo:\
	case eSO_SearchCoverAroundPoint:\
	case eSO_None:\
	case eSO_Last:\
	default:\
		ignoreFlag = 0;\
		break;\
	}\
\
	switch (order.type)\
	{\
	case eSO_ConqGoTo:\
	case eSO_Guard:\
		order.stepActions[step2] = "conqueror_goto_a2_d3_r3";\
		order.stepActions[step3] = "conqueror_goto_a0_d2_r3";\
		break;\
	case eSO_SearchEnemy:\
		order.stepActions[step2] = "conqueror_goto_a2_d3_r3";\
		order.stepActions[step3] = "squad_search_enemy";\
		break;\
	case eSO_FollowLeader:\
		order.targetId = pLeader->GetEntityId();\
		order.targetPos = pLeader->GetEntity()->GetWorldPos();\
		order.stepActions[step2] = "conqueror_goto_a2_d0_r3";\
		order.stepActions[step3] = "squad_follow_leader";\
		break;\
	case eSO_SubPrimaryShootAt:\
		order.stepActions[step1] = "squad_select_primary";\
		order.stepActions[step2] = "squad_select_primary";\
		order.stepActions[step3] = "squad_shoot_7s";\
		break;\
	case eSO_SubSecondaryShootAt:\
		order.stepActions[step1] = "squad_select_secondary";\
		order.stepActions[step2] = "squad_select_secondary";\
		order.stepActions[step3] = "squad_shoot_7s";\
		break;\
	case eSO_SubEnterVehicle:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_vehicle_enter_fast";\
		break;\
	case eSO_SubExitVehicle:\
		order.stepActions[step1] = "squad_vehicle_exit";\
		order.stepActions[step2] = "squad_vehicle_exit";\
		order.stepActions[step3] = "squad_vehicle_exit";\
		break;\
	case eSO_SubPrimaryPickupItem:\
	case eSO_SubSecondaryPickupItem:\
		break;\
	case eSO_SubUseVehicleTurret:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_vehicle_enter_gunner_fast";\
		break;\
	case eSO_DebugEnableCombat:\
	case eSO_DebugDisableCombat:\
	case eSO_DebugStanceRelaxed:\
	case eSO_DebugStanceStanding:\
	case eSO_DebugStanceStealth:\
	case eSO_DebugStanceCrouch:\
	case eSO_ConqSearchCoverAroundArea:\
	case eSO_SearchCoverAroundPoint:\
	case eSO_None:\
	default:\
		break;\
	}\

#define APPLY_HUMAN_EXECUTION(order)\
	switch (order.type)\
	{\
	case eSO_ConqGoTo:\
	case eSO_Guard:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "conqueror_goto_a0_d0_r3";\
		break;\
	case eSO_SearchEnemy:\
		order.stepActions[step2] = "conqueror_goto_a5_d2_r3";\
		order.stepActions[step3] = "squad_search_enemy";\
		break;\
	case eSO_FollowLeader:\
		order.targetId = pLeader->GetEntityId();\
		order.targetPos = pLeader->GetEntity()->GetWorldPos();\
		order.stepActions[step2] = "conqueror_goto_a0_d3_r3";\
		order.stepActions[step3] = "squad_blank_action";\
		break;\
	case eSO_SubPrimaryShootAt:\
		order.stepActions[step1] = "squad_select_primary";\
		order.stepActions[step2] = "squad_select_primary";\
		order.stepActions[step3] = "squad_shoot_5s";\
		break;\
	case eSO_SubSecondaryShootAt:\
		order.stepActions[step1] = "squad_select_secondary";\
		order.stepActions[step2] = "squad_select_secondary";\
		order.stepActions[step3] = "squad_shoot_5s";\
		break;\
	case eSO_SubEnterVehicle:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_vehicle_enter_fast";\
		break;\
	case eSO_SubExitVehicle:\
		order.stepActions[step1] = "squad_vehicle_exit";\
		order.stepActions[step2] = "squad_vehicle_exit";\
		order.stepActions[step3] = "squad_vehicle_exit";\
		break;\
	case eSO_SubPrimaryPickupItem:\
		order.stepActions[step1] = "squad_select_primary";\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_pickup_item";\
		break;\
	case eSO_SubSecondaryPickupItem:\
		order.stepActions[step1] = "squad_select_secondary";\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_pickup_item";\
		break;\
	case eSO_SubUseVehicleTurret:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_vehicle_enter_gunner_fast";\
		break;\
	case eSO_DebugEnableCombat:\
	case eSO_DebugDisableCombat:\
	case eSO_DebugStanceRelaxed:\
	case eSO_DebugStanceStanding:\
	case eSO_DebugStanceStealth:\
	case eSO_DebugStanceCrouch:\
		break;\
	case eSO_ConqSearchCoverAroundArea:\
	case eSO_SearchCoverAroundPoint:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "squad_hiding_in_cover";\
		order.targetRadius = 15.0f;\
		break;\
	case eSO_None:\
	default:\
		break;\
	}\

#define APPLY_ALIEN_EXECUTION(order)\
	switch (order.type)\
	{\
	case eSO_ConqGoTo:\
	case eSO_Guard:\
		order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
		order.stepActions[step3] = "conqueror_goto_a0_d0_r3";\
		break;\
	case eSO_SearchEnemy:\
		order.stepActions[step2] = "conqueror_goto_a5_d2_r3";\
		order.stepActions[step3] = "squad_search_enemy";\
		break;\
	case eSO_FollowLeader:\
		order.targetId = pLeader->GetEntityId();\
		order.targetPos = pLeader->GetEntity()->GetWorldPos();\
		if (memClass == "Scout")\
		{\
			order.stepActions[step2] = "conqueror_goto_a0_d5_r3";\
		}\
		else\
		{\
			order.stepActions[step2] = "conqueror_goto_a0_d3_r3";\
		}\
		order.stepActions[step3] = "squad_blank_action";\
		break;\
	case eSO_SubPrimaryShootAt:\
		order.stepActions[step1] = "squad_select_primary";\
		order.stepActions[step2] = "squad_select_primary";\
		order.stepActions[step3] = "squad_shoot_5s";\
		break;\
	case eSO_SubSecondaryShootAt:\
		order.stepActions[step1] = "squad_select_secondary";\
		order.stepActions[step2] = "squad_select_secondary";\
		if (memClass == "Scout")\
		{\
			order.stepActions[step3] = "squad_shoot_8s";\
		}\
		else\
		{\
			order.stepActions[step3] = "squad_shoot_5s";\
		}\
		break;\
	case eSO_SubEnterVehicle:\
	case eSO_SubExitVehicle:\
	case eSO_SubPrimaryPickupItem:\
	case eSO_SubSecondaryPickupItem:\
	case eSO_SubUseVehicleTurret:\
	case eSO_DebugEnableCombat:\
	case eSO_DebugDisableCombat:\
	case eSO_DebugStanceRelaxed:\
	case eSO_DebugStanceStanding:\
	case eSO_DebugStanceStealth:\
	case eSO_DebugStanceCrouch:\
		break;\
	case eSO_ConqSearchCoverAroundArea:\
	case eSO_SearchCoverAroundPoint:\
	{\
		if (memClass != "Scout" && memClass != "Hunter" && memClass != "Drone")\
		{\
			order.stepActions[step2] = "conqueror_goto_a0_d0_r3";\
			order.stepActions[step3] = "squad_hiding_in_cover";\
			order.targetRadius = 15.0f;\
		}\
	}\
	break;\
	case eSO_None:\
	default:\
		break;\
	}\

#define DEFINE_ACTION_VALUES \
	int maxAlertness = 120; \
	int goalPipeId = -1; \
	EAAEFlag actionFlag = eAAEF_None; \
	string cause \

#define DEFINE_STEPS \
	const auto step1 = EOrderExecutingStep::GotAnOrder; \
	const auto step2 = EOrderExecutingStep::GotoTarget; \
	const auto step3 = EOrderExecutingStep::PerformingAction \

#define DEFINE_SQUAD_FOOT_MAIN_ORDER_UPDATE(pMember) \
	if (!pMember || !GetLeader()) \
		return; \
	\
	SOrderInfo orderInfo;\
	pMember->GetOrderInfo(orderInfo, false);\
	\
	const auto pOrderObject = GET_ENTITY(orderInfo.targetId);\
	const auto objectPos = pOrderObject->GetWorldPos();\
	\
	CActor* pMemberActor = static_cast<CActor*>(GetActor(pMember->GetId()));\
	IAIObject* pMemberAI = pMemberActor->GetEntity()->GetAI();\
	auto pMemberPipe = pMemberAI->CastToIPipeUser();\
	const auto isInCombat = TOS_AI::IsInCombat(pMemberAI);\
	const string memberClass = pMemberActor->GetEntity()->GetClass()->GetName();\
	const string leaderClass = GetLeader()->GetEntity()->GetClass()->GetName();\
	\
	const auto leaderPos = GetLeader()->GetEntity()->GetWorldPos();\
	const auto memberPos = pMemberActor->GetEntity()->GetWorldPos();\
	const Vec3 refPos = pMember->GetActionRef()->GetWorldPos();\
	\
	const float actRefDist = (memberPos - refPos).GetLength();\
	const float actObjectDist = (memberPos - objectPos).GetLength(); \
	\
	/*For player-given order see ClientApplyExecution for more info*/ \
	/*For ai - given order see moment when ExecuteOrder is called*/ \
	const auto step = pMember->GetMainStep(); \
	const char* actionName = orderInfo.stepActions[step]; \
	const bool isExecuting = TOS_AI::IsExecuting(pMemberAI, actionName) \


#define DEFINE_SQUAD_VEHICLE_MAIN_ORDER_UPDATE(pMember) \
	if (!pMember || !GetLeader()) \
		return; \
	\
	SOrderInfo orderInfo;\
	pMember->GetOrderInfo(orderInfo, false);\
	\
	const auto pOrderObject = GET_ENTITY(orderInfo.targetId);\
	const auto objectPos = pOrderObject->GetWorldPos();\
	\
	auto pMemberActor = static_cast<CActor*>(GetActor(pMember->GetId()));\
	\
	auto pMemberVeh = TOS_Vehicle::GetVehicle(pMemberActor);\
	const auto isAir = TOS_Vehicle::IsAir(pMemberVeh);\
	const auto isTank = TOS_Vehicle::IsTank(pMemberVeh);\
	const auto isCar = TOS_Vehicle::IsCar(pMemberVeh);\
	const auto isSea = TOS_Vehicle::IsSea(pMemberVeh);\
	const auto isPLV = TOS_Vehicle::IsPLV(pMemberVeh);\
	\
	auto pMemberVehAI = pMemberVeh->GetEntity()->GetAI();\
	auto pMemberVehPipe = pMemberVehAI->CastToIPipeUser();\
	const auto isInCombat = TOS_AI::IsInCombat(pMemberVehAI);\
	\
	const auto leaderPos = GetLeader()->GetEntity()->GetWorldPos();\
	const auto vehPos = pMemberVeh->GetEntity()->GetWorldPos();\
	const Vec3 refPos = pMember->GetActionRef() ? pMember->GetActionRef()->GetWorldPos() : Vec3(0);\
	\
	const float vehRefDist = (vehPos - refPos).GetLength();\
	const float vehObjectDist = (vehPos - objectPos).GetLength(); \
	\
	/*For player-given order see ClientApplyExecution for more info*/ \
	/*For ai - given order see moment when ExecuteOrder is called*/ \
	const auto step = pMember->GetMainStep(); \
	const char* actionName = orderInfo.stepActions[step]; \
	const bool isExecuting = TOS_AI::IsExecuting(pMemberVehAI, actionName) \

#define AIR_REF_REDEFENITION(pMember, pOrderObject, safeFly, isAir)\
	auto pRef = pMember->GetActionRef();\
	if (!pRef)\
		pRef = GetActionRef(pMember, pOrderObject->GetWorldPos());\
	\
	if(safeFly && isAir)\
	{\
		auto pMemPos = GET_ENTITY(pMember->GetId())->GetWorldPos();\
		auto mat34 = pOrderObject->GetWorldTM(); \
		auto pos = mat34.GetTranslation(); \
		const auto safeFlyDist = (leaderClass == "Hunter") ? SAFE_FLY_DISTANCE_HUNTER : SAFE_FLY_HEIGHT; \
		\
		/*GET_LOCATION_FROM_GROUND_WITH_HEIGHT(pMemberActor, pos, safeFlyDist);*/ \
		GET_SAFEFLY_LOCATION_FROM_TARGET(pMemberActor, pMemPos, pos, safeFlyDist); \
		\
		mat34.SetTranslation(pos); \
		pRef->SetWorldTM(mat34);\
	}\
	else\
	{\
		auto mat34 = pOrderObject->GetWorldTM();\
		pRef->SetWorldTM(mat34);\
	}\

#define LAND_REF_REDEFENITION(pMember, pOrderObject, height)\
	auto pRef = pMember->GetActionRef();\
	if (!pRef)\
		pRef = GetActionRef(pMember, pOrderObject->GetWorldPos());\
	\
	auto mat34 = pOrderObject->GetWorldTM(); \
	auto pos = mat34.GetTranslation(); \
	\
	GET_LOCATION_FROM_GROUND(pMemberActor, pos, height); \
	\
	mat34.SetTranslation(pos); \
	pRef->SetWorldTM(mat34)\

	//auto pRef = pMember->GetActionRef();\
	//if (!pRef)\
	//pRef = GetActionRef(pMember, pOrderObject->GetWorldPos());\
	//\
	//auto mat34 = pOrderObject->GetWorldTM();\
	//auto pos = mat34.GetTranslation();\
	//pos.z += (leaderClass == "Hunter") ? SAFE_FLY_DISTANCE_HUNTER : SAFE_FLY_HEIGHT;\
	//mat34.SetTranslation(pos);\
	//pRef->SetWorldTM(mat34)\

#define DEFINE_MEMBER_ACTOR(memInstance)\
	auto pMemAct = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(memInstance.GetId()));\
	if (!pMemAct)\
		continue;\
	\
	auto pMemberAI = pMemAct->GetEntity()->GetAI();\
	if (!pMemberAI)\
		continue\

#define DEFINE_MEMBER_ACTOR_FROM_ID(memId, pSquad)\
	auto pMemInstance = pSquad->GetMemberInstance(memId);\
	if(!pMemInstance)\
		continue;\
	\
	auto pMemAct = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(memId));\
	if (!pMemAct)\
		continue;\
	\
	auto pMemberAI = pMemAct->GetEntity()->GetAI();\
	if (!pMemberAI)\
		continue\

constexpr auto MAX_HUD_SQUADS = 6;


struct ISquadEventListener
{
	virtual void OnOrderExecuted(CMember* pMember, const SOrderInfo& order) = 0;
	virtual void OnOrderExecuteFailed(CMember* pMember, const SOrderInfo& order) = 0;
	virtual void OnOrderPerfomingFailed(CMember* pMember, const SOrderInfo& order) = 0;
};

struct SMemberStats
{
	SMemberStats()
	{
		reset();
	}

	void reset()
	{
		//lastTimeLeaderVehEnter = 0;
		//lastTimeLeaderVehExit = 0;
		lastTimeConquerorSpawned = 0;
		//lastLeaderOperatedVehicleId = 0;
		//lastOperatedVehicleId = 0;
		//lastTimeVehicleEnter = 0;
		//lastTimeVehicleExit = 0;
		deathTimer = 0;
		aliveTimer = 0;
		lastTimeNewSubOrder = 0;
		lastTimeNewOrder = 0;
		lastTimeAllOrder = 0;
		lastTimeSetStep = 0;
		lastTimeSetSubStep = 0;
		lastTimeSubOrderFinished = 0;
	}

	//float lastTimeLeaderVehEnter;
	//float lastTimeLeaderVehExit;
	float lastTimeConquerorSpawned;
	//EntityId lastLeaderOperatedVehicleId;	
	//EntityId lastOperatedVehicleId;	
	//float lastTimeVehicleEnter;
	//float lastTimeVehicleExit;
	float deathTimer;
	float aliveTimer;
	float lastTimeNewSubOrder;
	float lastTimeNewOrder;
	float lastTimeAllOrder;
	float lastTimeSetStep;
	float lastTimeSetSubStep;
	float lastTimeSubOrderFinished;
};

struct SOrderInfo
{
	//If you change this also change CMember::GetOrderInfo and Member::SetOrderInfo
	SOrderInfo()
	{
		targetPos = Vec3(0);
		targetId = 0;
		targetRadius = 0.0f;
		ignoreFlag = 0;
		type = eSO_None;
		safeFly = false;

		for (int i = 0; i < (int)EOrderExecutingStep::Last; i++)
		{
			stepActions[EOrderExecutingStep(i)] = "nullActionName";
		}
	}

	std::map<EOrderExecutingStep, const char*> stepActions;
	Vec3 targetPos; //Used as position when creating member's action reference entity
	EntityId targetId;
	float targetRadius;//targetRadius for failed used as Timer
	uint ignoreFlag;//EOrderIgnoreEnemyFlag
	bool safeFly;
	ESquadOrders type;
};

struct SDetachedMemberData
{
public:
	friend class CMember;

	SDetachedMemberData()
	{
		reset();
	};

	void reset()
	{
		enableUpdate = false;
		routineType = eDRT_FirstType;
		pathName.clear();
		points.clear();
		targetId = 0;
		lastDetachedUpdateTime = 0;
	}

	bool enableUpdate;
	string pathName;
	std::vector<Vec3> points;
	EntityId targetId;
	EDetachedRoutineType routineType;
	float lastDetachedUpdateTime;
};

class CMember
{
public:
	friend class CSquad;
	friend struct SDetachedMemberData;

	CMember();
	CMember(IActor* _pAct);
	CMember(EntityId _id);
	virtual void Init();
	virtual void Reset();
	virtual void Serialize(TSerialize ser);
	virtual void GetMemoryStatistics(ICrySizer* s);

	virtual void SetFailedOrderInfo(const SOrderInfo& info);
	virtual void GetFailedOrderInfo(SOrderInfo& info);

	virtual void SetOrderInfo(const SOrderInfo& info, bool previous);
	virtual void GetOrderInfo(SOrderInfo& info, bool previous) const;

	virtual void SetSubOrderInfo(const SOrderInfo& info);
	virtual void GetSubOrderInfo(SOrderInfo& info) const;

	//virtual Vec3 GetOrderTargetLocation(bool previous) const;
	virtual SMemberStats* GetStats();
	virtual EOrderExecutingStep GetMainStep() const;
	virtual EOrderExecutingStep GetSubStep() const;
	virtual void SetStep(EOrderExecutingStep step, bool sub);
	virtual void SetActionRef(IEntity* pEntity);
	virtual IEntity* GetActionRef() const;

	virtual void ResetOrder(bool main, bool sub, bool previous);

	virtual EntityId GetId() const;
	virtual int GetIndex() const;

	virtual int GetGroupId() const;
	virtual void SetCurrentGroupId(int id);

	virtual bool CanBeUpdated() const;

	virtual void GetDetachedData(SDetachedMemberData &data) const;
	virtual void SetDetachedData(const SDetachedMemberData &data);

	bool operator == (const CMember* member2)
	{
		return this->m_entityId == member2->m_entityId;
	}

	bool operator == (CMember member2)
	{
		return this->m_entityId == member2.m_entityId;
	}

private:
	void OnSubOrderFinish();
	void EnableUpdating(bool enable);
	
	SOrderInfo m_previousOrderInfo;
	int m_previousGroupId;

	int m_groupId;
	int m_index;

	bool m_isUpdating;
	EntityId m_entityId;
	EntityId m_actionRefId;
	
	//SOrderInfo m_failedOrderInfo;
	SOrderInfo m_currentOrderInfo;
	SOrderInfo m_subOrderInfo;

	CSquadSystem* m_pSquadSystem;

	SDetachedMemberData m_detachedData;
	SMemberStats m_stats;
	EOrderExecutingStep m_mainStep;
	EOrderExecutingStep m_subStep;

	

	float m_lastUpdateTime;
};

//class CLeaderInstance
//{
//public:
//	CLeaderInstance();
//	CLeaderInstance(IActor* _pAct);
//
//	void Init();
//	void Reset();
//	void Serialize(TSerialize ser);
//	void GetMemoryStatistics(ICrySizer* s);
//
//	void SetOrderInfo(const SOrderInfo& info, bool previous);
//	void GetOrderInfo(SOrderInfo& info, bool previous) const;
//
//	void SetSubOrderInfo(const SOrderInfo& info);
//	void GetSubOrderInfo(SOrderInfo& info) const;
//
//	void GetDetachedData(SDetachedMemberData& data) const;
//	void SetDetachedData(const SDetachedMemberData& data);
//
//	EntityId GetId() const;
//
//	void SetActionRef(IEntity* pEntity);
//	IEntity* GetActionRef() const;
//	void SetStep(EOrderExecutingStep step, bool sub);
//
//private:
//	EOrderExecutingStep m_mainStep;
//	EOrderExecutingStep m_subStep;
//	SDetachedMemberData m_detachedData;
//	bool m_isUpdating;
//	EntityId m_entityId;
//	EntityId m_actionRefId;
//
//	SOrderInfo m_currentOrderInfo;
//	SOrderInfo m_subOrderInfo;
//
//
//};

class CSquad
{
public:
	friend class CConquerorCommander;
	friend class CSquadSystem;
	friend class CMember;

	CSquad();
	CSquad(IActor* _Leader, uint _squadId);
	~CSquad();

	float GetAverageDistanceToMembers(const Vec3& startPos);
	Vec3& GetAveragePos(bool includeLeader);

	bool AddMember(CMember& member);
	bool AddMember(const IActor* pActor);
	bool RemoveMember(CMember* member);

	//CLeaderInstance* GetLeaderInstance();
	CMember* GetMemberInstance(const EntityId id);
	CMember* GetMemberInstance(const IActor* pActor);
	CMember* GetMemberAlive();
	CMember* GetMemberFromIndex(const int index);
	void	 GetMembersOnFoot(std::vector<EntityId>& members);
	void	 GetMembersNotInVehicle(const IVehicle* pVehicle, std::vector<EntityId>& members);
	void	 GetMembersInVehicle(const IVehicle* pVehicle, std::vector<EntityId>& members);
	void	 GetMembersInCombat(std::vector<EntityId>& members);
	void	 GetMembersInRadius(std::vector<EntityId>& members, Vec3 pos, float radius);

	//Get the count of existing members in the squad but also leader not included
	int GetMembersCount() const { return m_members.size(); };
	int GetMembersCount(IVehicle* pVehicle);
	int GetAliveMembersCount();

	void SetAllMembers(TMembers members) { m_members = members; };
	auto GetAllMembers() { return m_members; };

	//The leader of squad can not be a member of this squad!
	//AI Leader cannot migrate from one squad to another
	void SetLeader(const IActor* pLeaderCandidate, bool isConquerorGamemode);

	void SetOldAILeader(IActor* pLeader) { m_pConquerorOldAILeader = pLeader; };
	auto GetOldAILeader() { return m_pConquerorOldAILeader; };
	IActor* GetLeader() const;
	IActor* RequestNewLeader(bool getFromMembers, bool getFromOldLeader);

	void ExecuteOrder(CMember* Member, const SOrderInfo& order, uint executeFlags);
	//void ExecuteOrderEx(ESquadOrders order, CMember* Member, uint executeFlags, EntityId processedId, Vec3& processedPos, const float radius);
	//void ExecuteOrderAllMembers(ESquadOrders order, uint executeFlags, EntityId processedId = 0, Vec3& processedPos = Vec3(0,0,0), const float radius = 0);
	//void ExecutePreviousOrder(CMember* Member);
	//bool ExecuteOrderFG(ESquadOrders order, CMember& Member, Vec3& refPoint = Vec3(0, 0, 0));

	//int GetOrder(const EntityId id);
	//int GetOrder(const IActor* act);
	//int	GetOrder(const int index);

	//auto GetLeaderOrder() const { return m_eLeaderCurrentOrder; };
	//void SetLeaderOrder(ESquadOrders order) { m_eLeaderCurrentOrder = order; };

	void GetLeaderDetachedData(SDetachedMemberData& data);

	bool IsMember(const CMember* Member) const;
	bool IsMember(const IActor* pActor) const;
	bool IsMember(const EntityId id) const;

	bool IsLeader(const IActor* pActor) const noexcept;
	bool IsLeaderDetached() const noexcept;
	bool IsLeaderInCombat() const;

	bool IsAllMembersSelected();

	bool IsMemberSelected(const IActor* pActor);
	bool IsMemberSelected(const int index);

	bool IsMemberDetached(const IActor* pActor);
	bool IsMemberDetached(const int index);
	bool IsMemberDetached(const EntityId id);
	bool IsMemberDetached(const CMember* Member);

	void MarkDetached(EntityId id, const SDetachedMemberData& data);
	void MarkUndetached(EntityId id);

	int	GetIndexFromMember(const CMember& Member);
	int	GetIndexFromMember(const IActor* pActor);
	int	GetIndexFromMember(const EntityId id);
	int	GetFreeMemberIndex() const;

	//Get the minimal distance between the leader and squad members
	float GetMinDistance() const;

	void RemoveMemberFromSelected(const IActor* pActor);
	void RemoveMemberFromSelected(const int index);
	void RemoveAllSelected();

	void AddMemberToSelected(const CMember* member);
	void AddMemberToSelected(const IActor* pActor);
	void AddMemberToSelected(const int index);

	bool HasClientMember() const;
	bool HasClientLeader() const;

	void OnPlayerAdded();
	void OnPlayerRemoved();

	void OnEnterVehicle(IActor* pActor);
	void OnExitVehicle(IActor* pActor);
	void OnStartCombat();
	void OnFinishCombat();

	void UpdateConquerorDetachedHuman(float frametime, IActor* pActor, const SDetachedMemberData& data);
	void UpdateConquerorDetachedVehicle(float frametime, IVehicle* pVehicle, const SDetachedMemberData& data);
	void UpdateConquerorDetachedAlien(float frametime, IActor* pActor, const SDetachedMemberData& data);
	void UpdateConquerorDetached(float frametime);
	void UpdateDebugOrder(float frametime, CMember* pMember);
	void UpdateOrdersNew(float frametime);
	void UpdateOrders(float frametime);
	void UpdateMembersHUD();
	void UpdateStats(float frametime);

	void HandleMouseFSCommand(const char* fscommand);

	auto* GetActor(EntityId id) { return dynamic_cast<CActor*> (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id)); };

	void Init();
	void Shutdown();

	bool VehicleIsUsedByMember(EntityId vehicleId);

	void Reset()
	{
		TMembers::iterator it = m_members.begin();
		const TMembers::iterator end = m_members.end();
		for (; it != end; it++)
		{
			it->Reset();
		}
		m_searchRadius = 20;
		m_squadId = -1;
		//m_leaderInstance = CLeaderInstance();
		m_leaderId = 0;
		m_members.clear();
		m_selectedMembers.clear();
		m_detachedMembers.clear();
	};

	void Hide(bool hide);

	void Serialize(TSerialize ser);

	inline void SetId(const int id) noexcept { m_squadId = id; };
	inline int GetId() const noexcept { return m_squadId; };

	inline void SetFlags(uint flags) noexcept { m_flags = flags; };
	inline uint GetFlags() noexcept { return m_flags; };
	inline void SetSpecies(ESpeciesType species) { m_conquerorSpecies = species; };
	inline ESpeciesType GetSpecies() { return m_conquerorSpecies; };

	void GetMemoryStatistics(ICrySizer* s);

	//const SMemberStats& GetLeaderStats();
	ESquadOrders GetLastAllOrder () const;

	bool IsInCombat();

	//Get or create member's action reference point at position
	IEntity* GetActionRef(CMember* pMember, Vec3 position);

	void AddListener(ISquadEventListener* pListener);
	void RemoveListener(ISquadEventListener* pListener);

	void SetConqIterationTime(float seconds);
	float GetConqIterationTime() const;

	virtual void AddRef() const { ++m_refs; };
	virtual uint GetRefCount() const { return m_refs; };
	virtual void Release() const
	{
		if (--m_refs <= 0)
			delete this;
	}

	bool operator == (CSquad* pSquad2)
	{
		return this->m_squadId == pSquad2->m_squadId;
	}

	bool operator == (CSquad pSquad2)
	{
		return this->m_squadId == pSquad2.m_squadId;
	}

	//std::map<EntityId, SMemberStats> m_leadersStats;

private:

	void UpdateVehicleSubOrder(float frametime, CMember* pMember);
	void UpdateFootSubOrder(float frametime, CMember* pMember);
	void UpdateHumanFootOrder(float frametime, CMember* pMember);
	void UpdateHumanVehicleOrder(float frametime, CMember* pMember);
	void UpdateAlienOrder(float frametime, CMember* pMember);

	//Events
	//void		OnOrderStop(ESquadOrders order, CMember* pMember);
	//void		OnOrderStart(ESquadOrders order, CMember* pMember);
	//void		OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId);

	void		OnActorDeath(IActor* pActor);
	void		OnActorGrabbed(IActor* pActor, EntityId grabbedId);
	void		OnActorDropped(IActor* pActor, EntityId grabbedId);
	//void		SetLastAllOrder(ESquadOrders order);

	//void ApplyAIAction(const CMember* pMember, IEntity* pObject, IVehicle* pMemberVehicle, ESquadOrders type, EOrderExecutingStep step, string cause);
	//~Events
protected:
	mutable uint	m_refs;
	int m_squadId;

	EntityId m_leaderId;
	//CLeaderInstance m_leaderInstance;

	TMembers m_members;
	TSMembers m_selectedMembers;
	TSMembers m_detachedMembers;
	//ESquadOrders m_eLeaderCurrentOrder;
	int m_searchRadius;
	CSquadSystem* m_pSquadSystem;
	uint m_flags;
	bool m_hided;
	std::map<EntityId, SDetachedMemberData> m_detachedLeadersData;

	//CConquerorCommander* m_pCommander;

	IActor* m_pConquerorOldAILeader;
	ESpeciesType m_conquerorSpecies;
	ESquadOrders m_lastAllOrder;

	float m_lastTimeStartCombat;
	float m_lastTimeFinishCombat;
	bool m_inCombat;

	float m_lastConqIterationTime;

	std::vector<ISquadEventListener*> m_listeners;
};

class CSquadSystem : public IControlSystemChild
{
	// enums and typedefs and nested classes
public:
	friend class CControlSystem;
	friend class CControlClient;
	friend class CSquad;
	friend class CMember;
	friend class CHUD;

	CSquadSystem();
	~CSquadSystem();

public:

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
	bool		OnInputEvent(const SInputEvent& event) override;
	void		Init() override;
	void		Update(float frametime) override;
	void		Serialize(TSerialize ser) override;
	void		GetMemoryStatistics(ICrySizer* s) override;
	const char* GetChildName() { return "CSquadSystem"; };
	//~IControlSystemChild

	void		InitHUD(bool active);
	void		AnySquadClientLeft();
	bool		AnySquadActorIsLeader(const IActor* pActor) const noexcept;
	bool		AnySquadActorIsMember(IActor* pActor) const noexcept;

	std::vector<int> GetSquadIdsFromSpecies(ESpeciesType species, bool ignoreBookedVehicle);//ignore detached squad leaders
	CSquad*		GetSquadFromMember(const IActor* pActor, bool includeLeader);
	CSquad*		GetSquadFromLeader(IActor* pLeader);
	CSquad*		GetSquadFromId(int squadId) const;
	CSquad*		GetClientSquad() const;

	int			GetSquadsCount() const;

	CSquad*		CreateSquad();//Return true when Squad with its id is created
	//bool		RemoveSquad(CSquad* pSquad);//Return true when Squad is removed
	bool		RemoveSquad(uint squadId);//Return true when Squad is removed

	int			GetFreeSquadIndex() const;

	int			RequestGroupId(int excludeId, IEntity* pEntity);

	void		SetCommandMode(bool value);

	//void		GetNearestHideSpots(const Vec3& fromPoint, const float radius, std::vector<EntityId>& spots);
	//Vec3		GetNearestHidespot(const Vec3& fromPoint);
	EntityId	GetNearestFreeHidespot(const Vec3& fromPoint, const float radius);

	string		GetString(ESquadOrders orderType) const;
	void ClientApplyExecution(CMember* pMember, SOrderInfo& order, EExecuteOrderFlag executionFlag, EntityId targetId = 0, Vec3 targetPos = Vec3(0), bool safeFly = false);
	IEntity*	CreateActionReference(CMember* pMember, Vec3 position);

	EntityId	BookFreeHideSpot(IActor* pActor, Vec3 pos, float radius, const char* cause);
	EntityId	GetBookedHideSpot(IActor* pActor);
	bool		IsBookedHideSpot(EntityId spotId);
	bool		UnbookHideSpot(IActor* pActor, const char* cause);
	bool		CanBookHideSpot(IActor* pActor);

private:
	//HUD
	void		UpdateHUD();
	void		UpdateSelectedHUD();//Update selected members hud
	void		UpdatePlayerOrderHUD();
	void		UpdateMouseOrdersHUD(bool updateMousePos = true);

	void		ShowVehicleIndicatorHUD(bool show, int slot, const char* seatType);
	void		ShowDeadSquadMemberHUD(int slot); //now we need show as dead only last member
	void		ShowSquadMemberHUD(const bool active, const int slot);
	void		ShowSquadControlHUD(int index, bool show);//show the '1' or '2' squad control hud
	void		ShowAllSquadControlsHUD(bool active);
	void		ShowAllSquadControlsRedHUD(bool active);

	void		SetMouseOrdersPosHUD(int x, int y);
	void		ShowMouseOrderHUD(int index, bool show);
	//~HUD

	//Input
	bool		OnInputCommandMode(EntityId entityId, EInputState activationMode, float value);
	bool		OnInputOrderFollow(EntityId entityId, EInputState activationMode, float value);//Z
	//bool		OnInputSwitchOrder(EntityId entityId, EInputState activationMode, float value);//T
	//bool		OnInputExecuteOrder(EntityId entityId, EInputState activationMode, float value);//R
	bool		OnInputSelectMember(int index, EntityId entityId, EInputState activationMode, float value);
	bool		OnInputSelectAll(EntityId entityId, EInputState activationMode, float value);
	//~Input

	void		AddAvailableOrder(ESquadOrders order);
	void		RemoveAvailableOrder(ESquadOrders order);
	void		ClearAvailableOrders();

	void		Reset();
	//void		InitOrderActions(bool reset);

	void		OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId);
	void		SpawnOrderParticle(ESquadOrders order, const Vec3& pos);
	//void		PlayOrderOffHandAnimation(COrder* order);
public:
	TSquadsPtr m_allSquads;
	bool m_isCommandMode;

protected:
	CGameFlashAnimation m_animSquadMembers;
	CGameFlashAnimation m_animSquadMouseOrders;
private:
	bool m_isDebugLog;
	bool m_showSquadControls;

	_smart_ptr<ISound> m_pSoundCommandBegin;
	_smart_ptr<ISound> m_pSoundCommandEnd;

	TOrders m_availableMouseOrders;
	EntityId m_storedMouseId;
	Vec3 m_storedMouseWorldPos;

	std::map<EntityId, EntityId> m_bookedHidespots;//hideId, bookerId
	std::map<ESquadOrders,string> m_ordersStringMap;
	std::vector<ESquadOrders> m_allowedDetachedOrders;
	std::vector<EntityId> m_actionReferences;
	//std::map<ESquadOrders, std::vector<const char*>> m_orderStepActions;

	//static TActionHandler<CSquadSystem> s_actionHandler;
};