--------------------------------------------------
-- SneakerInterested
--------------------------
--   created: Mikko Mononen 21-6-2006
--   created: Integrated to Trooper AI by Denisz PolgпїЅr 31.01.2008.


AIBehaviour.Trooper2Interested = {
	Name = "Trooper2Interested",
	Base = "TROOPERDEFAULT",
	alertness = 0,
	
	Constructor = function (self, entity)
		if (entity.AI.ignoreSignals ~= true) then
			AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_UNAVAIL);

			-- store original position.
			if(not entity.AI.idlePos) then
				entity.AI.idlePos = {x=0, y=0, z=0};
				CopyVector(entity.AI.idlePos, entity:GetPos());
			end

		AI.SetPFBlockerRadius(entity.id, PFB_DEAD_BODIES, 7);
		AI.SetPFBlockerRadius(entity.id, PFB_EXPLOSIVES, 7);

			entity.AI.firstContact = true;
			
			-- Search at least 10s.
			entity.AI.allowLeave = false;
			entity.AI.searchTimer = Script.SetTimerForFunction(10*1000.0,"AIBehaviour.Trooper2Interested.SEARCH_TIMER",entity);

			--entity:EnableDiscoveryBeam(true);
		end
	end,
	
	---------------------------------------------
	Destructor = function (self, entity)
		if (entity.AI.searchTimer) then
			Script.KillTimer(entity.AI.searchTimer);
		end
		
		--entity:EnableDiscoveryBeam(false);
	end,

	---------------------------------------------
	SEARCH_TIMER = function(entity,timerid)
		entity.AI.searchTimer = nil;
		entity.AI.allowLeave = true;
		if (AI.GetTargetType(entity.id) == AITARGET_NONE) then
			AI.SetRefPointPosition(entity.id,entity.AI.idlePos);
			entity:SelectPipe(0,"cv_get_back_to_idlepos");
		end
	end,
	
	---------------------------------------------
	INVESTIGATE_DONE = function( self, entity )
		local target = AI.GetTargetType(entity.id);
		if(target == AITARGET_ENEMY) then
			AI.Signal(SIGNALFILTER_SENDER, 1, "GO_TO_ATTACK",entity.id);
		else
			AI.SetRefPointPosition(entity.id,entity.AI.idlePos);
			entity:SelectPipe(0,"cv_get_back_to_idlepos");
		end
		entity:HolsterItem(true);
	end,

	---------------------------------------------
--	OnNoTargetAwareness = function (self, entity)
--		self:INVESTIGATE_DONE(entity);
--	end,

	---------------------------------------------
	INVESTIGATE_READABILITY = function( self, entity )
		entity:Readibility("alert_idle_relax",1);
	end,
	
	---------------------------------------------
	OnNoTarget = function( self, entity )
		if (entity.AI.allowLeave == true) then
			self:INVESTIGATE_DONE(entity);
		end
	end,

	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance, data )
		if (entity.AI.ignoreSignals ~= true) then
			entity:MakeAlerted();
			entity:TriggerEvent(AIEVENT_DROPBEACON);

			AI_Utils:CommonEnemySeen(entity, data);
		end
	end,

	---------------------------------------------
	OnEnemyMemory = function( self, entity )
	end,

	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_AUDIO ) > 0.0 ) then
			entity:Readibility("idle_interest_hear",1,1,0.6,1);
			AI_Utils:CheckInterested(entity)
		end;
	end,

	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		entity:Readibility("idle_interest_see",1,1,0.6,1);
		AI_Utils:CheckInterested(entity);
	end,

	---------------------------------------------
	OnReload = function( self, entity )
--		entity:Readibility("reloading",1);
	end,

	---------------------------------------------
	OnNoHidingPlace = function( self, entity, sender )
	end,	

	--------------------------------------------------
	OnNoFormationPoint = function ( self, entity, sender)
	end,

	--------------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
	end,

	---------------------------------------------
	OnDamage = function ( self, entity, sender)
	end,
	
	--------------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function (self, entity, sender)
		if(AI.GetTargetType(entity.id) ~= AITARGET_ENEMY) then
			AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_SEEK",entity.id);
		end
	end,
	
	--------------------------------------------------
--	INCOMING_FIRE = function (self, entity, sender)
		-- do nothing on this signal
--	end,

	--------------------------------------------------
	OnEnemyDamage = function( self, entity, sender,data )
		if (entity.AI.ignoreSignals ~= true) then
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


	---------------------------------------------
	COVER_NORMALATTACK = function (self, entity, sender)
	end,


}
