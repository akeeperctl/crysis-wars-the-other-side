#pragma once

#include <IGameObject.h>
#include "TheOtherSideMP/Game/Modules/Master/MasterModule.h"
#include "TheOtherSideMP/Game/Modules/GenericSynchronizer.h"


// Description: 
//    Network synchronizer for Master Module.
//    Can send RMI's to synchronize Master Module states.
class CTOSMasterSynchronizer : public CTOSGenericSynchronizer
{
public:
	CTOSMasterSynchronizer();
	~CTOSMasterSynchronizer();

	// IGameObjectExtension
	//virtual bool Init(IGameObject* pGameObject);
	//virtual void InitClient(int channelId);
	void PostInit(IGameObject* pGameObject) override;
	// void PostInitClient(int channelId);
	void Release() override;
	void FullSerialize(TSerialize ser) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	// void PostUpdate(float frameTime) {};
	// void PostRemoteSpawn() {};
	void HandleEvent(const SGameObjectEvent&) override;
	void ProcessEvent(SEntityEvent&) override;
	// void SetChannelId(uint16 id) {}
	// void SetAuthority(bool auth);
	void GetMemoryStatistics(ICrySizer* s) override;
	//~IGameObjectExtension

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterAdd, MasterAddingParams, eNRT_ReliableOrdered);

protected:

private:
};