-- PLAYER CHARACTER SCRIPT

AICharacter.Player = {

	Class = UNIT_CLASS_LEADER,
	
	InitItems = function(self,entity)
		entity.AI.WeaponAccessoryTable = {};
		entity.AI.WeaponAccessoryTable["SCARIncendiaryAmmo"] = 0;
		entity.AI.WeaponAccessoryTable["SCARNormalAmmo"] = 2;
		entity.AI.WeaponAccessoryTable["Silencer"] = 0;
		entity.AI.NanoSuitCloak = false;
		entity.AI.NanoSuitMode = 0;
		--entity.AI.WeaponAccessoryTable["Flashlight"] = 0;
	end,
	
	Constructor = function(self,entity)
		self:InitItems(entity);
		AI.ChangeParameter( entity.id, AIPARAM_COMBATCLASS, AICombatClasses.Infantry );
	end,
	
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",		
		GO_TO_TOS_OBEY = "TOS_Obey",
		--~TheOtherSide
	
		-----------------------------------
		-- Vehicles related - player should not be AI-enabling vehicle
		entered_vehicle = "",
		entered_vehicle_gunner = "",
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
		OnFallAndPlayWakeUp              = "",
		OnBackOffFailed 				 = "",

		GO_TO_ATTACK_FORCED              = "PlayerAttack",
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
		OnFallAndPlayWakeUp              = "",
		OnBackOffFailed 				 = "",
	},
	--~TheOtherSide
	
	PlayerIdle = {
		-----------------------------------
		-- Vehicles related - player should not be AI-enabling vehicle
		entered_vehicle = "",
		entered_vehicle_gunner = "",
		START_ATTACK = "PlayerAttack",
	},

	PlayerAttack = {
		entered_vehicle = "",
		entered_vehicle_gunner = "",
		OnLeaderActionCompleted = "PlayerIdle",
		OnLeaderActionFailed = "PlayerIdle",
	},
}
