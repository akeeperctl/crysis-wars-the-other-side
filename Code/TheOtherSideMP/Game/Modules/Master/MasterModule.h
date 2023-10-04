#pragma once

#include "../GenericModule.h"

#include "../../TOSGame.h"

#include "TheOtherSideMP/TOSSmartStruct.h"

class CTOSMasterSynchronizer;
class CTOSGenericSynchronizer;

/**
 * \brief хранит информацию о работе одного мастера
 */
struct STOSMasterInfo : STOSSmartStruct  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSMasterInfo() :
		slaveId(0)
	{}

	explicit STOSMasterInfo(const EntityId _slaveId) :
		slaveId(_slaveId)
	{}

	EntityId slaveId;
	string desiredSlaveClassName;
};

/**
 * TOS Master System
 * На сервере: хранит информацию о работе всех мастеров.
 * На клиенте: ничего не делает.
 * Мастер - это игрок, управляющий несвоим актёром.
 * Автоудаление: отсутствует.
 */
class CTOSMasterModule : public CTOSGenericModule  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSMasterModule();
	~CTOSMasterModule() override;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override {return "CTOSMasterModule";};
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//ITOSGameModule

	void MasterAdd(const IEntity* pMasterEntity);
	void MasterRemove(const IEntity* pMasterEntity);
	bool IsMaster(const IEntity* pMasterEntity);
	void GetMasters(std::map<EntityId, EntityId>& masters) const;
	IEntity* GetSlave(const IEntity* pMasterEntity);
	void DebugDraw(const Vec2& screenPos = {20,300}, float fontSize = 1.2f, float interval = 20.0f, int maxElemNum = 5);

private:
	std::map<EntityId, EntityId> m_masters;

};