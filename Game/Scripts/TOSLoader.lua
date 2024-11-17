--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

-- File loads all TheOtherSide lua files except TOSHandleOrder,
-- because its loaded from C++ when click the RMB in zeus

Script.ReloadScript("Scripts/Utils/TOSCommon.lua")
Script.ReloadScript("Scripts/Utils/TOSTable.lua")
Script.ReloadScript("Scripts/Utils/TOSVehicle.lua")
Script.ReloadScript("Scripts/Utils/TOSAI.lua")

Script.ReloadScript("Scripts/AI/TOS/OrderHandlers/OrdersEnum.lua")
Script.ReloadScript("Scripts/AI/TOS/OrderHandlers/AI_GOTO.lua")
Script.ReloadScript("Scripts/AI/TOS/OrderHandlers/AI_ENTERVEHICLE.lua")
Script.ReloadScript("Scripts/AI/TOS/OrderHandlers/AI_PURSUIT_AND_KILL.lua")
Script.ReloadScript("Scripts/AI/TOS/OrderHandlers/AI_FOLLOW_AND_PROTECT.lua")

-- manual loaded
-- Script.ReloadScript("Scripts/AI/TOS/TOSHandleOrder.lua")