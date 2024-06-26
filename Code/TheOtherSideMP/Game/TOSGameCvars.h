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
	static void CmdGetEntityById(IConsoleCmdArgs* pArgs);
	static void CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs);
	static void CmdGetSyncs(IConsoleCmdArgs* pArgs);

	static void CmdConsumerSetEnergy(IConsoleCmdArgs* pArgs);
	static void CmdConsumerSetDrain(IConsoleCmdArgs* pArgs);
	static void CmdConsumerSetDebugEntityName(IConsoleCmdArgs* pArgs);


	// CLIENT COMMANDS
	static void CmdGetLocalName(IConsoleCmdArgs *pArgs);

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

	int tos_sv_EnableShotValidator;

	int tos_any_EventRecorderLogVanilla;

	int tos_cl_DisableLookAt;

	int tos_cl_ShowModVersion;
	float tos_sv_chargingJumpInputTime;
};

extern STOSCvars* g_pTOSGameCvars;