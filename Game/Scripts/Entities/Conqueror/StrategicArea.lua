StrategicArea = 
{
	type = "StrategicArea",
	Properties = 
	{
		soclasses_areaFlags = "",
		species = -1,
		bCapturable = 1,
		bEnable = 1,
		captureTime = 15,
		captureRequirement = 1,
		bDrawDebugLog = 0,

		ConquerorBuyOptions =
		{
			bEnabled = 1,
			--bVehicles = 0,
			bWeapons = 1,
			--bEquipment = 1,
			--bPrototypes = 1,
			bAmmo = 1,
		},
		
		ConquerorSquad = 
		{
			bCanSpawnSquadFromClasses = 0, --One Squad Per Species. One Leader Class Per Species.
			bCanUnlockClassesForPlayer = 0,
			bCanUnlockClassesForAI = 0,
			membersCount = 3 --members count to spawn
		},

		--bCanUnlockWeapons = 0, -- not implemented yet

		Species_0 = 
		{
			unlockedClass_0 = "", -- One class in the line
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_1 = 
		{
			unlockedClass_0 = "", --Example Leader
			unlockedClass_1 = "", --Example member
			unlockedClass_2 = "", --Example member
			unlockedClass_3 = "", --Example member
		},

		Species_2 = 
		{
			unlockedClass_0 = "",--Example member
			unlockedClass_1 = "",--Example member
			unlockedClass_2 = "",--Example Leader
			unlockedClass_3 = "",--Example member
		},

		Species_3 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_4 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_5 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_6 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_7 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_8 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},

		Species_9 = 
		{
			unlockedClass_0 = "",
			unlockedClass_1 = "",
			unlockedClass_2 = "",
			unlockedClass_3 = "",
		},
	},

	Client = {},
	Server = {},
	
	-- Temp.
	_Flags = {},

	Editor=
	{
		--Model="Editor/Objects/T.cgf",
		Icon="StrategicArea.bmp",
	},
}

function StrategicArea:OnInit()
	self:OnReset();
end

function StrategicArea:OnPropertyChange()
	self:OnReset();
end

function StrategicArea:OnReset()
end

function StrategicArea:OnSave(tbl)
	tbl.Properties.soclasses_areaFlags=self.Properties.soclasses_areaFlags;
	tbl.Properties.species=self.Properties.species;
	tbl.Properties.bCapturable=self.Properties.bCapturable;
	tbl.Properties.bEnable=self.Properties.bEnable;
	tbl.Properties.captureTime=self.Properties.captureTime;
	tbl.Properties.captureRequirement=self.Properties.captureRequirement;
end

function StrategicArea:OnLoad(tbl)
	self.Properties.soclasses_areaFlags=tbl.Properties.soclasses_areaFlags;
	self.Properties.species=tbl.Properties.species;
	self.Properties.bCapturable=tbl.Properties.bCapturable;
	self.Properties.bEnable=tbl.Properties.bEnable;
	self.Properties.captureTime=tbl.Properties.captureTime;
	self.Properties.captureRequirement=tbl.Properties.captureRequirement;
end

function StrategicArea:OnShutDown()
end

function StrategicArea:Event_TargetReached( sender )
end

function StrategicArea:Event_Enable( sender )
end

function StrategicArea:Event_Disable( sender )
end

function StrategicArea:ScriptEvent(event,value,str)
	if (event == "OnCaptured") then
		self:ActivateOutput("OnCaptured", value)
	elseif (event == "OnUncaptured") then
		self:ActivateOutput("OnUncaptured", value)
	elseif (event == "OnNeutral") then
		self:ActivateOutput("OnNeutral", value)
	elseif (event == "OnContested") then
		self:ActivateOutput("OnContested", value)
	end
end

StrategicArea.FlowEvents =
{
	Inputs =
	{

	},
	Outputs =
	{
		OnCaptured = "int",
		OnUncaptured = "int",
		OnNeutral = "int",
		OnContested = "int",
	},
}

--MakeBuyZone(StrategicArea, bor(bor(SinglePlayer.BUY_WEAPON,SinglePlayer.BUY_AMMO), SinglePlayer.BUY_EQUIPMENT));