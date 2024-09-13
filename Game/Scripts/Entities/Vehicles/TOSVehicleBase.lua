
--TheOtherSide
--------------------------------------------------------------------------
function VehicleBase:TOSDriverSelectWeaponByIndex(index)
	local savedIndex = index;
	local i;

	for i,seat in pairs(self.Seats) do
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
							System.LogAlways("[LUA][VehicleBase:DriverSelectWeaponByIndex] select: "..tostring(index))
						end
					end
				end
			end
		end
	end
end


--------------------------------------------------------------------------
function VehicleBase:TOSGetSeatWeaponCount(seatId)
	local seat = self.Seats[seatId];
	if (not seat) then
		--System.LogAlways("[LUA][VehicleBase:GetSeatWeaponCount] Seat NULL");
		return;
	end

	local count = seat.seat:GetWeaponCount()

	--System.LogAlways("[LUA][VehicleBase:GetSeatWeaponCount] Seat: "..seatId.." Count: "..count);
	return count;
end


--------------------------------------------------------------------------
function VehicleBase:TOSRequestGunnerSeat(userId)
	for i,seat in pairs(self.Seats) do
		--System.LogAlways("[LUA][VehicleBase:RequestGunnerSeat] "..i);
		if (seat:IsFree() and seat.seat:GetWeaponCount() > 0 and not seat.IsDriver) then
			--System.LogAlways("[LUA][VehicleBase:RequestGunnerSeat] GUNNER");
			return i;
		end
	end

	return nil;
end

--------------------------------------------------------------------------
function VehicleBase:TOSRequestSeat(userId)
	for i,seat in pairs(self.Seats) do
		if (seat:IsFree()) then
			return i;
		end
	end

	return nil;
end
--~TheOtherSide