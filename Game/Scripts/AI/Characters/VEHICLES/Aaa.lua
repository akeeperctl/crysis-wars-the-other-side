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
--
--------------------------------------------------------------------------

AICharacter.AAA = {

	Constructor = function(self, entity)
		--		entity.AI_DesiredFireDistance[1] = 30; -- main gun
		--		entity.AI_DesiredFireDistance[2] = 6; -- secondary machine gun
		entity.AI.weaponIdx = 1; --temp: select main gun by default
	end,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY  = "TOS_Obey",
		--~TheOtherSide
		STOP_VEHICLE    = "TankIdle",
		GO_TO_IDLE      = "TankIdle"
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
		OnFallAndPlayWakeUp        = "TankIdle",
		OnBackOffFailed            = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_IDLE_FORCED          = "TankIdle",

		TO_TANK_ALERT              = "",
		TO_TANK_ALERT2             = "",
		TO_TANK_ATTACK             = "",
		TO_TANK_MOVE               = "",
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

		TO_TANK_ALERT              = "",
		TO_TANK_ALERT2             = "",
		TO_TANK_ATTACK             = "",
		TO_TANK_MOVE               = "",
	},
	--~TheOtherSide

	AAAIdle = {
		-----------------------------------
		GO_PATH            = "Vehicle_Path",
		FOLLOW             = "TankFollow",
		ACT_GOTO           = "TankGoto",

		EVERYONE_OUT       = "",
		STOP_VEHICLE       = "",
		DRIVER_OUT         = "",
		VEHICLE_GOTO_DONE  = "",

		OnFollowPath       = "TankFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "TankAttack",

	},

	TankIdle = {
		-----------------------------------
		GO_PATH            = "Vehicle_Path",
		FOLLOW             = "TankFollow",
		ACT_GOTO           = "TankGoto",

		EVERYONE_OUT       = "",
		STOP_VEHICLE       = "",
		DRIVER_OUT         = "",
		VEHICLE_GOTO_DONE  = "",

		OnFollowPath       = "TankFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "TankAttack",

	},

	TankFollow = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "TankGoto",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "TankIdle",

		OnFollowPath       = "TankFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "TankAttack",

	},

	TankGoto = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "TankFollow",
		ACT_GOTO           = "",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "TankIdle",

		OnFollowPath       = "TankFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "TankAttack",

	},

	TankFollowPath = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "TankIdle",

		OnFollowPath       = "TankFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "TankAttack",

	},


	TankAttack = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "",

		OnFollowPath       = "",
		OnFollowPathLeader = "",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "",
		TO_TANK_MOVE       = "TankMove",

		OnPlayerSeen       = "",

	},

	TankMove = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "",

		OnFollowPath       = "",
		OnFollowPathLeader = "",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "TankAlert",
		TO_TANK_ATTACK     = "TankAttack",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "",

	},

	TankAlert = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",

		EVERYONE_OUT       = "TankIdle",
		STOP_VEHICLE       = "TankIdle",
		DRIVER_OUT         = "TankIdle",
		VEHICLE_GOTO_DONE  = "",

		OnFollowPath       = "",
		OnFollowPathLeader = "",

		TO_TANK_ALERT      = "",
		TO_TANK_ALERT2     = "",
		TO_TANK_ATTACK     = "TankAttack",
		TO_TANK_MOVE       = "",

		OnPlayerSeen       = "",

	},



}
