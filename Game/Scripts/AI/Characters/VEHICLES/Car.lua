--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2005.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for basic Car (4 wheeled vehicle)
-- this is non combat ground vehicle
--
--------------------------------------------------------------------------
--  History:
--  - 17/05/2005   : Created by Luciano Morpurgo
--
--------------------------------------------------------------------------

AICharacter.Car = {

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY     = "TOS_Obey",
		GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT     = "TOS_Obey_Follow_and_Protect",

		--~TheOtherSide
		STOP_VEHICLE    = "CarIdle",
		GO_TO_IDLE      = "CarIdle"
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

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		CarAlerted 				   = "",
		TO_CAR_SKID 			   = "",
		GO_TO_IDLE_FORCED          = "CarIdle",
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

		CarAlerted 				   = "",
		TO_CAR_SKID 			   = "",
		GO_TO_IDLE_FORCED          = "CarIdle",
		STOP_VEHICLE               = "",
	},
	--~TheOtherSide

	CarIdle = {
		-----------------------------------
		GO_PATH            = "Vehicle_Path",
		FOLLOW             = "Car_follow",
		ACT_GOTO           = "CarGoto",
		EVERYONE_OUT       = "",
		STOP_VEHICLE       = "",
		DRIVER_OUT         = "",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "VehicleFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",
		OnPlayerSeen       = "CarAlerted",
		TO_CAR_SKID        = "CarSkid",
	},

	CarAlerted = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "",
		TO_CAR_SKID        = "CarSkid",
		-----------------------------------
		OnNoTarget         = "CarIdle",
	},

	CarGoto = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "Car_follow",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "CarIdle",
		OnFollowPath       = "",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "CarAlerted",
		TO_CAR_SKID        = "CarSkid",
	},

	Car_path = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "",
		TO_CAR_SKID        = "CarSkid",
	},

	Car_follow = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "CarGoto",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "VehicleFollowPath",
		OnFollowPathLeader = "VehicleFollowPathLeader",
		OnPlayerSeen       = "",
		TO_CAR_SKID        = "CarSkid",
	},

	VehicleFollowPath = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "VehicleFollowPath",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "",
		TO_CAR_SKID        = "CarSkid",
	},

	VehicleFollowPathLeader = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "VehicleFollowPath",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "",
		TO_CAR_SKID        = "CarSkid",
	},

	CarSkid = {
		-----------------------------------
		GO_PATH            = "",
		FOLLOW             = "",
		ACT_GOTO           = "",
		EVERYONE_OUT       = "CarIdle",
		STOP_VEHICLE       = "CarIdle",
		DRIVER_OUT         = "CarIdle",
		VEHICLE_GOTO_DONE  = "",
		OnFollowPath       = "",
		OnFollowPathLeader = "",
		OnPlayerSeen       = "",

		TO_CAR_IDLE        = "CarIdle",
		TO_CAR_SKID        = "",
	},

}
