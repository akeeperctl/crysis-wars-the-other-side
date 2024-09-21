----------------------------------------------------------------------------------------------------
--  Crytek Source File.
--  Copyright (C), Crytek Studios, 2001-2004.
----------------------------------------------------------------------------------------------------
--  $Id$
--  $DateTime$
--  Description: SNIPER CHARACTER SCRIPT
--
----------------------------------------------------------------------------------------------------
--  History:
--  - 30:nov:2005   : Created by Kirill Bulatsev
--
----------------------------------------------------------------------------------------------------пїЅ



AICharacter.Sniper = {

	Class = UNIT_CLASS_INFANTRY,
	
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY = "TOS_Obey",
		--~TheOtherSide
		GO_TO_IDLE = "SniperIdle",
		RETURN_TO_FIRST = "FIRST",
		USE_MOUNTED_WEAPON = "UseMounted",
		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",
		OnExplosionDanger		= "HBaseGrenadeRun",
		GRENADE = "HBaseGrenadeRun",
--		GO_TO_ALERT = "SniperAlert",

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
		GO_TO_STATIC              		 = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "SniperHide",
		OnBackOffFailed 				 = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "CamperAttack",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "SniperHide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "Cover2Threatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "Cover2Reload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "SniperIdle",
		GO_TO_PANIC_FORCED               = "Cover2Panic",
		GO_TO_PREVIOUS_FORCED            = "PREVIOUS",
		GO_TO_STATIC_FORCED              = "HBaseStaticShooter",
	},

	TOSSHARED = {

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
		GO_TO_STATIC              		 = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "Cover2Idle",
		OnBackOffFailed 				 = "",
	},
	--~TheOtherSide
	
	SniperIdle = {
		on_spot = "SniperSnipe",
			
--		HEADS_UP_GUYS		= "SniperHide",	
--		OnPlayerSeen    	= "SniperSnipe",
--		OnInterestingSoundHeard = "",
--		OnSomethingSeen		= "",
--		GET_ALERTED	 = "SniperHide",
--		NORMAL_THREAT_SOUND 	= "SniperMove",
--		OnThreateningSoundHeard = "CoverThreatened",

--		OnBulletRain		= "SniperHide",
--		OnEnemyDamage	= "SniperHide",
--		OnDamage	= "CamperAlert",
--		OnGroupMemberDiedNearest 	= "",

--		OnSomebodyDied	= "SniperAlert",
--		OnGrenadeSeen		= "SniperAlert",
--		INCOMING_FIRE		= "SniperHide",
		-----------------------------------


		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
--		exited_vehicle = "PREVIOUS",
	},
	
	

	SniperAlert = {
		OnPlayerSeen    	= "SniperMove",
		OnInterstingSoundHeard  = "",
		OnThreateningSoundHeard  = "",

		HEADS_UP_GUYS		= "SniperMove",	

		OnEnemyDamage	= "SniperHide",
		OnDamage	= "SniperHide",
		SniperCloseContact = "CamperAttack",		

		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		
		-- pre combat
	},

	SniperSnipe= {
--		OnNoHidingPlace = "CoverHold",
		OnNoTarget		= "",
		relocate = "SniperMove",
		SniperCloseContact = "CamperAttack",
		
		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		--OnLowHideSpot	= "DigIn",
	},

	SniperMove= {
		on_spot = "SniperSnipe",
		SniperCloseContact = "CamperAttack",
		OnEnemyDamage = "CamperAttack",
	},

	SniperHide= {
		SniperCloseContact = "CamperAttack",

--		OnNoHidingPlace = "CoverHold",
--		OnNoTarget		= "CamperAlert",
		
		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		--OnLowHideSpot	= "DigIn",

	},

	CamperAttack= {
		OnTargetFleeing = "SniperMove",
	},

	UseMounted = {
		ORDER_HIDE = "",
		ORDER_FIRE = "",
		USE_MOUNTED_WEAPON = "",
		ACT_GOTO = "SniperIdle",
		ACT_FOLLOWPATH = "SniperIdle",
	},

	UseMountedIdle = {
		USE_MOUNTED_WEAPON = "",
		--OnPlayerSeen = "UseMounted",
		--OnEnemyDamage = "UseMounted",
		--OnBulletRain = "UseMounted",
		TO_USE_MOUNTED = "UseMounted",
		TOO_FAR_FROM_WEAPON = "SniperIdle",
		ACT_GOTO = "SniperIdle",
		ACT_FOLLOWPATH = "SniperIdle",
	},

	HBaseGrenadeRun = {
		GRENADE_RUN_OVER = "CoverAlert",
		RESUME_FOLLOWING	= "",
		ORDER_HIDE = "",
		ENTERING_VEHICLE = "",
		ORDER_USE_RPG = "",
		USE_MOUNTED_WEAPON = "",
		ORDER_FIRE = "",
		OnPlayerSeen    	= "",
		OnBulletRain		= "",
		OnGrenadeSeen		= "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		entered_vehicle	="",
		exited_vehicle	="",
		exited_vehicle_investigate = "",
		ORDER_HOLD	 = "",
		ORDER_FORM		="",
		ORDER_FOLLOW_HIDE		="",
		ORDER_MOVE	 = "",	
		ORDER_ENTER_VEHICLE = "",	
		ORDER_PLANT_BOMB = "",	
		OnSomethingSeen			="",
		OnExplosionDanger = "",
		
	},


	-- JOBS
	Job_WalkFollow = {
		ORDER_FOLLOW = "",
	},

	InVehicleTranquilized = {
		OnFallAndPlayWakeUp = 		"PREVIOUS",
		OnFallAndPlay			= "",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =						"",
		OnBulletRain =						"",
		OnGrenadeSeen =						"",
		OnInterestingSoundHeard =	"",
		OnThreateningSoundHeard =	"",
		entered_vehicle	=					"",
		exited_vehicle =					"",
		exited_vehicle_investigate = "",
		OnSomethingSeen =					"",
		GO_TO_SEEK =							"",
		GO_TO_IDLE = 							"",
		GO_TO_SEARCH =						"",
		GO_TO_ATTACK =						"",
		GO_TO_AVOIDEXPLOSIVES =   "",
		GO_TO_ALERT =							"",
		GO_TO_CHECKDEAD =					"",
	},


}
