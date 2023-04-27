ConquerorFlag =
{	
	type = "ConquerorFlag",

	Properties =
	{
		objModel = "objects/library/props/flags/mp_flags.cdf",
		species	= -1 --neutral
	},

	Editor=
	{
		--Model="Editor/Objects/T.cgf",
		Icon="Tornado.bmp",
	},
};

MakeUsable(ConquerorFlag);

----------------------------------------------------------------------------------------------------
function ConquerorFlag:OnPreFreeze(freeze, vapor)
	if (freeze) then
		return false; -- don't allow freezing
	end
end

----------------------------------------------------------------------------------------------------
function ConquerorFlag:CanShatter()
	return 0;
end



----------------------------------------------------------------------------------------------------
function ConquerorFlag:LoadGeometry(slot, model)
	if (string.len(model) > 0) then
		local ext = string.lower(string.sub(model, -4));

		if ((ext == ".chr") or (ext == ".cdf") or (ext == ".cga")) then
			self:LoadCharacter(slot, model);
		else
			self:LoadObject(slot, model);
		end
	end
end


-- -------------------------------------------------------------------------
function ConquerorFlag:OnSpawn()
	CryAction.CreateGameObjectForEntity(self.id);
	CryAction.BindGameObjectToNetwork(self.id);
	CryAction.ForceGameObjectUpdate(self.id, true);
	
	self:LoadGeometry(0, self.Properties.objModel);
	self:Physicalize(0, PE_RIGID, { mass=0 });
	self:RedirectAnimationToLayer0(0, true);
	self:Activate(1);
end


-- ----------------------------------------------------------------------------------------------------
function ConquerorFlag:SetSpecies(inputSpecies)
	if (self.speciesIndex~=inputSpecies) then
		local action= "";
		--local species = -1;
		local speed = 1;

		if (self.speciesIndex and self.speciesIndex ~= -1) then
			if (inputSpecies ~= -1) then
				action="up";
				speed=500;
				self:ApplyFlagMaterial(inputSpecies);
			else
				action="down";
			end
		else
			if (inputSpecies ~= -1) then
				action="up";
				self:ApplyFlagMaterial(inputSpecies);
			end
		end

		if (action ~= "") then
			local animation=string.format("mp_flags_black_%s", action);
			self:StartAnimation(0, animation, 0, 0.250, speed, false, false, true);
			self:ForceCharacterUpdate(0, true);

			local time=self:GetAnimationLength(0, animation)*1000/speed;
			time=math.max(0, time-125);
			self:SetTimer(0, time);
		end
		self.speciesIndex=inputSpecies;
	end
end


-- ----------------------------------------------------------------------------------------------------
function ConquerorFlag:OnTimer(timerId, msec)
	if(self.speciesIndex ~= -1) then
		local animation=string.format("mp_flags_black_%s", "loop");
		self:StartAnimation(0, animation, 0, 0.250, 1, true, false, true);
	end
end


-- ----------------------------------------------------------------------------------------------------
function ConquerorFlag:OnReset()
	CryAction.DontSyncPhysics(self.id);
	self.speciesIndex=-1;
	self:ApplyFlagMaterial(-1);
	self:SetSpecies(self.Properties.species);
	--self:StopAnimation(0, 0);
	--self:ReplaceMaterial( -1, "objects/library/props/flags/conqueror_flagpole_aliens.mtl", 0)
	--self:SetMaterial( 0, "objects/library/props/flags/conqueror_flagpole_aliens.mtl")
	--self:SetSlotMaterial(self.id, -1, "objects/library/props/flags/conqueror_flagpole_cell.mtl")
end

function ConquerorFlag:OnInit() 
	self:OnReset() 
end

----------------------------------------------------------------------------------------------------
function ConquerorFlag:OnPropertyChange()
	self:OnReset();
end

function ConquerorFlag:ApplyFlagMaterial(speciesIndex)
	local matName = "objects/library/props/flags/conqueror_flagpole_neutral";

	if (speciesIndex == 0) then
		matName = "objects/library/props/flags/conqueror_flagpole_usa";
	elseif (speciesIndex == 1) then
		matName = "objects/library/props/flags/conqueror_flagpole_nk";
	elseif (speciesIndex == 2) then
		matName = "objects/library/props/flags/conqueror_flagpole_aliens";
	elseif (speciesIndex == 3) then
		matName = "objects/library/props/flags/conqueror_flagpole_cell";
	end

	g_gameRules.game:SetEntityMaterial(0, self.id, matName);--slot, id, name
end

function ConquerorFlag:OnUsed()
	
end

ConquerorFlag.FlowEvents =
{
	Inputs =
	{
		Used = { ConquerorFlag.Event_Used, "bool" },
		EnableUsable = { ConquerorFlag.Event_EnableUsable, "bool" },
		DisableUsable = { ConquerorFlag.Event_DisableUsable, "bool" },
	},
	Outputs =
	{
		Used = "bool",
		EnableUsable = "bool",
		DisableUsable = "bool",
	}
}

