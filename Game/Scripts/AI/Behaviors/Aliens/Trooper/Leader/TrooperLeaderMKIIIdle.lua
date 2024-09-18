--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2007.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Idle behavior for Guardian troopers
--  and most common callback from lua
--  Modify it only to add new empty system/most common callbacks
--  
--------------------------------------------------------------------------
--  History:
--  - 26.07.2007     : Created by Denisz Polgar
--
--------------------------------------------------------------------------
AIBehaviour.TrooperLeaderMKIIIdle = {
	Name = "TrooperLeaderMKIIIdle",-- must be the same as the behavior name
	Base = "TrooperMKIIIdle",

	---------------------------------------------
	Constructor = function(self,entity)
		
		--TheOtherSide
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		entity:ApplyLeaderStuff();
		AIBehaviour.TrooperMKIIIdle:Constructor( entity );
		--System.LogAlways("FROM LUA ApplyGuardianStuff");
		
		
	end,	

	Destructor = function(self,entity)
		AIBehaviour.TrooperMKIIIdle:Destructor( entity );
	end,
}