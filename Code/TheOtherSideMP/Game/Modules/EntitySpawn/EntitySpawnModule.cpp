// ReSharper disable CppLocalVariableMayBeConst
#include "StdAfx.h"
#include "EntitySpawnModule.h"

#include "Game.h"
#include "GameRules.h"
#include "IEntity.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

#include "TheOtherSideMP/Helpers/TOS_Entity.h"
#include "TheOtherSideMP/Helpers/TOS_STL.h"

#include "CryMemoryAllocator.h"
#include "CryMemoryManager.h"

TVecEntities CTOSEntitySpawnModule::s_markedForRecreation;
TMapDelayTOSParams CTOSEntitySpawnModule::s_scheduledSpawnsDelay;

CTOSEntitySpawnModule::CTOSEntitySpawnModule()
{}

CTOSEntitySpawnModule::~CTOSEntitySpawnModule()
{}

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
			auto pParams = new STOSEntitySpawnParams(*static_cast<STOSEntitySpawnParams*>(event.extra_data));
			assert(pParams);

			if (!HaveSavedParams(pEntity))
			{

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
				//just log
				TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_TOSEntityOnRemove, "", true));
			}

			//3
			if (MustBeRecreated(pEntity))
			{
				ScheduleRecreation(pEntity);
			}

			break;
		}
		case eEGE_OnServerStartRestarting:
		case eEGE_OnLevelLoadingStart:
		{
			Reset();
		}
		default:
			break;
	}
}

void CTOSEntitySpawnModule::Reset()
{
	Init();
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

	auto it = s_scheduledSpawnsDelay.begin();
	while (it != s_scheduledSpawnsDelay.end())
	{
		const int scheduledRecordId = it->first;
		_smart_ptr<STOSEntityDelaySpawnParams> pSpawnParams = it->second;

		assert(pSpawnParams.get() != NULL);

		const float curTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		const float recordedTime = pSpawnParams->scheduledTimeStamp;
		const float delay = pSpawnParams->spawnDelay;

		if (curTime - recordedTime > delay)
		{
			SpawnEntity(*pSpawnParams, true);

			s_scheduledSpawnsDelay.erase(it++);
			break;
		}
		else
		{
			++it;
		}
	}

	auto end = m_scheduledRecreations.end();

	for (auto it = m_scheduledRecreations.begin(); it != end;)
	{
		EntityId scheduledId = it->first;
		IEntity* pScheduledEnt = gEnv->pEntitySystem->GetEntity(scheduledId);
		STOSEntitySpawnParams* pScheduledParams = it->second;

		// Проверка на необходимость выполнить пересоздание
		if (!pScheduledEnt && (pScheduledParams->tosFlags & TOS_ENTITY_FLAG_SCHEDULED_RECREATION))
		{
			// После рекреации помечаем флагом, чтобы рекреация повторилась
			pScheduledParams->tosFlags = TOS_ENTITY_FLAG_MUST_RECREATED;

			IEntity* pRecreatedEntity = SpawnEntity(*pScheduledParams);
			assert(pRecreatedEntity);

			TOS_RECORD_EVENT(pRecreatedEntity->GetId(), STOSGameEvent(eEGE_TOSEntityRecreated, "", true));

			m_scheduledRecreations.erase(it++); // Удаление текущего элемента и переход к следующему
			break; // Выход из цикла после удаления элемента
		}
		else
		{
			++it; // Переход к следующему элементу, если условие не выполнено
		}
	}

	for (TMapAuthorityParams::iterator it = m_scheduledAuthorities.begin(); it != m_scheduledAuthorities.end();)
	{
		EntityId scheduledId = it->first;
		IEntity* pScheduledEnt = gEnv->pEntitySystem->GetEntity(scheduledId);

		if (!pScheduledEnt)
			break;

		const char* schedName = pScheduledEnt->GetName();
		const char* playerName = it->second.playerName.c_str();
		const bool forceStartControl = it->second.forceStartControl;

		IEntity* pPlayerEnt = gEnv->pEntitySystem->FindEntityByName(playerName);

		if (pPlayerEnt)
		{
			IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pPlayerEnt->GetId());
			assert(pGO);

			INetContext* pNetContext = g_pGame->GetIGameFramework()->GetNetContext();

			const uint16 playerChannelId = pGO->GetChannelId();
			INetChannel* pPlayerNetChannel = g_pGame->GetIGameFramework()->GetNetChannel(playerChannelId);
			bool isAuth = pNetContext->RemoteContextHasAuthority(pPlayerNetChannel, scheduledId);

			if (!isAuth)
			{
				pNetContext->DelegateAuthority(scheduledId, pPlayerNetChannel);
			}
			else
			{
				m_scheduledAuthorities.erase(it++);
				char buffer[256];
				sprintf(buffer, "%s take own of %s", playerName, schedName);
				TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_TOSEntityAuthorityDelegated, buffer, true));

				if (forceStartControl)
				{
					const uint16 masterChannelId = playerChannelId;
					TOS_RECORD_EVENT(scheduledId, STOSGameEvent(eEGE_ForceStartControl, "", true, false, nullptr, 0.0f, masterChannelId));
				}
			}

			break;
		}
		else
		{
			const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

			if (currentTime - it->second.scheduledTimeStamp > 10.0f)
			{
				m_scheduledAuthorities.erase(it++);
				break;
			}
			else
			{
				++it;
			}
		}
	}
}

void CTOSEntitySpawnModule::Serialize(TSerialize ser)
{}

IEntity* CTOSEntitySpawnModule::SpawnEntity(STOSEntitySpawnParams& params, bool sendTosEvent /*= true*/)
{
	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity spawning process only can be on the server");
	if (!gEnv->bServer)
		return nullptr;

	const auto pEntSys = gEnv->pEntitySystem;
	assert(pEntSys);
	if (!pEntSys)
		return nullptr;

	TMapTOSParams::iterator iter = g_pTOSGame->GetEntitySpawnModule()->m_savedSpawnParams.begin();
	TMapTOSParams::iterator end = g_pTOSGame->GetEntitySpawnModule()->m_savedSpawnParams.end();

	for (; iter != end; ++iter)
	{
		// Недопустимо чтобы 1 игрок-мастер мог управлять сразу двумя рабами

		bool alreadyHaveSaved = !params.authorityPlayerName.empty() && iter->second->authorityPlayerName == params.authorityPlayerName;
		if (alreadyHaveSaved)
		{
			CryLog("%s[C++][SpawnEntity] Slave entity spawn interrupted! The system already has a saved slave for player %s", TOS_COLOR_YELLOW, params.authorityPlayerName);

			return nullptr;
		}
	}


	const auto pEntity = pEntSys->SpawnEntity(params.vanilla, false);
	assert(pEntity);

	if (!params.savedName.empty())
		pEntity->SetName(params.savedName);

	gEnv->pEntitySystem->InitEntity(pEntity, params.vanilla);

	const EntityId entityId = pEntity->GetId();
	CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));

	IEntity* pAuthorityPlayerEnt = gEnv->pEntitySystem->FindEntityByName(params.authorityPlayerName);
	if (pAuthorityPlayerEnt)
	{
		g_pGame->GetGameRules()->MovePlayer(pActor, pAuthorityPlayerEnt->GetWorldPos(), Ang3(pAuthorityPlayerEnt->GetWorldRotation()));
	}


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

	TMapTOSParams::iterator savedIter = pSpawnModule->m_savedSpawnParams.begin();
	TMapTOSParams::iterator endIter = pSpawnModule->m_savedSpawnParams.end();
	for (; savedIter != endIter; ++savedIter)
	{
		// Недопустимо чтобы 1 игрок-мастер мог управлять сразу двумя рабами

		bool alreadyHaveSaved = savedIter->second->authorityPlayerName == params.authorityPlayerName;
		if (alreadyHaveSaved)
		{
			if (gEnv->pSystem->IsDevMode())
				CryLogAlways("%s[C++][SpawnEntityDelay] Warning!!! The system already has a saved slave(id:%i) for player %s",
							 TOS_COLOR_YELLOW, savedIter->first, params.authorityPlayerName);

			return false;
		}
	}

	// Спавнит сразу, если задержка очень маленькая
	if (params.spawnDelay < 0.001f)
	{
		return SpawnEntity(params, sendTosEvent);
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
	TMapTOSParams::const_iterator it = m_savedSpawnParams.begin();
	TMapTOSParams::const_iterator end = m_savedSpawnParams.end();

	for (; it != end; ++it)
	{
		const bool find = it->second->authorityPlayerName == authorityPlayerName && it->second->forceStartControl;

		if (find)
		{
			return TOS_GET_ENTITY(it->first);
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
	TMapTOSParams::const_iterator it = m_savedSpawnParams.begin();

	for (; it != m_savedSpawnParams.end(); ++it)
	{
		const EntityId id = it->first;
		const auto pEnt = gEnv->pEntitySystem->GetEntity(id);
		if (!pEnt)
			continue;

		const string& entName = pEnt->GetName();
		const string& savedName = it->second->vanilla.sName;

		const int index = TOS_STL::GetIndexFromMapKey(m_savedSpawnParams, id) + 1;

		float color[] = {1,1,1,1};

		gEnv->pRenderer->Draw2dLabel(
			screenPos.x,
			screenPos.y + index * interval,
			fontSize,
			color,
			false,
			"%i) %s:%s",
			index, savedName.c_str(), entName.c_str());
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
