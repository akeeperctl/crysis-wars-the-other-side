--------------------------------------------------
--    Created By: AlienKeeper
--   Description: Поведение для регистрации функций как сигналов из TOS_AI.h
--------------------------
--
Script.ReloadScript("Scripts/Utils/TOSTable.lua")

AIBehaviour.TOSSHARED = {
	Name = "TOSSHARED",
	-- NOPREVIOUS = 1,
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

	-- События из CRYENGINE 5
	OnActionCreated = function (self, entity, data, sender)
		LogAlways("[%s] TOSSHARED.OnActionCreated sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	OnActionDone = function (self, entity, data, sender)
		LogAlways("[%s] TOSSHARED.OnActionDone sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	OnActionEnd = function (self, entity, data, sender)
		LogAlways("[%s] TOSSHARED.OnActionEnd sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
	end,

	OnActionStart = function (self, entity, data, sender)
		LogAlways("[%s] TOSSHARED.OnActionStart sender: %s, data: %s", EntityName(entity), EntityName(sender), EntityName(data))
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

-- LogAlways("TOSSHARED = %s", tostring( AIBehaviour.TOSSHARED))
-- LogAlways("TOSSHARED.Constructor = %s", tostring( AIBehaviour.TOSSHARED.Constructor))
-- LogAlways("TOSSHARED.Destructor = %s", tostring( AIBehaviour.TOSSHARED.Destructor))
-- LogAlways("TOSSHARED.ENABLE_COMBAT = %s", tostring( AIBehaviour.TOSSHARED.ENABLE_COMBAT))
-- LogAlways("TOSSHARED.OnActionCreated = %s", tostring( AIBehaviour.TOSSHARED.OnActionCreated))
-- LogAlways("TOSSHARED.OnActionDone = %s", tostring( AIBehaviour.TOSSHARED.OnActionDone))
-- LogAlways("TOSSHARED.OnActionEnd = %s", tostring( AIBehaviour.TOSSHARED.OnActionEnd))
-- LogAlways("TOSSHARED.OnActionStart = %s", tostring( AIBehaviour.TOSSHARED.OnActionStart))
