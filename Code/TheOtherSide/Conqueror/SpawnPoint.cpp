#include "StdAfx.h"
#include "IEntitySystem.h"

#include "Actor.h"
#include "StrategicArea.h"
#include "SpawnPoint.h"

CSpawnPoint::CSpawnPoint(IEntity* pEntity, CStrategicArea* pArea)
{
	m_entityId = pEntity->GetId();
	m_recentlySpawnedTimer = 0;
	m_recentlySpawned = 0;
	m_isForAirUnits = false;
	m_pArea = pArea;

	m_index = 0;
}

void CSpawnPoint::Update(float frametime)
{
	if (m_recentlySpawned)
	{
		if (m_recentlySpawnedTimer > 0.0f)
			m_recentlySpawnedTimer -= frametime;

		if (m_recentlySpawnedTimer <= 0.0f)
		{
			m_recentlySpawnedTimer = 0.0f;
			m_recentlySpawned = 0;
		}
	}
}

void CSpawnPoint::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CSpawnPoint::SetRecentlySpawned(int value, float restTime /*= 0*/)
{
	m_recentlySpawned = value;
	m_recentlySpawnedTimer = 5.0f;

	if (restTime != 0)
		m_recentlySpawnedTimer = restTime;
}

void CSpawnPoint::SpawnActor(IActor* pActor)
{
	if (!GetEntity())
		return;

	if (pActor)
	{
		auto& pointPos = GetPosition();
		auto& pointRot = GetRotation();

		Matrix34 actorMatrix34 = pActor->GetEntity()->GetWorldTM();
		actorMatrix34.SetTranslation(pointPos);

		pActor->GetEntity()->SetWorldTM(actorMatrix34);
		pActor->GetEntity()->SetRotation(pointRot);

		SetRecentlySpawned(true);
	}
}
