#pragma once

class CCapturableArea :public CGameObjectExtensionHelper<CCapturableArea, IGameObjectExtension>
{
public:
	CCapturableArea();
	~CCapturableArea();

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

	//Resetting the area to time when the game is started
	void Reset();

private:

	std::vector<EntityId> m_EntitiesInside;
};