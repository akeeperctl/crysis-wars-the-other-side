--------------------------------------------------------------------------
--
-- Description :		Синхронизатор event'ов между trackView и FlowGraph
--
--------------------------------------------------------------------------
--  History:
--  17.03.2020   11:15 : Created by Kayriat
--------------------------------------------------------------------------
TrackViewSync = {

	Properties = {
		bEnabled=1,
	},

	Editor={
		Model="Editor/Objects/T.cgf",
		Icon="Target.bmp",
	},

	Enabled=1,
}

function TrackViewSync:OnPropertyChange()
	self.Enabled = self.Properties.bEnabled;
end

function TrackViewSync:OnInit()
	self:OnPropertyChange();
end

function TrackViewSync:OnSave(props)
	props.Enabled = self.Enabled;	
end


function TrackViewSync:OnLoad(props)
	self.Enabled = props.Enabled;	
end

function TrackViewSync:OnReset()
	self:OnPropertyChange();
end

function TrackViewSync:OnShutDown()
end

function TrackViewSync:Event_Sync1( sender )
	
	if (self.Enabled ~= 1) then
		return	
	end
	
	-- if(not g_gameRules.game:GetControlSystemEnabled())then
	-- 	--System.LogAlways("TheOtherSide not enabled");
	-- else 
	-- 	local EntityId = g_gameRules.game:GetControlActorId();
	-- 	--System.LogAlways("TheOtherSide enabled at entityId %f"..EntityId);
	-- end

	BroadcastEvent( self,"Sync1" );
end

function TrackViewSync:Event_Sync2( sender )

	if (self.Enabled ~= 1) then
		return
	end

	BroadcastEvent( self,"Sync2" );
end

function TrackViewSync:Event_Sync3( sender )

	if (self.Enabled ~= 1) then
		return
	end

	BroadcastEvent( self,"Sync3" );
end

function TrackViewSync:Event_Sync4( sender )

	if (self.Enabled ~= 1) then
		return
	end

	BroadcastEvent( self,"Sync4" );
end

function TrackViewSync:Event_Sync5( sender )

	if (self.Enabled ~= 1) then
		return
	end

	BroadcastEvent( self,"Sync5" );
end

function TrackViewSync:Event_Sync6( sender )

	if (self.Enabled ~= 1) then
		return
	end

	BroadcastEvent( self,"Sync6" );
end

function TrackViewSync:Event_Enable( sender )
	self.Enabled = 1;
	BroadcastEvent( self,"Enable" );
end

function TrackViewSync:Event_Disable( sender )
	self.Enabled = 0;
	BroadcastEvent( self,"Disable" );
end

TrackViewSync.FlowEvents =
{
	Inputs =
	{
		Disable = { TrackViewSync.Event_Disable, "bool" },
		Enable = { TrackViewSync.Event_Enable, "bool" },
		Sync1 = { TrackViewSync.Event_Sync1, "bool" },
		Sync2 = { TrackViewSync.Event_Sync2, "bool" },
		Sync3 = { TrackViewSync.Event_Sync3, "bool" },
		Sync4 = { TrackViewSync.Event_Sync4, "bool" },
		Sync5 = { TrackViewSync.Event_Sync5, "bool" },
		Sync6 = { TrackViewSync.Event_Sync6, "bool" },
	},
	Outputs =
	{
		Disable = "bool",
		Enable = "bool",
		Sync1 = "bool",
		Sync2 = "bool",
		Sync3 = "bool",
		Sync4 = "bool",
		Sync5 = "bool",
		Sync6 = "bool",
	},
}
