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

	void InitCVars(IConsole *pConsole);
	void InitCCommands(IConsole *pConsole) const;
	void ReleaseCVars() const;
	void ReleaseCCommands() const;

	// SERVER COMMANDS
	static void CmdNetChName(IConsoleCmdArgs *pArgs);	

	static void CmdSpawnEntity(IConsoleCmdArgs* pArgs);
	static void CmdRemoveEntity(IConsoleCmdArgs* pArgs);

	static void CmdGetEntityScriptValue(IConsoleCmdArgs* pArgs);
	static void CmdGetEntityById(IConsoleCmdArgs* pArgs);
	static void CmdGetEntitiesByClass(IConsoleCmdArgs* pArgs);
	static void CmdGetSyncs(IConsoleCmdArgs* pArgs);

	// CLIENT COMMANDS
	static void CmdGetLocalName(IConsoleCmdArgs *pArgs);

	// Правило написания консольных значений
	// мод_среда_ОписаниеДействия

	int tos_sv_AIActionTrackerDebugDraw;
	int tos_sv_AIActionTrackerDebugLog;
	int tos_sv_AllDebugLog;

	int tos_any_EventRecorderLogVanilla;

	int tos_cl_ShowModVersion;
};

extern STOSCvars* g_pTOSGameCvars;