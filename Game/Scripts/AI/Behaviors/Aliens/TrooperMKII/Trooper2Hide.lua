--------------------------------------------------
--    Created By: Luciano
--   created: Integrated to Trooper AI by Denisz PolgпїЅr 31.01.2008.
--   Description: 	Cover goes hiding under fire
--------------------------
--

AIBehaviour.Trooper2Hide = {
	Name = "Trooper2Hide",
--	Base = "Trooper2Attack",
	Base = "TROOPERDEFAULT",
	alertness = 1,

	-----------------------------------------------------
	Constructor = function(self,entity)

		if (entity.AI.ignoreSignals ~= true) then
			entity:GettingAlerted();

	--		entity.AI.changeCoverLastTime = _time;
	--		entity.AI.changeCoverInterval = random(7,11);
	--		entity.AI.fleeLastTime = _time;
	--		entity.AI.lastLiveEnemyTime = _time;
	--		entity.AI.lastBulletReactionTime = _time - 10;
	--		entity.AI.lastFriendInWayTime = _time - 10;

			entity.AI.lastBulletReactionTime = _time - 10;
			
			entity:Readibility("taking_fire",1,1, 0.1,0.4);
			self:HandleThreat(entity);
		end
	end,
	
	-----------------------------------------------------
	Destructor = function(self,entity)
	end,

	-----------------------------------------------------
	HandleThreat = function(self, entity, sender)
		if (entity.AI.ignoreSignals ~= true) then

			local	dt = _time - entity.AI.lastBulletReactionTime;

			local reactionTime = 3.0;
			if (AI.IsMoving(entity.id,1) == 1) then
				reactionTime = 6.0;
			end
			if(dt > reactionTime) then
				if(not sender or AI.Hostile(entity.id, sender.id)) then
					entity.AI.lastBulletReactionTime = _time;
					AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_HIDING);
	--				entity:SelectPipe(0,"do_nothing");

	--				entity:SelectPipe(0,"do_nothing");

	--				entity:SelectPipe(0,"cv_hide_unknown");

					entity:SelectPipe(0,"cv_bullet_reaction");
				end
			end
		end
		
	end,

	-----------------------------------------------------
	HandleHit = function(self, entity, sender)
--		local	dt = _time - entity.AI.lastBulletReactionTime;
--		if(dt > 0.5) then
--			if(not sender or AI.Hostile(entity.id, sender.id)) then
--				entity.AI.lastBulletReactionTime = _time;
--				AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_HIDING);
--				entity:SelectPipe(0,"tr_dodge_right_short_anim2");
--			end
--		end

--		if(Trooper_LowHealth2(entity)) then 
--			entity:SelectPipe(0,"tr_dodge_right_short_anim2");
--		end

		Trooper_HitReaction(entity);		
	end,

	-----------------------------------------------------
	COVER_NORMALATTACK = function(self, entity)
		-- Choose proper action after being interrupted.
		AI_Utils:CommonContinueAfterReaction(entity);
	end,

	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy stops having an attention target
	end,
	
	---------------------------------------------
	OnNoTarget = function( self, entity )
		-- called when the enemy stops having an attention target
--		self:HandleThreat(entity);
	end,

	---------------------------------------------		
	OnPlayerSeen = function( self, entity, fDistance, data )
		entity:MakeAlerted();
		entity:TriggerEvent(AIEVENT_DROPBEACON);
		
		AI_Utils:CommonEnemySeen(entity, data);
	end,

	---------------------------------------------		
	OnInterestingSoundHeard = function( self, entity, fDistance )
--		entity:TriggerEvent(AIEVENT_CLEAR);
	end,

	---------------------------------------------
	OnThreateningSeen = function( self, entity )
		entity:TriggerEvent(AIEVENT_DROPBEACON);
	end,

	---------------------------------------------		
	OnThreateningSoundHeard = function( self, entity, fDistance )
		entity:TriggerEvent(AIEVENT_DROPBEACON);
	end,

	---------------------------------------------		
	OnSommethingSeen = function( self, entity, fDistance )
		entity:TriggerEvent(AIEVENT_DROPBEACON);
	end,

	---------------------------------------------
	OnEnemyDamage = function ( self, entity, sender,data)
	
		if (entity.AI.ignoreSignals ~= true) then

			Trooper_HitReaction(entity);
		
			entity.AI.coverCompromized = true;

			-- set the beacon to the enemy pos
			local shooter = System.GetEntity(data.id);
			if(shooter) then
				AI.SetBeaconPosition(entity.id, shooter:GetPos());
				AI.Signal(SIGNALFILTER_GROUPONLY_EXCEPT,1,"INCOMING_FIRE",entity.id);
			end

			-- called when the enemy is damaged
			--self:HandleThreat(entity, shooter);
			
			entity:Readibility("taking_fire",1,1, 0.1,0.4);
		end
	end,

	---------------------------------------------
	OnBulletRain = function(self, entity, sender, data)
		if (entity.AI.ignoreSignals ~= true) then
			entity:Readibility("bulletrain",1,1, 0.1,0.4);
			--self:HandleThreat(entity, sender);

			local shooter = System.GetEntity(sender.id);
			if(shooter) then
				AI.SetBeaconPosition(entity.id, shooter:GetPos());
				AI.Signal(SIGNALFILTER_GROUPONLY_EXCEPT,1,"INCOMING_FIRE",entity.id);
			end
		end
	end,

	---------------------------------------------
	OnReload = function( self, entity )
	end,

	--------------------------------------------------------
	OnHideSpotReached = function(self,entity,sender)
	end,

	---------------------------------------------
	INCOMING_FIRE = function (self, entity, sender)
	end,

	--------------------------------------------------
	OnBulletHit = function( self, entity, sender,data )
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
