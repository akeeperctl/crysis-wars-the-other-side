/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

FGPS авторы писали определения функциям
**************************************************************************/

#pragma once
#include "IConsole.h"
#include "Nodes\G2FlowBaseNode.h"
#include "PluginCommon.h"


// DLL FG plugin definition
struct SFGPlugin
{
	void* hLib;
	string szPath;
	CG2AutoRegFlowNodeBase* nodes;

	SFGPlugin() : hLib(NULL), nodes(NULL)
	{
	}
};

typedef std::vector<SFGPlugin> FGPluginList;

// Extended resources
struct SExtendedResource
{
	EPluginResources eId;
	void *pData;

	SExtendedResource(EPluginResources _eId, void *_pData) : eId(_eId), pData(_pData)
	{
		CRY_ASSERT(pData);
		CRY_ASSERT_MESSAGE(eId > ePR_Invalid && eId < ePR_Count, "Invalid Resource Id");
	}

	bool operator ==(const SExtendedResource& o)
	{
		return (eId == o.eId);
	}
};
typedef std::vector<SExtendedResource> ExtendedResourceList;

class CFGPluginLoader
{
public:
	friend class CTOSGame;

	CFGPluginLoader(IConsole* pConsole, SCVars* pGameCvars);
	////////////////////////////////////////////////////
	// RegisterPlugin
	//
	// Purpose: Register a plug-in Dll with the system
	//
	// In:	pDll - Dll to load
	//
	// Returns TRUE if Dll was loaded
	////////////////////////////////////////////////////
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

private:
	void RegisterPlugins();
	bool RegisterPlugin(char const* pDll);
	void FreePluginLibraries();
	void Shutdown();


	//Console commands
	static void CmdDumpPlugins(IConsoleCmdArgs* cmdArgs);
	static void CmdShowVersion(IConsoleCmdArgs* pArgs);

	//Console variables
	int fgps_debug;

	IConsole* m_pConsole;
	SCVars* m_pGameCVars;

	// Dll Plugin list
	int m_nPluginCounter;
	static FGPluginList m_Plugins;

	static ExtendedResourceList m_ResourceList;
	static CG2AutoRegFlowNodeBase *m_LastNext;
	static CG2AutoRegFlowNodeBase *m_Last;
};