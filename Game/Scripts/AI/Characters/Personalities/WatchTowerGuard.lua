-- COVER CHARACTER SCRIPT

AICharacter.WatchTowerGuard = {

	Class = UNIT_CLASS_INFANTRY,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED         = "TOSSHARED",
		GO_TO_TOS_OBEY     = "TOS_Obey",
		GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT     = "TOS_Obey_Follow_and_Protect",

		--~TheOtherSide
		GO_TO_IDLE              = "WatchTowerGuardIdle",
		ENTERING_VEHICLE        = "EnteringVehicle",
		RETURN_TO_FIRST         = "FIRST",
		USE_MOUNTED_WEAPON      = "UseMounted",
		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",
		GO_TO_DUMB              = "Dumb",
		GO_TO_AVOIDEXPLOSIVES   = "Cover2AvoidExplosives",
		GO_TO_CHECKDEAD         = "CheckDead",
		OnFallAndPlay           = "HBaseTranquilized",

		GO_TO_WATCH_TOWER_IDLE     = "WatchTowerGuardIdle",
		GO_TO_WATCH_TOWER_ALERTED  = "WatchTowerGuardAlerted",
		GO_TO_WATCH_TOWER_COMBAT   = "WatchTowerGuardCombat",


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
		GO_TO_STATIC                     = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "WatchTowerGuardIdle",
		OnBackOffFailed                  = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "WatchTowerGuardCombat",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "Cover2Hide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "WatchTowerGuardAlerted",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "Cover2Reload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "WatchTowerGuardIdle",
		GO_TO_PANIC_FORCED               = "Cover2Panic",
		GO_TO_PREVIOUS_FORCED            = "PREVIOUS",
		GO_TO_STATIC_FORCED              = "HBaseStaticShooter",
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
		OnFallAndPlayWakeUp        = "WatchTowerGuardIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide

	HBaseTranquilized = {
		GO_TO_IDLE = "WatchTowerGuardIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "",
		OnTankSeen = "",
		OnHeliSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		entered_vehicle = "",
		exited_vehicle = "",
		exited_vehicle_investigate = "",
		OnSomethingSeen = "",
		GO_TO_SEEK = "",
		GO_TO_SEARCH = "",
		GO_TO_ATTACK = "",
		GO_TO_AVOIDEXPLOSIVES = "",
		GO_TO_ALERT = "",
		GO_TO_CHECKDEAD = "",
	},

	UseMounted = {
		ORDER_HIDE = "",
		ORDER_FIRE = "",
		USE_MOUNTED_WEAPON = "",
		ACT_GOTO = "WatchTowerGuardIdle",
		ACT_FOLLOWPATH = "WatchTowerGuardIdle",
	},

	UseMountedIdle = {
		USE_MOUNTED_WEAPON = "",
		--OnPlayerSeen = "UseMounted",
		--OnEnemyDamage = "UseMounted",
		--OnBulletRain = "UseMounted",
		TO_USE_MOUNTED = "UseMounted",
		TOO_FAR_FROM_WEAPON = "WatchTowerGuardIdle",
		ACT_GOTO = "WatchTowerGuardIdle",
		ACT_FOLLOWPATH = "WatchTowerGuardIdle",
	},


	Cover2AvoidExplosives = {
		GO_TO_IDLE = "WatchTowerGuardIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		OnSomethingSeen = "",
		OnNoTarget = "",
	},

	WatchTowerGuardIdle = {
		OnPlayerSeen             = "",
		OnTankSeen               = "",
		OnHeliSeen               = "",
		OnInterestingSoundHeard  = "",
		OnSomethingSeen          = "",
		ENEMYSEEN_FIRST_CONTACT  = "",
		ENEMYSEEN_DURING_COMBAT  = "",
		OnBulletRain             = "",
		OnEnemyDamage            = "",
		OnDamage                 = "",
		OnGroupMemberDied        = "",
		INCOMING_FIRE            = "",
		GO_TO_HIDE               = "",
		GO_TO_ATTACK             = "",
		GO_TO_ATTACK_GROUP       = "",
		GO_TO_HIDE               = "",
		GO_TO_THREATENED         = "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED         = "",
		GO_TO_SEEK               = "",
		GO_TO_SEARCH             = "",
		GO_TO_AVOID_TANK         = "",
	},

	WatchTowerGuardAlerted = {
		OnPlayerSeen             = "",
		OnTankSeen               = "",
		OnHeliSeen               = "",
		OnInterestingSoundHeard  = "",
		OnSomethingSeen          = "",
		OnBulletRain             = "",
		OnEnemyDamage            = "",
		OnDamage                 = "",
		OnGroupMemberDied        = "",
		INCOMING_FIRE            = "",
		ENEMYSEEN_FIRST_CONTACT  = "",
		ENEMYSEEN_DURING_COMBAT  = "",
		GO_TO_HIDE               = "",
		GO_TO_ATTACK             = "",
		GO_TO_ATTACK_GROUP       = "",
		GO_TO_HIDE               = "",
		GO_TO_THREATENED         = "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED         = "",
		GO_TO_SEEK               = "",
		GO_TO_SEARCH             = "",
		GO_TO_AVOID_TANK         = "",
	},

	WatchTowerGuardCombat = {
		OnPlayerSeen             = "",
		OnTankSeen               = "",
		OnHeliSeen               = "",
		OnInterestingSoundHeard  = "",
		OnSomethingSeen          = "",
		OnBulletRain             = "",
		OnEnemyDamage            = "",
		OnDamage                 = "",
		OnGroupMemberDied        = "",
		INCOMING_FIRE            = "",
		ENEMYSEEN_FIRST_CONTACT  = "",
		ENEMYSEEN_DURING_COMBAT  = "",
		GO_TO_HIDE               = "",
		GO_TO_ATTACK             = "",
		GO_TO_ATTACK_GROUP       = "",
		GO_TO_HIDE               = "",
		GO_TO_THREATENED         = "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED         = "",
		GO_TO_SEEK               = "",
		GO_TO_SEARCH             = "",
		GO_TO_AVOID_TANK         = "",
	},

	-- Vehicles related signals
	-- there are some cases that you have to mask signals when you add in AnyBehavior.
	-- these charactors also should be supported for cover2/Sneaker/Camper 10/07/2006 tetsuji

	EnteringVehicle = {
		exited_vehicle           = "FIRST",
		do_exit_vehicle          = "FIRST",
		GO_TO_ATTACK             = "",
		GO_TO_ATTACK_GROUP       = "",
		GO_TO_HIDE               = "",
		GO_TO_THREATENED         = "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED         = "",
		GO_TO_SEEK               = "",
		GO_TO_SEARCH             = "",
		entered_vehicle          = "InVehicle",
		entered_vehicle_gunner   = "InVehicleGunner",
		OnFallAndPlay            = "InVehicleTranquilized",
	},


	InVehicle = {

		exited_vehicle_investigate = "Job_Investigate",
		exited_vehicle             = "FIRST",
		do_exit_vehicle            = "FIRST",

		OnPlayerSeen               = "InVehicleAlerted",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnInterestingSoundHeard    = "",
		OnSomethingSeen            = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		GO_TO_HIDE                 = "",

		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_HIDE                 = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_AVOID_TANK           = "",

		controll_vehicle           = "InVehicleControlled",

		OnFallAndPlay              = "InVehicleTranquilized",
	},

	InVehicleAlerted = {
		exited_vehicle_investigate = "Job_Investigate",
		exited_vehicle             = "FIRST",
		do_exit_vehicle            = "FIRST",

		OnPlayerSeen               = "",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnInterestingSoundHeard    = "",
		OnSomethingSeen            = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		GO_TO_HIDE                 = "",

		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_HIDE                 = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_AVOID_TANK           = "",

		controll_vehicle           = "InVehicleControlled",
		OnFallAndPlay              = "InVehicleTranquilized",
	},

	InVehicleGunner = {

		exited_vehicle_investigate = "Job_Investigate",
		exited_vehicle             = "FIRST",
		do_exit_vehicle            = "FIRST",

		OnPlayerSeen               = "",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnInterestingSoundHeard    = "",
		OnSomethingSeen            = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		GO_TO_HIDE                 = "",

		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_HIDE                 = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_AVOID_TANK           = "",

		controll_vehicleGunner     = "InVehicleControlledGunner",

		OnFallAndPlay              = "InVehicleTranquilized",
	},

	InVehicleControlledGunner = {
		exited_vehicle_investigate = "Job_Investigate",
		exited_vehicle             = "FIRST",
		do_exit_vehicle            = "FIRST",

		OnPlayerSeen               = "",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnInterestingSoundHeard    = "",
		OnSomethingSeen            = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		GO_TO_HIDE                 = "",

		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_HIDE                 = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_AVOID_TANK           = "",

		OnFallAndPlay              = "InVehicleTranquilized",
	},

	InVehicleControlled = {
		exited_vehicle_investigate = "Job_Investigate",
		exited_vehicle             = "FIRST",
		do_exit_vehicle            = "FIRST",

		OnPlayerSeen               = "",
		OnTankSeen                 = "",
		OnHeliSeen                 = "",
		OnInterestingSoundHeard    = "",
		OnSomethingSeen            = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		ENEMYSEEN_FIRST_CONTACT    = "",
		ENEMYSEEN_DURING_COMBAT    = "",
		GO_TO_HIDE                 = "",

		GO_TO_ATTACK               = "",
		GO_TO_ATTACK_GROUP         = "",
		GO_TO_HIDE                 = "",
		GO_TO_THREATENED           = "",
		GO_TO_THREATENED_STANDBY   = "",
		GO_TO_INTERESTED           = "",
		GO_TO_SEEK                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_AVOID_TANK           = "",

		OnFallAndPlay              = "InVehicleTranquilized",
	},

	Dumb = {
		GO_TO_IDLE = "WatchTowerGuardIdle",
		RESUME_FOLLOWING = "",
		ENTERING_VEHICLE = "",
		USE_MOUNTED_WEAPON = "",
		ENEMYSEEN_FIRST_CONTACT = "",
		ENEMYSEEN_DURING_COMBAT = "",
		OnPlayerSeen = "",
		OnTankSeen = "",
		OnHeliSeen = "",
		OnBulletRain = "",
		OnGrenadeSeen = "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		entered_vehicle = "",
		exited_vehicle = "",
		exited_vehicle_investigate = "",
		OnSomethingSeen = "",
	},


	InVehicleTranquilized = {
		OnFallAndPlayWakeUp        = "PREVIOUS",
		OnFallAndPlay              = "",
		RESUME_FOLLOWING           = "",
		ENTERING_VEHICLE           = "",
		USE_MOUNTED_WEAPON         = "",
		OnPlayerSeen               = "",
		OnBulletRain               = "",
		OnGrenadeSeen              = "",
		OnInterestingSoundHeard    = "",
		OnThreateningSoundHeard    = "",
		entered_vehicle            = "",
		exited_vehicle             = "",
		exited_vehicle_investigate = "",
		OnSomethingSeen            = "",
		GO_TO_SEEK                 = "",
		GO_TO_IDLE                 = "",
		GO_TO_SEARCH               = "",
		GO_TO_ATTACK               = "",
		GO_TO_AVOIDEXPLOSIVES      = "",
		GO_TO_ALERT                = "",
		GO_TO_CHECKDEAD            = "",
	},

}
