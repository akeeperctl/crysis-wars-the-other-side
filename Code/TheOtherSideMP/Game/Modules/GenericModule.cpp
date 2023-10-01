#include "StdAfx.h"
#include "GenericModule.h"
#include "GenericSynchronizer.h"

#include "../TOSGame.h"
#include "../TOSGameEventRecorder.h"

#include "Game.h"

CTOSGenericModule::CTOSGenericModule()
{
	m_pSynchonizer = nullptr;
	g_pTOSGame->ModuleAdd(this, false);
}

CTOSGenericModule::~CTOSGenericModule()
{
	g_pTOSGame->ModuleRemove(this, false);
}

void CTOSGenericModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	TOS_INIT_EVENT_VALUES(pEntity, event);

	switch (event.event)
	{
	case eGE_GameReset:
		m_pSynchonizer = nullptr;
		break;
	default:
		break;
	}
}

void CTOSGenericModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(this);
}

void CTOSGenericModule::Init()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_GameModuleInit, GetName(), true));
}

void CTOSGenericModule::Update(float frametime)
{
}

void CTOSGenericModule::Serialize(TSerialize ser)
{
}

CTOSGenericSynchronizer* CTOSGenericModule::GetSynchronizer() const
{
	return m_pSynchonizer;
}

CTOSGenericSynchronizer* CTOSGenericModule::CreateSynchonizer(const char* entityName, const char* extensionName)
{
	//IEntity* pSynchronizer = gEnv->pEntitySystem->FindEntityByName("MasterSynchronizer");
	if (m_pSynchonizer)
	{
		IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(m_pSynchonizer->GetEntityId());
		if (pGO)
		{
			m_pSynchonizer = dynamic_cast<CTOSGenericSynchronizer*>(pGO->AcquireExtension(extensionName));
			assert(m_pSynchonizer);

			//m_pSynchonizer->SetModule(this);
		}

		return m_pSynchonizer;
	}

	//auto pSynchronizerCls = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TOSMasterSynchronizer");
	//assert(pSynchronizerCls);

	//if (!pSynchronizerCls)
	//	return;

	SEntitySpawnParams params;
	params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	params.bStaticEntityId = true;
	params.sName = entityName;
	//params.nFlags |= ENTITY_FLAG_NO_PROXIMITY;
	//���� ENTITY_FLAG_UNREMOVABLE �� �������� ��� sv_restart
	params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;
	//params.id = 2;

	IEntity* pSynchEntity = gEnv->pEntitySystem->SpawnEntity(params);
	assert(pSynchEntity);

	if (!pSynchEntity)
		return nullptr;

	//IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchronizer->GetId());
	IGameObject* pGO = g_pGame->GetIGameFramework()->GetIGameObjectSystem()->CreateGameObjectForEntity(pSynchEntity->GetId());
	if (pGO)
	{
		m_pSynchonizer = dynamic_cast<CTOSGenericSynchronizer*>(pGO->AcquireExtension(extensionName));
		assert(m_pSynchonizer);

		//m_pSynchonizer->SetModule(this);

		m_pSynchonizer->GetGameObject()->ForceUpdate(true);
	}

	return m_pSynchonizer;
}
