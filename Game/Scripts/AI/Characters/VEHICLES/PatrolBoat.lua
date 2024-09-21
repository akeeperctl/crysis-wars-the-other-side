--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2005.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for basic PatrolBoat (small, armed)
--
--------------------------------------------------------------------------
--  History:
--  -  25/07/2005  : Created by Kirill Bulatsev
--
--------------------------------------------------------------------------

AICharacter.PatrolBoat = {

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY  = "TOS_Obey",
		--~TheOtherSide
		STOP_VEHICLE    = "PatrolBoatIdle",
		GO_TO_IDLE      = "PatrolBoatIdle"
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

		TO_PATROLBOAT_ALERT        = "",
		TO_PATROLBOAT_ATTACK       = "",
		TO_PATROLBOAT_ATTACK2      = "",

		STOP_VEHICLE               = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.

		GO_TO_IDLE_FORCED          = "PatrolBoatIdle",
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

	PatrolBoatIdle = {
		STOP_VEHICLE          = "",
		EVERYONE_OUT          = "",
		DRIVER_OUT            = "",
		OnPlayerSeen          = "PatrolBoatAttack2",
		OnSomethingSeen       = "PatrolBoatAttack2",
		ACT_GOTO              = "PatrolBoatGoto",
		TO_PATROLBOAT_ALERT   = "",
		TO_PATROLBOAT_ATTACK  = "PatrolBoatAttack",
		TO_PATROLBOAT_ATTACK2 = "PatrolBoatAttack2",
		VEHICLE_GOTO_DONE     = "",
	},

	PatrolBoatGoto = {
		STOP_VEHICLE          = "PatrolBoatIdle",
		EVERYONE_OUT          = "PatrolBoatIdle",
		DRIVER_OUT            = "PatrolBoatIdle",
		OnPlayerSeen          = "PatrolBoatAttack2",
		OnSomethingSeen       = "PatrolBoatAttack2",
		ACT_GOTO              = "",
		FOLLOW                = "",
		TO_PATROLBOAT_ALERT   = "",
		TO_PATROLBOAT_ATTACK  = "PatrolBoatAttack",
		TO_PATROLBOAT_ATTACK2 = "PatrolBoatAttack2",
		VEHICLE_GOTO_DONE     = "PatrolBoatIdle",
	},

	PatrolBoatAttack = {
		STOP_VEHICLE          = "PatrolBoatIdle",
		EVERYONE_OUT          = "PatrolBoatIdle",
		DRIVER_OUT            = "PatrolBoatIdle",
		OnPlayerSeen          = "",
		OnSomethingSeen       = "",
		ACT_GOTO              = "",
		FOLLOW                = "",
		TO_PATROLBOAT_ALERT   = "PatrolBoatAlert",
		TO_PATROLBOAT_ATTACK  = "",
		TO_PATROLBOAT_ATTACK2 = "PatrolBoatAttack2",
		VEHICLE_GOTO_DONE     = "",
	},

	PatrolBoatAttack2 = {
		STOP_VEHICLE          = "PatrolBoatIdle",
		EVERYONE_OUT          = "PatrolBoatIdle",
		DRIVER_OUT            = "PatrolBoatIdle",
		OnPlayerSeen          = "",
		OnSomethingSeen       = "",
		ACT_GOTO              = "",
		FOLLOW                = "",
		TO_PATROLBOAT_ALERT   = "PatrolBoatAlert",
		TO_PATROLBOAT_ATTACK  = "PatrolBoatAttack",
		TO_PATROLBOAT_ATTACK2 = "",
		VEHICLE_GOTO_DONE     = "",
	},

	PatrolBoatAlert = {
		STOP_VEHICLE          = "PatrolBoatIdle",
		EVERYONE_OUT          = "PatrolBoatIdle",
		DRIVER_OUT            = "PatrolBoatIdle",
		OnPlayerSeen          = "",
		OnSomethingSeen       = "",
		ACT_GOTO              = "",
		FOLLOW                = "",
		TO_PATROLBOAT_ALERT   = "",
		TO_PATROLBOAT_ATTACK  = "PatrolBoatAttack",
		TO_PATROLBOAT_ATTACK2 = "PatrolBoatAttack2",
		VEHICLE_GOTO_DONE     = "",
	},

}
