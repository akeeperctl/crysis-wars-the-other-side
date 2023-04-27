AICharacter.Drone = {

	AnyBehavior = 
	{
		RETURN_TO_FIRST = "FIRST",
		TO_FIRST		= "FIRST",
	},
	DroneIdle = 
	{
		TO_SCOUTMOAC_IDLE					= "",
		TO_SCOUTMOAC_ATTACK				= "DroneAttack",
		TO_SCOUTMOAC_PATROL				= "DronePatrol",
	},	
	DroneAttack = 
	{
		TO_SCOUTMOAC_IDLE					= "DroneIdle",
		TO_SCOUTMOAC_ATTACK				= "",
		TO_SCOUTMOAC_PATROL				= "DronePatrol",
	},
	DronePatrol = 
	{
		TO_SCOUTMOAC_IDLE					= "DroneCIdle",
		TO_SCOUTMOAC_ATTACK				= "DroneAttack",
		TO_SCOUTMOAC_PATROL				= "",
	},

}
