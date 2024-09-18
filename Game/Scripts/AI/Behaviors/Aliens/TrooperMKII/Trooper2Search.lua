--------------------------------------------------
-- SneakerSearch
--------------------------
--   created: Mikko Mononen 21-6-2006
--   created: Integrated to Trooper AI by Denisz PolgпїЅr 31.01.2008.

AIBehaviour.Trooper2Search = {
	Name = "Trooper2Search",
	Base = "TROOPERDEFAULT",
	alertness = 1,

	---------------------------------------------
	Constructor = function(self, entity)
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			entity:Readibility("searching_for_enemy",1,1,0.3,1.0);

			AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_SEEKING);

			-- check if the AI is at the edge of the territory and cannot move.
			if(AI_Utils:IsTargetOutsideTerritory(entity) == 1) then
				-- at the edge, wait, aim and shoot.
				entity:SelectPipe(0,"sn_wait_and_aim");
			else
				-- enough space, search.
				entity:SelectPipe(0,"tr2_seek_target_random");
			end
			
	--		AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_SEARCHING);
			
			entity.AI.lastLookAtTime = _time;
			
			--entity:EnableSearchBeam(true);
		end
	end,

	---------------------------------------------
	Destructor = function(self, entity)
		entity.anchor = nil;
		--entity:EnableSearchBeam(false);
	end,

	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance, data )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			-- called when the enemy sees a living player
			entity:MakeAlerted();
			entity:TriggerEvent(AIEVENT_DROPBEACON);

			AI_Utils:CommonEnemySeen(entity, data);
		end
	end,
	
	---------------------------------------------
	OnNoTarget = function( self, entity )
--		AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_UNAVAIL);
--		AI.SetRefPointPosition(entity.id,entity.AI.idlePos);
--		entity:SelectPipe(0,"cv_get_back_to_idlepos");
--		entity:Readibility("alert_idle_relax",1,1,0.3,0.6);
--		AI.Signal(SIGNALFILTER_SENDER,1,"RETURN_TO_FIRST",entity.id);
	end,

	---------------------------------------------
	COVER_NORMALATTACK = function (self, entity)
		-- check if the AI is at the edge of the territory and cannot move.
		if(AI_Utils:IsTargetOutsideTerritory(entity) == 1) then
			-- at the edge, wait, aim and shoot.
			entity:SelectPipe(0,"sn_wait_and_aim");
		else
			AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_SEEKING);
			-- enough space, search.
			entity:SelectPipe(0,"tr2_seek_target_random");
		end
	end,

	---------------------------------------------
	OnReload = function( self, entity )
--		entity:Readibility("reloading",1);
	end,

	--------------------------------------------------
	HIDE_FAILED = function (self, entity, sender)
		-- no hide points, goto group combat location to avoid clustering
		AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_SEEKING);
		entity:SelectPipe(0,"tr2_seek_target_nocover");
	end,

	--------------------------------------------------
	LOOK_FOR_TARGET = function (self, entity, sender)
		-- check if the AI is at the edge of the territory and cannot move.
		if(AI_Utils:IsTargetOutsideTerritory(entity) == 1) then
			-- at the edge, wait, aim and shoot.
			entity:SelectPipe(0,"sn_wait_and_aim");
		else
			AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_SEEKING);
			-- enough space, search.
			entity:SelectPipe(0,"tr2_seek_target_random");
		end
	end,	

	--------------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function (self, entity, sender)
		-- there is still some room for moving.
		AI.Signal(SIGNALFILTER_SENDER,1,"TO_SEEK",entity.id);
	end,

	--------------------------------------------------
	ENEMYSEEN_DURING_COMBAT = function (self, entity, sender)
		-- there is still some room for moving.
		AI.Signal(SIGNALFILTER_SENDER,1,"TO_SEEK",entity.id);
	end,

	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_AUDIO ) > 0.0 ) then
			entity:TriggerEvent(AIEVENT_DROPBEACON);
			AI_Utils:CheckThreatened(entity, 15.0);
		end
	end,
	
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity, fDistance )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_AUDIO) > 0.0 ) then
			entity:TriggerEvent(AIEVENT_DROPBEACON);
			AI_Utils:CheckThreatened(entity, 15.0);
		end
	end,

	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			entity:TriggerEvent(AIEVENT_DROPBEACON);
			AI_Utils:CheckThreatened(entity, 15.0);
		end
	end,

	---------------------------------------------
	OnThreateningSeen = function( self, entity )
		entity:TriggerEvent(AIEVENT_DROPBEACON);
		AI_Utils:CheckThreatened(entity, 15.0);
	end,

	---------------------------------------------
	INVESTIGATE_CONTINUE = function( self, entity )
		entity:SelectPipe(0,"tr2_investigate_threat_closer");
	end,
	
	---------------------------------------------
	INVESTIGATE_DONE = function( self, entity )
		self:COVER_NORMALATTACK(entity);
	end,

	--------------------------------------------------
	OnEnemyDamage = function( self, entity, sender,data )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			Trooper_HitReaction(entity);
		end
	end,

	--------------------------------------------------
	OnBulletHit = function( self, entity, sender,data )
	end,

	---------------------------------------------
	OnBulletRain = function(self, entity, sender, data)
	end,

	---------------------------------------------
	PANIC_DONE = function(self, entity)
		AI.Signal(SIGNALFILTER_GROUPONLY_EXCEPT, 1, "ENEMYSEEN_FIRST_CONTACT",entity.id);
		-- Choose proper action after being interrupted.
		AI_Utils:CommonContinueAfterReaction(entity);
	end,

	--------------------------------------------------
	INVESTIGATE_TARGET = function (self, entity, sender)
	end,
}
