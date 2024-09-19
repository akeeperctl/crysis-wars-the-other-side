--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for alien outdoors
--  
--------------------------------------------------------------------------
--  History:
--  - 09/05/2005   : Created by Mikko Mononen
--
--------------------------------------------------------------------------

AICharacter.HunterNew = {

	AnyBehavior = {
		RETURN_TO_FIRST = "FIRST",
		TO_FIRST		= "FIRST",
		GO_TO_IDLE = 		"HunterNewIdle",
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		--~TheOtherSide
	},

	HunterNewIdle = {
		OnPlayerSeen    				= "HunterNewAttack",
		OnEnemyDamage						= "HunterNewAttack",
	--	OnThreateningSoundHeard = "HunterAttack",
	--	GO_PATROL								= "HunterPatrol",
	--	GO_TO_ATTACK								= "HunterAttack",
	},

	HunterNewAttack = {
		OnPlayerSeen    				= "",
--		OnNoHidingPlace 				= "",
		OnNoTarget							= "HunterNewIdle",
--		OnSoreDamage						= "HunterRecoil",
--		GO_ENEMY_LOST						= "HunterPatrol",
--		GO_RECOIL								= "HunterRecoil",
	},

	HunterNewPatrol = {
--		OnPlayerSeen    				= "HunterAttack",
--		OnEnemyDamage						= "HunterAttack",
--		OnThreateningSoundHeard = "HunterAttack",
--		GO_ENEMY_FOUND					= "HunterAttack",
--		GO_TO_ATTACK								= "HunterAttack",
--		GO_SEARCH    						= "HunterSearch",
--		GO_ESCORT								= "HunterEscort",
	},
}
