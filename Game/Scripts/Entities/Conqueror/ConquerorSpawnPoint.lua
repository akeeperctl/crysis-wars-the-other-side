ConquerorSpawnPoint = {
	Client = {},
	Server = {},

	Editor={
		--Icon="SpawnPoint.bmp",
		Model="Editor/Objects/spawnpointhelper.cgf",
		DisplayArrow=1,
	},
	
	type = "ConquerorSpawnPoint",
	Properties=
	{
		bEnabled=1,
		bForAirUnits=0,
	},
}

--------------------------------------------------------------------------
function ConquerorSpawnPoint:ForcedSpawnActor(entityId)
	local actor = System.GetEntity(entityId);
	if (actor) then
		actor:SetWorldPos(self:GetWorldPos(g_Vectors.temp_v1));
		actor:SetWorldAngles(self:GetAngles(g_Vectors.temp_v1));
	end
	--System.LogAlways("[LUA][ConquerorSpawnPoint][OnStartGame]");
end

--------------------------------------------------------------------------
function ConquerorSpawnPoint.Server:OnStartGame()
	self:Enable(tonumber(self.Properties.bEnabled)~=0);	
	--System.LogAlways("[LUA][ConquerorSpawnPoint][OnStartGame]");
end

--------------------------------------------------------------------------
function ConquerorSpawnPoint.Server:OnInit()
	--self:Enable(tonumber(self.Properties.bEnabled)~=0);	
end

----------------------------------------------------------------------------------------------------
function ConquerorSpawnPoint:Enable(enable)
	if (enable) then
		g_gameRules.game:AddSpawnLocation(self.id);
	else
		g_gameRules.game:RemoveSpawnLocation(self.id);
	end
	self.enabled=enable;
end

--------------------------------------------------------------------------
function ConquerorSpawnPoint.Server:OnShutDown()
	if (g_gameRules) then
		g_gameRules.game:RemoveSpawnLocation(self.id);
	end
end

--------------------------------------------------------------------------
function ConquerorSpawnPoint:Spawned(entity)
	BroadcastEvent(self, "Spawn");
end

--------------------------------------------------------------------------
function ConquerorSpawnPoint:IsEnabled()
	return self.enabled;
end

--------------------------------------------------------------------------
-- Event is generated when something is spawned using this spawnpoint
--------------------------------------------------------------------------
function ConquerorSpawnPoint:Event_Spawn()
	local player = g_localActor;
	if (self.enabled) then
		player:SetWorldPos(self:GetWorldPos(g_Vectors.temp_v1));		
		--set angles
		player:SetWorldAngles(self:GetAngles(g_Vectors.temp_v1));
		BroadcastEvent(self, "Spawn");
	end
end

function ConquerorSpawnPoint:Event_Disable()	
	self:Enable(false);	
	BroadcastEvent(self, "Disabled");
end

function ConquerorSpawnPoint:Event_Enable()	
	self:Enable(true);		
	BroadcastEvent(self, "Enabled");
end

ConquerorSpawnPoint.FlowEvents =
{
	Inputs =
	{
		Enable = { ConquerorSpawnPoint.Event_Enable, "bool" },
		Disable = { ConquerorSpawnPoint.Event_Disable, "bool" },
		Spawn = { ConquerorSpawnPoint.Event_Spawn, "bool" },
	},
	Outputs =
	{
		Enabled = "bool",
		Disabled = "bool",
		Spawn = "bool",
	},
}
