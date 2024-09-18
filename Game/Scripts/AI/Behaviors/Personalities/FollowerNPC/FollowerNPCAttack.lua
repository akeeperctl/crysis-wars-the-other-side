
AIBehaviour.FollowerNPCAttack = {
	Name = "FollowerNPCAttack",
	Base = "FollowerNPCIdle",
	alertness = 2,

	-----------------------------------------------------
	Constructor = function (self, entity)
		--TheOtherSide
		entity.AI.currentBehaviour = self.Name
		--~TheOtherSide	

		entity:MakeAlerted();
		AIBehaviour.FollowerNPCIdle:CheckWeapon(entity);
		AIBehaviour.FollowerNPCIdle:ContinueCombat(entity);
	end,

	-----------------------------------------------------
	Destructor = function(self,entity)
	end,


}
