--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

function PipeManager:OnInitTOS()

    AI.BeginGoalPipe("tos_setup_combat_idle");	
	    AI.PushGoal("firecmd",0,0);	
	    AI.PushGoal("bodypos",0,BODYPOS_STAND);	
	    AI.PushGoal("run",0,0);
    AI.EndGoalPipe();

    AI.BeginGoalPipe("tos_simple_goto");
        AI.PushGoal("devalue",1);
        AI.PushGoal("bodypos",0,BODYPOS_STAND);
        AI.PushGoal("locate",1,"refpoint");
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("run",0,2);
        AI.PushGoal("approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.EndGoalPipe();

    --not work properly
    AI.BeginGoalPipe("squad_follow_leader_backoff");
        AI.PushGoal("locate",1,"refpoint");
        AI.PushGoal("approach",1,0.1,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
        --AI.PushGoal("+backoff",1,2,1,AI_LOOK_FORWARD+AI_MOVE_LEFT+AI_MOVE_RIGHT+AI_MOVE_BACKLEFT+AI_MOVE_BACKRIGHT+ AI_RANDOM_ORDER);
    AI.EndGoalPipe()

    AI.BeginGoalPipe("squad_follow_leader");
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("+bodypos",0,BODYPOS_STAND);
        AI.PushGoal("branch", 1, "START", BRANCH_ALWAYS);
        AI.PushLabel("START");
            AI.PushGoal("locate",1,"refpoint");
            AI.PushGoal("pathfind",1,"refpoint");
            AI.PushGoal("+branch", 1, "START", IF_NO_PATH);
            AI.PushGoal("+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
        AI.PushLabel("PATH_FOUND");
            AI.PushGoal("run",0,3);
            AI.PushGoal("stick",1,2.5,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.EndGoalPipe();

    AI.CreateGoalPipe("tos_detached_search_enemy");
    AI.PushGoal("tos_detached_search_enemy","+devalue",0);
    AI.PushGoal("tos_detached_search_enemy","+firecmd",0,0);
    AI.PushGoal("tos_detached_search_enemy","+bodypos",0,BODYPOS_STEALTH);
    AI.PushGoal("tos_detached_search_enemy","branch", 1, "START", BRANCH_ALWAYS);	
    AI.PushLabel("tos_detached_search_enemy","START");
        AI.PushGoal("tos_detached_search_enemy","locate",1,"refpoint");
        AI.PushGoal("tos_detached_search_enemy","pathfind",1,"refpoint");
        AI.PushGoal("tos_detached_search_enemy","+branch", 1, "LOOKAROUND", IF_NO_PATH);
        AI.PushGoal("tos_detached_search_enemy","+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
    AI.PushLabel("tos_detached_search_enemy","PATH_FOUND");
        AI.PushGoal("tos_detached_search_enemy","signal",0,1,"OnUnitMoving",SIGNALFILTER_LEADER);		
        AI.PushGoal("tos_detached_search_enemy","run",0,2);
        AI.PushGoal("tos_detached_search_enemy","approach",1,1.5,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
        AI.PushGoal("tos_detached_search_enemy","branch", 1, "LOOKAROUND", BRANCH_ALWAYS);
    AI.PushLabel("tos_detached_search_enemy","LOOKAROUND");
        AI.PushGoal("tos_detached_search_enemy","signal",0,1,"OnUnitStop",SIGNALFILTER_LEADER);
        AI.PushGoal("tos_detached_search_enemy","lookat", 1, -500);
        AI.PushGoal("tos_detached_search_enemy","lookaround",1,75,0.5,1,1.5,AI_BREAK_ON_LIVE_TARGET);
        AI.PushGoal("tos_detached_search_enemy","branch", 1, "START", BRANCH_ALWAYS);	

    AI.BeginGoalPipe("tos_detached_goto");
        AI.PushGoal("devalue",1);
        AI.PushGoal("bodypos",0,BODYPOS_STAND);
        AI.PushGoal("locate",1,"refpoint");
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("run",0,2);
        AI.PushGoal("approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.EndGoalPipe();

    AI.BeginGoalPipe("tos_detached_land_point_guard");
        AI.PushGoal("devalue",1);
        AI.PushGoal("bodypos",0,BODYPOS_STAND);
        AI.PushGoal("locate",1,"refpoint");
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("run",0,2);
        AI.PushGoal("approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.EndGoalPipe();

    AI.CreateGoalPipe("tos_rar_wait");
    AI.PushGoal("tos_rar_wait","devalue",1);
    AI.PushGoal("tos_rar_wait","bodypos",0,BODYPOS_STAND);
    AI.PushGoal("tos_rar_wait","locate",1,"refpoint");
    AI.PushGoal("tos_rar_wait","firecmd",0,0);
    AI.PushGoal("tos_rar_wait","run",0,2);
    AI.PushGoal("tos_rar_wait","approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);

    AI.CreateGoalPipe("tos_rar_goto");
    AI.PushGoal("tos_rar_goto","devalue",1);
    AI.PushGoal("tos_rar_goto","bodypos",0,BODYPOS_STAND);
    AI.PushGoal("tos_rar_goto","locate",1,"refpoint");
    AI.PushGoal("tos_rar_goto","firecmd",0,0);
    AI.PushGoal("tos_rar_goto","run",0,2);
    AI.PushGoal("tos_rar_goto","approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);

    AI.CreateGoalPipe("tos_commander_ord_goto");
    AI.PushGoal("tos_commander_ord_goto","devalue",1);
    AI.PushGoal("tos_commander_ord_goto","bodypos",0,BODYPOS_STAND);
    AI.PushGoal("tos_commander_ord_goto","locate",1,"refpoint");
    AI.PushGoal("tos_commander_ord_goto","firecmd",0,0);
    AI.PushGoal("tos_commander_ord_goto","run",0,2);
    AI.PushGoal("tos_commander_ord_goto","approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);

    -- AI.CreateGoalPipe("tos_commander_ord_goto_sub_hide");
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide", "strafe", 0, 100, 100);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","bodypos",0,BODYPOS_STEALTH);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","firecmd",0,0);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","run",0,2);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","signal",1,1,"TOS_AREA_GET_NEAREST_HIDESPOT", SIGNALFILTER_SENDER);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide", "locate", 0, "refpoint");
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","approach",1,0.1,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH, 0.3);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","timeout",1,0.3,1.0);
    -- AI.PushGoal("tos_commander_ord_goto_sub_hide","signal",1,1,"TOS_UNIT_INCOVER", SIGNALFILTER_SENDER);

    AI.BeginGoalPipe("tos_commander_ord_goto_sub_hide_incover");
        AI.PushGoal("bodypos",0,BODYPOS_STEALTH);
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("timeout",1,0.3,1.0);
        AI.PushGoal("lookaround",1,15,0.5,1,1.5,AI_BREAK_ON_LIVE_TARGET);
    AI.EndGoalPipe();

    AI.BeginGoalPipe("sqd_search_cover");
        AI.PushGoal("strafe", 0, 100, 100);
        AI.PushGoal("bodypos",0,BODYPOS_STAND);
        AI.PushGoal("firecmd",0,0);
        AI.PushGoal("run",0,2);
        -- get refpoint from c++
        AI.PushGoal("locate", 0, "refpoint");
        AI.PushGoal("approach",1,0.1,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH, 0.3);
        AI.PushGoal("timeout",1,0.3,1.0);
        AI.PushGoal("signal",1,1,"TOS_UNIT_INCOVER", SIGNALFILTER_SENDER);
    AI.EndGoalPipe();

    AI.CreateGoalPipe("ord_goto");
    AI.PushGoal("ord_goto","devalue",1);
    AI.PushGoal("ord_goto","bodypos",0,BODYPOS_STAND);
    AI.PushGoal("ord_goto","locate",1,"refpoint");
    AI.PushGoal("ord_goto","firecmd",0,0);
    AI.PushGoal("ord_goto","run",0,2);
    AI.PushGoal("ord_goto","approach",1,1.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.PushGoal("ord_goto","signal",1,1,"TOS_GOTO_GUARD",0);

    AI.CreateGoalPipe("ord_goto_vehicle");
    AI.PushGoal("ord_goto_vehicle","devalue",1);
    AI.PushGoal("ord_goto_vehicle","locate",1,"refpoint");
    AI.PushGoal("ord_goto_vehicle","firecmd",0,0);
    AI.PushGoal("ord_goto_vehicle","run",0,3);
    AI.PushGoal("ord_goto_vehicle","approach",1,3.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
    AI.PushGoal("ord_goto_vehicle","signal",1,1,"TOS_GOTO_GUARD",0);

    AI.CreateGoalPipe("ord_cooldown_goto");
    AI.PushGoal("ord_goto","signal",1,-1,"TOS_COOLDOWN_AND_GOTO",0);

    AI.CreateGoalPipe("ord_ready_guard");
    AI.PushGoal("ord_ready_guard","timeout",1,0.1,0.2);

    AI.CreateGoalPipe("tos_on_end_shooting");
    AI.PushGoal("tos_on_end_shooting","timeout",1,0.1,0.2);

    AI.CreateGoalPipe("ord_guard");
    AI.PushGoal("ord_guard","+bodypos",0,BODYPOS_STAND);
    AI.PushGoal("ord_guard","locate",1,"refpoint");
    AI.PushGoal("ord_guard","run",0,2);
    AI.PushGoal("ord_guard","approach",1,1.0,AILASTOPRES_USE +AI_REQUEST_PARTIAL_PATH);

    AI.CreateGoalPipe("ord_follow_player_vehicle");
    AI.PushGoal("ord_follow_player_vehicle","branch", 1, "START", BRANCH_ALWAYS);
    AI.PushLabel("ord_follow_player_vehicle","START");
        AI.PushGoal("ord_follow_player_vehicle","locate",1,"refpoint");
        --AI.PushGoal("ord_follow_player_vehicle","pathfind",1,"refpoint");
        AI.PushGoal("ord_follow_player_vehicle","+branch", 1, "START", IF_NO_PATH);
        AI.PushGoal("ord_follow_player_vehicle","+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
    AI.PushLabel("ord_follow_player_vehicle","PATH_FOUND");
        AI.PushGoal("ord_follow_player_vehicle","run",0,2);
        --AI.PushGoal("ord_follow_player_vehicle","approach",1,5.0,AI_REQUEST_PARTIAL_PATH);
        AI.PushGoal("ord_follow_player_vehicle", "stick", 1, 5.0, AI_ADJUST_SPEED, STICK_BREAK);
        AI.PushGoal("ord_follow_player_vehicle", "branch", 1, "START", IF_LASTOP_FAILED );

    AI.CreateGoalPipe("ord_follow_vehicle_onfoot");
    AI.PushGoal("ord_follow_vehicle_onfoot","clear",0,1);
    AI.PushGoal("ord_follow_vehicle_onfoot","devalue",0);
    AI.PushGoal("ord_follow_vehicle_onfoot","firecmd",0,0);
    AI.PushGoal("ord_follow_vehicle_onfoot","+bodypos",0,BODYPOS_STAND);
    AI.PushGoal("ord_follow_vehicle_onfoot","branch", 1, "START", BRANCH_ALWAYS);
    AI.PushLabel("ord_follow_vehicle_onfoot","START");
        AI.PushGoal("ord_follow_vehicle_onfoot","locate",1,"refpoint");
        AI.PushGoal("ord_follow_vehicle_onfoot","pathfind",1,"refpoint");
        AI.PushGoal("ord_follow_vehicle_onfoot","+branch", 1, "START", IF_NO_PATH);
        AI.PushGoal("ord_follow_vehicle_onfoot","+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
    AI.PushLabel("ord_follow_vehicle_onfoot","PATH_FOUND");
        AI.PushGoal("ord_follow_vehicle_onfoot","run",0,3);
        AI.PushGoal("ord_follow_vehicle_onfoot","stick",1,4.5,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);

    -- AI.CreateGoalPipe("ord_follow_player");
    -- AI.PushGoal("ord_follow_player","clear",0,1);
    -- AI.PushGoal("ord_follow_player","devalue",0);
    -- AI.PushGoal("ord_follow_player","firecmd",0,0);
    -- AI.PushGoal("ord_follow_player","+bodypos",0,BODYPOS_STAND);
    -- AI.PushGoal("ord_follow_player","branch", 1, "START", BRANCH_ALWAYS);
    -- AI.PushLabel("ord_follow_player","START");
    --     AI.PushGoal("ord_follow_player","locate",1,"refpoint");
    --     AI.PushGoal("ord_follow_player","pathfind",1,"refpoint");
    --     AI.PushGoal("ord_follow_player","+branch", 1, "START", IF_NO_PATH);
    --     AI.PushGoal("ord_follow_player","+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
    -- AI.PushLabel("ord_follow_player","PATH_FOUND");
    --     AI.PushGoal("ord_follow_player","run",0,3);
    --     AI.PushGoal("ord_follow_player","stick",1,2.5,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);

    AI.CreateGoalPipe("ord_follow_player_quickly");
    AI.PushGoal("ord_follow_player_quickly","ignoreall",1,1);
    AI.PushGoal("ord_follow_player_quickly","devalue",1);
    AI.PushGoal("ord_follow_player_quickly","firecmd",1,0);
    AI.PushGoal("ord_follow_player_quickly","bodypos",0,BODYPOS_STAND);
    AI.PushGoal("ord_follow_player_quickly","branch", 1, "START", BRANCH_ALWAYS);
    AI.PushLabel("ord_follow_player_quickly","START");
        AI.PushGoal("ord_follow_player_quickly","locate",1,"refpoint");
        AI.PushGoal("ord_follow_player_quickly","pathfind",1,"refpoint");
        AI.PushGoal("ord_follow_player_quickly","+branch", 1, "START", IF_NO_PATH);
        AI.PushGoal("ord_follow_player_quickly","+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
    AI.PushLabel("ord_follow_player_quickly","PATH_FOUND");
        AI.PushGoal("ord_follow_player_quickly","run",0,2);
        AI.PushGoal("ord_follow_player_quickly","stick",1,2.0,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH, STICK_BREAK,0,1.0);
        AI.PushGoal("ord_follow_player_quickly","ignoreall",1,0);
        AI.PushGoal("ord_follow_player_quickly","branch", 1, "END", BRANCH_ALWAYS);
    AI.PushLabel("ord_follow_player_quickly","END");
        AI.PushGoal("ord_follow_player_quickly","signal",0,-1,"TOS_QUICKLY_FOLLOW_END",0);

    AI.BeginGoalPipe("squad_search_enemy"); --for scout(0.5sec) -- РЅР° СЃРєР°СѓС‚Рµ СЃ РіСЂСѓРїРїРѕР№ РЅРµ РїСЂРѕРІРµСЂСЏР»
        AI.PushGoal("+firecmd",0,0);
        AI.PushGoal("+bodypos",0,BODYPOS_STEALTH);
        AI.PushGoal("branch", 1, "START", BRANCH_ALWAYS);	
        AI.PushLabel("START");
            AI.PushGoal("signal",0,1,"GET_RANDOM_POINT",SIGNALFILTER_SENDER);		
            AI.PushGoal("locate",1,"refpoint");
            AI.PushGoal("pathfind",1,"refpoint");
            AI.PushGoal("+branch", 1, "START", IF_NO_PATH);
            AI.PushGoal("+branch", 1, "PATH_FOUND", NOT+IF_NO_PATH);	
        AI.PushLabel("PATH_FOUND");
            --AI.PushGoal("signal",0,1,"OnUnitMoving",SIGNALFILTER_LEADER);		
            AI.PushGoal("run",0,2);
            AI.PushGoal("approach",1,1.5,AILASTOPRES_USE + AI_REQUEST_PARTIAL_PATH);
            AI.PushGoal("branch", 1, "LOOKAROUND", BRANCH_ALWAYS);
        AI.PushLabel("LOOKAROUND");
            AI.PushGoal("signal",0,1,"OnUnitStop",SIGNALFILTER_LEADER);
            AI.PushGoal("lookat", 1, -500);
            AI.PushGoal("lookaround",1,75,0.5,1,1.5,AI_BREAK_ON_LIVE_TARGET);
            AI.PushGoal("branch", 1, "START", BRANCH_ALWAYS);
    AI.EndGoalPipe();


    AI.BeginGoalPipe("ord_cooldown"); --for scout(0.5sec) -- РЅР° СЃРєР°СѓС‚Рµ СЃ РіСЂСѓРїРїРѕР№ РЅРµ РїСЂРѕРІРµСЂСЏР»
        AI.PushGoal("firecmd",1,0);
        AI.PushGoal("bodypos",1,BODYPOS_STAND);
        AI.PushGoal("branch", 1, "END", BRANCH_ALWAYS);
        AI.PushLabel("END");
            AI.PushGoal("signal",1,-1,"RETURN_TO_FIRST",0); -- ideal work on scouts (0.5)
            AI.PushGoal("clear",1,1); -- clear РІСЃРµРіРґР° РЅСѓР¶РЅРѕ СЃС‚Р°РІРёС‚СЊ РІ РєРѕРЅРµС† goalpipe
    AI.EndGoalPipe();

    AI.BeginGoalPipe("ord_cooldown_trooper");--for trooper(1.5sec) --РЅРµ СЂР°Р±РѕС‚Р°РµС‚ РЅРѕСЂРјР°Р»СЊРЅРѕ СЃ РіСЂСѓРїРїРѕР№
        AI.PushGoal("clear",1,1);
        AI.PushGoal("firecmd",1,0);
        AI.PushGoal("bodypos",1,BODYPOS_STAND);
        AI.PushGoal("branch", 1, "END", BRANCH_ALWAYS);
        AI.PushLabel("END");
            AI.PushGoal("signal",1,-1,"RETURN_TO_FIRST",0); -- ideal work on scouts (0.5)
            --AI.PushGoal("signal",1,-1,"TOS_ONRESET",0)
            --AI.PushGoal("clear",1,0); -- clear РІСЃРµРіРґР° РЅСѓР¶РЅРѕ СЃС‚Р°РІРёС‚СЊ РІ РєРѕРЅРµС† goalpipe
            --AI.PushGoal("clear",1,1); -- clear РІСЃРµРіРґР° РЅСѓР¶РЅРѕ СЃС‚Р°РІРёС‚СЊ РІ РєРѕРЅРµС† goalpipe
            --AI.PushGoal("clear",1,1);
    AI.EndGoalPipe();
end