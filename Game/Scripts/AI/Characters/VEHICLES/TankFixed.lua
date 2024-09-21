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
--  - 17/07/2006   : Dulplicated for the special tank by Tetsuji
--
--------------------------------------------------------------------------

AICharacter.TankFixed = {

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
		STOP_VEHICLE    = "TankFixedIdle",
		GO_TO_IDLE      = "TankFixedIdle"

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
		OnFallAndPlayWakeUp        = "TankFixedIdle",
		OnBackOffFailed            = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_IDLE_FORCED          = "TankFixedIdle",

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

	TankFixedIdle = {
		-----------------------------------
		FOLLOW              = "TankFixedFollow",
		ACT_GOTO            = "TankFixedGoto",

		EVERYONE_OUT        = "",
		STOP_VEHICLE        = "",
		DRIVER_OUT          = "",
		VEHICLE_GOTO_DONE   = "",

		TO_TANKCLOSE_ATTACK = "",

		OnPlayerSeen        = "",

	},

	TankFixedFollow = {
		-----------------------------------
		FOLLOW              = "",
		ACT_GOTO            = "TankFixedGoto",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE        = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "",

		OnPlayerSeen        = "",

	},

	TankFixedGoto = {
		-----------------------------------
		FOLLOW              = "TankFixedFollow",
		ACT_GOTO            = "",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE        = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "",

		OnPlayerSeen        = "",

	},

	TankFixedAttack = {
		-----------------------------------
		FOLLOW              = "",
		ACT_GOTO            = "",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE        = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "",

		OnPlayerSeen        = "",

	},


}
