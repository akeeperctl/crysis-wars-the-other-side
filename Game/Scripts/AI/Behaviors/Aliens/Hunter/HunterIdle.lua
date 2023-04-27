--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Implementation of Hunter alien idle behavior
--  
--------------------------------------------------------------------------
--  History:
--  - 09/05/2005   : Created by Mikko Mononen
--	- sept 2005: Luciano Morpurgo - adding throwing objects
--------------------------------------------------------------------------
AIBehaviour.HunterIdle = {
	Name = "HunterIdle",

	---------------------------------------------
	Constructor = function( self, entity )
		
		--TheOtherSide
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		AI.ChangeParameter( entity.id, AIPARAM_COMBATCLASS, AICombatClasses.Hunter );
		entity.AI.SCUsage = 0;
	end,

	Destructor = function( self, entity )
	end,
	---------------------------------------------
	
	OnPlayerSeen = function(self,entity,sender)
	end,
	
	---------------------------------------------
	OnEnemyDamage = function(self,entity,sender,data)
	end,

	HunterUseTentacle = function( self, entity, sender, data )
		entity:SetGrabbingScheme(data.ObjectName);
	end,

	-- make the guy shoot at the provided point	for fValue seconds
	SHOOT_NOW = function( self, entity, sender, data )
		if ( entity:DoShootWeapon() ~= 1 ) then
			entity:CancelSubpipe(  ); -- report as failed
		end;
	end,

	-- make the guy shoot at the provided point	for fValue seconds
	ACT_SHOOTAT = function( self, entity, sender, data )
		entity:HolsterItem( false );

		-- use dynamically created goal pipe to set shooting time
		AI.CreateGoalPipe("action_shoot_at");
		AI.PushGoal("action_shoot_at", "firecmd", 0, FIREMODE_OFF);
		AI.PushGoal("action_shoot_at", "locate", 0, "refpoint");
		AI.PushGoal("action_shoot_at", "lookat", 0, 0, 0, true, 1);
		AI.PushGoal("action_shoot_at", "firecmd", 0, FIREMODE_FORCED, AILASTOPRES_USE);
		AI.PushGoal("action_shoot_at", "timeout", 1, 8.5); -- data.fValue);
		AI.PushGoal("action_shoot_at", "firecmd", 0, FIREMODE_OFF);
		AI.PushGoal("action_shoot_at", "lookat", 0, -500, 0);
		AI.PushGoal("action_shoot_at", "timeout", 1, 5);
		
		AI.SetRefPointPosition( entity.id, data.point );
		entity:InsertSubpipe( AIGOALPIPE_SAMEPRIORITY, "action_shoot_at", nil, data.iValue );
	end,

	---------------------------------------------
	DoScream = function( self, entity, sender, data )
		if (entity:DoScream() ~= 1) then
			entity:InsertSubpipe( AIGOALPIPE_SAMEPRIORITY, "action_dummy", nil, data.iValue );
			entity:CancelSubpipe( data.iValue );
			return;
		end
		AI.CreateGoalPipe("action_scream");
		AI.PushGoal("action_scream", "timeout", 0, 8);
		entity:InsertSubpipe( AIGOALPIPE_SAMEPRIORITY, "action_scream", nil, data.iValue );
	end,
		
	--
	---------------------------------------------
	ACT_ANIM = function( self, entity, sender, data )
		if (data.ObjectName == "scream") then
			self:DoScream( entity, sender, data );
			return;
		end
		
		local animState = entity.actor:GetCurrentAnimationState();
		if (animState=="WeaponFire" or animState=="Screaming" or animState=="Throw" or animState=="ThrowHuman") then
			entity:InsertSubpipe( AIGOALPIPE_SAMEPRIORITY, "action_dummy", nil, data.iValue );
			entity:CancelSubpipe( data.iValue );
			return;
		end
		
		AIBehaviour.DEFAULT:ACT_ANIM( entity, sender, data );
	end,
	
	---------------------------------------------
	CheckSingularityCannonUsage = function( self, entity, sender, data )
		
		local target = AI.GetAttentionTargetEntity(entity.id);
		if( target) then
			local SCUsage = entity.AI.SCUsage or 0;
			if(SCUsage==0) then 
				entity.AI.SCUsage = 10;
				return 
			end		
			if(target.actorStats and target.actorStats.isFrozen) then 
				entity:SelectSecondaryWeapon();
				return
			end
			local dist = AI.GetAttentionTargetDistance(entity.id);
			if(dist and dist>70) then 
				entity:SelectSecondaryWeapon();
			end
			
			if (dist) then
				local r = random(25,70);
				if (r<dist) then
					entity:SelectSecondaryWeapon();
				end
			end
		end
	end,
	---------------------------------------------
	SelectPrimaryWeapon = function( self, entity, sender, data )
		entity:SelectPrimaryWeapon();
	end,

	-------------------------------------------------------
	-- debug
	CHECK_TROOPER_GROUP = function(self,entity,sender)
		AI.Warning(entity:GetName().. " IS IN SAME GROUP WITH TROOPER "..sender:GetName()..", groupid = "..AI.GetGroupOf(entity.id));
	end,
	
}
