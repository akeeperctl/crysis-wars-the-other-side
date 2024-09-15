// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
#include "StdAfx.h"
#include "TOSGameCvars.h"

#include "Game.h"
#include "IConsole.h"
#include "ScriptUtils.h"

#include "Modules/GenericSynchronizer.h"
#include "Modules/EntitySpawn/EntitySpawnModule.h"
#include "Modules/Master/MasterModule.h"

#include "TheOtherSideMP/Actors/TOSActor.h"
#include "TheOtherSideMP/Extensions/EnergyСonsumer.h"

void STOSCvars::InitCVars(IConsole* pConsole)
{
	// 04/10/2023 пока что уберем неиспользуемые консольные значения
	//pConsole->Register("tos_debug_draw_aiactiontracker", &tos_debug_draw_aiactiontracker, 0, 0, "");
	//pConsole->Register("tos_debug_log_aiactiontracker", &tos_debug_log_aiactiontracker, 0, 0, "");
	//pConsole->Register("tos_debug_log_all", &tos_debug_log_all, 0, 0, "");
	//pConsole->Register("tos_show_version", &tos_show_version, 1, 0, "");
	//VF_DUMPTODISK

	pConsole->Register("tos_cl_DisableSlaveRequestMovement", &tos_cl_DisableSlaveRequestMovement, 0, VF_RESTRICTEDMODE | VF_NOT_NET_SYNCED, "Disable slave entity look at crosshair");
	pConsole->Register("tos_any_EventRecorderLogVanilla", &tos_any_EventRecorderLogVanilla, 0, 0, "Log vanilla events to the console (eGE_ prefix) 1 - yes, 0 - no");
	pConsole->Register("tos_sv_EnableShotValidator", &tos_sv_EnableShotValidator, 1, 0, "Enable shot validator in multiplayer 1 - yes, 0 - no");
	pConsole->Register("tos_tr_charging_jump_input_time", &tos_tr_charging_jump_input_time, 0.20f, 0, "Time between press jump and jump action confirm");
	pConsole->Register("tos_sv_PlayerAlwaysAiming", &tos_sv_PlayerAlwaysAiming, 1, 0, "");
	pConsole->Register("tos_sv_enable_ghost_item_fix", &tos_sv_enable_ghost_item_fix, 1, VF_CHEAT, "");
	pConsole->Register("tos_sv_enable_ghost_item_fix_log", &tos_sv_enable_ghost_item_fix_log, 0, VF_CHEAT, "");
	pConsole->Register("tos_temp_var", &tos_temp_var, 0, 0, "");

	tos_sv_AlienMPEquipPack = pConsole->RegisterString("tos_sv_AlienMPEquipPack", "Alien_naked", 0, "");
	tos_sv_HunterMPEquipPack = pConsole->RegisterString("tos_sv_HunterMPEquipPack", "Alien_Hunter", 0, "");
	tos_sv_ScoutMPEquipPack = pConsole->RegisterString("tos_sv_ScoutMPEquipPack", "Alien_Scout_Gunner", 0, "");
	tos_sv_TrooperMPEquipPack = pConsole->RegisterString("tos_sv_TrooperMPEquipPack", "Alien_Trooper", 0, "");
	tos_sv_HumanGruntMPEquipPack = pConsole->RegisterString("tos_sv_HumanGruntMPEquipPack", "NK_Pistol", 0, "");
	tos_sv_EnableMPStealthOMeterForTeam = pConsole->RegisterString("tos_sv_EnableMPStealthOMeterForTeam", "aliens", 0, "Enable for only one Team. Teams: all, black, tan, aliens");

	for (std::vector<ITOSGameModule*>::iterator it = g_pTOSGame->m_modules.begin(); it != g_pTOSGame->m_modules.end(); ++it)
		(*it)->InitCVars(pConsole);
}

void STOSCvars::InitCCommands(IConsole* pConsole)
{
	//SERVER COMMANDS
	pConsole->AddCommand("netchname", CmdNetChName);
	pConsole->AddCommand("getentitiesbyclass", CmdGetEntitiesByClass);
	pConsole->AddCommand("getentityscriptvalue", CmdGetEntityScriptValue);
	pConsole->AddCommand("dumpsynchronizers", CmdDumpSynchronizers);
	pConsole->AddCommand("dumpentityinfo", CmdDumpEntityInfo);
	pConsole->AddCommand("dumpactorinfo", CmdDumpActorInfo);

	// Отладочные команды потребителя энергии
	pConsole->AddCommand("consumersetenergy", CmdConsumerSetEnergy);
	pConsole->AddCommand("consumersetdrain", CmdConsumerSetDrain);
	pConsole->AddCommand("consumersetdebugentname", CmdConsumerSetDebugEntityName);

	//CLIENT COMMANDS
	pConsole->AddCommand("getdudename", CmdGetDudeName);

	for (std::vector<ITOSGameModule*>::iterator it = g_pTOSGame->m_modules.begin(); it != g_pTOSGame->m_modules.end(); ++it)
		(*it)->InitCCommands(pConsole);
}

void STOSCvars::ReleaseCCommands()
{
	for (std::vector<ITOSGameModule*>::iterator it = g_pTOSGame->m_modules.begin(); it != g_pTOSGame->m_modules.end(); ++it)
		(*it)->ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("netchname");
	pConsole->RemoveCommand("getlocalname");
	pConsole->RemoveCommand("consumersetenergy");
	pConsole->RemoveCommand("consumersetdrain");
	pConsole->RemoveCommand("consumersetdebugentname");
	pConsole->RemoveCommand("dumpactorinfo");
	pConsole->RemoveCommand("netchname");
	pConsole->RemoveCommand("getentitiesbyclass");
	pConsole->RemoveCommand("getentityscriptvalue");
	pConsole->RemoveCommand("dumpsynchronizers");
	pConsole->RemoveCommand("dumpentityinfo");
	pConsole->RemoveCommand("dumpactorinfo");
	pConsole->RemoveCommand("consumersetenergy");
	pConsole->RemoveCommand("consumersetdrain");
	pConsole->RemoveCommand("consumersetdebugentname");
	pConsole->RemoveCommand("getdudename");
}

void STOSCvars::ReleaseCVars()
{
	for (std::vector<ITOSGameModule*>::iterator it = g_pTOSGame->m_modules.begin(); it != g_pTOSGame->m_modules.end(); ++it)
		(*it)->ReleaseCVars();


	const auto pConsole = gEnv->pConsole;

	//pConsole->UnregisterVariable("tos_debug_draw_aiactiontracker", true);
	//pConsole->UnregisterVariable("tos_debug_log_aiactiontracker", true);
	//pConsole->UnregisterVariable("tos_debug_log_all", true);
	//pConsole->UnregisterVariable("tos_show_version", true);
	//pConsole->UnregisterVariable("tos_cl_SlaveEntityClass", true);

	//pConsole->UnregisterVariable("tos_cl_JoinAsMaster", true);
	pConsole->UnregisterVariable("tos_sv_enable_ghost_item_fix", true);
	pConsole->UnregisterVariable("tos_sv_enable_ghost_item_fix_log", true);

	pConsole->UnregisterVariable("tos_temp_var", true);
	pConsole->UnregisterVariable("tos_sv_PlayerAlwaysAiming", true);
	pConsole->UnregisterVariable("tos_cl_DisableSlaveRequestMovement", true);
	pConsole->UnregisterVariable("tos_any_EventRecorderLogVanilla", true);

	pConsole->UnregisterVariable("tos_sv_EnableShotValidator", true);
	pConsole->UnregisterVariable("tos_sv_EnableMPStealthOMeterForTeam", true);
	pConsole->UnregisterVariable("tos_tr_charging_jump_input_time", true);

	pConsole->UnregisterVariable("tos_sv_AlienMPEquipPack", true);
	pConsole->UnregisterVariable("tos_sv_HunterMPEquipPack", true);
	pConsole->UnregisterVariable("tos_sv_ScoutMPEquipPack", true);
	pConsole->UnregisterVariable("tos_sv_TrooperMPEquipPack", true);
	pConsole->UnregisterVariable("tos_sv_HumanGruntMPEquipPack", true);
}

void STOSCvars::CmdNetChName(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

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

void STOSCvars::CmdDumpActorInfo(IConsoleCmdArgs* pArgs)
{
	//m_currentPhysProfile

	ONLY_SERVER_CMD;

	GET_ENTITY_FROM_FIRST_ARG;

	auto pActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: nullptr pActor");
		return;
	}

	pActor->DumpActorInfo();
}

void STOSCvars::CmdGetEntityScriptValue(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	const auto pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable)
	{
		CryLogAlways("Failed: pointer to script table is NULL");
		return;
	}

	const char* pathToValue = pArgs->GetArg(2);
	const string tokenStream(pathToValue);
	int curPos = 0;

	ScriptAnyValue value;

	// Deal with first token specially
	string token = tokenStream.Tokenize(".", curPos);
	if (token.empty())
	{
		CryLogAlways("Failed: path to value is NULL");
		return; // Catching, say, an empty string
	}
	if (!pScriptTable->GetValueAny(token, value))
	{
		CryLogAlways("Failed: script table value %s not found", token.c_str());
		return;
	}

	// Tokenize remainder
	token = tokenStream.Tokenize(".", curPos);
	while (!token.empty())
	{
		// Make sure the last step was a table
		if (value.type != ANY_TTABLE)
		{
			CryLogAlways("Failed: previos path of %s is not a table", token.c_str());
			return;
		}

		// Must use temporary 
		ScriptAnyValue getter;
		value.table->GetValueAny(token, getter);
		value = getter;
		token = tokenStream.Tokenize(".", curPos);
	}

	switch (value.type)
	{
		case ANY_TNIL:
			CryLogAlways("Failed: %s not found in script table", pathToValue);
			break;
		case ANY_TBOOLEAN:
			CryLogAlways("Result: %s = %i", pathToValue, value.b);
			break;
		case ANY_TNUMBER:
			CryLogAlways("Result: %s = %f", pathToValue, value.number);
			break;
		case ANY_TSTRING:
			CryLogAlways("Result: %s = %s", pathToValue, value.str);
			break;
		case ANY_TVECTOR:
			CryLogAlways("Result: %s = (%1.f, %1.f, %1.f)", pathToValue, value.vec3.x, value.vec3.y, value.vec3.z);
			break;
		case ANY_TTABLE:
			CryLogAlways("Result: values of table %s", pathToValue);
			value.table->Dump(g_pTOSGame);
			break;
		case ANY_THANDLE:
		case ANY_ANY:
		case ANY_TFUNCTION:
		case ANY_TUSERDATA:
		case ANY_COUNT:
			break;
	}
}

void STOSCvars::CmdDumpEntityInfo(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	GET_ENTITY_FROM_FIRST_ARG;

	const string playerName = pArgs->GetArg(2);
	const auto pPlayerEntity = gEnv->pEntitySystem->FindEntityByName(playerName);
	if (!pPlayerEntity)
	{
		CryLogAlways("Failed: cant find player with name %s", playerName.c_str());
	}
	const char* strName = pEntity ? pEntity->GetName() : "NULL";

	CryLogAlways("Result: ");
	CryLogAlways("	Name: %s", strName);

	if (pPlayerEntity)
	{
		const auto pPlayer = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pPlayerEntity->GetId());
		const auto playerChannelId = pPlayer->GetChannelId();
		const auto pPlayerNetChannel = g_pGame->GetIGameFramework()->GetNetChannel(playerChannelId);

		const auto isAuth = g_pGame->GetIGameFramework()->GetNetContext()->RemoteContextHasAuthority(pPlayerNetChannel, pEntity->GetId());

		const char* strAuth = isAuth ? "True" : "False";
		CryLogAlways("	Authority: %s", strAuth);
	}

	TOS_Debug::DumpEntityFlags(pEntity);

	IEntityRenderProxy* pRenderProxy = static_cast<IEntityRenderProxy*>(pEntity->GetProxy(ENTITY_PROXY_RENDER));

	if (pRenderProxy)
	{
		IRenderNode* pRenderNode = pRenderProxy ? pRenderProxy->GetRenderNode() : 0;

		if (pRenderNode)
		{
			//pRenderNode->SetViewDistRatio(255);
			//pRenderNode->SetLodRatio(80); //IVO: changed to fix LOD problem in MP

			CryLogAlways("	LodRatio: %i", pRenderNode->GetLodRatio());
			CryLogAlways("	ViewDistRatio: %i", pRenderNode->GetViewDistRatio());
		}
	}
}

void STOSCvars::CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

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

void STOSCvars::CmdDumpSynchronizers(IConsoleCmdArgs* pArgs)
{
	CryLogAlways("Result: (name|id)");

	TSynches syncs;
	CTOSGenericSynchronizer::GetSynchonizers(syncs);

	for (TSynches::const_iterator it = syncs.begin(); it != syncs.end(); ++it)
	{
		const char* name = it->first;
		const int id = it->second; // Предполагается, что id имеет тип int

		CryLogAlways("	%s|%i", name, id);
	}
}

void STOSCvars::CmdConsumerSetEnergy(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	GET_ENTITY_FROM_FIRST_ARG;

	const auto pActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: actor not found");
		return;
	}

	const string energyStr = pArgs->GetArg(2);
	const int energy = atoi(energyStr.empty() ? 0 : energyStr);
	pActor->GetEnergyConsumer()->SetEnergy(energy);
}

void STOSCvars::CmdConsumerSetDrain(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;
	GET_ENTITY_FROM_FIRST_ARG;

	const string energyStr = pArgs->GetArg(2);
	const int energy = atoi(energyStr.empty() ? 0 : energyStr);

	const auto pActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: actor not found");
		return;
	}

	pActor->GetEnergyConsumer()->SetDrainValue(energy);
}

void STOSCvars::CmdConsumerSetDebugEntityName(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;
	GET_ENTITY_FROM_FIRST_ARG;


	CTOSEnergyConsumer::SetDebugEntityName(pEntity->GetName());
}

void STOSCvars::CmdGetDudeName(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;

	const auto pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
	assert(pPlayer);

	CryLogAlways("Result: %s", pPlayer->GetEntity()->GetName());
}
