-- COVER CHARACTER SCRIPT

AICharacter.SuitBoss = {

	Class = UNIT_CLASS_INFANTRY,

	Constructor = function(self, entity)
		--		AI.Signal(SIGNALFILTER_SENDER,1,"ConstructorCover2",entity.id);
	end,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED          = "TOSSHARED",
		GO_TO_TOS_OBEY           = "TOS_Obey",
		--~TheOtherSide
		GO_TO_IDLE               = "SuitIdle",
		ENTERING_VEHICLE         = "EnteringVehicle",
		RETURN_TO_FIRST          = "FIRST",
		USE_MOUNTED_WEAPON       = "UseMounted",
		USE_MOUNTED_WEAPON_INIT  = "UseMountedIdle",
		GO_TO_DUMB               = "Dumb",
		GO_TO_AVOIDEXPLOSIVES    = "Cover2AvoidExplosives",
		GO_TO_CHECKDEAD          = "CheckDead",
		TRANQUILIZED             = "HBaseTranquilized",

		GO_TO_ATTACK             = "SuitBossP1",
		GO_TO_PHASE_1B              = "SuitBossP1b",
		GO_TO_PHASE_2               = "SuitBossP2",
		GO_TO_PHASE_3               = "SuitBossP3",

		GO_TO_ATTACK_GROUP       = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK        = "Cover2RushAttack",
		GO_TO_HIDE               = "SuitHide",
		GO_TO_AVOID_TANK         = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK         = "Cover2RPGAttack",
		GO_TO_THREATENED         = "Cover2Threatened",
		GO_TO_THREATENED_STANDBY = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED         = "Cover2Interested",
		GO_TO_SEEK               = "Cover2Seek",
		GO_TO_SEARCH             = "Cover2Search",
		GO_TO_RELOAD             = "SuitReload",
		TO_STEALTH               = "", --"SuitStealth",
		HIDE_DONE                = "SuitBossP1",

		GO_TO_STATIC             = "HBaseStaticShooter"
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
		OnFallAndPlayWakeUp              = "HBaseAlerted",
		OnBackOffFailed                  = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "SuitAttack",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "SuitHide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "SuitThreatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "SuitReload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "SuitIdle",
		GO_TO_PANIC_FORCED               = "Cover2Panic",
		GO_TO_PREVIOUS_FORCED            = "PREVIOUS",
		GO_TO_STATIC_FORCED              = "HBaseStaticShooter",

		GO_TO_PROTECT_FORCED             = "SuitProtect",
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
		OnFallAndPlayWakeUp        = "SuitIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide

	HBaseAlerted = {

	},

	HBaseTranquilized = {
		GO_TO_IDLE = "SuitIdle",
		FALL_AND_PLAY_WAKEUP = "HBaseAlerted",
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
		ACT_GOTO = "SuitIdle",
		ACT_FOLLOWPATH = "SuitIdle",
	},

	UseMountedIdle = {
		USE_MOUNTED_WEAPON = "",
		--OnPlayerSeen = "UseMounted",
		--OnEnemyDamage = "UseMounted",
		--OnBulletRain = "UseMounted",
		TO_USE_MOUNTED = "UseMounted",
		TOO_FAR_FROM_WEAPON = "SuitIdle",
		ACT_GOTO = "SuitIdle",
		ACT_FOLLOWPATH = "SuitIdle",
	},


	Cover2AvoidExplosives = {
		GO_TO_IDLE = "SuitIdle",
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

	SuitIdle = {

		OnPlayerSeen             = "",
		OnInterestingSoundHeard  = "",
		OnSomethingSeen          = "",
		OnThreateningSoundHeard  = "",
		OnBulletRain             = "",
		OnNearMiss               = "",
		OnEnemyDamage            = "",
		OnGroupMemberDiedNearest = "",
		OnSomebodyDied           = "",
		GET_ALERTED              = "",
		HEADS_UP_GUYS            = "",
		OnExplosionDanger        = "HBaseGrenadeRun",
		-----------------------------------
		-- Vehicles related
		entered_vehicle          = "InVehicle",
		exited_vehicle           = "PREVIOUS",
	},

	Cover2Interested = {
	},

	Cover2Threatened = {
	},

	SuitHide = {
	},

	Cover2AvoidTank = {
	},

	Cover2RPGAttack = {
	},

	Cover2Seek = {
	},

	Cover2Search = {
	},

	SuitBossP1 = {
		GO_TO_RELOAD = "",
	},

	SuitBossP1b = {
		GO_TO_RELOAD = "",
	},

	SuitReload = {
	},

	SuitStealth = {
		OnEnemyDamage = "SuitBossP1",
	},

	Cover2AttackGroup = {
	},

	CheckDead = {
		OnPlayerSeen = "SuitBossP1",
		OnThreateningSoundHeard = "Cover2Threatened",
		OnInterestingSoundHeard = "",
		OnSomethingSeen = "",
		OnBulletRain = "SuitHide",
		GET_ALERTED = "SuitBossP1",
		SEEK_KILLER = "Cover2Threatened",
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
		TRANQUILIZED             = "InVehicleTranquilized",
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
		GET_ALERTED                = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		HEADS_UP_GUYS              = "",
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

		TRANQUILIZED               = "InVehicleTranquilized",
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
		GET_ALERTED                = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		HEADS_UP_GUYS              = "",
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
		TRANQUILIZED               = "InVehicleTranquilized",
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
		GET_ALERTED                = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		HEADS_UP_GUYS              = "",
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

		TRANQUILIZED               = "InVehicleTranquilized",
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
		GET_ALERTED                = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		HEADS_UP_GUYS              = "",
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

		TRANQUILIZED               = "InVehicleTranquilized",
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
		GET_ALERTED                = "",
		OnBulletRain               = "",
		OnEnemyDamage              = "",
		OnDamage                   = "",
		OnGroupMemberDied          = "",
		INCOMING_FIRE              = "",
		HEADS_UP_GUYS              = "",
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

		TRANQUILIZED               = "InVehicleTranquilized",
	},

	Dumb = {
		GO_TO_IDLE = "SuitIdle",
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
	},
}
