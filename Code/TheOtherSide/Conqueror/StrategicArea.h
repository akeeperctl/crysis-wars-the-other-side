#include <IGameObject.h>
#include "SpawnPoint.h"

#ifndef __CAPTURABLEAREA_H__
#define __CAPTURABLEAREA_H__
#pragma once

class CSpawnPoint;
class CAreaVehicleSpawnPoint;
enum ERespawnEvent;

enum class EAreaFlag
{
	FirstType,
	Centre = 0,
	AirSpawner,
	LandSpawner,
	SeaSpawner,
	SoldierSpawner,
	//eAF_StrongPoint,
	//eAF_ChokePoint,
	Bridge,
	Base,
	AirField,
	SupplyPoint,
	//eAF_Route,
	ControlPoint,
	North,
	West,
	South,
	East,
	Safe,
	//eAF_Enemy,
	Neutral,
	Front,

	LastType
};

enum ESpawnpointGameStatusFlag
{
	eSGSF_NotHaveRecentlySpawned = 0,
	eSGSF_NoMatter,
};

enum EAreaGameStatusFlag
{
	eAGSF_Enabled = 0,
	eAGSF_Capturable,
	eAGSF_EnabledAndCapturable,
	eAGSF_NoMatter,
};

enum class ECaptureState
{
	NOTCAPTURED = 0x00,
	CAPTURED = 0x01,
	CONTESTED = 0x02,
	CAPTURING = 0x04,
	UNCAPTURING = 0x08
};

enum ESpeciesType
{
	eST_NEUTRAL = -1,

	eST_FirstPlayableSpecies = 0,
	eST_USA = 0,
	eST_NK = 1,
	eST_Aliens = 2,
	eST_CELL = 3,
	eST_SPECIES4 = 4,
	eST_SPECIES5 = 5,
	eST_SPECIES6 = 6,
	eST_SPECIES7 = 7,
	eST_SPECIES8 = 8,
	eST_SPECIES9 = 9,
	eST_SPECIES10 = 10,
	eST_SPECIES11 = 11,
	eST_SPECIES12 = 12,
	eST_LastPlayableSpecies = eST_SPECIES12,

	eST_LAST = 13,
};

enum EEntityType
{
	UNKNOWN = 0x00,
	VALID = 0x01,
	AI = 0x02,
	ACTOR = 0x04,
	VEHICLE = 0x08,
	ITEM = 0x10,
};

enum class EMultiplayerSide
{
	SPECIES,
	TEAM
};

enum EBuyZoneOption
{
	eBZO_Vehicles = BIT(0),
	eBZO_Weapons = BIT(1),
	eBZO_Equipment = BIT(2),
	eBZO_Prototypes = BIT(3),
	eBZO_Ammo = BIT(4)
};

typedef std::vector<EAreaFlag> TAreaFlags;

//struct SUnloadSpotInfo
//{
//	SUnloadSpotInfo()
//	{
//		entityId = 0;
//		radius = 0;
//		//bookedPositions.clear();
//	}
//
//	EntityId entityId;
//	float radius;
//	//std::map<EntityId, primitives::box> bookedPositions; //vehicleId, vehicleBox
//
//	bool operator == (const SUnloadSpotInfo& info) 
//	{
//		return this->entityId == info.entityId;
//	}
//
//	bool operator == (const SUnloadSpotInfo* pInfo)
//	{
//		if (!pInfo)
//			return false;
//
//		return this->entityId == pInfo->entityId;
//	}
//
//};

struct SQueueRespawnInfo
{
	SQueueRespawnInfo()
	{
		entityId = 0;
		event = ERespawnEvent(0);
	}

	EntityId entityId;
	ERespawnEvent event;

	bool operator == (const SQueueRespawnInfo& info)
	{
		return this->entityId == info.entityId;
	}

	bool operator == (const SQueueRespawnInfo* pInfo)
	{
		if (!pInfo)
			return false;

		return this->entityId == pInfo->entityId;
	}

	bool operator == (EntityId id)
	{
		return this->entityId == id;
	}

	SQueueRespawnInfo& operator = (const SQueueRespawnInfo& info)
	{
		this->entityId = info.entityId;
		this->event = info.event;

		return *this;
	}

	SQueueRespawnInfo& operator = (const SQueueRespawnInfo* pInfo)
	{
		this->entityId = pInfo->entityId;
		this->event = pInfo->event;

		return *this;
	}

	//SQueueRespawnInfo& operator = (const SQueueRespawnInfo info)
	//{
	//	this->entityId = info.entityId;
	//	this->event = info.event;

	//	return *this;
	//}

};

struct SAreaQueueInfo
{
	SAreaQueueInfo()
	{
		reset();
	}

	void reset()
	{
		respawns.clear();
		respawnTimer = 0;
		lastTimeCreated = 0;
		lastTimeRemoved = 0;
	}

	std::deque<SQueueRespawnInfo> respawns;
	float respawnTimer;	
	float lastTimeCreated;
	float lastTimeRemoved;
};

struct SBuyZoneInfo
{
	SBuyZoneInfo()
	{
		reset();
	}

	void reset()
	{
		enabled = false;
		flags = 0;
	}

	bool enabled;
	uint flags;
};

class CStrategicArea :public CGameObjectExtensionHelper<CStrategicArea, IGameObjectExtension>
{
public:
	CStrategicArea();
	virtual ~CStrategicArea();

	friend class CConquerorSystem;
	friend class CAreaVehicleSpawnPoint;

	// IGameObjectExtension
	virtual bool Init(IGameObject* pGameObject);
	virtual void InitClient(int channelId) {};
	virtual void PostInit(IGameObject* pGameObject);
	virtual void PostInitClient(int channelId) {};
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual void PostUpdate(float frameTime) {};
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent(const SGameObjectEvent&);
	virtual void ProcessEvent(SEntityEvent&);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryStatistics(ICrySizer* s);
	//~IGameObjectExtension

	// Summary 
	//  Resetting the area to time when the game is started
	// Returns
	//  Boolean value
	//  true - when resetting is successfully complete
	bool		Reset();

	bool		IsActorInside(EntityId entityId);
	bool		IsActorInside(IActor* actor);
	bool		IsCapturable();
	bool		IsEnabled();
	bool		CanUnlockClasses(bool forPlayer);
	int			GetMembersCount();
	bool		CanSpawnSquadFromClasses();
	bool		IsUnlockedClass(ESpeciesType species, const char* name) const;

	IEntity*	GetAIAnchor();

	EntityId	GetActorIdAt(int entityIndex);
	IActor*		GetActorAt(int entityIndex);
	int			GetActorCount();
	int			GetActorCountBySpecies(int _species);
	const int GetAgentCount(ESpeciesType speciesType);
	int			GetEnemiesCount(ESpeciesType species);
	void		GetMaxAgentInfo(int& agentCount, ESpeciesType& agentSpecies);
	ESpeciesType GetSpecies() const;
	int			GetTeam();

	const float GetCaptureProgress(ESpeciesType species) const;
	void		GetMaxCaptureProgress(float& value, ESpeciesType& species);
	ECaptureState GetCaptureState();


	void		StartCapturing(ESpeciesType capturingSpecies);
	void		StartUncapturing(ESpeciesType capturingSpecies);

	void		SetCaptured(ESpeciesType& capturingSpecies);
	void		SetNeutral(bool reset);

	void		OnCaptured(ESpeciesType species);
	void		OnContested(ESpeciesType species);
	void		OnActorDeath(IActor* pActor);
	void		OnClientEnter(bool vehicle);
	void		OnClientExit();

	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle);
	void		OnExitVehicle(IActor* pActor);
	void		OnVehicleDestroyed(IVehicle* pVehicle);

	float		GetCaptureTime() const;

	ESpeciesType GetCapturingSpecies() const;
	float		GetEnemyCaptureProgress(ESpeciesType species) const;
	bool		IsNeutral() const;

	void		CreateScriptEvent(const char* event, float value, const char* str = NULL);

	void		AddSpawnPoint(EntityId id);
	bool		FindSpawnPoint(EntityId id);
	void		RemoveSpawnPoint(EntityId id);

	void		AddVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner);
	bool		FindVehicleSpawner(EntityId id) const;
	CAreaVehicleSpawnPoint*		GetVehicleSpawner(EntityId id) const;
	void		RemoveVehicleSpawner(CAreaVehicleSpawnPoint* pSpawner);

	CSpawnPoint* GetSpawnPointInstance(EntityId entityId);
	CSpawnPoint* GetSpawnPoint(ESpawnpointGameStatusFlag flag, uint speciesClassFlags);// pointStatus(1 = only noRecentlySpawned, 2 = noMatter)
	CSpawnPoint* GetSpawnPoint(ESpawnpointGameStatusFlag flag);// pointStatus(1 = only noRecentlySpawned, 2 = noMatter)
	CSpawnPoint* GetSpawnPointAt(int index, bool air);

	bool		IsHaveFlag(EAreaFlag flag);
	const		TAreaFlags&		GetFlags();

	void		GetUnlockedClasses(ESpeciesType species, std::vector<const char*>& classes);
	void		SpawnSquadForced(ESpeciesType species, const std::vector<const char*>& classes);
	void		DespawnSquadForced(const int squadId);

	const int	GetSpawnPointCount() const;

	//--Queue define--
	bool		CreateQueue(float respawnTimer);
	bool		IsQueueCreated();
	bool		IsInQueue(EntityId entityId);
	void		AddToQueue(EntityId entityId, ERespawnEvent event);//Respawn Event is no matter now
	bool		EraseFromQueue(EntityId entityId);
	void		EraseFirstFromQueue();
	int			GetQueueSize() const;
	float		GetQueueTimer() const;
	void GetFirstInQueue(SQueueRespawnInfo& info);
	SQueueRespawnInfo* GetLastInQueue();
	void		DeleteQueue();
	void		UpdateQueue(float frametime);

	//TODO: add OnQueueDeleted() when area is captured by enemy faction

	//--Booked SpawnPoints--
	bool		IsBookedSpawnPoint(EntityId spawnpointId);
	CSpawnPoint* GetBookedSpawnPoint(IActor* pActor);
	CSpawnPoint* BookFreeSpawnPoint(IActor* pActor, bool forAIR);
	bool		UnbookSpawnPoint(IActor* pActor);
	bool		UnbookSpawnPoint(EntityId spawnpointId);

	//Unload spots
	bool		IsMyUnloadSpot(EntityId spotId);
	void		GetUnloadSpots(const char* spotType, std::vector<EntityId>& spots);
	//EntityId	GetBookedUnloadSpot(IVehicle* pVehicle);
	EntityId	GetBookedUnloadSpot(IEntity* pEntity);
	//EntityId	BookFreeUnloadSpot(IVehicle* pVehicle);
	EntityId	BookFreeUnloadSpot(IEntity* pEntity);
	bool		IsBookedUnloadSpot(EntityId spotId) const;
	//bool		UnbookUnloadSpot(IVehicle* pVehicle);
	bool		UnbookUnloadSpot(IEntity* pEntity);

	void		GetSpawnedVehicles(std::vector<EntityId>& vehicles) const;
	void		GetSpawnedVehicles(std::vector<EntityId>& vehicles, uint flags, int minSeatCount) const;

	void		OnAIJoinGame();

	Vec3		GetWorldPos();
	void		GetBuyZoneInfo(SBuyZoneInfo& info);
	bool IsBuyZoneActived(ESpeciesType species) const;

protected:
	std::deque<SQueueRespawnInfo>::iterator GetQueueBeginIt();
	std::deque<SQueueRespawnInfo>::iterator GetQueueEndIt();
private:

	void		AddVehicleUnloadSpot(const char* spotType, EntityId spotId);
	void		ResetVehicleUnloadSpots();

	// Summary 
	//  Add's the entity to inside the area;
	void		AddToArea(EntityId entityId, bool vehicle);
	void		AddToArea(IActor* pActor, bool vehicle);

	// Summary 
	//  Removes the entity from the area;
	void		RemoveFromArea(EntityId entityId);
	void		RemoveFromArea(IActor* pActor);
	void		RemoveActorAt(int entityIndex);


	EEntityType GetEntityType(EntityId id);
	bool		GetContested(ESpeciesType capturingSpecies);

	void		ChangeAgentCount(EntityId entityId, const char* operation);
	void		GetLuaValues();
	void		SetCaptureProgress(ESpeciesType species, float progress);

	// Summary 
	//  Returns the only agents of this species in the area?
	// Returns
	//  Boolean value
	//  true - only agents of this species in the area
	//  false - not only agents of this species in the area
	bool IsSpeciesExclusive(ESpeciesType species);

	bool		IsValidEntityType(const EEntityType& requestedType, const EEntityType& type);
	bool IsAreaOwner(const ESpeciesType& species);
	
	void		OnGameStart();

	std::vector<EntityId> m_ActorsInside;
	std::vector<CSpawnPoint*> m_spawnPoints;
	//std::vector<CSpawnPoint> m_airSpawnPoints;
	std::vector<EntityId> m_vehicleSpawners;
	//std::vector<int> m_forcedspawnedSquadIds;
	std::map<ESpeciesType, int> m_forcedspawnedSquadId;

	std::map<ESpeciesType, int> m_SpeciesMembers;
	std::map<ESpeciesType, float> m_CaptureProgress;

	TAreaFlags m_AreaFlags;

	//Get Values From Editor:
	ESpeciesType m_LuaSpecies;
	int m_LuaTeam;

	EMultiplayerSide m_DefinedMultiplayerSide;
	ESpeciesType m_Species;
	//ESpeciesType m_OldOwnerSpecies;
	int m_Team;
	int m_CaptureRequirement;
	float m_CaptureTime;
	bool m_bIsCapturable;
	bool m_bIsEnabled;

	int m_SpawnedMembersCount;
	bool m_bCanUnlockClassesForAI;
	bool m_bCanUnlockClassesForPlayer;
	bool m_bSpawnSquadFromClasses;
	//~Get Values From Editor

	float m_lastUpdateTime;

	std::map<EntityId, EntityId> m_bookedUnloadSpots; //vehicleId, spotId
	std::map<const char*, std::vector<EntityId>> m_unloadSpots;

	SBuyZoneInfo m_buyZone;
	EntityId m_flagEntityId;
	EntityId m_centreAnchorId;
	EntityId m_ShapeEntityId;
	float m_CaptureStep;
	ESpeciesType m_CapturingSpecies;
	ECaptureState m_CaptureState;
	std::map<ESpeciesType, std::vector<const char*>> m_unlockedClasses;

	//  Capture progress from 0 to Capture Time;
	float m_FinalCaptureProgress;

	std::map<EntityId, EntityId> m_bookedSpawnPoints; //actorId, spawnpointId
	SAreaQueueInfo* m_pQueue;
};

#endif //__CAPTURABLEAREA_H__