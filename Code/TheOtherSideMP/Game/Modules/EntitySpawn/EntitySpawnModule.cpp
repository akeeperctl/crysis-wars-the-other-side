// ReSharper disable CppLocalVariableMayBeConst
#include "StdAfx.h"
#include "EntitySpawnModule.h"

#include "Game.h"
#include "IEntity.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

#include "TheOtherSideMP/Helpers/TOS_Entity.h"
#include "TheOtherSideMP/Helpers/TOS_STL.h"

TVecEntities CTOSEntitySpawnModule::s_markedForRecreation;

CTOSEntitySpawnModule::CTOSEntitySpawnModule()
{
}

CTOSEntitySpawnModule::~CTOSEntitySpawnModule()
{
}

void CTOSEntitySpawnModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	switch (event.event)
	{
	case eEGE_TOSEntityOnSpawn:
	{
		//2
		if (!HaveSavedParams(pEntity))
		{
			auto pParams = new SEntitySpawnParams(*static_cast<SEntitySpawnParams*>(event.extra_data));
			assert(pParams);

			// id должен генерироваться
			pParams->id = 0;

			//pParams->sName = pEntity->GetName();

			//CryLogAlways("[%s|%s|%id] Create saved params ", pParams->sName, pEntity->GetName(), entId);

			m_savedParams[entId] = pParams;
		}

		break;
	}

	case eEGE_EntityOnRemove:
	{
		if (HaveSavedParams(pEntity))
		{
			TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_TOSEntityOnRemove, "", true));
		}

		//3
		if (MustBeRecreated(pEntity))
		{
			ScheduleRecreation(pEntity);
		}

		break;
	}
	default: 
		break;
	}
}

void CTOSEntitySpawnModule::Init()
{
	CTOSGenericModule::Init();

	m_scheduledRecreations.clear();

	for (auto savedPair : m_savedParams)
		delete savedPair.second;

	m_savedParams.clear();
	s_markedForRecreation.clear();
}

void CTOSEntitySpawnModule::Update(float frametime)
{
	//DebugDraw(Vec2(20, 70), 1.2f, 1.0f, 5, true);

	for (auto &schedPair : m_scheduledRecreations)
	{
		EntityId scheduledId = schedPair.first;
		IEntity* pScheduledEnt = gEnv->pEntitySystem->GetEntity(scheduledId);
		STOSEntitySpawnParams* pScheduledParams = schedPair.second;

		if (pScheduledEnt)
		{
			CryLogAlways("");
		}

		// Проверка на необходимость выполнить пересоздание
		if (!pScheduledEnt && (pScheduledParams->tosFlags & TOS_ENTITY_FLAG_SCHEDULED_RECREATION))
		{
			//После рекреации помечаем флагом, чтобы рекреация повторилась
			pScheduledParams->tosFlags = TOS_ENTITY_FLAG_MUST_RECREATED;

			// Обнаружен баг. В переменной pScheduledParams.vanilla исчезает имя сущности sName
			// Имя исчезает, когда указатель pScheduledEnt становится nullptr!

			auto pRecreatedEntity = SpawnEntity(*pScheduledParams);
			assert(pRecreatedEntity);

			TOS_RECORD_EVENT(pRecreatedEntity->GetId(), STOSGameEvent(eEGE_TOSEntityRecreated, "", true));

			m_scheduledRecreations.erase(scheduledId);
			//CryLogAlways("[%s|%id] name getted from vanilla params", pScheduledParams->vanilla.sName, pRecreatedEntity->GetId());
			//CryLogAlways("[%s|%id] Remove from scheduled recreations map ", "NULL",scheduledId);
			break;
		}
	}
}

void CTOSEntitySpawnModule::Serialize(TSerialize ser)
{
}

IEntity* CTOSEntitySpawnModule::SpawnEntity(STOSEntitySpawnParams& params, bool sendTosEvent)
{
	const auto pEntSys = gEnv->pEntitySystem;
	assert(pEntSys);
	if (!pEntSys)
		return nullptr;

	const auto pEntity = pEntSys->SpawnEntity(params.vanilla);
	assert(pEntity);

	pEntity->SetName(params.savedName);

	//1
	const EntityId entityId = pEntity->GetId();

	if (sendTosEvent)
		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityOnSpawn, params.savedName, true, false, &params.vanilla));

	if (params.tosFlags & TOS_ENTITY_FLAG_MUST_RECREATED)
	{
		auto alreadyInside = stl::find(s_markedForRecreation, entityId);
		if (!alreadyInside)
		{
			s_markedForRecreation.push_back(entityId);
			TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityMarkForRecreation, params.savedName, true));
		}
	}

	return pEntity;
}

bool CTOSEntitySpawnModule::MustBeRecreated(const IEntity* pEntity) const
{
	assert(pEntity);
	if (!pEntity)
		return false;

	auto entId = pEntity->GetId();
	auto result = stl::find(s_markedForRecreation, entId) && HaveSavedParams(pEntity);

	return result;
}

bool CTOSEntitySpawnModule::HaveSavedParams(const IEntity* pEntity) const
{
	assert(pEntity);
	if (!pEntity)
		return false;

	return m_savedParams.find(pEntity->GetId()) != m_savedParams.end();
}

void CTOSEntitySpawnModule::DebugDraw(const Vec2& screenPos, float fontSize, float interval, int maxElemNum, bool draw)
{
	if (!draw)
		return;

	//Header
	TOS_Debug::Draw2dText(
		screenPos.x,
		screenPos.y - interval * 2,
		fontSize + 0.2f,
		"--- TOS Entity Spawn Module (savedName|realName) ---");

	//Body
	for (const auto& ppair : m_savedParams)
	{
		const auto pEnt = gEnv->pEntitySystem->GetEntity(ppair.first);
		if (!pEnt)
			continue;

		string entName = pEnt->GetName();
		string savedName = ppair.second->sName;

		const int index = TOS_STL::GetIndexFromMapKey(m_savedParams, ppair.first) + 1;

		float color[] = { 1,1,1,1 };


		gEnv->pRenderer->Draw2dLabel(
			screenPos.x,
			screenPos.y + index * interval,
			fontSize,
			color,
			false,
			"%i) %s:%s",
			index, savedName, entName);

		//TOS_Debug::Draw2dText(
		//	screenPos.x,
		//	screenPos.y + index * interval,
		//	fontSize,
		//	"%i) %s:%s",
		//	index, savedName, entName);
	}

}

void CTOSEntitySpawnModule::ScheduleRecreation(const IEntity* pEntity)
{
	// До вызова этой функции дожно быть выполнено
	// 1) запись с параметрами спавна pEntity в m_savedParams
	// 2) entityId сущности pEntity должен быть в s_markedForRecreation
	// Или
	// 1) MustBeRecreated(pEntity) должен вернуть True

	assert(pEntity);
	if (!pEntity)
		return;

	const auto entId = pEntity->GetId();

	auto it = m_scheduledRecreations.find(entId);
	bool alreadyScheduled = it != m_scheduledRecreations.end();
	assert(!alreadyScheduled);

	if (alreadyScheduled)
		return;

	auto pParams = new STOSEntitySpawnParams();
	auto pSavedScript = gEnv->pScriptSystem->CreateTable(false);

	pSavedScript->Clone(pEntity->GetScriptTable());

	pParams->pSavedScript = pSavedScript;
	pParams->tosFlags |= TOS_ENTITY_FLAG_SCHEDULED_RECREATION;
	pParams->vanilla = *m_savedParams[entId];

	// Излишние строки для проверки сохраняемости имени
	//pParams->vanilla.sName = pEntity->GetName();
	pParams->savedName = pEntity->GetName();

	// Здесь, в переменной pParams.vanilla имя sName присутствует
	m_scheduledRecreations[entId] = pParams;

	//CryLogAlways("BEFORE SAVED DELETION sName = %s", pParams->savedName);

	stl::find_and_erase(s_markedForRecreation, entId);

	auto it2 = m_savedParams.find(entId);
	if (it2 != m_savedParams.end())
	{
		delete it2->second;
		m_savedParams.erase(entId);

		//CryLogAlways("AFTER SAVED DELETION sName = %s", pParams->savedName);
		//CryLogAlways("[%s|%id] Remove saved params", pEntity->GetName(), entId);
	}
}
