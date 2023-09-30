#include "StdAfx.h"

#include "IConsole.h"

#include "TOSGameCvars.h"
#include "Modules/MasterSystem/MasterModule.h"
#include "Modules/GenericSynchronizer.h"
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
	pConsole->AddCommand("spawnentity", CmdSpawnEntity);
	pConsole->AddCommand("removeentity", CmdRemoveEntity);
	pConsole->AddCommand("getentitiesbyclass", CmdGetEntitiesByClass);
	pConsole->AddCommand("getsync", CmdGetSync);

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
	{
		CryLogAlways("IsMaster failed: not found entity with id (%i)", strPlayerId);
		return;
	}
		

	const bool isMaster = g_pTOSGame->GetMasterModule()->IsMaster(pEntity);
	const char* result = isMaster ? "Yes" : "No";

	CryLogAlways("Result: (%i|%s)", playerId, result);
}

void STOSCvars::CmdSpawnEntity(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("Spawn failed: can spawn only on the server");
		return;
	}

	auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pArgs->GetArg(1));
	assert(pClass);

	const char* plName = pArgs->GetArg(2);

	auto pPlayerEntity = gEnv->pEntitySystem->FindEntityByName(plName);
	assert(pPlayerEntity);
	if (!pPlayerEntity)
	{
		CryLogAlways("Spawn failed: player entity %(s) not found", plName);
		return;
	}

	SEntitySpawnParams params;
	params.bStaticEntityId = true;
	params.pClass = pClass;
	params.sName = "spawned_1";
	params.vPosition = pPlayerEntity->GetWorldPos();
	params.nFlags |= ENTITY_FLAG_UNREMOVABLE;

	gEnv->pEntitySystem->SpawnEntity(params);
}

void STOSCvars::CmdRemoveEntity(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("Remove failed: can remove only on the server");
		return;
	}

	const char* entName = pArgs->GetArg(1);

	auto pEntity = gEnv->pEntitySystem->FindEntityByName(entName);
	assert(pEntity);
	if (!pEntity)
	{
		CryLogAlways("Remove failed: entity %s not found", entName);
		return;
	}

	pEntity->ClearFlags(ENTITY_FLAG_UNREMOVABLE);
	gEnv->pEntitySystem->RemoveEntity(pEntity->GetId());
}

void STOSCvars::CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bServer)
	{
		CryLogAlways("Failed: only on the server");
		return;
	}

	CryLogAlways("Result: (entity_name|entity_id)");

	auto iter = gEnv->pEntitySystem->GetEntityIterator();
	while(!iter->IsEnd())
	{
		auto pEntity = iter->Next();
		if (!pEntity)
			continue;

		string name = pEntity->GetName();
		string clsname = pEntity->GetClass()->GetName();
		string argName = pArgs->GetArg(1);
		if (clsname == argName) //"TOSMasterSynchronizer")
			CryLogAlways("	%s|%i", name, pEntity->GetId());
	}
}

void STOSCvars::CmdGetSync(IConsoleCmdArgs* pArgs)
{
	auto pSynchronizer = g_pTOSGame->GetMasterModule()->GetSynchronizer();

	const char* name = pSynchronizer ? pSynchronizer->GetEntity()->GetName() : "NULL";

	CryLogAlways("Result: %s", name);
}

void STOSCvars::CmdGetLocalName(IConsoleCmdArgs* pArgs)
{
	if (!gEnv->bClient)
		return;

	auto pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
	assert(pPlayer);

	CryLogAlways("Result: (%s)", pPlayer->GetEntity()->GetName());
}

