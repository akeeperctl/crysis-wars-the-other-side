// ReSharper disable CppLocalVariableMayBeConst
#include "StdAfx.h"
#include "EntitySpawnModule.h"

#include "Game.h"
#include "IEntity.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

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
			m_scheduledAuthorities[entId].playerName = playerName;
			m_scheduledAuthorities[entId].scheduledTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
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
	m_savedParams.clear();
	s_markedForRecreation.clear();
	m_scheduledAuthorities.clear();
}

void CTOSEntitySpawnModule::Update(float frametime)
{
	//DebugDraw(Vec2(20, 70), 1.2f, 1.0f, 5, true);

	if (!gEnv->bServer)
		return;

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

		const char* schedName = pScheduledEnt->GetName();
		const char* playerName = schedPair.second.playerName.c_str();

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
				CryLogAlways("[%s] Try delegate authority to player %s", schedName, playerName);
				pNetContext->DelegateAuthority(scheduledId, pPlayerNetChannel);
			}
			else
			{
				m_scheduledAuthorities.erase(scheduledId);

				char buffer[256];
				sprintf(buffer, "%s take own of %s", playerName, schedName);

				TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_TOSEntityAuthorityDetegated, buffer, true));
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
			if (currentTime - schedPair.second.scheduledTime > 10.0f)
			{
				m_scheduledAuthorities.erase(scheduledId);
			}
		}
	}
}

void CTOSEntitySpawnModule::Serialize(TSerialize ser)
{
}

IEntity* CTOSEntitySpawnModule::SpawnEntity(STOSEntitySpawnParams& params, const bool sendTosEvent)
{
	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity spawning process only can be on the server");
	if (!gEnv->bServer)
		return nullptr;

	const auto pEntSys = gEnv->pEntitySystem;
	assert(pEntSys);
	if (!pEntSys)
		return nullptr;

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
		TOS_RECORD_EVENT(entityId, STOSGameEvent(eEGE_TOSEntityScheduleDelegateAuthority, params.authorityPlayerName.c_str(), true));
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
		string savedName = ppair.second->vanilla.sName;

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

	CRY_ASSERT_MESSAGE(gEnv->bServer, "Entity scheduling process only can be on the server");
	if (!gEnv->bServer)
		return;

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
	pParams->vanilla = m_savedParams[entId]->vanilla;

	pParams->savedName = pEntity->GetName();
	pParams->authorityPlayerName = m_savedParams[entId]->authorityPlayerName;

	// Здесь, в переменной pParams.vanilla имя sName присутствует
	m_scheduledRecreations[entId] = pParams;

	//CryLogAlways("BEFORE SAVED DELETION sName = %s", pParams->savedName);

	stl::find_and_erase(s_markedForRecreation, entId);

	auto it2 = m_savedParams.find(entId);
	if (it2 != m_savedParams.end())
	{
		m_savedParams.erase(entId);

		//CryLogAlways("AFTER SAVED DELETION sName = %s", pParams->savedName);
		//CryLogAlways("[%s|%id] Remove saved params", pEntity->GetName(), entId);
	}
}
