--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2019-2024.
--*************************************************************************
--------------------------------------------------------------------------
--  History:
--  - 31/08/2022     : Created by Akeeper
--
--------------------------------------------------------------------------

-- use own static vectors here, to not mess up with g_Vectors.* used in signals calling these functions
TrVector_v0 = {x=0,y=0,z=0};
TrVector_v1 = {x=0,y=0,z=0};
TrVector_v2 = {x=0,y=0,z=0};
TrVector_v3 = {x=0,y=0,z=0};
TrVector_v4 = {x=0,y=0,z=0};
TrVector_v5 = {x=0,y=0,z=0};
Trooper_bbmin_cache = {x=0,y=0,z=0};
Trooper_bbmax_cache = {x=0,y=0,z=0};

function TOS_Trooper_BerserkTime(entity)
	local maxHealth = entity.actor:GetMaxHealth()
	local currentHealth = entity.actor:GetHealth();

	local berserkTime = currentHealth <= (maxHealth * 0.2); --20% of health

	if(berserkTime) then 
		return true;
	end
		return false;
end