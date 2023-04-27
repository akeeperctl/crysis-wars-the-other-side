Script.ReloadScript( "SCRIPTS/Entities/AI/Aliens/Pinger_x.lua");

CreateAlien(Pinger_x);
Pinger = CreateAI(Pinger_x)
Pinger:Expose();