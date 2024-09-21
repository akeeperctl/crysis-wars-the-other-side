AICharacter.TrooperCloak = {

	Constructor = function(self, entity)
		entity.AI.FireMode = 0;
		-- if(not AI.GetLeader(entity.id)) then
		-- 	AI.SetLeader(entity.id);
		-- end
	end,

	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED        = "TOSSHARED",
		GO_TO_TOS_OBEY         = "TOS_Obey",
		--~TheOtherSide
		RETURN_TO_FIRST        = "FIRST",
		ORDER_FORM             = "TrooperForm",
		ORDER_MOVE             = "TrooperMove",
		MOVE                   = "TrooperMove",
		JOIN_TEAM              = "TrooperGroupIdle",
		ORDER_APPROACH         = "TrooperApproach",
		ORDER_SEARCH           = "TrGroupSearch",
		ORDER_COVER_SEARCH     = "TrGroupCoverSearch",
		ORDER_HIDE_AROUND      = "TrooperGroupCombat", --"TrooperHideShoot"
		ORDER_FLANK            = "TrooperAttackFlank",
		ORDER_FIRE             = "TrooperGroupFire",
		GO_TO_THREATENED       = "TrooperThreatened",
		GO_TO_INTERESTED       = "TrooperInterested",
		GO_TO_AMBUSH           = "TrooperAmbush",
		GO_TO_DUMB             = "TrooperDumb",
		GO_TO_SEARCH           = "TrooperSearch",
		GO_TO_ATTACK           = "TrooperAttack",
		GO_TO_DEFENSE          = "TrooperDefend",
		GO_TO_MOAR             = "TrooperAttackMoar",
		GO_TO_DODGE            = "TrooperDodge",
		GO_TO_RETREAT          = "TrooperRetreat",
		GO_TO_GRABBED          = "TrooperGrabbedByScout",
		GO_TO_IDLE             = "TrooperIdle",
		GO_TO_ATTACK_JUMP      = "TrooperAttackJump",
		GUARDIAN_ENSLAVE       = "TrooperGuardianMinionBasic",

		OnAttackChase          = "TrooperChase",
		DODGE_GRENADE          = "TrooperDodgeGrenade",
		OnExplosionDanger      = "TrooperAvoidExplosion",
		PURSUE                 = "TrooperAttackPursue",
		OnAttackShootSpot      = "TrooperShootOnSpot",
		GO_TO_ON_ROCK          = "TrooperShootOnRock",
		GO_TO_ON_WALL          = "TrooperShootOnWall",
		OnAttackSwitchPosition = "TrooperAttackSwitchPosition",
		OnSpecialAction        = "TrooperAttackSpecialAction",
		GO_TO_SPECIAL_ACTION   = "TrooperAttackSpecialAction",
		OnFallAndPlay          = "TrooperDown",
	},

	--TheOtherSide
	TOS_Obey = {
		-- Стандартные сигналы
		-- переопределяют те, что в AnyBehavior
		RESUME_FOLLOWING            = "",
		ENTERING_VEHICLE            = "",
		USE_MOUNTED_WEAPON          = "",
		OnPlayerSeen                = "",
		OnTankSeen                  = "",
		OnHeliSeen                  = "",
		OnBulletRain                = "",
		OnGrenadeSeen               = "",
		OnInterestingSoundHeard     = "",
		OnThreateningSoundHeard     = "",
		entered_vehicle             = "",
		exited_vehicle              = "",
		exited_vehicle_investigate  = "",
		OnSomethingSeen             = "",
		GO_TO_AVOIDEXPLOSIVES       = "",
		GO_TO_AVOIDVEHICLE          = "",
		GO_TO_CHECKDEAD             = "",
		GO_TO_IDLE                  = "",
		GO_TO_ATTACK                = "",
		GO_TO_ATTACK_GROUP          = "",
		GO_TO_RUSH_ATTACK           = "",
		GO_TO_HIDE                  = "",
		GO_TO_AVOID_TANK            = "",
		GO_TO_RPG_ATTACK            = "",
		GO_TO_THREATENED            = "",
		GO_TO_THREATENED_STANDBY    = "",
		GO_TO_INTERESTED            = "",
		GO_TO_SEEK                  = "",
		GO_TO_SEARCH                = "",
		GO_TO_RELOAD                = "",
		GO_TO_CALL_REINFORCEMENTS   = "",
		GO_TO_PANIC                 = "",
		GO_TO_STATIC                = "",
		ENEMYSEEN_FIRST_CONTACT     = "",
		ENEMYSEEN_DURING_COMBAT     = "",
		OnFallAndPlayWakeUp         = "TrooperIdle",
		OnBackOffFailed             = "",

		ORDER_FORM                  = "",
		ORDER_MOVE                  = "",
		MOVE                        = "",
		JOIN_TEAM                   = "",
		ORDER_APPROACH              = "",
		ORDER_SEARCH                = "",
		ORDER_COVER_SEARCH          = "",
		ORDER_HIDE_AROUND           = "",
		ORDER_FLANK                 = "",
		ORDER_FIRE                  = "",
		GO_TO_AMBUSH                = "",
		GO_TO_DUMB                  = "",
		GO_TO_DEFENSE               = "",
		GO_TO_MOAR                  = "",
		GO_TO_DODGE                 = "",
		GO_TO_RETREAT               = "",
		GO_TO_GRABBED               = "",
		GO_TO_ATTACK_JUMP           = "",
		GUARDIAN_ENSLAVE            = "",

		OnAttackChase               = "",
		DODGE_GRENADE               = "",
		OnExplosionDanger           = "",
		PURSUE                      = "",
		OnAttackShootSpot           = "",
		GO_TO_ON_ROCK               = "",
		GO_TO_ON_WALL               = "",
		OnAttackSwitchPosition      = "",
		OnSpecialAction             = "",
		GO_TO_SPECIAL_ACTION        = "",
		OnFallAndPlay               = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		ORDER_FORM_FORCED           = "TrooperForm",
		ORDER_MOVE_FORCED           = "TrooperMove",
		MOVE_FORCED                 = "TrooperMove",
		JOIN_TEAM_FORCED            = "TrooperGroupIdle",
		ORDER_APPROACH_FORCED       = "TrooperApproach",
		ORDER_SEARCH_FORCED         = "TrGroupSearch",
		ORDER_COVER_SEARCH_FORCED   = "TrGroupCoverSearch",
		ORDER_HIDE_AROUND_FORCED    = "TrooperGroupCombat",
		ORDER_FLANK_FORCED          = "TrooperAttackFlank",
		ORDER_FIRE_FORCED           = "TrooperGroupFire",
		GO_TO_THREATENED_FORCED     = "TrooperThreatened",
		GO_TO_INTERESTED_FORCED     = "TrooperInterested",
		GO_TO_AMBUSH_FORCED         = "TrooperAmbush",
		GO_TO_DUMB_FORCED           = "TrooperDumb",
		GO_TO_SEARCH_FORCED         = "TrooperSearch",
		GO_TO_ATTACK_FORCED         = "TrooperAttack",
		GO_TO_DEFENSE_FORCED        = "TrooperDefend",
		GO_TO_MOAR_FORCED           = "TrooperAttackMoar",
		GO_TO_DODGE_FORCED          = "TrooperDodge",
		GO_TO_RETREAT_FORCED        = "TrooperRetreat",
		GO_TO_GRABBED_FORCED        = "TrooperGrabbedByScout",
		GO_TO_IDLE_FORCED           = "TrooperIdle",
		GO_TO_ATTACK_JUMP_FORCED    = "TrooperAttackJump",
		GUARDIAN_ENSLAVE_FORCED     = "TrooperGuardianMinionBasic",
		GO_TO_ON_ROCK_FORCED        = "TrooperShootOnRock",
		GO_TO_ON_WALL_FORCED        = "TrooperShootOnWall",
		GO_TO_SPECIAL_ACTION_FORCED = "TrooperAttackSpecialAction",
		DODGE_GRENADE_FORCED        = "TrooperDodgeGrenade",
		PURSUE_FORCED               = "TrooperAttackPursue",
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
		OnFallAndPlayWakeUp        = "TrooperIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide

	TrooperChase = {
		OnAttackChase = "",
		--		OnAttackSwitchPosition = "",
	},

	TrooperRetreat = {
		OnAttackChase = "",
		OnAttackShootSpot = "",
		OnAttackSwitchPosition = "",
		GO_TO_ATTACK = "TrooperAttack",
		RETREAT_OK = "TrooperHide",
		REGROUP = "TrooperAttack",
	},

	TrooperHide = {
		OnAttackChase = "",
		OnAttackShootSpot = "",
		OnAttackSwitchPosition = "",
		REGROUP = "TrooperAttack",
	},


	TrGroupSearch = {
		ORDER_SEARCH = "",
	},

	TrGroupCoverSearch = {
	},




	TrooperIdle = {
		--OnInterestingSoundHeard = "TrooperInterested",
		--GET_ALERTED	 				= "TrooperAlert",
		--		HEADS_UP_GUYS				= "TrooperAttack",
	},

	TrooperInterested = {
		OnPlayerSeen    = "TrooperAttack",
		--OnThreateningSoundHeard = "TrooperThreatened",
		--		OnBulletRain				= "TrooperAlert",
		--OnGroupMemberDied		= "TrooperAlert",
		--OnObjectSeen				= "TrooperAlert",
		RETURN_TO_FIRST = "FIRST",
	},

	TrooperThreatened = {
		OnPlayerSeen    = "TrooperAttack",
		--OnNoTarget					= "TrooperAlert",
		RETURN_TO_FIRST = "FIRST",
	},

	TrooperAlert = {
		OnPlayerSeen            = "TrooperAttack",
		OnInterestingSoundHeard = "",
		OnThreateningSoundHeard = "",
	},

	TrooperAttack = {
		GO_TO_SEARCH     = "TrooperSearch",
		GO_TO_INTERESTED = "",
		GO_TO_THREATENED = "",
		--OnAttackShootSpot = "",
	},

	TrooperAttackSwitchPosition = {
		--		OnNoTarget					= "TrooperSearch",
		--		LOOK_FOR_TARGET			= "TrooperSearch",
		GO_TO_JUMP             = "",
		GO_TO_MELEE            = "TrooperAttackSwitchPositionMelee",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION  = "TrooperAttackSwitchPosition",

		--OnPlayerLooking = "",
		--OnPlayerLooking = "TrooperDodge",
	},

	TrooperAttackSwitchPositionMelee = {
		END_MELEE = "TrooperAttackSwitchPosition",
		MELEE_FAILED = "TrooperAttackSwitchPosition",
		OnAttackShootSpot = "",
		OnAttackSwitchPosition = "",
		OnLand = "TrooperAttackSwitchPosition",
		BackToSwitchPosition = "TrooperAttackSwitchPosition",
	},

	TrooperAttackSpecialAction = {
		--END_MELEE = "TrooperAttackSwitchPosition",
		MELEE_FAILED = "TrooperAttackSwitchPosition",
		--GO_TO_MELEE				= "TrooperAttackSwitchPositionMelee",

		OnAttackShootSpot = "",
		OnAttackSwitchPosition = "",
		OnLand = "",
		OnSpecialAction = "",
		BackToSwitchPosition = "TrooperAttackSwitchPosition",
	},


	TrooperShootOnSpot = {
		GO_TO_MELEE = "TrooperAttackSwitchPositionMelee",

	},

	TrooperShootOnRock = {
		OnSpecialAction = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION = "TrooperAttackSwitchPosition",
		--		GO_TO_SPECIAL_ACTION= "TrooperAttackSpecialAction",
	},

	TrooperShootOnWall = {
		OnSpecialAction = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION = "TrooperAttackSwitchPosition",
		--	GO_TO_SPECIAL_ACTION= "TrooperAttackSpecialAction",
	},



	TrooperAttackMoar = {
		OnAttackSwitchPosition = "",
		OnAttackShootSpot      = "",
		GO_TO_MOAR             = "",
		GO_TO_ON_SPOT          = "TrooperShootOnSpot",
		OnSpecialAction        = "",
	},

	TrooperAttackPursue = {
		--		OnNoTarget					= "TrooperSearch",
		--		LOOK_FOR_TARGET			= "TrooperSearch",
	},

	TrooperGrabbedByScout = {
		GO_TO_INTERESTED       = "",
		--GO_TO_ATTACK = "",
		OnAttackSwitchPosition = "",
		GO_TO_SWITCH_POSITION  = "TrooperAttackSwitchPosition",
		OnAttackShootSpot      = "",
		GO_TO_MOAR             = "",
		GO_TO_ON_SPOT          = "TrooperShootOnSpot",
		GO_TO_SPECIAL_ACTION   = "TrooperAttackSpecialAction",
		OnSpecialAction        = "",
		GO_TO_ON_ROCK          = "",
		GO_TO_ON_WALL          = "",
		ORDER_SEARCH           = "",
		ORDER_COVER_SEARCH     = "",
		GO_TO_SEARCH           = "TrGroupSearch",
		GO_TO_MELEE            = "",
		GO_TO_DODGE            = "",
		GO_TO_ATTACK_JUMP      = "",
		OnAttackChase          = "",
		OnExplosionDanger      = "",
		PURSUE                 = "",
		OnFallAndPlay          = "",
		GRABBED_TO_INTERESTED  = "TrooperInterested",
		GRABBED_TO_ATTACK      = "TrooperAttack",
		--ORDER_SEARCH = "",
	},

	-- Search job, use idle transitions.	
	TrooperSearch = {
		OnPlayerSeen = "TrooperAttack",
		HIDESPOT_NOT_FOUND = "TrooperIdle",
		GO_TO_IDLE = "TrooperIdle",
		SpotReached = "TrooperAttack",
	},

	TrooperDumb = {
		GO_TO_SEARCH      = "TrooperSearch",
		DODGE             = "",
		DODGE_GRENADE     = "",
		OnExplosionDanger = "",
	},

	TrooperDodge = {
		DODGE_FAILED           = "TrooperAttackSwitchPosition",
		DODGE_FINISHED         = "TrooperAttackSwitchPosition",
		OnAttackSwitchPosition = "",
		OnAttackShootSpot      = "",
		END_DODGE_FORWARD      = "TrooperAttackSpecialAction",
	},

	TrooperAvoidExplosion = {
		DODGE             = "",
		DODGE_GRENADE     = "",
		OnExplosionDanger = "",
		PURSUE            = "",
	},

	TrooperAttackJump = {
		--		OnLand = "TrooperAttackSwitchPosition",
		OnAttackSwitchPosition = "",
		OnSpecialAction = "",
		OnAttackShootSpot = "",
		JUMP_ON_ROCK = "",
		JUMP_ON_WALL = "",
		GO_TO_SWITCH_POSITION = "TrooperAttackSwitchPosition",
		--		GO_TO_SPECIAL_ACTION= "TrooperAttackSpecialAction",
		GO_TO_ON_SPOT = "TrooperShootOnSpot",
	},

	--HBaseTranquilized = {
	TrooperDown = {
		GO_TO_IDLE = "TrooperIdle",
		OnFallAndPlayWakeUp = "TrooperAttack",
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

	--- Old behaviors--------------------------------------------------------------------------------

	TrooperLure = {
		JOIN_TEAM    = "",
		GO_TO_AMBUSH = "",
		GO_TO_DUMB   = "TrooperGroupDumb",
	},

	TrooperAvoidExplosion2 = {
		DODGE                  = "",
		DODGE_GRENADE          = "",
		OnExplosionDanger      = "",
		PURSUE                 = "",
		OnAttackSwitchPosition = "",
		BackToSwitchPosition   = "TrooperAttackSwitchPosition",

	},
	TrooperForm = {
		--		ORDER_ATTACK_FORMATION = "TrooperAttackFormation",
		ORDER_FLANK = "TrooperAttackFlank",
		GO_TO_DUMB  = "TrooperGroupDumb",
		--		OnLeaderDied = "TrooperBerserk",
	},

	TrooperMove = {
		--		ORDER_ATTACK_FORMATION = "TrooperAttackFormation",
		ORDER_FLANK = "TrooperAttackFlank",
		GO_TO_DUMB  = "TrooperGroupDumb",
		--		OnLeaderDied = "TrooperBerserk",
	},

	--	TrooperAttackFormation = {
	--		OnLeaderDied = "TrooperAttack",
	--		ORDER_FLANK				= "TrooperAttackFlank",
	----		ORDER_HIDE_AROUND = "TrGroupCombat",
	--	},

	TrooperHideShoot = {
	},

	TrooperGroupCombat = {
		GO_TO_DUMB = "TrooperGroupDumb",
	},

	TrooperGroupFire = {
		GO_TO_DUMB = "TrooperGroupDumb",
	},

	TrooperCollectiveFire1 = {

	},

	TrooperCollectiveFire2 = {

	},


	TrooperAmbush = {
		JOIN_TEAM  = "",
		GO_TO_DUMB = "TrooperGroupDumb",
	},

	TrooperGroupIdle = {
		ORDER_MOVE       = "TrooperMove",
		ORDER_FORM       = "TrooperForm",
		GO_TO_INTERESTED = "TrooperGroupInterested",
		GO_TO_DUMB       = "TrooperGroupDumb",
	},

	TrooperGroupInterested = {
		ORDER_MOVE = "TrooperMove",
		ORDER_FORM = "TrooperForm",
		GO_TO_DUMB = "TrooperGroupDumb",
	},
	TrooperApproach = {
		ORDER_APPROACH = "",

	},

	TrooperAttackFlank = {
		--OnLeaderDied = "TrooperBerserk",
	},

	TrooperJump = {
		OnPlayerSeen = "TrooperAttack",
		JUMP_TO = "PREVIOUS",
	},

	TrooperGuardianMinionBasic = {
		----------------------------------
		-- These events and signals are handled here to prevent guardian minions to handle them
		--	from TrooperCloak character AnyBehavior
		----------------------------------
		RETURN_TO_FIRST        = "",
		ORDER_FORM             = "",
		ORDER_MOVE             = "",
		MOVE                   = "",
		JOIN_TEAM              = "",
		ORDER_APPROACH         = "",
		ORDER_SEARCH           = "",
		ORDER_COVER_SEARCH     = "",
		ORDER_HIDE_AROUND      = "",
		ORDER_FLANK            = "",
		ORDER_FIRE             = "",
		GO_TO_THREATENED       = "",
		GO_TO_INTERESTED       = "",
		GO_TO_AMBUSH           = "",
		GO_TO_DUMB             = "",
		GO_TO_SEARCH           = "",
		GO_TO_ATTACK           = "",
		GO_TO_DEFENSE          = "",
		GO_TO_MOAR             = "",
		GO_TO_DODGE            = "",
		GO_TO_RETREAT          = "",
		GO_TO_GRABBED          = "",
		GO_TO_IDLE             = "",

		GUARDIAN_ENSLAVE       = "",
		GUARDIAN_DIED          = "TrooperAttack",

		OnAttackChase          = "",
		DODGE_GRENADE          = "",
		OnExplosionDanger      = "",
		PURSUE                 = "",
		--		ORDER_COORDINATED_FIRE1 = "",
		--		ORDER_COORDINATED_FIRE2 = "",
		OnAttackShootSpot      = "",
		GO_TO_ON_ROCK          = "",
		GO_TO_ON_WALL          = "",
		OnAttackSwitchPosition = "",
		--OnSpecialAction		= "",
		OnSpecialAction        = "",
		--OnAvoidDanger			= "",
		--OnFallAndPlay			= "",
	},
}
