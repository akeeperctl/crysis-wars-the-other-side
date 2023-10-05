#pragma once

#include "Actor.h"
#include "ITOSMasterControllable.h"

class CTOSActor: public CActor, ITOSMasterControllable  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSActor();
	~CTOSActor() override;

	// CActor
	void PostInit( IGameObject * pGameObject ) override;
	void InitClient(int channelId ) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void Release() override;
	void Revive(bool fromInit = false) override;
	// ~CActor

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) override;
	//~ITOSMasterControllable

	virtual Matrix33 GetViewMtx() { return {}; }
	virtual Matrix33 GetBaseMtx() { return {}; }
	virtual Matrix33 GetEyeMtx() { return {}; }

	void SetMasterEntityId(EntityId id);
	EntityId GetMasterEntityId() const {return m_masterEntityId;}
	void SetSlaveEntityId(EntityId id);
	EntityId GetSlaveEntityId() const { return m_slaveEntityId;}

private:

	EntityId m_slaveEntityId;
	EntityId m_masterEntityId;
};
