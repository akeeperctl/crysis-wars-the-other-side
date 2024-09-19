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
    executor.order = g_OrderData
    local executorName = executor:GetName()
    local executorIndex = executorTable.index
    local maxExecutorsCount = executorTable.maxCount

    local orderPosition = orderTable.pos
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
                local maxAlertness = 3
                local seatInstance = orderTarget.Seats[seatId]

                local enterRadius = TOS_Vehicle:GetEnterRadius(orderTarget) / 2
                local endAccuracy = 0
                local distanceVariation = 0

                TOS_AI.BeginGoalPipe("goto_enter_vehicle")
                    TOS_AI.PushGoal(GO.LOCATE, 1, orderTargetName)
                    TOS_AI.PushGoal(GO.ACQUIRE_TARGET, 1, orderTargetName)
                    TOS_AI.PushGoal(GO.BRANCH, 1, "MOVING", IF_TARGET_DIST_GREATER, enterRadius)

                    TOS_AI.PushLabel("MOVING")
                    TOS_AI.PushGoal(GO.BODYPOS, 0, STANCE_STAND)
                    TOS_AI.PushGoal(GO.RUN, 0, 5)
                    TOS_AI.PushGoal(GO.STICK, 1, enterRadius, AILASTOPRES_USE + AI_DONT_STEER_AROUND_TARGET + AI_CONSTANT_SPEED, STICK_BREAK, endAccuracy, distanceVariation);
                    TOS_AI.PushGoal(GO.BRANCH, 1, "STOP", IF_TARGET_DIST_LESS, enterRadius)

                    TOS_AI.PushLabel("STOP")
                    TOS_AI.PushGoal(GO.SIGNAL, 1, 1, "TOS_CALL_ORDER_FUNC", SIGNALFILTER_SENDER)
                    --TOS_AI.PushGoal(GO.SIGNAL, 1, 1, "TOS_ORDER_ENTER_VEHICLE", SIGNALFILTER_SENDER)
                    --TOS_AI.PushGoal(GO.SIGNAL, 1, 1, "TOS_ORDER_END_CURRENT", SIGNALFILTER_SENDER)

                    TOS_AI.PushLabel("NO_TARGET")
                    TOS_AI.PushGoal("do_nothing", 1)             
                TOS_AI.EndGoalPipe()

                TOS_AI.BeginGoalPipe("test")
                    TOS_AI.PushGoal(GO.IGNOREALL, 1, 1)
                    TOS_AI.PushGoal(GO.SIGNAL, 1, 1, "TestSignal", SIGNALFILTER_SENDER, 5555)
                TOS_AI.EndGoalPipe()

                local fast = true;
                executor.order.funcName = "ENTER_VEHICLE"
                executor.order.funcParams = {orderTarget.id, executor.id, seatId, fast}

                -- AI.SetRefPointPosition(executor.id, seatPos)
                TOS_AI.InsertSubpipe(executor, orderGoalPipeId, "goto_enter_vehicle", 0, 0)
                --TOS_AI.InsertSubpipe(executor, orderGoalPipeId, "test", 0, 0)

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
        

    end

    -- output = string.format("[ORDER] target is actor = %s", tostring(targetActor ~= nil))
    -- System.LogAlways(output)

    -- output = string.format("[ORDER] target is item = %s", tostring(targetItem ~= nil))
    -- System.LogAlways(output)

    --g_SignalData.iValue = orderGoalPipeId
    --TOS_AI.SendSignal(SIGNALFILTER_SENDER, 100, "TOS_ORDER_END_CURRENT", executor.id, g_SignalData)

    return outputOrderType
end



-- function AIBehaviour.DEFAULT:TOS_CALL_ORDER_FUNC(entity, sender, data)

--     local order = sender.order

--     g_Functions[order.funcName](unpack(order.funcParams))
--     --System.LogAlways(string.format("[ORDER] frame<%i> EXECUTING '%s'", System.GetFrameID(), order.funcName))
-- end

function AIBehaviour.DEFAULT:TOS_EXECUTE_ACTION(entity, sender, data)
    local userId = sender.id

    local actionId = g_ActionData.actionId
    local goalPipeId = g_ActionData.goalPipeId
    local maxAlertness = g_ActionData.maxAlertness
    local objectId = g_ActionData.objectId
    local userId = g_ActionData.userId

    if (not objectId) then
        objectId = 0
    end

    System.LogAlways(string.format("[TOS_EXECUTE_ACTION] frame<%i> actionId: %s, objectId: %s", 
        System.GetFrameID(), 
        actionId, 
        tostring(objectId)))

    TOS_AI.ExecuteAction(actionId, userId, objectId, maxAlertness, goalPipeId)

    g_ActionData = {}
end

function AIBehaviour.DEFAULT:TOS_ORDER_END_CURRENT(entity, sender, data)
    System.LogAlways(string.format("[ORDER] frame<%i> %i END", System.GetFrameID(), data.iValue))
end

function AIBehaviour.DEFAULT:TestSignal(entity, sender, data)
    local entName = "NONE"
    if (entity) then
        entName = entity:GetName()
    end

    System.LogAlways(string.format("[TESTSIGNAL] entity: %s, sender: %s, data: %s", entName, sender:GetName(), dump_table(data)))
end