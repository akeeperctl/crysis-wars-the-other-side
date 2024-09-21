--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

Script.ReloadScript("Scripts/Utils/TOSCommon.lua")
Script.ReloadScript("Scripts/Utils/TOSTable.lua")

AIBehaviour.TOS_Obey = {
	Name = "TOS_Obey",
	NOPREVIOUS = 1, -- Если 1, то не изменяет предыдущее поведение, записанное в "PREVIOUS"
	-- Alertness = 0,
	-- Exclusive = 1,
	-- LeaveCoverOnStart = 1,

	-- TheOtherSide
	---------------------------------------------
	Constructor = function (self, entity, data)
		-- called when the behaviour is selected
		-- the extra data is from the signal that caused the behavior transition
		LogAlways("[%s] TOS_Obey.Constructor data %s", EntityName(entity), table.safedump(data))
	end,
	---------------------------------------------
	Destructor = function (self, entity, data)
		-- called when the behaviour is de-selected
		-- the extra data is from the signal that is causing the behavior transition
		LogAlways("[%s] TOS_Obey.Destructor data %s", EntityName(entity), EntityName(data))
	end,
	-- ~TheOtherSide

	-- События из кода CRYENGINE 5
	-- Вызывается только OnActionDone
	OnActionDone = function (self, entity, data, sender)
		LogAlways("[%s] TOS_Obey.OnActionDone sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	-- OnActionCreated = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey.OnActionCreated sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionEnd = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey.OnActionEnd sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,

	-- OnActionStart = function (self, entity, data, sender)
	-- 	LogAlways("[%s] TOS_Obey.OnActionStart sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	-- end,
	---------------------------------------------
	OnObjectSeen = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey.OnObjectSeen data: %s", EntityName(entity), table.safedump(data))
	end,

	---------------------------------------------
	OnFriendInWay = function( self, entity, data )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey.OnFriendInWay data: %s", EntityName(entity), table.safedump(data))
	end,

	--------------------------------------------------------------------------
	OnSeenByEnemy = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey.OnSeenByEnemy sender: %s", EntityName(entity), EntityName(sender))
	end,
	--------------------------------------------------------------------------
	OnCloseContact= function( self, entity )
		LogAlways("[%s] TOS_Obey.OnCloseContact sender: %s", EntityName(entity))
	end,

	OnTargetDead = function( self, entity )
		-- called when the attention target died
		LogAlways("[%s] TOS_Obey.OnTargetDead", EntityName(entity))
	end,

	OnTargetNavTypeChanged= function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey.OnTargetNavTypeChanged sender: %s, data: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------
	OnTargetCloaked = function(self, entity)
		LogAlways("[%s] TOS_Obey.OnTargetCloaked", EntityName(entity))
	end,

	---------------------------------------------
	ORDER_TIMEOUT = function( self, entity, sender,data )
		LogAlways("[%s] TOS_Obey.ORDER_TIMEOUT", EntityName(entity))
	end,
	
	---------------------------------------------
	ORDER_ACQUIRE_TARGET = function(self , entity, sender, data)
		LogAlways("[%s] TOS_Obey.ORDER_ACQUIRE_TARGET sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	---------------------------------------------------------------------------------------------------------------------------------------
	ORDER_COVER_SEARCH = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.ORDER_COVER_SEARCH sender: %s", EntityName(entity), EntityName(sender))
	end,	

	---------------------------------------------------------------------------------------------------------------------------------------
	ORD_ATTACK = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.ORD_ATTACK sender: %s", EntityName(entity), EntityName(sender))
	end,

	ORD_DONE = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey.ORD_DONE sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnReinforcementRequested = function ( self, entity, sender, extraData )
		LogAlways("[%s] TOS_Obey.OnReinforcementRequested sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnCallReinforcement = function(self, entity, sender, extraData)
		LogAlways("[%s] TOS_Obey.OnCallReinforcement sender: %s", EntityName(entity), EntityName(sender), table.safedump(extraData))
	end,
	--------------------------------------------------------------------------
	OnPathFound = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey.OnPathFound sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------------------
	OnRequestUpdate = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey.OnRequestUpdate sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnPlayerDied = function( self, entity, sender )
		LogAlways("[%s] TOS_Obey.OnPlayerDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDied = function ( self, entity, sender)
		LogAlways("[%s] TOS_Obey.OnLeaderDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderAssigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity назначается лидер группы через AI.SetLeader(entityId)
		LogAlways("[%s] TOS_Obey.OnLeaderAssigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	---------------------------------------------
	OnLeaderDeassigned = function ( self, entity, sender)
		-- Вызывается когда сущность entity разназначается лидером группы
		LogAlways("[%s] TOS_Obey.OnLeaderDeassigned sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnUnitDied = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.OnUnitDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	OnJoinTeam = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.OnJoinTeam sender: %s", EntityName(entity), EntityName(sender))
	end,

	-----------------------------------------------------------
	GroupMemberDied = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.GroupMemberDied sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	OnBodyFallSound = function(self, entity, sender, data)
		LogAlways("[%s] TOS_Obey.OnBodyFallSound sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnGroupMemberDiedNearest = function ( self, entity, sender)
		LogAlways("[%s] TOS_Obey.OnGroupMemberDiedNearest sender: %s", EntityName(entity), EntityName(sender))
	end,

	-- OnDeath = function ( self, entity, sender)
	-- 	LogAlways("[%s] TOS_Obey.OnDeath sender: %s", EntityName(entity), EntityName(sender))
	-- end,

	--------------------------------------------------
	ENEMYSEEN_FIRST_CONTACT = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.ENEMYSEEN_FIRST_CONTACT sender: %s", EntityName(entity), EntityName(sender))
	end,

	--------------------------------------------------
	ENEMYSEEN_DURING_COMBAT = function(self,entity,sender)
		LogAlways("[%s] TOS_Obey.ENEMYSEEN_DURING_COMBAT sender: %s", EntityName(entity), EntityName(sender))
	end,

	OnBackOffFailed = function(self,entity)
		LogAlways("[%s] TOS_Obey.OnBackOffFailed sender: %s", EntityName(entity))
	end,

	---------------------------------------------
	OnVehicleDanger = function(self,entity)
		LogAlways("[%s] TOS_Obey.OnVehicleDanger sender: %s", EntityName(entity))
	end,
	
	---------------------------------------------
	OnExplosionDanger = function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey.OnExplosionDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(data))
	end,

	--------------------------------------------------
	OnGrenadeDanger = function( self, entity, sender, signalData )
		LogAlways("[%s] TOS_Obey.OnGrenadeDanger sender: %s", EntityName(entity), EntityName(sender), table.safedump(signalData))
	end,

	-- SYSTEM EVENTS			-----
	---------------------------------------------
	OnSelected = function( self, entity )
		LogAlways("[%s] TOS_Obey.OnSelected", EntityName(entity))
	end,
	---------------------------------------------
	OnSpawn = function( self, entity )
		-- called when enemy spawned or reset
		LogAlways("[%s] TOS_Obey.OnSpawn", EntityName(entity))
	end,
	---------------------------------------------
	OnActivate = function( self, entity )
		-- called when enemy receives an activate event (from a trigger, for example)
		LogAlways("[%s] TOS_Obey.OnActivate", EntityName(entity))
	end,
	---------------------------------------------
	OnNoTarget = function( self, entity )
		-- called when the enemy stops having an attention target
		LogAlways("[%s] TOS_Obey.OnNoTarget", EntityName(entity))
	end,
	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance )
		-- called when the enemy sees a living player
		LogAlways("[%s] TOS_Obey.OnPlayerSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnGrenadeSeen = function( self, entity, fDistance )
		-- called when the enemy sees a grenade
		LogAlways("[%s] TOS_Obey.OnGrenadeSeen, fDistance = %i", EntityName(entity), tonumber(fDistance))
	end,
	---------------------------------------------
	OnEnemySeen = function( self, entity )
		-- called when the enemy sees a foe which is not a living player
		LogAlways("[%s] TOS_Obey.OnEnemySeen", EntityName(entity))
	end,
	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		-- called when the enemy sees something that it cant identify
		LogAlways("[%s] TOS_Obey.OnSomethingSeen", EntityName(entity))
	end,
	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		LogAlways("[%s] TOS_Obey.OnEnemyMemory", EntityName(entity))
	end,
	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		-- called when the enemy hears an interesting sound
		LogAlways("[%s] TOS_Obey.OnInterestingSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity )
		-- called when the enemy hears a scary sound
		LogAlways("[%s] TOS_Obey.OnThreateningSoundHeard", EntityName(entity))
	end,
	---------------------------------------------
	OnReload = function( self, entity )
		-- called when the enemy goes into automatic reload after its clip is empty
		LogAlways("[%s] TOS_Obey.OnReload", EntityName(entity))
	end,
	---------------------------------------------
	OnReloadDone = function( self, entity )
		-- called after reloading is done
		LogAlways("[%s] TOS_Obey.OnReloadDone", EntityName(entity))
	end,
	---------------------------------------------
	OnGroupMemberDied = function( self, entity )
		-- called when a member of the group dies
		LogAlways("[%s] TOS_Obey.OnGroupMemberDied", EntityName(entity))
	end,
	---------------------------------------------
	OnNoHidingPlace = function( self, entity, sender )
		-- called when no hiding place can be found with the specified parameters

		LogAlways("[%s] TOS_Obey.OnNoHidingPlace, sender: %s", EntityName(entity), EntityName(sender))
	end,	
	--------------------------------------------------
	OnNoFormationPoint = function ( self, entity, sender)
		-- called when the enemy found no formation point

		LogAlways("[%s] TOS_Obey.OnNoFormationPoint, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnReceivingDamage = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOS_Obey.OnReceivingDamage, sender: %s", EntityName(entity), EntityName(sender))
	end,
	---------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
		-- called when the enemy is damaged

		LogAlways("[%s] TOS_Obey.OnCoverRequested, sender: %s", EntityName(entity), EntityName(sender))

	end,
	--------------------------------------------------
	OnBulletRain = function ( self, entity, sender)
		-- called when the enemy detects bullet trails around him

		LogAlways("[%s] TOS_Obey.OnBulletRain, sender: %s", EntityName(entity), EntityName(sender))

	end,

	-- ИЗ ДРУГИХ ПОВЕДЕНИЙ
	---------------------------------------------
	OnTargetApproaching	= function (self, entity)
		LogAlways("[%s] TOS_Obey.OnTargetApproaching", EntityName(entity))
	end,
	---------------------------------------------
	OnTargetFleeing	= function (self, entity)
		LogAlways("[%s] TOS_Obey.OnTargetFleeing", EntityName(entity))
	end,
	--------------------------------------------------
	OnCoverCompromised = function(self, entity, sender, data)
		LogAlways("[%s] TOS_Obey.OnCoverCompromised", EntityName(entity))
	end,

	---------------------------------------------
	OnFallAndPlayWakeUp = function( self, entity )
		AI_Utils:CommonContinueAfterReaction(entity);
		LogAlways("[%s] TOS_Obey.OnFallAndPlayWakeUp", EntityName(entity))
	end,

	---------------------------------------------
	OnPlayerTeamKill = function(self,entity,sender,data)
		LogAlways("[%s] TOS_Obey.OnPlayerTeamKill", EntityName(entity))
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