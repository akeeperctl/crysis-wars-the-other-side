--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character script for alien indoors
--  
--------------------------------------------------------------------------
--  History:
--  - 7/7/2004   : Created by Mikko Mononen
--
--------------------------------------------------------------------------

AICharacter.GuardNeue = {

	AnyBehavior = {
		RETURN_TO_FIRST = "FIRST",
		TO_FIRST					= "FIRST",
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		--~TheOtherSide

--		TO_RUNTOFRIEND		= "GuardCover",
--		TO_CALLREINF			=	"GuardCallReinf",
--		TO_PURSUE					=	"GuardPursue",
		OnFlowgraphCapture	= "GuardDumb",
		GO_TO_IDLE						=	"GuardNeueIdle",
		GO_TO_COMBAT					=	"GuardNeueCombat",
		OnFallAndPlay =						"HBaseTranquilized",
	},

	GuardDumb = {
		TO_PURSUE					=	"GuardPursue",
		OnFlowgraphCapture	= "",
	},

	GuardNeueIdle = {
	},

	GuardNeueCombat = {
	},
	
	HBaseTranquilized = {
		GO_TO_IDLE =							"GuardNeueIdle",
		OnFallAndPlayWakeUp = 		"GuardNeueIdle",
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
	
	
}