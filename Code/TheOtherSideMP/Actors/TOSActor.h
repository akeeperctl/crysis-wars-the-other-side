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

class CTOSActor: public CActor, ITOSMasterControllable  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterClient;
	friend class CCompatibilityAlienMovementController;

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

	const STOSSlaveStats& ReadSlaveStats() const { return m_slaveStats; } ///< Считать статистику раба. Изменять нельзя.

private:
	STOSSlaveStats& GetSlaveStats() { return m_slaveStats; } ///< Получить статистику раба. Изменять можно.


	EntityId m_slaveEntityId;
	EntityId m_masterEntityId;
	STOSSlaveStats m_slaveStats;
};
