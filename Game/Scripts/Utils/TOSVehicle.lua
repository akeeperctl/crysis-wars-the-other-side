--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************


TOS_Vehicle = {

}
--------------------------------------------------------------------------
function TOS_Vehicle:DriverSelectWeaponByIndex(vehEntity, index)
	local savedIndex = index;
	local i;

	for i,seat in pairs(vehEntity.Seats) do
		if( seat.passengerId ) then
			local member = System.GetEntity( seat.passengerId );
			if( member ~= nil ) then
				if (seat.isDriver) then
					local howManyWeapons = seat.seat:GetWeaponCount();
					if ( howManyWeapons > 0 ) then
						if (savedIndex > howManyWeapons) then
							savedIndex = howManyWeapons;
						end

						local weaponId = seat.seat:GetWeaponId(savedIndex);
						local w = System.GetEntity(weaponId);

						if (w) then
							seat.seat:SetAIWeapon(weaponId);
							-- System.LogAlways("[LUA][TOS_Vehicle:DriverSelectWeaponByIndex] select: "..tostring(index))
						end
					end
				end
			end
		end
	end
end


--------------------------------------------------------------------------
function TOS_Vehicle:GetSeatWeaponCount(vehEntity, seatId)
	local seat = vehEntity.Seats[seatId];
	if (not seat) then
		--System.LogAlways("[LUA][TOS_Vehicle:GetSeatWeaponCount] Seat NULL");
		return;
	end

	local count = seat.seat:GetWeaponCount()

	--System.LogAlways("[LUA][TOS_Vehicle:GetSeatWeaponCount] Seat: "..seatId.." Count: "..count);
	return count;
end


--------------------------------------------------------------------------
function TOS_Vehicle:RequestGunnerSeat(vehEntity)
	for i,seat in pairs(vehEntity.Seats) do
		--System.LogAlways("[LUA][TOS_Vehicle:RequestGunnerSeat] "..i);
		if (seat:IsFree() and seat.seat:GetWeaponCount() > 0 and not seat.IsDriver) then
			--System.LogAlways("[LUA][TOS_Vehicle:RequestGunnerSeat] GUNNER");
			return i;
		end
	end

	return nil;
end

--------------------------------------------------------------------------
function TOS_Vehicle:RequestSeat(vehEntity)
	for i,seat in pairs(vehEntity.Seats) do
		if (seat:IsFree()) then
			return i;
		end
	end

	return nil;
end

function TOS_Vehicle:GetEnterRadius(vehEntity)
	local maximum = 10

	--if (self.vehicle:GetMovementType()=="air" and not self.vehicle:IsEngineDisabled()) then
	if (vehEntity.vehicle:GetMovementType()=="air") then
		maximum = maximum + 5
	end

	local bbmin, bbmax = vehEntity:GetWorldBBox();
	local output = g_Vectors.temp_v1
	SubVectors(output, bbmax, bbmin)

	local radius = LengthVector(output) * 0.5; 
	return math.min(math.max(4.0, radius), maximum)
end
--~TheOtherSide