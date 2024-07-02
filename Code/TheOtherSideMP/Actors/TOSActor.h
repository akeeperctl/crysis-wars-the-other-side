#pragma once

#include "Actor.h"
#include "ITOSMasterControllable.h"

class CTOSEnergyConsumer;

struct STOSSlaveStats
{
	STOSSlaveStats()
		: lookAtFriend(false),
		jumpCount(0),
		chargingJumpPressDur(0)
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
	float chargingJumpPressDur;
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

struct NetMarkMeParams
{
	NetMarkMeParams()
		: value(false) {};

	explicit NetMarkMeParams(const bool _slave_or_master) :
		value(_slave_or_master)
	{}

	bool value;

	void SerializeWith(TSerialize ser)
	{
		ser.Value("value", value, 'bool');
	}
};

struct NetHideMeParams
{
	NetHideMeParams()
		: hide(false) {};

	explicit NetHideMeParams(const bool _hide) :
		hide(_hide)
	{}

	bool hide;

	void SerializeWith(TSerialize ser)
	{
		ser.Value("hide", hide, 'bool');
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
		worldPos(ZERO),
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
		ser.Value("world_position", worldPos, 'wrld');
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
		moveTarget = Vec3(0, 0, 0);
		aimTarget = Vec3(0, 0, 0);
		lookTarget = Vec3(0, 0, 0);
		bodyTarget = Vec3(0, 0, 0);
		fireTarget = Vec3(0, 0, 0);
		deltaMov = Vec3(0, 0, 0);
		worldPos = Vec3(0, 0, 0);

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

	Vec3 worldPos;

	//float pseudoSpeed;
	float desiredSpeed;

	//int alertness;
	int stance;
	//int suitMode;

	bool hidden;
	//bool m_allowStrafing;
	//bool m_hasAimTarget;
};

class CTOSActor: 
	public CActor, 
	public ITOSMasterControllable  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:

	friend class CTOSMasterModule;
	friend class CTOSMasterClient;

	enum MPTimers
	{
		eMPTIMER_GIVEWEAPONDELAY = 0x110,
		eMPTIMER_REMOVEWEAPONSDELAY = 0x111,
	};

	struct NetAttachChild
	{
		NetAttachChild()
			: id(0), flags(0)
		{}

		EntityId id;
		uint flags;

		void SerializeWith(TSerialize ser)
		{
			CryLogAlways("<c++> call %f", __FUNCTION__);
			ser.Value("childId", id, 'eid');
			ser.Value("flags", flags, 'ui8');
		}
	};

	CTOSActor();
	~CTOSActor() ;

	// CActor
	bool Init(IGameObject* pGameObject) ;
	void PostInit( IGameObject * pGameObject ) ;
	void InitClient(int channelId ) ;
	void ProcessEvent(SEntityEvent& event) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;
	void SelectNextItem(int direction, bool keepHistory, const char* category) ;
	void HolsterItem(bool holster) ;
	void SelectLastItem(bool keepHistory, bool forceNext) ;
	void SelectItemByName(const char* name, bool keepHistory) ;
	void SelectItem(EntityId itemId, bool keepHistory) ;
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	void Release() ;
	void Revive(bool fromInit = false) ;
	void Kill() ;
	void PlayAction(const char* action, const char* extension, bool looping = false) ;
	void AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event) ;

	void NetKill(EntityId shooterId, uint16 weaponClassId, int damage, int material, int hit_type, int killerHealthOnKill) ;
	void NetReviveAt(const Vec3& pos, const Quat& rot, int teamId) ;
	void NetReviveInVehicle(EntityId vehicleId, int seatId, int teamId) ;
	void NetSimpleKill() ;
	// ~CActor

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov)  {};
	void ApplyMasterMovement(const Vec3& delta)  {};
	//~ITOSMasterControllable

	bool ResetActorWeapons(int delayMilliseconds);

	virtual Matrix33 GetViewMtx() { return Matrix33(); };
	virtual Matrix33 GetBaseMtx() { return Matrix33(); };
	virtual Matrix33 GetEyeMtx() { return Matrix33(); };
	
	virtual bool ShouldUsePhysicsMovement();
	virtual bool ApplyActions(int actions); // нужна для поддержки m_actions не только в игроке

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
	bool IsMaster() const {return m_isMaster;}
	bool IsLocalSlave() const; ///< проверка на локальной машине является ли актёр рабом

	virtual CTOSEnergyConsumer* GetEnergyConsumer() const;
	bool IsHaveChargingJump() const {return m_chargingJump;}

	bool UpdateLastMPSpawnPointRotation(const Quat& rotation);
	bool UpdateLastShooterId(const EntityId id);

protected:
	// Сделать мастером или рабом на стороне сервера
	bool SetMeSlave(bool value);
	bool SetMeMaster(bool value);
	string m_debugName;

private:
	// Скрыть актера на стороне клиента
	bool HideMe(bool value);

	string m_sLastNetworkedAnim;

	Quat m_lastSpawnPointRotation;
	EntityId m_lastShooterId;

	bool m_isSlave; // сериализованное по сети значение, является ли актёр рабом
	bool m_isMaster; // сериализованное по сети значение, является ли актёр мастером

	// Зафиксировать на всех клиентах и сервере, что данный актёр стал рабом. Вызывать только с клиента!
	//void NetMarkMeSlave(bool slave) const;

	// Зафиксировать на всех клиентах и сервере, что данный актёр стал master. Вызывать только с клиента!
	//void NetMarkMeMaster(bool master) const;

	DECLARE_SERVER_RMI_NOATTACH(SvRequestPlayAnimation, NetPlayAnimationParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClPlayAnimation, NetPlayAnimationParams, eNRT_ReliableOrdered);

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMarkMeAsSlave, NetMarkMeParams, eNRT_ReliableOrdered);
	//DECLARE_CLIENT_RMI_NOATTACH(ClMarkMeAsSlave, NetMarkMeParams, eNRT_ReliableOrdered);	
	//
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMarkMeAsMaster, NetMarkMeParams, eNRT_ReliableOrdered);
	//DECLARE_CLIENT_RMI_NOATTACH(ClMarkMeAsMaster, NetMarkMeParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvRequestHideMe, NetHideMeParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClMarkHideMe, NetHideMeParams, eNRT_ReliableOrdered);

	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClTOSJump, NoParams, eNRT_ReliableUnordered);
	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestTOSJump, NoParams, eNRT_ReliableUnordered);

	DECLARE_CLIENT_RMI_POSTATTACH(ClAttachChild, NetAttachChild, eNRT_ReliableUnordered);
	DECLARE_SERVER_RMI_POSTATTACH(SvRequestAttachChild, NetAttachChild, eNRT_ReliableUnordered);

protected:
	bool m_chargingJump;///< Если *true, то высота прыжка зависит от длительности нажатия на дейсвие прыжка [jump]

	STOSSlaveStats m_slaveStats;
	//Vec3 m_filteredDeltaMovement;
	STOSNetBodyInfo m_netBodyInfo;///< Информация о состоянии тела актёра, передаваемая по сети

	CTOSEnergyConsumer* m_pEnergyConsumer;
};
