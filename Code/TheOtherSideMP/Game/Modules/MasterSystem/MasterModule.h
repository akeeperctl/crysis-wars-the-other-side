#pragma once

#include "TheOtherSideMP/Game/Modules/ITOSGameModule.h"
#include "TheOtherSideMP/Game/TOSGame.h"

class CTOSMasterRMISender;

/**
 * TOS Master Info
 * На сервере: хранит информацию о работе одного мастера
 * На клиенте: отсутсвует.
 * Включено автоудаление.
 */
//struct STOSMasterInfo
//{
//public:
//	STOSMasterInfo() :
//		id(0),
//		slaveId(0)
//	{}
//
//	STOSMasterInfo(EntityId _id, EntityId _slaveId) :
//		id(_id),
//		slaveId(_slaveId)
//	{}
//
//	~STOSMasterInfo()
//	{
//		delete this;
//	}
//
//	EntityId id;
//	EntityId slaveId;
//};

/**
 * TOS Master System
 * На сервере: хранит информацию о работе всех мастеров.
 * На клиенте: ничего не делает.
 * Мастер - это игрок, управляющий несвоим актёром.
 * Автоудаление: отсутствует.
 */
class CTOSMasterModule : ITOSGameModule
{
public:
	CTOSMasterModule();
	~CTOSMasterModule();

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	void GetMemoryStatistics(ICrySizer* s);
	const char* GetName() {return "CTOSMasterSystem";};
	void Init();
	void Update(float frametime);
	void Serialize(TSerialize ser);
	//ITOSGameModule

	void MasterAdd(const IEntity* pMasterEntity);
	void MasterRemove(const IEntity* pMasterEntity);
	bool IsMaster(const IEntity* pMasterEntity);
	void GetMasters(std::map<EntityId, EntityId>& masters);

	IEntity* GetSlave(const IEntity* pMasterEntity);

	void DebugDrawMasters(const Vec2& screenPos = {20,300}, float fontSize = 1.2f, float interval = 20.0f, int maxElemNum = 5);


	CTOSMasterRMISender* GetRMISender() const;

private:
	std::map<EntityId, EntityId> m_masters;
	CTOSMasterRMISender* m_pRMISender;

};