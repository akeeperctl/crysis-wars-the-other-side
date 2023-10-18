#pragma once

#include "Actor.h"
#include "ITOSMasterControllable.h"

struct STOSSlaveStats
{
	STOSSlaveStats()
		: jumpCount(0) { }

	//TODO: 10/15/2023, 20:26 нужно добавить механики
	//CCoherentValue<bool> canShoot;
	//CCoherentValue<bool> canMove;
	//CCoherentValue<bool> canLookAtCamera;
	//CCoherentValue<bool> isAiming;
	//CCoherentValue<bool> isShooting;
	//CCoherentValue<bool> isUsingBinocular;

	uint jumpCount;
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
	friend class CTOSMasterClient;

	CTOSActor();
	~CTOSActor() override;

	// CActor
	void PostInit( IGameObject * pGameObject ) override;
	void InitClient(int channelId ) override;
	void ProcessEvent(SEntityEvent& event) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void Release() override;
	void Revive(bool fromInit = false) override;
	void Kill() override;
	// ~CActor

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) override {};
	void ApplyMasterMovement(const Vec3& delta) override {};
	//~ITOSMasterControllable

	virtual Matrix33 GetViewMtx() { return {}; }
	virtual Matrix33 GetBaseMtx() { return {}; }
	virtual Matrix33 GetEyeMtx() { return {}; }

	void SetMasterEntityId(EntityId id);
	EntityId GetMasterEntityId() const {return m_masterEntityId;}
	void SetSlaveEntityId(EntityId id);
	EntityId GetSlaveEntityId() const { return m_slaveEntityId;}

	//Новые функции сюда
	//const Vec3& FilterDeltaMovement(const Vec3& deltaMov);

	const STOSSlaveStats& ReadSlaveStats() const { return m_slaveStats; } ///< Считать статистику раба. Изменять нельзя.

protected:
	STOSSlaveStats& GetSlaveStats() { return m_slaveStats; } ///< Получить статистику раба. Изменять можно.

	STOSSlaveStats m_slaveStats;
	//Vec3 m_filteredDeltaMovement;
	STOSNetBodyInfo m_netBodyInfo;///< Информация о состоянии тела актёра, передаваемая по сети

private:
	EntityId m_slaveEntityId;
	EntityId m_masterEntityId;
};
