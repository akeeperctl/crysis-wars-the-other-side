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

                if (AI.Hostile(executor.id, orderTarget.id)) then
                    AI_PURSUIT_AND_KILL(executor, orderTarget, 40, speed)
                else 
                    AI_FOLLOW_AND_PROTECT(executor, orderTarget, 1.5, speed, 2)
                end
            end
        end
    else
        

        -- Если цели нет, то бежим
        AI_GOTO(executor, orderPosition, speed)

    end
    return outputOrderType
end

----------------------------------------------------------------
function StopOrder(executorId)
    local executor = System.GetEntity(executorId)
    local order = executor.currentExecutedOrder

    if (order == EOrders.AI_ENTERVEHICLE) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_ENTERVEHICLE_ENDED", executor.id)
    elseif (order == EOrders.AI_GOTO) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_GOTO_ENDED", executor.id)
    elseif (order == EOrders.AI_PURSUIT_AND_KILL) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_PURSUIT_AND_KILL_ENDED", executor.id)
    elseif (order == EOrders.AI_FOLLOW_AND_PROTECT) then
        TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "AI_FOLLOW_AND_PROTECT_ENDED", executor.id)
    end

end

----------------------------------------------------------------
function OnOrderComplete(executor)
    Zeus.OnOrderComplete(executor.id)
end

-- Дебаг ниже
----------------------------------------------------------------
function AIBehaviour.DEFAULT:TestSignal(entity, sender, data)
    local entName = "NONE"
    if (entity) then
        entName = entity:GetName()
    end

    System.LogAlways(string.format("[AI_SIGNAL] <TestSignal> entity: %s, sender: %s, data: %s", entName, sender:GetName(), table.dump(data)))
end
