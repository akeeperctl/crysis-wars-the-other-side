#pragma once

#include <IGameObject.h>

#include "MasterModule.h"

#include "../GenericSynchronizer.h"


// Description: 
//    Network synchronizer for Master Module.
//    Can send RMI's to synchronize Master Module states.
class CTOSMasterSynchronizer final : public CTOSGenericSynchronizer  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSMasterSynchronizer();
	~CTOSMasterSynchronizer() override;

	// IGameObjectExtension
	void PostInit(IGameObject* pGameObject) override;
	void Release() override;
	void FullSerialize(TSerialize ser) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void HandleEvent(const SGameObjectEvent&) override;
	void ProcessEvent(SEntityEvent&) override;
	void GetMemoryStatistics(ICrySizer* s) override;
	//~IGameObjectExtension

	const char* GetClassName() override {return "CTOSMasterSynchronizer";}

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterAdd, MasterAddingParams, eNRT_ReliableOrdered);

protected:

private:
};