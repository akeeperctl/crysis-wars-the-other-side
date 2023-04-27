--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2008.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character script for Advanced Trooper Drone
--  
--------------------------------------------------------------------------
--  History:
--  - 30/01/2008   : Created by Denisz Polgar
--
--------------------------------------------------------------------------

AICharacter.TrooperMKII = {

	Class = UNIT_CLASS_INFANTRY,
	TeamRole = GU_HUMAN_COVER,
	--TeamRole = GU_HUMAN_SNEAKER,

	Constructor = function(self,entity)
		entity.AI.target = {x=0, y=0, z=0};
		entity.AI.targetFound = 0;
		AI_Utils:SetupTerritory(entity);
		AI_Utils:SetupStandby(entity, true);
		AI.NotifyGroupTacticState(entity.id, 0, GN_INIT, 0);
		
		entity.AI.FireMode = 0;
	end,

--------------------------------------------------------------------------
-- COMPOSITE ANY
--------------------------------------------------------------------------
	AnyBehavior = 
	{
		-- Defaults
		RETURN_TO_FIRST			= "FIRST",
		TO_PREVIOUS				= "PREVIOUS",
		TO_IDLE					= "TrooperMKIIIdle",
		
		-- COMMON
		OnFallAndPlay			= "TrooperDown",
		OnExplosionDanger		= "Trooper2AvoidExplosives",
		GO_TO_DUMB				= "Trooper2Dumb",
		-- /COMMON
	
		-- HUMAN	
		-- Combat states
		TO_ATTACK				= "Trooper2Attack",
		TO_ATTACK_GROUP			= "Trooper2AttackGroup",
		TO_HIDE					= "Trooper2Hide",
		TO_THREATENED			= "Trooper2Threatened",
		TO_THREATENED_STANDBY	= "Trooper2ThreatenedStandby",
		TO_INTERESTED			= "Trooper2Interested",
		TO_SEEK					= "Trooper2Seek",
		TO_SEARCH				= "Trooper2Search",
-- EXTINCT!			TO_RELOAD				= "Trooper2Reload",
		
		-- Miscellaneous
		GO_TO_AVOIDEXPLOSIVES	= "Trooper2AvoidExplosives",
		GO_TO_AVOIDVEHICLE		= "Trooper2AvoidVehicle",
		GO_TO_CHECKDEAD			= "Trooper2CheckDead",
		TO_CALL_REINFORCEMENTS	= "Trooper2CallReinforcements",

		-- To Remove
-- Don't use		TO_AVOID_TANK			= "Cover2AvoidTank",
-- Don't use		TO_RPG_ATTACK			= "Cover2RPGAttack",
-- EXTINCT!			TO_RUSH_ATTACK			= "Cover2RushAttack",
-- Don't use		TO_PANIC				= "Cover2Panic",
			
		-- Vehicles & mounts
-- Don't use		ENTERING_VEHICLE		= "EnteringVehicle",
-- Don't use		USE_MOUNTED_WEAPON		= "UseMounted",
-- Don't use		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",
		
		-- Miscellaneous	
-- EXTINCT!			TO_STATIC 				= "HBaseStaticShooter",
		-- /To Remove
		-- /HUMAN
		
		-- TROOPER
--		OnAttackShootSpot					= "Trooper2ShootOnSpot",
		OnAttackSwitchPosition				= "Trooper2AttackSwitchPosition",
		
		GO_TO_GRABBED						= "Trooper2GrabbedByScout",
		
		-- Smart Objects
		GO_TO_ON_ROCK						= "Trooper2ShootOnRock",
		GO_TO_ON_WALL						= "Trooper2ShootOnWall",
		-- /Smart Objects
		-- /TROOPER
		
		-- Additional switch control
		CONTROL_TROOPER					= "Trooper2AttackT",
		CONTROL_COVER					= "Trooper2Attack",
		
		-- These are duplicate now, but may be used for refining later (NOT USED ATM)
		CHOOSE_TO_SEEK					= "Trooper2Seek",
		CHOOSE_TO_SEARCH				= "Trooper2Search",
		CHOOSE_TO_ATTACK				= "Trooper2Attack"

	},

-- HUMAN
	TrooperMKIIIdle = 
	{
		OnPlayerSeen					= "",
		OnInterestingSoundHeard			= "",
		OnSomethingSeen					= "",
		OnThreateningSoundHeard			= "",
		OnBulletRain					= "",
		OnNearMiss						= "",
		OnEnemyDamage					= "",
		OnGroupMemberDiedNearest 		= "",
		OnSomebodyDied					= "",
		ENEMYSEEN_FIRST_CONTACT	 		= "",
		ENEMYSEEN_DURING_COMBAT			= "",
		OnExplosionDanger				= "HBaseGrenadeRun",
	},
	
	Trooper2AvoidExplosives = 
	{
		GO_TO_IDLE =					"TrooperMKIIIdle",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =					"",
		OnBulletRain =					"",
		OnGrenadeSeen =					"",
		OnInterestingSoundHeard =		"",
		OnThreateningSoundHeard =		"",
		OnSomethingSeen =				"",
		OnNoTarget = 					"",
	},

	Trooper2AvoidVehicle = 
	{
		GO_TO_IDLE =					"TrooperMKIIIdle",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
	},
	
	Trooper2Attack = {},
	
	Trooper2Hide = {},
	
	Trooper2Threatened = {},

	Trooper2Interested = {},
		
	Trooper2Seek = {},

	Trooper2Search = {},

	Trooper2Reload = {},

	Trooper2CheckDead = {},

	Trooper2CallReinforcements = {},

-- /HUMAN

-- TROOPER
	TrooperDown = 
	{
		--GO_TO_IDLE =					"TrooperMKIIIdle",
		OnFallAndPlayWakeUp = 			"Trooper2Attack",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =					"",
		OnTankSeen =					"",
		OnHeliSeen =					"",
		OnBulletRain =					"",
		OnGrenadeSeen =					"",
		OnInterestingSoundHeard =		"",
		OnThreateningSoundHeard =		"",
		entered_vehicle	=				"",
		exited_vehicle =				"",
		exited_vehicle_investigate =	"",
		OnSomethingSeen =				"",
		GO_TO_SEEK =					"",
		GO_TO_IDLE = 					"",
		GO_TO_SEARCH =					"",
		GO_TO_ATTACK =					"",
		GO_TO_AVOIDEXPLOSIVES =			"",
		GO_TO_ALERT =					"",
		GO_TO_CHECKDEAD =				"",
	},
	
	Trooper2Dumb = 
	{	
		GO_TO_IDLE =					"TrooperMKIIIdle",
		GO_TO_SEARCH =					"Trooper2Search",
		DODGE =							"",
		DODGE_GRENADE =					"",
		RESUME_FOLLOWING =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =					"",
		OnTankSeen =					"",
		OnHeliSeen =					"",
		OnBulletRain =					"",
		OnGrenadeSeen =					"",
		OnInterestingSoundHeard =		"",
		OnThreateningSoundHeard =		"",
		OnSomethingSeen =				"",
		TO_ATTACK =						"",
		TO_HIDE =						"",
		TO_THREATENED =					"",
		TO_THREATENED_STANDBY =			"",
		TO_INTERESTED =					"",
		TO_SEEK =						"",
		TO_SEARCH =						"",
		TO_RELOAD =						"",
		TO_CALL_REINFORCEMENTS =		"",
		ENEMYSEEN_FIRST_CONTACT =		"",
		ENEMYSEEN_DURING_COMBAT =		"",
	},
	
	Trooper2GrabbedByScout= 
	{
		GO_TO_IDLE						= "TrooperMKIIIdle",
		GO_TO_SWITCH_POSITION			= "Trooper2AttackSwitchPosition",
		GO_TO_SEARCH					= "Trooper2Search",
		GRABBED_TO_INTERESTED			= "Trooper2Interested",
		GRABBED_TO_ATTACK				= "Trooper2Attack",
		
		GO_TO_INTERESTED				= "",
		GO_TO_ATTACK					= "",
		OnAttackSwitchPosition			= "",
		OnAttackShootSpot				= "",
		GO_TO_MOAR						= "",
--		GO_TO_ON_SPOT					= "Trooper2ShootOnSpot",
--		GO_TO_SPECIAL_ACTION			= "TrooperAttackSpecialAction",
		OnSpecialAction					= "",
		GO_TO_ON_ROCK					= "",
		GO_TO_ON_WALL					= "",
		ORDER_SEARCH					= "",
		ORDER_COVER_SEARCH				= "",
		GO_TO_MELEE						= "",
		GO_TO_DODGE						= "",
		GO_TO_ATTACK_JUMP				= "",
		OnAttackChase					= "",
		OnExplosionDanger				= "",
		PURSUE							= "",
		OnFallAndPlay					= "",	
		--ORDER_SEARCH = "",
	},
	
	Trooper2AttackSwitchPosition = 
	{
		GO_TO_JUMP				= "",
		GO_TO_MELEE				= "Trooper2AttackSwitchPositionMelee",
		OnAttackSwitchPosition	= "",
		GO_TO_SWITCH_POSITION	= "Trooper2AttackSwitchPosition",
	},
	
	Trooper2AttackSwitchPositionMelee = 
	{
		END_MELEE				= "Trooper2AttackSwitchPosition",
		MELEE_FAILED			= "Trooper2AttackSwitchPosition",
		OnAttackShootSpot		= "",
		OnAttackSwitchPosition	= "",
		OnLand					= "Trooper2AttackSwitchPosition",
		BackToSwitchPosition	= "Trooper2AttackSwitchPosition",
	},
	
-- Smart Objects
--	Trooper2ShootOnSpot = 
--	{
--		GO_TO_MELEE				= "TrooperAttackSwitchPositionMelee",
--
--	},
	
	Trooper2ShootOnRock = 
	{
		OnSpecialAction = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION = "Trooper2AttackSwitchPosition",
	},
	
	Trooper2ShootOnWall = 
	{
		OnSpecialAction = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION = "Trooper2AttackSwitchPosition",
	},
-- /Smart Objects
-- /TROOPER
}

