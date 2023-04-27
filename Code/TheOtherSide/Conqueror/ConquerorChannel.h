#pragma once

class CSpawnPoint;

enum EConquerorChannelState
{
	eCCS_Alive,
	eCCS_Spectator,
	eCCS_Dead,
};

enum ESpectatorEvent
{
	eSC_EnterLobby = 0,
	eSC_PlayerSpawned = 0,
};

enum ERespawnEvent
{
	eRC_OnGameStartSpawn = 0,
	eRC_OnKilledRespawn,
	eRC_ForcedRespawn,
	eRC_OnClassChangedRespawn,
	eRC_OnClientSpectatorRespawn,
};

class CConquerorChannel
{
public:
	CConquerorChannel(IEntity* pEntity, const char* className);
	~CConquerorChannel();

	friend class CConquerorSystem;

	//IGoalPipeListener
	//virtual void OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, int goalPipeId);
	//~IGoalPipeListener


	//Events
	void			OnKilled();
	void			OnRespawned(ERespawnEvent respawnEvent);
	void			OnSpectator(ESpectatorEvent spectatorCase);
	//~Events

	void			SetState(EConquerorChannelState state);
	void			SetControlledEntity(EntityId entityId);

	void			GetMemoryStatistics(ICrySizer* s);

	float			GetRemainingRespawnTime();
	float			GetStateDuration(EConquerorChannelState state);
	EConquerorChannelState	GetState() const;
	ESpeciesType	GetSpecies();
	IEntity*		GetEntity();
	IEntity*		GetControlledEntity();
	IActor*			GetActor();
	CSpeciesClass*	GetClass();

	const char*		GetName();
	void			SetName(const char* name);

	bool			SpawnActor(ERespawnEvent respawnCase, CSpawnPoint* pSpawnPoint);
	bool			IsPlayer();
	bool			IsClient() noexcept;

	void			Update(float frametime);

	int				GetPoints() noexcept;
	void			AddPoints(int value) noexcept;

	void			DisableRespawnRequest(bool disable);
	void			SetForcedStrategicArea(CStrategicArea* pArea);
	void			SetSelectedStrategicArea(CStrategicArea* pArea);
	const CStrategicArea* GetLastArea();
	CStrategicArea* GetSelectedArea();
private:
	void			AddStateDuration(EConquerorChannelState state, float duration);

	void			OnSetState(EConquerorChannelState state);
	void			OnSetControlled(EntityId entityId);

	//CSpawnPoint*	RequestSpawnPoint(bool requestForClient, CStrategicArea* pCapturableArea=nullptr);
	//CStrategicArea* RequestStrategicArea(bool requestForClient, std::vector<EntityId>* excludeAreaIds);
	//void			RequestRespawn(ERespawnEvent respawnCase);
	CStrategicArea* GetLeaderStrategicArea();

	void			SetSpecies(int species);
	void			SetSpecies(IEntity* pEntity);

	void			SetSquadId(int id);
	void			SetClass(CSpeciesClass* classInfo);
	void			RemoveInventoryItems(IEntity* pEntity);

	void			ResetStateDuration(EConquerorChannelState state);

	std::map<EConquerorChannelState, float> m_stateDurationMap;

	bool m_disableRespawnRequest;

	int m_id;
	int m_squadId;
	int m_species; // team analog
	int m_points;
	Vec3 m_lastDeathPos;
	EntityId m_entityId;
	EntityId m_controlledEntityId;
	EConquerorChannelState m_state;
	CSpeciesClass* m_pClass;
	CStrategicArea* m_pLastStrategicArea;
	CStrategicArea* m_pForcedStrategicArea;
	CStrategicArea* m_pSelectedStrategicArea;
};