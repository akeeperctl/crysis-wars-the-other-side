/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

FGPS авторы писали определения функциям
**************************************************************************/

#include "StdAfx.h"

#include "FGPluginLoader.h"
#include "CryLibrary.h"
#include <windows.h>

ExtendedResourceList CFGPluginLoader::m_ResourceList;
FGPluginList CFGPluginLoader::m_Plugins;

bool CFGPluginLoader::RegisterPlugin(char const* pDll)
{
	HMODULE hLib = LoadLibrary(pDll);
	if (hLib)
	{
		// Attempt the register
		RegisterFunc pFunc = (RegisterFunc)GetProcAddress(hLib, PLUGIN_NAME_RegisterFunc);
		SPluginRegister registerResults;
		if (!pFunc || !pFunc(gEnv->pSystem, m_pGameCVars, registerResults))
		{
			CryLogAlways("[FGPS] [%d] Registration failed on plugin's end", m_nPluginCounter);
			return false;
		}

		// Check version
		if (PLUGIN_VERSION[3] > registerResults.version[3] ||
			PLUGIN_VERSION[2] > registerResults.version[2] ||
			PLUGIN_VERSION[1] > registerResults.version[1])
		{
			char systemver[20], pluginver[20];
			PLUGIN_VERSION.ToString(systemver);
			registerResults.version.ToString(pluginver);
			CryLogAlways("[FGPS] [%d] Version mismatch - System: %s - Plugin: %s", m_nPluginCounter, systemver, pluginver);
			return false;
		}

		// Handle extended resources registrations
		AddResourceFunc pResourceFunc = (AddResourceFunc)GetProcAddress(hLib, PLUGIN_NAME_AddResourceFunc);
		if (pResourceFunc)
		{
			ExtendedResourceList::const_iterator itResource = m_ResourceList.begin();
			ExtendedResourceList::const_iterator itResourceEnd = m_ResourceList.end();
			for (; itResource != itResourceEnd; ++itResource)
			{
				if (!pResourceFunc(itResource->eId, itResource->pData))
				{
					CryLogAlways("[FGPS] [%d] Failed to add extended resource %d", m_nPluginCounter, (int)itResource->eId);
					return false;
				}
			}
		}

		GetDataFunc pNameFunc = (GetDataFunc)GetProcAddress(hLib, PLUGIN_NAME_GetName);
		GetDataFunc pAuthorFunc = (GetDataFunc)GetProcAddress(hLib, PLUGIN_NAME_GetAuthor);
		GetDataFunc pVersionFunc = (GetDataFunc)GetProcAddress(hLib, PLUGIN_NAME_GetVersion);
		CryLogAlways("[FGPS] [%d] Name: %s; Author: %s; Version: %s", m_nPluginCounter,
					 pNameFunc ? pNameFunc() : "No Name",
					 pAuthorFunc ? pAuthorFunc() : "No Author",
					 pVersionFunc ? pVersionFunc() : "Unknown"
		);

		// Make new entry
		SFGPlugin pluginEntry;
		pluginEntry.hLib = static_cast<void*>(hLib);
		pluginEntry.szPath = pDll;
		pluginEntry.nodes = registerResults.nodesFirst;
		m_Plugins.push_back(pluginEntry);

		// Output found nodes
		CG2AutoRegFlowNodeBase* node = registerResults.nodesFirst;
		CryLogAlways("[FGPS] [%d] Flowgraph nodes registered:", m_nPluginCounter);
		if (node)
		{
			int count = 0;
			while (node)
			{
				CryLogAlways("[FGPS] [%d] -> (%d) %s", m_nPluginCounter, ++count, node->m_sClassName);
				node = node->m_pNext;
			}
			if (count == 1)
			{
				CryLogAlways("[FGPS] [%d] The %d node was registered successfully!", m_nPluginCounter, count);
			}
			else
			{
				CryLogAlways("[FGPS] [%d] All %d nodes were registered successfully!", m_nPluginCounter, count);
			}

			// Add to flowgraph registration list
			CG2AutoRegFlowNodeBase::m_pLast->m_pNext = registerResults.nodesFirst;
			CG2AutoRegFlowNodeBase::m_pLast = registerResults.nodesLast;
		}
		else
		{
			CryLogAlways("[FGPS] [%d] -> No nodes were found!", m_nPluginCounter);
		}

		return true;
	}

	return false;

}

////////////////////////////////////////////////////
void CFGPluginLoader::FreePluginLibraries()
{
	std::vector<SFGPlugin>::iterator it;

	for (it = m_Plugins.begin(); it != m_Plugins.end(); it++)
	{
		SFGPlugin pP = *it;
		CryFreeLibrary(pP.hLib);
	}
	
	m_Plugins.clear();
}

////////////////////////////////////////////////////
void CFGPluginLoader::CmdDumpPlugins(IConsoleCmdArgs* cmdArgs)
{
	CryLogAlways("--------------------------------");
	CryLogAlways("[FGPS] PLUGINLIST:");
	CryLogAlways("--------------------------------");

	std::vector<SFGPlugin>::iterator it;

	for (it = m_Plugins.begin(); it != m_Plugins.end(); it++)
	{
		SFGPlugin pP = *it;
		CryLogAlways("%s", PathUtil::GetFileName((*it).szPath));
	}

	CryLogAlways("--------------------------------");
}

////////////////////////////////////////////////////
void CFGPluginLoader::CmdShowVersion(IConsoleCmdArgs* pArgs)
{
	CryLogAlways("[FGPS] Version: %s", CURRENT_PLUGIN_VERSION);
}

////////////////////////////////////////////////////
void CFGPluginLoader::RegisterConsoleCommands()
{
	m_pConsole->AddCommand("fgps_show_version", CmdShowVersion, 0, "Shows the actual FGPS version");
	m_pConsole->AddCommand("fgps_dump_plugins", CmdDumpPlugins, 0, "Lists all loaded plugins to the console");
	// m_pConsole->AddCommand("fgps_dump_nodes", CmdDumpFGNodes, 0, "Output an XML file for all nodes defined in the Flowgraph Plugin System");
	m_pConsole->Register("fgps_debug", &fgps_debug, 0, 0, "Show FGPS debug informations");
}

////////////////////////////////////////////////////
void CFGPluginLoader::UnregisterConsoleCommands()
{
	m_pConsole->RemoveCommand("fgps_show_version");
	m_pConsole->RemoveCommand("fgps_dump_plugins");
	// m_pConsole->RemoveCommand("fgps_dump_nodes");
	m_pConsole->UnregisterVariable("fgps_debug", true);
}

void CFGPluginLoader::RegisterPlugins()
{
	ICryPak* pCryPak = gEnv->pCryPak;
	const string szModDir = pCryPak->GetModDir();

	// Parse plugins folder
	string szPlugins = szModDir;
	szPlugins += "FGPlugins";
#if !defined(WIN64)
	szPlugins += "\\bin32\\";
#else
	szPlugins += "\\bin64\\";
#endif //WIN32
	CryLogAlways("[FGPS] Scanning Plugins directory at \'%s\'", szPlugins.c_str());
	_finddata_t fileData;
	char szPluginPath[MAX_PATH];
	strncpy_s(szPluginPath, MAX_PATH, szPlugins.c_str(), MAX_PATH);
	strncat_s(szPluginPath, MAX_PATH, "*.dll", MAX_PATH);
	intptr_t hFile = pCryPak->FindFirst(szPluginPath, &fileData);
	if (hFile > -1)
	{
		do
		{
			m_nPluginCounter++;

			string dllPath = (szPlugins + fileData.name);
			CryLogAlways("---------------------------");
			CryLogAlways("[FGPS] [%d] Attempting to load plugin \'%s\'...", m_nPluginCounter, fileData.name);
			if (false == RegisterPlugin(dllPath.c_str()))
			{
				CryLogAlways("[Warning] [FGPS] [%d] Warning: Failed to load plugin!", m_nPluginCounter);
			}
		}
		while (pCryPak->FindNext(hFile, &fileData) > -1);
		pCryPak->FindClose(hFile);
	}
	CryLogAlways("---------------------------");
	CryLogAlways("[FGPS] Finished scanning for Plugins");

}

void CFGPluginLoader::Shutdown()
{
	if (gEnv->pConsole->GetCVar("fgps_debug"))
		CryLogAlways("[FGPS] GAME CLASS SHUTDOWN SEQUENCE STARTED...");

	if (!m_Plugins.empty())
	{
		std::vector<SFGPlugin>::iterator it;

		for (it = m_Plugins.begin(); it != m_Plugins.end(); it++)
		{
			SFGPlugin pP = *it;
			GetDataFunc pReleaseFunc = reinterpret_cast<GetDataFunc>(GetProcAddress(static_cast<HMODULE>(pP.hLib), PLUGIN_NAME_Release));

			if (pReleaseFunc)
				pReleaseFunc();
		}

		FreePluginLibraries();
	}

	//gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	//if (m_pModuleManager)
	//{
	//	m_pModuleManager->Shutdown();
	//}

	// Base shutdown
	//CGame::Shutdown();

	if (gEnv->pConsole->GetCVar("fgps_debug"))
		CryLogAlways("[FGPS] GAME CLASS SHUTDOWN SEQUENCE FINISHED...");

}
