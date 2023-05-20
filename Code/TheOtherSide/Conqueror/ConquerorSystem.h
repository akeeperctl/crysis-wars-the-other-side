#pragma once

#include "HUD/GameFlashAnimation.h"
#include "RequestsAndResponses/RARSystem.h"
#include "../Control/IControlSystemChild.h"

class CAreaVehicleSpawnPoint;
class CConquerorCommander;
class CStrategicArea;
class CConquerorChannel;
class CSpeciesClass;
class CSquad;
class CSpawnPoint;
class CConquerorShop;
struct SClassAI;
struct SClassEquipment;
struct SClassModel;
struct SConquerorStrategy;
struct SGenericCondition;
struct SStrategyPriority;
struct ILevelInfo;
struct ILevel;
struct IPipeUser;
enum class EAreaFlag;
enum ESpeciesType;
enum EAreaBusyFlags;
enum EAreaGameStatusFlag;
enum ESquadOrders;
enum ERespawnEvent;
enum EGoalPipeEvent;

typedef std::vector<_smart_ptr<SConquerorStrategy>> TStrategies;
typedef std::vector<CStrategicArea*> TAreas;
typedef std::vector<CConquerorChannel*> TCChannels;

typedef std::vector<CSquad> TSquads;
typedef std::vector<CSpeciesClass> TClasses;
typedef std::vector<_smart_ptr<CSpeciesClass>> TClassesPtr;

typedef std::map<ESpeciesType, TSquads> TSpeciesSquads;
typedef std::map<ESpeciesType, TClassesPtr> TSpeciesClasses;
typedef std::map<ESpeciesType, _smart_ptr<CSpeciesClass>> TSpeciesDefaultClasses;

constexpr auto HOSTILE = "Hostile";
constexpr auto NEUTRAL = "Neutral";
constexpr auto OWNED = "Owned";
constexpr auto ANY = "Any";

enum EVehicleClassTypes
{
	eVCT_Default = -1,
	eVCT_Air = 0,
	eVCT_Cars,
	eVCT_PLV,
	eVCT_Tanks
};

enum EGameStatus
{
	eGS_GameStart,
	eGS_Battle,
	eGS_GameOver
};

enum class EConquerLobbyState
{
	PRE_GAME,//selected in case if non-conquer gamemode
	IN_LOBBY,//selected from flow graph node
	IN_GAME, //selected in case if the player join game from the lobby
	LAST
};

//struct SConquerEntityRespawnData
//{
//	SmartScriptTable properties;
//	Vec3 position;
//	Quat rotation;
//	Vec3 scale;
//	int	flags;
//	IEntityClass* pClass;
//	primitives::box		obb;
//};
//
//struct SConquerEntityRespawn
//{
//	bool	unique;
//	bool	spatialcheck;
//	float	timer;
//	float	sctimer;
//};

//typedef std::map<EntityId, SConquerEntityRespawn> TConqEntityRespawnMap;
//typedef std::map<EntityId, SConquerEntityRespawnData> TConqEntityRespawnDataMap;

struct SVehicleClass
{
	SVehicleClass()
	{
		name = "";
		type = eVCT_Default;
	}

	string name;
	EVehicleClassTypes type;
};

class CConquerAICountInfo
{
public:
	friend class CConquerorSystem;

	CConquerAICountInfo()
	{

	}
	
	void Clear()
	{
		speciesSquadsCountMap.clear();
		speciesUnitsCountMap.clear();
		speciesReinforcementCountMap.clear();
	}

	int GetSquadsCount(const ESpeciesType species) const
	{ 
		// auto it = speciesSquadsCountMap.cbegin();
		// auto end = speciesSquadsCountMap.cend();
		//
		// for (; it != end; ++it)
		// {
		// 	if (it->first == species)
		// 		return it->second;
		// }
		const auto it = speciesSquadsCountMap.find(species);
		return it == speciesSquadsCountMap.end() ? 0 : it->second;
	}

	int GetUnitsCount(const ESpeciesType species) const 
	{ 
		// auto it = speciesUnitsCountMap.cbegin();
		// const auto end = speciesUnitsCountMap.cend();
		//
		// for (; it != end; ++it)
		// {
		// 	if (it->first == species)
		// 		return it->second;
		// }

		const auto it = speciesUnitsCountMap.find(species);
		return it == speciesUnitsCountMap.end() ? 0 : it->second;
	}

	int GetReinforcementsCount(const ESpeciesType species) const
	{
		const auto it = speciesReinforcementCountMap.find(species);
		return it != speciesReinforcementCountMap.end() ? it->second : 0;
	}

	int GetMaxReinforcementsCount(const ESpeciesType species) const
	{
		// auto it = speciesReinforcementCountConstMap.cbegin();
		// auto end = speciesReinforcementCountConstMap.cend();
		//
		// for (; it != end; it++)
		// {
		// 	if (it->first == species)
		// 		return it->second;
		// }

		//return 0;

		const auto it = speciesReinforcementCountConstMap.find(species);
		return it != speciesReinforcementCountConstMap.end() ? it->second : 0;
	}


private:
	void SetReinforcementsCount(const ESpeciesType species, const int count)
	{
		speciesReinforcementCountMap[species] = count;
	}

	std::map<ESpeciesType, int> speciesUnitsCountMap;
	std::map<ESpeciesType, int> speciesSquadsCountMap;
	std::map<ESpeciesType, int> speciesReinforcementCountMap;
	std::map<ESpeciesType, int> speciesReinforcementCountConstMap;
};

struct SConquerLobbyInfo
{
public:
	friend class CConquerorSystem;

	SConquerLobbyInfo();

	EConquerLobbyState state;
	EntityId modelEntityId;
	bool isConquest;

	Vec3 modelPos;
	Vec3 modelScale;
};

struct SPreRespawnData
{
	SPreRespawnData()
	{
		actorId = 0;
		areaId = 0;
		event = static_cast<ERespawnEvent>(0);
	}

	SPreRespawnData(const EntityId _actorid, const EntityId _areaId, const ERespawnEvent _event)
	{
		actorId = _actorid;
		areaId = _areaId;
		event = _event;
	}

	bool operator < (const SPreRespawnData& data) const { return data.actorId < this->actorId; };

	EntityId actorId;
	EntityId areaId;
	ERespawnEvent event;
};

class CConquerorSystem : public IControlSystemChild
{
public:
	friend class CConquerorCommander;
	friend class CConquerorChannel;
	friend class CStrategicArea;
	friend class CHUDRadar;
	friend class CHUD;

	CConquerorSystem();
	~CConquerorSystem();

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
	const char* GetChildName() override { return "CConquerorSystem"; }
	//~IControlSystemChild

	//Events
	void			OnActorRespawned(IActor* pActor, ERespawnEvent event);
	void			OnActorAddedInQueue(IActor* pActor, CStrategicArea* pArea);
	void			OnActorRemovedFromQueue(IActor* pActor);

	void			OnAreaCaptured(CStrategicArea* pArea, ESpeciesType ownerSpecies);
	void			OnAreaLost(CStrategicArea* pArea, ESpeciesType oldSpecies);
	//void			OnAreaUnderAttack(const CStrategicArea* pArea);

	void			OnCmdJoinGame(IActor* pDude);
	void			OnCmdSpectator(IActor* pActor);
	void			OnVehicleDestroyed(IVehicle* pVehicle) const;
	void			OnVehicleStuck(IVehicle* pVehicle, bool stuck) const;

	void			OnGoalPipeEvent(IEntity* pUserEntity, IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId);

private:
	void			OnChangeConfirmedLobbySpecies();
	void			OnSpeciesDestroyed(ESpeciesType species);

	void			OnClientAreaEnter(CStrategicArea* area, bool vehicle);
	void			OnClientAreaExit();
	void			OnLobbySetInfo(SConquerLobbyInfo& info);
	void			OnLobbySetTeam(ESpeciesType speciesIndex, int classIndex);
	//~Events

public:
	
	void			AddVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner);
	CAreaVehicleSpawnPoint* GetVehicleSpawner(EntityId id) const;
	void			RemoveVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner);
	bool			IsExistVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner) const;

	//Capturable Areas
	void			AddStrategicArea(CStrategicArea* area);
	void			RemoveStrategicArea(CStrategicArea* area);
	bool			IsExistStrategicArea(CStrategicArea* area);

	void			GetStrategicAreas(std::vector<CStrategicArea*>& areas, ESpeciesType targetSpecies, EAreaGameStatusFlag gameStatus, ESpeciesType owner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag) const;
	
	CStrategicArea* GetNearestStrategicArea(const Vec3& pos, const string& areaStatus, EAreaGameStatusFlag gameStatus, ESpeciesType desiredOwner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag) const;
	CStrategicArea* GetNearestStrategicArea(const Vec3& pos, ESpeciesType targetSpecies, EAreaGameStatusFlag gameStatus, ESpeciesType desiredOwner, EAreaBusyFlags busyFlags, EAreaFlag areaFlag) const;
	
	CStrategicArea* GetStrategicArea(EntityId id, std::vector<EntityId>* excludeAreaIds, bool getOnlyEnabled = true) const;
	CStrategicArea* GetStrategicArea(ESpeciesType species, std::vector<EntityId>* excludeAreaIds, bool getOnlyEnabled = true) const;
	CStrategicArea* GetBaseStrategicArea(ESpeciesType species) const;
	
	int				GetHostileAreasCount(ESpeciesType myspecies, const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const;
	int				GetHostileAreasCount(ESpeciesType myspecies, EAreaGameStatusFlag gameStatus) const;

	int				GetStrategicAreaCount(ESpeciesType species, const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const;
	int				GetStrategicAreaCount(ESpeciesType species, EAreaGameStatusFlag gameStatus) const;

	int				GetStrategicAreaCount(const std::vector<EAreaFlag>& flags, EAreaGameStatusFlag gameStatus) const;
	int				GetStrategicAreaCount(EAreaGameStatusFlag gameStatus) const;
	//~Capturable Areas

	//Channels
	CConquerorChannel* CreateConquerorChannel(IEntity* pEntity, CSpeciesClass& classInfo);
	void			RemoveConquerorChannel(const IEntity* pEntity);
	void			RemoveAllChannels();
	bool			IsExistConquerorChannel(IEntity* pEntity);
	CConquerorChannel* GetClientConquerorChannel();
	CConquerorChannel* GetConquerorChannel(const IEntity* pEntity) const;
	CConquerorChannel* GetConquerorChannel(EntityId entityId) const;
	CConquerorChannel* GetConquerorChannel(int idx) const;
	CConquerorChannel* GetConquerorChannelById(int id) const;
	int				GetConquerorChannelsCount();
	//~Channels

	int				GetSpeciesFromEntity(IEntity* pEntity);
	const char*		GetSpeciesName(ESpeciesType type) const;
	void			GetSpeciesTeammates(ESpeciesType species, std::vector<EntityId>& teammates, bool onlyAlive = false) const;
	void			SetSpeciesReinforcements(ESpeciesType species, int count);
	void			AddSpeciesReinforcements(ESpeciesType species, int count, int multiplyer);
	int				GetSpeciesReinforcements(ESpeciesType species) const;//Get the species reinforcements info from the .xml file
	int				GetMaxSpeciesReinforcements(ESpeciesType species) const;//Get the species reinforcements info from the .xml file
	int				GetActorPoints(IEntity* pEntity);

	void			AddPointsToActor(IEntity* pEntity, int value);
	void			InitHUD(bool show);
	void			InitPowerStruggleHUD();
	void			InitGamemodeFromFG(SConquerLobbyInfo& info);
	void			ShowRespawnCycle(bool show);
	void			SetRespawnCycleRemainingTime(int respawnTime, float remainingSeconds);
	void			SetPlayerModel(IActor* pPlayer, const SClassModel& info);
	void			SetPlayerMaterial(IActor* pPlayer, const SClassModel& info);

	float			GetRespawnTime() const;

	std::vector<EntityId> GetSpeciesCapturableAreas(ESpeciesType species);
	std::vector<int> GetAllowableStrategies(CConquerorCommander* pDesiredOwner, bool includeCurrentStrategy) const;

	bool			IsGamemode();
	bool			ReadStrategyCondition(const CConquerorCommander* pDesiredOwner, const SGenericCondition& condition, bool goal) const;
	bool			ReadStrategyGoals(const CConquerorCommander* pStrategyOwner) const;
	bool			ReadClassConditions(ESpeciesType species, const CSpeciesClass* pClass, bool AI);//For ai or the player?
	//bool			ApplyStrategy(CConquerorCommander* pDesiredOwner, const SConquerorStrategy& strategy);

	ESpeciesType	GetClientSpecies();
	ESpeciesType	GetSpeciesTypeFromString(const char* speciesName) const;

	CConquerorCommander* GetSpeciesCommander(ESpeciesType species) const;
	SConquerorStrategy* GetStrategy(uint index) const;
	SConquerorStrategy* GetStrategy(const char* name) const;
	CSpeciesClass*		GetClass(ESpeciesType species, const char* name) const;
	EVehicleClassTypes GetVehicleClassType(const IVehicle* pVehicle);
	IEntity*		GetNearestLandingSpot(IEntity* pAirEntity);
	IEntity*		GetNearestLandingSpot(IEntity* pAirEntity, IEntity* pAreaEntity, float radius);
	IEntity*		GetClientAreaEntity();
	CSpeciesClass* GetClientSelectedClass();
	CSpeciesClass* GetDefaultClass();
	SConquerLobbyInfo& GetLobbyInfo();

	void UnbookUnloadSpot(IEntity* pEntity);
	void			SetCanBookSpot(IVehicle* pVehicle, bool can, const char* cause);
	bool			CanBookUnloadSpot(IEntity* pEntity);
	bool IsHaveUnloadSpot(IEntity* pEntity);
	bool			RegisterToRespawn(IActor* pActor, CStrategicArea* pArea, ERespawnEvent event);
	bool			IsBookedVehicle(IVehicle* pVehicle) const;

	CRequestsAndResponses* GetRAR();
	CConquerorShop* GetShop();
	CStrategicArea* GetClientArea();
	CSpawnPoint* GetPerfectFreeSpawnPoint(IActor* pActor);

	int				GetCommandersCount() const;
	int				GetSpeciesChangeLimit() const;
	int				GetSpeciesChangeCount() const;

	bool		IsGameOver() const { return m_gameOver; };
	ESpeciesType	GetWinnerSpecies() const { return m_winnerSpecies; };

	EGameStatus	GetGameStatus() const { return m_gameStatus; }

	void			ForceKillSpecies(ESpeciesType species);

private:

	void			SetSpeciesChangeLimit(int value);
	int				GetSpeciesFlagIndex(ESpeciesType species);
	ESpeciesType	GetFlagIndexSpecies(int index);
	ESpeciesType	GetSpeciesLobby();
	bool			IsSpeciesAllowed(ESpeciesType species, bool inLobby);
	bool			CanJoinGame();
	bool			CanSwitchSpecies(IEntity* pEntity);
	bool			IsInQueue(IActor* pActor);
	bool			RemoveFromQueue(const IActor* pActor);
	bool			CopyQueue(CStrategicArea* pAreaSource, CStrategicArea* pAreaDestination);//Copy queue from source to destination

	void			GameOver(bool isOver);
	void			SetLobbyInfo(SConquerLobbyInfo& state);
	void			SetFriendAreaPointsCount(int count);
	void			HUDLobbyManageClass(ESpeciesType species, const char* className, bool remove);
	void			HUDLobbyEnableSpecies(ESpeciesType species, bool enable);
	void			HUDLobbyUpdateSpeciesMenu(bool lobbySpecies, bool saveSelection);
	void			HUDLobbyMenuShow(bool show);
	void			UpdateHUDSOM(bool reset);
	void			HUDSOMReset();
	void			HUDSOMSetPointsCount(bool friendPoints, int count);
	void			HUDSOMSetFlagSpecies(int flagIndex, ESpeciesType species);
	void			HUDSOMSetFlagUnitsCount(int flagIndex, int count);
	void			HUDSOMSetFlagLoseAdvantage(int flagIndex, bool lose);
	void			HUDSOMSetFlagEnabled(int flagIndex, bool enabled);
	void			HUDSOMSetFlagRelationship(int flagIndex, const char* relationship); // relationship may be friend or enemy or neutral
	void			GetSpeciesClasses(ESpeciesType species, TClassesPtr& classes);
	void			GetSpeciesRandomClasses(ESpeciesType species, uint flags, int classCount, TClassesPtr& classes);
	void			HandleFSCommand(const char* command, const char* arguments);
	void			HandleWarningAnswer(const char* warning);
	void			SetClientAreaEntity(IEntity* pEntity);
	void			InitAllowedSpecies();

	//bool			CreateSquadFromSpecies(ESpeciesType species, CSquad& squad);
	CSquad*			CreateSquadFromSpecies(ESpeciesType species);
	bool			CreateAIEntity(CSpeciesClass* classInfo, string entityName, ESpeciesType species);
	void			CreateAICommander(ESpeciesType species);

	IEntity*		EmergencyCreateAIEntity(CSpeciesClass* classInfo, const char* name);

	//Technically pLevel is null when run this function in editor environment
	void			InitConquerorAICountInfo(bool forceReload, const char* levelName = "");
	void			InitConquerorStrategies(bool forceReload, const char* levelName = "");
	
	void			InitPlayerClasses(bool forceReload);
	void			InitVehicleClasses(bool forceReload);
	void			AddConquerorClass(ESpeciesType species, CSpeciesClass* pConquerorClass, bool isDefaultClass = false);
	void			RemoveAllClasses();

	int				GetRandomSquadId(ESpeciesType species);

	void			SetClientArea(CStrategicArea* area);

	//void			SpawnAIBotsOld();
	void			GameStartSpawnAI();
	void			ResetAllAI();

	IEntity* m_pSelectedClientAreaEntity;
	CStrategicArea* m_pClientArea;
	CGameFlashAnimation m_animConquerorProgress;
	CGameFlashAnimation m_animConquerorLobby;
	CGameFlashAnimation m_animRespawmTimer;
	CGameFlashAnimation m_animSwingOMeter;
	CGameFlashAnimation m_animBuyZoneIndicator;

	CGameFlashAnimation* m_pAnimBuyMenu;
	CGameFlashAnimation* m_pAnimScoreBoard;
	CGameFlashAnimation* m_pAnimPlayerPP;
	CGameFlashAnimation* m_pAnimPlayerPPChange;
	CGameFlashAnimation* m_pAnimMPMessages;
	CGameFlashAnimation* m_pAnimBattleLog;
	CGameFlashAnimation* m_pAnimKillLog;
	SConquerLobbyInfo m_LobbyInfo;

	std::vector<CAreaVehicleSpawnPoint*> m_vehicleSpawners;
	TAreas m_strategicAreas;
	TCChannels m_conquerorChannels;
	//TSpeciesSquads m_speciesSquadsMap;

	TSpeciesClasses m_speciesClassesMap;
	TSpeciesDefaultClasses m_speciesDefaultClassesMap;

	CConquerAICountInfo* m_pXMLAICountInfo;
	EntityId m_usedAlienId;
	std::vector<ESpeciesType> m_gameAllowedSpecies;
	std::vector<ESpeciesType> m_lobbyAllowedSpecies;

	std::map<ESpeciesType, _smart_ptr<CConquerorCommander>> m_speciesCommanders;
	//std::vector<_smart_ptr<CConquerorCommander>> m_speciesCommanders;
	std::map<ESpeciesType, string> m_speciesCharNameMap;
	std::map<ESpeciesType, int> m_speciesLeadersCountMap;
	std::map<ESpeciesType, int> m_speciesFlagIndexMap;
	std::map<ESpeciesType, float> m_speciesAutoDestroyTime;
	//std::map<EntityId, int> m_areaSpawnedLeadersCountMap;

	std::map<EntityId, bool> m_vehiclesCanBookUnloadSpots;
	std::vector<SVehicleClass> m_vehicleClasses;
	std::vector< _smart_ptr<SConquerorStrategy>> m_strategies;
	std::map<SPreRespawnData, float> m_preRespawns;

	CConquerorShop* m_pShop;
	CRequestsAndResponses* m_pRARSystem;

	EGameStatus m_gameStatus;

	bool m_isDebugLog;

	float m_botsSpawnedTime;
	bool m_haveBotsSpawned;
	bool m_gameOver;
	ESpeciesType m_winnerSpecies;
	//int m_botsMaxCount;

	int m_oldLobbySpeciesIndex;
	int m_oldLobbyClassIndex;

	int m_lobbyConfirmedSpeciesIndex;
	int m_lobbyConfirmedClassIndex;

	int m_lobbySelectedSpeciesIndex;
	int m_lobbySelectedClassIndex;

	int m_lastClientPointsSet;
	int m_friendPointsCount;
	int m_enemyPointsCount;

	int m_speciesSwitchCount;

	//timers
	float m_lastTimeAIReset;

	//Cvars
	//int m_CVarBotsPerSquadCount;
	//int m_CVarBotsPerSpeciesCount;
	bool m_CVarBotsJoinBeforePlayer;
	float m_CVarRespawnTime;
	float m_CVarTimeLimit;
	int m_CVarSpeciesChangeLimit;
	//~Cvars
};