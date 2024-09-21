--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2005.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for basic Boat (small, armed)
--
--------------------------------------------------------------------------
--  History:
--  -  25/07/2005  : Created by Kirill Bulatsev
--
--------------------------------------------------------------------------

AICharacter.Boat = {

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY  = "TOS_Obey",
		--~TheOtherSide
		STOP_VEHICLE    = "BoatIdle",
		GO_TO_IDLE      = "BoatIdle"
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
		GO_TO_IDLE_FORCED          = "BoatIdle",
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

		STOP_VEHICLE               = "",
	},
	--~TheOtherSide
	
	BoatIdle = {
		STOP_VEHICLE      = "",
		EVERYONE_OUT      = "",
		DRIVER_OUT        = "",
		OnPlayerSeen      = "",
		ACT_GOTO          = "BoatGoto",
		FOLLOW            = "",
		TO_BOAT_ALERT     = "",
		TO_BOAT_ATTACK    = "",
		VEHICLE_GOTO_DONE = "",
	},

	BoatGoto = {
		STOP_VEHICLE      = "BoatIdle",
		EVERYONE_OUT      = "BoatIdle",
		DRIVER_OUT        = "BoatIdle",
		OnPlayerSeen      = "",
		ACT_GOTO          = "",
		FOLLOW            = "",
		TO_BOAT_ALERT     = "",
		TO_BOAT_ATTACK    = "",
		VEHICLE_GOTO_DONE = "BoatIdle",
	},

	BoatAttack = {
		STOP_VEHICLE      = "BoatIdle",
		EVERYONE_OUT      = "BoatIdle",
		DRIVER_OUT        = "BoatIdle",
		OnPlayerSeen      = "",
		ACT_GOTO          = "",
		FOLLOW            = "",
		TO_BOAT_ALERT     = "",
		TO_BOAT_ATTACK    = "",
		VEHICLE_GOTO_DONE = "",
	},

	BoatAlert = {
		STOP_VEHICLE      = "BoatIdle",
		EVERYONE_OUT      = "BoatIdle",
		DRIVER_OUT        = "BoatIdle",
		OnPlayerSeen      = "",
		ACT_GOTO          = "",
		FOLLOW            = "",
		TO_BOAT_ALERT     = "",
		TO_BOAT_ATTACK    = "",
		VEHICLE_GOTO_DONE = "",
	},

}
