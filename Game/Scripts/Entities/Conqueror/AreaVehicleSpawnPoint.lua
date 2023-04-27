AreaVehicleSpawnPoint = 
{
	type = "AreaVehicleSpawnPoint",
	Properties = 
	{
		bEnable = 1,
		bMarkDetached = 0, -- The one who enters in this transport becomes detached squad member/leader

		-- Species_0 = 
		-- {
		-- 	vehicle_0 = "vehicle_archetype1",
		-- 	vehicle_1 = "vehicle_archetype2",
		-- 	vehicle_2 = "vehicle_archetype3",
		-- 	vehicle_3 = "vehicle_archetype4",
		-- },

		Species_0 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_1 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_2 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_3 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_4 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_5 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_6 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_7 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_8 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
		Species_9 = 
		{
			vehicle_0 = "",
			vehicle_1 = "",
			vehicle_2 = "",
			vehicle_3 = "",
		},
	},

	Client = {},
	Server = {},
	
	-- Temp.
	_Flags = {},

	Editor=
	{
		Model="Editor/Objects/vehiclespawn.cgf",
		--Icon="Tornado.bmp",
	},
}

function AreaVehicleSpawnPoint:OnInit()
	self:OnReset();
end

function AreaVehicleSpawnPoint:OnPropertyChange()
	self:OnReset();
end

function AreaVehicleSpawnPoint:OnReset()
end

function AreaVehicleSpawnPoint:OnSave(tbl)
end

function AreaVehicleSpawnPoint:OnLoad(tbl)
end

function AreaVehicleSpawnPoint:OnShutDown()
end

function AreaVehicleSpawnPoint:ScriptEvent(event,value,str)
	-- if (event == "OnCaptured") then
	-- 	self:ActivateOutput("OnCaptured", value)
	-- elseif (event == "OnUncaptured") then
	-- 	self:ActivateOutput("OnUncaptured", value)
	-- elseif (event == "OnNeutral") then
	-- 	self:ActivateOutput("OnNeutral", value)
	-- elseif (event == "OnContested") then
	-- 	self:ActivateOutput("OnContested", value)
	-- end
end

AreaVehicleSpawnPoint.FlowEvents =
{
	Inputs =
	{

	},
	Outputs =
	{

	},
}