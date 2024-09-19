-- File loads all TheOtherSide lua files

Script.ReloadScript("Scripts/Utils/TOSCommon.lua")
Script.ReloadScript("Scripts/Utils/TOSTable.lua")
Script.ReloadScript("Scripts/Utils/TOSVehicle.lua")
Script.ReloadScript("Scripts/Utils/TOSAI.lua")

Script.ReloadScript("Scripts/AI/TOS/TOSPipeManager.lua")
PipeManager:OnInitTOS();

-- manual loaded
-- Script.ReloadScript("Scripts/AI/TOS/TOSHandleOrder.lua")