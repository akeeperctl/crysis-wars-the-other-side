--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

Script.ReloadScript("Scripts/Utils/TOSCommon.lua")
Script.ReloadScript("Scripts/Utils/TOSTable.lua")

local FOLLOW_UPDATE_TIMER = 0
local minUpdateTime = 0.1;

local FOLLOW = {

}

AIBehaviour.TOS_Obey_Follow_and_Protect = {
	Base = "TOS_Obey",
	Name = "TOS_Obey_Follow_and_Protect",
	NOPREVIOUS = 1, -- Если 1, то не изменяет предыдущее поведение, записанное в "PREVIOUS"
	-- Alertness = 0,
	-- Exclusive = 1,
	-- LeaveCoverOnStart = 1,

	-- TheOtherSide
	---------------------------------------------
	Constructor = function (self, entity, data)
		-- called when the behaviour is selected
		-- the extra data is from the signal that caused the behavior transition
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.Constructor data %s", EntityName(entity), table.safedump(data))

		FOLLOW = {
			target = entity.orderRefEnt,
			distance = entity.follow_distance,
			speed = entity.follow_speed
		}		

		TOS_AI.BeginGoalPipe("ai_follow_and_protection")
			TOS_AI.PushGoal(GO.STRAFE, 1, 100, 100)
			TOS_AI.PushGoal(GO.RUN, 1, FOLLOW.speed)
			TOS_AI.PushGoal(GO.STICK, 1, FOLLOW.distance, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 2, 0)
		TOS_AI.EndGoalPipe()

		local args = {
			self = self,
			entity = entity,
			follow = FOLLOW
		}
		FOLLOW_UPDATE_TIMER = Script.SetTimerForFunction( minUpdateTime * 1000 , "AIBehaviour.TOS_Obey_Follow_and_Protect.Update", args );
	end,
	---------------------------------------------
	Destructor = function (self, entity, data)
		-- called when the behaviour is de-selected
		-- the extra data is from the signal that is causing the behavior transition
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.Destructor data %s", EntityName(entity), EntityName(data))

		-- unreserve vehicle seat
		if (entity.currentExecutedOrder == EOrders.AI_ENTERVEHICLE) then
			CLEAR_DATA_AI_ENTERVEHICLE(entity)
		end

		-- этло 
		-- Script.KillTimer(FOLLOW_UPDATE_TIMER)
	end,
	-- ~TheOtherSide

	-- ---------------------------------------------
	-- FollowEnable = function (entity, target, enable)
	-- 	if (enable) then
	-- 		TOS_AI.InsertSubpipe(AIGOALPIPE_NOTDUPLICATE, entity, "ai_follow_and_protection", target.id)
	-- 	--else
	-- 	--	AI.CancelSubpipe(target.id)
	-- 	end

	-- 	-- entity.follow_enabled = enable
	-- end,

	-- ---------------------------------------------
	-- ProtectEnable = function (entity, enable)
	-- 	if (enable) then
	-- 		TOS_AI.InsertSubpipe(AIGOALPIPE_RUN_ONCE, entity, "ai_protect_enable")
	-- 	else
	-- 		TOS_AI.InsertSubpipe(AIGOALPIPE_RUN_ONCE, entity, "ai_protect_disable")
	-- 	end

	-- 	entity.protect_enabled = enable
	-- end,

	---------------------------------------------
	Update = function (args)

		local vDirToTarget = {}
		SubVectors( vDirToTarget, FOLLOW.target:GetWorldPos(), args.entity:GetWorldPos() );
		
		local distanceToTarget = LengthVector( vDirToTarget );
		if (distanceToTarget > FOLLOW.distance + 1) then
			TOS_AI.InsertSubpipe(AIGOALPIPE_NOTDUPLICATE, args.entity, "ai_follow_and_protection", args.follow.target.id)
		end

		-- if (TOS_AI.GetTargetType(args.entity.id) == AITARGET_ENEMY) then
		-- 	if (args.entity.protect_enabled ~= true) then
		-- 		args.self.ProtectEnable(args.entity, true)
		-- 	end
		-- else 
		-- 	if (args.entity.protect_enabled ~= false) then
		-- 		args.self.ProtectEnable(args.entity, false)
		-- 	end
		-- end

		FOLLOW_UPDATE_TIMER = Script.SetTimerForFunction( minUpdateTime * 1000 , "AIBehaviour.TOS_Obey_Follow_and_Protect.Update", args );
	end,

	-- События из кода CRYENGINE 5
	-- Вызывается только OnActionDone
	OnActionDone = function (self, entity, data, sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnActionDone sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	-- OnActionCreated = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnActionCreated sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionEnd = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnActionEnd sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionStart = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnActionStart sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,
	---------------------------------------------
	OnObjectSeen = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnObjectSeen data: %s", EntityName(entity), table.safedump(data))
	end,

	---------------------------------------------
	OnFriendInWay = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnFriendInWay data: %s", EntityName(entity), table.safedump(data))
	end,

	--------------------------------------------------------------------------
	OnSeenByEnemy = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnSeenByEnemy sender: %s", EntityName(entity), EntityName(sender))
	end,
	--------------------------------------------------------------------------
	OnCloseContact= function( self, entity )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnCloseContact", EntityName(entity))
	end,

	OnTargetDead = function( self, entity )
		-- called when the attention target died
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnTargetDead", EntityName(entity))
	end,

	OnTargetNavTypeChanged= function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnTargetNavTypeChanged sender: %s, data: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------
	OnTargetCloaked = function(self, entity)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnTargetCloaked", EntityName(entity))
	end,

	---------------------------------------------
	ORDER_TIMEOUT = function( self, entity, sender,data )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ORDER_TIMEOUT", EntityName(entity))
	end,
	
	---------------------------------------------
	ORDER_ACQUIRE_TARGET = function(self , entity, sender, data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ORDER_ACQUIRE_TARGET sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------------------------------------------------------------------------------------------------
	ORDER_COVER_SEARCH = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ORDER_COVER_SEARCH sender: %s", EntityName(entity), EntityName(sender))
	end,	

	---------------------------------------------------------------------------------------------------------------------------------------
	ORD_ATTACK = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ORD_ATTACK sender: %s", EntityName(entity), EntityName(sender))
	end,

	ORD_DONE = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ORD_DONE sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnReinforcementRequested = function ( self, entity, sender, extraData )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnReinforcementRequested sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnCallReinforcement = function(self, entity, sender, extraData)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnCallReinforcement sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnPathFound = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnPathFound sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------------------
	OnRequestUpdate = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnRequestUpdate sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnPlayerDied = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnPlayerDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDied = function ( self, entity, sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnLeaderDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderAssigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity назначается лидер группы через AI.SetLeader(entityId)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnLeaderAssigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDeassigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity разназначается лидером группы
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnLeaderDeassigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnUnitDied = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnUnitDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnJoinTeam = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnJoinTeam sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	GroupMemberDied = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.GroupMemberDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	OnBodyFallSound = function(self, entity, sender, data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnBodyFallSound sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnGroupMemberDiedNearest = function ( self, entity, sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnGroupMemberDiedNearest sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnDeath = function ( self, entity, sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnDeath sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ENEMYSEEN_FIRST_CONTACT sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	ENEMYSEEN_DURING_COMBAT = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ENEMYSEEN_DURING_COMBAT sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnBackOffFailed = function(self,entity)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnBackOffFailed sender: %s", EntityName(entity))
	end,

	---------------------------------------------
	OnVehicleDanger = function(self,entity)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnVehicleDanger sender: %s", EntityName(entity))
	end,
	
	---------------------------------------------
	OnExplosionDanger = function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnExplosionDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	--------------------------------------------------
	OnGrenadeDanger = function( self, entity, sender, signalData )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnGrenadeDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(signalData))
	end,

	-- SYSTEM EVENTS			-----
	---------------------------------------------
	OnSelected = function( self, entity )
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnSelected", EntityName(entity))
	end,
	---------------------------------------------
	OnSpawn = function( self, entity )
		-- called when enemy spawned or reset
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnSpawn", EntityName(entity))
	end,
	---------------------------------------------
	OnActivate = function( self, entity )
		-- called when enemy receives an activate event (from a trigger, for example)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnActivate", EntityName(entity))
	end,
	---------------------------------------------
	OnNoTarget = function( self, entity )
		-- called when the enemy stops having an attention target
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnNoTarget", EntityName(entity))
	end,
	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance )
		-- called when the enemy sees a living player
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnPlayerSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnGrenadeSeen = function( self, entity, fDistance )
		-- called when the enemy sees a grenade
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnGrenadeSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnEnemySeen = function( self, entity )
		-- called when the enemy sees a foe which is not a living player
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnEnemySeen", EntityName(entity))
	end,
	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		-- called when the enemy sees something that it cant identify
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnSomethingSeen", EntityName(entity))
	end,
	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnEnemyMemory", EntityName(entity))
	end,
	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		-- called when the enemy hears an interesting sound
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnInterestingSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity )
		-- called when the enemy hears a scary sound
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnThreateningSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnReload = function( self, entity )
		-- called when the enemy goes into automatic reload after its clip is empty
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnReload", EntityName(entity))
	end,
	---------------------------------------------
	OnReloadDone = function( self, entity )
		-- called after reloading is done
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnReloadDone", EntityName(entity))
	end,
	---------------------------------------------
	OnGroupMemberDied = function( self, entity )
		-- called when a member of the group dies
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnGroupMemberDied", EntityName(entity))
	end,
	---------------------------------------------
	OnNoHidingPlace = function( self, entity, sender )
		-- called when no hiding place can be found with the specified parameters

		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnNoHidingPlace, sender: %s", EntityName(entity), EntityName(sender))
	end,	
	--------------------------------------------------
	OnNoFormationPoint = function ( self, entity, sender)
		-- called when the enemy found no formation point

		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnNoFormationPoint, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnReceivingDamage = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnReceivingDamage, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnCoverRequested, sender: %s", EntityName(entity), EntityName(sender))

	end,
	--------------------------------------------------
	OnBulletRain = function ( self, entity, sender)
		-- called when the enemy detects bullet trails around him

		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnBulletRain, sender: %s", EntityName(entity), EntityName(sender))

	end,

	-- ИЗ ДРУГИХ ПОВЕДЕНИЙ
	---------------------------------------------
	OnTargetApproaching	= function (self, entity)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnTargetApproaching", EntityName(entity))
	end,
	---------------------------------------------
	OnTargetFleeing	= function (self, entity)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnTargetFleeing", EntityName(entity))
	end,
	--------------------------------------------------
	OnCoverCompromised = function(self, entity, sender, data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnCoverCompromised", EntityName(entity))
	end,

	---------------------------------------------
	OnFallAndPlayWakeUp = function( self, entity )
		AI_Utils:CommonContinueAfterReaction(entity);
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnFallAndPlayWakeUp", EntityName(entity))
	end,

	---------------------------------------------
	OnPlayerTeamKill = function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.OnPlayerTeamKill", EntityName(entity))
	end,
	--------------------------------------------------

	-- GROUP SIGNALS
	---------------------------------------------	
	KEEP_FORMATION = function (self, entity, sender)
		-- the team leader wants everyone to keep formation
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.KEEP_FORMATION", EntityName(entity))
	end,
	---------------------------------------------	
	BREAK_FORMATION = function (self, entity, sender)
		-- the team can split
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.BREAK_FORMATION", EntityName(entity))
	end,
	---------------------------------------------	
	SINGLE_GO = function (self, entity, sender)
		-- the team leader has instructed this group member to approach the enemy
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.SINGLE_GO", EntityName(entity))
	end,
	---------------------------------------------	
	GROUP_COVER = function (self, entity, sender)
		-- the team leader has instructed this group member to cover his friends
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.GROUP_COVER", EntityName(entity))
	end,
	---------------------------------------------	
	IN_POSITION = function (self, entity, sender)
		-- some member of the group is safely in position
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.IN_POSITION", EntityName(entity))
	end,
	---------------------------------------------	
	GROUP_SPLIT = function (self, entity, sender)
		-- team leader instructs group to split
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.GROUP_SPLIT", EntityName(entity))
	end,
	---------------------------------------------	
	PHASE_RED_ATTACK = function (self, entity, sender)
		-- team leader instructs red team to attack
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.PHASE_RED_ATTACK", EntityName(entity))
	end,
	---------------------------------------------	
	PHASE_BLACK_ATTACK = function (self, entity, sender)
		-- team leader instructs black team to attack
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.PHASE_BLACK_ATTACK", EntityName(entity))
	end,
	---------------------------------------------	
	GROUP_MERGE = function (self, entity, sender)
		-- team leader instructs groups to merge into a team again
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.GROUP_MERGE", EntityName(entity))
	end,
	---------------------------------------------	
	CLOSE_IN_PHASE = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.CLOSE_IN_PHASE", EntityName(entity))
	end,
	---------------------------------------------	
	ASSAULT_PHASE = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.ASSAULT_PHASE", EntityName(entity))
	end,
	---------------------------------------------	
	GROUP_NEUTRALISED = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
		LogAlways("[%s] TOS_Obey_Follow_and_Protect.GROUP_NEUTRALISED", EntityName(entity))
	end,
}