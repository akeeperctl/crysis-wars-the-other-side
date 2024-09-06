// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "StdAfx.h"

#include "TheOtherSideMP\Game\Modules\Zeus\ZeusModule.h"

void CTOSZeusModule::InitCVars(IConsole* pConsole)
{
	CTOSGenericModule::InitCVars(pConsole);

	pConsole->Register("tos_sv_zeus_mass_selection_hold_sec", &tos_sv_zeus_mass_selection_hold_sec, 0.2f, VF_CHEAT,
					   "Delay in sec, how long you need to hold LMB down before the multiple object selection mode is enabled");

	pConsole->Register("tos_cl_zeus_dragging_draw_debug", &tos_cl_zeus_dragging_draw_debug, 0, VF_CHEAT | VF_NOT_NET_SYNCED,
					   "ZEUS dragging draw debug");

	pConsole->Register("tos_sv_zeus_always_select_parent", &tos_sv_zeus_always_select_parent, 1, VF_CHEAT,
					   "When ZEUS selects child entities, only the parent will be selected");
}

void CTOSZeusModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;
	pConsole->UnregisterVariable("tos_sv_zeus_mass_selection_hold_sec", true);
	pConsole->UnregisterVariable("tos_cl_zeus_dragging_draw_debug", true);
	pConsole->UnregisterVariable("tos_sv_zeus_always_select_parent", true);
}

void CTOSZeusModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	//pConsole->AddCommand("tos_cmd_dumpmasterslist", CmdDumpMastersList);
}

void CTOSZeusModule::ReleaseCCommands()
{
	CTOSGenericModule::ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;
	//pConsole->RemoveCommand("tos_cmd_getmasterslist");

}