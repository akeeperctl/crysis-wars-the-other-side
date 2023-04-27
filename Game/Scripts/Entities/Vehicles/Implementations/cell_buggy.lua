--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2006.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Implementation of Civ_car1
--  
--------------------------------------------------------------------------
--  History:11
--  - : Created by Michael Rauh
--
--------------------------------------------------------------------------


--------------------------------------------------------------------------


--------------------------------------------------------------------------
cell_buggy =
{	
}



--------------------------------------------------------------------------


cell_buggy.AIProperties = 
{
	-- AI attributes
  AIType = AIOBJECT_CAR,
  PropertiesInstance = 
  {
    aibehavior_behaviour = "CarIdle",
  },
  Properties = 
  {
    aicharacter_character = "Car",
  },
  AIMovementAbility = 
  {
		walkSpeed = 30.0,
		runSpeed = 60.0,
		sprintSpeed = 90.0,
		maneuverSpeed = 20.0,
    minTurnRadius = 2,
    maxTurnRadius = 85,    
    pathType = AIPATH_CAR,
		pathLookAhead = 32,
		pathRadius = 0.8,
		pathSpeedLookAheadPerSpeed = 16,
		cornerSlowDown = 0.5,
		pathFindPrediction = 0.90,
    velDecay = 160,
		resolveStickingInTrace = 0,
		pathRegenIntervalDuringTrace = 4.0,
		avoidanceRadius = 10.0,
  },     
	hidesUser=1,
}

