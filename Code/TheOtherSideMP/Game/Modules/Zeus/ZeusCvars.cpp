/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "StdAfx.h"

#include "TheOtherSideMP\Game\Modules\Zeus\ZeusModule.h"

void CTOSZeusModule::InitCVars(IConsole* pConsole)
{
	CTOSGenericModule::InitCVars(pConsole);

	pConsole->Register("tos_sv_zeus_update", &tos_sv_zeus_update, 1, VF_CHEAT,
					   "0 - update disabled \n1 - update enabled");

	pConsole->Register("tos_sv_zeus_mass_selection_hold_sec", &tos_sv_zeus_mass_selection_hold_sec, 0.2f, VF_CHEAT,
					   "Delay in sec, how long you need to hold LMB down before the multiple object selection mode is enabled");

	// ZEUS DRAGGING
	pConsole->Register("tos_sv_zeus_dragging_ignore_dead_bodies", &tos_sv_zeus_dragging_ignore_dead_bodies, 1, VF_CHEAT,
					   "");

	pConsole->Register("tos_cl_zeus_dragging_draw_debug", &tos_cl_zeus_dragging_draw_debug, 0, VF_CHEAT | VF_NOT_NET_SYNCED,
					   "ZEUS dragging draw debug");

	pConsole->Register("tos_sv_zeus_dragging_entities_auto_height", &tos_sv_zeus_dragging_entities_auto_height, 1, VF_CHEAT,
					   "0 - it will not \n1 - the height of the entity in 3D space will be automatically calculated for each selected entity");

	pConsole->Register("tos_sv_zeus_dragging_entities_height_type", &tos_sv_zeus_dragging_entities_height_type, 0, VF_CHEAT,
					   "0 - keep the start height \n1 - use the height of the mouse click point in 3D space");

	pConsole->Register("tos_sv_zeus_dragging_move_start_delay", &tos_sv_zeus_dragging_move_start_delay, 0.05f, VF_CHEAT,
					   "Delay in sec in starting to move entities after enabling drag and drop");

	//pConsole->Register("tos_sv_zeus_dragging_move_boxes_separately", &tos_sv_zeus_dragging_move_boxes_separately, 1, VF_CHEAT,
	//				   "0 - entities and selection boxes move together in realtime. Copying with 0 cause move bug \n1 - selection boxes move separately from entities");


	// ZEUS ON SCREEN ICONS
	pConsole->Register("tos_sv_zeus_on_screen_force_show", &tos_sv_zeus_on_screen_force_show, 1, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_on_screen_near_size", &tos_sv_zeus_on_screen_near_size, 1.4f, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_on_screen_far_size", &tos_sv_zeus_on_screen_far_size, 0.7f, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_on_screen_near_distance", &tos_sv_zeus_on_screen_near_distance, 10, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_on_screen_far_distance", &tos_sv_zeus_on_screen_far_distance, 500, VF_CHEAT,
					   "");

	pConsole->Register("tos_sv_zeus_on_screen_offsetX", &tos_sv_zeus_on_screen_offsetX, 0, VF_CHEAT,
					   "On screen icon 2d offset on X axis");

	pConsole->Register("tos_sv_zeus_on_screen_offsetY", &tos_sv_zeus_on_screen_offsetY, 0, VF_CHEAT,
					   "On screen icon 2d offset on Y axis");

	// ZEUS SELECTION
	pConsole->Register("tos_sv_zeus_selection_ignore_default", &tos_sv_zeus_selection_ignore_default, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class Default when selecting \n1 - zeus will ignore entities with class Default when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_basic_entity", &tos_sv_zeus_selection_ignore_basic_entity, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class BasicEntity when selecting \n1 - zeus will ignore entities with class BasicEntity when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_rigid_body", &tos_sv_zeus_selection_ignore_rigid_body, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class RigidBody and RigidBodyEx when selecting \n1 - zeus will ignore entities with class RigidBody and RigidBodyEx when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_destroyable_object", &tos_sv_zeus_selection_ignore_destroyable_object, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class DestroyableObject when selecting \n1 - zeus will ignore entities with class DestroyableObject when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_breakable_object", &tos_sv_zeus_selection_ignore_breakable_object, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class BreakableObject when selecting \n1 - zeus will ignore entities with class BreakableObject when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_anim_object", &tos_sv_zeus_selection_ignore_anim_object, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class AnimObject when selecting \n1 - zeus will ignore entities with class AnimObject when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_pressurized_object", &tos_sv_zeus_selection_ignore_pressurized_object, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class Pressurized when selecting \n1 - zeus will ignore entities with class Pressurized when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_switch", &tos_sv_zeus_selection_ignore_switch, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class Switch when selecting \n1 - zeus will ignore entities with class Switch when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_spawn_group", &tos_sv_zeus_selection_ignore_spawn_group, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class SpawnGroup when selecting \n1 - zeus will ignore entities with class SpawnGroup when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_interactive_entity", &tos_sv_zeus_selection_ignore_interactive_entity, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class InteractiveEntity when selecting \n1 - zeus will ignore entities with class InteractiveEntity when selecting");

	pConsole->Register("tos_sv_zeus_selection_ignore_vehicle_part_detached", &tos_sv_zeus_selection_ignore_vehicle_part_detached, 1, VF_CHEAT,
					   "0 - zeus will not ignore entities with class VehiclePartDetached when selecting \n1 - zeus will ignore entities with class VehiclePartDetached when selecting");

	pConsole->Register("tos_sv_zeus_selection_always_select_parent", &tos_sv_zeus_selection_always_select_parent, 1, VF_CHEAT,
					   "When ZEUS selects child entities, only the parent will be selected");
}

void CTOSZeusModule::ReleaseCVars()
{
	CTOSGenericModule::ReleaseCVars();

	const auto pConsole = gEnv->pConsole;
	pConsole->UnregisterVariable("tos_sv_zeus_update", true);
	pConsole->UnregisterVariable("tos_sv_zeus_mass_selection_hold_sec", true);

	pConsole->UnregisterVariable("tos_sv_zeus_dragging_ignore_dead_bodies", true);
	pConsole->UnregisterVariable("tos_sv_zeus_dragging_entities_auto_height", true);
	pConsole->UnregisterVariable("tos_sv_zeus_dragging_entities_height_type", true);
	pConsole->UnregisterVariable("tos_cl_zeus_dragging_draw_debug", true);

	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_near_size", true);
	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_far_size", true);
	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_near_distance", true);
	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_far_distance", true);
	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_offsetX", true);
	pConsole->UnregisterVariable("tos_sv_zeus_on_screen_offsetY", true);

	pConsole->UnregisterVariable("tos_sv_zeus_selection_always_select_parent", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_default", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_basic_entity", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_rigid_body", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_destroyable_object", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_breakable_object", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_anim_object", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_pressurized_object", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_switch", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_spawn_group", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_interactive_entity", true);
	pConsole->UnregisterVariable("tos_sv_zeus_selection_ignore_vehicle_part_detached", true);
}

void CTOSZeusModule::InitCCommands(IConsole* pConsole)
{
	CTOSGenericModule::InitCCommands(pConsole);

	pConsole->AddCommand("tos_cmd_reload_zeus_menu_items", CmdReloadMenuItems);
}

void CTOSZeusModule::ReleaseCCommands()
{
	CTOSGenericModule::ReleaseCCommands();

	const auto pConsole = gEnv->pConsole;
	pConsole->RemoveCommand("tos_cmd_reload_zeus_menu_items");
}

void CTOSZeusModule::CmdReloadMenuItems(IConsoleCmdArgs* pArgs)
{
	g_pTOSGame->GetZeusModule()->MenuLoadItems();
}