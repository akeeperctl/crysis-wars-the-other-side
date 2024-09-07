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

	pConsole->Register("tos_sv_zeus_can_drag_dead_bodies", &tos_sv_zeus_can_drag_dead_bodies, 0, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_dragging_entities_auto_height", &tos_sv_zeus_dragging_entities_auto_height, 1, VF_CHEAT,
					   "0 - it will not \n1 - the height of the entity in 3D space will be automatically calculated for each selected entity");

	pConsole->Register("tos_sv_zeus_dragging_entities_height_type", &tos_sv_zeus_dragging_entities_height_type, 0, VF_CHEAT,
					   "0 - keep the start height \n1 - use the height of the mouse click point in 3D space");

	pConsole->Register("tos_sv_zeus_dragging_move_start_delay", &tos_sv_zeus_dragging_move_start_delay, 0.05f, VF_CHEAT,
					   "Delay in sec in starting to move entities after enabling drag and drop");

	pConsole->Register("tos_sv_zeus_force_on_screen_icons", &tos_sv_zeus_force_on_screen_icons, 1, VF_CHEAT,
					   "");
}

void CTOSZeusModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;
	pConsole->UnregisterVariable("tos_sv_zeus_mass_selection_hold_sec", true);
	pConsole->UnregisterVariable("tos_cl_zeus_dragging_draw_debug", true);
	pConsole->UnregisterVariable("tos_sv_zeus_always_select_parent", true);
	pConsole->UnregisterVariable("tos_sv_zeus_can_drag_dead_bodies", true);
	pConsole->UnregisterVariable("tos_sv_zeus_dragging_entities_auto_height", true);
	pConsole->UnregisterVariable("tos_sv_zeus_dragging_entities_height_type", true);
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