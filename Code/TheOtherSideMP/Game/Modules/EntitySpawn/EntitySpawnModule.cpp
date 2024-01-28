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
TMapDelayTOSParams CTOSEntitySpawnModule::s_scheduledSpawnsDelay;

CTOSEntitySpawnModule::CTOSEntitySpawnModule()
{
}

CTOSEntitySpawnModule::~CTOSEntitySpawnModule()
{
}

void CTOSEntitySpawnModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	if (!gEnv->bServer)
		return;

	switch (event.event)
	{
	case eEGE_TOSEntityScheduleDelegateAuthority:
	{
		const char* playerName = event.description;

		auto it = m_scheduledAuthorities.find(entId);
		if (it == m_scheduledAuthorities.end())
		{
			m_scheduledAuthorities[entId].forceStartControl = static_cast<bool>(event.int_value);
			m_scheduledAuthorities[entId].playerName = playerName;
			m_scheduledAuthorities[entId].scheduledTimeStamp = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		}

		break;
	}
	case eEGE_TOSEntityOnSpawn:
	{
		//2
		if (!HaveSavedParams(pEntity))
		{
			auto pParams = new STOSEntitySpawnParams(*static_cast<STOSEntitySpawnParams*>(event.extra_data));
			assert(pParams);

			// id должен генерироваться
			pParams->vanilla.id = 0;

			//pParams->sName = pEntity->GetName();

			if (gEnv->pSystem->IsDevMode())
				CryLogAlways("[%s|%s|%id] Create saved params ", pParams->savedName, pEntity->GetName(), entId);

			m_savedSpawnParams[entId] = pParams;
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
	m_savedSpawnParams.clear();
	s_markedForRecreation.clear();
	m_scheduledAuthorities.clear();
	s_scheduledSpawnsDelay.clear();
}

void CTOSEntitySpawnModule::Update(float frametime)
{
	//DebugDraw(Vec2(20, 70), 1.2f, 1.0f, 5, true);

	if (!gEnv->bServer)
		return;

	for (auto &schedPair : s_scheduledSpawnsDelay)
	{
		const int scheduledRecordId = schedPair.first;
		auto pSpawnParams = schedPair.second;
		assert(pSpawnParams);

		const float curTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		const float recordedTime = pSpawnParams->scheduledTimeStamp;
		const float delay = pSpawnParams->spawnDelay;

		if (curTime - recordedTime > delay)
		{
			// Обновим актуальные координаты мастера в записи сущности, если таковой мастер имеется.
			// Это нужно для того, чтобы когда сущность респавнилась она была в одной позиции с её мастером
			auto pAuthorityPlayerEnt = gEnv->pEntitySystem->FindEntityByName(pSpawnParams->authorityPlayerName);
			if (pAuthorityPlayerEnt)
			{
				pSpawnParams->vanilla.vPosition = pAuthorityPlayerEnt->GetWorldPos();
				pSpawnParams->vanilla.qRotation = pAuthorityPlayerEnt->GetWorldRotation();
			}

			SpawnEntity(*pSpawnParams, true);

			s_scheduledSpawnsDelay.erase(scheduledRecordId);
			break;
		}
	}

	for (auto &schedPair : m_scheduledRecreations)
	{
		EntityId scheduledId = schedPair.first;
		IEntity* pScheduledEnt = gEnv->pEntitySystem->GetEntity(scheduledId);
		STOSEntitySpawnParams* pScheduledParams = schedPair.second;

		// Проверка на необходимость выполнить пересоздание
		if (!pScheduledEnt && (pScheduledParams->tosFlags & TOS_ENTITY_FLAG_SCHEDULED_RECREATION))
		{
			//После рекреации помечаем флагом, чтобы рекреация повторилась
			pScheduledParams->tosFlags = TOS_ENTITY_FLAG_MUST_RECREATED;

			// Обнаружен баг. В переменной pScheduledParams.vanilla исчезает имя сущности sName
			// Имя исчезает, когда указатель pScheduledEnt становится nullptr!
			// Баг исправлен

			// Применение сохраненной таблицы к сущности
			// 04/10/2023 Пока что оставим без применения сохраненной таблицы

			//auto pSavedScriptTable = pScheduledParams->pSavedScript;
			//if (pSavedScriptTable)
			//{
			//	SmartScriptTable props;
			//	SmartScriptTable instanceProps;

			//	if (pSavedScriptTable->GetValue("Properties", props) &&
			//		pSavedScriptTable->GetValue("PropertiesInstance", instanceProps))
			//	{
			//		pScheduledParams->vanilla.pPropertiesTable = props;
			//		pScheduledParams->vanilla.pPropertiesInstanceTable = instanceProps;
			//	}
			//}

			auto pRecreatedEntity = SpawnEntity(*pScheduledParams);
			assert(pRecreatedEntity);

			TOS_RECORD_EVENT(pRecreatedEntity->GetId(), STOSGameEvent(eEGE_TOSEntityRecreated, "", true));

			m_scheduledRecreations.erase(scheduledId);
			//CryLogAlways("[%s|%id] name getted from vanilla params", pScheduledParams->vanilla.sName, pRecreatedEntity->GetId());
			//CryLogAlways("[%s|%id] Remove from scheduled recreations map ", "NULL",scheduledId);
			break;
		}
	}

	for (auto& schedPair : m_scheduledAuthorities)
	{
		EntityId scheduledId = schedPair.first;
		IEntity* pScheduledEnt = gEnv->pEntitySystem->GetEntity(scheduledId);

		// Как-то раз словил вылет при sv_restart во время перезапуска :)
		if (!pScheduledEnt)
			break;

		const char* schedName = pScheduledEnt->GetName();
		const char* playerName = schedPair.second.playerName.c_str();
		const bool forceStartControl = schedPair.second.forceStartControl;

		const auto pPlayerEnt = gEnv->pEntitySystem->FindEntityByName(playerName);
		//assert(pPlayerEnt);

		if (pPlayerEnt)
		{
			auto pGO = g_pGame->GetIGameFramework()->GetGameObject(pPlayerEnt->GetId());
			assert(pGO);

			auto pNetContext = g_pGame->GetIGameFramework()->GetNetContext();

			const auto playerChannelId = pGO->GetChannelId();
			const auto pPlayerNetChannel = g_pGame->GetIGameFramework()->GetNetChannel(playerChannelId);
			auto isAuth = pNetContext->RemoteContextHasAuthority(pPlayerNetChannel, scheduledId);

			if (!isAuth)
			{
				//CryLogAlways("[%s] Try delegate authority to player %s", schedName, playerName);
				pNetContext->DelegateAuthority(scheduledId, pPlayerNetChannel);
			}
			else
			{
				m_scheduledAuthorities.erase(scheduledId);

				char buffer[256];
				sprintf(buffer, "%s take own of %s", playerName, schedName);

				TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_TOSEntityAuthorityDelegated, buffer, true));

				if (forceStartControl)
				{
					const auto masterChannelId = playerChannelId;
					TOS_RECORD_EVENT(scheduledId, STOSGameEvent(eEGE_ForceStartControl, "", true, false, nullptr, 0.0f, masterChannelId));
				}
			}

			break;
			//isAuth = pNetContext->RemoteContextHasAuthority(pPlayerNetChannel, entityId);

			//if (isAuth)
			//{
			//	CryLogAlways("[%s] Server delegate authority to this entity to player with name %s", pEntity->GetName(), pPlayerEnt->GetName());
			//}
			//else
			//{
			//	CryLogAlways("[%s] Server NOT delegate authority of this entity to player with name %s", pEntity->GetName(), pPlayerEnt->GetName());
			//}
		}
		// ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
		else
		{
			const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			// если запланированной передачи власти не было более 10 секунд, то удаляем пару
			if (currentTime - schedPair.second.scheduledTimeStamp > 10.0f)
			{
				m_scheduledAuthorities.erase(scheduledId);
				break;
			}
		}
	}

	// вызывает удаление раба после sv_restart
	//for (auto& savedPair : m_savedSpawnParams)
	//{
	//	const EntityId savedId = savedPair.first;
	//	const char* playerName = savedPair.second->authorityPlayerName.c_str();

	//	const auto pPlayerEnt = gEnv->pEntitySystem->FindEntityByName(playerName);
	//	if (!pPlayerEnt)
	//	{
	//		m_savedSpawnParams.erase(savedId);
	//		RemoveEntityForced(savedId);
	//		break;
	//	}
	//}
}

void CTOSEntitySpawnModule::Serialize(TSerialize ser)
{
}

IEntity* CTOSEntitySpawnModule::SpawnEntity(STOSEntitySpawnParams& params, bool sendTosEvent /*= true*/)
{
	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity spawning process only can be on the server");
	if (!gEnv->bServer)
		return nullptr;

	const auto pEntSys = gEnv->pEntitySystem;
	assert(pEntSys);
	if (!pEntSys)
		return nullptr;

	for (auto& iter : g_pTOSGame->GetEntitySpawnModule()->m_savedSpawnParams)
	{
		// Недопустимо чтобы 1 игрок-мастер мог управлять сразу двумя рабами

		bool alreadyHaveSaved = iter.second->authorityPlayerName == params.authorityPlayerName;
		if (alreadyHaveSaved)
		{
			CryLogAlwaysDev("%s[C++][SpawnEntity] Slave entity spawn interrupted! The system already has a saved slave for player %s", TOS_COLOR_YELLOW, params.authorityPlayerName);

			return nullptr;
		}
	}


	const auto pEntity = pEntSys->SpawnEntity(params.vanilla, false);
	assert(pEntity);

	pEntity->SetName(params.savedName);
	const EntityId entityId = pEntity->GetId();
	
	gEnv->pEntitySystem->InitEntity(pEntity, params.vanilla);

	//1
	if (sendTosEvent)
		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityOnSpawn, "", true, false, &params));

	if (params.tosFlags & TOS_ENTITY_FLAG_MUST_RECREATED)
	{

		auto alreadyInside = stl::find(s_markedForRecreation, entityId);
		if (!alreadyInside)
		{
			s_markedForRecreation.push_back(entityId);
			TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityMarkForRecreation, "", true));
		}
	}

	// Планирование передачи игроку власти на сущность
	// Осуществление самой передачи происходит тогда, когда указатель на игрока будет валидным
	if (!params.authorityPlayerName.empty())
	{
		const char* plName = params.authorityPlayerName;
		const bool forceStartControl = params.forceStartControl;

		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityScheduleDelegateAuthority, plName, true, false, nullptr, 0.0f, forceStartControl));
	}


	return pEntity;
}

bool CTOSEntitySpawnModule::SpawnEntityDelay(STOSEntityDelaySpawnParams& params, bool sendTosEvent /*= true*/)
{
	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity spawning process only can be on the server");
	if (!gEnv->bServer)
		return false;

	const auto pEntSys = gEnv->pEntitySystem;
	assert(pEntSys);
	if (!pEntSys)
		return false;

	auto pSpawnModule = g_pTOSGame->GetEntitySpawnModule();

	for (auto& savedIter : pSpawnModule->m_savedSpawnParams)
	{
		// Недопустимо чтобы 1 игрок-мастер мог управлять сразу двумя рабами

		bool alreadyHaveSaved = savedIter.second->authorityPlayerName == params.authorityPlayerName;
		if (alreadyHaveSaved)
		{
			if (gEnv->pSystem->IsDevMode())
				CryLogAlways("%s[C++][SpawnEntityDelay] Warning!!! The system already has a saved slave(id:%i) for player %s", TOS_COLOR_YELLOW, savedIter.first, params.authorityPlayerName);
			//CryLogAlways("%s[C++][SpawnEntityDelay] The system already has a saved slave for player %s -> force old deletion...", TOS_COLOR_YELLOW, params.authorityPlayerName);

			//RemoveEntityForced(savedIter.first);


			return false;
		}
	}

	if (params.spawnDelay < 0.001f)
	{
		return SpawnEntity(params,sendTosEvent);
	}

	// Мы не можем проверить наличие этой записи в map, потому что у нас нет идентификатора записи.
	// Поэтому просто добавим следующую запись.
	auto delayedParams = new STOSEntityDelaySpawnParams(params);

	const int curRecordId = static_cast<int>(s_scheduledSpawnsDelay.size());
	s_scheduledSpawnsDelay[curRecordId] = delayedParams;

	return true;
}

void CTOSEntitySpawnModule::RemoveEntityForced(EntityId id)
{
	// Обманка для компилятора, чтобы я мог в static методе использовать не static переменные
	auto pSM = g_pTOSGame->GetEntitySpawnModule();
	assert(pSM);

	stl::find_and_erase(s_markedForRecreation, id);

	if (pSM->m_scheduledAuthorities.find(id) != pSM->m_scheduledAuthorities.end())
		pSM->m_scheduledAuthorities.erase(id);

	if (pSM->m_scheduledRecreations.find(id) != pSM->m_scheduledRecreations.end())
		pSM->m_scheduledRecreations.erase(id);

	if (pSM->m_savedSpawnParams.find(id) != pSM->m_savedSpawnParams.end())
		pSM->m_savedSpawnParams.erase(id);

	TOS_RECORD_EVENT(id, STOSGameEvent(eEGE_EntityRemovedForced, "", true));

	gEnv->pEntitySystem->RemoveEntity(id);
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

IEntity* CTOSEntitySpawnModule::GetSavedSlaveByAuthName(const char* authorityPlayerName) const
{
	for (auto &savedPair : m_savedSpawnParams)
	{
		const auto &spawnParams = savedPair.second;
		const bool find = spawnParams->authorityPlayerName == authorityPlayerName && spawnParams->forceStartControl;

		if (find)
		{
			return TOS_GET_ENTITY(savedPair.first);
		}
	}

	return nullptr;
}

bool CTOSEntitySpawnModule::HaveSavedParams(const IEntity* pEntity) const
{
	assert(pEntity);
	if (!pEntity)
		return false;

	return m_savedSpawnParams.find(pEntity->GetId()) != m_savedSpawnParams.end();
}

void CTOSEntitySpawnModule::DebugDraw(const Vec2& screenPos, float fontSize, float interval, int maxElemNum, bool draw) const
{
	if (!draw)
		return;

	//Header
	DRAW_2D_TEXT(
		screenPos.x,
		screenPos.y - interval * 2,
		fontSize + 0.2f,
		"--- TOS Entity Spawn Module (savedName|realName) ---");

	//Body
	for (const auto& ppair : m_savedSpawnParams)
	{
		const auto pEnt = gEnv->pEntitySystem->GetEntity(ppair.first);
		if (!pEnt)
			continue;

		string entName = pEnt->GetName();
		string savedName = ppair.second->vanilla.sName;

		const int index = TOS_STL::GetIndexFromMapKey(m_savedSpawnParams, ppair.first) + 1;

		float color[] = { 1,1,1,1 };


		gEnv->pRenderer->Draw2dLabel(
			screenPos.x,
			screenPos.y + index * interval,
			fontSize,
			color,
			false,
			"%i) %s:%s",
			index, savedName, entName);

		//DRAW_2DTEXT(
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

	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity scheduling process only can be on the server");
	if (!gEnv->bServer)
		return;

	assert(pEntity);
	if (!pEntity)
		return;

	const auto entId = pEntity->GetId();
	const auto entName = pEntity->GetName();

	auto it = m_scheduledRecreations.find(entId);
	bool alreadyScheduled = it != m_scheduledRecreations.end();
	assert(!alreadyScheduled);

	if (alreadyScheduled)
		return;

	auto pParams = new STOSEntitySpawnParams();
	auto pSavedScript = gEnv->pScriptSystem->CreateTable(false);

	if (pSavedScript->Clone(pEntity->GetScriptTable()))
	{
		//CryLogAlways("[%s] Script table successfully cloned", entName);
		//CryLogAlways("[%s] Script dump:", entName);
		//pSavedScript->Dump(g_pTOSGame);
	}

	pParams->pSavedScript = pSavedScript;
	pParams->tosFlags |= TOS_ENTITY_FLAG_SCHEDULED_RECREATION;
	pParams->vanilla = m_savedSpawnParams[entId]->vanilla;

	pParams->savedName = entName;
	pParams->authorityPlayerName = m_savedSpawnParams[entId]->authorityPlayerName;
	pParams->forceStartControl = m_savedSpawnParams[entId]->forceStartControl;

	// Здесь, в переменной pParams.vanilla имя sName присутствует
	m_scheduledRecreations[entId] = pParams;

	//CryLogAlways("BEFORE SAVED DELETION sName = %s", pParams->savedName);

	stl::find_and_erase(s_markedForRecreation, entId);

	auto it2 = m_savedSpawnParams.find(entId);
	if (it2 != m_savedSpawnParams.end())
	{
		m_savedSpawnParams.erase(entId);

		//CryLogAlways("AFTER SAVED DELETION sName = %s", pParams->savedName);
		//CryLogAlways("[%s|%id] Remove saved params", pEntity->GetName(), entId);
	}
}
