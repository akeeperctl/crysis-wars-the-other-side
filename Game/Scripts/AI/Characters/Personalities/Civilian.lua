-- CIVILIAN CHARACTER
-- Civilian has a species, he's scared by enemy but 
-- he is unarmed and hides in presence of them
-- he can help his comrades to spot the enemy
-- History:
--	created by: Luciano Morpurgo

AICharacter.Civilian = {

	Class = UNIT_CLASS_CIVILIAN,
	
	AnyBehavior = {
		
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY     = "TOS_Obey",
		GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT     = "TOS_Obey_Follow_and_Protect",

		--~TheOtherSide
		
		GO_TO_IDLE					= "CivilianIdle",
		OnPlayerSeen    	= "CivilianAlert",
		OnInterestingSoundHeard = "",
		OnSomethingSeen		= "",
		OnThreateningSoundHeard = "CivilianAlert",
		OnDamage	= "",
		OnEnemyDamage	= "CivilianHide",
		OnFriendlyDamage	= "",

		OnGroupMemberDied	= "",
		OnExplosionDanger		= "HBaseGrenadeRun",
		INCOMING_FIRE		= "CivilianHide",
		GET_ALERTED = "",
		SURRENDER = "CivilianSurrender",
		GO_TO_HIDE = "CivilianHide",
		GO_TO_COVER= "CivilianCower",
		ENTERING_VEHICLE = "EnteringVehicle",
		entered_vehicle = "InVehicle",
		exited_vehicle 	= "PREVIOUS",

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
		GO_TO_STATIC              		 = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "CivilianIdle",
		OnBackOffFailed 				 = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "Cover2Attack",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "CivilianHide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "Cover2Threatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "Cover2Reload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "CivilianIdle",
		GO_TO_PANIC_FORCED               = "Cover2Panic",
		GO_TO_PREVIOUS_FORCED            = "PREVIOUS",
		GO_TO_STATIC_FORCED              = "HBaseStaticShooter",
	},

	-- TOS_Obey_Follow_And_Protect = TOS_Obey,

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


	CivilianIdle = {
		END_TIMEOUT = "CivilianHide",
--		OnBulletRain		= "CivilianCower",
--		OnEnemyDamage		= "CivilianCower",
--		OnNearMiss		= "CivilianCower",
		
	},

	CivilianAlert = {
		OnPlayerSeen    	= "",
		OnBulletRain		= "",
		OnEnemyDamage		= "",
	},

	CivilianSurrender = {
		OnPlayerSeen    	= "",
		SURRENDER = "",
		SET_FREE = "CivilianHide",
	},


	HBaseGrenadeRun = {
		OnPlayerSeen    	= "",
	},
	
	CivilianHide = {	
		OnPlayerSeen    	= "",
		OnNoHidingPlace    	= "CivilianCower",
	},
	
	CivilianCower = {	
		OnPlayerSeen    	= "",
	},

	EnteringVehicle = {
		exited_vehicle = "FIRST",
		do_exit_vehicle= "FIRST",
		GO_TO_ATTACK				= "",
		GO_TO_HIDE					= "",
		GO_TO_THREATENED		= "",
		GO_TO_INTERESTED		= "",
		GO_TO_SEEK					= "",
		GO_TO_SEARCH				= "",
		entered_vehicle = "InVehicle",
		entered_vehicle_gunner = "InVehicle",
	},
	
	InVehicle = {

		exited_vehicle_investigate = "FIRST",
		exited_vehicle = "FIRST",
		do_exit_vehicle= "FIRST",

		OnPlayerSeen =						"",
		OnInterestingSoundHeard =	"",
		OnSomethingSeen =					"",
		GET_ALERTED =							"",
		OnBulletRain =						"",
		OnEnemyDamage	=						"",
		OnDamage =								"",
		OnGroupMemberDied	=				"",
		INCOMING_FIRE =						"",
		HEADS_UP_GUYS =						"",
		GO_TO_HIDE =							"",

		GO_TO_ATTACK				= "",
		GO_TO_HIDE					= "",
		GO_TO_THREATENED		= "",
		GO_TO_INTERESTED		= "",
		GO_TO_SEEK					= "",
		GO_TO_SEARCH				= "",

	},
	
	InVehicleGunner = {

		exited_vehicle_investigate = "FIRST",
		exited_vehicle = "FIRST",
		do_exit_vehicle= "FIRST",

		OnPlayerSeen =						"",
		OnInterestingSoundHeard =	"",
		OnSomethingSeen =					"",
		GET_ALERTED =							"",
		OnBulletRain =						"",
		OnEnemyDamage	=						"",
		OnDamage =								"",
		OnGroupMemberDied	=				"",
		INCOMING_FIRE =						"",
		HEADS_UP_GUYS =						"",
		GO_TO_HIDE =							"",

		GO_TO_ATTACK				= "",
		GO_TO_HIDE					= "",
		GO_TO_THREATENED		= "",
		GO_TO_INTERESTED		= "",
		GO_TO_SEEK					= "",
		GO_TO_SEARCH				= "",

		controll_vehicleGunner = "",

	},

	
}