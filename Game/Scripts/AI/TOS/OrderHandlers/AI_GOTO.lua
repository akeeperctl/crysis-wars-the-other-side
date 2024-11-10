--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

---ИИ доберется до позиции независимо от обстоятельств
---@param entity table ИИ-Сущность
---@param position Vec3 позиция, куда нужно добраться
---@param speed integer скорость с которой нужно добраться (0, 1, 2, 3)
function AI_GOTO(entity, position, speed)
    if (not entity) then
        LogAlways("[ORDER] <AI_GOTO> entity not found")
        return
    end

    local orderRefName = EntityName(entity).."_orderRefPoint"
    local orderRefEnt = EntityNamed(orderRefName)

    -- получаем сущность для определения позиции приказа
    if (not orderRefEnt) then

        local spawnParams  = {}
        spawnParams.class = "TagPoint"
        spawnParams.position = position
        spawnParams.orientation = {x=0,y=-1,z=0};
        spawnParams.name = orderRefName

        orderRefEnt = System.SpawnEntity(spawnParams)

       --  LogAlways("<AI_GOTO> order tag point spawned: %s, ai: %s", orderRefEnt:GetName(), tostring(AI.HasAI(orderRefEnt.id)))
        g_SpawnParams:reset()
    else
        orderRefEnt:SetWorldPos(position)
        -- LogAlways("<AI_GOTO> order tag point updated: %s", orderRefName)
    end

    entity.orderRefEnt = orderRefEnt
    entity.currentExecutedOrder = EOrders.AI_GOTO

    TOS_AI.BeginGoalPipe("ai_goto")
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_GOTO_STARTED", SIGNALFILTER_SENDER)

        if TOS_AI.GetTargetType(entity.id) == AITARGET_ENEMY then
            TOS_AI.PushGoal(GO.STRAFE, 1, 5, 7)
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_BURST_DRAWFIRE)
        else
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
        end

        TOS_AI.PushGoal(GO.BODYPOS, 1, BODYPOS_STAND)
        TOS_AI.PushGoal(GO.RUN, 1, speed)
        TOS_AI.PushGoal(GO.STICK, 1, 1, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 5)
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_GOTO_ENDED", SIGNALFILTER_SENDER)
    TOS_AI.EndGoalPipe()

    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOS_OBEY", entity.id)
    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "START_AI_GOTO", entity.id)
end

function CLEAR_DATA_AI_GOTO(entity)
    if (not entity) then
        LogError("<CLEAR_DATA_AI_GOTO> entity not found")
        return
    end

    entity.currentExecutedOrder = EOrders.NONE

    if (entity.orderRefEnt) then
        System.RemoveEntity(entity.orderRefEnt.id)
        Log("<CLEAR_DATA_AI_GOTO> order ref entity removed")
    end

    entity.orderRefEnt = nil
end

function AIBehaviour.DEFAULT:START_AI_GOTO(entity, sender)
    System.LogAlways(string.format("[ORDER] <START_AI_GOTO> entity: %s", 
        EntityName(entity))
    )

    TOS_AI.SelectPipe(AIGOALPIPE_LOOP, entity, "ai_goto", entity.orderRefEnt.id)
end

function AIBehaviour.DEFAULT:AI_GOTO_STARTED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_GOTO_STARTED> entity: %s", 
        EntityName(entity))
    )
end

function AIBehaviour.DEFAULT:AI_GOTO_ENDED(entity, sender)
    System.LogAlways(string.format("[ORDER] <AI_GOTO_ENDED> entity: %s", 
        EntityName(entity))
    )

    CLEAR_DATA_AI_GOTO(entity)

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