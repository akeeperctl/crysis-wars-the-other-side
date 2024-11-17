--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

---ИИ доберется до позиции независимо от обстоятельств
---@param executor table ИИ-Сущность
---@param target table ИИ-Сущность, которую будет преследовать executor
---@param distance integer дистанция, которую будет держать executor от цели
---@param speed integer скорость с которой нужно добраться (0, 1, 2, 3)
function AI_PURSUIT_AND_KILL(executor, target, distance, speed)

    --TODO: сделать версию для транспорта
    --TODO: сделать версию для скаутов

    if (not executor) then
        LogAlways("[ORDER] <AI_PURSUIT_AND_KILL> executor not found")
        return
    end
    if (not target) then
        LogAlways("[ORDER] <AI_PURSUIT_AND_KILL> target not found")
        return
    end

    LogAlways("[ORDER] <AI_PURSUIT_AND_KILL> target: "..target:GetName())

    executor.orderRefEnt = target
    executor.currentExecutedOrder = EOrders.AI_PURSUIT_AND_KILL

    TOS_AI.BeginGoalPipe("ai_pursuit_and_kill")
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_PURSUIT_AND_KILL_STARTED", SIGNALFILTER_SENDER)
        TOS_AI.PushGoal(GO.LOCATE, 1, executor.orderRefEnt:GetName())
        TOS_AI.PushGoal(GO.ACQUIRE_TARGET, 1, "")
        TOS_AI.PushGoal(GO.BRANCH, 1, "PURSUIT", BRANCH_ALWAYS)

        TOS_AI.PushLabel("PURSUIT")
            -- if TOS_AI.GetTargetType(executor.id) == AITARGET_ENEMY then
            --     TOS_AI.PushGoal(GO.STRAFE, 1, 10, distance * 1.5)
            --     TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_BURST_DRAWFIRE)
            -- else
            --     TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
            -- end

            -- TOS_AI.PushGoal(GO.STRAFE, 1, 100, 100)
            -- TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_BURST_DRAWFIRE)
            TOS_AI.PushGoal(GO.BODYPOS, 1, BODYPOS_STAND)
            TOS_AI.PushGoal(GO.RUN, 1, speed)
            TOS_AI.PushGoal(GO.STICK, 1, distance, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 5)
            TOS_AI.PushGoal(GO.HIDE, 1, distance, HM_NEAREST_TO_TARGET)
            TOS_AI.PushGoal(GO.BRANCH, 1, "PURSUIT_NEAR", NOT + IF_SEES_TARGET)
            TOS_AI.PushGoal(GO.BRANCH, 1, "KILL", IF_SEES_TARGET)
            TOS_AI.PushGoal(GO.BRANCH, 1, "LOOP", BRANCH_ALWAYS)

        TOS_AI.PushLabel("PURSUIT_NEAR")
            TOS_AI.PushGoal(GO.STRAFE, 1, 1, distance)
            TOS_AI.PushGoal(GO.BODYPOS, 1, BODYPOS_STAND)
            TOS_AI.PushGoal(GO.RUN, 1, speed)
            TOS_AI.PushGoal(GO.STICK, 1, 5, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 5)
            TOS_AI.PushGoal(GO.HIDE, 1, distance, HM_NEAREST_TO_TARGET)
            TOS_AI.PushGoal(GO.BRANCH, 1, "KILL", IF_SEES_TARGET)
            TOS_AI.PushGoal(GO.BRANCH, 1, "LOOP", BRANCH_ALWAYS)

        TOS_AI.PushLabel("KILL")
            -- TOS_AI.PushGoal(GO.LOCATE, 1, executor.orderRefEnt:GetName(), 1)
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_KILL)
            TOS_AI.PushGoal(GO.TIMEOUT, 1, 2.0)
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
            TOS_AI.PushGoal(GO.BRANCH, 1, "PURSUIT_NEAR", NOT + IF_SEES_TARGET)
    
        TOS_AI.PushLabel("END")
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
            TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_PURSUIT_AND_KILL_ENDED", SIGNALFILTER_SENDER)

        TOS_AI.PushLabel("LOOP")
            TOS_AI.PushGoal(GO.BRANCH, 1, "PURSUIT", IF_TARGET_DIST_GREATER, distance)
            TOS_AI.PushGoal(GO.BRANCH, 1, "END", IF_NO_ENEMY_TARGET + NOT + IF_SEES_TARGET)
    TOS_AI.EndGoalPipe()

    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOS_OBEY", executor.id)
    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "START_AI_PURSUIT_AND_KILL", executor.id)
end

function CLEAR_DATA_AI_PURSUIT_AND_KILL(entity)
    if (not entity) then
        LogError("<CLEAR_DATA_AI_PURSUIT_AND_KILL> entity not found")
        return
    end

    entity.currentExecutedOrder = EOrders.NONE
    AI.ClearPrimaryHostile(entity.id)

    -- if (entity.orderRefEnt) then
    --     System.RemoveEntity(entity.orderRefEnt.id)
    --     Log("<CLEAR_DATA_AI_PURSUIT_AND_KILL> order ref entity removed")
    -- end

    entity.orderRefEnt = nil
end

function AIBehaviour.DEFAULT:START_AI_PURSUIT_AND_KILL(entity, sender)
    System.LogAlways(string.format("[ORDER] <START_AI_PURSUIT_AND_KILL> entity: %s", 
        EntityName(entity))
    )

    AI.MakePrimaryHostile(entity.id, entity.orderRefEnt.id)
    TOS_AI.SelectPipe(AIGOALPIPE_LOOP, entity, "ai_pursuit_and_kill", entity.orderRefEnt.id)
end

function AIBehaviour.DEFAULT:AI_PURSUIT_AND_KILL_STARTED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_PURSUIT_AND_KILL_STARTED> entity: %s", 
        EntityName(entity))
    )
end

function AIBehaviour.DEFAULT:AI_PURSUIT_AND_KILL_ENDED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_PURSUIT_AND_KILL_ENDED> entity: %s", 
        EntityName(entity))
    )

    CLEAR_DATA_AI_PURSUIT_AND_KILL(entity)

    local targetType = TOS_AI.GetTargetType(entity.id)
    if targetType == AITARGET_ENEMY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_ATTACK_FORCED", entity.id)
    elseif targetType == AITARGET_MEMORY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_SEARCH_FORCED", entity.id)
    else
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_IDLE_FORCED", entity.id)
    end

    TOS_AI.SelectPipe(AIGOALPIPE_SAMEPRIORITY, entity, "do_it_standing")
    OnOrderComplete(entity)
end