#include "StdAfx.h"
#include "TOSGame.h"

#include "Game.h"
#include "TOSGameEventRecorder.h"

#include "Modules/ITOSGameModule.h"
#include "Modules/Master/MasterModule.h"
#include "Modules/RestartResurrection/RestartResurrectionModule.h"

CTOSGame::CTOSGame():
	m_pAIActionTracker(nullptr),
	m_pLocalControlClient(nullptr),
	m_pEventRecorder(nullptr),
	m_pMasterModule(nullptr),
	m_pResurrectionModule(nullptr)
{
}

CTOSGame::~CTOSGame()
{
	SAFE_DELETE(m_pEventRecorder);

	//Modules

	SAFE_DELETE(m_pMasterModule);
	SAFE_DELETE(m_pResurrectionModule);

	//~Modules

	delete this;
}

void CTOSGame::Init()
{
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->RegisterListener(this);
	if (gEnv->pInput) 
		gEnv->pInput->AddEventListener(this);

	m_pEventRecorder = new CTOSGameEventRecorder();

	//Modules

	m_pMasterModule = new CTOSMasterModule();
	m_pResurrectionModule = new CTOSRestartResurrectionModule();

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

bool CTOSGame::ModuleAdd(ITOSGameModule* pModule, bool flowGraph)
{
	auto& modules = flowGraph == true ? m_flowgraphModules : m_modules;

	return stl::push_back_unique(modules, pModule);
}

bool CTOSGame::ModuleRemove(ITOSGameModule* pModule, bool flowGraph)
{
	auto& modules = flowGraph == true ? m_flowgraphModules : m_modules;

	return stl::find_and_erase(modules, pModule);
}
