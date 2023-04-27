#pragma once
#include <IGameObject.h>
#include <IVehicleSystem.h>

class IVehicleEventListener;
class CStrategicArea;

enum EVehicleEvent;
enum ESpeciesType;
enum EVehicleGetFlags;

enum EVehicleMovementType
{
	eVMT_Sea = 0,
	eVMT_Air,
	eVMT_Land,
	eVMT_Amphibious,
	eVMT_Other
};

enum EVehicleDestroyFlag
{
	eVDF_ForRespawn = 0,
	eVDF_ForRemove,
};

class CAreaVehicleSpawnPoint : public CGameObjectExtensionHelper<CAreaVehicleSpawnPoint, IGameObjectExtension>, public IVehicleEventListener
{
public:
	CAreaVehicleSpawnPoint();

	friend class CConquerorSystem;

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

	//IVehicleEventListener
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params);
	//~IVehicleEventListener

	//Events
	void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle);
	void		OnExitVehicle(IActor* pActor);
	void		OnStrategicAreaSpeciesChanged(ESpeciesType species);
	void AttachStrategicArea(CStrategicArea* pArea);

	//~Events

	EntityId	SpawnVehicleArchetype(const char* archetype);
	void		DeleteSpawnedVehicle();
	//void		RespawnVehicle(EntityId id);
	//void		TriggerVehicleDestroy(EntityId id, bool removeFromGame);
	IVehicle*	GetSpawnedVehicle();
	IVehicle*	GetVehicle(EVehicleMovementType movType, EVehicleGetFlags flag, int maxSeatCount);// pointStatus(1 = only noRecentlySpawned, 2 = noMatter)
	IVehicle*	GetVehicle(ESpeciesType species, EVehicleMovementType movType, EVehicleGetFlags flag, int maxSeatCount);// pointStatus(1 = only noRecentlySpawned, 2 = noMatter)

	ESpeciesType GetSpecies() const;

	//bool		GameStart();
	bool		Reset();
	bool		IsEnabled();
private:
	//void		UpdateOldVehicleScheduler(float frametime);
	void		UpdateSpawnedVehicleScheduler(float frametime);
	void ScheduleVehicleAutoDestroy(EntityId entityId, float timer);
	void		CancelVehicleAutoDestroy(EntityId entityId);

	//void		DefineOldVehicles(std::vector<EntityId>& spawnedVehicles);
	//void		ForceDeleteAllVehicles();

	//void		InitVehicleRespawn(EntityId id);
	//void		TriggerVehicleRespawn(EntityId id);

	void		GetLuaValues();
	EntityId SpawnRandomArchetype(ESpeciesType species);

	bool		m_bIsEnabled;
	ESpeciesType		m_species;

	std::map<ESpeciesType, std::vector<const char*>> m_speciesVehicleArchetypesMap;

	EntityId m_spawnedVehicleId;
	//float m_lastTimeVehicleDestroyed;
	bool m_canRespawn;
	bool m_canAbandon;
	float m_respawnTimer;
	float m_abandonTimer;
	CStrategicArea* m_pArea;
	//float m_lastTimeVehicleSpawned;

	//std::vector<EntityId> m_spawnedVehicles;
	//std::vector<EntityId> m_oldSpawnedVehicles;

	std::map<EntityId, int> m_vehiclePlayingAlarm;
	std::map<EntityId, float> m_vehicleAutoDestroyScheduler;
	std::map<EntityId, float> m_vehicleRespawnScheduler;
	std::map<EntityId, std::vector<EntityId>> m_vehiclePassengers;
};