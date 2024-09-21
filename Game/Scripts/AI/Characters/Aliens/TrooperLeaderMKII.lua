--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2008.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character script for Trooper Leader Drone
--
--------------------------------------------------------------------------
--  History:
--  - 24/7/2007		: Created by Denisz Polgar
--	- 10/3/2008		: Conformed to MK II troopers	
--------------------------------------------------------------------------

AICharacter.TrooperLeaderMKII = {

	Class = UNIT_CLASS_INFANTRY,
	TeamRole = GU_HUMAN_COVER,

	Constructor = function(self, entity)
		entity.AI.target = { x = 0, y = 0, z = 0 };
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
		--TheOtherSide
		GO_TO_TOSSHARED           = "TOSSHARED",
		GO_TO_TOS_OBEY            = "TOS_Obey",
		--~TheOtherSide

		-- Defaults
		RETURN_TO_FIRST           = "FIRST",
		GO_TO_PREVIOUS            = "PREVIOUS",
		GO_TO_IDLE                = "TrooperLeaderMKIIIdle",

		-- COMMON
		OnFallAndPlay             = "TrooperDown",
		OnExplosionDanger         = "Trooper2AvoidExplosives",
		GO_TO_DUMB                = "Trooper2Dumb",
		-- /COMMON

		-- HUMAN	
		-- Combat states
		GO_TO_ATTACK              = "Trooper2Attack",
		GO_TO_ATTACK_GROUP        = "Trooper2AttackGroup",
		GO_TO_HIDE                = "Trooper2Hide",
		GO_TO_THREATENED          = "Trooper2Threatened",
		GO_TO_THREATENED_STANDBY  = "Trooper2ThreatenedStandby",
		GO_TO_INTERESTED          = "Trooper2Interested",
		GO_TO_SEEK                = "Trooper2Seek",
		GO_TO_SEARCH              = "Trooper2Search",
		-- EXTINCT!			GO_TO_RELOAD				= "Trooper2Reload",

		-- Miscellaneous
		GO_TO_AVOIDEXPLOSIVES     = "Trooper2AvoidExplosives",
		GO_TO_AVOIDVEHICLE        = "Trooper2AvoidVehicle",
		GO_TO_CHECKDEAD           = "Trooper2CheckDead",
		GO_TO_CALL_REINFORCEMENTS = "Trooper2CallReinforcements",

		-- To Remove
		-- Don't use		GO_TO_AVOID_TANK			= "Cover2AvoidTank",
		-- Don't use		GO_TO_RPG_ATTACK			= "Cover2RPGAttack",
		-- EXTINCT!			GO_TO_RUSH_ATTACK			= "Cover2RushAttack",
		-- Don't use		GO_TO_PANIC				= "Cover2Panic",

		-- Vehicles & mounts
		-- Don't use		ENTERING_VEHICLE		= "EnteringVehicle",
		-- Don't use		USE_MOUNTED_WEAPON		= "UseMounted",
		-- Don't use		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",

		-- Miscellaneous	
		-- EXTINCT!			GO_TO_STATIC 				= "HBaseStaticShooter",
		-- /To Remove
		-- /HUMAN

		-- TROOPER
		--		OnAttackShootSpot					= "Trooper2ShootOnSpot",
		OnAttackSwitchPosition    = "Trooper2AttackSwitchPosition",

		GO_TO_GRABBED             = "Trooper2GrabbedByScout",

		-- Smart Objects
		GO_TO_ON_ROCK             = "Trooper2ShootOnRock",
		GO_TO_ON_WALL             = "Trooper2ShootOnWall",
		-- /Smart Objects
		-- /TROOPER

		-- Additional switch control
		CONTROL_TROOPER           = "Trooper2AttackT",
		CONTROL_COVER             = "Trooper2Attack",

		-- These are duplicate now, but may be used for refining later (NOT USED ATM)
		CHOOSE_TO_SEEK            = "Trooper2Seek",
		CHOOSE_TO_SEARCH          = "Trooper2Search",
		CHOOSE_TO_ATTACK          = "Trooper2Attack"

	},
	--TheOtherSide
	TOS_Obey = {
		-- Стандартные сигналы
		-- переопределяют те, что в AnyBehavior
		RESUME_FOLLOWING                 = "",
		ENTERING_VEHICLE                 = "",
		USE_MOUNTED_WEAPON               = "",
		OnPlayerSeen                     = "",
		OnTankSeen                       = "",
		OnHeliSeen                       = "",
		OnBulletRain                     = "",
		OnGrenadeSeen                    = "",
		OnInterestingSoundHeard          = "",
		OnThreateningSoundHeard          = "",
		entered_vehicle                  = "",
		exited_vehicle                   = "",
		exited_vehicle_investigate       = "",
		OnSomethingSeen                  = "",
		GO_TO_AVOIDEXPLOSIVES            = "",
		GO_TO_AVOIDVEHICLE               = "",
		GO_TO_CHECKDEAD                  = "",
		GO_TO_IDLE                       = "",
		GO_TO_ATTACK                     = "",
		GO_TO_ATTACK_GROUP               = "",
		GO_TO_RUSH_ATTACK                = "",
		GO_TO_HIDE                       = "",
		GO_TO_AVOID_TANK                 = "",
		GO_TO_RPG_ATTACK                 = "",
		GO_TO_THREATENED                 = "",
		GO_TO_THREATENED_STANDBY         = "",
		GO_TO_INTERESTED                 = "",
		GO_TO_SEEK                       = "",
		GO_TO_SEARCH                     = "",
		GO_TO_RELOAD                     = "",
		GO_TO_CALL_REINFORCEMENTS        = "",
		GO_TO_PANIC                      = "",
		GO_TO_STATIC                     = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "TrooperLeaderMKIIIdle",
		OnBackOffFailed                  = "",

		ORDER_FORM                       = "",
		ORDER_MOVE                       = "",
		MOVE                             = "",
		JOIN_TEAM                        = "",
		ORDER_APPROACH                   = "",
		ORDER_SEARCH                     = "",
		ORDER_COVER_SEARCH               = "",
		ORDER_HIDE_AROUND                = "",
		ORDER_FLANK                      = "",
		ORDER_FIRE                       = "",
		GO_TO_AMBUSH                     = "",
		GO_TO_DUMB                       = "",
		GO_TO_DEFENSE                    = "",
		GO_TO_MOAR                       = "",
		GO_TO_DODGE                      = "",
		GO_TO_RETREAT                    = "",
		GO_TO_GRABBED                    = "",
		GO_TO_ATTACK_JUMP                = "",
		GUARDIAN_ENSLAVE                 = "",

		OnAttackChase                    = "",
		DODGE_GRENADE                    = "",
		OnExplosionDanger                = "",
		PURSUE                           = "",
		OnAttackShootSpot                = "",
		GO_TO_ON_ROCK                    = "",
		GO_TO_ON_WALL                    = "",
		OnAttackSwitchPosition           = "",
		OnSpecialAction                  = "",
		GO_TO_SPECIAL_ACTION             = "",
		OnFallAndPlay                    = "",
		CONTROL_TROOPER                  = "",
		CONTROL_COVER                    = "",
		CHOOSE_TO_SEEK                   = "",
		CHOOSE_TO_SEARCH                 = "",
		CHOOSE_TO_ATTACK                 = "",
		DODGE                            = "",
		ORDER_COORDINATED_FIRE1          = "",
		ORDER_COORDINATED_FIRE2          = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		--TR Leader
		DODGE_FORCED                     = "TrooperDodge",
		ORDER_COORDINATED_FIRE1_FORCED   = "TrooperLeaderCollectiveFire",
		ORDER_COORDINATED_FIRE2_FORCED   = "TrooperCollectiveFire2",

		--TR1
		GO_TO_IDLE_FORCED                = "TrooperLeaderMKIIIdle",
		ORDER_FORM_FORCED                = "TrooperForm",
		ORDER_MOVE_FORCED                = "TrooperMove",
		MOVE_FORCED                      = "TrooperMove",
		ORDER_APPROACH_FORCED            = "TrooperApproach",
		ORDER_SEARCH_FORCED              = "TrGroupSearch",
		ORDER_COVER_SEARCH_FORCED        = "TrGroupCoverSearch",
		ORDER_HIDE_AROUND_FORCED         = "TrooperLeaderAttack",
		ORDER_FLANK_FORCED               = "TrooperAttackFlank",
		ORDER_FIRE_FORCED                = "TrooperLeaderFire",
		GO_TO_AMBUSH_FORCED              = "TrooperAmbush",
		JOIN_TEAM_FORCED                 = "TrooperGroupIdle",
		GO_TO_DEFENSE_FORCED             = "TrooperDefend",
		GO_TO_MOAR_FORCED                = "TrooperAttackMoar",
		GO_TO_DODGE_FORCED               = "TrooperDodge",
		GO_TO_RETREAT_FORCED             = "TrooperRetreat",
		GO_TO_ATTACK_JUMP_FORCED         = "TrooperAttackJump",
		GUARDIAN_ENSLAVE_FORCED          = "TrooperGuardianMinionBasic",
		GO_TO_SPECIAL_ACTION_FORCED      = "TrooperAttackSpecialAction",
		DODGE_GRENADE_FORCED             = "TrooperDodgeGrenade",
		PURSUE_FORCED                    = "TrooperAttackPursue",

		--TR2
		GO_TO_DUMB_FORCED                = "Trooper2Dumb",
		GO_TO_ATTACK_FORCED              = "Trooper2Attack",
		GO_TO_ATTACK_GROUP_FORCED        = "Trooper2AttackGroup",
		GO_TO_HIDE_FORCED                = "Trooper2Hide",
		GO_TO_THREATENED_FORCED          = "Trooper2Threatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Trooper2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Trooper2Interested",
		GO_TO_SEEK_FORCED                = "Trooper2Seek",
		GO_TO_SEARCH_FORCED              = "Trooper2Search",
		GO_TO_AVOIDEXPLOSIVES_FORCED     = "Trooper2AvoidExplosives",
		GO_TO_AVOIDVEHICLE_FORCED        = "Trooper2AvoidVehicle",
		GO_TO_CHECKDEAD_FORCED           = "Trooper2CheckDead",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Trooper2CallReinforcements",
		OnAttackSwitchPosition_FORCED    = "Trooper2AttackSwitchPosition",
		GO_TO_GRABBED_FORCED             = "Trooper2GrabbedByScout",
		GO_TO_ON_ROCK_FORCED             = "Trooper2ShootOnRock",
		GO_TO_ON_WALL_FORCED             = "Trooper2ShootOnWall",
		CONTROL_TROOPER_FORCED           = "Trooper2AttackT",
		CONTROL_COVER_FORCED             = "Trooper2Attack",
		CHOOSE_TO_SEE_FORCEDK            = "Trooper2Seek",
		CHOOSE_TO_SEARCH_FORCED          = "Trooper2Search",
		CHOOSE_TO_ATTACK_FORCED          = "Trooper2Attack",
	},

	TOSSHARED = {

		-- Стандартные сигналы
		-- переопределяют те, что в AnyBehavior
		RESUME_FOLLOWING           = "",
		ENTERING_VEHICLE           = "",
		USE_MOUNTED_WEAPON         = "",
		OnPlayerSeen               = "",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnBulletRain               = "",
		OnGrenadeSeen              = "",
		OnInterestingSoundHeard    = "",
		OnThreateningSoundHeard    = "",
		entered_vehicle            = "",
		exited_vehicle             = "",
		exited_vehicle_investigate = "",
		OnSomethingSeen            = "",
		GO_TO_AVOIDEXPLOSIVES      = "",
		GO_TO_AVOIDVEHICLE         = "",
		GO_TO_CHECKDEAD            = "",
		GO_TO_IDLE                 = "",
		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_RUSH_ATTACK          = "",
		GO_TO_HIDE                 = "",
		GO_TO_AVOID_TANK           = "",
		GO_TO_RPG_ATTACK           = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_RELOAD               = "",
		GO_TO_CALL_REINFORCEMENTS  = "",
		GO_TO_PANIC                = "",
		GO_TO_STATIC               = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		OnFallAndPlayWakeUp        = "TrooperLeaderMKIIIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide

	-- HUMAN
	TrooperMKIIIdle =
	{
		OnPlayerSeen             = "",
		OnInterestingSoundHeard  = "",
		OnSomethingSeen          = "",
		OnThreateningSoundHeard  = "",
		OnBulletRain             = "",
		OnNearMiss               = "",
		OnEnemyDamage            = "",
		OnGroupMemberDiedNearest = "",
		OnSomebodyDied           = "",
		ENEMYSEEN_FIRST_CONTACT  = "",
		ENEMYSEEN_DURING_COMBAT  = "",
		OnExplosionDanger        = "HBaseGrenadeRun",
	},

	Trooper2AvoidExplosives =
	{
		GO_TO_IDLE = "TrooperLeaderMKIIIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		OnSomethingSeen = "",
		OnNoTarget = "",
	},

	Trooper2AvoidVehicle =
	{
		GO_TO_IDLE = "TrooperLeaderMKIIIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
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
		GO_TO_IDLE = "TrooperLeaderMKIIIdle",
		OnFallAndPlayWakeUp = "TrooperLeaderMKIIIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "",
		OnTankSeen = "",
		OnHeliSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		entered_vehicle = "",
		exited_vehicle = "",
		exited_vehicle_investigate = "",
		OnSomethingSeen = "",
		GO_TO_SEEK = "",
		GO_TO_SEARCH = "",
		GO_TO_ATTACK = "",
		GO_TO_AVOIDEXPLOSIVES = "",
		GO_TO_ALERT = "",
		GO_TO_CHECKDEAD = "",
	},

	Trooper2Dumb =
	{
		GO_TO_IDLE = "TrooperLeaderMKIIIdle",
		GO_TO_SEARCH = "Trooper2Search",
		DODGE = "",
		DODGE_GRENADE = "",
		RESUME_FOLLOWING = "",
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "",
		OnTankSeen = "",
		OnHeliSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		OnSomethingSeen = "",
		GO_TO_ATTACK = "",
		GO_TO_HIDE = "",
		GO_TO_THREATENED = "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED = "",
		GO_TO_SEEK = "",
		GO_TO_SEARCH = "",
		GO_TO_RELOAD = "",
		GO_TO_CALL_REINFORCEMENTS = "",
		ENEMYSEEN_FIRST_CONTACT = "",
		ENEMYSEEN_DURING_COMBAT = "",
	},

	Trooper2GrabbedByScout =
	{
		GO_TO_INTERESTED       = "",
		GO_TO_ATTACK           = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION  = "Trooper2AttackSwitchPosition",
		OnAttackShootSpot      = "",
		GO_TO_MOAR             = "",
		--		GO_TO_ON_SPOT					= "Trooper2ShootOnSpot",
		--		GO_TO_SPECIAL_ACTION			= "TrooperAttackSpecialAction",
		OnSpecialAction        = "",
		GO_TO_ON_ROCK          = "",
		GO_TO_ON_WALL          = "",
		ORDER_SEARCH           = "",
		ORDER_COVER_SEARCH     = "",
		GO_TO_SEARCH           = "",
		GO_TO_MELEE            = "",
		GO_TO_DODGE            = "",
		GO_TO_ATTACK_JUMP      = "",
		OnAttackChase          = "",
		OnExplosionDanger      = "",
		PURSUE                 = "",
		OnFallAndPlay          = "",
		GRABBED_TO_INTERESTED  = "Trooper2Interested",
		GRABBED_TO_ATTACK      = "Trooper2Attack",
		--ORDER_SEARCH = "",
	},

	Trooper2AttackSwitchPosition =
	{
		GO_TO_JUMP             = "",
		GO_TO_MELEE            = "Trooper2AttackSwitchPositionMelee",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION  = "Trooper2AttackSwitchPosition",
	},

	Trooper2AttackSwitchPositionMelee =
	{
		END_MELEE              = "Trooper2AttackSwitchPosition",
		MELEE_FAILED           = "Trooper2AttackSwitchPosition",
		OnAttackShootSpot      = "",
		OnAttackSwitchPosition = "",
		OnLand                 = "Trooper2AttackSwitchPosition",
		BackToSwitchPosition   = "Trooper2AttackSwitchPosition",
	},

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
