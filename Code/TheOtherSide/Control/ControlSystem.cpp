#include <StdAfx.h>

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"

#include "Player.h"

#include "HUD/HUD.h"
#include "HUD/HUDCrosshair.h"

SControlSystem* g_pControlSystem = new SControlSystem();

SControlSystem::SControlSystem()
	: pControlClient(0),
	pSquadSystem(nullptr),
	pAbilitiesSystem(nullptr),
	isEnabled(false)
{
	//CryLogAlways("[SControlSystem] Costructor");
};
SControlSystem::~SControlSystem()
{
	SAFE_DELETE(pControlClient);
	SAFE_DELETE(pSquadSystem);
	SAFE_DELETE(pAbilitiesSystem);
	isEnabled = 0;
	//CryLogAlways("[SControlSystem] Destructor");
};

void SControlSystem::Start()
{
	if (!pControlClient)
		return;

	if (isEnabled == false)
	{
		pControlClient->InitDudeToControl(true);
		isEnabled = true;
	}
}

void SControlSystem::Stop()
{
	if (!pControlClient)
		return;

	if (isEnabled == true)
	{
		pControlClient->InitDudeToControl(false);
		ResetSystem();
	}
}

void SControlSystem::ResetSystem()
{
	isEnabled = false;
	if (pControlClient)
		pControlClient->Reset();
}

void SControlSystem::RegisterControlClient(SControlClient* _pControlClient)
{
	pControlClient = _pControlClient;
}

void SControlSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	if (pControlClient)
		pControlClient->GetMemoryStatistics(s);

	if (pAbilitiesSystem)
		pAbilitiesSystem->GetMemoryStatistics(s);

	if (pSquadSystem)
		pSquadSystem->GetMemoryStatistics(s);
}