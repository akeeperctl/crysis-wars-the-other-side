--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: Defend behavior for Alien Trooper. 
--  
--------------------------------------------------------------------------
--  History:
--  - 4/12/2005     : Created by Luciano Morpurgo
--
--------------------------------------------------------------------------
AIBehaviour.TrooperAttackMelee = {
	Name = "TrooperAttackMelee",
	Base = "TrooperAttack",
	alertness = 2,

	---------------------------------------------
	Constructor = function (self, entity)
		
		--TheOtherSide
		entity.AI.previousBehaviour = entity.AI.currentBehaviour
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) == 0.0 ) then
			return;
		end

		entity:SelectPipe(0,"tr_prepare_melee");
	end,
	
	END_MELEE = function(self,entity,sender)

		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) == 0.0 ) then
			return;
		end

		if(random(1,100) <50) then 
			entity:SelectPipe(0,"tr_melee_backoff");
		end

		if (entity.IsHaveCloak == true and entity.cloaked == 0) then
			entity.Cloak(1);
		end
	end,
		
	OnCloseContact = function(self,entity,sender)
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			local target = AI.GetAttentionTargetEntity(entity.id);
			if(target) then

				if (entity.IsHaveCloak == true and entity.cloaked == 1) then
					entity.Cloak(0);
				end		

				entity.AI.meleeTarget = target;
				local vel = g_Vectors.temp;
				target:GetVelocity(vel);
				local speed = target:GetSpeed();
				if(speed > 1) then
					local y = dotproduct2d( entity:GetDirectionVector(1), vel);
					if(y>=0) then 
						return;
					end
				end
				entity.AI.lastMeleeTime = curTime;
	--			entity:MeleeAttack(target);
	--			entity:SelectPipe(0,"do_nothing");
	--			entity:SelectPipe(0,"tr_melee_timeout");
				--entity:InsertSubpipe(AIGOALPIPE_NOTDUPLICATE,"tr_do_melee");
				AI.Animation(entity.id,AIANIM_SIGNAL,"meleeAttack");

				if (entity.IsHaveCloak == true and entity.cloaked == 0) then
					entity.Cloak(1);
				end
		
			end
		end
	end,
	
	OnBulletRain = function(self,entity,sender)
	
	end,
	
	OnPlayerSeen = function(self,entity,sender)
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) > 0.0 ) then
			entity:SelectPipe(0,"tr_prepare_melee");
		end
	end,
	
	END_MELEE_BACKOFF = function(self,entity,sender)
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) == 0.0 ) then
			return;
		end

		entity:SelectPipe(0,"tr_prepare_melee");
	end,
	
	TR_NORMALATTACK = function(self,entity,sender)
		if ( AI.GetAIParameter( entity.id, AIPARAM_PERCEPTIONSCALE_VISUAL ) == 0.0 ) then
			return;
		end

		if(AI.GetTargetType(entity.id)==AITARGET_ENEMY) then
			entity:SelectPipe(0,"tr_prepare_melee");
		else
			entity:SelectPipe(0,"tr_seek_target");
		end
	end,
}