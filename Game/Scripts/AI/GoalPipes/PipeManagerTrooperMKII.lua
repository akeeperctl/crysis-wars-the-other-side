
TROOPER_CLOSE_DISTANCE_STICK =2.2;
TROOPER_VEHICLE_DISTANCE_STICK = 5.6;

function PipeManager:OnInitTrooperMKII() 
	AI.LogEvent("TROOPERMKIICALLED");
	
-------------------------------------------------------
-- Dodge
-------------------------------------------------------
	AI.BeginGoalPipe("tr2_dodge_right_short_anim_rand");
		AI.BeginGroup();
			AI.PushGoal("branch", 1, "OTHER_DIR", IF_RANDOM, 0.5);
				AI.PushGoal("animation", 0, AIANIM_SIGNAL, "dodgeShortRight");
				AI.PushGoal("branch", 1, "DONE", BRANCH_ALWAYS);
			AI.PushLabel("OTHER_DIR");
				AI.PushGoal("animation", 0, AIANIM_SIGNAL, "dodgeShortLeft");
				AI.PushGoal("branch", 1, "DONE", BRANCH_ALWAYS);
			AI.PushLabel("DONE");
			AI.PushGoal("timeout", 0, 2);--fallback, but it should never fail
		AI.EndGroup();
		AI.PushGoal("wait", 1, WAIT_ANY);
		AI.PushGoal("signal",0,1,"TR2_DODGE_DONE",0);	
	AI.EndGoalPipe();
-------------------------------------------------------
	AI.BeginGoalPipe("tr2_dodge_right_short_anim_right");
		AI.BeginGroup();
			AI.PushGoal("animation", 0, AIANIM_SIGNAL, "dodgeShortRight");
			AI.PushGoal("timeout", 0, 2);--fallback, but it should never fail
		AI.EndGroup();
		AI.PushGoal("wait", 1, WAIT_ANY);
		AI.PushGoal("signal",0,1,"TR2_DODGE_DONE",0);	
	AI.EndGoalPipe();
-------------------------------------------------------
	AI.BeginGoalPipe("tr2_dodge_right_short_anim_left");
		AI.BeginGroup();
			AI.PushGoal("animation", 0, AIANIM_SIGNAL, "dodgeShortLeft");
			AI.PushGoal("timeout", 0, 2);--fallback, but it should never fail
		AI.EndGroup();
		AI.PushGoal("wait", 1, WAIT_ANY);
		AI.PushGoal("signal",0,1,"TR2_DODGE_DONE",0);	
	AI.EndGoalPipe();
	
-------------------------------------------------------
-- Check fallen
-------------------------------------------------------
	AI.CreateGoalPipe("tr2_check_dead");
--	AI.PushGoal("tr2_check_dead","acqtarget",0,"");
--	AI.PushGoal("tr2_check_dead","timeout",1,0.2,0.4);
--	AI.PushGoal("tr2_check_dead","lookat",1,0,0,1);
--	AI.PushGoal("tr2_check_dead","timeout",1,0.2,0.4);
	AI.PushGoal("tr2_check_dead","bodypos",0,BODYPOS_STAND);	
	AI.PushGoal("tr2_check_dead","run",0,2);		
	AI.PushGoal("tr2_check_dead","locate",0,"refpoint");	
	AI.PushGoal("tr2_check_dead","approach",0,2,AILASTOPRES_USE);
	-- loop until the target is visible
	AI.PushLabel("tr2_check_dead", "VISIBLE_LOOP");
		AI.PushGoal("tr2_check_dead","locate",0,"refpoint");	
		AI.PushGoal("tr2_check_dead","branch", 1, "TARGET_VISIBLE", IF_SEES_LASTOP, 50.0);
		AI.PushGoal("tr2_check_dead","branch", 1, "VISIBLE_LOOP", IF_ACTIVE_GOALS);	
	-- If the following gets executed the approach finished already while approaching fast.	
	AI.PushGoal("tr2_check_dead","branch", 1, "APPROACH_DONE", BRANCH_ALWAYS);
	-- approach more cautiously	
	AI.PushLabel("tr2_check_dead", "TARGET_VISIBLE");
	-- NO STEALTH POS FOR TROOPERS!
--	AI.PushGoal("tr2_check_dead","bodypos",0,BODYPOS_STEALTH);	
	AI.PushGoal("tr2_check_dead","bodypos",0,BODYPOS_STAND);	
	AI.PushGoal("tr2_check_dead","run",0,1);
--	AI.PushGoal("tr2_check_dead","lookat",0,0,0,1);
--	AI.PushGoal("tr2_check_dead","timeout",1,0.3,0.8);
	AI.PushLabel("tr2_check_dead", "SEEK_LOOP");
		AI.PushGoal("tr2_check_dead","lookat",0,-90,90,0,1);
		AI.PushGoal("tr2_check_dead","timeout",1,.61,.73);
		AI.PushGoal("tr2_check_dead","lookat",1,-790);	
	AI.PushGoal("tr2_check_dead","branch", 0, "SEEK_LOOP", IF_ACTIVE_GOALS);	
	AI.PushLabel("tr2_check_dead", "APPROACH_DONE");
	-- At the target, check it.	
	AI.PushGoal("tr2_check_dead","bodypos",0,BODYPOS_CROUCH);	
	AI.PushGoal("tr2_check_dead","signal",1,1,"CHECKING_DEAD",0);
	AI.PushGoal("tr2_check_dead","locate",0,"beacon");	
	AI.PushGoal("tr2_check_dead","lookat",1,0,0,1);
	AI.PushGoal("tr2_check_dead","timeout",1,1,2);	
	AI.PushGoal("tr2_check_dead","lookat",0,-90,90);
	AI.PushGoal("tr2_check_dead","timeout",1,0.6,0.8);	
	AI.PushGoal("tr2_check_dead","lookat",0,-90,90);
	AI.PushGoal("tr2_check_dead","timeout",1,0.6,0.8);
	AI.PushGoal("tr2_check_dead","signal",1,1,"BE_ALERTED",0);
-------------------------------------------------------	
	AI.BeginGoalPipe("tr2_check_sleeping");
		AI.PushGoal("bodypos",0,BODYPOS_STAND);	
		AI.PushGoal("run",0,2);		
		AI.PushGoal("locate",0,"refpoint");	
		AI.PushGoal("approach",0,2,AILASTOPRES_USE);
		-- loop until the target is visible
		AI.PushLabel("VISIBLE_LOOP");
			AI.PushGoal("locate",0,"refpoint");	
			AI.PushGoal("branch", 1, "TARGET_VISIBLE", IF_SEES_LASTOP, 50.0);
			AI.PushGoal("branch", 1, "VISIBLE_LOOP", IF_ACTIVE_GOALS);	
		-- If the following gets executed the approach finished already while approaching fast.	
		AI.PushGoal("branch", 1, "APPROACH_DONE", BRANCH_ALWAYS);
		-- approach more cautiously	
		AI.PushLabel("TARGET_VISIBLE");
		-- NO STEALTH FOR TROOPER!
		--AI.PushGoal("bodypos",0,BODYPOS_STEALTH);
		AI.PushGoal("bodypos",0,BODYPOS_STAND);
		AI.PushGoal("run",0,1);
		AI.PushLabel( "SEEK_LOOP");
			AI.PushGoal("lookat",0,-90,90,0,1);
			AI.PushGoal("timeout",1,.61,.73);
			AI.PushGoal("lookat",1,-790);	
		AI.PushGoal("branch", 0, "SEEK_LOOP", IF_ACTIVE_GOALS);	
		AI.PushLabel("APPROACH_DONE");
		-- At the target, check it.	
		AI.PushGoal("bodypos",0,BODYPOS_CROUCH);	
		AI.PushGoal("signal",1,1,"CHECKING_DEAD",0);
		AI.PushGoal("locate",0,"beacon");	
		AI.PushGoal("lookat",1,0,0,1);
		AI.PushGoal("timeout",1,1,2);	
		AI.PushGoal("lookat",0,-90,90);
		AI.PushGoal("timeout",1,0.6,0.8);	
		AI.PushGoal("lookat",0,-90,90);
		AI.PushGoal("timeout",1,0.6,0.8);
		AI.PushGoal("signal",1,1,"CHECK_DONE",0);
	AI.EndGoalPipe();
	
-------------------------------------------------------
-- Check fallen
-------------------------------------------------------	
	AI.CreateGoalPipe("tr2_check_it_out");
	-- NO STEALTH POS FOR TROOPERS!
	--AI.PushGoal("tr2_check_it_out","bodypos",0,BODYPOS_STEALTH);
	AI.PushGoal("tr2_check_it_out","bodypos",0,BODYPOS_STAND);
	AI.PushGoal("tr2_check_it_out","lookat",1,-90,90);
	AI.PushGoal("tr2_check_it_out","lookat",1,-90,90);
	AI.PushGoal("tr2_check_it_out","bodypos",0,BODYPOS_STAND);
-------------------------------------------------------	

-------------------------------------------------------
-- Cover 2 pipes converted
-------------------------------------------------------

	---------------------------------------------------
	-- Advance compatibility (actually these might be just communication anims)
	---------------------------------------------------
	AI.BeginGoalPipe("tr2_signal_advance_left");
		AI.PushGoal("firecmd", 0, 0);
		AI.PushGoal("signal",1,1,"move_command",SIGNALID_READIBILITY,115);
		--AI.PushGoal("animation",1,AIANIM_SIGNAL,"signalAdvanceLeft");
		AI.PushGoal("signal",1,1,"COVER_NORMALATTACK",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_signal_advance_right");
		AI.PushGoal("firecmd", 0, 0);
		AI.PushGoal("signal",1,1,"move_command",SIGNALID_READIBILITY,115);
		--AI.PushGoal("animation",1,AIANIM_SIGNAL,"signalAdvanceRight");
		AI.PushGoal("signal",1,1,"COVER_NORMALATTACK",0);
	AI.EndGoalPipe();

	---------------------------------------------------
	-- Stealth compatibility
	---------------------------------------------------

	AI.BeginGoalPipe("tr2_refpoint_investigate");
		AI.PushGoal("bodypos",0,BODYPOS_STAND,1);
		AI.PushGoal("run",0,1);
		AI.PushGoal("strafe",0,1,2);
		AI.PushGoal("firecmd",0,0);	
		AI.PushGoal("lookaround",0,45,3,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,7,0,15.0);
		AI.PushGoal("clear",0,0);
		AI.PushGoal("firecmd",0,1);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",0,BODYPOS_STEALTH,1);
		AI.PushGoal("bodypos",0,BODYPOS_STAND,1);
		AI.PushGoal("lookaround",0,45,2,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,1,0,15.0);
		AI.PushGoal("signal",1,1,"LOOK_FOR_TARGET",0);
	AI.EndGoalPipe();
	
	---------------------------------------------
	AI.BeginGoalPipe("tr2_investigate_probable_target");
		AI.PushGoal("bodypos",0,BODYPOS_STAND,1);
		AI.PushGoal("run",0,1);
		AI.PushGoal("strafe",0,1,2);
		AI.PushGoal("firecmd",0,0);	
		AI.PushGoal("lookaround",0,45,3,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"probtarget_in_territory");
		AI.PushGoal("+approach",1,7,AILASTOPRES_USE,15.0);
		AI.PushGoal("clear",0,0);
		AI.PushGoal("firecmd",0,1);
		AI.PushGoal("run",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",0,BODYPOS_STEALTH,1);
		AI.PushGoal("bodypos",0,BODYPOS_STAND,1);
		AI.PushGoal("lookaround",0,45,2,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"probtarget_in_territory");
		AI.PushGoal("+approach",1,2,AILASTOPRES_USE,15.0);
		AI.PushGoal("signal",1,1,"LOOK_FOR_TARGET",0);
	AI.EndGoalPipe();
	
	---------------------------------------------
	AI.BeginGoalPipe("tr2_investigate_threat_closer");
		AI.PushGoal("run",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH,1);	
		AI.PushGoal("bodypos",1,BODYPOS_STAND,1);	
		AI.PushGoal("firecmd",0,FIREMODE_AIM);

		AI.PushGoal("locate",0,"refpoint");
--		AI.PushGoal("+approach",1,5,AILASTOPRES_USE,15.0, 3);
		AI.PushGoal("+stick",1,5,AILASTOPRES_USE,STICK_BREAK,10.0, 3);

--		AI.PushGoal("locate",0,"refpoint");
--		AI.PushGoal("+stick",1,2,AILASTOPRES_USE,STICK_BREAK,10.0);

--		AI.PushGoal("locate", 1, "probtarget_in_territory");
--		AI.PushGoal("+approach",1,4,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
--		AI.PushGoal("clear",0,0);
--		AI.PushGoal("lookaround",0,20,3,100,100,AI_BREAK_ON_LIVE_TARGET);
--		AI.PushGoal("bodypos",1,BODYPOS_CROUCH);	
--		AI.PushGoal("locate", 1, "probtarget_in_territory");
--		AI.PushGoal("+approach",1,1,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
		AI.PushGoal("clear",0,0);
		AI.PushGoal("lookaround",1,120,3,1,3,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("signal",0,1,"INVESTIGATE_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_investigate_threat_far");
		AI.PushGoal("strafe",0,1.5,20);
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("lookaround",1,30,3,1.5,2,AI_BREAK_ON_LIVE_TARGET);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH,1);	
		AI.PushGoal("bodypos",1,BODYPOS_STAND,1);
		AI.PushGoal("run",0,1,0,20);
		AI.PushGoal("locate", 1, "probtarget_in_territory");
		AI.PushGoal("+approach",1,10,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
		AI.PushGoal("bodypos",1,BODYPOS_CROUCH);
		AI.PushGoal("lookaround",1,45,4,2,6,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("signal",0,1,"INVESTIGATE_DONE",0);
	AI.EndGoalPipe();
	
	---------------------------------------------
	AI.BeginGoalPipe("tr2_investigate_threat_distance");
		AI.PushGoal("strafe",0,1.5,3);
		AI.PushGoal("firecmd",0,1);	
		AI.PushGoal("lookaround",1,45,4,1.5,2,AI_BREAK_ON_LIVE_TARGET);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);	
		AI.PushGoal("bodypos",1,BODYPOS_STAND);	
		AI.PushGoal("run",0,1);
		AI.PushGoal("lookaround",0,10,3,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,15,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
		AI.PushGoal("clear",0,0);
		AI.PushGoal("lookaround",0,20,2,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("run",0,0);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,7,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
		AI.PushGoal("clear",0,0);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,2,AILASTOPRES_USE+AILASTOPRES_LOOKAT,15.0);
		AI.PushGoal("bodypos",1,BODYPOS_CROUCH);
		AI.PushGoal("lookaround",1,45,3,2,4,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("signal",0,1,"INVESTIGATE_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_seek_target_random");
		AI.PushGoal("run",0,0);
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("strafe",0,4,10);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushGoal("locate", 1, "probtarget_in_territory");
		AI.PushGoal("+hide", 1, 25, HM_RANDOM+HM_AROUND_LASTOP+HM_INCLUDE_SOFTCOVERS, 1); -- lookat hide
		AI.PushGoal("branch", 1, "HIDE_OK", IF_LASTOP_SUCCEED);
			AI.PushGoal("signal",0,1,"HIDE_FAILED",0);
		AI.PushLabel("HIDE_OK");

--		AI.PushGoal("firecmd",0,FIREMODE_AIM);
		AI.PushGoal("lookaround",1,120,3,1,3,AI_BREAK_ON_LIVE_TARGET);

		AI.PushGoal("signal",0,1,"LOOK_FOR_TARGET",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_seek_target_nocover");

		AI.PushGoal("run",0,0);
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("strafe",0,4,10);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);

		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,-15,AILASTOPRES_USE,15,"",8);

		AI.PushGoal("lookaround",1,120,3,1,3,AI_BREAK_ON_LIVE_TARGET);

		AI.PushGoal("signal",0,1,"LOOK_FOR_TARGET",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_backoff_from_explosion");
		AI.PushGoal("firecmd",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH,1);
		AI.PushGoal("bodypos",1,BODYPOS_STAND,1);
		AI.PushGoal("firecmd",0,FIREMODE_BURST_WHILE_MOVING);
		AI.PushGoal("run",0,2);
		AI.PushGoal("strafe",0,2,2);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,-11,AILASTOPRES_USE+AI_REQUEST_PARTIAL_PATH,15,"",4);
		AI.PushGoal("run",0,0);
		AI.PushGoal("signal",1,1,"END_BACKOFF",SIGNALFILTER_SENDER);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_backoff_from_explosion_short");
		AI.PushGoal("firecmd",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH,1);
		AI.PushGoal("bodypos",1,BODYPOS_STAND,1);
		AI.PushGoal("firecmd",0,FIREMODE_BURST_WHILE_MOVING);
		AI.PushGoal("run",0,2);
		AI.PushGoal("strafe",0,2,2);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,-7,AILASTOPRES_USE+AI_REQUEST_PARTIAL_PATH,15,"",2);
		AI.PushGoal("run",0,0);
		AI.PushGoal("signal",1,1,"END_BACKOFF",SIGNALFILTER_SENDER);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_fast_advance_to_target");
		AI.PushGoal("firecmd",0,FIREMODE_BURST_WHILE_MOVING);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushGoal("run",0,2,0,25);
		-- at this point refpoint equals beacon.
		AI.PushGoal( "branch", 1, "NO_APPROACH", IF_TARGET_TO_REFPOINT_DIST_LESS, 12.0);
			AI.PushGoal("locate",0,"refpoint");
			AI.PushGoal("+approach",1,10,AILASTOPRES_USE);
		AI.PushLabel("NO_APPROACH");
		-- sets the refpoint to the point to advance to
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushGoal("signal",0,1,"SELECT_ADVANCE_POINT",0);
--		AI.PushGoal("timeout",1,0.15);
		AI.PushGoal("strafe",0,0,4);
		AI.PushGoal("lookaround",0,20,3,100,100,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("run",0,1,0,5);
		AI.PushGoal("+approach",1,1,AILASTOPRES_USE,5.0,"ADVANCE_NOPATH");
		AI.PushGoal("clear",0,0);
--		AI.PushGoal("run",0,0);
--		AI.PushGoal("strafe",0,0,1.5);
--		AI.PushGoal("firecmd",0,1);
		
--		AI.PushGoal("locate",0,"probtarget");
--		AI.PushGoal("adjustaim",0,0,1);
--		AI.PushGoal("timeout",1,1,1.5);
--		AI.PushGoal("clear",0,0);
		
--		AI.PushGoal("locate",0,"probtarget");
--		AI.PushGoal("usecover",1,COVER_UNHIDE,1.5,2,1);
--
--		AI.PushGoal("branch", 1, "SKIP_STAND", IF_EXPOSED_TO_TARGET, 5, 0.5);	
--			-- NO STEALTH POS FOR TROOPERS!
--			--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
--			AI.PushGoal("bodypos",1,BODYPOS_STAND);
--			AI.PushGoal("timeout",1,1,1.5);
--		AI.PushLabel("SKIP_STAND");

		AI.PushGoal("signal",0,1,"COVER_NORMALATTACK",0);
	AI.EndGoalPipe();


	---------------------------------------------
	AI.BeginGoalPipe("tr2_use_cover_safe");
		AI.PushGoal("firecmd",0,FIREMODE_BURST_WHILE_MOVING);
--		AI.PushGoal("check_cover_fire",1);
		AI.PushGoal("run",0,2,0,20);
		AI.PushGoal( "branch", 1, "DO_HIDE", IF_COVER_SOFT);
		AI.PushGoal( "branch", 1, "SKIP_HIDE", IF_COVER_NOT_COMPROMISED);
		AI.PushLabel("DO_HIDE");
			-- NO STEALTH POS FOR TROOPERS!
			--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
			AI.PushGoal("bodypos",1,BODYPOS_STAND);
			AI.PushGoal("strafe",0,1,2);
			AI.PushGoal("hide",1,15,HM_NEAREST);
		AI.PushLabel("SKIP_HIDE");
		
--		AI.PushGoal("locate",0,"probtarget");
--		AI.PushGoal("strafe",0,0,2);
--		AI.PushGoal("run",0,0);
--		AI.PushGoal("usecover",1,COVER_HIDE,1,1.5,1);
--		AI.PushGoal("firecmd",0,1);
--		AI.PushGoal("signal",0,1,"NOTIFY_COVERING",0);
--		AI.PushGoal("usecover",1,COVER_UNHIDE,4,6,1,1);
				
		AI.PushGoal("run",0,0);
		AI.PushGoal("firecmd",0,1);
		AI.PushGoal( "branch", 1, "SKIP_USECOVER", IF_COVER_COMPROMISED);
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("strafe",0,1,2);
			AI.PushGoal("signal",0,1,"NOTIFY_COVERING",0);
			AI.PushGoal("usecover",1,COVER_UNHIDE,5,7,1,1);
			AI.PushGoal("usecover",1,COVER_HIDE,0.5,1,1);
			AI.PushGoal("branch", 1, "END_STANCE", BRANCH_ALWAYS);
		AI.PushLabel("SKIP_USECOVER");
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("adjustaim",0,0,1);
			AI.PushGoal("timeout",1,1.0,2.0);
			AI.PushGoal("clear",0,0);
		AI.PushLabel("DONE");

		AI.PushGoal("signal",0,1,"COVER_NORMALATTACK",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_take_cover_reload");
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("branch", 1, "SKIP_STAND", IF_EXPOSED_TO_TARGET, 5, 0.15);
			-- Duck
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("+adjustaim",0,1,1); --hide
			AI.PushGoal("timeout",1,0.3);
			AI.PushGoal("branch", 1, "DONE", BRANCH_ALWAYS);
		AI.PushLabel("SKIP_STAND");
			-- Hide & duck
			AI.PushGoal("firecmd",0,0);
			AI.PushGoal("run", 0, 2, 0, 20);
			-- NO STEALTH POS FOR TROOPERS!
			--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
			AI.PushGoal("bodypos",1,BODYPOS_STAND);
			AI.PushGoal("strafe",0,1,2);
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("+seekcover", 1, COVER_HIDE, 7.0, 3, 1);
--				AI.PushGoal("+seekcover", 1, COVER_HIDE, 4, 1, 1);
			AI.PushGoal("+adjustaim",0,1,1); --hide
			AI.PushGoal("timeout",1,0.3);
		AI.PushLabel("DONE");
		AI.PushGoal("run",0,0);
		AI.PushGoal("signal",0,1,"MKII_HANDLE_RELOAD",0);
	AI.EndGoalPipe();

	

	---------------------------------------------
	AI.BeginGoalPipe("tr2_look_closer_standby");
		AI.PushGoal("firecmd",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushGoal("run",0,1,0,4);
		AI.PushGoal("strafe",0,2,2);
--		AI.PushGoal("locate",0,"probtarget_in_territory_and_refshape");
		AI.PushGoal("locate",0,"refpoint");
		AI.PushGoal("+approach",1,3,AILASTOPRES_USE,10.0,"",3);
--		AI.PushGoal("locate",0,"probtarget");
--		AI.PushGoal("+seekcover", 1, COVER_UNHIDE, 7.0, 3, 1);

--		AI.PushGoal("hide",1,10,HM_NEAREST+HM_AROUND_LASTOP+HM_INCLUDE_SOFTCOVERS);
		AI.PushGoal("lookaround",0,45,3,1,2,AI_BREAK_ON_LIVE_TARGET);
		AI.PushGoal("signal",0,1,"APPROACH_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_target_cloak_reaction");
		AI.PushGoal("signal",1,1,"confused",SIGNALID_READIBILITY,115);

		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("timeout",1,0,0.3);
		
		AI.PushGoal("branch", 1, "DONT_SHOOT", IF_RANDOM, 0.5);
			AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushLabel("DONT_SHOOT");
			-- NO STEALTH POS FOR TROOPERS!
			--AI.PushGoal("bodypos",1,BODYPOS_STEALTH);
			AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushLabel("DONE_SETUP");
		
		AI.PushGoal("firecmd",0, FIREMODE_PANIC_SPREAD);
		AI.PushGoal("timeout",1,1,3);
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("bodypos",1,BODYPOS_STAND);
		AI.PushGoal("run",0,2,1,10);
		AI.PushGoal("strafe",0,2,0);
		AI.PushGoal("+seekcover", 1, COVER_HIDE, 5.0, 3, 1);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	--
	--	RPG vs TANK related pipes
	--
	------------------------------------------------------------------------------------------

	---------------------------------------------
	AI.BeginGoalPipe("tr2_seek");

		AI.PushGoal("branch", 1, "SETUP_SLOW", IF_TARGET_DIST_LESS, 15.0);
			AI.PushGoal("run",0,1);
			AI.PushGoal("bodypos",1,BODYPOS_STEALTH, 1);
			--AI.PushGoal("bodypos",1,BODYPOS_STAND, 1);
			AI.PushGoal("firecmd",0,0);
			AI.PushGoal("strafe",0,2,2);
			AI.PushGoal("firecmd",0,FIREMODE_AIM);
			AI.PushGoal("branch", 1, "SETUP_DONE", BRANCH_ALWAYS);
		AI.PushLabel("SETUP_SLOW");
			AI.PushGoal("run",0,0);
			-- NO STEALTH POS FOR TROOPERS!
			AI.PushGoal("bodypos",1,BODYPOS_STEALTH, 1);
			--AI.PushGoal("bodypos",1,BODYPOS_STAND, 1);
			AI.PushGoal("firecmd",0,FIREMODE_AIM);
			AI.PushGoal("strafe",0,4,4);
			AI.PushGoal("locate",0,"refpoint");
			AI.PushGoal("+lookat",0,0,0,1);
		AI.PushLabel("SETUP_DONE");

		AI.PushGoal("branch", 1, "APPROACH_FAR", IF_TARGET_DIST_LESS, 30.0);
			-- move to group pos 
			AI.PushGoal("locate",0,"refpoint");
			AI.PushGoal("+approach",1,-15,AILASTOPRES_USE,15,"",8);
			AI.PushGoal("branch", 1, "APPROACH_DONE", BRANCH_ALWAYS);
		AI.PushLabel("APPROACH_FAR");
			-- move closer to target
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("+approach",1,29,AILASTOPRES_USE,15,"",10);
		AI.PushLabel("APPROACH_DONE");

		AI.PushGoal("branch", 1, "SKIP_UNHIDE", IF_SEES_TARGET, 20.0);
			AI.PushGoal("locate",0,"probtarget");
			AI.PushGoal("+seekcover", 1, COVER_UNHIDE, 3.0, 2, 1);
		AI.PushLabel("SKIP_UNHIDE");

--		AI.PushGoal("clear",0,0); -- kill lookat

		AI.PushGoal("signal",1,1,"COVER_NORMALATTACK",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_seek_direct");
		AI.PushGoal("run",0,1);
		AI.PushGoal("bodypos",1,BODYPOS_STAND, 1);
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("strafe",0,2,2,1);
		AI.PushGoal("approach",1,10,0,15,"",3);

		AI.PushGoal("run",0,0);
		-- NO STEALTH POS FOR TROOPERS!
		--AI.PushGoal("bodypos",1,BODYPOS_STEALTH, 1);
		AI.PushGoal("bodypos",1,BODYPOS_STAND, 1);
		AI.PushGoal("firecmd",0,FIREMODE_AIM);
		AI.PushGoal("strafe",0,4,4,1);
		AI.PushGoal("approach",1,2,0,15,"",2);

		AI.PushGoal("locate",0,"probtarget");
		AI.PushGoal("+seekcover", 1, COVER_UNHIDE, 3.0, 2, 1);

		AI.PushGoal("signal",1,1,"SEEK_DIRECT_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_panic_left");
		AI.PushGoal("firecmd",0,0);
--			AI.PushGoal("ignoreall",0,1);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortLeft", 0.6);
--			AI.PushGoal("ignoreall",0,0);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_panic_right");
		AI.PushGoal("firecmd",0,0);
--			AI.PushGoal("ignoreall",0,1);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortRight", 0.6);
--			AI.PushGoal("ignoreall",0,0);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_panic_front");
		AI.PushGoal("firecmd",0,0);
--			AI.PushGoal("ignoreall",0,1);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortLeft", 0.6);
--			AI.PushGoal("ignoreall",0,0);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_panic_above");
		AI.PushGoal("firecmd",0,0);
--			AI.PushGoal("ignoreall",0,1);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortRight", 0.6);
--			AI.PushGoal("ignoreall",0,0);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_panic_aboveFire");
		AI.PushGoal("firecmd",0,0);
--			AI.PushGoal("ignoreall",0,1);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_ACTION, "dodgeShortLeft", 0.6);
		AI.PushGoal("timeout",1,0.4);
--		AI.PushGoal("animation", 1, AIANIM_ACTION, "idle", 0.6);
--		AI.PushGoal("firecmd",0, 1);
--		AI.PushGoal("animation", 1, AIANIM_ACTION, "dodgeShortRight", 0.6);
		AI.PushGoal("firecmd",0, FIREMODE_PANIC_SPREAD);
		AI.PushGoal("timeout",1,3,4);
		AI.PushGoal("firecmd",0, 1);		
		AI.PushGoal("timeout",1,0.2,0.4);
		AI.PushGoal("animation", 1, AIANIM_ACTION, "idle", 0.6);
--			AI.PushGoal("ignoreall",0,0);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();


	---------------------------------------------
	AI.BeginGoalPipe("tr2_flinch_left");
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortLeft", 0.6);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_flinch_right");
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortRight", 0.6);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_flinch_front");
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortLeft", 0.6);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();

	---------------------------------------------
	AI.BeginGoalPipe("tr2_flinch_above");
		AI.PushGoal("firecmd",0,0);
		AI.PushGoal("signal",1,1,"surprised",SIGNALID_READIBILITY,115);
		AI.PushGoal("animation", 1, AIANIM_SIGNAL, "dodgeShortRight", 0.6);
		AI.PushGoal("signal",1,1,"PANIC_DONE",0);
	AI.EndGoalPipe();
end
