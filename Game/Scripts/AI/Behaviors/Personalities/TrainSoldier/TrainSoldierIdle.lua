
AIBehaviour.TrainSoldierIdle = {
	Name = "TrainSoldierIdle",
	alertness = 0,
	
	-----------------------------------------------------
	Constructor = function (self, entity)
		entity:InitAIRelaxed();

		
		--TheOtherSide
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		AI.ChangeParameter(entity.id, AIPARAM_COMBATCLASS, AICombatClasses.Squadmate);

		AI.ChangeParameter(entity.id, AIPARAM_AIM_TURNSPEED, 250);
		AI.ChangeParameter(entity.id, AIPARAM_FIRE_TURNSPEED, 250);
		AI.ChangeParameter(entity.id, AIPARAM_LOOKIDLE_TURNSPEED, 250);
		AI.ChangeParameter(entity.id, AIPARAM_LOOKCOMBAT_TURNSPEED, 250);

		---------------------------------------------
		AI.BeginGoalPipe("fn_simple_flinch");
			AI.PushGoal("ignoreall",0,1);
			AI.PushGoal("+firecmd",0,0);
			AI.PushGoal("+bodypos",0,BODYPOS_STAND);
			AI.PushGoal("+animation",0,AIANIM_SIGNAL,"flinch");
			AI.PushGoal("+timeout",1,2,3);
			AI.PushGoal("+ignoreall",0,0);
			AI.PushGoal("signal",0,1,"FLINCH_DONE",0);
		AI.EndGoalPipe();

		---------------------------------------------
	end,

	-----------------------------------------------------
	Destructor = function(self,entity)
	end,

	---------------------------------------------
	OnQueryUseObject = function ( self, entity, sender, extraData )
	end,

	FLINCH_DONE = function (self, entity)		
		entity:SelectPipe(0,"fn_just_shoot");
	end,

	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance, data )
		entity:Readibility("first_contact",1,2, 0.3,0.5);
		AI.Signal(SIGNALFILTER_SENDER, 1, "TO_ATTACK",entity.id);
	end,

	---------------------------------------------
	OnNoTarget = function(self,entity,sender)
	end,

	---------------------------------------------
	OnTargetDead = function( self, entity )
	end,
	
	--------------------------------------------------
	OnNoHidingPlace = function( self, entity, sender,data )
	end,	

	---------------------------------------------
	OnBackOffFailed = function(self,entity,sender)
	end,

	---------------------------------------------
	OnEnemyMemory = function( self, entity )
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
		entity:GettingAlerted();
		entity:Readibility("idle_alert_threat_hear",1,1, 0.1,0.4);
	end,

	--------------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
	end,

	---------------------------------------------
	OnDamage = function ( self, entity, sender)
		-- called when the enemy is damaged
		entity:Readibility("taking_fire",1,1, 0.3,0.5);
		entity:GettingAlerted();
	end,

	---------------------------------------------
	OnFriendlyDamage = function ( self, entity, sender, data)
		if(data.id == g_localActor.id) then 		
			local health = entity.actor:GetHealth();
			local maxHealth = entity.actor:GetMaxHealth();
			if (health < maxHealth * 0.9) then
				entity:Readibility("friendly_fire",1,1, 0.6,1);
			end
		end
	end,

	---------------------------------------------
	OnEnemyDamage = function (self, entity, sender, data)
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
		end
	end,

	---------------------------------------------
	OnPlayerTeamKill = function(self,entity,sender,data)
		AI.ChangeParameter(entity.id, AIPARAM_SPECIES, 5);
	end,

	--------------------------------------------------
	OnCollision = function(self,entity,sender,data)
	end,	
	
	--------------------------------------------------
	OnCloseContact = function ( self, entity, sender,data)
	end,


	--------------------------------------------------
	OnGroupMemberDied = function(self, entity, sender, data)
		entity:Readibility("ai_down",1,1, 0.3,0.5);
	end,

	--------------------------------------------------
	OnGroupMemberDiedNearest = function(self, entity, sender, data)
	end,

	---------------------------------------------
	SHOT_BY_PLAYER = function( self, entity )
		AI.ChangeParameter(entity.id, AIPARAM_SPECIES, 5);
	end,

	---------------------------------------------
	OnShapeEnabled = function (self, entity, sender, data)
	end,

	--------------------------------------------------
	OnCallReinforcements = function (self, entity, sender, data)
	end,

	--------------------------------------------------
	OnGroupChanged = function (self, entity)
	end,

	--------------------------------------------------
	OnExposedToFlashBang = function (self, entity, sender, data)
	end,

	--------------------------------------------------
	OnExposedToSmoke = function (self, entity)
		entity:Readibility("cough",1,115, 0.1,4.5);
	end,

	---------------------------------------------
	OnExposedToExplosion = function(self, entity, data)
		entity:SelectPipe(0,"fn_simple_flinch");
	end,

	---------------------------------------------
	OnGroupMemberMutilated = function(self, entity)
	end,

	---------------------------------------------
	OnTargetCloaked = function(self, entity)
	end,

	--------------------------------------------------	
	OnOutOfAmmo = function (self,entity, sender)
		-- player would not have Reload implemented
		entity:Readibility("reloading",1,2, 0.3,0.5);
		if (entity.Reload == nil) then
			System.Log("  - no reload available");
			do return end
		end
		entity:Reload();
	end,

	---------------------------------------------
	OnGrenadeDanger = function( self, entity, sender, signalData )			
		entity:SelectPipe(0,"fn_simple_flinch");
		entity:Readibility("incoming_grenade",1,5);
	end,

	---------------------------------------------
	OnCloseCollision = function(self, entity, data)
		entity:SelectPipe(0,"fn_simple_flinch");
	end,

	---------------------------------------------
	OnFallAndPlayWakeUp = function( self, entity )
		AI.Signal(SIGNALFILTER_SENDER, 1, "TO_ATTACK",entity.id);
	end,

	---------------------------------------------
	OnPlayerLooking = function(self,entity,sender,data)
	
		if(DialogSystem.IsEntityInDialog(entity.id)) then return end
		-- data.fValue = player distance
--		AI.LogEvent("Player looking at "..entity:GetName());
		if(data.fValue<6) then 
			-- react, readability
			entity:Readibility("staring",1,0,1,2);
			entity:SelectPipe(0,"look_at_player");			
		end
	end,

	---------------------------------------------
	OnPlayerLookingAway = function(self,entity,sender,data)
	
		if(DialogSystem.IsEntityInDialog(entity.id)) then return end
		entity:SelectPipe(0,"stand_only");
		entity:InsertSubpipe(0,"clear_all");
		entity:InsertSubpipe(0,"reset_lookat");
		entity:InsertSubpipe(0,"random_timeout");
	end,


	---------------------------------------------
	OnPlayerSticking = function(self,entity,sender,data)
		if(DialogSystem.IsEntityInDialog(entity.id)) then return end
		entity:Readibility("staring",1,0,1,2);
		entity:SelectPipe(0,"look_at_player");			
	end,

	----------------------------------
	OnPlayerGoingAway = function(self,entity,sender,data)
		AIBehaviour.TrainSoldierIdle:OnPlayerLookingAway(entity,sender,data);
	end,
}

