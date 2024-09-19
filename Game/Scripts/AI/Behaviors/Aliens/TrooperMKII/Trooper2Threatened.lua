--------------------------------------------------
-- SneakerThreatened
--
-- Description:
--	Investigate a threatening event. The event can be for excample a sound or a sight of a sniper.
--	The threatened behavior will be only executed for limited amount of time.
--	The idea behind this is that as the player makes mistakes, the AI gets closer and closer.
--	The total effect of the threatened search is the approach in this behavior plus the 
--	time spend investigating the close by covers.
--------------------------
--   created: Mikko Mononen 21-6-2006
--   created: Integrated to Trooper AI by Denisz PolgпїЅr 31.01.2008.

AIBehaviour.Trooper2Threatened = {
	Name = "Trooper2Threatened",
	Base = "TROOPERDEFAULT",
	alertness = 1,

	---------------------------------------------
	Constructor = function (self, entity)
		if (entity.AI.ignoreSignals == false) then
			AI.NotifyGroupTacticState(entity.id, 0, GN_NOTIFY_UNAVAIL);
			-- store original position.
			if(not entity.AI.idlePos) then
				entity.AI.idlePos = {x=0, y=0, z=0};
				CopyVector(entity.AI.idlePos, entity:GetPos());
			end

			entity:MakeAlerted();
			AI_Utils:CheckThreatened(entity, 30.0);

			-- Keep track on everyone in the group who are in threatened.
	--		local groupId = AI.GetGroupOf(entity.id);
	--		if (not AIBlackBoard[groupId]) then
	--			AIBlackBoard[groupId] = {};
	--		end
	--		if (not AIBlackBoard[groupId].threatened) then
	--			AIBlackBoard[groupId].threatened = 0;
	--		end
	--		AIBlackBoard[groupId].threatened = AIBlackBoard[groupId].threatened + 1;

			local range = entity.Properties.preferredCombatDistance/2;
			local radius = 4.0;
			if(AI.GetNavigationType(entity.id) == NAV_WAYPOINT_HUMAN) then
				range = range / 2;
				radius = 2.5;
			end
	--  	AI.SetPFBlockerRadius(entity.id, PFB_ATT_TARGET, range/2);
	--  	AI.SetPFBlockerRadius(entity.id, PFB_BEACON, range/2);
		AI.SetPFBlockerRadius(entity.id, PFB_DEAD_BODIES, radius);
		AI.SetPFBlockerRadius(entity.id, PFB_EXPLOSIVES, radius);

			entity.AI.firstContact = true;

			-- Approach the target only limited amount of time.
	--		entity.AI.threatTimer = Script.SetTimer(entity.AI.checkThreatenTime*1000,AIBehaviour.Trooper2Threatened.OnStopApproachTimer,entity);
	--		entity.AI.first = true;
		end
	end,

	---------------------------------------------
	Destructor = function (self, entity)
--		local groupId = AI.GetGroupOf(entity.id);
--		AIBlackBoard[groupId].threatened = AIBlackBoard[groupId].threatened - 1;
--		if (entity.AI.threatTimer) then
--			Script.KillTimer(entity.AI.threatTimer);
--			entity.AI.threatTimer = nil;
--		end
	end,

	---------------------------------------------
	SEEK_KILLER = function (self, entity)
		if (entity.AI.ignoreSignals == false) then
			AI_Utils:CheckThreatened(entity, 15.0);
		end
	end,

--	-----------------------------------------------------
--	OnStopApproachTimer = function(entity,timerid)
--		-- The first unit to reach the end if its' timer will tell the whole group to stop
--		-- searching and switch to search behavior.
--		entity.AI.threatTimer = nil;
--		AI.Signal(SIGNALFILTER_GROUPONLY, 1, "STOP_APPROACH",entity.id);
--
--		entity:Readibility("alert_interest_hear",1,1,0.3,0.6);
--	end,
--
--	---------------------------------------------
--	STOP_APPROACH = function(self, entity)
--		-- This signal is send the first unit in the group to reach the time target.
--		-- The whole group will start searching.
--		if (entity.AI.threatTimer) then
--			Script.KillTimer(entity.AI.threatTimer);
--			entity.AI.threatTimer = nil;
--		end
--
--		AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_SEARCH",entity.id);
--	end,

	---------------------------------------------
	OnNoTarget = function (self, entity)
--		AI.SetRefPointPosition(entity.id,entity.AI.idlePos);
--		entity:SelectPipe(0,"cv_get_back_to_idlepos");
	end,

	---------------------------------------------
	INVESTIGATE_DONE = function( self, entity )
		if (entity.AI.ignoreSignals == false) then
			local target = AI.GetTargetType(entity.id);
			if(target == AITARGET_ENEMY) then
				entity:Readibility("taunt",1,3,0.3,0.6);
				AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_ATTACK",entity.id);
			elseif(target == AITARGET_NONE) then
				entity:Readibility("alert_idle_relax",1,1, 0.3,0.6);
				AI.SetRefPointPosition(entity.id,entity.AI.idlePos);
				entity:SelectPipe(0,"cv_get_back_to_idlepos");
			else
				AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_SEARCH",entity.id);
			end
		end
	end,

	---------------------------------------------
	INVESTIGATE_CONTINUE = function( self, entity )
		entity:SelectPipe(0,"tr2_investigate_threat_closer");
	end,

	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance, data )
		if (entity.AI.ignoreSignals == false) then
			-- called when the enemy sees a living player
			entity:TriggerEvent(AIEVENT_DROPBEACON);

			AI_Utils:CommonEnemySeen(entity, data);
		end
	end,

	---------------------------------------------
	OnEnemySeen = function( self, entity )
		-- called when the enemy sees a foe which is not a living player
	end,

	---------------------------------------------
	OnFriendSeen = function( self, entity )
		-- called when the enemy sees a friendly target
	end,

	---------------------------------------------
	OnDeadBodySeen = function( self, entity )
		-- called when the enemy a dead body
	end,

	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
	end,

	---------------------------------------------
	CheckToChangeTarget = function( self, entity )
		-- If the attention target has changed a lot, choose new approach.
--		local	attPos = g_Vectors.temp_v1;
--		AI.GetAttentionTargetPosition(entity.id, attPos);
--		local dist = DistanceVectors(attPos, entity.AI.target);
--		if(dist > 5.0) then
			AI_Utils:CheckThreatened(entity, 15.0);
--		elseif(dist > 3.0) then
--			entity:InsertSubpipe(AIGOALPIPE_NOTDUPLICATE,"cv_look_at_target");
--		end
	end,

	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_AUDIO) > 0.0 ) then
	--		entity:TriggerEvent(AIEVENT_CLEAR);
	--		entity:Readibility("alert_interest_hear",1,1,0.3,0.6);
			self:CheckToChangeTarget(entity);
		end
	end,
	
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_AUDIO) > 0.0 ) then
	--		entity:TriggerEvent(AIEVENT_CLEAR);
			entity:TriggerEvent(AIEVENT_DROPBEACON);
			entity:Readibility("alert_interest_hear",1,1,0.3,0.6);
			self:CheckToChangeTarget(entity);
		end
	end,

	---------------------------------------------
	OnThreateningSeen = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL) > 0.0 ) then
			entity:TriggerEvent(AIEVENT_DROPBEACON);
			entity:Readibility("alert_interest_see",1,1,0.3,0.6);
			self:CheckToChangeTarget(entity);
		end
	end,

	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL) > 0.0 ) then
			entity:Readibility("alert_interest_see",1,1,0.3,0.6);
			self:CheckToChangeTarget(entity);
		end
	end,

	---------------------------------------------
	OnReload = function( self, entity )
	end,

	---------------------------------------------
	OnDamage = function ( self, entity, sender)
	end,

	---------------------------------------------
	ENEMYSEEN_DURING_COMBAT = function (self, entity, sender)
		if(AI.GetTargetType(entity.id) ~= AITARGET_ENEMY) then
			AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_SEEK",entity.id);
		end
	end,
	
	---------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function (self, entity, sender)
		if(AI.GetTargetType(entity.id) ~= AITARGET_ENEMY) then
			AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_SEEK",entity.id);
		end
	end,

	--------------------------------------------------
	INCOMING_FIRE = function (self, entity, sender)
		-- do nothing on this signal
	end,
	--------------------------------------------------
	OnEnemyDamage = function( self, entity, sender,data )
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL) > 0.0 ) then
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
