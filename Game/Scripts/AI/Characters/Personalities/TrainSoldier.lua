

AICharacter.TrainSoldier = {
	
	Constructor = function(self,entity)
		entity.actor:SetDampingForTrainSoldier(); --set damping on the train to avoid ragdoll sliding
	end, 
	
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		--~TheOtherSide
		ENTERING_VEHICLE =				"",
		RETURN_TO_FIRST =					"FIRST",
		USE_MOUNTED_WEAPON = 			"UseMounted",
		USE_MOUNTED_WEAPON_INIT = "UseMountedIdle",
		GO_TO_DUMB =							"Dumb",
		OnFallAndPlay =						"HBaseTranquilized",

		GO_TO_ATTACK					= "TrainSoldierAttack",
		GO_TO_IDLE						= "TrainSoldierIdle",
		GO_TO_PREVIOUS				= "PREVIOUS",
	},
	
	HBaseTranquilized = {
		GO_TO_IDLE =							"TrainSoldierIdle",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =						"",
		OnTankSeen =							"",
		OnHeliSeen =							"",
		OnBulletRain =						"",
		OnGrenadeSeen =						"",
		OnInterestingSoundHeard =	"",
		OnThreateningSoundHeard =	"",
		entered_vehicle	=					"",
		exited_vehicle =					"",
		exited_vehicle_investigate = "",
		OnSomethingSeen =					"",
		GO_TO_SEEK =							"",
		GO_TO_SEARCH =						"",
		GO_TO_ATTACK =						"",
		GO_TO_AVOIDEXPLOSIVES =   "",
		GO_TO_ALERT =							"",
		GO_TO_CHECKDEAD =					"",
	},
	
	UseMounted = {
		ORDER_HIDE = "",
		ORDER_FIRE = "",
		USE_MOUNTED_WEAPON = "",
		ACT_GOTO = "TrainSoldierIdle",
		ACT_FOLLOWPATH = "TrainSoldierIdle",
		LeaveMG = "TrainSoldierAttack",
	},

	UseMountedIdle = {
		USE_MOUNTED_WEAPON = "",
		OnPlayerSeen = "UseMounted",
		OnEnemyDamage = "UseMounted",
		OnBulletRain = "UseMounted",
		TOO_FAR_FROM_WEAPON = "TrainSoldierIdle",
		ACT_GOTO = "TrainSoldierIdle",
		ACT_FOLLOWPATH = "TrainSoldierIdle",
	},
	
	TrainSoldierIdle = {
		OnPlayerSeen    = "",
		OnInterestingSoundHeard = "",
		OnSomethingSeen	= "",
		OnThreateningSoundHeard = "",
		OnBulletRain		= "",
		OnNearMiss			= "",
		OnEnemyDamage		= "",
		OnGroupMemberDiedNearest 	= "",
		OnSomebodyDied	= "",
		ENEMYSEEN_FIRST_CONTACT	 		= "",
		ENEMYSEEN_DURING_COMBAT		= "",
	},

	TrainSoldierAttack = {
	},

	Dumb = {
		GO_TO_IDLE =							"TrainSoldierIdle",
		RESUME_FOLLOWING =				"",
		ENTERING_VEHICLE =				"",
		USE_MOUNTED_WEAPON =			"",
		OnPlayerSeen =						"",
		OnTankSeen =							"",
		OnHeliSeen =							"",
		OnBulletRain =						"",
		OnGrenadeSeen =						"",
		OnInterestingSoundHeard =	"",
		OnThreateningSoundHeard =	"",
		entered_vehicle	=					"",
		exited_vehicle =					"",
		exited_vehicle_investigate = "",
		OnSomethingSeen =					"",
		GO_TO_ATTACK				= "",
		GO_TO_ATTACK_GROUP	= "",
		GO_TO_RUSH_ATTACK	= "",
		GO_TO_HIDE					= "",
		GO_TO_AVOID_TANK		= "",
		GO_TO_RPG_ATTACK		= "",
		GO_TO_THREATENED		= "",
		GO_TO_THREATENED_STANDBY = "",
		GO_TO_INTERESTED		= "",
		GO_TO_SEEK					= "",
		GO_TO_SEARCH				= "",
		GO_TO_RELOAD				= "",
		GO_TO_CALL_REINFORCEMENTS	= "",
		ENEMYSEEN_FIRST_CONTACT	 		= "",
		ENEMYSEEN_DURING_COMBAT		= "",
		GO_TO_PANIC				= "",
	},
}
