#include "StdAfx.h"

#include "Control/ControlSystem.h"

#include "Game/TOSGameCvars.h"
#include "Game/Modules/Master/MasterClient.h"
#include "Game/Modules/Master/MasterModule.h"
#include "Game/Modules/Master/MasterSynchronizer.h"

void CTOSMasterModule::InitCVars(IConsole* pConsole)
{
	CTOSGenericModule::InitCVars(pConsole);

	// консольные значения локального клиента
	tos_cl_SlaveEntityClass = pConsole->RegisterString("tos_cl_SlaveEntityClass", "Trooper", VF_NOT_NET_SYNCED, 
		"Class of entity that will be created as a slave for the client. "
		"\n Usage: tos_cl_SlaveEntityClass Trooper", 
		CTOSMasterModule::CVarSetDesiredSlaveCls);

	pConsole->Register("tos_cl_JoinAsMaster", &tos_cl_JoinAsMaster, 1, VF_NOT_NET_SYNCED, 
		"When the client enters the game,"
		"he will control a slave.");

	pConsole->Register("tos_sv_SlaveSpawnDelay", &tos_sv_SlaveSpawnDelay, 0.03f, VF_CHEAT, 
		"Delay in seconds before slave spawns. \n" 
		"It is necessary so that the slave cannot appear before the master respawns");

	// Не используется, да и как показала практика, что с 1.0, что с 0.0 отрицательного влияния не наблюдается
	pConsole->Register("tos_sv_MasterStartControlDelay", &tos_sv_MasterStartControlDelay, 0.0f, VF_CHEAT,
		"Delay in seconds before master starts control a slave.");

	pConsole->Register("tos_sv_pl_inputAccel", &tos_sv_pl_inputAccel, 30.0f, VF_CHEAT,
		"Movement input acceleration");

}

void CTOSMasterModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	pConsole->AddCommand("getmasterslist", CmdGetMastersList);
	pConsole->AddCommand("ismaster", CmdIsMaster);
	pConsole->AddCommand("mc_stopcontrol", CmdMCStopControl);
	pConsole->AddCommand("showdudeitems", CmdShowDudeItems);
	pConsole->AddCommand("showactoritems", CmdShowActorItems);
	pConsole->AddCommand("setactorhealth", CmdSetActorHealth);
	pConsole->AddCommand("getactorhealth", CmdGetActorHealth);

}

void CTOSMasterModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;

	pConsole->UnregisterVariable("tos_cl_SlaveEntityClass", true);
	pConsole->UnregisterVariable("tos_cl_JoinAsMaster", true);
	pConsole->UnregisterVariable("tos_sv_SlaveSpawnDelay", true);
	pConsole->UnregisterVariable("tos_sv_MasterStartControlDelay", true);
	pConsole->UnregisterVariable("tos_sv_pl_inputAccel", true);
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

void CTOSMasterModule::CmdMCStopControl(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;

	const auto pLocalMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	assert(pLocalMC);

	pLocalMC->StopControl();
}

void CTOSMasterModule::CmdShowDudeItems(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;

	const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	assert(pPlayer);
	if (!pPlayer)
		return;

	CryLogAlways("Result: ");

	const IInventory* pInventory = pPlayer->GetInventory();
	if (pInventory)
	{
		for (int i = 0; i < pInventory->GetCount(); i++)
		{
			const auto itemId = pInventory->GetItem(i);
			const auto pItemEnt = TOS_GET_ENTITY(itemId);
			const char* name = pItemEnt ? pItemEnt->GetClass()->GetName() : "<UNDEFINED>";

			CryLogAlways("	%i) %s", i, name);
		}
	}
}

void CTOSMasterModule::CmdShowActorItems(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	const char* strPlayerId = pArgs->GetArg(1);
	const EntityId playerId = atoi(strPlayerId);

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));
	assert(pActor);
	if (!pActor)
		return;

	CryLogAlways("Result: ");

	const IInventory* pInventory = pActor->GetInventory();
	if (pInventory)
	{
		for (int i = 0; i < pInventory->GetCount(); i++)
		{
			const auto itemId = pInventory->GetItem(i);
			const auto pItemEnt = TOS_GET_ENTITY(itemId);
			const char* name = pItemEnt ? pItemEnt->GetClass()->GetName() : "<UNDEFINED>";

			CryLogAlways("	%i) %s", i, name);
		}
	}
}

void CTOSMasterModule::CmdSetActorHealth(IConsoleCmdArgs* pArgs)
{
	const char* strId = pArgs->GetArg(1);
	const char* strHP = pArgs->GetArg(2);
	const EntityId Id = atoi(strId);
	const int HP = atoi(strHP);

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(Id));
	assert(pActor);
	if (!pActor)
		return;

	pActor->SetHealth(HP);
}

void CTOSMasterModule::CmdGetActorHealth(IConsoleCmdArgs* pArgs)
{
	const char* strId = pArgs->GetArg(1);
	const EntityId Id = atoi(strId);

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(Id));
	if (!pActor)
		return;

	CryLogAlways("Result: HP = %i", pActor->GetHealth());
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

		NetDesiredSlaveClsParams params;
		//id мастера
		params.entityId = g_pGame->GetIGameFramework()->GetClientActorId();
		params.desiredSlaveClassName = clsName;

		pSynch->RMISend(CTOSMasterSynchronizer::SvRequestSetDesiredSlaveCls(), params, eRMI_ToServer);
	}
}
