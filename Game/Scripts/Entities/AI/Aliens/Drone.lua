Script.ReloadScript( "SCRIPTS/Entities/AI/Aliens/Drone_x.lua");

CreateAlien(Drone_x);
Drone = CreateAI(Drone_x)
Drone:Expose();