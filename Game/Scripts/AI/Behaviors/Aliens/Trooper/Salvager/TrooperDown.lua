--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--   Description: reaction on grenade - ingnore everything and run away
--  
--------------------------------------------------------------------------
--  History:
--  - 07.07.27. Denisz Polgar
--------------------------------------------------------------------------



AIBehaviour.TrooperDown = {
	Name = "TrooperDown",
	Base = "Dumb",
	alertness = 0,
	exclusive = 0,

	Constructor = function(self,entity,data)
		
		--TheOtherSide
		entity.AI.previousBehaviour = entity.AI.currentBehaviour
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		--entity:SelectPipe(0,"do_nothing");
		--entity:InsertSubpipe(AIGOALPIPE_SAMEPRIORITY,"stop_fire");
		AI.ModifySmartObjectStates(entity.id,"Busy");
		entity:TriggerEvent(AIEVENT_SLEEP);
		AI.ChangeParameter(entity.id,AIPARAM_GROUPID,-1);
		LogWarning("<%s> down!!!", entity:GetName());
		
		entity:StopSounds();
	end,	
	---------------------------------------------
	Destructor = function(self,entity)
		AI.ModifySmartObjectStates(entity.id,"-Busy");
		entity:Readibility("startup",1,1000);
		
		entity:PlayIdleSound(entity.voiceTable.idle);
	end,
	-------------------------------------------------
}
