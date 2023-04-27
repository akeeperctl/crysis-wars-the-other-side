#pragma once

#include "Actor.h"
#include "CommanderOrder.h"

enum ESpeciesType;
enum ESquadOrders;
enum EOrderCause;
enum EOrderExecuteState;
enum ECommanderOrders;
enum EAreaGameStatusFlag;
enum class EAreaFlag;
typedef EntityId LeaderId;
class CSquadSystem;
class CMember;
class CCommanderOrder;
struct SConquerorStrategy;

constexpr auto COMMANDER_ANY_ORD_GOTO_GOALPIPE_ID = 800;
constexpr auto COMMANDER_ORD_DEFEND_GOALPIPE_ID = 801;
constexpr auto COMMANDER_ORD_GOTO_SUB_HIDE_GOALPIPE_ID = 802;
constexpr auto COMMANDER_ORD_GOTO_SUB_HIDE_INCOVER_GOALPIPE_ID = 803;

enum EStrategyGoalTemplates
{
	eSGT_CapturedAreasCount = 0,
	eSGT_DefendingAreasCount,
	eSGT_Last
};

enum EAreaBusyFlags
{
	eABF_AreaIsSquadTarget = (1 << 0),
	eABF_AreaIsHaveEnemyGuards = (1 << 1),
	eABF_AreaIsNOTSquadTarget = (1 << 2),
	eABF_AreaIsNOTHaveEnemyGuards = (1 << 3),
	eABF_NoMatter = (1 << 4),
};

enum EVehicleGetFlags
{
	eVGF_NearestOrLinks = (1 << 0),
	eVGF_Air = (1 << 1),
	eVGF_Land = (1 << 2),
	eVGF_Amphibious = (1 << 3),
	eVGF_Sea = (1 << 4),
	eVGF_MustHaveGun = (1 << 5),
	eVGF_NoMatter = (1 << 6)
};

class CConquerorCommander
{
public:
	friend class CConquerorSystem;
	friend class CConquerorChannel;
	friend class CStrategicArea;
	friend class CSquad;

	CConquerorCommander();
	~CConquerorCommander();
	CConquerorCommander(ESpeciesType species);

	void		Init(ESpeciesType species);

	//update functions
	void		Update(float frameTime);
	void		UpdateNew(float frameTime);
	void		UpdateLeaderHumanFoot(float frameTime, CSquad* pSquad);
	void		UpdateLeaderHumanVehicle(float frameTime, CSquad* pSquad);
	void		UpdateLeaderAlien(float frameTime, CSquad* pSquad);
	//void		UpdateTactics(ECommanderTactics tactics);

	ESpeciesType GetSpecies() const;
	//ECommanderTactics GetCurrentTactics();
	//ECommanderTactics GetPreviousTactics();
	CCommanderOrder* GetCommanderOrder(EntityId id);
	CCommanderOrder* GetCommanderOrder(IActor* pLeader);

	//CCommanderOrder* RequestOrderFromTactics(CSquad* pSquad, ECommanderTactics tactics);

	void		GetMemoryStatistics(ICrySizer* s);
	void		Serialize(TSerialize ser);

	int GetEnemyCountAroundArea(CStrategicArea* pArea, bool vehicle);

	CStrategicArea* GetNearestArea(const Vec3& pos, ESpeciesType targetSpecies, bool mostPrioritry, int maxAssignedSquads, EAreaGameStatusFlag gameStatus);
	CStrategicArea* GetNearestArea(const Vec3& pos, ESpeciesType targetSpecies, EAreaBusyFlags busyFlags, EAreaFlag areaFlag);

	CStrategicArea* GetNearestArea(const Vec3& pos, const string& areaStatus, bool mostPrioritry, int maxAssignedSquads, EAreaGameStatusFlag gameStatus);
	CStrategicArea* GetNearestArea(const Vec3& pos, const string& areaStatus, EAreaBusyFlags busyFlags, EAreaFlag areaFlag);
	
	CStrategicArea* GetArea(EAreaGameStatusFlag gameFlag, const string& areaStatus, EAreaBusyFlags busyFlags, EAreaFlag areaFlag);

	const char* GetSpeciesName();

	CSquad* GetNearestSquad(const Vec3& fromPoint, const bool useVehicle);
	CSquad* GetNearestSquad(const ECommanderOrders commanderOrder, const Vec3& fromPoint, const bool useVehicle);
	CSquad* GetNearestSquad(const ECommanderOrders commanderOrder, const EOrderExecuteState orderState, const Vec3& fromPoint, const bool useVehicle);

	//CSquad* GetAreaGuadingSquad(CCapturableArea* pArea);

	std::vector<int>* GetAssignedSquadsForArea(CStrategicArea* pArea);
	bool IsAssignedSquad(CStrategicArea* pArea, CSquad* pSquad);
	CStrategicArea* GetAssignedArea(CSquad* pSquad);
	int GetAssignedSquadNumber(CStrategicArea* pArea, CSquad* pSquad);

	float GetDistanceToOrderTarget(CSquad* pSquad);
	float GetElapsedOrderChangesTime(CCommanderOrder* pOrder, bool getState); //Get State or Get Order type
	//bool IsSquadGuardingArea(CCapturableArea* pArea, CSquad* pSquad);

	SConquerorStrategy* GetCurrentStrategy() const;
	void SetCurrentStrategy(const SConquerorStrategy* pStrategy);

	bool RequestNewStrategy(bool gameStart);
	bool IsHaveClientSquad();

	std::vector<EAreaFlag> GetMostPriorityFlags(string areaStatus);
	std::vector<EAreaFlag> GetMostPriorityFlags(ESpeciesType targetSpecies);

	//float GetAreaFlagPriority(string areaStatus, string targetSpecies, EAreaFlag flag);
	void SetAreaFlagPriority(string areaStatus, string targetSpecies, EAreaFlag flag, float prioritry);
	bool ReadAreaFlagPriorities(const SConquerorStrategy* pStrategy);

	IVehicle* GetFreeVehicle(CStrategicArea* pArea, const IEntity* pRequester, float radius, uint flags, int minSeatCount = -1);
	IVehicle* GetBookedVehicle(int squadId);

	virtual void AddRef() const { ++m_refs; };
	virtual uint GetRefCount() const { return m_refs; };
	virtual void Release() const
	{
		if (--m_refs <= 0)
			delete this;
	}

	int GetCurrentStrategyGoals(EStrategyGoalTemplates goal) const;

	bool operator == (CConquerorCommander* pCommander)
	{
		if (!pCommander)
			return false;

		return this->GetSpecies() == pCommander->GetSpecies();
	}

protected:
	void		OnAreaCaptured(const CStrategicArea* pArea);
	void		OnAreaLost(const CStrategicArea* pArea);
	void		OnAreaUnderAttack(const CStrategicArea* pArea);

	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle);
	void		OnExitVehicle(IActor* pActor);
	void		OnVehicleDestroyed(IVehicle* pVehicle);
	void		OnVehicleStuck(IVehicle* pVehicle, bool stuck);

	void		OnActorDeath(IActor* pActor);

	void		OnSquadCallBackup(CSquad* pSquad);
	void		OnNewStrategicTarget(CSquad* pSquad, CStrategicArea* pArea);
	void		OnSquadLeaderSpawned(CSquad* pSquad);
	void		OnSquadMemberSpawned(CSquad* pSquad, CMember* pMember);

	void		OnSquadStartCombat(CSquad* pSquad);
	void		OnSquadFinishCombat(CSquad* pSquad);

	void		OnAIJoinGame();

	string		GetString(EOrderExecuteState state);
	string		GetString(ECommanderOrders types);
	string		GetString(EAreaFlag flag);
	string		GetString(ESquadOrders orders);

	mutable uint	m_refs;
private:
	void		OnStartLoosingAdvantage();
	void		OnStopLoosingAdvantage();

	void		OnParatroopersUnloaded(IVehicle* pVehicle);
	void		OnParatrooperEnter(IVehicle* pVehicle, IActor* pParatrooperActor);
	void		OnParatrooperRespawned(IVehicle* pVehicle, IActor* pParatrooperActor);
	bool		IsHaveParatroopers(const CSquad* pSquad);

	void		GetParatroopers(const CSquad* pSquad, std::vector<EntityId>& paratroopers);
	void		GetParatroopers(const CSquad* pSquad, const IVehicle* pVehicle, std::vector<EntityId>& paratroopers);

	int			GetParatroopsCount(const CSquad* pSquad, const IVehicle* pVehicle);
	int			GetParatroopsCount(const CSquad* pSquad);

	bool		IsParatrooper(EntityId memberId);
	void		MarkParatrooper(const CSquad* pSquad, EntityId memberId);
	void		UnmarkParatrooper(EntityId memberId);

	bool		IsBookedVehicle(IVehicle* pVehicle);
	IVehicle* BookFreeVehicle(CStrategicArea* pArea, int squadId, float radius, uint flags, int minSeatCount /*= -1*/);
	bool		UnbookVehicle(const IVehicle* pVehicle);
	bool		UnbookVehicle(int squadId);

	//ECommanderOrders GetNextOrderType(const SCommanderOrder* pOrder);
	//int			GetProperGoalPipeId(const CCommanderOrder* pOrder);
	void		SetSpecies(ESpeciesType species);
	//void		SetTactics(ECommanderTactics tactics);
	void		SetLeaderOrder(EntityId leaderId, CCommanderOrder&);

	CConquerorSystem* m_pConqueror;
	CSquadSystem* m_pSquadSystem;

	//species
	ESpeciesType m_species;

	//global tactics
	//ECommanderTactics m_currentTactics;
	//ECommanderTactics m_prevTactics;
	
	//leader's orders
	std::map<LeaderId, CCommanderOrder> m_commanderOrdersMap;

	//species squads
	std::vector<int> m_subordinateSquadIds;

	std::vector<const char*> m_strategiesNamesHistory;

	std::map<string, std::map<EAreaFlag, float>> m_areaFlagPriorities;

	std::map<CStrategicArea*, std::vector<int>> m_targettedAreasMap;//Capturable Area, Squad Id

	std::map<int, EntityId> m_squadBookedVehicle;
	std::map<int, std::vector<EntityId>> m_squadParatroopers;

	//std::map<int, std::vector<EntityId>> m_squadsVehicles;//Squad Id, Vehicle Ids

	//std::vector<int> m_disableSquadUpdate;

	std::map<EStrategyGoalTemplates, int> m_currentStrategyGoals;

	string m_currentStrategyName;
	string m_lastStrategyName;

	bool m_loosingAdvantage;
	float m_lastTimeLoosingAdvantage;

	//SConquerorStrategy* m_pCurrentStrategy;
	//SConquerorStrategy* m_pLastStrategy;

	float m_currentStrategyTimeLimit;
	float m_lastTimeStrategyChange;

};