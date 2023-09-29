#include "StdAfx.h"

#include "IConsole.h"

#include "TOSGameCvars.h"
#include "Modules/MasterSystem/MasterModule.h"
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
	//SERVER COMMANDS
	pConsole->AddCommand("netchname", CmdNetChName);
	pConsole->AddCommand("getmasterslist", CmdGetMastersList);
	pConsole->AddCommand("ismaster", CmdIsMaster);
	pConsole->AddCommand("spawntrooper", CmdSpawnTrooper);
	pConsole->AddCommand("removeentity", CmdRemoveEntity);

	//CLIENT COMMANDS
	pConsole->AddCommand("getlocalname", CmdGetLocalName);
}
	


void STOSCvars::ReleaseCCommands()
{
	auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("netchname");
	pConsole->RemoveCommand("getlocalname");
	pConsole->RemoveCommand("getmasterslist");
	pConsole->RemoveCommand("ismaster");
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

void STOSCvars::CmdGetMastersList(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
		return;

	std::map<EntityId, EntityId> masters;
	g_pTOSGame->GetMasterModule()->GetMasters(masters);

	CryLogAlways("Result: (master(id)|slave(id))");
	for (auto& masterPair : masters)
	{
		const int masterId = masterPair.first;
		const int slaveId = masterPair.second;

		auto pMasterEnt = gEnv->pEntitySystem->GetEntity(masterId);
		auto pSlaveEnt = gEnv->pEntitySystem->GetEntity(slaveId);

		const char* masterName = pMasterEnt ? pMasterEnt->GetName() : "NULL";
		const char* slaveName = pSlaveEnt ? pSlaveEnt->GetName() : "NULL";

		CryLogAlways("	%s(%i)|%s(%i)", masterName, masterId, slaveName, slaveId);
	}
}

void STOSCvars::CmdIsMaster(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
		return;

	const char* strPlayerId = pArgs->GetArg(1);
	const EntityId playerId = atoi(strPlayerId);

	auto pEntity = gEnv->pEntitySystem->GetEntity(playerId);
	assert(pEntity);
	if (!pEntity)
		return;

	const bool isMaster = g_pTOSGame->GetMasterModule()->IsMaster(pEntity);
	const char* result = isMaster ? "Yes" : "No";

	CryLogAlways("Result: (%i|%s)", playerId, result);
}

void STOSCvars::CmdSpawnTrooper(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("can spawn only on the server");
		return;
	}

	auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Trooper");
	assert(pClass);

	auto pPlayerEntity = gEnv->pEntitySystem->FindEntityByName(pArgs->GetArg(1));
	assert(pPlayerEntity);

	SEntitySpawnParams params;
	params.bStaticEntityId = true;
	params.pClass = pClass;
	params.sName = "spawned_1";
	params.vPosition = pPlayerEntity->GetWorldPos();

	gEnv->pEntitySystem->SpawnEntity(params);
}

void STOSCvars::CmdRemoveEntity(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("can remove only on the server");
		return;
	}

	auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Trooper");
	assert(pClass);

	auto pEntity = gEnv->pEntitySystem->FindEntityByName(pArgs->GetArg(1));
	assert(pEntity);

	gEnv->pEntitySystem->RemoveEntity(pEntity->GetId());
}

void STOSCvars::CmdGetLocalName(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bClient)
		return;

	auto pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
	assert(pPlayer);

	CryLogAlways("Result: (%s)", pPlayer->GetEntity()->GetName());
}

