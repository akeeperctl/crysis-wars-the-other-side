#include "StdAfx.h"
#include "TOSGameCvars.h"
#include "TOSGameEventRecorder.h"
#include "ITOSGameModule.h"
#include "TOSGame.h"

CTOSGame::CTOSGame()
{
	m_pEventRecorder = new CTOSGameEventRecorder();
}

CTOSGame::~CTOSGame()
{
	SAFE_DELETE(m_pEventRecorder);
}

void CTOSGame::Init()
{
	//m_pEventRecorder->RecordEvent(nullptr, STOSGameEvent(eEGE_TOSGame_Init, "", 0.0f, 0, true));
}

void CTOSGame::Shutdown()
{
	delete this;
}

void CTOSGame::Update(float frameTime, int frameId)
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