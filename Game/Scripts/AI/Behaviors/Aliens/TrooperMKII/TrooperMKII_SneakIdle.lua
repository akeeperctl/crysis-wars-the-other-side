--------------------------------------------------
-- SneakerIdle
--------------------------
--   created: Mikko Mononen 21-6-2006
--   created: Integrated to Trooper AI by Denisz Polgár 31.01.2008.


AIBehaviour.TrooperMKII_SneakIdle = {
	Name = "TrooperMKII_SneakIdle",
	Base = "TrooperMKIIIdle",		

	---------------------------------------------
	Constructor = function(self,entity)
		AIBehaviour.TrooperMKIIIdle:Constructor( entity );
	end,	

	Destructor = function(self,entity)
		AIBehaviour.TrooperMKIIIdle:Destructor( entity );
	end,
}
