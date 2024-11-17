--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2006.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for Vtol
--
--------------------------------------------------------------------------
--  History:
--  - 13/06/2005   : the first version by Tetsuji Iwasaki
--
--------------------------------------------------------------------------

AICharacter.Vtol = {

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		TO_VTOL_FLY_TOS = "VtolFlyTOS",
		--~TheOtherSide

		TO_HELI_IDLE = "VtolIdle",
		GO_TO_IDLE = "VtolIdle",
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
		GO_TO_IDLE_FORCED          = "VtolIdle",
		STOP_VEHICLE               = "",
	},

	-- TOS_Obey_Follow_And_Protect = TOS_Obey,

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

		GO_TO_IDLE_FORCED          = "VtolIdle",
		STOP_VEHICLE               = "",
	},
	--~TheOtherSide

	VtolIdle = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "VtolSmoothGoto",
		TO_HELI_REINFORCEMENT    = "VtolReinforcement",
		TO_HELI_LANDING          = "VtolLanding",
		TO_HELI_LANDING2         = "VtolLanding",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "VtolShootAt",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "VtolFly",

		ACT_GOTO                 = "VtolGoto",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolGoto = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "VtolReinforcement",
		TO_HELI_LANDING          = "VtolLanding",
		TO_HELI_LANDING2         = "VtolLanding",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "VtolShootAt",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "VtolFly",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "VtolIdle",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	---------------------------------------------------------------

	VtolFlyOver = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolHoverAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolPickAttack = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolPatrol = {
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "VtolReinforcement",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolReinforcement = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "VtolLanding",
		TO_HELI_LANDING2         = "VtolLanding",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolLanding = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "VtolReinforcement",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolShootAt = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "VtolIdle",

		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolEmergencyLanding = {
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",

		STOP_VEHICLE             = "",
		EVERYONE_OUT             = "",
		DRIVER_OUT               = "",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolvsBoat = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolvsAir = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PATROL           = "",
		TO_HELI_ATTACK           = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},

	VtolSmoothGoto = {
		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "VtolEmergencyLanding",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "VtolLanding",
		TO_HELI_LANDING2         = "VtolLanding",
		TO_HELI_IDLE             = "VtolIdle",

		TO_HELI_HOVERATTACK      = "",
		TO_HELI_FLYOVER          = "",
		TO_HELI_PICKATTACK       = "",
		TO_HELI_SHOOTAT          = "",
		TO_HELI_PRIVIOUS         = "",
		TO_HELI_VSBOAT           = "",
		TO_HELI_VSAIR            = "",

		TO_VTOL_FLY              = "",

		ACT_GOTO                 = "",
		FOLLOW                   = "",
		VEHICLE_GOTO_DONE        = "",
		STOP_VEHICLE             = "VtolIdle",
		EVERYONE_OUT             = "VtolIdle",
		DRIVER_OUT               = "VtolIdle",

		OnPlayerSeen             = "",

		MAKE_ME_IGNORANT         = "VtolIgnorant",
		MAKE_ME_UNIGNORANT       = "VtolUnIgnorant",

	},


	VtolIgnorant = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

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

		TO_VTOL_FLY              = "",

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

	VtolUnIgnorant = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

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

		TO_VTOL_FLY              = "",

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

	VtolFly = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

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

		TO_VTOL_FLY              = "",

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

	VtolFlyTOS = {

		-----------------------------------
		TO_HELI_EMERGENCYLANDING = "",

		TO_HELI_SMOOTHGOTO       = "",
		TO_HELI_REINFORCEMENT    = "",
		TO_HELI_LANDING          = "",
		TO_HELI_LANDING2         = "",
		TO_HELI_IDLE             = "VtolIdle",

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

		TO_VTOL_FLY              = "",

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
	VtolPath = {
		OnPlayerSeen = "",
	},


}
