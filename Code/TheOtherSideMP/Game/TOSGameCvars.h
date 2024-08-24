// ReSharper disable CppInconsistentNaming
#pragma once

#define ONLY_SERVER_CMD \
if (!gEnv->bServer)\
{\
	CryLogAlways("Failed: only on the server");\
	return;\
}\

#define ONLY_CLIENT_CMD \
if (!gEnv->bClient)\
{\
	CryLogAlways("Failed: only on the client");\
	return;\
}\

#define GET_ENTITY_FROM_FIRST_ARG \
const string name = pArgs->GetArg(1);\
const auto pEntity = gEnv->pEntitySystem->FindEntityByName(name.c_str());\
if (!pEntity)\
{\
	CryLogAlways("Failed: cant find entity with name %s", name.c_str());\
	return;\
}


struct IConsole;
struct SCVars;

struct STOSCvars  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSCvars()  // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		memset(this, 0, sizeof(STOSCvars));  // NOLINT(bugprone-undefined-memory-manipulation)
	}

	~STOSCvars()
	{
		ReleaseCVars();
		ReleaseCCommands();
		delete this;
	}

	void        InitCVars(IConsole * pConsole);
	static void InitCCommands(IConsole * pConsole);
	static void ReleaseCVars();
	static void ReleaseCCommands();

	// SERVER COMMANDS
	static void CmdNetChName(IConsoleCmdArgs *pArgs);	

	static void CmdDumpActorInfo(IConsoleCmdArgs* pArgs);
	static void CmdGetEntityScriptValue(IConsoleCmdArgs* pArgs);
	static void CmdDumpEntityInfo(IConsoleCmdArgs* pArgs);
	static void CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs);
	static void CmdDumpSynchronizers(IConsoleCmdArgs* pArgs);

	static void CmdConsumerSetEnergy(IConsoleCmdArgs* pArgs);
	static void CmdConsumerSetDrain(IConsoleCmdArgs* pArgs);
	static void CmdConsumerSetDebugEntityName(IConsoleCmdArgs* pArgs);


	// CLIENT COMMANDS
	static void CmdGetDudeName(IConsoleCmdArgs *pArgs);

	// Правило написания консольных значений
	// мод_среда_ОписаниеДействия

	int tos_sv_AIActionTrackerDebugDraw;
	int tos_sv_AIActionTrackerDebugLog;
	int tos_sv_AllDebugLog;
	ICVar* tos_sv_HumanGruntMPEquipPack;
	ICVar* tos_sv_TrooperMPEquipPack;
	ICVar* tos_sv_ScoutMPEquipPack;
	ICVar* tos_sv_AlienMPEquipPack;
	ICVar* tos_sv_HunterMPEquipPack;
	ICVar* tos_sv_EnableMPStealthOMeterForTeam;

	int tos_sv_PlayerAlwaysAiming;
	int tos_sv_EnableShotValidator;

	int tos_any_EventRecorderLogVanilla;

	int tos_cl_DisableSlaveRequestMovement;

	int tos_cl_ShowModVersion;
	float tos_tr_charging_jump_input_time;
};

extern STOSCvars* g_pTOSGameCvars;