#pragma once

#include "Actor.h"

class CTOSActor: public CActor
{
public:
	CTOSActor();
	virtual ~CTOSActor();

	virtual void PostInit( IGameObject * pGameObject );
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);

	void SetMasterEntityId(EntityId id);
	const EntityId GetMasterEntityId() {return m_masterEntityId;};
	void SetSlaveEntityId(EntityId id);
	const EntityId GetSlaveEntityId() { return m_slaveEntityId;};

private:

	EntityId m_slaveEntityId;
	EntityId m_masterEntityId;
};