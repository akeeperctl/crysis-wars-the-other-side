/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#define TOS_CHECK_CONSUMER_EXISTING(pActor)\
	CRY_ASSERT_MESSAGE(pActor, "Pointer to actor is nullptr");\
	CRY_ASSERT_MESSAGE((pActor)->GetEnergyConsumer(), "Pointer to Energy Consumer instance is nullptr")\

/**
 * \brief Безопасно добавить энергии актёру-потребителю
 * \param pActor - укзатель на актёра
 * \param energy - величина энергии
 */
#define TOS_SAFE_ADD_ENERGY(pActor, energy)\
	assert(pActor);\
	assert((pActor)->GetEnergyConsumer());\
	if ((pActor) && (pActor)->GetEnergyConsumer())\
	{\
		(pActor)->GetEnergyConsumer()->AddEnergy(energy);\
	}\

/**
 * \brief Безопасно получить энергию актёра-потребителя
 * \param pActor - укзатель на актёра
 */
#define TOS_SAFE_GET_ENERGY(pActor)\
	(pActor) && (pActor)->GetEnergyConsumer() ? (pActor)->GetEnergyConsumer()->GetEnergy() : 0.0f\

#include "TheOtherSideMP/Game/Modules/GenericSynchronizer.h"

class CTOSEnergyConsumer : public CGameObjectExtensionHelper<CTOSEnergyConsumer, IGameObjectExtension, 32>
{
public:
	CTOSEnergyConsumer();
	virtual ~CTOSEnergyConsumer();

	// IGameObjectExtension
	bool Init(IGameObject* pGameObject) ;
	void PostInit(IGameObject* pGameObject) ;
	void InitClient(int channelId) ;
	void PostInitClient(int channelId) ;
	void Release() ;
	void FullSerialize(TSerialize ser) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;
	void PostSerialize()  {};
	void SerializeSpawnInfo(TSerialize ser)  {}
	ISerializableInfoPtr GetSpawnInfo() {return 0;}
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	void PostUpdate(float frameTime)  {};
	void PostRemoteSpawn()  {};
	void HandleEvent(const SGameObjectEvent&) ;
	void ProcessEvent(SEntityEvent&) ;
	void SetChannelId(uint16 id)  {};
	void SetAuthority(bool auth)  {};
	void GetMemoryStatistics(ICrySizer* s) ;
	//~IGameObjectExtension

	bool		AddEnergy(float value);
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
	float		GetRegenStartDelay() const;
	void		SetRechargeTimeSP(float time);
	void		SetRechargeTimeMP(float time);

	bool		SetRegenStartDelaySP(float val);
	bool		SetRegenStartDelayMP(float val);
	bool		SetRegenStartDelay20Boundary(float val);


	static bool SetDebugEntityName(const char* name);

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	struct NetEnergyParams
	{
		float energy;
		bool initiated;
		bool forced;
		NetEnergyParams()
			: energy(0),
			initiated(false),
			forced(false) {} ;

		explicit NetEnergyParams(float _energy, bool _initiated, bool _forced)
			: energy(_energy), initiated(_initiated), forced(_forced)
		{
		}

		void SerializeWith(TSerialize ser)
		{
			ser.Value("energy", energy);
			ser.Value("initiated", initiated, 'bool');
			ser.Value("forced", forced, 'bool');
		}
	};


	DECLARE_SERVER_RMI_PREATTACH(SvRequestSetEnergy, NetEnergyParams, eNRT_ReliableOrdered);

	static string s_debugEntityName;
	static const int DEFAULT_ENERGY = 200;

private:
	float m_energy;
	float m_maxEnergy;
	float m_regenStartDelay; // задержка в сек. перед стартом регенерации
	float m_regenStartDelaySP;
	float m_regenStartDelayMP;
	float m_regenStartDelay20Boundary;
	float m_rechargeTimeSP;
	float m_rechargeTimeMP;
	float m_drainValue; // единицы в секунду

	bool m_enableUpdate;
};
