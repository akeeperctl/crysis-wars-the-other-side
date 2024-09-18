--------------------------------------------------
--    Created By: AlienKeeper
--   Description: Поведение для регистрации функций как сигналов из TOS_AI.h
--------------------------
--

AIBehaviour.TOSSHARED = {
	Name = "TOSSHARED",

	-- TheOtherSide
	---------------------------------------------
	Constructor = function (self, entity)
		-- called when the behaviour is selected
		-- the extra data is from the signal that caused the behavior transition
		System.LogAlways(string.format("[%s] TOSSHARED.Constructor", entity:GetName()))
	end,
	---------------------------------------------
	Destructor = function (self, entity,data)
		-- called when the behaviour is de-selected
		-- the extra data is from the signal that is causing the behavior transition
	end,

	ENABLE_COMBAT = function(self, entity, sender, data)
		entity:CancelSubpipe()
		TOS_AI.InsertSubpipe(entity, "devalue_target", AIGOALPIPE_RUN_ONCE, 0, 0)
	end,
	-- ~TheOtherSide

	-- События из CRYENGINE 5
	OnActionCreated = function (self, entity)
		System.LogAlways(string.format("[%s] TOSSHARED.OnActionCreated", entity:GetName()))
	end,

	OnActionDone = function (self, entity)
		System.LogAlways(string.format("[%s] TOSSHARED.OnActionDone", entity:GetName()))
	end,

	OnActionEnd = function (self, entity)
		System.LogAlways(string.format("[%s] TOSSHARED.OnActionEnd", entity:GetName()))
	end,

	OnActionStart = function (self, entity)
		System.LogAlways(string.format("[%s] TOSSHARED.OnActionStart", entity:GetName()))
	end,


	-- SYSTEM EVENTS			-----
	---------------------------------------------
	OnSelected = function( self, entity )
		System.LogAlways(string.format("[%s] TOSSHARED.OnSelected", entity:GetName()))
	end,
	---------------------------------------------
	OnSpawn = function( self, entity )
		-- called when enemy spawned or reset
		System.LogAlways(string.format("[%s] TOSSHARED.OnSpawn", entity:GetName()))
	end,
	---------------------------------------------
	OnActivate = function( self, entity )
		-- called when enemy receives an activate event (from a trigger, for example)
		System.LogAlways(string.format("[%s] TOSSHARED.OnActivate", entity:GetName()))
	end,
	---------------------------------------------
	OnNoTarget = function( self, entity )
		-- called when the enemy stops having an attention target
		System.LogAlways(string.format("[%s] TOSSHARED.OnNoTarget", entity:GetName()))
	end,
	---------------------------------------------
	OnPlayerSeen = function( self, entity, fDistance )
		-- called when the enemy sees a living player
		System.LogAlways(string.format("[%s] TOSSHARED.OnPlayerSeen, fDistance = %i", entity:GetName(), tonumber(fDistance)))
	end,
	---------------------------------------------
	OnGrenadeSeen = function( self, entity, fDistance )
		-- called when the enemy sees a grenade
		System.LogAlways(string.format("[%s] TOSSHARED.OnGrenadeSeen, fDistance = %i", entity:GetName(), tonumber(fDistance)))
	end,
	---------------------------------------------
	OnEnemySeen = function( self, entity )
		-- called when the enemy sees a foe which is not a living player
		System.LogAlways(string.format("[%s] TOSSHARED.OnEnemySeen", entity:GetName()))
	end,
	---------------------------------------------
	OnSomethingSeen = function( self, entity )
		-- called when the enemy sees something that it cant identify
		System.LogAlways(string.format("[%s] TOSSHARED.OnSomethingSeen", entity:GetName()))
	end,
	---------------------------------------------
	OnEnemyMemory = function( self, entity )
		-- called when the enemy can no longer see its foe, but remembers where it saw it last
		System.LogAlways(string.format("[%s] TOSSHARED.OnEnemyMemory", entity:GetName()))
	end,
	---------------------------------------------
	OnInterestingSoundHeard = function( self, entity )
		-- called when the enemy hears an interesting sound
		System.LogAlways(string.format("[%s] TOSSHARED.OnInterestingSoundHeard", entity:GetName()))
	end,
	---------------------------------------------
	OnThreateningSoundHeard = function( self, entity )
		-- called when the enemy hears a scary sound
		System.LogAlways(string.format("[%s] TOSSHARED.OnThreateningSoundHeard", entity:GetName()))
	end,
	---------------------------------------------
	OnReload = function( self, entity )
		-- called when the enemy goes into automatic reload after its clip is empty
		System.LogAlways(string.format("[%s] TOSSHARED.OnReload", entity:GetName()))
	end,
	---------------------------------------------
	OnReloadDone = function( self, entity )
		-- called after reloading is done
		System.LogAlways(string.format("[%s] TOSSHARED.OnReloadDone", entity:GetName()))
	end,
	---------------------------------------------
	OnGroupMemberDied = function( self, entity )
		-- called when a member of the group dies
		System.LogAlways(string.format("[%s] TOSSHARED.OnGroupMemberDied", entity:GetName()))
	end,
	---------------------------------------------
	OnNoHidingPlace = function( self, entity, sender )
		-- called when no hiding place can be found with the specified parameters
		local senderName = "NONE"
		if sender then
			senderName = sender:GetName()
		end

		System.LogAlways(string.format("[%s] TOSSHARED.OnNoHidingPlace, sender: %s", entity:GetName(), senderName))
	end,	
	--------------------------------------------------
	OnNoFormationPoint = function ( self, entity, sender)
		-- called when the enemy found no formation point

		local senderName = "NONE"
		if sender then
			senderName = sender:GetName()
		end

		System.LogAlways(string.format("[%s] TOSSHARED.OnNoFormationPoint, sender: %s", entity:GetName(), senderName))
	end,
	---------------------------------------------
	OnReceivingDamage = function ( self, entity, sender)
		-- called when the enemy is damaged

		local senderName = "NONE"
		if sender then
			senderName = sender:GetName()
		end

		System.LogAlways(string.format("[%s] TOSSHARED.OnReceivingDamage, sender: %s", entity:GetName(), senderName))
	end,
	---------------------------------------------
	OnCoverRequested = function ( self, entity, sender)
		-- called when the enemy is damaged
		local senderName = "NONE"
		if sender then
			senderName = sender:GetName()
		end

		System.LogAlways(string.format("[%s] TOSSHARED.OnCoverRequested, sender: %s", entity:GetName(), senderName))

	end,
	--------------------------------------------------
	OnBulletRain = function ( self, entity, sender)
		-- called when the enemy detects bullet trails around him
		local senderName = "NONE"
		if sender then
			senderName = sender:GetName()
		end

		System.LogAlways(string.format("[%s] TOSSHARED.OnBulletRain, sender: %s", entity:GetName(), senderName))

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

-- System.LogAlways(string.format("TOSSHARED = %s", tostring( AIBehaviour.TOSSHARED)))
-- System.LogAlways(string.format("TOSSHARED.Constructor = %s", tostring( AIBehaviour.TOSSHARED.Constructor)))
-- System.LogAlways(string.format("TOSSHARED.Destructor = %s", tostring( AIBehaviour.TOSSHARED.Destructor)))
-- System.LogAlways(string.format("TOSSHARED.ENABLE_COMBAT = %s", tostring( AIBehaviour.TOSSHARED.ENABLE_COMBAT)))
-- System.LogAlways(string.format("TOSSHARED.OnActionCreated = %s", tostring( AIBehaviour.TOSSHARED.OnActionCreated)))
-- System.LogAlways(string.format("TOSSHARED.OnActionDone = %s", tostring( AIBehaviour.TOSSHARED.OnActionDone)))
-- System.LogAlways(string.format("TOSSHARED.OnActionEnd = %s", tostring( AIBehaviour.TOSSHARED.OnActionEnd)))
-- System.LogAlways(string.format("TOSSHARED.OnActionStart = %s", tostring( AIBehaviour.TOSSHARED.OnActionStart)))
