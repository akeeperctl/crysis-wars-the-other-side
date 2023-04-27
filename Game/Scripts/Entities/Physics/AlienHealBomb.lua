Script.ReloadScript("scripts/Utils/EntityUtils.lua")

-- Basic entity
AlienHealBomb = {
	type = "AlienHealBomb",
	Properties = {
		soclasses_SmartObjectClass = "",
		-- bAutoGenAIHidePts = 0,
		
		Particle = "expansion_fx_tos.weapons.heal_grenade",
		object_Model = "objects/library/alien/level_specific_elements/maintenance/sewers/trooper_battery.cgf",
		Physics = {
			bPhysicalize = 1, -- True if object should be physicalized at all.
			--bRigidBody = 1, -- True if rigid body, False if static.
		
			Density = 1000,
			Mass = 200,
		},
		
		-- bFreezable=1,
		bCanShatter=1,
	},

	Client = {},
	Server = {},
	
	-- Temp.
	_Flags = {},
}

function AlienHealBomb:OnInit()
	self:OnReset();
end

function AlienHealBomb:OnPropertyChange()
	self:OnReset();
end

function AlienHealBomb:OnReset()
end

function AlienHealBomb:OnSave(tbl)
end

function AlienHealBomb:OnLoad(tbl)
end

function AlienHealBomb:OnShutDown()
end

function AlienHealBomb:Event_TargetReached( sender )
end

function AlienHealBomb:Event_Enable( sender )
end

function AlienHealBomb:Event_Disable( sender )
end

-- local Physics_DX9MP_Simple = {
		
-- 	Density = 1000,
-- 	Mass = 200,
-- }

-- local BOMB_HEALING_TIMER = 1000;

-- MakeUsable(AlienHealBomb);
-- -- MakePickable(AlienHealBomb);

-- function AlienHealBomb:OnUpdate(frametime)

-- 	System.LogAlways("OnUpdate")

-- 	local vUp = self:GetDirectionVector(2);
-- 	local vSrc = self:GetWorldPos();
-- 	local slotNum;

-- 	FastScaleVector( vUp, vUp, 5.0 );
-- 	FastScaleVector( vDown, vUp, -5000.0 );
-- 	-- FastScaleVector( vDir, vDown, 2.0 );
-- 	-- FastSumVectors( vSrc, vSrc, vUp );

-- 	if (self.bPlanted == false) then
-- 		local hits = Physics.RayWorldIntersection(vSrc,vDown,1,ent_terrain+ent_static,self.id,nil,g_HitTable);
-- 		if (hits > 1) then
-- 			local firstHit = g_HitTable[1];
-- 			if( firstHit.dist < 0.03) then
-- 				slotNum=self:LoadParticleEffect( -1, "expansion_fx_tos.weapons.heal_grenade",{});
-- 				self.bPlanted = true;
-- 			end
-- 		end
-- 	end
-- end

-- function AlienHealBomb:OnTimer(timerId,mSec)
-- 	if (timerId == self.BOMB_HEALING_TIMER) then
-- 		System.LogAlways("OnTimer")
-- 	end
-- end

-- function AlienHealBomb:OnShutDown()
-- end

-- function AlienHealBomb:OnInit()
-- 	self:SetFromProperties();
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:OnSpawn()
-- 	self:SetFromProperties();
-- end


-- ----------------------------------------------------------------------------------------------------
-- -- function AlienHealBomb:OnPreFreeze(freeze, vapor)
-- -- 	if (freeze) then
-- -- 		return self.freezable;
-- -- 	end
-- -- end

-- ----------------------------------------------------------------------------------------------------
-- function AlienHealBomb:GetParticle()
-- 	return self.Properties.Particle;
-- end

-- ----------------------------------------------------------------------------------------------------
-- function AlienHealBomb:CanShatter()
-- 	return self.Properties.bCanShatter;
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:SetFromProperties()
-- 	local Properties = self.Properties;

-- 	if (Properties.object_Model == "") then
-- 		do return end;
-- 	end
	
-- 	-- self.freezable=tonumber(Properties.bFreezable)~=0;
	
-- 	self:LoadObject(0,Properties.object_Model);
-- 	-- --self:CharacterUpdateOnRender(0,1); -- If it is a character force it to update on render.
	
-- 	-- if (Properties.object_ModelFrozen ~= "") then
--  	-- 	self.frozenModelSlot = self:LoadObject(-1, Properties.object_ModelFrozen);
--  	-- 	self:DrawSlot(self.frozenModelSlot, 0);
--  	-- else
--   	-- self.frozenModelSlot = nil;
--     -- end
	
-- 	if (Properties.Physics.bPhysicalize == 1) then
-- 		self:PhysicalizeThis();
-- 	end

-- 	self:SetViewDistRatio(350);

-- 	-- Mark AI hideable flag.
-- 	-- if (Properties.bAutoGenAIHidePts == 1) then
-- 	-- 	self:SetFlags(ENTITY_FLAG_AI_HIDEABLE, 0); -- set
-- 	-- else
-- 	-- 	self:SetFlags(ENTITY_FLAG_AI_HIDEABLE, 2); -- remove
-- 	-- end
	
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:IsRigidBody()

-- 	local Properties = self.Properties;
-- 	local Mass = Properties.Mass; 
--   local Density = Properties.Density;
--   if (Mass == 0 or Density == 0 or Properties.bPhysicalize ~= 1) then 
--   	return false;
--   end
--   return true;
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:PhysicalizeThis()
-- 	-- -- Init physics.
-- 	-- local Physics = self.Properties.Physics;
-- 	-- if (CryAction.IsImmersivenessEnabled() == 0) then
-- 	-- 	Physics = Physics_DX9MP_Simple;
-- 	-- end
-- 	-- EntityCommon.PhysicalizeRigid( self,0,Physics,true );
-- end

-- ------------------------------------------------------------------------------------------------------
-- -- OnPropertyChange called only by the editor.
-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:OnPropertyChange()
-- 	self:SetFromProperties();
-- end

-- ------------------------------------------------------------------------------------------------------
-- -- OnReset called only by the editor.
-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:OnReset()
-- 	self:ResetOnUsed()
	
-- 	local PhysProps = self.Properties.Physics;
-- 	if (PhysProps.bPhysicalize == 1) then
-- 		self:PhysicalizeThis();
-- 		self:AwakePhysics(0);
-- 	end
-- end

-- ------------------------------------------------------------------------------------------------------
-- -- function AlienHealBomb:GetFrozenSlot()
-- -- 	if (self.frozenModelSlot) then
-- -- 		return self.frozenModelSlot;
-- -- 	end
-- -- 	return -1;
-- -- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:Event_Remove()
-- --	self:Hide(1);
-- 	self:DrawSlot(0,0);
-- 	self:DestroyPhysics();
-- 	self:ActivateOutput( "Remove", true );
-- end


-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:Event_Hide()
-- 	self:Hide(1);
-- 	self:ActivateOutput( "Hide", true );
-- 	--self:DrawObject(0,0);
-- 	--self:DestroyPhysics();
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:Event_UnHide()
-- 	self:Hide(0);
-- 	self:ActivateOutput( "UnHide", true );
-- 	--self:DrawObject(0,1);
-- 	--self:SetPhysicsProperties( 1,self.bRigidBodyActive );
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb:Event_RagDollize()  
-- 	self:RagDollize(0);
-- 	self:ActivateOutput( "RagDollized", true );
-- end

-- ------------------------------------------------------------------------------------------------------
-- function AlienHealBomb.Client:OnPhysicsBreak( vPos,nPartId,nOtherPartId )
-- 	self:ActivateOutput("Break",nPartId+1 );
-- end

-- ------------------------------------------------------------------------------------------------------
-- --function AlienHealBomb:OnDamage( hit )
-- --	if( hit.ipart and hit.ipart>=0 ) then
-- --		self:AddImpulse( hit.ipart, hit.pos, hit.dir, hit.impact_force_mul );
-- --	end
-- --end

-- function AlienHealBomb:IsUsable(user)
-- 	local ret = nil
-- 	-- From EntityUtils.lua
-- 	if not self.__usable then self.__usable = self.Properties.bUsable end
	
-- 	-- local mp = System.IsMultiplayer();
-- 	-- if(mp and mp~=0) then
-- 	-- 	return 0;
-- 	-- end

-- 	if (self.__usable == 1) then
-- 		ret = 2
-- 	else
-- 		local PhysProps = self.Properties.Physics;
-- 		if (self:IsRigidBody() == true and user and user.CanGrabObject) then
-- 			ret = user:CanGrabObject(self)
-- 		end
-- 	end
		
-- 	return ret or 0
-- end

-- AlienHealBomb.FlowEvents =
-- {
-- 	Inputs =
-- 	{
-- 		Used = { AlienHealBomb.Event_Used, "bool" },
-- 		EnableUsable = { AlienHealBomb.Event_EnableUsable, "bool" },
-- 		DisableUsable = { AlienHealBomb.Event_DisableUsable, "bool" },

-- 		Hide = { AlienHealBomb.Event_Hide, "bool" },
-- 		UnHide = { AlienHealBomb.Event_UnHide, "bool" },
-- 		Remove = { AlienHealBomb.Event_Remove, "bool" },
-- 		RagDollize = { AlienHealBomb.Event_RagDollize, "bool" },
-- 	},
-- 	Outputs =
-- 	{
-- 		Used = "bool",
-- 		EnableUsable = "bool",
-- 		DisableUsable = "bool",
-- 		Activate = "bool",
-- 		Hide = "bool",
-- 		UnHide = "bool",
-- 		Remove = "bool",
-- 		RagDollized = "bool",		
-- 		Break = "int",
-- 	},
-- }