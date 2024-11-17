--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character script for Trooper Leader Drone
--
--------------------------------------------------------------------------
--  History:
--  - 8/2/2006   : Created by Luciano Morpurgo
--
--------------------------------------------------------------------------

AICharacter.TrooperLeader = {

	Constructor = function(self, entity)
		entity.AI.FireMode = 0;
		entity.AI.bIsLeader = true;
	end,

	AnyBehavior = {
		RETURN_TO_FIRST         = "FIRST",

		--TheOtherSide
		GO_TO_TOSSHARED         = "TOSSHARED",
		GO_TO_TOS_OBEY     = "TOS_Obey",
		GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT     = "TOS_Obey_Follow_and_Protect",

		--~TheOtherSide

		GO_TO_IDLE              = "TrooperLeaderIdle",
		ORDER_FORM              = "TrooperForm",
		ORDER_MOVE              = "TrooperMove",
		MOVE                    = "TrooperMove",
		ORDER_APPROACH          = "TrooperApproach",
		ORDER_SEARCH            = "TrGroupSearch",
		ORDER_COVER_SEARCH      = "TrGroupCoverSearch",
		ORDER_HIDE_AROUND       = "TrooperLeaderAttack",
		ORDER_FLANK             = "TrooperAttackFlank",
		ORDER_FIRE              = "TrooperLeaderFire",
		GO_TO_AMBUSH            = "TrooperAmbush",
		GO_TO_DUMB              = "TrooperDumb",
		DODGE                   = "TrooperDodge",
		ORDER_COORDINATED_FIRE1 = "TrooperLeaderCollectiveFire",
		ORDER_COORDINATED_FIRE2 = "TrooperCollectiveFire2",
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
		OnFallAndPlayWakeUp              = "TrooperGuardianIdle",
		OnBackOffFailed                  = "",

		ORDER_FORM                       = "",
		ORDER_MOVE                       = "",
		MOVE                             = "",
		JOIN_TEAM                        = "",
		ORDER_APPROACH                   = "",
		ORDER_SEARCH                     = "",
		ORDER_COVER_SEARCH               = "",
		ORDER_HIDE_AROUND                = "",
		ORDER_FLANK                      = "",
		ORDER_FIRE                       = "",
		GO_TO_AMBUSH                     = "",
		GO_TO_DUMB                       = "",
		GO_TO_DEFENSE                    = "",
		GO_TO_MOAR                       = "",
		GO_TO_DODGE                      = "",
		GO_TO_RETREAT                    = "",
		GO_TO_GRABBED                    = "",
		GO_TO_ATTACK_JUMP                = "",
		GUARDIAN_ENSLAVE                 = "",

		OnAttackChase                    = "",
		DODGE_GRENADE                    = "",
		OnExplosionDanger                = "",
		PURSUE                           = "",
		OnAttackShootSpot                = "",
		GO_TO_ON_ROCK                    = "",
		GO_TO_ON_WALL                    = "",
		OnAttackSwitchPosition           = "",
		OnSpecialAction                  = "",
		GO_TO_SPECIAL_ACTION             = "",
		OnFallAndPlay                    = "",
		CONTROL_TROOPER                  = "",
		CONTROL_COVER                    = "",
		CHOOSE_TO_SEEK                   = "",
		CHOOSE_TO_SEARCH                 = "",
		CHOOSE_TO_ATTACK                 = "",
		DODGE                   = "",
		ORDER_COORDINATED_FIRE1 = "",
		ORDER_COORDINATED_FIRE2 = "",

		-- Форсированные стандартные сигналы
		-- ИИ поведения, созданные крайтеками используют
		-- стандартные сигналы, а эти нужны для
		-- вызовов при завершении приказа или в аналогичных ситуациях,
		-- где нужен 100% переход в нужное поведение.
		--TR Leader
		DODGE_FORCED                     = "TrooperDodge",
		ORDER_COORDINATED_FIRE1_FORCED   = "TrooperLeaderCollectiveFire",
		ORDER_COORDINATED_FIRE2_FORCED   = "TrooperCollectiveFire2",

		--TR1
		GO_TO_IDLE_FORCED                = "TrooperLeaderIdle",
		ORDER_FORM_FORCED                = "TrooperForm",
		ORDER_MOVE_FORCED                = "TrooperMove",
		MOVE_FORCED                      = "TrooperMove",
		ORDER_APPROACH_FORCED            = "TrooperApproach",
		ORDER_SEARCH_FORCED              = "TrGroupSearch",
		ORDER_COVER_SEARCH_FORCED        = "TrGroupCoverSearch",
		ORDER_HIDE_AROUND_FORCED         = "TrooperLeaderAttack",
		ORDER_FLANK_FORCED               = "TrooperAttackFlank",
		ORDER_FIRE_FORCED                = "TrooperLeaderFire",
		GO_TO_AMBUSH_FORCED              = "TrooperAmbush",
		JOIN_TEAM_FORCED                 = "TrooperGroupIdle",
		GO_TO_DEFENSE_FORCED             = "TrooperDefend",
		GO_TO_MOAR_FORCED                = "TrooperAttackMoar",
		GO_TO_DODGE_FORCED               = "TrooperDodge",
		GO_TO_RETREAT_FORCED             = "TrooperRetreat",
		GO_TO_ATTACK_JUMP_FORCED         = "TrooperAttackJump",
		GUARDIAN_ENSLAVE_FORCED          = "TrooperGuardianMinionBasic",
		GO_TO_SPECIAL_ACTION_FORCED      = "TrooperAttackSpecialAction",
		DODGE_GRENADE_FORCED             = "TrooperDodgeGrenade",
		PURSUE_FORCED                    = "TrooperAttackPursue",

		--TR2
		GO_TO_DUMB_FORCED                = "Trooper2Dumb",
		GO_TO_ATTACK_FORCED              = "Trooper2Attack",
		GO_TO_ATTACK_GROUP_FORCED        = "Trooper2AttackGroup",
		GO_TO_HIDE_FORCED                = "Trooper2Hide",
		GO_TO_THREATENED_FORCED          = "Trooper2Threatened",
		GO_TO_THREATENED_STANDBY_FORCED  = "Trooper2ThreatenedStandby",
		GO_TO_INTERESTED_FORCED          = "Trooper2Interested",
		GO_TO_SEEK_FORCED                = "Trooper2Seek",
		GO_TO_SEARCH_FORCED              = "Trooper2Search",
		GO_TO_AVOIDEXPLOSIVES_FORCED     = "Trooper2AvoidExplosives",
		GO_TO_AVOIDVEHICLE_FORCED        = "Trooper2AvoidVehicle",
		GO_TO_CHECKDEAD_FORCED           = "Trooper2CheckDead",
		GO_TO_CALL_REINFORCEMENTS_FORCED = "Trooper2CallReinforcements",
		OnAttackSwitchPosition_FORCED    = "Trooper2AttackSwitchPosition",
		GO_TO_GRABBED_FORCED             = "Trooper2GrabbedByScout",
		GO_TO_ON_ROCK_FORCED             = "Trooper2ShootOnRock",
		GO_TO_ON_WALL_FORCED             = "Trooper2ShootOnWall",
		CONTROL_TROOPER_FORCED           = "Trooper2AttackT",
		CONTROL_COVER_FORCED             = "Trooper2Attack",
		CHOOSE_TO_SEE_FORCEDK            = "Trooper2Seek",
		CHOOSE_TO_SEARCH_FORCED          = "Trooper2Search",
		CHOOSE_TO_ATTACK_FORCED          = "Trooper2Attack",
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
		OnFallAndPlayWakeUp        = "TrooperGuardianIdle",
		OnBackOffFailed            = "",
	},
	--~TheOtherSide


	TrooperApproach = {
		ORDER_APPROACH = "",
	},

	TrooperAttackFlank = {
	},

	TrooperJump = {
		OnPlayerSeen = "TrooperAttack",
		JUMP_TO = "PREVIOUS",
	},

	TrGroupSearch = {
		ORDER_SEARCH = "",
	},

	TrGroupCoverSearch = {
	},

	TrooperLure = {
		JOIN_TEAM    = "",
		GO_TO_AMBUSH = "",
		GO_TO_DUMB   = "TrooperGroupDumb",
	},

	TrooperForm = {
		ORDER_FLANK = "TrooperAttackFlank",
		GO_TO_DUMB  = "TrooperGroupDumb",
	},

	TrooperMove = {
		ORDER_FLANK = "TrooperAttackFlank",
		GO_TO_DUMB  = "TrooperGroupDumb",
	},


	TrooperLeaderAttack = {
		GO_TO_DUMB = "TrooperGroupDumb",
	},

	TrooperLeaderFire = {
		GO_TO_DUMB = "TrooperGroupDumb",
	},


	TrooperLeaderIdle = {
		ORDER_MOVE       = "TrooperMove",
		ORDER_FORM       = "TrooperForm",
		GO_TO_INTERESTED = "TrooperGroupInterested",
		GO_TO_DUMB       = "TrooperGroupDumb",
		--		OnEnemySeenByUnit = "TrooperLeaderPreAttack",
	},

	TrooperLeaderPreAttack = {
		ORDER_MOVE       = "TrooperMove",
		ORDER_FORM       = "TrooperForm",
		GO_TO_INTERESTED = "TrooperGroupInterested",
		GO_TO_DUMB       = "TrooperGroupDumb",
	},

	TrooperLeaderCollectiveFire =
	{
	},


	TrooperAmbush = {
		JOIN_TEAM  = "",
		GO_TO_DUMB = "TrooperGroupDumb",
	},

	--	TrooperGroupIdle = {
	--		ORDER_MOVE				= "TrooperMove",
	--		ORDER_FORM				= "TrooperForm",
	--		GO_TO_INTERESTED  = "TrooperGroupInterested",
	--		GO_TO_DUMB				= "TrooperGroupDumb",
	--	},

	TrooperGroupInterested = {
		ORDER_MOVE = "TrooperMove",
		ORDER_FORM = "TrooperForm",
		GO_TO_DUMB = "TrooperGroupDumb",
	},


	TrooperCollectiveFire2 = {

	},

	TrooperDodge = {
		GO_TO_GROUP_ATTACK = "TrooperLeaderAttack",
	},
}
