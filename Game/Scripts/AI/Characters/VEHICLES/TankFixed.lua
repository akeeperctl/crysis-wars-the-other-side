--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Character SCRIPT for Tank
--  
--------------------------------------------------------------------------
--  History:
--  - 06/02/2005   : Created by Kirill Bulatsev
--  - 17/07/2006   : Dulplicated for the special tank by Tetsuji
--  
--------------------------------------------------------------------------

AICharacter.TankFixed = {
	
	Constructor = function(self,entity)	
--		entity.AI.DesiredFireDistance[1] = 30; -- main gun
--		entity.AI.DesiredFireDistance[2] = 6; -- secondary machine gun
		entity.AI.weaponIdx = 1; --temp: select main gun by default
	end,
	
	AnyBehavior = {
		--TheOtherSide
		GO_TO_TOSSHARED = "TOSSHARED",
		--~TheOtherSide
		STOP_VEHICLE = "TankFixedIdle",
		GO_TO_IDLE = "TankFixedIdle"

	},
	
	TankFixedIdle = {
		-----------------------------------
		FOLLOW              = "TankFixedFollow",
		ACT_GOTO            = "TankFixedGoto",

		EVERYONE_OUT        = "",
		STOP_VEHICLE 			  = "",
		DRIVER_OUT          = "",
		VEHICLE_GOTO_DONE   = "",

		TO_TANKCLOSE_ATTACK = "";
	
		OnPlayerSeen        = "",		

	},
	
	TankFixedFollow = {
		-----------------------------------
		FOLLOW              = "",
		ACT_GOTO            = "TankFixedGoto",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE 			  = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "";
	
		OnPlayerSeen        = "",		

	},

	TankFixedGoto = {
		-----------------------------------
		FOLLOW              = "TankFixedFollow",
		ACT_GOTO            = "",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE 			  = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "";
	
		OnPlayerSeen        = "",		

	},

	TankFixedAttack = {
		-----------------------------------
		FOLLOW              = "",
		ACT_GOTO            = "",

		EVERYONE_OUT        = "TankFixedIdle",
		STOP_VEHICLE 			  = "TankFixedIdle",
		DRIVER_OUT          = "TankFixedIdle",
		VEHICLE_GOTO_DONE   = "TankFixedIdle",

		TO_TANKCLOSE_ATTACK = "";
	
		OnPlayerSeen        = "",		

	},

	
}
