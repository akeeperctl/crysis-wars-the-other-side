#pragma once

/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// PluginCommon.h
//
// Purpose: Common functionality shared by plugins and system
//
// History:
//	- 6/01/08 : File created - KAK
/////////////////////////////////////////////////////////////////

#ifndef _PLUGINCOMMON_H_
#define _PLUGINCOMMON_H_

#include <CryVersion.h>
#include "GameCVars.h"
#include "Game.h"

class CG2AutoRegFlowNodeBase;

#ifdef PLUGIN_EXPORTS
	extern IGame* g_pGame;
#endif //PLUGIN_EXPORTS

// Current version
// Notes on version control: a.b.c.d
//	a = Master version, for generation control
//	b = Game version, for expansion control
//	c = Iteration version, for large changes that do not support backwards compatibility
//	d = Sub-version, for small change logging (no version control used at this level)
#define CURRENT_PLUGIN_VERSION ("1.1.0.5")
static SFileVersion PLUGIN_VERSION;

// Game dll resources for plugin declarations
enum EPluginResources
{
	ePR_Invalid = -1,

	// Begin custom entries
	ePR_GameFlashAnimationFactory,
	ePR_GameRules,
	// End custom entries

	// ALWAYS MUST BE LAST!
	ePR_Count,
};

// Registration table
struct SPluginRegister
{
	// Version information
	SFileVersion version;

	// Flownode registration list
	CG2AutoRegFlowNodeBase *nodesFirst;
	CG2AutoRegFlowNodeBase *nodesLast;

	SPluginRegister() : nodesFirst(NULL), nodesLast(NULL) {}
};

// Typedef for plug-in functions
typedef bool (*RegisterFunc)(ISystem *pSystem, SCVars *pCVars, SPluginRegister &outRegister);
typedef bool (*AddResourceFunc)(EPluginResources eResourceType, void *pResource);
typedef const char* (*GetDataFunc)();

// Plug-in function names
#define PLUGIN_NAME_RegisterFunc	("RegisterWithPluginSystem")
#define PLUGIN_NAME_AddResourceFunc	("AddExtendedResource")
#define PLUGIN_NAME_GetName			("GetName")
#define PLUGIN_NAME_GetAuthor		("GetAuthor")
#define PLUGIN_NAME_GetVersion		("GetVersionStr")
#define PLUGIN_NAME_GetNodeList		("GetNodeList")
#define PLUGIN_NAME_Release			("Release")

#endif //_PLUGINCOMMON_H_
