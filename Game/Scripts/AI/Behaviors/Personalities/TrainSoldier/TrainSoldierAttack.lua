
AIBehaviour.TrainSoldierAttack = {
	Name = "TrainSoldierAttack",
	alertness = 2,
	
	-----------------------------------------------------
	Constructor = function (self, entity)
		entity:MakeAlerted();

		
		--TheOtherSide
		entity.AI.previousBehaviour = entity.AI.currentBehaviour
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		---------------------------------------------
		AI.BeginGoalPipe("fn_just_shoot");
			AI.PushGoal("locate", 0, "probtarget");
			AI.PushGoal("+adjustaim",0,0,1);
			AI.PushGoal("firecmd",0,FIREMODE_BURST);
			AI.PushGoal("timeout",1,2.0,4.0);
			AI.PushGoal("firecmd",0,0);
			AI.PushGoal("timeout",1,0.4,0.6);
		AI.EndGoalPipe();

		---------------------------------------------
		AI.BeginGoalPipe("fn_simple_bulletreaction");
			AI.PushGoal("firecmd",0,FIREMODE_BURST);
			AI.PushGoal("run", 0, 2,1,6);
			AI.PushGoal("bodypos",1,BODYPOS_STAND, 1);
			AI.PushGoal("locate",0,"probtarget");
--			AI.PushGoal("+seekcover", 1, COVER_HIDE, 2.5, 2, 1);
		AI.EndGoalPipe();

		---------------------------------------------
		AI.BeginGoalPipe("fn_melee");
			AI.PushGoal("bodypos",0,BODYPOS_STAND);			
			AI.PushGoal("firecmd",0,FIREMODE_MELEE);
			AI.PushGoal("stick",0,1,0,STICK_BREAK+STICK_SHORTCUTNAV);
			AI.PushGoal("timeout",1,2.5);
			AI.PushGoal("firecmd",0,0);
			AI.PushGoal("clear",0,0);
			AI.PushGoal("signal",0,1,"MELEE_DONE",0);
		AI.EndGoalPipe();
	
		---------------------------------------------

		entity:SelectPipe(0,"fn_just_shoot");

		entity.AI.bulletReactionTime = _time - 10.0;
		entity.AI.meleeBlockTime = _time;

	end,

	-----------------------------------------------------
	Destructor = function(self,entity)
	end,

	---------------------------------------------
	OnBulletRain = function(self, entity, sender, data)
		-- only react to hostile bullets.
		if(not AI.Hostile(entity.id, sender.id)) then
			if(sender==g_localActor) then 
				entity:Readibility("friendly_fire",1,1, 0.6,1);
				entity:InsertSubpipe(AIGOALPIPE_NOTDUPLICATE,"look_at_player_5sec");			
				entity:InsertSubpipe(AIGOALPIPE_NOTDUPLICATE,"do_nothing");		-- make the timeout goal in previous subpipe restart if it was there already
			end
		else
			entity:Readibility("taking_fire",1,2, 0.3,0.5);
			local dt = _time - entity.AI.bulletReactionTime;
			if (dt > 8.0) then
				entity:SelectPipe(0,"fn_simple_bulletreaction");
				entity.AI.bulletReactionTime = _time;
			end
		end
	end,

	---------------------------------------------
	OnEnemyDamage = function (self, entity, sender, data)
		entity:Readibility("taking_fire",1,2, 0.3,0.5);
		local dt = _time - entity.AI.bulletReactionTime;
		if (dt > 8.0) then
			entity:SelectPipe(0,"fn_simple_bulletreaction");
			entity.AI.bulletReactionTime = _time;
		end
	end,

	--------------------------------------------------
	OnCloseContact = function ( self, entity, sender,data)
		-- Do melee at close range.
		local dt = _time - entity.AI.meleeBlockTime;
		if(AI.CanMelee(entity.id) and dt > 7.0) then
			entity:SelectPipe(0,"fn_melee");
			entity.AI.meleeBlockTime = _time;
		end
	end,

	---------------------------------------------
	MELEE_DONE = function (self, entity)		
		entity:SelectPipe(0,"fn_just_shoot");
	end,

	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance, data )
		entity:Readibility("during_combat",1,3, 0.1,0.4);
	end,
	
	---------------------------------------------
	OnNoTarget = function(self,entity,sender)
	end,

	---------------------------------------------
	OnNoTargetAwareness = function(self,entity,sender)
		entity:Readibility("combat_seek",1,3, 0.1,0.4);
	end,

	---------------------------------------------
	OnNoTargetVisible = function(self,entity)
		entity:Readibility("taunt",1,3, 0.1,0.4);
	end,

	---------------------------------------------
	OnTargetDead = function( self, entity )
		entity:Readibility("target_down",1,3, 0.1,0.4);
	end,

	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		entity:Readibility("interest_see",1,1, 0.1,0.4);
	end,
	
	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		entity:Readibility("interest_hear",1,1, 0.1,0.4);
	end,

	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity, fDistance )
		entity:Readibility("interest_hear",1,1, 0.1,0.4);
	end,

	---------------------------------------------
	OnPlayerLooking = function(self,entity,sender,data)
	end,

	---------------------------------------------
	OnPlayerLookingAway = function(self,entity,sender,data)
	end,

	---------------------------------------------
	OnPlayerSticking = function(self,entity,sender,data)
	end,

	---------------------------------------------
	OnPlayerGoingAway = function(self,entity,sender,data)
	end,

	---------------------------------------------
	---------------------------------------------	
}
