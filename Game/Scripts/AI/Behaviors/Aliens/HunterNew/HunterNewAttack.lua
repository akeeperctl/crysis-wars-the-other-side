--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Implementation of a simple outdoor indoor alien behavior
--  
--------------------------------------------------------------------------
--  History:
--  - 09/05/2005   : Created by Mikko Mononen
--	Sept 2005			: Modified by Luciano Morpurgo
--------------------------------------------------------------------------
AIBehaviour.HunterNewAttack = {
	Name = "HunterNewAttack",
	alertness = 2,

	---------------------------------------------
	Constructor = function( self , entity )
		
		--TheOtherSide
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		if ( entity.AI.PathStep == nil ) then 
			entity.AI.PathStep = 0;
		end
		--self:Move( entity );
		entity:MakeAlerted( false );
		entity:HolsterItem( false );
		entity:SelectPipe( 0, "do_nothing" );
	end,
	---------------------------------------------
	Destructor = function( self , entity )
		entity:InsertSubpipe( 0, "stop_fire" );
	end,

	---------------------------------------------

	OnPlayerSeen = function(self,entity,distance)
	end,
	---------------------------------------------

	OnEnemyMemory = function(self,entity,distance)
	end,
	---------------------------------------------

	OnNoTarget = function(self,entity,sender)
	end,
	---------------------------------------------
	HT_SHOOT = function( self, entity )

		-- AI.LogEvent( "Hunter shoot" );

		-- -- Make sure the target is nice.
		-- local	enemy = AI.GetAttentionTargetEntity(entity.id);
		-- if( not enemy ) then
		-- 	AI.LogEvent( "  - could not find enemy entity" );
		-- 	return;
		-- end		
		-- AI.LogEvent( "  - Hunter SHOOT!" );
		-- entity:DoShootWeapon();
	end,

	---------------------------------------------
	HT_END_LOOKAROUND = function( self, entity )
		--entity:SelectPipe(0,"do_nothing");
	end,	
	---------------------------------------------
	DROP_OBJECT = function(self,entity,sender)
		-- local targetType = AI.GetTargetType(entity.id);
		-- local targetPos = g_Vectors.temp;
		-- AI.GetAttentionTargetPosition(entity.id,targetPos);
		-- if(targetType ~= AITARGET_NONE) then 
		-- 	if(targetType == AITARGET_ENEMY) then 
		-- 		targetPos.z = targetPos.z - 1.65;
		-- 	end
		-- 	entity:DropObjectAtPoint(targetPos);
		-- end
		-- entity:SelectPipe(0,"do_nothing");
		-- AI.SetSmartObjectState(entity.id,"Attack");
		-- self:Move(entity);
	end,

	--------------------------------------------
	
	Move = function( self,entity)	
--		local targetpos  = g_Vectors.temp;
--		AI.GetAttentionTargetPosition(entity.id,targetpos);
--		local anchorname;
--		if(IsNullVector(targetpos)) then
--			anchorname = AI.GetAnchor(entity.id,100,AIAnchorTable.COMBAT_SHOOTSPOTSTAND,AIANCHOR_RANDOM_IN_RANGE);
--		else
--			anchorname = AI.FindObjectOfType(targetpos,100,AIAnchorTable.COMBAT_SHOOTSPOTSTAND);
--		end
--		if(anchorname) then
--			local anchor = System.GetEntityByName(anchorname);
--			if(anchor) then
--				CopyVector(targetpos,anchor:GetPos());
--				AI.SetRefPointPosition(entity.id,targetpos);
--				entity:SelectPipe(0,"approach_refpoint");
--			end
--		end
	
	end,
	--------------------------------------------
	
	MoveOld = function( self,entity)	
		-- TO DO: hunter is not supposed to fly

	--	local targettype = AI.GetTargetType(entity.id);
--		if(AI.Hostile(entity.id,AI.GetAttentionTargetOf(entity.id))) then
--			local targetpos =g_Vectors.temp;
--			AI.GetAttentionTargetPosition(entity.id,targetpos);
--			targetpos.z = targetpos.z + 30;
--			AI.SetRefPointPosition(entity.id,targetpos);
--			entity:SelectPipe(0,"approach_refpoint");
--			entity:InsertSubpipe(0,"do_it_running");
--		else
			-- local name = entity:GetName();
			-- local tpname = name.."_P0";	
			-- local TagPoint = System.GetEntityByName(name.."_P"..entity.AI.PathStep);
			-- if (TagPoint) then 		
			-- 	tpname = name.."_P"..entity.AI.PathStep;
			-- else
			-- 	entity.AI.PathStep = 0;
			-- 	tpname = name.."_P0";
			-- 	local TagPoint = System.GetEntityByName(tpname);
			-- 	if (TagPoint == nil and not AI.Hostile(entity.id,AI.GetAttentionTargetOf(entity.id))) then 		
			-- 		AI.Signal(SIGNALFILTER_SENDER,1,"GO_TO_IDLE",entity.id);
			-- 		return;
			-- 	end
			-- end
			-- entity:SelectPipe(0,"ht_patrol",tpname);
			-- entity.AI.PathStep = entity.AI.PathStep + 1;
--		end
	end,	

--		---------------------------------------------
--	REFPOINT_REACHED = function(self,entity,sender)
--		if(AI.Hostile(entity.id,AI.GetAttentionTargetOf(entity.id))) then
--			entity:SelectPipe(0,"do_nothing");
--		else
--			self:Move(entity);
--		end
--	end,
	
	---------------------------------------------
	HT_NEXT_POINT = function( self,entity, sender )	
		--self:Move(entity);
	end,
--	---------------------------------------------
	OnEnemyDamage = function ( self, entity, sender, data)
		System.LogAlways("HunterAttack: OnEnemyDamage")

		-- first send him OnSeenByEnemy signal
		AI.Signal(SIGNALFILTER_SUPERGROUP, 1, "OnSeenByEnemy", g_localActor.id);

		-- Drop beacon and let the other know here's something to fight for.
		entity:TriggerEvent(AIEVENT_DROPBEACON);
		AI.Signal(SIGNALFILTER_GROUPONLY, 1, "GO_ENEMY_FOUND",entity.id);
		
		entity.AI.lastSeenName = AI.GetAttentionTargetOf(entity.id);

		--local nearestVehicleName = AI.FindObjectOfType(entity.id,30,AIOBJECT_VEHICLE,AIFAF_VISIBLE_TARGET);
		local nearestVehicle = System.GetEntityByName("Asian_ltv11");
		if (nearestVehicle) then		
			local targetPos = {};
			local hunterPos = {};

			AI.GetAttentionTargetPosition(entity.id, targetPos);
			entity:GetWorldPos(hunterPos);
				
			SubVectors(targetPos,targetPos,hunterPos);
			
			local len = LengthVector(targetPos)*1;
			NormalizeVector(targetPos);
			
			targetPos.z = targetPos.z - 0.60;
			targetPos.y = targetPos.y - 0.35;
			NormalizeVector(targetPos);
			
			FastScaleVector(targetPos,targetPos,len);

			entity:SetGrabbingScheme("Right");
			
			

			entity:GrabObject(nearestVehicle,nil);	
			entity:DropObject( true, targetPos, 2 );
			
			System.LogAlways("HunterAttack: SetGrabbingScheme")
		end
	end,	
}
