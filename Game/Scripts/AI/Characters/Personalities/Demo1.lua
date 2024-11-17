-- CAMPER CHARACTER SCRIPT
-- This suposed to be a very careful soldier, tends to prone/crouch a lot;
-- stays in cover a lot, does not move much
--
--History:
--	created by: Kirill Bulatsev 
--
AICharacter.Demo1 = {

	Class = UNIT_CLASS_INFANTRY,
	
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		GO_TO_TOS_OBEY     = "TOS_Obey",
		GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT     = "TOS_Obey_Follow_and_Protect",

		--~TheOtherSide

		ENTERING_VEHICLE = "EnteringVehicle",
		OnExplosionDanger		= "HBaseGrenadeRun",
		RETURN_TO_FIRST = "FIRST",
		USE_MOUNTED_WEAPON = "UseMounted",
		ATTACK_TANK_RPG = "HBaseAttackTankRpg",
		ATTACK_TANK_GRENADES = "HBaseAttackTankGrenades",
		HIDE_FROM_TANK = "CamperHide",
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
	
	DemoIdle1 = {
		HEADS_UP_GUYS		= "CamperUnderFire",	
		OnPlayerSeen    	= "DemoShoot1",
		OnInterestingSoundHeard = "DemoShoot1",
		OnSomethingSeen		= "DemoShoot1",
		GET_ALERTED	 = "",
		NORMAL_THREAT_SOUND 	= "DemoShoot1",
--		OnThreateningSoundHeard = "CoverThreatened",

		OnBulletRain		= "",
		OnEnemyDamage	= "",
--		OnDamage	= "CamperAlert",
		OnGroupMemberDiedNearest 	= "",
--		GRENADE		= "HBaseGrenadeRun",

		OnSomebodyDied	= "",
		OnGrenadeSeen		= "",
		INCOMING_FIRE		= "",
		-----------------------------------


		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
--		exited_vehicle = "PREVIOUS",
	},
	
	
--	MountedGuy = {
--		SELECT_RED		= "CoverRedIdle",
--		SELECT_BLACK		= "CoverBlackIdle",
--		CONVERSATION_FINISHED   = "MountedGuy",
--	},

	HBaseAttackTankRpg = {
		OnPlayerSeen    	= "CamperAttack",
		OnSomethingSeen   = "CamperAlert",
		OnThreateningSoundHeard  = "CamperAttack",
		OnInterstingSoundHeard  = "CamperAlert",
		OnCloseContact = "CamperAttack",
	},

	CamperNoTarget = {
		OnPlayerSeen    	= "CamperAttack",
		GO_TO_ATTACK    	= "CamperAttack",
		OnInterstingSoundHeard  = "",
		OnThreateningSoundHeard  = "",

		HEADS_UP_GUYS		= "CamperUnderFire",	

		OnEnemyDamage	= "CamperUnderFire",
		OnDamage	= "CamperUnderFire",

		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		
		-- pre combat
	},


	CamperAlert = {
		OnPlayerSeen    	= "CamperAttack",
		GO_TO_ATTACK    	= "CamperAttack",
		OnInterstingSoundHeard  = "",
		OnThreateningSoundHeard  = "",

		HEADS_UP_GUYS		= "CamperUnderFire",	

		OnEnemyDamage	= "CamperUnderFire",
		OnDamage	= "CamperUnderFire",

		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		
		-- pre combat
	},

	CamperHide = {
		REFPOINT_REACHED = "CamperAlert",
		SWITCH_TO_ATTACK = "CamperAttack",
--		OnLowHideSpot	= "DigIn",
	},

	CamperAttack = {
--		OnNoHidingPlace = "CoverHold",
		OnNoTarget		= "CamperNoTarget",
		OnEnemyMemory = "CamperPeek",
		OnLowHideSpot = "HBaseLowHide",
		BACKOFF = "HBaseBackOff",
		
		-----------------------------------
		-- Vehicles related
--		entered_vehicle = "InVehicle",
		--OnLowHideSpot	= "DigIn",
		LEFT_LEAN_ENTER		= "LeanFire",
		RIGHT_LEAN_ENTER	= "LeanFire",
	},

	HBaseBackOff= {
--		REFPOINT_REACHED = "CoverAttack",
		BACKOFF_DONE = "CamperAttack",
		BACKOFF = "",
	},

	HBaseLowHide = {
--		OnNoHidingPlace = "CoverHold",
		BACKOFF = "HBaseBackOff",

		DONT_SEE_TARGET = "CamperPeek",
		OnNoTarget		= "CamperNoTarget",
--		SpotCompromized	= "CamperAttack",	
--		OnEnemyMemory = "CamperPeek",
		OnEnemyMemory = "",	
		
		LEFT_LEAN_ENTER		= "LeanFire",
		RIGHT_LEAN_ENTER	= "LeanFire",
	},


	CamperPeek = {
		OnPlayerSeen    	= "CamperAttack",
		OnNoTarget		= "CamperNoTarget",
		
		OnInterestingSoundHeard = "",
		OnSomethingSeen		= "",
		GET_ALERTED	 = "CamperAttack",
		HOLD_POSITION		= "CamperAttack",
--		NORMAL_THREAT_SOUND 	= "CamperAlert",
--		OnThreateningSoundHeard = "CoverThreatened",

		OnEnemyDamage	= "CamperUnderFire",
--		OnDamage	= "CamperAlert",
--		GRENADE		= "HBaseGrenadeRun",
		INCOMING_FIRE		= "CamperUnderFire",
	},


	CamperUnderFire = {
		OnPlayerSeen    	= "CamperAttack",	
	},

	CheckDead= {
		OnBulletRain		= "",	-- don't react on bullets rain - keep going
		HEADS_UP_GUYS		= "CamperUnderFire",
		be_alerted		= "CamperUnderFire",
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

}
