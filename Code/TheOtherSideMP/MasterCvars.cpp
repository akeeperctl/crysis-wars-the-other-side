// ReSharper disable CppParameterMayBeConstPtrOrRef
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
	//tos_cl_SlaveEntityClass = pConsole->RegisterString("tos_cl_SlaveEntityClass", "Trooper", VF_NOT_NET_SYNCED, 
		//"Class of entity that will be created as a slave for the client. "
		//"\n Usage: tos_cl_SlaveEntityClass Trooper", 
		//CTOSMasterModule::CVarSetDesiredSlaveCls);

	//pConsole->Register("tos_cl_JoinAsMaster", &tos_cl_JoinAsMaster, 0, VF_NOT_NET_SYNCED, 
		//"When the client enters the game,"
		//"he will control a slave.");

	pConsole->Register("tos_cl_playerFeedbackSoundsVersion", &tos_cl_playerFeedbackSoundsVersion, 1, VF_NOT_NET_SYNCED,
					   "Version of player character feedback sounds. \n 1 - from Crysis 1, \n 2 - from Crysis 2");

	pConsole->Register("tos_cl_nanosuitSoundsVersion", &tos_cl_nanosuitSoundsVersion, 1, VF_NOT_NET_SYNCED,
					   "Version of nanosuit sounds. \n 1 - from Crysis 1, \n 2 - from Crysis 2");

	pConsole->Register("tos_sv_SlaveSpawnDelay", &tos_sv_SlaveSpawnDelay, 0.1f, VF_CHEAT,
					   "Delay in seconds before slave spawns. \n"
					   "It is necessary so that the slave cannot appear before the master respawns");

	// Не используется, да и как показала практика, что с 1.0, что с 0.0 отрицательного влияния не наблюдается
	pConsole->Register("tos_sv_mc_StartControlDelay", &tos_sv_mc_StartControlDelay, 0.0f, VF_CHEAT,
					   "Delay in seconds before master starts control a slave.");

	pConsole->Register("tos_sv_pl_inputAccel", &tos_sv_pl_inputAccel, 30.0f, VF_CHEAT,
					   "Movement input acceleration");

	pConsole->Register("tos_sv_mc_LookDebugDraw", &tos_sv_mc_LookDebugDraw, 1, VF_CHEAT,
					   "Display look debug of the controlled character");

	// Trooper консольные значения
	pConsole->Register("tos_tr_jump_height", &tos_tr_jump_height, 3, VF_CHEAT,
					   "The higher the value, the higher the trooper's jump.");

	pConsole->Register("tos_tr_double_jump_force", &tos_tr_double_jump_force, 1000.0f, VF_CHEAT,
					   "The higher the value, the higher the trooper's second jump.");

	pConsole->Register("tos_tr_charged_jump_mul", &tos_tr_charged_jump_mul, 2.0f, VF_CHEAT,
					   "Charged jump multiplier, normal jump comes with 1");

	pConsole->Register("tos_tr_melee_energy_costs", &tos_tr_melee_energy_costs, 15.0f, VF_CHEAT,
					   "The amount of energy a trooper spends when do melee attack on ground");

	pConsole->Register("tos_tr_double_jump_energy_cost", &tos_tr_double_jump_energy_cost, 50.0f, VF_CHEAT,
					   "The amount of energy a trooper spends when double jumping");

	pConsole->Register("tos_tr_double_jump_melee_energy_cost", &tos_tr_double_jump_melee_energy_cost, 65.0f, VF_CHEAT,
					   "The amount of energy a trooper spends when do melee attack in jumping state");

	pConsole->Register("tos_tr_double_jump_melee_rest_seconds", &tos_tr_double_jump_melee_rest_seconds, 2.0f, VF_CHEAT,
					   "Time in seconds during which the trooper cannot jump after a jump");

	pConsole->Register("tos_tr_regen_energy_start_delay_sp", &tos_tr_regen_energy_start_delay_sp, 6.0f, VF_CHEAT | VF_REQUIRE_LEVEL_RELOAD,
					   "Delay before starting to restore trooper energy after spend it singleplayer");

	pConsole->Register("tos_tr_regen_energy_start_delay_mp", &tos_tr_regen_energy_start_delay_mp, 4.0f, VF_CHEAT | VF_REQUIRE_LEVEL_RELOAD,
					   "Delay before starting to restore trooper energy after spend it in multiplayer");

	pConsole->Register("tos_tr_regen_energy_start_delay_20boundary", &tos_tr_regen_energy_start_delay_20boundary, 2.0f, VF_CHEAT | VF_REQUIRE_LEVEL_RELOAD,
					   "Delay before starting to restore trooper energy after spend");

	pConsole->Register("tos_tr_regen_energy_recharge_time_mp", &tos_tr_regen_energy_recharge_time_mp, 1.0f, VF_CHEAT,
					   "Modify energy recharge for Trooper in multiplayer.");

	pConsole->Register("tos_tr_regen_energy_recharge_time_sp", &tos_tr_regen_energy_recharge_time_sp, 1.0f, VF_CHEAT,
					   "Modify energy recharge for Trooper in singleplayer.");
}

void CTOSMasterModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;

	//pConsole->UnregisterVariable("tos_cl_SlaveEntityClass", true);
	pConsole->UnregisterVariable("tos_cl_JoinAsMaster", true);
	pConsole->UnregisterVariable("tos_cl_playerFeedbackSoundsVersion", true);
	pConsole->UnregisterVariable("tos_cl_nanosuitSoundsVersion", true);

	pConsole->UnregisterVariable("tos_sv_SlaveSpawnDelay", true);
	pConsole->UnregisterVariable("tos_sv_MasterStartControlDelay", true);
	pConsole->UnregisterVariable("tos_sv_pl_inputAccel", true);
	pConsole->UnregisterVariable("tos_sv_mc_StartControlDelay");
	pConsole->UnregisterVariable("tos_sv_pl_inputAccel", true);
	pConsole->UnregisterVariable("tos_sv_mc_LookDebugDraw", true);

	pConsole->UnregisterVariable("tos_tr_jump_height", true);
	pConsole->UnregisterVariable("tos_tr_double_jump_force", true);
	pConsole->UnregisterVariable("tos_tr_charged_jump_mul", true);
	pConsole->UnregisterVariable("tos_tr_melee_energy_costs", true);
	pConsole->UnregisterVariable("tos_tr_double_jump_energy_cost", true);
	pConsole->UnregisterVariable("tos_tr_double_jump_melee_energy_cost", true);
	pConsole->UnregisterVariable("tos_tr_double_jump_melee_rest_seconds", true);
	pConsole->UnregisterVariable("tos_tr_regen_energy_start_delay_sp", true);
	pConsole->UnregisterVariable("tos_tr_regen_energy_start_delay_mp", true);
	pConsole->UnregisterVariable("tos_tr_regen_energy_start_delay_20boundary", true);
	pConsole->UnregisterVariable("tos_tr_regen_energy_recharge_time_mp", true);
	pConsole->UnregisterVariable("tos_tr_regen_energy_recharge_time_sp", true);

}

void CTOSMasterModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	pConsole->AddCommand("tos_cmd_dumpmasterslist", CmdDumpMastersList);
	pConsole->AddCommand("tos_cmd_dumpdudeitems", CmdDumpDudeItems);
	pConsole->AddCommand("tos_cmd_dumpactoritems", CmdDumpActorItems);
	pConsole->AddCommand("tos_cmd_setphysprofile", CmdSetPhysProfile);
	pConsole->AddCommand("tos_cmd_ismaster", CmdIsMaster);
	pConsole->AddCommand("tos_cmd_stopcontrol", CmdMCStopControl);
	pConsole->AddCommand("tos_cmd_setactorhealth", CmdSetActorHealth);
	pConsole->AddCommand("tos_cmd_getactorhealth", CmdGetActorHealth);
	pConsole->AddCommand("tos_cmd_getactorcurrentitem", CmdGetActorCurrentItem);
	pConsole->AddCommand("tos_cmd_playsound2d", CmdPlaySound2D);
}

void CTOSMasterModule::ReleaseCCommands()
{
	CTOSGenericModule::ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;

	pConsole->RemoveCommand("tos_cmd_getmasterslist");
	pConsole->RemoveCommand("tos_cmd_ismaster");
	pConsole->RemoveCommand("tos_cmd_dumpmasterslist");
	pConsole->RemoveCommand("tos_cmd_dumpdudeitems");
	pConsole->RemoveCommand("tos_cmd_setphysprofile");
	pConsole->RemoveCommand("tos_cmd_dumpactoritems");
	pConsole->RemoveCommand("tos_cmd_ismaster");
	pConsole->RemoveCommand("tos_cmd_stopcontrol");
	pConsole->RemoveCommand("tos_cmd_setactorhealth");
	pConsole->RemoveCommand("tos_cmd_getactorhealth");
	pConsole->RemoveCommand("tos_cmd_getactorcurrentitem");
	pConsole->RemoveCommand("tos_cmd_playsound2d");

}

void CTOSMasterModule::OnHit(const HitInfo& info)
{
	IEntity* const pShooter = TOS_GET_ENTITY(info.shooterId);
	CTOSActor* const pTargetActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(info.targetId));

	if (!pShooter || !pTargetActor)
		return;

	if (pShooter->GetId() != pTargetActor->GetEntityId())
	{
		pTargetActor->UpdateLastShooterId(pShooter->GetId());
	}
}

void CTOSMasterModule::CmdDumpMastersList(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;

	std::map<EntityId, _smart_ptr<STOSMasterInfo>> masters;
	g_pTOSGame->GetMasterModule()->GetMasters(masters);

	CryLogAlways("Result: master:id | slave:id");
	for (std::map<EntityId, _smart_ptr<STOSMasterInfo>>::const_iterator it = masters.begin(); it != masters.end(); ++it)
	{
		const EntityId masterId = it->first;
		const auto masterInfo = it->second.get();
		const EntityId slaveId = masterInfo->slaveId;

		const IEntity* pMasterEnt = gEnv->pEntitySystem->GetEntity(masterId);
		const IEntity* pSlaveEnt = gEnv->pEntitySystem->GetEntity(slaveId);

		const char* masterName = pMasterEnt ? pMasterEnt->GetName() : "NULL";
		const char* slaveName = pSlaveEnt ? pSlaveEnt->GetName() : "NULL";
		const char* desiredSlaveClass = masterInfo->desiredSlaveClassName;

		CryLogAlways("	%s:%i | %s:%i | desired slave class: %s", masterName, masterId, slaveName, slaveId, desiredSlaveClass);
	}

}

void CTOSMasterModule::CmdIsMaster(IConsoleCmdArgs* pArgs)
{
	ONLY_SERVER_CMD;
	GET_ENTITY_FROM_FIRST_ARG;


	const bool isMaster = g_pTOSGame->GetMasterModule()->IsMaster(pEntity);
	const char* result = isMaster ? "Yes" : "No";

	CryLogAlways("Result: '%s' is master: %s", name, result);
}

void CTOSMasterModule::CmdMCStopControl(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;

	const auto pLocalMC = g_pTOSGame->GetMasterModule()->GetMasterClient();
	assert(pLocalMC);

	pLocalMC->StopControl();
}

void CTOSMasterModule::CmdDumpDudeItems(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;

	const auto pPlayer = dynamic_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	//assert(pPlayer);
	if (!pPlayer)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	CryLogAlways("Result: ");

	const IInventory* pInventory = pPlayer->GetInventory();
	if (pInventory)
	{
		for (int i = 0; i < pInventory->GetCount(); i++)
		{
			const auto itemId = pInventory->GetItem(i);
			const auto pItemEnt = TOS_GET_ENTITY(itemId);
			const char* name = pItemEnt ? pItemEnt->GetClass()->GetName() : "<UNDEFINED>";

			CryLogAlways("	%i. %s", i, name);
		}
	}
}

void CTOSMasterModule::CmdDumpActorItems(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	CryLogAlways("Result: ");

	const IInventory* pInventory = pActor->GetInventory();
	if (pInventory)
	{
		for (int i = 0; i < pInventory->GetCount(); i++)
		{
			const auto itemId = pInventory->GetItem(i);
			const auto pItemEnt = TOS_GET_ENTITY(itemId);
			const char* name = pItemEnt ? pItemEnt->GetClass()->GetName() : "<UNDEFINED>";

			CryLogAlways("	%i. %s", i, name);
		}
	}
}

void CTOSMasterModule::CmdSetPhysProfile(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	string profile = pArgs->GetArg(2);
	CTOSActor* pActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	uint p = 0;
	if (profile == "alive0")
	{
		// при использовании на клиенте 1, то у клиента 1 трупер будет двигаться нормально
		pActor->Physicalize(STANCE_NULL);
	}
	else if (profile == "alive1")
	{
		// при использовании на клиенте 1, то у клиента 1 трупер будет двигаться нормально
		pActor->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_NonPushable, eColliderModeLayer_Game, "CMDSetPhysProfile");
	}
	else if (profile == "alive2")
	{
		pActor->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Undefined, eColliderModeLayer_Game, "Actor::SetAspectProfile");
		if (IPhysicalEntity* pPhysics = pActor->GetEntity()->GetPhysics())
		{
			if (ICharacterInstance* pCharacter = pActor->GetEntity()->GetCharacter(0))
			{
				pCharacter->GetISkeletonPose()->DestroyCharacterPhysics(2);

				if (IPhysicalEntity* pCharPhysics = pCharacter->GetISkeletonPose()->GetCharacterPhysics())
				{
					pe_params_articulated_body body;
					body.pHost = pPhysics;
					pCharPhysics->SetParams(&body);
				}
			}
		}
	}
	else if (profile == "alive3")
	{
		if (ICharacterInstance* pCharacter = pActor->GetEntity()->GetCharacter(0))
			pCharacter->GetISkeletonPose()->DestroyCharacterPhysics(1);

		pActor->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
		pActor->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "Actor::SetAspectProfile");
	}
	else if (profile == "alive4")
	{
		if (pActor->GetAnimatedCharacter())
			pActor->GetAnimatedCharacter()->ResetState();
	}
	else if (profile == "unragdoll")
	{
		SEntityPhysicalizeParams params;
		params.type = PE_NONE;
		pActor->GetEntity()->Physicalize(params);
	}
	else
	{
		CryLogAlways("Failed: invalid physics profile");
		return;
	}
}

void CTOSMasterModule::CmdSetActorHealth(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	const char* strHP = pArgs->GetArg(2);
	const int HP = atoi(strHP);

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	//assert(pActor);
	if (!pActor)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	pActor->SetHealth(HP);
}

void CTOSMasterModule::CmdGetActorHealth(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	CryLogAlways("Result: HP = %i", pActor->GetHealth());
}

void CTOSMasterModule::CmdGetActorCurrentItem(IConsoleCmdArgs* pArgs)
{
	GET_ENTITY_FROM_FIRST_ARG;

	const auto pActor = (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
	if (!pActor)
	{
		CryLogAlways("Failed: not found actor");
		return;
	}

	const auto pItem = pActor->GetCurrentItem();

	CryLogAlways("Result: Item name = %s", pItem ? pItem->GetEntity()->GetClass()->GetName() : "<undefined>");
}

void CTOSMasterModule::CmdPlaySound2D(IConsoleCmdArgs* pArgs)
{
	ONLY_CLIENT_CMD;
	const string soundPath = pArgs->GetArg(1);

	if (soundPath.empty())
	{
		CryLogAlways("Usage 1: tos_cmd_play_sound_2d Localized/Languages/dialog/suit/v2/suit_voice_danger.mp2");
		CryLogAlways("Usage 2: tos_cmd_play_sound_2d Sounds/interface:suit:suit_armor_use");
		CryLogAlways("Usage 3: tos_cmd_play_sound_2d Sounds/<folder_name>:<event_group_name>:<event_name>");
		return;
	}

	const _smart_ptr<ISound> pSound = gEnv->pSoundSystem->CreateSound(soundPath, FLAG_SOUND_2D | FLAG_SOUND_VOICE);
	if (pSound)
	{
		pSound->SetPosition(g_pTOSGame->GetActualClientActor()->GetEntity()->GetWorldPos());
		pSound->Play();
	}
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
