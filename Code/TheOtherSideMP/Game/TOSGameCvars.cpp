#include "StdAfx.h"

#include "IConsole.h"

#include "TOSGameCvars.h"
#include "../Actors/Player/TOSPlayer.h"

#include "Game.h"
#include "GameCVars.h"


void STOSCvars::InitCVars(IConsole *pConsole)
{
	pConsole->Register("tos_debug_draw_aiactiontracker", &tos_debug_draw_aiactiontracker, 0, 0, "");
	pConsole->Register("tos_debug_log_aiactiontracker", &tos_debug_log_aiactiontracker, 0, 0, "");
	pConsole->Register("tos_debug_log_all", &tos_debug_log_all, 0, 0, "");
	pConsole->Register("tos_show_version", &tos_show_version, 1, 0, "");

}

void STOSCvars::InitCCommands(IConsole* pConsole)
{
	pConsole->AddCommand("netchname", CmdNetChName);
	pConsole->AddCommand("getlocalname", CmdGetLocalName);
}

void STOSCvars::ReleaseCCommands()
{
	auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("netchname");
}

void STOSCvars::ReleaseCVars()
{
	auto pConsole = gEnv->pConsole;

	pConsole->UnregisterVariable("tos_debug_draw_aiactiontracker", true);
	pConsole->UnregisterVariable("tos_debug_log_aiactiontracker", true);
	pConsole->UnregisterVariable("tos_debug_log_all", true);
	pConsole->UnregisterVariable("tos_show_version", true);
}

void STOSCvars::CmdNetChName(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
		return;

	const char* playerEntityName = pArgs->GetArg(1);

	auto pEntity = gEnv->pEntitySystem->FindEntityByName(playerEntityName);
	assert(pEntity);
	if (!pEntity)
		return;

	auto pGO = g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId());
	assert(pGO);
	if (!pGO)
		return;

	auto pChannel = pGO->GetNetChannel();
	assert(pChannel);
	if (!pChannel)
		return;

	CryLogAlways("Result: (%s|%s)", playerEntityName, pChannel->GetName());
}

void STOSCvars::CmdGetLocalName(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bClient)
		return;

	auto pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
	assert(pPlayer);

	CryLogAlways("Result: (%s)", pPlayer->GetEntity()->GetName());
}
