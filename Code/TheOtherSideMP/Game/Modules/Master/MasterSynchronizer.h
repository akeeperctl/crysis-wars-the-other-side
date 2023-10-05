#pragma once

#include <IGameObject.h>

#include "MasterModule.h"

#include "../GenericSynchronizer.h"

struct MasterAddingParams
{
	EntityId entityId;
	string desiredSlaveClassName;

	MasterAddingParams() :
		entityId(0), desiredSlaveClassName(nullptr) {};

	explicit MasterAddingParams(const EntityId entId, const char* slaveClsName) :
		entityId(entId), desiredSlaveClassName(slaveClsName) {}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("entityId", entityId, 'eid');
		ser.Value("desiredSlaveClassName", desiredSlaveClassName, 'stab');
	}
};

struct MasterStartControlParams
{
	EntityId slaveId;

	MasterStartControlParams()
		: slaveId(0) {}

	explicit MasterStartControlParams(const EntityId entId, const char* slaveClsName)
		: slaveId(entId) {}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("slaveId", slaveId, 'eid');
	}
};

typedef MasterAddingParams DesiredSlaveClsParams;

/**
 * \brief TOS Master Module сетевой синхронизатор
 * \note Обеспечивает вызов RMI'шек, необходимых для сетевой синхронизации работы модуля
 */
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

	DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterAdd, MasterAddingParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvRequestSetDesiredSlaveCls, DesiredSlaveClsParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClMasterStartControl, MasterStartControlParams, eNRT_ReliableOrdered);
};