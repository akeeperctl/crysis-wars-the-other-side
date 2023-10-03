// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
#include "StdAfx.h"
#include "TOSGameCvars.h"

#include "Game.h"
#include "IConsole.h"

#include "Modules/GenericSynchronizer.h"
#include "Modules/EntitySpawn/EntitySpawnModule.h"
#include "Modules/Master/MasterModule.h"

#include "TheOtherSideMP/Helpers/TOS_Entity.h"

#define ONLY_SERVER \
if (!gEnv->bServer)\
{\
	CryLogAlways("Failed: only on the server");\
	return;\
}\

#define ONLY_CLIENT \
if (!gEnv->bClient)\
{\
	CryLogAlways("Failed: only on the client");\
	return;\
}\


void STOSCvars::InitCVars(IConsole* pConsole)
{
	pConsole->Register("tos_debug_draw_aiactiontracker", &tos_debug_draw_aiactiontracker, 0, 0, "");
	pConsole->Register("tos_debug_log_aiactiontracker", &tos_debug_log_aiactiontracker, 0, 0, "");
	pConsole->Register("tos_debug_log_all", &tos_debug_log_all, 0, 0, "");
	pConsole->Register("tos_show_version", &tos_show_version, 1, 0, "");
}

void STOSCvars::InitCCommands(IConsole* pConsole) const
{
	//SERVER COMMANDS
	pConsole->AddCommand("netchname", CmdNetChName);
	pConsole->AddCommand("getmasterslist", CmdGetMastersList);
	pConsole->AddCommand("ismaster", CmdIsMaster);
	pConsole->AddCommand("spawnentity", CmdSpawnEntity);
	pConsole->AddCommand("removeentity", CmdRemoveEntity);
	pConsole->AddCommand("getentitiesbyclass", CmdGetEntitiesByClass);
	pConsole->AddCommand("getsyncs", CmdGetSyncs);
	pConsole->AddCommand("getentbyid", CmdGetEntityById);

	//CLIENT COMMANDS
	pConsole->AddCommand("getlocalname", CmdGetLocalName);
}

void STOSCvars::ReleaseCCommands() const
{
	const auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("netchname");
	pConsole->RemoveCommand("getlocalname");
	pConsole->RemoveCommand("getmasterslist");
	pConsole->RemoveCommand("ismaster");
}

void STOSCvars::ReleaseCVars() const
{
	const auto pConsole = gEnv->pConsole;

	pConsole->UnregisterVariable("tos_debug_draw_aiactiontracker", true);
	pConsole->UnregisterVariable("tos_debug_log_aiactiontracker", true);
	pConsole->UnregisterVariable("tos_debug_log_all", true);
	pConsole->UnregisterVariable("tos_show_version", true);
}

void STOSCvars::CmdNetChName(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	const char* playerEntityName = pArgs->GetArg(1);

	const auto pEntity = gEnv->pEntitySystem->FindEntityByName(playerEntityName);
	assert(pEntity);
	if (!pEntity)
		return;

	const auto pGO = g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId());
	assert(pGO);
	if (!pGO)
		return;

	const auto pChannel = pGO->GetNetChannel();
	assert(pChannel);
	if (!pChannel)
		return;

	CryLogAlways("Result: (%s|%s)", playerEntityName, pChannel->GetName());
}

void STOSCvars::CmdGetMastersList(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	std::map<EntityId, EntityId> masters;
	g_pTOSGame->GetMasterModule()->GetMasters(masters);

	CryLogAlways("Result: (master(id)|slave(id))");
	for (const auto& masterPair : masters)
	{
		const EntityId masterId = masterPair.first;
		const EntityId slaveId = masterPair.second;

		const auto pMasterEnt = gEnv->pEntitySystem->GetEntity(masterId);
		const auto pSlaveEnt = gEnv->pEntitySystem->GetEntity(slaveId);

		const char* masterName = pMasterEnt ? pMasterEnt->GetName() : "NULL";
		const char* slaveName = pSlaveEnt ? pSlaveEnt->GetName() : "NULL";

		CryLogAlways("	%s(%i)|%s(%i)", masterName, masterId, slaveName, slaveId);
	}
}

void STOSCvars::CmdIsMaster(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	const char* strPlayerId = pArgs->GetArg(1);
	const EntityId playerId = atoi(strPlayerId);

	const auto pEntity = gEnv->pEntitySystem->GetEntity(playerId);
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
	ONLY_SERVER;

	const auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pArgs->GetArg(1));
	assert(pClass);

	const string newEntName = pArgs->GetArg(2);
	const string plName = pArgs->GetArg(3);

	const auto pPlayerEntity = gEnv->pEntitySystem->FindEntityByName(plName.c_str());
	assert(pPlayerEntity);
	if (!pPlayerEntity)
	{
		CryLogAlways("Spawn failed: player entity %(s) not found", plName.c_str());
		return;
	}

	STOSEntitySpawnParams params;
	params.vanilla.bStaticEntityId = true;
	params.vanilla.pClass = pClass;
	//params.vanilla.sName = newEntName.c_str();
	params.savedName = newEntName.c_str();
	params.vanilla.vPosition = pPlayerEntity->GetWorldPos();
	params.tosFlags |= TOS_ENTITY_FLAG_MUST_RECREATED;

	TOS_Entity::Spawn(params);
}

void STOSCvars::CmdRemoveEntity(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	const char* entName = pArgs->GetArg(1);

	const auto pEntity = gEnv->pEntitySystem->FindEntityByName(entName);
	assert(pEntity);
	if (!pEntity)
	{
		CryLogAlways("Remove failed: entity %s not found", entName);
		return;
	}

	gEnv->pEntitySystem->RemoveEntity(pEntity->GetId(), true);
}

void STOSCvars::CmdGetEntityById(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	const EntityId id = atoi(pArgs->GetArg(1));
	const EntityId playerId = atoi(pArgs->GetArg(2));

	const auto pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (!pEntity)
	{
		CryLogAlways("Failed: wrong 1 arg entityId");
		return;
	}

	const auto pPlayer = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId);
	if (!pPlayer)
	{
		CryLogAlways("Failed: wrong 2 arg playerId");
		return;
	}

	const auto playerChannelId = pPlayer->GetChannelId();
	const auto pPlayerChannel = g_pGame->GetIGameFramework()->GetNetChannel(playerChannelId);
	const auto isAuth = g_pGame->GetIGameFramework()->GetNetContext()->RemoteContextHasAuthority(pPlayerChannel, id);

	const char* strName = pEntity ? pEntity->GetName() : "NULL";
	const char* strAuth = isAuth ? "True" : "False";

	CryLogAlways("Result: ");
	CryLogAlways("	Name: %s", strName);
	CryLogAlways("	Authority: %s", strAuth);

	TOS_Debug::LogEntityFlags(pEntity);
}

void STOSCvars::CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER;

	CryLogAlways("Result: (entity_name|entity_id)");

	const auto iter = gEnv->pEntitySystem->GetEntityIterator();
	while (!iter->IsEnd())
	{
		const auto pEntity = iter->Next();
		if (!pEntity)
			continue;

		const string name = pEntity->GetName();
		string clsname = pEntity->GetClass()->GetName();
		string argName = pArgs->GetArg(1);
		if (clsname == argName) //"TOSMasterSynchronizer")
			CryLogAlways("	%s|%i", name.c_str(), pEntity->GetId());
	}
}

void STOSCvars::CmdGetSyncs(IConsoleCmdArgs* pArgs)
{
	CryLogAlways("Result: (name|id)");

	TSynches syncs;
	CTOSGenericSynchronizer::GetSynchonizers(syncs);

	for (const auto& syncPair : syncs)
	{
		const char* name = syncPair.first;
		const auto pEntity = gEnv->pEntitySystem->FindEntityByName(name);
		const auto id = syncPair.second;

		CryLogAlways("	%s|%i", name, id);
	}
}

void STOSCvars::CmdGetLocalName(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT;

	const auto pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
	assert(pPlayer);

	CryLogAlways("Result: (%s)", pPlayer->GetEntity()->GetName());
}

