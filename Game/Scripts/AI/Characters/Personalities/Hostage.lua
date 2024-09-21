-- FP HOSTAGE CHARACTER

AICharacter.Hostage = {

	Class = UNIT_CLASS_CIVILIAN,

	Constructor = function(self,entity)
		AIBehaviour.HostageIdle:Constructor(entity);
	end,
		
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY = "TOS_Obey",
		--~TheOtherSide
		RESUME_FOLLOWING	= "HostageFollow",
		ORDER_HIDE = "HostageHideSquad",
		ORDER_HIDE_FIRE = "HostageHideSquad",
		ENTERING_VEHICLE = "EnteringVehicle",
		ORDER_FIRE = "",
		ORDER_FOLLOW_FIRE = "HostageHideSquad",
		ORDER_FORM		="HostageFollow",
		ORDER_FOLLOW	="HostageFollow",
		RETRIEVE_OBJECT	="HostageRetrieve",
		ORDER_FOLLOW_HIDE ="HostageFollowHide",
		GO_TO_DUMB = "Dumb",
		GO_TO_PREVIOUS = "PREVIOUS",
		OnExplosionDanger		= "HBaseGrenadeRun",
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
		OnFallAndPlayWakeUp              = "Cover2Idle",
		OnBackOffFailed 				 = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		GO_TO_ATTACK_FORCED              = "Cover2Attack",
		GO_TO_ATTACK_GROUP_FORCED        = "Cover2AttackGroup",
		GO_TO_RUSH_ATTACK_FORCED         = "Cover2RushAttack",
		GO_TO_HIDE_FORCED                = "Cover2Hide",
		GO_TO_AVOID_TANK_FORCED          = "Cover2AvoidTank",
		GO_TO_RPG_ATTACK_FORCED          = "Cover2RPGAttack",
		GO_TO_THREATENED_FORCED          = "Cover2Threatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Cover2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Cover2Interested",
		GO_TO_SEEK_FORCED                = "Cover2Seek",
		GO_TO_SEARCH_FORCED              = "Cover2Search",
		GO_TO_RELOAD_FORCED              = "Cover2Reload",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Cover2CallReinforcements",
		GO_TO_IDLE_FORCED                = "Cover2Idle",
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
	
	
	HostageIdle = {
		OnPlayerSeen    	= "",
		OnInterestingSoundHeard = "",
		OnSomethingSeen		= "",
		ORDER_HIDE = "",
	--	OnThreateningSoundHeard = "HostageHide",


--		OnBulletRain		= "HostageHide",
--		OnDamage	= "HostageHide",
--		OnEnemyDamage	= "HostageHide",
--		OnFriendlyDamage	= "HostageHide",
--
--		OnGroupMemberDied	= "",
--		OnGrenadeSeen		= "HostageHide",
--		INCOMING_FIRE		= "HostageHide",
--
--		ORDER_HOLD	 = "HostageHideSquad",
--		ORDER_MOVE	 = "HostageMove",		
		
	},


	HostageFollow = {	
		OnPlayerSeen			="",
		-----------------------------------
		--odresr here

		--ORDER_FORM		="HostageFollow",--do not remove this, needed for following while in combat
		ORDER_FORM		="",
		ORDER_FOLLOW  ="",
		FOLLOW_ME_HERE = "HostageFollowed",
		ORDER_FOLLOW_HIDE ="HostageFollowHide",
		GO_TO_HIDE = "HostageHide",
		OnEnemyDamage = "HostageHide",
--		ORDER_USE	 = "HostageUse",
	},

	HostageHide = {	
		ORDER_FORM		="HostageFollow",
		ORDER_HOLD = "HostageHide",
		ORDER_FOLLOW_HIDE ="HostageFollowHide",
	},

	HostageFollowed = {	
		REFPOINT_REACHED = "HostageIdle",
		ENTERING_VEHICLE = "",
	},
	
	HostageHideSquad = {	
		--ORDER_FOLLOW		="HostageFollow",
		ORDER_FOLLOW		="HostageFollow",
		ORDER_FORM  ="HostageFollow",
		GO_TO_FOLLOW		="HostageFollow",
		ORDER_FOLLOW_HIDE ="HostageFollowHide",
		--ORDER_FOLLOW_FIRE = "HostageFollow",
	},

	HostageRetrieve = {
		OnExplosionDanger = "",
		IGNORE_THREATS = "Dumb",
	},
--	HostageMove = {	
--		ORDER_HOLD	 = "HostageHide",
--		ORDER_FORM		="HostageFollow",
--		ORDER_MOVE	 = "HostageMove",		
--		ORDER_FOLLOW_HIDE ="HostageFollowHide",
--	},

	CivilianCower = {
	
	},

	EnteringVehicle = {
--		REFPOINT_REACHED = "InVehicle",
		entered_vehicle	= "InVehicle",
		exited_vehicle	= "HostageFollow",
	},

	HostageTied = {
		ORDER_FORM		="HostageFollow",
		ORDER_FOLLOW_HIDE ="HostageFollowHide",
	},

	HBaseGrenadeRun = {
		GRENADE_END_REACTION = "",
		GRENADE_END_REACTION_GROUP = "",
		ORDER_HIDE = "",
		USE_MOUNTED_WEAPON = "",
		ORDER_FIRE = "",
		OnPlayerSeen    	= "",
		OnBulletRain		= "",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
		entered_vehicle	="",
		exited_vehicle	="",
		exited_vehicle_investigate = "",
		OnSomethingSeen			="",
		OnExplosionDanger = "",
		END_BACKOFF = "",
		OnExplosion = "",
	},
	
		Dumb = {
		GO_TO_IDLE =							"HostageIdle",
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
	},
}