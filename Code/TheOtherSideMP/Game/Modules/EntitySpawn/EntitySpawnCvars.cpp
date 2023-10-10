#include "StdAfx.h"
#include "EntitySpawnModule.h"

#include "TheOtherSideMP/Game/TOSGameCvars.h"

void CTOSEntitySpawnModule::InitCVars(IConsole* pConsole)
{
	CTOSGenericModule::InitCVars(pConsole);
}

void CTOSEntitySpawnModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	pConsole->AddCommand("spawnentity", CmdSpawnEntity);
	pConsole->AddCommand("removeentity", CmdRemoveEntityById);
	pConsole->AddCommand("removeentityforced", CmdRemoveEntityByIdForced);
	pConsole->AddCommand("getlistsavedentites", CmdGetListEntities);
}

void CTOSEntitySpawnModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();
}
void CTOSEntitySpawnModule::ReleaseCCommands()
{
	CTOSGenericModule::ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("spawnentity");
	pConsole->RemoveCommand("removeentitybyid");
	pConsole->RemoveCommand("removeentitybyidforced");
	pConsole->RemoveCommand("getlistsavedentites");
}

void CTOSEntitySpawnModule::CmdSpawnEntity(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

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
	params.authorityPlayerName = plName;
	params.vanilla.bStaticEntityId = true;
	params.vanilla.pClass = pClass;
	params.savedName = newEntName.c_str();
	params.vanilla.vPosition = pPlayerEntity->GetWorldPos();
	params.vanilla.qRotation = pPlayerEntity->GetWorldRotation();
	params.tosFlags |= TOS_ENTITY_FLAG_MUST_RECREATED;

	CTOSEntitySpawnModule::SpawnEntity(params);
}

void CTOSEntitySpawnModule::CmdRemoveEntityById(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	const EntityId id = atoi(pArgs->GetArg(1));

	const auto pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (!pEntity)
	{
		CryLogAlways("Failed: wrong 1 arg entityId");
		return;
	}

	gEnv->pEntitySystem->RemoveEntity(id);
}

void CTOSEntitySpawnModule::CmdRemoveEntityByIdForced(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	const EntityId id = atoi(pArgs->GetArg(1));

	const auto pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (!pEntity)
	{
		CryLogAlways("Failed: wrong 1 arg entityId");
		return;
	}

	RemoveEntityForced(id);
}

void CTOSEntitySpawnModule::CmdGetListEntities(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	const auto pModule = g_pTOSGame->GetEntitySpawnModule();
	assert(pModule);

	CryLogAlways("Result: ");
	CryLogAlways("	saved: ");
	for (const auto& savedPair : pModule->m_savedParams)
	{
		const auto savedName = savedPair.second->savedName;
		const auto savedAuthPlayer = savedPair.second->authorityPlayerName;
		const auto savedBeSlave = savedPair.second->willBeSlave;

		CryLogAlways("		--- name: %s, authName: %s, willSlave: %i", 
			savedName.c_str(), 
			savedAuthPlayer.c_str(), 
			savedBeSlave);
	}

	CryLogAlways("	marked for recreation: ");
	for (const auto markedId : pModule->s_markedForRecreation)
	{
		auto pEntity = gEnv->pEntitySystem->GetEntity(markedId);
		if (!pEntity)
			continue;

		const auto markedName = pEntity->GetName();

		CryLogAlways("		--- name: %s, id: %i", markedName, markedId);
	}


}
