--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

Script.ReloadScript("Scripts/TOSLoader.lua")
Script.ReloadScript("Scripts/TOSDebug.lua")

local function CryLogAlways(str, ...)
    System.LogAlways(string.format(str, ...))
end

--[[
    Обрабатывает задачу для сущности.

    executorTable       Таблица исполнителя
    orderTable          Таблица задания
]]
function HandleOrder(executorTable, orderTable)

    local outputOrderType = 0
    local enableLog = false

    local executor = System.GetEntity(executorTable.entityId)
    local executorName = EntityName(executor)
    local executorIndex = executorTable.index
    local maxExecutorsCount = executorTable.maxCount  

    -- System.LogAlways(string.format("[%s].AI = %s", executorName, table.dump(executor.AI)))

    local orderPosition = g_Vectors.v000
    CopyVector(orderPosition, orderTable.pos)

    local orderGoalPipeId = orderTable.goalPipeId
    local orderTarget = System.GetEntity(orderTable.targetId)
    local orderTargetName = ""

    if (orderTarget) then
        orderTargetName = orderTarget:GetName()
    end

    -- Принт приказа
    if (enableLog) then
        local output = string.format("[ORDER] executor(%i): %s, order id: %i, order target: %s, order pos: (%1.f,%1.f,%1.f)",
        executorIndex,
        executor:GetName(),
        orderTable.goalPipeId,
        orderTargetName,
        orderTable.pos.x,
        orderTable.pos.y,
        orderTable.pos.z)

        System.LogAlways(output)
    end

    local executorVehicle = executor.vehicle
    local executorActor = executor.actor
    local executorItem = executor.item

    -- Работа с целью приказа
    if (orderTarget) then
        local targetVehicle = orderTarget.vehicle
        local targetActor = orderTarget.actor
        local targetItem = orderTarget.item
    
        -- Исполнитель актер
        if (executorActor) then
            
            -- Посадка в транспорт
            if (targetVehicle) then
                --output = string.format("[ORDER] target is vehicle = %s", tostring(targetVehicle ~= nil))
                --System.LogAlways(output)

                local seatId = orderTarget:RequestSeat(executor.id) + executorIndex
                local seatPos = orderTarget:GetSeatPos(seatId)
                local maxAlertness = 2
                local seatInstance = orderTarget.Seats[seatId]

                local enterRadius = TOS_Vehicle:GetEnterRadius(orderTarget) / 2

                -- ScriptHandle user, ScriptHandle object, const char* actionName, const float maxAlertness, int actionGoalPipeId, const int combatFlag, const char* luaCallback, const char* desiredGoalName
                --AITracker.ExecuteAIAction(executor.id, orderTarget.id, "conqueror_goto_a0_d0_r3", maxAlertness, orderGoalPipeId, IGNORE_COMBAT_DURING_ACTION, "OnActionEnd", "")

                local output = string.format("[ORDER] executor(%i) %s enter vehicle %s on seat %i(%i)",
                    executorIndex,
                    executorName,
                    orderTargetName,
                    seatId,
                    seatId - executorIndex)

                System.LogAlways(output)
            end
        end
    else
        -- Если цели нет, то бежим
        AI_GOTO(executor, orderPosition, 3)

    end
    return outputOrderType
end

function AI_GOTO(entity, position, speed)
    if (not entity) then
        CryLogAlways("<AI_GOTO>: entity not found")
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

    TOS_AI.BeginGoalPipe("ai_goto")
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_GOTO_STARTED", SIGNALFILTER_SENDER)

        if TOS_AI.GetTargetType(entity.id) == AITARGET_ENEMY then
            TOS_AI.PushGoal(GO.STRAFE, 1, 5, 5)
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_BURST_DRAWFIRE)
        else
            TOS_AI.PushGoal(GO.FIRECMD, 1, FIREMODE_OFF)
        end

        TOS_AI.PushGoal(GO.BODYPOS, 1, BODYPOS_STAND)
        TOS_AI.PushGoal(GO.RUN, 1, speed)
        TOS_AI.PushGoal(GO.STICK, 1, 1, AILASTOPRES_USE + AI_CONSTANT_SPEED + AI_REQUEST_PARTIAL_PATH, STICK_BREAK, 5)
        TOS_AI.PushGoal(GO.SIGNAL, 1, AISIGNAL_DEFAULT, "AI_GOTO_ENDED", SIGNALFILTER_SENDER)
    TOS_AI.EndGoalPipe()

    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOSSHARED", entity.id)
    TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_PROCESS_NEXT_UPDATE, "START_AI_GOTO", entity.id)
end

function AITracker:OnActionEnd(data)
    System.LogAlways(string.format("OnActionEnd data %s", table.dump(data)))
end

function AIBehaviour.DEFAULT:START_AI_GOTO(entity, sender, data)
    System.LogAlways(string.format("[ORDER] <START_AI_GOTO> entity: %s, sender: %s, data: %s", 
        EntityName(entity), 
        EntityName(sender), 
        table.safedump(data))
    )

    TOS_AI.SelectPipe(AIGOALPIPE_LOOP, entity, "ai_goto", entity.orderRefEnt.id)
end

function AIBehaviour.DEFAULT:AI_GOTO_STARTED(entity, sender, data)
    System.LogAlways(string.format("[ORDER] <AI_GOTO_START> entity: %s, sender: %s, data: %s", 
        EntityName(entity), 
        EntityName(sender), 
        table.safedump(data))
    )

    -- entity.AI.ignoreSignals = true
end

function AIBehaviour.DEFAULT:AI_GOTO_ENDED(entity, sender, data)
    System.LogAlways(string.format("[ORDER] <AI_GOTO_END> entity: %s, sender: %s, data: %s", 
        EntityName(entity), 
        EntityName(sender), 
        table.safedump(data))
    )

    -- entity.AI.ignoreSignals = false

    if (entity.orderRefEnt) then
        System.RemoveEntity(entity.orderRefEnt.id)
    end

    local targetType = TOS_AI.GetTargetType(entity.id)
    if targetType == AITARGET_ENEMY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_ATTACK_FORCED", entity.id)
    elseif targetType == AITARGET_MEMORY then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_SEARCH_FORCED", entity.id)
    else
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_IDLE_FORCED", entity.id)
    end
    
    TOS_AI.SelectPipe(AIGOALPIPE_SAMEPRIORITY, entity, "do_it_standing")
    AI_Utils:CommonContinueAfterReaction(entity);
end

-- ИИ Перестает реагировать на сигналы, но враги его игнорировать не будут
-- function DISABLE_COMBAT(entity, returnToFirst)
--     LogAlways("[ORDER] <DISABLE_COMBAT> entity: %s, returnToFirst: %s", 
--         EntityName(entity), 
--         tostring(returnToFirst)
--     )

--     -- return to first?
--     if (returnToFirst and returnToFirst == true) then
--         entity:CancelSubpipe()
--         TOS_AI.InsertSubpipe(AIGOALPIPE_SAMEPRIORITY, entity, "devalue_target");
--         TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "RETURN_TO_FIRST", entity.id)
--         TOS_AI.SelectPipe(AIGOALPIPE_LOOP, entity, "do_nothing", 0, 0, true)

--         if entity.vehicle then
--             TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_IDLE", entity.id)
--         else
--             TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_IDLE", entity.id)
--         end
--     end

--     entity.AI.lastCombatClass = TOS_AI.GetAIParameter(entity, AIPARAM_COMBATCLASS)
--     -- entity.AI.ignoreSignals = true
--     TOS_AI.ChangeParameter(entity, AIPARAM_COMBATCLASS, AICombatClasses.Ignore)
--     TOS_AI.ChangeParameter(entity, AIPARAM_PERCEPTIONSCALE_AUDIO, 0)
--     TOS_AI.ChangeParameter(entity, AIPARAM_PERCEPTIONSCALE_VISUAL, 0)
-- end

-- function ENABLE_COMBAT(entity)
--     System.LogAlways(string.format("[ORDER] <ENABLE_COMBAT> entity: %s", 
--         EntityName(entity)
--     ))

--     --entity.AI.ignoreSignals = false
--     if (entity.AI.lastCombatClass ~= nil) then
--         TOS_AI.ChangeParameter(entity, AIPARAM_COMBATCLASS, entity.AI.lastCombatClass)
--     end

--     TOS_AI.ChangeParameter(entity, AIPARAM_PERCEPTIONSCALE_AUDIO, 1)
--     TOS_AI.ChangeParameter(entity, AIPARAM_PERCEPTIONSCALE_VISUAL, 1)
-- end

-- Дебаг ниже
----------------------------------------------------------------
function AIBehaviour.DEFAULT:TestSignal(entity, sender, data)
    local entName = "NONE"
    if (entity) then
        entName = entity:GetName()
    end

    System.LogAlways(string.format("[AI_SIGNAL] <TestSignal> entity: %s, sender: %s, data: %s", entName, sender:GetName(), table.dump(data)))
end