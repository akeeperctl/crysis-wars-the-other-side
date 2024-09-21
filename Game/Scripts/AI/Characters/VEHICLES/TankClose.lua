--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for Tank
--
--------------------------------------------------------------------------
--  History:
--  - 06/02/2005   : Created by Kirill Bulatsev
--  - 10/07/2006   : Dulplicated for the special tank by Tetsuji
--
--------------------------------------------------------------------------

AICharacter.TankClose = {

	Constructor = function(self, entity)
		--		entity.AI.DesiredFireDistance[1] = 30; -- main gun
		--		entity.AI.DesiredFireDistance[2] = 6; -- secondary machine gun
		entity.AI.weaponIdx = 1; --temp: select main gun by default
	end,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY  = "TOS_Obey",
		--~TheOtherSide
		STOP_VEHICLE    = "TankCloseIdle",
		GO_TO_IDLE      = "TankCloseIdle"

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
		OnFallAndPlayWakeUp        = "TankCloseIdle",
		OnBackOffFailed            = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_IDLE_FORCED          = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "",
		TO_TANKCLOSE_RUNAWAY    = "",
		TO_TANKCLOSE_IDLE       = "",
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

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "",
		TO_TANKCLOSE_RUNAWAY    = "",
		TO_TANKCLOSE_IDLE       = "",
	},
	--~TheOtherSide

	TankCloseIdle = {
		-----------------------------------
		FOLLOW                  = "TankCloseFollow",
		ACT_GOTO                = "TankCloseGoto",

		EVERYONE_OUT            = "",
		STOP_VEHICLE            = "",
		DRIVER_OUT              = "",
		VEHICLE_GOTO_DONE       = "",

		TO_TANKCLOSE_ATTACK     = "TankCloseAttack",
		TO_TANKCLOSE_GOTOPATH   = "TankCloseGotoPath",
		TO_TANKCLOSE_SWITCHPATH = "TankCloseSwitchPath",
		TO_TANKCLOSE_RUNAWAY    = "TankCloseRunAway",
		TO_TANKCLOSE_IDLE       = "",

		OnPlayerSeen            = "",

	},

	TankCloseFollow = {
		-----------------------------------
		FOLLOW                  = "",
		ACT_GOTO                = "TankCloseGoto",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "",
		TO_TANKCLOSE_RUNAWAY    = "",
		TO_TANKCLOSE_IDLE       = "",

		OnPlayerSeen            = "TankCloseAttack",

	},

	TankCloseGoto = {
		-----------------------------------
		FOLLOW                  = "TankCloseFollow",
		ACT_GOTO                = "",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "TankCloseSwitchPath",
		TO_TANKCLOSE_RUNAWAY    = "TankCloseRunAway",
		TO_TANKCLOSE_IDLE       = "",

		OnPlayerSeen            = "TankCloseAttack",

	},

	TankCloseAttack = {
		-----------------------------------
		FOLLOW                  = "",
		ACT_GOTO                = "",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "TankCloseSwitchPath",
		TO_TANKCLOSE_RUNAWAY    = "TankCloseRunAway",
		TO_TANKCLOSE_IDLE       = "",

		OnPlayerSeen            = "",

	},

	TankCloseGotoPath = {
		-----------------------------------
		FOLLOW                  = "",
		ACT_GOTO                = "",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "TankCloseSwitchPath",
		TO_TANKCLOSE_RUNAWAY    = "TankCloseRunAway",
		TO_TANKCLOSE_IDLE       = "TankCloseIdle",

		OnPlayerSeen            = "TankCloseAttack",

	},

	TankCloseSwitchPath = {
		-----------------------------------
		FOLLOW                  = "",
		ACT_GOTO                = "",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "TankCloseAttack",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "",
		TO_TANKCLOSE_RUNAWAY    = "TankCloseRunAway",
		TO_TANKCLOSE_IDLE       = "TankCloseIdle",

		OnPlayerSeen            = "",

	},

	TankCloseRunAway = {
		-----------------------------------
		FOLLOW                  = "",
		ACT_GOTO                = "",

		EVERYONE_OUT            = "TankCloseIdle",
		STOP_VEHICLE            = "TankCloseIdle",
		DRIVER_OUT              = "TankCloseIdle",
		VEHICLE_GOTO_DONE       = "TankCloseIdle",

		TO_TANKCLOSE_ATTACK     = "",
		TO_TANKCLOSE_GOTOPATH   = "",
		TO_TANKCLOSE_SWITCHPATH = "",
		TO_TANKCLOSE_RUNAWAY    = "",
		TO_TANKCLOSE_IDLE       = "TankCloseIdle",

		OnPlayerSeen            = "",

	},


}
