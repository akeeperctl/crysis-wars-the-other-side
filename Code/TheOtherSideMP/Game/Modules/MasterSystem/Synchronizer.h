#pragma once

#include <IGameObject.h>
#include "TheOtherSideMP/Game/Modules/MasterSystem/MasterModule.h"
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
	virtual void PostInit(IGameObject* pGameObject);
	//virtual void PostInitClient(int channelId);
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	//virtual void PostUpdate(float frameTime) {};
	//virtual void PostRemoteSpawn() {};
	virtual void HandleEvent(const SGameObjectEvent&);
	virtual void ProcessEvent(SEntityEvent&);
	//virtual void SetChannelId(uint16 id) {}
	//virtual void SetAuthority(bool auth);
	virtual void GetMemoryStatistics(ICrySizer* s);
	//~IGameObjectExtension

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterAdd, MasterAddingParams, eNRT_ReliableOrdered);

protected:

private:
};