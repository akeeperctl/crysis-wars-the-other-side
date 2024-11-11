--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

function AI_ENTERVEHICLE(entity, vehicle)
    if (not entity) then
        LogError("<AI_ENTERVEHICLE> entity not found")
        return
    elseif not vehicle then
        LogError("<AI_ENTERVEHICLE> vehicle not found")
        return
    end

    local seatId = TOS_Vehicle:RequestSeat(vehicle)
    local reserved = TOS_Vehicle:ReserveSeatForPassenger(vehicle, seatId, entity.id)
    if (not reserved) then
        return
    end

    entity.currentExecutedOrder = EOrders.AI_ENTERVEHICLE
    entity.vehicleToEnter = vehicle
    entity.seatIdToEnter = seatId
    entity.animatedEnter = false

    TOS_AI.BeginGoalPipe("ai_entervehicle")
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_ENTERVEHICLE_STARTED", SIGNALFILTER_SENDER)

        if TOS_AI.GetTargetType(entity.id) == AITARGET_ENEMY then
            TOS_AI.PushGoal(GO.STRAFE, 1, 5, 7)
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_BURST_DRAWFIRE)
        else
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
        end

        TOS_AI.PushGoal(GO.BODYPOS, 1, BODYPOS_STAND)
        TOS_AI.PushGoal(GO.RUN, 1, 3)
        TOS_AI.PushGoal(GO.STICK, 1, 1, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 5)
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_ENTERVEHICLE_PREEND", SIGNALFILTER_SENDER)
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_PROCESS_NEXT_UPDATE, "AI_ENTERVEHICLE_ENDED", SIGNALFILTER_SENDER)
    TOS_AI.EndGoalPipe()

    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOS_OBEY", entity.id)
    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "START_AI_ENTERVEHICLE", entity.id)
end

function CLEAR_DATA_AI_ENTERVEHICLE(entity)
    if (not entity) then
        LogError("<CLEAR_DATA_AI_ENTERVEHICLE> entity not found")
        return
    end

    if (entity.vehicleToEnter and entity.seatIdToEnter) then
        local vehicle = entity.vehicleToEnter
        local seatId = entity.seatIdToEnter

        local unreserved = TOS_Vehicle:UnreserveSeat(vehicle, seatId)
        if (unreserved == true) then
            Log("[%s] <CLEAR_DATA_AI_ENTERVEHICLE> seat %s unreserved", EntityName(entity), tostring(seatId))
        end

    end

    entity.vehicleToEnter = nil
    entity.seatIdToEnter = nil
    entity.animatedEnter = nil
    Log("[%s] <CLEAR_DATA_AI_ENTERVEHICLE> vehicle data cleared", EntityName(entity))
end

function AIBehaviour.DEFAULT:START_AI_ENTERVEHICLE(entity, sender)
    System.LogAlways(string.format("[%s] <START_AI_ENTERVEHICLE> %s", 
        EntityName(entity),
        EntityName(entity.vehicleToEnter))
    )

    TOS_AI.SelectPipe(AIGOALPIPE_LOOP, entity, "ai_entervehicle", entity.vehicleToEnter.id)
end

function AIBehaviour.DEFAULT:AI_ENTERVEHICLE_STARTED(entity, sender)
    System.LogAlways(string.format("[%s] <AI_ENTERVEHICLE_STARTED>", 
        EntityName(entity))
    )

end

function AIBehaviour.DEFAULT:AI_ENTERVEHICLE_PREEND(entity, sender)
    System.LogAlways(string.format("[%s] <AI_ENTERVEHICLE_PREEND>", 
        EntityName(entity))
    )

    local vehicle = entity.vehicleToEnter
    local seatId = entity.seatIdToEnter
    local animated = entity.animatedEnter

    vehicle.vehicle:EnterVehicle(entity.id, seatId, animated)
end

function AIBehaviour.DEFAULT:AI_ENTERVEHICLE_ENDED(entity, sender)
    System.LogAlways(string.format("[%s] <AI_ENTERVEHICLE_ENDED>", 
        EntityName(entity))
    )

    local vehicleId = entity.actor:GetLinkedVehicleId()
    if (not vehicleId) then

        local targetType = TOS_AI.GetTargetType(entity.id)

        if targetType == AITARGET_ENEMY then
            TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_ATTACK_FORCED", entity.id)
        elseif targetType == AITARGET_MEMORY then
            TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_SEARCH_FORCED", entity.id)
        else
            TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_IDLE_FORCED", entity.id)
        end
    
        TOS_AI.SelectPipe(AIGOALPIPE_SAMEPRIORITY, entity, "do_it_standing")
    else
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "entered_vehicle", entity.id)
    end

    CLEAR_DATA_AI_ENTERVEHICLE(entity)
    OnOrderComplete(entity)
end