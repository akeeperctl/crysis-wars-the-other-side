--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

local g_SpawnParams_point = {x=0,y=0,z=0}
local g_SpawnParams_rot = {x=0,y=-1,z=0};
local g_SpawnParams_scale = {x=1,y=1,z=1}

g_SpawnParams = {
    name = "",
    class = "",
    archetype = "",
    flags = 0, -- ENTITY_FLAG_NEVER_NETWORK_STATIC ||| ENTITY_FLAG_ON_RADAR ||| ENTITY_FLAG_CASTSHADOW ||| ENTITY_FLAG_UNREMOVABLE ||| ENTITY_FLAG_CLIENT_ONLY ||| ENTITY_FLAG_SERVER_ONLY ||| ENTITY_FLAG_AI_HIDEABLE ||| ENTITY_FLAG_HAS_AI
    position = g_SpawnParams_point,
    orientation = g_SpawnParams_rot,
    scale = g_SpawnParams_scale,
    properties = {},
    propertiesInstance = {},

    reset = function (self)
        self.name = ""
        self.class = ""
        self.archetype = ""
        self.flags = 0
        self.position = g_SpawnParams_point
        self.orientation = g_SpawnParams_rot
        self.scale = g_SpawnParams_scale
        self.properties = {}
        self.propertiesInstance = {}
    end
}

function LogAlways(fmt, ...)
	System.LogAlways(string.format(fmt, ...));
end

function __FILE__() return debug.getinfo(2, 'S').source end
function __LINE__() return debug.getinfo(2, 'l').currentline end
function __FUNC__() return debug.getinfo(2, 'n').name end