--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

local MIN_UPDATE_TIME = 0.1;
local FOLLOW_UPDATE_TIMERS = {

}
local FOLLOW = {

}

---ИИ будет следовать и защищать цель
---@param executor table ИИ-Сущность
---@param target table ИИ-Сущность, за которой будет следовать executor
---@param distance integer дистанция, которую будет держать executor от цели
---@param speed integer скорость с которой нужно добраться (0, 1, 2, 3)
function AI_FOLLOW_AND_PROTECT(executor, target, distance, speed, max_alertness)

    --TODO: сделать версию для транспорта
    --TODO: сделать версию для скаутов

    if (not executor) then
        LogAlways("[ORDER] <AI_FOLLOW_AND_PROTECT> executor not found")
        return
    end
    
    if (not target) then
        LogAlways("[ORDER] <AI_FOLLOW_AND_PROTECT> target not found")
        return
    end

    LogAlways("[ORDER] <AI_FOLLOW_AND_PROTECT> target: "..target:GetName())

    executor.orderRefEnt = target
    executor.currentExecutedOrder = EOrders.AI_FOLLOW_AND_PROTECT
    executor.followUpdateTimer = nil
    executor.follow = {
        target = executor.orderRefEnt,
        distance = distance,
        speed = speed
    }

    TOS_AI.BeginGoalPipe("ai_follow_and_protection")
        TOS_AI.PushGoal(GO.STRAFE, 1, 100, 100)
        TOS_AI.PushGoal(GO.RUN, 1, executor.follow.speed)
        TOS_AI.PushGoal(GO.STICK, 1, executor.follow.distance, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 2, 0)
    TOS_AI.EndGoalPipe()
    -- TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOS_OBEY_FOLLOW_AND_PROTECT", executor.id)
    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "START_AI_FOLLOW_AND_PROTECT", executor.id)
end

function CLEAR_DATA_AI_FOLLOW_AND_PROTECT(entity)
    if (not entity) then
        LogError("<CLEAR_DATA_AI_FOLLOW_AND_PROTECT> entity not found")
        return
    end

    if (entity.followUpdateTimer) then
        Script.KillTimer(entity.followUpdateTimer)
        LogAlways("<CLEAR_DATA_AI_FOLLOW_AND_PROTECT> kill timer %s", tostring(entity.followUpdateTimer))
        entity.followUpdateTimer = nil
    end

    entity.currentExecutedOrder = EOrders.NONE
    entity.orderRefEnt = nil
    entity.follow = nil
end

function UPDATE_AI_FOLLOW_AND_PROTECT(entity)
    if (not entity) then
        LogError("<UPDATE_AI_FOLLOW_AND_PROTECT> entity not found")
        return
    end

    local vDirToTarget = {}
    SubVectors( vDirToTarget, entity.orderRefEnt:GetWorldPos(), entity:GetWorldPos() );
    
    local distanceToTarget = LengthVector( vDirToTarget );
    if (distanceToTarget > entity.follow.distance + 1) then
        TOS_AI.InsertSubpipe(AIGOALPIPE_NOTDUPLICATE, entity, "ai_follow_and_protection", entity.orderRefEnt.id)
        TOS_AI.SetRefPointPosition(entity.id, entity.orderRefEnt:GetWorldPos())
    end

    entity.followUpdateTimer = Script.SetTimerForFunction(MIN_UPDATE_TIME * 1000 , "UPDATE_AI_FOLLOW_AND_PROTECT", entity)
end

function AIBehaviour.DEFAULT:START_AI_FOLLOW_AND_PROTECT(entity, sender)
    System.LogAlways(string.format("[ORDER] <START_AI_FOLLOW_AND_PROTECT> entity: %s", 
        EntityName(entity))
    )

    if (entity.followUpdateTimer ~= nil) then
        Script.KillTimer(entity.followUpdateTimer)
        entity.followUpdateTimer = nil
    end

    entity.followUpdateTimer = Script.SetTimerForFunction(MIN_UPDATE_TIME * 1000 , "UPDATE_AI_FOLLOW_AND_PROTECT", entity)
end

function AIBehaviour.DEFAULT:AI_FOLLOW_AND_PROTECT_STARTED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_FOLLOW_AND_PROTECT_STARTED> entity: %s", 
        EntityName(entity))
    )
end

function AIBehaviour.DEFAULT:AI_FOLLOW_AND_PROTECT_ENDED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_FOLLOW_AND_PROTECT_ENDED> entity: %s", 
        EntityName(entity))
    )

    CLEAR_DATA_AI_FOLLOW_AND_PROTECT(entity)

    -- TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOS_OBEY", entity.id)

    local targetType = TOS_AI.GetTargetType(entity.id)
    if targetType == AITARGET_ENEMY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "GO_TO_ATTACK_FORCED", entity.id)
    elseif targetType == AITARGET_MEMORY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "GO_TO_SEARCH_FORCED", entity.id)
    else
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "GO_TO_IDLE_FORCED", entity.id)
    end

    TOS_AI.SelectPipe(AIGOALPIPE_SAMEPRIORITY, entity, "do_it_standing")
    OnOrderComplete(entity)
end

