#pragma once

#include "Actor.h"

class CTOSActor: public CActor  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSActor();
	~CTOSActor() override;


	void PostInit( IGameObject * pGameObject ) override;
	void InitClient(int channelId ) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void Release() override;

	void SetMasterEntityId(EntityId id);
	EntityId GetMasterEntityId() const {return m_masterEntityId;}
	void SetSlaveEntityId(EntityId id);
	EntityId GetSlaveEntityId() const { return m_slaveEntityId;}

private:

	EntityId m_slaveEntityId;
	EntityId m_masterEntityId;
};