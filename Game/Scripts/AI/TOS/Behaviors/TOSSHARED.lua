--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

-- Файл используется для обучения и понимания работы поведений в CE2.
-- Не все функции сигналов вызываются, поэтому здесь всюду логи.

Script.ReloadScript("Scripts/Utils/TOSCommon.lua")
Script.ReloadScript("Scripts/Utils/TOSTable.lua")

AIBehaviour.TOSSHARED = {
	Name = "TOSSHARED",
	NOPREVIOUS = 1, -- Если 1, то не изменяет предыдущее поведение, записанное в "PREVIOUS"
	-- Alertness = 0,
	-- Exclusive = 1,
	-- LeaveCoverOnStart = 1,

	-- TheOtherSide
	---------------------------------------------
	Constructor = function (self, entity, data)
		-- called when the behaviour is selected
		-- the extra data is from the signal that caused the behavior transition
		LogAlways("[%s] TOSSHARED.Constructor data %s", EntityName(entity), table.safedump(data))
	end,
	---------------------------------------------
	Destructor = function (self, entity, data)
		-- called when the behaviour is de-selected
		-- the extra data is from the signal that is causing the behavior transition
		LogAlways("[%s] TOSSHARED.Destructor data %s", EntityName(entity), EntityName(data))
	end,
	-- ~TheOtherSide

	-- События из кода CRYENGINE 5
	-- Вызывается только OnActionDone
	OnActionDone = function (self, entity, data, sender)
		LogAlways("[%s] TOSSHARED.OnActionDone sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	-- OnActionCreated = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOSSHARED.OnActionCreated sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionEnd = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOSSHARED.OnActionEnd sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionStart = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOSSHARED.OnActionStart sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,
	---------------------------------------------
	OnObjectSeen = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOSSHARED.OnObjectSeen data: %s", EntityName(entity), table.safedump(data))
	end,

	---------------------------------------------
	OnFriendInWay = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOSSHARED.OnFriendInWay data: %s", EntityName(entity), table.safedump(data))
	end,

	--------------------------------------------------------------------------
	OnSeenByEnemy = function( self, entity, sender )
		LogAlways("[%s] TOSSHARED.OnSeenByEnemy sender: %s", EntityName(entity), EntityName(sender))
	end,
	--------------------------------------------------------------------------
	OnCloseContact= function( self, entity )
		LogAlways("[%s] TOSSHARED.OnCloseContact sender: %s", EntityName(entity))
	end,

	OnTargetDead = function( self, entity )
		-- called when the attention target died
		LogAlways("[%s] TOSSHARED.OnTargetDead", EntityName(entity))
	end,

	OnTargetNavTypeChanged= function(self,entity,sender,data)
		LogAlways("[%s] TOSSHARED.OnTargetNavTypeChanged sender: %s, data: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------
	OnTargetCloaked = function(self, entity)
		LogAlways("[%s] TOSSHARED.OnTargetCloaked", EntityName(entity))
	end,

	---------------------------------------------
	ORDER_TIMEOUT = function( self, entity, sender,data )
		LogAlways("[%s] TOSSHARED.ORDER_TIMEOUT", EntityName(entity))
	end,
	
	---------------------------------------------
	ORDER_ACQUIRE_TARGET = function(self , entity, sender, data)
		LogAlways("[%s] TOSSHARED.ORDER_ACQUIRE_TARGET sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------------------------------------------------------------------------------------------------
	ORDER_COVER_SEARCH = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.ORDER_COVER_SEARCH sender: %s", EntityName(entity), EntityName(sender))
	end,	

	---------------------------------------------------------------------------------------------------------------------------------------
	ORD_ATTACK = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.ORD_ATTACK sender: %s", EntityName(entity), EntityName(sender))
	end,

	ORD_DONE = function( self, entity, sender )
		LogAlways("[%s] TOSSHARED.ORD_DONE sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnReinforcementRequested = function ( self, entity, sender, extraData )
		LogAlways("[%s] TOSSHARED.OnReinforcementRequested sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnCallReinforcement = function(self, entity, sender, extraData)
		LogAlways("[%s] TOSSHARED.OnCallReinforcement sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnPathFound = function( self, entity, sender )
		LogAlways("[%s] TOSSHARED.OnPathFound sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------------------
	OnRequestUpdate = function( self, entity, sender )
		LogAlways("[%s] TOSSHARED.OnRequestUpdate sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnPlayerDied = function( self, entity, sender )
		LogAlways("[%s] TOSSHARED.OnPlayerDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDied = function ( self, entity, sender)
		LogAlways("[%s] TOSSHARED.OnLeaderDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderAssigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity назначается лидер группы через AI.SetLeader(entityId)
		LogAlways("[%s] TOSSHARED.OnLeaderAssigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDeassigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity разназначается лидером группы
		LogAlways("[%s] TOSSHARED.OnLeaderDeassigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnUnitDied = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.OnUnitDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnJoinTeam = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.OnJoinTeam sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	GroupMemberDied = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.GroupMemberDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	OnBodyFallSound = function(self, entity, sender, data)
		LogAlways("[%s] TOSSHARED.OnBodyFallSound sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnGroupMemberDiedNearest = function ( self, entity, sender)
		LogAlways("[%s] TOSSHARED.OnGroupMemberDiedNearest sender: %s", EntityName(entity), EntityName(sender))
	end,

	-- OnDeath = function ( self, entity, sender)
	-- 	LogAlways("[%s] TOSSHARED.OnDeath sender: %s", EntityName(entity), EntityName(sender))
	-- end,

	--------------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.ENEMYSEEN_FIRST_CONTACT sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	ENEMYSEEN_DURING_COMBAT = function(self,entity,sender)
		LogAlways("[%s] TOSSHARED.ENEMYSEEN_DURING_COMBAT sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnBackOffFailed = function(self,entity)
		LogAlways("[%s] TOSSHARED.OnBackOffFailed sender: %s", EntityName(entity))
	end,

	---------------------------------------------
	OnVehicleDanger = function(self,entity)
		LogAlways("[%s] TOSSHARED.OnVehicleDanger sender: %s", EntityName(entity))
	end,
	
	---------------------------------------------
	OnExplosionDanger = function(self,entity,sender,data)
		LogAlways("[%s] TOSSHARED.OnExplosionDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	--------------------------------------------------
	OnGrenadeDanger = function( self, entity, sender, signalData )
		LogAlways("[%s] TOSSHARED.OnGrenadeDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(signalData))
	end,

	-- SYSTEM EVENTS			-----
	---------------------------------------------
	OnSelected = function( self, entity )
		LogAlways("[%s] TOSSHARED.OnSelected", EntityName(entity))
	end,
	---------------------------------------------
	OnSpawn = function( self, entity )
		-- called when enemy spawned or reset
		LogAlways("[%s] TOSSHARED.OnSpawn", EntityName(entity))
	end,
	---------------------------------------------
	OnActivate = function( self, entity )
		-- called when enemy receives an activate event (from a trigger, for example)
		LogAlways("[%s] TOSSHARED.OnActivate", EntityName(entity))
	end,
	---------------------------------------------
	OnNoTarget = function( self, entity )
		-- called when the enemy stops having an attention target
		LogAlways("[%s] TOSSHARED.OnNoTarget", EntityName(entity))
	end,
	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance )
		-- called when the enemy sees a living player
		LogAlways("[%s] TOSSHARED.OnPlayerSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnGrenadeSeen = function( self, entity, fDistance )
		-- called when the enemy sees a grenade
		LogAlways("[%s] TOSSHARED.OnGrenadeSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnEnemySeen = function( self, entity )
		-- called when the enemy sees a foe which is not a living player
		LogAlways("[%s] TOSSHARED.OnEnemySeen", EntityName(entity))
	end,
	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		-- called when the enemy sees something that it cant identify
		LogAlways("[%s] TOSSHARED.OnSomethingSeen", EntityName(entity))
	end,
	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOSSHARED.OnEnemyMemory", EntityName(entity))
	end,
	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		-- called when the enemy hears an interesting sound
		LogAlways("[%s] TOSSHARED.OnInterestingSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity )
		-- called when the enemy hears a scary sound
		LogAlways("[%s] TOSSHARED.OnThreateningSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnReload = function( self, entity )
		-- called when the enemy goes into automatic reload after its clip is empty
		LogAlways("[%s] TOSSHARED.OnReload", EntityName(entity))
	end,
	---------------------------------------------
	OnReloadDone = function( self, entity )
		-- called after reloading is done
		LogAlways("[%s] TOSSHARED.OnReloadDone", EntityName(entity))
	end,
	---------------------------------------------
	OnGroupMemberDied = function( self, entity )
		-- called when a member of the group dies
		LogAlways("[%s] TOSSHARED.OnGroupMemberDied", EntityName(entity))
	end,
	---------------------------------------------
	OnNoHidingPlace = function( self, entity, sender )
		-- called when no hiding place can be found with the specified parameters

		LogAlways("[%s] TOSSHARED.OnNoHidingPlace, sender: %s", EntityName(entity), EntityName(sender))
	end,	
	--------------------------------------------------
	OnNoFormationPoint = function ( self, entity, sender)
		-- called when the enemy found no formation point

		LogAlways("[%s] TOSSHARED.OnNoFormationPoint, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnReceivingDamage = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOSSHARED.OnReceivingDamage, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOSSHARED.OnCoverRequested, sender: %s", EntityName(entity), EntityName(sender))

	end,
	--------------------------------------------------
	OnBulletRain = function ( self, entity, sender)
		-- called when the enemy detects bullet trails around him

		LogAlways("[%s] TOSSHARED.OnBulletRain, sender: %s", EntityName(entity), EntityName(sender))

	end,

	-- ИЗ ДРУГИХ ПОВЕДЕНИЙ
	---------------------------------------------
	OnTargetApproaching	= function (self, entity)
		LogAlways("[%s] TOSSHARED.OnTargetApproaching", EntityName(entity))
	end,
	---------------------------------------------
	OnTargetFleeing	= function (self, entity)
		LogAlways("[%s] TOSSHARED.OnTargetFleeing", EntityName(entity))
	end,
	--------------------------------------------------
	OnCoverCompromised = function(self, entity, sender, data)
		LogAlways("[%s] TOSSHARED.OnCoverCompromised", EntityName(entity))
	end,

	---------------------------------------------
	OnFallAndPlayWakeUp = function( self, entity )
		AI_Utils:CommonContinueAfterReaction(entity);
		LogAlways("[%s] TOSSHARED.OnFallAndPlayWakeUp", EntityName(entity))
	end,

	---------------------------------------------
	OnPlayerTeamKill = function(self,entity,sender,data)
		LogAlways("[%s] TOSSHARED.OnPlayerTeamKill", EntityName(entity))
	end,
	--------------------------------------------------

	-- GROUP SIGNALS
	---------------------------------------------	
	KEEP_FORMATION = function (self, entity, sender)
		-- the team leader wants everyone to keep formation
	end,
	---------------------------------------------	
	BREAK_FORMATION = function (self, entity, sender)
		-- the team can split
	end,
	---------------------------------------------	
	SINGLE_GO = function (self, entity, sender)
		-- the team leader has instructed this group member to approach the enemy
	end,
	---------------------------------------------	
	GROUP_COVER = function (self, entity, sender)
		-- the team leader has instructed this group member to cover his friends
	end,
	---------------------------------------------	
	IN_POSITION = function (self, entity, sender)
		-- some member of the group is safely in position
	end,
	---------------------------------------------	
	GROUP_SPLIT = function (self, entity, sender)
		-- team leader instructs group to split
	end,
	---------------------------------------------	
	PHASE_RED_ATTACK = function (self, entity, sender)
		-- team leader instructs red team to attack
	end,
	---------------------------------------------	
	PHASE_BLACK_ATTACK = function (self, entity, sender)
		-- team leader instructs black team to attack
	end,
	---------------------------------------------	
	GROUP_MERGE = function (self, entity, sender)
		-- team leader instructs groups to merge into a team again
	end,
	---------------------------------------------	
	CLOSE_IN_PHASE = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
	end,
	---------------------------------------------	
	ASSAULT_PHASE = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
	end,
	---------------------------------------------	
	GROUP_NEUTRALISED = function (self, entity, sender)
		-- team leader instructs groups to initiate part one of assault fire maneuver
	end,
}