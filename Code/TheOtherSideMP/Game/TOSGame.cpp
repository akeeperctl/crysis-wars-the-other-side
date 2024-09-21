/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "TOSGame.h"

#include "Game.h"
#include "NanoSuit.h"
#include "TOSGameEventRecorder.h"

#include "Modules/ITOSGameModule.h"
#include "Modules/EntitySpawn/EntitySpawnModule.h"
#include "Modules/Master/MasterClient.h"
#include "Modules/Master/MasterModule.h"

#include "TheOtherSideMP/AI/AITrackerModule.h"
#include "TheOtherSideMP/Helpers/TOS_Cache.h"
#include "TheOtherSideMP/Helpers/TOS_Script.h"

CTOSGame::CTOSGame()
	: m_pAITrackerModule(nullptr),
	m_pLocalControlClient(nullptr),
	m_pEventRecorder(nullptr),
	m_pMasterModule(nullptr),
	m_pEntitySpawnModule(nullptr),
	m_lastChannelConnectionState(0),
	m_lastContextViewState(0)
{}

CTOSGame::~CTOSGame()
{
	SAFE_DELETE(m_pEventRecorder);

	//Modules

	SAFE_DELETE(m_pMasterModule);
	SAFE_DELETE(m_pEntitySpawnModule);
	SAFE_DELETE(m_pZeusModule);

	//~Modules

	delete this;
}

void CTOSGame::Init()
{
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->RegisterListener(this);
	if (gEnv->pInput)
		gEnv->pInput->AddEventListener(this);
	if (gEnv->pEntitySystem)
		gEnv->pEntitySystem->AddSink(this);

	g_pGame->GetIGameFramework()->GetILevelSystem()->AddListener(this);

	m_pEventRecorder = new CTOSGameEventRecorder();

	//if (gEnv->bServer)
	{
		m_pAITrackerModule = new СTOSAIModule();
	}

	//Modules

	m_pZeusModule = new CTOSZeusModule();
	m_pEntitySpawnModule = new CTOSEntitySpawnModule();
	m_pMasterModule = new CTOSMasterModule();

	//~Modules

	// Исправление бага https://github.com/akeeperctl/crysis-wars-the-other-side/issues/8
	g_pGameCVars->hud_enableAlienInterference = 0;

	for (std::vector<ITOSGameModule*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->Init();
		}
	}
}

void CTOSGame::Shutdown()
{
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->UnregisterListener(this);
	if (gEnv->pInput)
		gEnv->pInput->RemoveEventListener(this);
	if (gEnv->pEntitySystem)
		gEnv->pEntitySystem->RemoveSink(this);
	g_pGame->GetIGameFramework()->GetILevelSystem()->RemoveListener(this);


	this->~CTOSGame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void CTOSGame::Update(const float frameTime, int frameId)
{
	UpdateChannelConnectionState();
	UpdateContextViewState();

	for (std::vector<ITOSGameModule*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->Update(frameTime);
		}
	}
}

void CTOSGame::InitScriptBinds()
{
	for (std::vector<ITOSGameModule*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->InitScriptBinds();
		}
	}

	m_pCustomScriptBind = new CScriptBind_Custom(gEnv->pSystem, g_pGame->GetIGameFramework());
}

void CTOSGame::ReleaseScriptBinds()
{
	for (std::vector<ITOSGameModule*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		ITOSGameModule* pModule = *it;
		if (pModule)
		{
			pModule->ReleaseScriptBinds();
		}
	}

	SAFE_DELETE(m_pCustomScriptBind);
}

void CTOSGame::OnLevelNotFound(const char* levelName)
{

}

void CTOSGame::OnLoadingStart(ILevelInfo* pLevel)
{
	m_pMasterModule->Reset();

	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_OnLevelLoadingStart, pLevel ? pLevel->GetName() : "", true));
}

void CTOSGame::CacheAssets()
{
	//Пришельцы
	TOS_Cache::CacheObject("Objects/Characters/Alien/trooper/Trooper.chr");
	TOS_Cache::CacheObject("Objects/Characters/Alien/trooper/trooper_leader.chr");
	TOS_Cache::CacheObject("Objects/Characters/Alien/scout/scout_base.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/scout/scout_leader.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/hunter/Hunter.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Alien/AlienBase/AlienBase.cdf");

	//Нанокостюмы
	TOS_Cache::CacheObject("Objects/Characters/Human/US/NanoSuit/nanosuit_us.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Human/US/NanoSuit/nanosuit_us_fp3p.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Human/US/NanoSuit/nanosuit_us_multiplayer.cdf");
	TOS_Cache::CacheObject("objects/weapons/arms_global/arms_nanosuit_us.chr");
	CNanoSuit::PrecacheMaterials(false);

	TOS_Cache::CacheObject("Objects/Characters/Human/Asian/NanoSuit/nanosuit_asian.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Human/Asian/NanoSuit/nanosuit_asian_fp3p.cdf");
	TOS_Cache::CacheObject("Objects/Characters/Human/Asian/NanoSuit/nanosuit_asian_multiplayer.cdf");
	TOS_Cache::CacheObject("objects/weapons/arms_global/arms_nanosuit_asian.chr");
	CNanoSuit::PrecacheMaterials(true);

	//Прочие объекты
	TOS_Cache::CacheObject("objects/effects/tracer_standard_new.cgf");
	TOS_Cache::CacheObject("objects/effects/tracer_standard_red_new.cgf");
	TOS_Cache::CacheMaterial("objects/effects/tracer_standard.mtl");

	TOS_Cache::CacheObject("objects/characters/human/asian/nk_soldier/nk_soldier_frozen_scatter.cgf");
}

void CTOSGame::OnLoadingComplete(ILevel* pLevel)
{
	//Кешируем модели объектов, чтобы при их спавне не было подгрузок/подлагиваний
	CacheAssets();
}

void CTOSGame::OnLoadingError(ILevelInfo* pLevel, const char* error)
{}

void CTOSGame::OnLoadingProgress(ILevelInfo* pLevel, int progressAmount)
{}

CTOSGameEventRecorder* CTOSGame::GetEventRecorder() const
{
	return m_pEventRecorder;
}

CTOSMasterModule* CTOSGame::GetMasterModule() const
{
	return m_pMasterModule;
}

CTOSZeusModule* CTOSGame::GetZeusModule() const
{
	return m_pZeusModule;
}

CTOSEntitySpawnModule* CTOSGame::GetEntitySpawnModule() const
{
	return m_pEntitySpawnModule;
}

СTOSAIModule* CTOSGame::GetAITrackerModule() const
{
	return m_pAITrackerModule;
}

bool CTOSGame::ModuleAdd(ITOSGameModule* pModule, const bool flowGraph)
{
	auto& modules = flowGraph == true ? m_flowgraphModules : m_modules;

	return stl::push_back_unique(modules, pModule);
}

bool CTOSGame::ModuleRemove(ITOSGameModule* pModule, const bool flowGraph)
{
	auto& modules = flowGraph == true ? m_flowgraphModules : m_modules;

	return stl::find_and_erase(modules, pModule);
}

void CTOSGame::UpdateChannelConnectionState()
{
	const auto pNetChannel = g_pGame->GetIGameFramework()->GetClientChannel();
	if (!pNetChannel)
		return;

	const uint currentState = pNetChannel->GetChannelConnectionState();

	if (m_lastChannelConnectionState != currentState)
	{
		m_lastChannelConnectionState = currentState;

		const char* state = "<unknown state>";
		switch (currentState)
		{
			case eCCS_StartingConnection:
				state = "eCCS_StartingConnection";
				break;
			case eCCS_InContextInitiation:
				state = "eCCS_InContextInitiation";
				break;
			case eCCS_InGame:
				state = "eCCS_InGame";
				break;
			case eCCS_Disconnecting:
				state = "eCCS_Disconnecting";
				break;
			default:
				break;
		}

		TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_UpdateChannelConnectionState, state, true));
	}
}
void CTOSGame::UpdateContextViewState()
{
	const auto pNetChannel = g_pGame->GetIGameFramework()->GetClientChannel();
	if (!pNetChannel)
		return;

	const uint currentState = pNetChannel->GetContextViewState();

	if (m_lastContextViewState != currentState)
	{
		m_lastContextViewState = currentState;

		const char* state = "<unknown state>";
		switch (currentState)
		{
			case eCVS_Initial:
				state = "eCVS_Initial: Requesting Game Environment";
				break;
			case eCVS_Begin:
				state = "eCVS_Begin: Receiving Game Environment";
				break;
			case eCVS_EstablishContext:
				state = "eCVS_EstablishContext: Loading Game Assets";
				break;
			case eCVS_ConfigureContext:
				state = "eCVS_ConfigureContext: Configuring Game Settings";
				break;
			case eCVS_SpawnEntities:
				state = "eCVS_SpawnEntities: Spawning Entities";
				break;
			case eCVS_PostSpawnEntities:
				state = "eCVS_PostSpawnEntities: Initializing Entities";
				break;
			case eCVS_InGame:
				state = "eCVS_InGame: In Game";
				break;
			case eCVS_NUM_STATES:
			default:   // NOLINT(clang-diagnostic-covered-switch-default)
				break;
		}

		TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_UpdateContextViewState, state, true));
	}

}

IActor* CTOSGame::GetActualClientActor() const
{
	const auto pMC = m_pMasterModule->GetMasterClient();
	if (!pMC)
		return nullptr;

	CTOSActor* pPlayer = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	CTOSActor* pSlave = pMC->GetSlaveActor();

	if (pSlave != nullptr)
		return pSlave;

	//assert(pPlayer);
	return pPlayer;
}
