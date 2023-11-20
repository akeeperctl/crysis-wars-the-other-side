#pragma once

#include "Actor.h"
#include "ITOSMasterControllable.h"

class CTOSEnergyConsumer;

struct STOSSlaveStats
{
	STOSSlaveStats()
		: lookAtFriend(false),
		jumpCount(0)
	{ }

	//TODO: 10/15/2023, 20:26 нужно добавить механики
	//CCoherentValue<bool> canShoot;
	//CCoherentValue<bool> canMove;
	//CCoherentValue<bool> canLookAtCamera;
	//CCoherentValue<bool> isAiming;
	//CCoherentValue<bool> isShooting;
	//CCoherentValue<bool> isUsingBinocular;

	CCoherentValue<bool> lookAtFriend;

	uint jumpCount;
};

struct NetPlayAnimationParams
{
	NetPlayAnimationParams()
		: mode(0) {} ;
	NetPlayAnimationParams(const uint _mode, const string& _animation) :
		mode(_mode),
		animation(_animation)
	{};

	uint mode;
	string animation;

	void SerializeWith(TSerialize ser)
	{
		ser.Value("mode", mode, 'ui2');
		ser.Value("animation", animation, 'stab');
	}
};

struct NetMarkMeAsSlaveParams
{
	NetMarkMeAsSlaveParams()
		: slave(false) {};

	explicit NetMarkMeAsSlaveParams(const bool _slave) :
		slave(_slave)
	{}

	bool slave;

	void SerializeWith(TSerialize ser)
	{
		ser.Value("slave", slave, 'bool');
	}
};

/**
 * \brief Информация о состоянии тела актёра, передеваемая по сети
	\note Канал, владеющий сущностью будет записывать данные, а остальные в т.ч сервер, будут принимать данные.
	\note В коопе по умолчанию сущностью владеет сервер. Это несколько упрощает задачу.
	\note В TOS сущностью может владеть не только сервер но и клиент тоже. Например когда управляет кем-то.
 */
struct STOSNetBodyInfo
{
	STOSNetBodyInfo()
		: moveTarget(ZERO),
		aimTarget(ZERO),
		lookTarget(ZERO),
		bodyTarget(ZERO),
		fireTarget(ZERO),
		deltaMov(ZERO),
		//pseudoSpeed(ZERO), grunt штука
		desiredSpeed(ZERO),
		//alertness(ZERO), grunt штука
		stance(ZERO),
		//suitMode(ZERO),
		hidden(false)
		//allowStrafing(false), grunt штука
		//hasAimTarget(false) grunt штука
	{ }

	void Serialize(IEntity* pInfoOwnerEntity, TSerialize ser)
	{
		assert(pInfoOwnerEntity);

		// GameServerDynamic в коопе был, может GameClientDynamic у меня?
		ser.Value("moveTarget", moveTarget, 'wrld');
		ser.Value("aimTarget", aimTarget, 'wrld');
		ser.Value("lookTarget", lookTarget, 'wrld');
		ser.Value("bodyTarget", bodyTarget, 'wrld');
		ser.Value("fireTarget", fireTarget, 'wrld');
		ser.Value("deltaMov", deltaMov, 'pMov');
		//ser.Value("pseudoSpeed", pseudoSpeed); grunt штука
		ser.Value("desiredSpeed", desiredSpeed);
		//~ GameServerDynamic

		// GameServerStatic в коопе был
		ser.Value("hidden", hidden, 'bool');

		if (ser.IsReading())
			pInfoOwnerEntity->Hide(hidden);

		//ser.Value("nAlert", alertness, 'i8'); grunt штука
		ser.Value("stance", stance, 'i8');
		//ser.Value("nFlags", nMovementNetworkFlags, 'i8');
		//ser.Value("nWep", nWeaponNetworkFlags, 'i8');
		// ~GameServerStatic
	}

	void Reset()
	{
		moveTarget = {0,0,0};
		aimTarget  = {0,0,0};
		lookTarget = {0,0,0};
		bodyTarget = {0,0,0};
		fireTarget = {0,0,0};
		deltaMov = {0,0,0};

		//pseudoSpeed;
		desiredSpeed = 0;

		//alertness;
		stance = 0;
		//suitMode;

		hidden = false;
		//bool allowStrafing;
		//bool hasAimTarget;

	}

	// Скопировано из CoopGrunt.h
	Vec3 moveTarget;
	Vec3 aimTarget;
	Vec3 lookTarget;
	Vec3 bodyTarget;
	Vec3 fireTarget;
	Vec3 deltaMov;

	//float pseudoSpeed;
	float desiredSpeed;

	//int alertness;
	int stance;
	//int suitMode;

	bool hidden;
	//bool m_allowStrafing;
	//bool m_hasAimTarget;
};

class CTOSActor: public CActor, ITOSMasterControllable  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	enum MPTimers
	{
		eMPTIMER_GIVEWEAPONDELAY = 0x110,
	};

	friend class CTOSMasterClient;

	CTOSActor();
	~CTOSActor() override;

	// CActor
	bool Init(IGameObject* pGameObject) override;
	void PostInit( IGameObject * pGameObject ) override;
	void InitClient(int channelId ) override;
	void ProcessEvent(SEntityEvent& event) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void SelectNextItem(int direction, bool keepHistory, const char* category) override;
	void HolsterItem(bool holster) override;
	void SelectLastItem(bool keepHistory, bool forceNext) override;
	void SelectItemByName(const char* name, bool keepHistory) override;
	void SelectItem(EntityId itemId, bool keepHistory) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void Release() override;
	void Revive(bool fromInit = false) override;
	void Kill() override;
	void PlayAction(const char* action, const char* extension, bool looping = false) override;
	void AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event) override;

	// ~CActor

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) override {};
	void ApplyMasterMovement(const Vec3& delta) override {};
	//~ITOSMasterControllable

	virtual Matrix33 GetViewMtx() { return {}; }
	virtual Matrix33 GetBaseMtx() { return {}; }
	virtual Matrix33 GetEyeMtx() { return {}; }



	//Новые функции сюда
	//const Vec3& FilterDeltaMovement(const Vec3& deltaMov);

	//const STOSSlaveStats& ReadSlaveStats() const { return m_slaveStats; } ///< Считать статистику раба. Изменять нельзя.

	// Скопировано из CActor в Crysis Co-op
	//struct SQueuedAnimEvent
	//{
	//	SQueuedAnimEvent() : sAnimEventName(""), fEventTime(0.f), fElapsed(0.f) {};
	//	SQueuedAnimEvent(const string& name, const float eventTime) : sAnimEventName(name), fEventTime(eventTime), fElapsed(0.f) {};
	//	string sAnimEventName;
	//	float fEventTime;
	//	float fElapsed;
	//};

	//virtual bool IsAnimEvent(const char* sAnimSignal, string* sAnimEventName, float* fEventTime)
	//{
	//	*sAnimEventName = "";
	//	*fEventTime = 0.f;
	//	return false;
	//};

	//void QueueAnimationEvent(const SQueuedAnimEvent& sEvent);
	//void UpdateAnimEvents(float fFrameTime);

	void OnAGSetInput(bool bSucceeded, IAnimationGraphState::InputID id, float value, TAnimationGraphQueryID* pQueryID);
	void OnAGSetInput(bool bSucceeded, IAnimationGraphState::InputID id, int value, TAnimationGraphQueryID* pQueryID);
	void OnAGSetInput(bool bSucceeded, IAnimationGraphState::InputID id, const char* value, TAnimationGraphQueryID* pQueryID);
	// ~Скопировано из CActor в Crysis Co-op

	STOSSlaveStats& GetSlaveStats() { return m_slaveStats; } ///< Получить статистику раба. Изменять можно.
	bool IsSlave() const {return m_isSlave;}
	bool IsLocalSlave() const; ///< проверка на локальной машине является ли актёр рабом

	virtual CTOSEnergyConsumer* GetEnergyConsumer() const;

private:

	//std::list<SQueuedAnimEvent> m_AnimEventQueue;
	string m_sLastNetworkedAnim;
	bool m_isSlave;///< сериализованное по сети через RMI значение, является ли актёр рабом

	void NetMarkMeSlave(bool slave) const;///< Зафиксировать на всех клиентах и сервере, что данный актёр стал рабом. \n Вызывать только с клиента!

	DECLARE_SERVER_RMI_NOATTACH(SvRequestPlayAnimation, NetPlayAnimationParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClPlayAnimation, NetPlayAnimationParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvRequestMarkMeAsSlave, NetMarkMeAsSlaveParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClMarkMeAsSlave, NetMarkMeAsSlaveParams, eNRT_ReliableOrdered);

protected:

	STOSSlaveStats m_slaveStats;
	//Vec3 m_filteredDeltaMovement;
	STOSNetBodyInfo m_netBodyInfo;///< Информация о состоянии тела актёра, передаваемая по сети

	CTOSEnergyConsumer* m_pEnergyConsumer;
};
