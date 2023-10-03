#include "StdAfx.h"
#include "TOSGame.h"

#include "Game.h"
#include "TOSGameEventRecorder.h"

#include "Modules/ITOSGameModule.h"
#include "Modules/Master/MasterModule.h"
#include "Modules/EntitySpawn/EntitySpawnModule.h"

CTOSGame::CTOSGame():
	m_pAIActionTracker(nullptr),
	m_pLocalControlClient(nullptr),
	m_pEventRecorder(nullptr),
	m_pMasterModule(nullptr),
	m_pEntitySpawnModule(nullptr)
{
}

CTOSGame::~CTOSGame()
{
	SAFE_DELETE(m_pEventRecorder);

	//Modules

	SAFE_DELETE(m_pMasterModule);
	SAFE_DELETE(m_pEntitySpawnModule);

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

	m_pEventRecorder = new CTOSGameEventRecorder();

	//Modules

	m_pEntitySpawnModule = new CTOSEntitySpawnModule();
	m_pMasterModule = new CTOSMasterModule();

	//~Modules

	for (ITOSGameModule* pModule : m_modules)
	{
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


	this->~CTOSGame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void CTOSGame::Update(const float frameTime, int frameId)
{
	for (ITOSGameModule* pModules : m_modules)
	{
		if (pModules)
		{
			pModules->Update(frameTime);
		}
	}
}

CTOSGameEventRecorder* CTOSGame::GetEventRecorder() const
{
	return m_pEventRecorder;
}

CTOSMasterModule* CTOSGame::GetMasterModule() const
{
	return m_pMasterModule;
}

CTOSEntitySpawnModule* CTOSGame::GetEntitySpawnModule() const
{
	return m_pEntitySpawnModule;
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
