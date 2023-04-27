#pragma once

#include "Actor.h"

class CStrategicArea;
class CSpawnPoint;

//typedef _smart_ptr<CSpawnPoint> CSpawnPointPtr;

class CSpawnPoint
{
public:
	CSpawnPoint(IEntity* pEntity, CStrategicArea* pArea);

	friend class CStrategicArea;

	void Update(float frametime);
	void GetMemoryStatistics(ICrySizer* s);

	inline int GetIndex() { return m_index; }; //from 0..
	inline void SetIndex(int idx) { m_index = idx; }; //from 0..

	inline IEntity* GetEntity() { return gEnv->pEntitySystem->GetEntity(m_entityId); };
	inline EntityId GetEntityId() { return m_entityId; };

	inline Vec3 GetPosition() { return GetEntity()->GetWorldPos(); };
	inline Quat GetRotation() { return GetEntity()->GetRotation(); };

	void SetRecentlySpawned(int value, float restTime = 0);
	inline bool IsRecentlySpawned() { return m_recentlySpawned; };
	inline bool IsForAir() { return m_isForAirUnits; };

	inline CStrategicArea* GetArea() { return m_pArea; };

	void SpawnActor(IActor* pActor);

	//virtual void AddRef() const { ++m_refs; };
	//virtual uint GetRefCount() const { return m_refs; };
	//virtual void Release() const
	//{
	//	if (--m_refs <= 0)
	//		delete this;
	//}

protected:
	//mutable uint	m_refs;
private:

	CStrategicArea* m_pArea;

	int m_index; //from 0..
	EntityId m_entityId;
	int m_recentlySpawned;
	bool m_isForAirUnits;
	float m_recentlySpawnedTimer;
};