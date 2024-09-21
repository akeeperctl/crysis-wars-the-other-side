-- SUIT SNIPER CHARACTER SCRIPT

AICharacter.SuitSniper = {

	Class = UNIT_CLASS_INFANTRY,
	TeamRole = GU_HUMAN_SNEAKER_SPECOP,

	Constructor = function(self, entity)
		AI.NotifyGroupTacticState(entity.id, 0, GN_INIT, 0);
	end,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED         = "TOSSHARED",
		GO_TO_TOS_OBEY          = "TOS_Obey",
		--~TheOtherSide
		GO_TO_IDLE              = "SuitSniperIdle",

		ENTERING_VEHICLE        = "EnteringVehicle",
		RETURN_TO_FIRST         = "FIRST",
		USE_MOUNTED_WEAPON      = "UseMounted",
		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",
		GO_TO_DUMB              = "Dumb",
		OnFallAndPlay           = "HBaseTranquilized",

		GO_TO_ATTACK            = "SuitSniperAttack",
		GO_TO_THREATENED        = "SuitSniperThreatened",
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
		GO_TO_STATIC                     = "",
		ENEMYSEEN_FIRST_CONTACT          = "",
		ENEMYSEEN_DURING_COMBAT          = "",
		OnFallAndPlayWakeUp              = "SuitSniperIdle",
		OnBackOffFailed                  = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "SuitSniperAttack",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "Cover2Hide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "SuitSniperThreatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "Cover2Reload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "SuitSniperIdle",
		GO_TO_PANIC_FORCED               = "Cover2Panic",
		GO_TO_PREVIOUS_FORCED            = "PREVIOUS",
		GO_TO_STATIC_FORCED              = "HBaseStaticShooter",
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
		OnFallAndPlayWakeUp        = "SuitSniperIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide

	HBaseAlerted = {

	},

	HBaseTranquilized = {
		GO_TO_IDLE = "SuitSniperIdle",
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
		GO_TO_AVOIDEXPLOSIVES = "",
	},

	UseMounted = {
		ORDER_HIDE = "",
		ORDER_FIRE = "",
		USE_MOUNTED_WEAPON = "",
		ACT_GOTO = "SuitSniperIdle",
		ACT_FOLLOWPATH = "SuitSniperIdle",
	},

	UseMountedIdle = {
		USE_MOUNTED_WEAPON = "",
		--OnPlayerSeen = "UseMounted",
		--OnEnemyDamage = "UseMounted",
		--OnBulletRain = "UseMounted",
		TO_USE_MOUNTED = "UseMounted",
		TOO_FAR_FROM_WEAPON = "SuitSniperIdle",
		ACT_GOTO = "SuitSniperIdle",
		ACT_FOLLOWPATH = "SuitSniperIdle",
	},

	SuitSniperIdle = {
		-----------------------------------
		-- Vehicles related
		entered_vehicle = "InVehicle",
		exited_vehicle  = "PREVIOUS",
	},

	SuitSniperThreatened = {
	},

	SuitSniperAttack = {
	},

	SuitSniperProtect = {
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

		OnFallAndPlay              = "InVehicleTranquilized",
	},

	Dumb = {
		GO_TO_IDLE = "SuitSniperIdle",
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
