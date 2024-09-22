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

---Запрос свободного и незабронированного места в транспорте
---@param vehEntity table
---@return number|nil seatId идентификатор места в транспорте
function TOS_Vehicle:RequestSeat(vehEntity)
	for i,seat in pairs(vehEntity.Seats) do
		if (seat:IsFree() and not seat.passengerId) then
			return i;
		end
	end

	return nil;
end

--- Забронировать место в транспорте для пассажира до момента, пока он не сядет.
---@param vehEntity table
---@param seatId number|nil
---@param passengerId userdata
---@return true|nil true в случае успеха
function TOS_Vehicle:ReserveSeatForPassenger(vehEntity, seatId, passengerId)
	if not vehEntity then
		LogError("<TOS_Vehicle:ReserveSeatForPassenger> vehicle not defined")
		return nil
	elseif not passengerId then
		LogError("[%s] <TOS_Vehicle:ReserveSeatForPassenger> invalid passengerId: %s", EntityName(vehEntity), tonumber(passengerId))
		return nil
	end
	
	local seatInstance = vehEntity.Seats[seatId]
	if not seatInstance then
		LogError("[%s] <TOS_Vehicle:ReserveSeatForPassenger> seat instance not defined with seatId: %s", EntityName(vehEntity), tostring(seatId))
		return nil
	elseif not seatInstance:IsFree() then
		LogWarning("[%s] <TOS_Vehicle:ReserveSeatForPassenger> seat is not free", EntityName(vehEntity))
		return nil
	elseif seatInstance.passengerId then
		LogWarning("[%s] <TOS_Vehicle:ReserveSeatForPassenger> seat is already reserved to %s", EntityName(vehEntity), EntityName(seatInstance.passengerId))
		return nil
	end

	seatInstance.passengerId = passengerId

	return true;
end

--- Разбронировать место в транспорте.
---@param vehEntity table
---@param seatId number
---@return unknown
function TOS_Vehicle:UnreserveSeat(vehEntity, seatId)
	if not vehEntity then
		LogError("<TOS_Vehicle:UnreserveSeat> vehicle is not defined")
		return nil
	end
	
	local seatInstance = vehEntity.Seats[seatId]
	if not seatInstance then
		LogError("[%s] <TOS_Vehicle:UnreserveSeat> seat instance is not defined", EntityName(vehEntity))
		return nil
	elseif not seatInstance.passengerId then
		LogWarning("[%s] <TOS_Vehicle:UnreserveSeat> seat is already unreserved", EntityName(vehEntity))
	end

	seatInstance.passengerId = nil

	return true;
end

--------------------------------------------------------------------------

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