--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

Script.ReloadScript("Scripts/TOSLoader.lua")
Script.ReloadScript("Scripts/TOSDebug.lua")

---Обрабатывает приказ исполнителю
---@param executorTable table данные об исполнителе
---@param orderTable table данные о приказе
---@return integer
function HandleOrder(executorTable, orderTable)
    
    local outputOrderType = 0

    local executor = System.GetEntity(executorTable.entityId)
    if not executor then
        LogError("<HandleOrder> executor not defined")
        return outputOrderType
    end

    local executorName = EntityName(executor)
    local executorIndex = executorTable.index
    local maxExecutorsCount = executorTable.maxCount  

    local executorVehicle = executor.vehicle
    local executorActor = executor.actor
    local executorItem = executor.item

    -- Если в данный момент приказ уже выполняется
    -- Нужно очистить/восстановить то, что в ходе его выполнения было создано/изменено
    local currentExecuted = executor.currentExecutedOrder

    AI.ClearPrimaryHostile(executor.id)

    -- unreserve vehicle seat
    if currentExecuted == EOrders.AI_ENTERVEHICLE then
         CLEAR_DATA_AI_ENTERVEHICLE(executor)
    -- elseif currentExecuted == EOrders.AI_GOTO then
    --     CLEAR_DATA_AI_GOTO(executor)
    end

    local orderPosition = g_Vectors.v000
    CopyVector(orderPosition, orderTable.pos)

    local orderGoalPipeId = orderTable.goalPipeId
    local orderTarget = System.GetEntity(orderTable.targetId)
    local orderTargetName = ""

    if (orderTarget) then
        orderTargetName = orderTarget:GetName()
    end

    local speed = 3 -- бег

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

                AI_ENTERVEHICLE(executor, orderTarget)
            elseif(targetActor) then
                AI_PURSUIT_AND_KILL(executor, orderTarget, 40, speed)
            end
        end
    else

        -- Если цели нет, то бежим
        AI_GOTO(executor, orderPosition, speed)

    end
    return outputOrderType
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

function StopOrder(executorId)
    local executor = System.GetEntity(executorId)
    local order = executor.currentExecutedOrder

    if (order == EOrders.AI_ENTERVEHICLE) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_ENTERVEHICLE_ENDED", executor.id)
    elseif (order == EOrders.AI_GOTO) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_GOTO_ENDED", executor.id)
    end

end

function OnOrderComplete(executor)
    Zeus.OnOrderComplete(executor.id)
end