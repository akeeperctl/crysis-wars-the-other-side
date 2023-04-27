--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Implementation of a US Hovercraft
--  
--------------------------------------------------------------------------
--  History:
--  - 04/22/2005   16:35 : Created by Michael Rauh
--
--------------------------------------------------------------------------



US_hovercraft = 
{
	--State = VehicleState,
	Sounds = {},
	
	Properties = 
	{		
		bDisableEngine = 0,
		material = "",
		bFrozen = 0,
		Modification = "",
		FrozenModel = "",
		soclasses_SmartObjectClass = "",
	},
		
	Client = {},
	Server = {}
}

--------------------------------------------------------------------------
function US_hovercraft:GetSpecificFireProperties()
	if (self.AIFireProperties) then
		self.AIFireProperties[1] = {};
		self.AIFireProperties[1].min_distance = 20;
		self.AIFireProperties[1].max_distance = 400;
	end
end

US_hovercraft.AIProperties = 
{
	-- AI attributes
  AIType = AIOBJECT_BOAT,
  AICombatClass = SafeTableGet(AICombatClasses, "Tank"),  
  PropertiesInstance = 
  {
    aibehavior_behaviour = "CarIdle",
	triggerRadius = 90,
  },
  Properties = 
  {
    aicharacter_character = "Car",

	Perception =
	{
		FOVPrimary = -1,			-- normal fov
		FOVSecondary = -1,		-- periferial vision fov
		sightrange = 400,
		persistence = 10,
	},		

  },
  AIMovementAbility = 
  {
		walkSpeed = 7.0,
		runSpeed = 15.0,
		sprintSpeed = 20.0,
		maneuverSpeed = 5.0,
    minTurnRadius = 0.5,
    maxTurnRadius = 7,
    passRadius = 8.0,
    pathType = AIPATH_BOAT,
    pathLookAhead = 20,
    pathRadius = 3,
		pathSpeedLookAheadPerSpeed = 3.0,
		cornerSlowDown = 0.75,
		pathFindPrediction = 1.0,
    velDecay = 3,
		maneuverTrh = 2.0,
		resolveStickingInTrace = 0,
		pathRegenIntervalDuringTrace = 10.0,
		avoidanceRadius = 3.0,
  },    
}