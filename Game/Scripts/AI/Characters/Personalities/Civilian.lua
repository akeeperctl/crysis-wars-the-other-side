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