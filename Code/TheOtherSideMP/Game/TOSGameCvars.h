#pragma once

struct IConsole;
struct SCVars;

struct STOSCvars
{
	STOSCvars()
	{
		memset(this, 0, sizeof(STOSCvars));
	}

	~STOSCvars()
	{
		ReleaseCVars();
		ReleaseCCommands();
		delete this;
	}

	void InitCVars(IConsole *pConsole);
	void InitCCommands(IConsole *pConsole);
	void ReleaseCVars();
	void ReleaseCCommands();

	// SERVER COMMANDS
	static void CmdNetChName(IConsoleCmdArgs *pArgs);	
	static void CmdGetMastersList(IConsoleCmdArgs *pArgs);
	static void CmdIsMaster(IConsoleCmdArgs *pArgs);

	static void CmdSpawnTrooper(IConsoleCmdArgs* pArgs);
	static void CmdRemoveEntity(IConsoleCmdArgs* pArgs);

	// CLIENT COMMANDS
	static void CmdGetLocalName(IConsoleCmdArgs *pArgs);

	int tos_debug_draw_aiactiontracker;
	int tos_debug_log_aiactiontracker;
	int tos_show_version;
	int tos_debug_log_all;
};

extern struct STOSCvars* g_pTOSGameCvars;