local g_SpawnParams_point = {x=0,y=0,z=0}
local g_SpawnParams_scale = {x=1,y=1,z=1}

g_SpawnParams = {
    name = "",
    class = "",
    archetype = "",
    flags = 0, -- ENTITY_FLAG_NEVER_NETWORK_STATIC ||| ENTITY_FLAG_ON_RADAR ||| ENTITY_FLAG_CASTSHADOW ||| ENTITY_FLAG_UNREMOVABLE ||| ENTITY_FLAG_CLIENT_ONLY ||| ENTITY_FLAG_SERVER_ONLY ||| ENTITY_FLAG_AI_HIDEABLE ||| ENTITY_FLAG_HAS_AI
    position = g_SpawnParams_point,
    orientation = g_SpawnParams_point,
    scale = g_SpawnParams_scale,
    properties = {},
    propertiesInstance = {},

    reset = function (self)
        self.name = ""
        self.class = ""
        self.archetype = ""
        self.flags = 0
        self.position = g_SpawnParams_point
        self.orientation = g_SpawnParams_point
        self.scale = g_SpawnParams_scale
        self.properties = {}
        self.propertiesInstance = {}
    end
}

function LogAlways(fmt, ...)
	System.LogAlways(string.format(fmt, ...));
end