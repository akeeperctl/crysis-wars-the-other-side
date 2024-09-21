--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2005.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for Helicopter
--
--------------------------------------------------------------------------
--  History:
--  - 31/10/2006   : Created by Tetsuji
--
--------------------------------------------------------------------------

AICharacter.HeliAggressive = {
	AnyBehavior =
	{
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		TO_VTOL_FLY_TOS = "VtolFlyTOS",
		--~TheOtherSide

		STOP_VEHICLE    = "HeliAggressiveIdle",
		GO_TO_IDLE      = "HeliAggressiveIdle",
	},

	--TheOtherSide
	TOS_Obey = {
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
		OnFallAndPlayWakeUp        = "",
		OnBackOffFailed            = "",

		TO_HELI_SMOOTHGOTO         = "",
		TO_HELI_REINFORCEMENT      = "",
		TO_HELI_LANDING            = "",
		TO_HELI_LANDING2           = "",
		TO_HELI_IDLE               = "",

		TO_HELI_HOVERATTACK        = "",
		TO_HELI_HOVERATTACK2       = "",
		TO_HELI_HOVERATTACK3       = "",
		TO_HELI_FLYOVER            = "",
		TO_HELI_PATROL             = "",
		TO_HELI_ATTACK             = "",
		TO_HELI_PICKATTACK         = "",
		TO_HELI_SHOOTAT            = "",
		TO_HELI_PRIVIOUS           = "",
		TO_HELI_VSBOAT             = "",
		TO_HELI_VSAIR              = "",
		TO_HELI_FLY                = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_IDLE_FORCED          = "HeliAggressiveIdle",
		STOP_VEHICLE               = "",
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
		OnBackOffFailed            = "",

		TO_HELI_EMERGENCYLANDING   = "",

		TO_HELI_SMOOTHGOTO         = "",
		TO_HELI_REINFORCEMENT      = "",
		TO_HELI_LANDING            = "",
		TO_HELI_LANDING2           = "",
		TO_HELI_IDLE               = "",

		TO_HELI_HOVERATTACK        = "",
		TO_HELI_HOVERATTACK2       = "",
		TO_HELI_HOVERATTACK3       = "",
		TO_HELI_FLYOVER            = "",
		TO_HELI_PATROL             = "",
		TO_HELI_ATTACK             = "",
		TO_HELI_PICKATTACK         = "",
		TO_HELI_SHOOTAT            = "",
		TO_HELI_PRIVIOUS           = "",
		TO_HELI_VSBOAT             = "",
		TO_HELI_VSAIR              = "",
		TO_HELI_FLY                = "",

		GO_TO_IDLE_FORCED          = "HeliAggressiveIdle",
		STOP_VEHICLE               = "",
	},
	--~TheOtherSide


	HeliAggressiveIdle = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "HeliSmoothGoto",
		TO_HELI_REINFORCEMENT    = "HeliReinforcement",
		TO_HELI_LANDING          = "HeliLanding",
		TO_HELI_LANDING2         = "HeliLanding",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "HeliShootAt",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "HeliFly",

		ACT_GOTO                 = "HeliGoto",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliIdle = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "HeliSmoothGoto",
		TO_HELI_REINFORCEMENT    = "HeliReinforcement",
		TO_HELI_LANDING          = "HeliLanding",
		TO_HELI_LANDING2         = "HeliLanding",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "HeliShootAt",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "HeliFly",

		ACT_GOTO                 = "HeliGoto",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "HeliAttack",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "HeliHoverAttack",
		TO_HELI_HOVERATTACK2     = "HeliHoverAttack2",
		TO_HELI_HOVERATTACK3     = "HeliHoverAttack3",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "HeliPickAttack",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "HelivsBoat",
		TO_HELI_VSAIR            = "HelivsAir",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliGoto = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "HeliReinforcement",
		TO_HELI_LANDING          = "HeliLanding",
		TO_HELI_LANDING2         = "HeliLanding",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "HeliShootAt",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "HeliAggressiveIdle",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "HeliAttack",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	---------------------------------------------------------------

	HeliFlyOver = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "HeliHoverAttack",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliHoverAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "HeliHoverAttack",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "HeliFlyOver",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "HeliPickAttack",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliHoverAttack2 = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliHoverAttack3 = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},


	HeliPickAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliPatrol = {
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "HeliReinforcement",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliReinforcement = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "HeliLanding",
		TO_HELI_LANDING2         = "HeliLanding",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliLanding = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "HeliReinforcement",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliShootAt = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "HeliAggressiveIdle",

		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliEmergencyLanding = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HelivsBoat = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "HeliHoverAttack",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HelivsAir = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "HeliHoverAttack",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliSmoothGoto = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "HeliEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "HeliPatrol",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "HeliAggressiveIdle",
		EVERYONE_OUT             = "HeliAggressiveIdle",
		DRIVER_OUT               = "HeliAggressiveIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "HeliIgnorant",
		MAKE_ME_UNIGNORANT       = "HeliUnIgnorant",

	},

	HeliIgnorant = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "",
		MAKE_ME_UNIGNORANT       = "",

	},

	HeliUnIgnorant = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "",
		MAKE_ME_UNIGNORANT       = "",

	},

	HeliFly = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "HeliAggressiveIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_HOVERATTACK2     = "",
		TO_HELI_HOVERATTACK3     = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "HeliAttack",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",
		TO_HELI_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "",
		MAKE_ME_UNIGNORANT       = "",

	},

	-- HeliPath is not supported any more 17/06/2006 Tetsuji
	HeliPath = {
		OnPlayerSeen = "",
	},


}
