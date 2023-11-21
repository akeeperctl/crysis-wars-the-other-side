#pragma once

class CTOSEnergyConsumer : public CGameObjectExtensionHelper<CTOSEnergyConsumer, IGameObjectExtension, 32>
{
public:
	CTOSEnergyConsumer();
	virtual ~CTOSEnergyConsumer();

	// IGameObjectExtension
	bool Init(IGameObject* pGameObject) override;
	void PostInit(IGameObject* pGameObject) override;
	void InitClient(int channelId) override;
	void PostInitClient(int channelId) override;
	void Release() override;
	void FullSerialize(TSerialize ser) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void PostSerialize() override {};
	void SerializeSpawnInfo(TSerialize ser) override {}
	ISerializableInfoPtr GetSpawnInfo() override{return 0;}
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void PostUpdate(float frameTime) override {};
	void PostRemoteSpawn() override {};
	void HandleEvent(const SGameObjectEvent&) override;
	void ProcessEvent(SEntityEvent&) override;
	void SetChannelId(uint16 id) override {};
	void SetAuthority(bool auth) override {};
	void GetMemoryStatistics(ICrySizer* s) override;
	//~IGameObjectExtension

	bool        SetEnergy(float value, bool initiated = false);
	bool        SetEnergyForced(float value);
	float       GetEnergy() const;
	bool        SetMaxEnergy(float value);
	float       GetMaxEnergy() const;
	bool        SetDrainValue(float value);
	float       GetDrainValue() const;
	void        EnableUpdate(bool enable);
	bool        IsUpdating() const;
	void        Reset();
	static bool SetDebugEntityName(const char* name);

	static string s_debugEntityName;
	static constexpr float DEFAULT_ENERGY = 200.0f;

private:
	float m_energy;
	float m_maxEnergy;
	float m_regenStartDelay; // задержка в сек. перед стартом регенерации
	float m_drainValue; // единицы в секунду

	bool m_enableUpdate;
};
