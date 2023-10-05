#include "StdAfx.h"

#include "Game/TOSGameCvars.h"
#include "Game/Modules/Master/MasterClient.h"
#include "Game/Modules/Master/MasterModule.h"
#include "Game/Modules/Master/MasterSynchronizer.h"

void CTOSMasterModule::InitCVars(IConsole* pConsole)
{
	CTOSGenericModule::InitCVars(pConsole);

	// консольные значения локального клиента
	tos_cl_SlaveEntityClass = pConsole->RegisterString("tos_cl_SlaveEntityClass", "Trooper", VF_NOT_NET_SYNCED, "Class of entity that will be created as a slave for the client. \n Example: Trooper", CTOSMasterModule::CVarSetDesiredSlaveCls);
	pConsole->Register("tos_cl_JoinAsMaster", &tos_cl_JoinAsMaster, 0, VF_NOT_NET_SYNCED, "When the client enters the game, he will control a slave.");
	pConsole->Register("tos_sv_SlaveSpawnDelay", &tos_sv_SlaveSpawnDelay, 0.03f, VF_CHEAT, "Delay in seconds before slave spawns. It is necessary so that the slave cannot appear before the master respawns");
}

void CTOSMasterModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	pConsole->AddCommand("getmasterslist", CmdGetMastersList);
	pConsole->AddCommand("ismaster", CmdIsMaster);
	pConsole->AddCommand("stopcontrol", CmdStopControl);

}

void CTOSMasterModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;

	pConsole->UnregisterVariable("tos_cl_SlaveEntityClass", true);
	pConsole->UnregisterVariable("tos_cl_JoinAsMaster", true);
	pConsole->UnregisterVariable("tos_sv_SlaveSpawnDelay", true);
}

void CTOSMasterModule::ReleaseCCommands()
{
	CTOSGenericModule::ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("getmasterslist");
	pConsole->RemoveCommand("ismaster");
}

void CTOSMasterModule::CmdGetMastersList(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	std::map<EntityId, STOSMasterInfo> masters;
	g_pTOSGame->GetMasterModule()->GetMasters(masters);

	CryLogAlways("Result: (master(id)|slave(id))");
	for (const auto& masterPair : masters)
	{
		const EntityId masterId = masterPair.first;
		const EntityId slaveId = masterPair.second.slaveId;

		const auto pMasterEnt = gEnv->pEntitySystem->GetEntity(masterId);
		const auto pSlaveEnt = gEnv->pEntitySystem->GetEntity(slaveId);

		const char* masterName = pMasterEnt ? pMasterEnt->GetName() : "NULL";
		const char* slaveName = pSlaveEnt ? pSlaveEnt->GetName() : "NULL";
		const char* desiredSlaveClass = masterPair.second.desiredSlaveClassName;

		CryLogAlways("	%s(%i)|%s(%i)(%s)", masterName, masterId, slaveName, slaveId, desiredSlaveClass);
	}
}

void CTOSMasterModule::CmdIsMaster(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

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

void CTOSMasterModule::CmdStopControl(IConsoleCmdArgs* pArgs)
{
	const auto pLocalMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	assert(pLocalMC);

	pLocalMC->StopControl();
}

void CTOSMasterModule::CVarSetDesiredSlaveCls(ICVar* pVar)
{
	if (gEnv->bClient)
	{
		const string clsName = pVar->GetString();
		const auto pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(clsName.c_str());

		if (!pClass)
		{
			CryLogAlways("Failed: %s class not found", clsName.c_str());
			return;
		}

		const auto pSynch = dynamic_cast<CTOSMasterSynchronizer*>(g_pTOSGame->GetMasterModule()->GetSynchronizer());
		assert(pSynch);

		if (!pSynch)
		{
			CryLogAlways("Failed: Master Module synchronizer's entity pointer is NULL");
			return;
		}

		DesiredSlaveClsParams params;
		//id мастера
		params.entityId = g_pGame->GetIGameFramework()->GetClientActorId();
		params.desiredSlaveClassName = clsName;

		pSynch->RMISend(CTOSMasterSynchronizer::SvRequestSetDesiredSlaveCls(), params, eRMI_ToServer);
	}
}
