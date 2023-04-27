--------------------------------------------------------------------------
--	Crytek Source File.
-- 	Copyright (C), Crytek Studios, 2001-2004.
--------------------------------------------------------------------------
--	$Id$
--	$DateTime$
--	Description: GameRules implementation for Death Match
--  
--------------------------------------------------------------------------
--  History:
--  - 22/ 9/2004   16:20 : Created by Mathieu Pinard
--  - 04/10/2004   10:43 : Modified by Craig Tiller
--  - 07/10/2004   16:02 : Modified by Marcio Martins
--
--------------------------------------------------------------------------
SinglePlayer = {
	DamagePlayerToAI =
	{
		helmet		= 4.0,
		kevlar		= 0.45,

		head 			= 20.0,
		torso 		= 1.4,
		arm_left	= 0.65,
		arm_right	= 0.65,
		hand_left	= 0.3,
		hand_right= 0.3,
		leg_left	= 0.65,
		leg_right	= 0.65,
		foot_left	= 0.3,
		foot_right= 0.3,
		assist_min	=0.8,
	},
	
	DamagePlayerToPlayer=
	{
		helmet		= 4.0,
		kevlar		= 0.45,

		head 			= 20.0,
		torso 		= 1.4,
		arm_left	= 0.65,
		arm_right	= 0.65,
		hand_left	= 0.3,
		hand_right= 0.3,
		leg_left	= 0.65,
		leg_right	= 0.65,
		foot_left	= 0.3,
		foot_right= 0.3,
		assist_min	=0.8,
	},
	
	DamageAIToPlayer=
	{
		helmet		= 4.0,
		kevlar		= 0.45,

		head 			= 20.0,
		torso 		= 1.4,
		arm_left	= 0.65,
		arm_right	= 0.65,
		hand_left	= 0.3,
		hand_right= 0.3,
		leg_left	= 0.65,
		leg_right	= 0.65,
		foot_left	= 0.3,
		foot_right= 0.3,
		assist_min	=0.8,
	},
	
	DamageAIToAI=
	{
		helmet		= 4.0,
		kevlar		= 0.45,

		head 			= 20.0,
		torso 		= 1.4,
		arm_left	= 0.65,
		arm_right	= 0.65,
		hand_left	= 0.3,
		hand_right= 0.3,
		leg_left	= 0.65,
		leg_right	= 0.65,
		foot_left	= 0.3,
		foot_right= 0.3,
		assist_min	=0.8,
	},
	

	tempVec = {x=0,y=0,z=0},
	playerDeathLocations = {},
	lastSaveName = "",
	lastSaveDeathCount = 0,
	hudWhite = { x=1, y=1, z=1},

	Client = {},
	Server = {},
	
	-- this table is used to track the available entities where we can spawn the
	-- player
	spawns = {},

	--TheOtherSide
	works={};

	SCORE_KILLS_KEY 		= 100;
	SCORE_DEATHS_KEY 		= 101;
	SCORE_HEADSHOTS_KEY 	= 102;
	SCORE_PING_KEY 			= 103;
	SCORE_LAST_KEY 			= 104;	-- make sure this is always the last one
	--~TheOtherSide
}
--TheOtherSide
Net.Expose {
	Class = SinglePlayer,
	ClientMethods = {
		--ClSetupPlayer					= { RELIABLE_UNORDERED, NO_ATTACH, DEPENTITYID, },
		--ClSetSpawnGroup	 			= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, },
		--ClSetPlayerSpawnGroup	= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, ENTITYID },
		--ClSpawnGroupInvalid		= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, },
		--ClVictory							= { RELIABLE_ORDERED, POST_ATTACH, ENTITYID, },
		--ClNoWinner						= { RELIABLE_ORDERED, POST_ATTACH, },
		
		ClStartWorking				= { RELIABLE_ORDERED, POST_ATTACH, ENTITYID; STRINGTABLE },
		ClStepWorking				= { RELIABLE_ORDERED, POST_ATTACH, INT8 },
		ClStopWorking				= { RELIABLE_ORDERED, POST_ATTACH, ENTITYID, BOOL },
		ClWorkComplete				= { RELIABLE_ORDERED, POST_ATTACH, ENTITYID, STRINGTABLE },
		ClPP						= { RELIABLE_UNORDERED, POST_ATTACH, FLOAT, BOOL },

		--ClClientConnect			= { RELIABLE_UNORDERED, POST_ATTACH, STRING, BOOL },
		--ClClientDisconnect		= { RELIABLE_UNORDERED, POST_ATTACH, STRING, },
		--ClClientEnteredGame		= { RELIABLE_UNORDERED, POST_ATTACH, STRING, },
		--ClTimerAlert					= { RELIABLE_UNORDERED, POST_ATTACH, INT8 },
	},
	ServerMethods = {
		--RequestRevive		 			= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, },
		--RequestSpawnGroup			= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, ENTITYID },
		--RequestSpectatorTarget= { RELIABLE_UNORDERED, POST_ATTACH, ENTITYID, INT8 },
	},
	ServerProperties = {
	},
};

-- pp values
SinglePlayer.ppList=
{
	SPAWN										= 100,
	START										= 100,
	KILL										= 100,
	KILL_RANKDIFF_MULT			= 10,
	TURRETKILL							= 0,
	HEADSHOT								= 0,
	MELEE										= 0,
	SUICIDE									= 0,
	TEAMKILL								= -200,
	
	REPAIR									= 1/10, -- points/per damage repaired

	LOCKPICK								= 50,
	DISARM									= 50,
	TAG_ENEMY								= 5,
		
	VEHICLE_REFUND_MULT		  = 0.90,
	VEHICLE_KILL_MIN				= 10,
	VEHICLE_KILL_MULT				= 0.25,
};

-- buy flags
SinglePlayer.BUY_WEAPON 		= 1;
SinglePlayer.BUY_VEHICLE		= 2;
SinglePlayer.BUY_EQUIPMENT	= 4;
SinglePlayer.BUY_AMMO			= 8;
SinglePlayer.BUY_PROTOTYPE	= 16;

SinglePlayer.BUY_ALL				= 32-1;


SinglePlayer.weaponList=
{
	{ id="flashbang",					name="@mp_eFlashbang",						price=10, 			amount=1, ammo=true, weapon=false, category="@mp_catExplosives", loadout=1},
	{ id="smokegrenade",			name="@mp_eSmokeGrenade",				price=10, 			amount=1, ammo=true, weapon=false, category="@mp_catExplosives", loadout=1 },
	{ id="explosivegrenade",	name="@mp_eFragGrenade",					price=75, 			amount=1, ammo=true, weapon=false, category="@mp_catExplosives", loadout=1 },
	{ id="empgrenade",			name="@mp_eEMPGrenade",					price=125,				amount=1, ammo=true, weapon=false, category="@mp_catExplosives", loadout=1 },
	
	{ id="pistol",						name="@mp_ePistol", 							price=100, 			class="SOCOM",						category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=3, uniqueloadoutcount=2},
	{ id="claymore",					name="@mp_eClaymore",						price=250,				class="Claymore",					buyammo="claymoreexplosive",	selectOnBuyAmmo="true", category="@mp_catExplosives", loadout=1 },
	{ id="avmine",						name="@mp_eMine",								price=300,				class="AVMine",						buyammo="avexplosive",				selectOnBuyAmmo="true", category="@mp_catExplosives", loadout=1 },
	{ id="c4",							name="@mp_eExplosive", 						price=500, 			class="C4", 							buyammo="c4explosive",				selectOnBuyAmmo="true", category="@mp_catExplosives", loadout=1 },
	{ id="ay69",						name="@mp_eAY69", 							price=120, 			class="AY69",						category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=4, uniqueloadoutcount=2},

	{ id="shotgun",					name="@mp_eShotgun", 						price=175, 			class="Shotgun", 					uniqueId=4,		category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="smg",							name="@mp_eSMG", 								price=175, 			class="SMG", 							uniqueId=5,		category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="fy71",							name="@mp_eFY71", 							price=350, 			class="FY71", 						uniqueId=6,		category="@mp_catWeapons", loadout=1, 	uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="macs",						name="@mp_eSCAR", 							price=375, 			class="SCAR", 						uniqueId=7,		category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="rpg",							name="@mp_eML", 								price=800, 			class="LAW", 							uniqueId=8,		category="@mp_catExplosives", loadout=1},

	{ id="dsg1",							name="@mp_eSniper"	,							price=500, 			class="DSG1", 						uniqueId=9,		category="@mp_catWeapons", loadout=1, uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="gauss",						name="@mp_eGauss", 							price=2000, 			class="GaussRifle",		level=50,		uniqueId=10,	category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="fgl40",						name="@mp_eFGL40", 							price=1500, 			class="FGL40",		    level=50, 	uniqueId=15,	category="@mp_catWeapons", loadout=1},
	{ id="minigun",				name="@mp_eMinigun",						price=1250, 		class="Hurricane", 					level=50,		uniqueId=13,	category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},

};


SinglePlayer.equipList=
{
	{ id="binocs",			name="@mp_eBinoculars",							price=50,				class="Binoculars", 			uniqueId=101,		category="@mp_catEquipment", loadout=1 },
	{ id="nsivion",			name="@mp_eNightvision", 						price=10, 			class="NightVision", 			uniqueId=102,		category="@mp_catEquipment", loadout=1 },
	--{ id="lockkit",			name="@mp_eLockpick",							price=50, 			class="LockpickKit",			uniqueId=110,		category="@mp_catEquipment", loadout=1, 	uniqueloadoutgroup=2, uniqueloadoutcount=1},
	{ id="repairkit",		name="@mp_eRepair",								price=50, 			class="RepairKit", 				uniqueId=110,		category="@mp_catEquipment", loadout=1, 	uniqueloadoutgroup=2, uniqueloadoutcount=1},
	{ id="radarkit",		name="@mp_eRadar",								price=75, 			class="RadarKit", 				uniqueId=110,		category="@mp_catEquipment", loadout=1, 	uniqueloadoutgroup=2, uniqueloadoutcount=1},
}


SinglePlayer.protoList=
{
	{ id="moac",					name="@mp_eAlienWeapon", 			price=200, 		class="AlienMount", 				level=50,		uniqueId=11,	category="@mp_catWeapons", loadout=1, 		uniqueloadoutgroup=1, uniqueloadoutcount=2},
	{ id="moar",					name="@mp_eAlienMOAR", 				price=400, 		class="MOARAttach", 			level=50,		uniqueId=12,	category="@mp_catWeapons", loadout=1 },
	
	{ id="tacgun",					name="@mp_eTACLauncher", 			price=1500, 		class="TACGun", 					level=100,	restricted=1,	energy=5, uniqueId=14,	category="@mp_catWeapons", md=true, loadout=1, uniqueloadoutgroup=1, uniqueloadoutcount=2},

	{ id="usmoac4wd",			name="@mp_eMOACVehicle",			price=100, 		class="US_ltv", 						level=50, 	modification="MOAC", 				vehicle=true, buildtime=10,	category="@mp_catVehicles", loadout=0 },
	{ id="usmoar4wd",			name="@mp_eMOARVehicle",			price=200,		class="US_ltv", 						level=50,		modification="MOAR", 				vehicle=true, buildtime=10,	category="@mp_catVehicles", loadout=0 },

	{ id="ussingtank",			name="@mp_eSingTank",					price=500, 		class="US_tank",		 			level=100, 	energy=10, modification="Singularity",	vehicle=true, md=true, buildtime=45,	category="@mp_catVehicles", loadout=0 },
	{ id="ustactank",			name="@mp_eTACTank",					price=500,		class="US_tank", 					level=100, 	energy=10, modification="TACCannon",		vehicle=true, md=true, buildtime=45,	category="@mp_catVehicles", loadout=0 },
	{ id="nktactank",			name="@mp_eLightTACTank",					price=500,		class="Asian_tank", 					level=100, 	energy=10, modification="TACCannon",		vehicle=true, md=true, buildtime=45,	category="@mp_catVehicles", loadout=0 },
}

SinglePlayer.vehicleList=
{	
	{ id="light4wd",				name="@mp_eLightVehicle", 				price=0,			class="US_ltv",						modification="Unarmed", 		buildtime=5,		category="@mp_catVehicles", loadout=0 },
	{ id="us4wd",					name="@mp_eHeavyVehicle", 			price=0,			class="US_ltv",						modification="MP", 		buildtime=5,					category="@mp_catVehicles", loadout=0 },
	{ id="usgauss4wd",		name="@mp_eGaussVehicle",			price=200,		class="US_ltv", 						modification="Gauss", buildtime=10,					category="@mp_catVehicles", loadout=0 },

	{ id="nktruck",				name="@mp_eTruck",						price=0,			class="Asian_truck", 				modification="Hardtop_MP", buildtime=5,			category="@mp_catVehicles", loadout=0 },

	{ id="ussupplytruck",		name="@mp_eSupplyTruck",				price=150,		class="Asian_truck",				modification="spawntruck",	teamlimit=3, abandon=0, spawngroup=true,	buyzoneradius=11, servicezoneradius=11,	buyzoneflags=bor(bor(SinglePlayer.BUY_AMMO, SinglePlayer.BUY_WEAPON), SinglePlayer.BUY_EQUIPMENT),			buildtime=15,		category="@mp_catVehicles", loadout=0		},
		
	{ id="usboat",					name="@mp_eSmallBoat", 				price=0,			class="US_smallboat", 			modification="MP", buildtime=5,				category="@mp_catVehicles", loadout=0 },
	{ id="nkboat",					name="@mp_ePatrolBoat", 				price=100,		class="Asian_patrolboat", 		modification="MP", buildtime=10,				category="@mp_catVehicles", loadout=0 },
	{ id="nkgaussboat",		name="@mp_eGaussPatrolBoat", 		price=200,		class="Asian_patrolboat", 		modification="Gauss", buildtime=10,		category="@mp_catVehicles", loadout=0 },
	{ id="ushovercraft",		name="@mp_eHovercraft", 				price=100,		class="US_hovercraft",			modification="MP", buildtime=10,			category="@mp_catVehicles", loadout=0 },
	{ id="nkaaa",					name="@mp_eAAVehicle",					price=200,		class="Asian_aaa", 				modification="MP",	buildtime=15,			category="@mp_catVehicles", loadout=0 },
		
	{ id="usasv",					name="@mp_eASV",							price=250,		class="US_asv",						buildtime=15,		category="@mp_catVehicles", loadout=0 },
	{ id="usapc",					name="@mp_eICV",							price=300,		class="US_apc", 					buildtime=20,		category="@mp_catVehicles", loadout=0 },
	{ id="nkapc",					name="@mp_eAPC",							price=350,		class="Asian_apc", 				buildtime=20,		category="@mp_catVehicles", loadout=0 },
	
	{ id="nktank",					name="@mp_eLightTank", 				price=400,		class="Asian_tank",				buildtime=25,		category="@mp_catVehicles", loadout=0 },
	{ id="ustank",					name="@mp_eBattleTank",				price=450,		class="US_tank", 					modification="MP", 	buildtime=30,		category="@mp_catVehicles", loadout=0 },
	{ id="usgausstank",		name="@mp_eGaussTank",				price=500,		class="US_tank", 					modification="GaussCannon", 	buildtime=35,		category="@mp_catVehicles", loadout=0 },
	
	{ id="nkhelicopter",		name="@mp_eHelicopter", 				price=350,		class="Asian_helicopter",		modification="MP",	buildtime=20,		category="@mp_catVehicles", loadout=0 },
	{ id="usvtol",					name="@mp_eVTOL", 						price=450,		class="US_vtol", 					modification="MP",	buildtime=20,		category="@mp_catVehicles", loadout=0 },	
};


--us4wd,nk4wd,nktruck,ustank,ustactank,usgausstank,usapc,nktank,nktactank,nkgausstank,usspawntruck,usammotruck


SinglePlayer.ammoList=
{
	{ id="",										name="@mp_eAutoBuy",				price=0,												category="@mp_catAmmo", loadout=1 },
	{ id="bullet",							name="@mp_eBullet", 					price=5,			amount=40,				category="@mp_catAmmo", loadout=1 },
	{ id="fybullet",						name="@mp_eFYBullet", 					price=5,			amount=30,				category="@mp_catAmmo", loadout=1 },
	{ id="shotgunshell",				name="@mp_eShotgunShell",		price=5,			amount=8,					category="@mp_catAmmo", loadout=1 },
	{ id="smgbullet",						name="@mp_eSMGBullet",				price=5,			amount=50,				category="@mp_catAmmo", loadout=1 },	
	{ id="aybullet",						name="@mp_eAYBullet",				price=5,			amount=40,				category="@mp_catAmmo", loadout=1 },	
	{ id="lightbullet",					name="@mp_eLightBullet",				price=5,			amount=40,				category="@mp_catAmmo", loadout=1 },
	
	{ id="sniperbullet",				name="@mp_eSniperBullet",			price=10,			amount=10,				category="@mp_catAmmo", loadout=1 },
	{ id="scargrenade",					name="@mp_eRifleGrenade",			price=50,			amount=1,					category="@mp_catAmmo", loadout=1 },
	{ id="gaussbullet",					name="@mp_eGaussSlug",				price=100,			amount=5, level=50, category="@mp_catAmmo", loadout=1 },
	
	{ id="hurricanebullet",			name="@mp_eMinigunBullet",		price=100,			amount=500,				category="@mp_catAmmo", loadout=1 },
	
	{ id="fgl40fraggrenade",		name="@mp_eFGL40FragGrenade",		price=200,			amount=6,				category="@mp_catAmmo", loadout=1 },
	{ id="fgl40empgrenade",		name="@mp_eFGL40EMPGrenade",			price=100,			amount=6,				category="@mp_catAmmo", loadout=1 },

	{ id="claymoreexplosive",																price=200,			amount=1,			invisible=true,		category="@mp_catAmmo", loadout=1 },
	{ id="avexplosive",																			price=300,			amount=1,			invisible=true,		category="@mp_catAmmo", loadout=1 },
	{ id="c4explosive",																			price=500,		amount=1,			invisible=true,		category="@mp_catAmmo", loadout=1 },

	{ id="Tank_singularityprojectile",name="@mp_eSingularityShell",			price=350,		amount=1,					category="@mp_catAmmo", loadout=0, vehicleammo=1},	
		
	{ id="towmissile",					name="@mp_eAPCMissile",				price=50,			amount=2,					category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="dumbaamissile",				name="@mp_eAAAMissile",			price=100,			amount=4,					category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="tank125",						name="@mp_eTankShells",			price=100,		amount=10,				category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="helicoptermissile",			name="@mp_eHelicopterMissile",	price=50,		amount=7,					category="@mp_catAmmo", loadout=0, vehicleammo=1 },

	{ id="tank30",							name="@mp_eAPCCannon",			price=100,		amount=100,				category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="tankaa",							name="@mp_eAAACannon",			price=100,		amount=250,				category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="a2ahomingmissile",		name="@mp_eVTOLMissile",			price=150,		amount=6,					category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	{ id="gausstankbullet",			name="@mp_eGaussTankSlug",		price=200,		amount=10,				category="@mp_catAmmo", loadout=0, vehicleammo=1 },
	
	{ id="tacprojectile",		name="@mp_eTACTankShell",	price=350,		amount=1,	ammo=true, 			level=100,		category="@mp_catAmmo", vehicleammo=1 },

	{ id="psilent",							name="@mp_ePSilencer",				price=10, 			class="SOCOMSilencer",			uniqueId=121, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="plam",								name="@mp_ePLAM",						price=25, 			class="LAM",				uniqueId=122, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="silent",							name="@mp_eRSilencer",				price=10, 			class="Silencer", 				uniqueId=123, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },	
	{ id="lam",								name="@mp_eRLAM",						price=25, 			class="LAMRifle",						uniqueId=124, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="reflex",							name="@mp_eReflex",					price=25,				class="Reflex", 					uniqueId=125, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="ascope",							name="@mp_eAScope",					price=50, 			class="AssaultScope", 			uniqueId=126, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="scope",							name="@mp_eSScope",					price=100, 			class="SniperScope", 			uniqueId=127, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	{ id="gl",									name="@mp_eGL",							price=200, 			class="GrenadeLauncher",		uniqueId=128, ammo=false, equip=true,		category="@mp_catAddons", loadout=1 },
	
};

SinglePlayer.buyList={};

for i,v in ipairs(SinglePlayer.weaponList) do SinglePlayer.buyList[v.id]=v; if (type(v.weapon)=="nil") then v.weapon=true; end;	end;
for i,v in ipairs(SinglePlayer.equipList) do SinglePlayer.buyList[v.id]=v; if (type(v.equip)=="nil") then	v.equip=true; end; end;
for i,v in ipairs(SinglePlayer.protoList) do SinglePlayer.buyList[v.id]=v; if (type(v.proto)=="nil") then	v.proto=true; end; end;
for i,v in ipairs(SinglePlayer.vehicleList) do SinglePlayer.buyList[v.id]=v; if (type(v.vehicle)=="nil") then v.vehicle=true; end; end;
for i,v in ipairs(SinglePlayer.ammoList) do SinglePlayer.buyList[v.id]=v; if (type(v.ammo)=="nil") then v.ammo=true; end; end;

--~TheOtherSide

if (not g_dmgMult) then g_dmgMult = 1.0; end
if (not g_barbWireMaterial) then g_barbWireMaterial = System.GetSurfaceTypeIdByName("mat_metal_barbwire"); end

----------------------------------------------------------------------------------------------------
function SinglePlayer:IsMultiplayer()
	return false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnReset(toGame)
	self.works={};
	self.inBuyZone={};

	AIReset();
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:InitHitMaterials()
	local mats={
		"mat_helmet",
		"mat_kevlar",
		"mat_head",
		"mat_torso",
		"mat_arm_left",
		"mat_arm_right",
		"mat_hand_left",
		"mat_hand_right",
		"mat_leg_left",
		"mat_leg_right",
		"mat_foot_left",
		"mat_foot_right",
			
			--aliens		
		"mat_alien_vulnerable",
		"mat_alien_hunter_leg",
		"mat_alien_hunter_torso",
		"mat_alien_hunter_head",
		"mat_alien_hunter_vulnerable",
		--
		"mat_alien_hunter_topFace",
		"mat_alien_hunter_bottomFace",
		"mat_alien_hunter_leftFace",
		"mat_alien_hunter_rightFace",
	};
	
	for i,v in ipairs(mats) do
		self.game:RegisterHitMaterial(v);
	end
	
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:InitHitTypes()
	local types={
		"normal",
		"repair",
		"lockpick",
		"bullet",
		"gaussbullet",
		"frost",
		"fire",
		"radiation",
		"melee",
		"tac",
		"frag",
		"fall",
		"collision",
		"event",
		"punish",
		"avmine",
		"moacbullet",
		"trooper_melee",
		"scout_moac",
		"aacannon",
		"emp",
		"law_rocket",
		
		"ht_alienmount_ACMO",
		"ht_AYBullet",
		"ht_Bullet",
		"ht_FYBullet",
		"ht_GaussBullet",
		"ht_HurricaneBullet",
		"ht_ShotgunShell",
		"ht_SMGBullet",
		"ht_SniperBullet",
		"ht_LightBullet",
		"ht_TACGunProjectile",
		"ht_SCARGrenade",
		"ht_Rocket",
		"ht_FGL40FragGrenade",
		"ht_ExplosiveGrenade",
		"ht_C4Explosive",
		"ht_AVExplosive",

		"ht_AACannon",
		"ht_AARocketLauncher",
		"ht_AHMachinegun",
		"ht_APCCannon",
		"ht_GaussCannon",
		"ht_USTankCannon",
		"ht_VehicleGaussMounted",
		"ht_VehicleMOACMounted",
		"ht_VehicleMOARMounted",
		"ht_VehicleTAC",
		"ht_VTOL",

		"ht_TrooperMoac",
		"exp1_scout_beam",
	};

	for i,v in ipairs(types) do
		self.game:RegisterHitType(v);
	end
	
	-- cache
	g_collisionHitTypeId = self.game:GetHitTypeId("collision");
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:IsHeadShot(hit)
	return hit.material_type and ((hit.material_type == "head") or (hit.material_type == "helmet")) or false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:CalcDamage(material_type, damage, tbl, assist)
	if (damage and damage~=0) then
		if (material_type and tbl) then
			local mult = tbl[material_type] or 1;
			local asm = assist*tbl["assist_min"];
			if (mult < asm) then
				mult=asm;
			end
			
			--if (assist > 0) then
			--	Log(">> SinglePlayer:CalcDamage assisted mult=(%f) <<", mult);
			--else
			--	Log(">> SinglePlayer:CalcDamage non-assisted mult=(%f) <<", mult);
			--end	
			
			return damage*mult;
		end
		return damage;
	end

	return 0;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:CalcExplosionDamage(entity, explosion, obstruction)
	local empExpl = explosion.type == "emp";
	-- impact explosions directly damage the impact target
	if ( (not empExpl) and explosion.impact and explosion.impact_targetId and explosion.impact_targetId==entity.id) then
		return explosion.damage;
	end

	local effect=1;
	if (not entity.vehicle) then
		local distance=vecLen(vecSub(entity:GetWorldPos(), explosion.pos));
		if (distance<=explosion.min_radius or explosion.min_radius==explosion.radius) then
			effect=1;
		else
			distance=math.max(0, math.min(distance, explosion.radius));
			local r=explosion.radius-explosion.min_radius;
			local d=distance-explosion.min_radius;
			effect=(r-d)/r;
			effect=math.max(math.min(1, effect*effect), 0);
		end

		effect=effect*(1-obstruction);
		
		if(entity.actor and entity.actor:GetPhysicalizationProfile() == "sleep") then
			return explosion.damage*2*effect; --sleeping targets get more damage
		end
	else
		effect=1-obstruction;
	end
	
	if (empExpl) then
		self.game:ProcessEMPEffect(entity.id, explosion.shooter.id, effect);
	end
	
	return explosion.damage*effect;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:EquipActor(actor)
	--Log(">> SinglePlayer:EquipActor(%s) <<", actor:GetName());
	
	if(self.game:IsDemoMode() ~= 0) then -- don't equip actors in demo playback mode, only use existing items
		--Log("Don't Equip : DemoMode");
		return;
	end;

	actor.inventory:Destroy();

	if (actor.actor:IsPlayer()) then
		ItemSystem.GiveItemPack(actor.id, "DefaultPlayer", false, true);
	end

	if (not actor.actor:IsPlayer()) then
		if (actor.Properties) then		
			local equipmentPack=actor.Properties.equip_EquipmentPack;
			if (equipmentPack and equipmentPack ~= "") then
				ItemSystem.GiveItemPack(actor.id, equipmentPack, false, false);
			end

	  	if(not actor.bGunReady) then
	  		actor:HolsterItem(true);
	  	end
	  end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnShoot(shooter)
	if (shooter and shooter.OnShoot) then
		if (not shooter:OnShoot()) then
			return false;
		end
	end
	
	return true;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:IsUsable(srcId, objId)
	if not objId then return 0 end;

	local obj = System.GetEntity(objId);
	if (obj.IsUsable) then
		if (obj:IsHidden()) then
			return 0;
		end;
		local src = System.GetEntity(srcId);
		if (src and src.actor and (src:IsDead() or (src.actor:GetSpectatorMode()~=0) or src.actorStats.isFrozen)) then
			return 0;
		end
		return obj:IsUsable(src);
	end

	return 0;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnNewUsable(srcId, objId, usableId)
	if(not self.game:GetControlSystemEnabled()) then
		if not srcId then return end
		if objId and not System.GetEntity(objId) then objId = nil end
		
		local src = System.GetEntity(srcId)
		if src and src.SetOnUseData then
			src:SetOnUseData(objId or NULL_ENTITY, usableId)
		end

		if srcId ~= g_localActorId then return end

		if self.UsableMessage then
			HUD.SetInstructionObsolete(self.UsableMessage)
			self.UsableMessage = nil
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnUsableMessage(srcId, objId, objEntityId, usableId)
	if(not self.game:GetControlSystemEnabled()) then
		if srcId ~= g_localActorId then return end
	
		local msg = "";
		
		if objId then
			obj = System.GetEntity(objId)
			if obj then

				if obj.GetUsableMessage then
					msg = obj:GetUsableMessage(usableId)
				else
					local state = obj:GetState()
					if state ~= "" then
						state = obj[state]
						if state.GetUsableMessage then
							msg = state.GetUsableMessage(obj, usableId)
						end
					end
				end
			end
		end
		
		if(HUD) then
			HUD.SetUsability(objEntityId, msg); --this triggers the usable marker in the HUD
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnLongHover(srcId, objId)
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:EndLevel( params )
	if (not System.IsEditor()) then		  
		if (not params.nextlevel) then		  
			Game.PauseGame(true);
			Game.ShowMainMenu();
		end
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CreateExplosion(shooterId,weaponId,damage,pos,dir,radius,angle,pressure,holesize,effect,effectScale, minRadius, minPhysRadius, physRadius)
	if (not dir) then
		dir=g_Vectors.up;
	end
	
	if (not radius) then
		radius=5.5;
	end

	if (not minRadius) then
		minRadius=radius/2;
	end

	if (not physRadius) then
		physRadius=radius;
	end

	if (not minPhysRadius) then
		minPhysRadius=physRadius/2;
	end

	if (not angle) then
		angle=0;
	end
	
	if (not pressure) then
		pressure=200;
	end
	
	if (holesize==nil) then
    holesize = math.min(radius, 5.0);
	end
	
	if (radius == 0) then
		return;
	end

	--System.LogAlways("Exp Radius = "..radius);
	--System.LogAlways("Exp Damage = "..damage);
	self.game:ServerExplosion(shooterId or NULL_ENTITY, weaponId or NULL_ENTITY, damage, pos, dir, radius, angle, pressure, holesize, effect, effectScale, nil, minRadius, minPhysRadius, physRadius);
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CreateHit(targetId,shooterId,weaponId,dmg,radius,material,partId,type,pos,dir,normal)
	if (not radius) then
		radius=0;
	end
	
	local materialId=0;
	
	if (material) then
		materialId=self.game:GetHitMaterialId(material);
	end
	
	if (not partId) then
		partId=-1;
	end
	
	local typeId=0;
	if (type) then
		typeId=self.game:GetHitTypeId(type);
	else
		typeId=self.game:GetHitTypeId("normal");
	end
	
	self.game:ServerHit(targetId, shooterId, weaponId, dmg, radius, materialId, partId, typeId, pos, dir, normal);

end


----------------------------------------------------------------------------------------------------
function SinglePlayer:ClientViewShake(pos, radius, amount, duration, frequency, source)
	if (g_localActor and g_localActor.actor) then
		self:ViewShake(g_localActor, pos, radius, amount, duration, frequency, source);
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:ViewShake(player, pos, radius, amount, duration, frequency, source)
	local delta = self.tempVec;
	CopyVector(delta,pos);
	FastDifferenceVectors(delta, delta, player:GetWorldPos());
	
	local distance = LengthVector(delta);
	local deltaDist = radius - distance;
	
	if (deltaDist > 0.0) then
		local amt = amount*(deltaDist/radius);
		local halfDur = duration * 0.5;
		player.actor:SetViewShake({x=2*g_Deg2Rad*amt, y=2*g_Deg2Rad*amt, z=2*g_Deg2Rad*amt}, {x=0.02*amt, y=0.02*amt, z=0.02*amt},halfDur + halfDur*(deltaDist/radius), 1/20, 4);
		player.viewBlur = duration;
		player.viewBlurAmt = 0.5;
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetCollisionMinVelocity(entity, collider, hit)
	
	local minVel=10;
	
	if ((entity.actor and not entity.actor:IsPlayer()) or entity.advancedDoor) then
		minVel=1; --Door or character hit	
	end	
	
	if(entity.actor and collider and collider.vehicle) then
		minVel=6; -- otherwise we don't get damage at slower speeds
		
		if((entity.actor:IsPlayer()) and self:IsMultiplayer()) then
			minVel=5;	-- reduce even further for MP players hit by vehicles.
		end
	end
	
	if(not entity.vehicle and hit.target_velocity and vecLenSq(hit.target_velocity) == 0) then -- if collision target it not moving
		minVel = minVel * 2;
	end
	
	return minVel;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetCollisionDamageMult(entity, collider, hit)		
	
	local mult = 1;	
	local debugColl = self.game:DebugCollisionDamage();
	
	if (collider) then
  	if (collider.GetForeignCollisionMult) then
  	  
  	  local foreignMult = collider.GetForeignCollisionMult(collider, entity, hit);
  		mult = mult * foreignMult;
  		
  		if (debugColl>0 and foreignMult ~= 1) then  		  
  		  Log("<%s>: collider <%s> has ForeignCollisionMult %.2f", entity:GetName(), collider:GetName(), foreignMult);  		  
  		end  		
  	end  	   	
  end
  
	if (entity.GetSelfCollisionMult) then 
	  
	  local selfMult = entity.GetSelfCollisionMult(entity, collider, hit);
		mult = mult * selfMult;
		
		if (debugColl>0 and selfMult ~= 1) then		  
		  Log("<%s>: returned SelfCollisionMult %.2f", entity:GetName(), selfMult);		  
		end  	
	end		
	
	return mult;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:OnCollision(entity, hit)
	local collider = hit.target;
	local colliderMass = hit.target_mass; -- beware, collider can be null (e.g. entity-less rigid entities)
	local contactVelocitySq;
	local contactMass;

	-- check if frozen
	if (self.game:IsFrozen(entity.id)) then
		if ((not entity.CanShatter) or (tonumber(entity:CanShatter())~=0)) then
			local energy = self:GetCollisionEnergy(entity, hit);
	
			local minEnergy = 1000;
			
			if (energy >= minEnergy) then
				if (not collider) then
					collider=entity;
				end
	
		    	local colHit = self.collisionHit;
				colHit.pos = hit.pos;
				colHit.dir = hit.dir or hit.normal;
				colHit.radius = 0;	
				colHit.partId = -1;
				colHit.target = entity;
				colHit.targetId = entity.id;
				colHit.weapon = collider;
				colHit.weaponId = collider.id
				colHit.shooter = collider;
				colHit.shooterId = collider.id
				colHit.materialId = 0;
				colHit.damage = 0;
				colHit.typeId = g_collisionHitTypeId;
				colHit.type = "collision";
				
				if (collider.vehicle and collider.GetDriverId) then
				  local driverId = collider:GetDriverId();
				  if (driverId) then
					  colHit.shooterId = driverId;
					  colHit.shooter=System.GetEntity(colHit.shooterId);
					end
				end
	
				self:ShatterEntity(entity.id, colHit);
			end
	
			return;
		end
	end
	
	if (not (entity.Server and entity.Server.OnHit)) then
	  return;
	end
	
	if (entity.IsDead and entity:IsDead()) then
	  return;
	end	
		
	local minVelocity;
	
	-- collision with another entity
	if (collider or colliderMass>0) then
		FastDifferenceVectors(self.tempVec, hit.velocity, hit.target_velocity);
		contactVelocitySq = vecLenSq(self.tempVec);
		contactMass = colliderMass;		
		minVelocity = self:GetCollisionMinVelocity(entity, collider, hit);
	else	-- collision with world		
		contactVelocitySq = vecLenSq(hit.velocity);
		contactMass = entity:GetMass();
		minVelocity = 7.5;
	end
	
	-- marcok: avoid fp exceptions, not nice but I don't want to mess up any damage calculations below at this stage
	if (contactVelocitySq < 0.01) then
		contactVelocitySq = 0.01;
	end
	
	local damage = 0;
	
	-- make sure we're colliding with something worthy
	if (contactMass > 0.01) then 		
		local minVelocitySq = minVelocity*minVelocity;
		local bigObject = false;
		--this should handle falling trees/rocks (vehicles are more heavy usually)
		if(contactMass > 200.0 and contactMass < 10000 and contactVelocitySq > 2.25) then
			if(hit.target_velocity and vecLenSq(hit.target_velocity) > (contactVelocitySq * 0.3)) then
				bigObject = true;
				--vehicles and doors shouldn't be 'bigObject'-ified
				if(collider and (collider.vehicle or collider.advancedDoor)) then
					bigObject = false;
				end
			end
		end
		
		local collideBarbWire = false;
		if(hit.materialId == g_barbWireMaterial and entity and entity.actor) then
			collideBarbWire = true;
		end
			
		--Log("velo : %f, mass : %f", contactVelocitySq, contactMass);
		if (contactVelocitySq >= minVelocitySq or bigObject or collideBarbWire) then		
			-- tell AIs about collision
			if(AI and entity and entity.AI and not entity.AI.Colliding) then 
				g_SignalData.id = hit.target_id;
				g_SignalData.fValue = contactVelocitySq;
				AI.Signal(SIGNALFILTER_SENDER,1,"OnCollision",entity.id,g_SignalData);
				entity.AI.Colliding = true;
				entity:SetTimer(COLLISION_TIMER,4000);
			end			
			
			-- marcok: Uncomment this stuff when you need it
		  --local debugColl = self.game:DebugCollisionDamage();
			
			local contactVelocity = math.sqrt(contactVelocitySq)-minVelocity;
			if (contactVelocity < 0.0) then
				contactVelocitySq = minVelocitySq;
				contactVelocity = 0.0;
			end
					 			  			
			-- damage computation
			if(entity.vehicle) then
				if(not self:IsMultiplayer()) then
					damage = 0.0005*self:GetCollisionEnergy(entity, hit); -- vehicles get less damage SINGLEPLAYER ONLY.
				else
					damage = 0.0002*self:GetCollisionEnergy(entity, hit);	-- keeping the original values for MP.
				end
			else
				damage = 0.0025*self:GetCollisionEnergy(entity, hit);
			end
	
			-- apply damage multipliers 
			damage = damage * self:GetCollisionDamageMult(entity, collider, hit);  
				
			if(collideBarbWire and entity.id == g_localActorId) then
				damage = damage * (contactMass * 0.15) * (30.0 / contactVelocitySq);
			end
	
      if(bigObject) then
        if (damage > 0.5) then

			---TheOtherSide
			if(collider and collider.Properties.species == entity.Properties.species and g_gameRules.game:IsConquestGamemode()) then
				return;
			end
			--~TheOtherSide

        	if (entity.actor and entity.id ~= g_localActorId and entity.Properties.bNanoSuit==1 and not self:IsMultiplayer()) then
        		if(damage > 500.0) then
					entity.actor:Fall(hit.pos);
        		end
        		damage = damage * 1; --to be tweaked
        	else
					  damage = damage * (contactMass / 10.0) * (10.0 / contactVelocitySq);
					  if(entity.id ~= g_localActorId) then
							damage = damage * 3;
				 		end
				  end
				else
				  return;
				end
			end	
			
			-- subtract collision damage threshold, if available
			if (entity.GetCollisionDamageThreshold) then
			  local old = damage;
			  damage = __max(0, damage - entity:GetCollisionDamageThreshold());		
			end

			if(damage < 1.0) then
				return;
			end
			
			if (entity.actor) then
				if(entity.actor:IsPlayer()) then 
					if(hit.target_velocity and vecLen(hit.target_velocity) == 0) then --limit damage from running agains static objects
						damage = damage * 0.2;
					end
					--DESIGN : ragdolls should not instant kill the player on collision
					if (collider and collider.actor and collider.actor:GetHealth() <= 0) then
						damage = 0;--__max(entity.actor:GetHealth() / 2, damage);
					end
				else
					local fallenTime = entity.actor:GetFallenTime();
					if(not self:IsMultiplayer() and fallenTime > 0 and fallenTime < 300) then --300ms window
						--damage = damage * 0.1; --this prevents actors already falling/fallen to die from multiple collisions with the same object
						return;
					end
				end
			
				if(collider and collider.class=="AdvancedDoor")then
					if(collider:GetState()=="Opened")then
						entity:KnockedOutByDoor(hit,contactMass,contactVelocity);
					end
				end;
				
				if (collider and not collider.actor) then
				  local contactVelocityCollider = __max(0, vecLen(hit.target_velocity)-minVelocity);  				  
				  local fallVelocity = (entity.collisionKillVelocity or 20.0);
				  
						--KYONG BATTLE FIX for patch2, workaround for random extreme velocities (100+ times more than normal)
						if(damage > 700.0) then
							if(entity.actor) then
								if((contactVelocityCollider > 4000) or string.find(entity:GetName(),"Kyong")) then
									damage = 700.0;
								end
							end
						end
				    				  
				  if(contactVelocity > fallVelocity and contactVelocityCollider > fallVelocity and colliderMass > 50 and not entity.actor:IsPlayer()) then  				  	
				  	local bNoDeath = entity.Properties.Damage.bNoDeath;
				  	local bFall = bNoDeath and bNoDeath~=0;
				  
				  	-- don't allow killing friendly AIs by collisions
						if(not AI.Hostile(entity.id, g_localActorId, false)) then
							return;
						end
					  
						if(g_localActor.actor:GetPlayerSlaveId() ~= 0)then
							local desiredSlaveId = g_localActor.actor:GetPlayerSlaveId();
							if(not AI.Hostile(entity.id, desiredSlaveId, false)) then
								return;
							end
						end

						
				  	--if (debugColl~=0) then
				  	--  Log("%s for <%s>, collider <%s>, contactVel %.1f, contactVelCollider %.1f, colliderMass %.1f", bFall and "FALL" or "KILL", entity:GetName(), collider:GetName(), contactVelocity, contactVelocityCollider, colliderMass);
				  	--end  				  	
				  	
				  	if(bFall) then
				  	  entity.actor:Fall(hit.pos);
						end
					else
						if(g_localActorId or g_localActor.actor:GetPlayerSlaveId()~=0 and AI.Hostile(entity.id, g_localActorId, false) or AI.Hostile(entity.id, g_localActor.actor:GetPlayerSlaveId(), false)) then
							if(not entity.isAlien and contactVelocity > 5.0 and contactMass > 10.0 and not (entity.actor:IsPlayer() or entity.actor:IsHaveOwner())) then
								if(damage < 50) then
									damage = 50;
									entity.actor:Fall(hit.pos);
								end
							else
							 if(not entity.isAlien and contactMass > 2.0 and contactVelocity > 15.0 and not (entity.actor:IsPlayer() or entity.actor:IsHaveOwner())) then
								if(damage < 50) then
									damage = 50;
									entity.actor:Fall(hit.pos);
								end
							 end 
							end
						end
					end
				end
			end
  		
			
			if (damage >= 0.5) then				  				
				if (not collider) then collider = entity; end;		
				
				--prevent deadly collision damage (old system somehow failed)
				if(entity.actor and not self:IsMultiplayer() and not AI.Hostile(entity.id, g_localActorId, false)) then
					if(entity.id ~= g_localActorId) then
						if(entity.actor:GetHealth() <= damage) then
							entity.actor:Fall(hit.pos);
							return;
						end
					end
				else
					if(entity.actor and collider and collider.actor and not self:IsMultiplayer()) then 
						entity.actor:Fall(hit.pos);
						return;
					end
				end

			  local curtime = System.GetCurrTime();
			  if (entity.lastCollDamagerId and entity.lastCollDamagerId==collider.id and 
					  entity.lastCollDamageTime+0.3>curtime and damage<entity.lastCollDamage*2) then
					return
				end
				entity.lastCollDamagerId = collider.id;
				entity.lastCollDamageTime = curtime;
				entity.lastCollDamage = damage;
				
				--if (debugColl>0) then
				--  Log("[SinglePlayer] <%s>: sending coll damage %.1f", entity:GetName(), damage);
				--end
			
		    local colHit = self.collisionHit;
				colHit.pos = hit.pos;
				colHit.dir = hit.dir or hit.normal;
				colHit.radius = 0;	
				colHit.partId = -1;
				colHit.target = entity;
				colHit.targetId = entity.id;
				colHit.weapon = collider;
				colHit.weaponId = collider.id
				colHit.shooter = collider;
				colHit.shooterId = collider.id
				colHit.materialId = 0;
				colHit.damage = damage;
				colHit.typeId = g_collisionHitTypeId;
				colHit.type = "collision";
				colHit.impulse=hit.impulse;
				
				if (collider.vehicle) then
					if(collider.GetDriverId) then
						local driverId = collider:GetDriverId();
						if(self:IsMultiplayer() and not driverId and collider.GetLastDriverId) then
							driverId = collider:GetLastDriverId();
						end
					  
						if (driverId) then
							colHit.shooterId = driverId;
							colHit.shooter=System.GetEntity(colHit.shooterId);
						end
					end
					
					if(entity.actor and entity.lastExitedVehicleId) then
						if(entity.lastExitedVehicleId == collider.id) then
							if(_time-entity.lastExitedVehicleTime < 2.5) then
								-- just got out of this vehicle. No damage.
								colHit.damage = 0;
							end
						end
					end
					
					--extra multiplier for friendly vehicles
					local colliderTeam = self.game:GetTeam(collider.id);
					if(colliderTeam ~= 0 and colliderTeam == self.game:GetTeam(entity.id)) then
						colHit.damage = colHit.damage * tonumber(System.GetCVar("g_friendlyVehicleCollisionRatio"));
					end
					
					-- and yet another one for vehicle-specific damage
					if(entity.actor) then
						colHit.damage = colHit.damage * collider.vehicle:GetPlayerCollisionMult();
					end
				end
				
				local deadly=false;
			
				if (entity.Server.OnHit(entity, colHit)) then
					-- special case for actors
					-- if more special cases come up, lets move this into the entity
						if (entity.actor and self.ProcessDeath) then
							self:ProcessDeath(colHit);
						end
					
					deadly=true;
				end
				
				local debugHits = self.game:DebugHits();
				
				if (debugHits>0) then
					self:LogHit(colHit, debugHits>1, deadly);
				end				
			end
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:OnSpawn()
	self:InitHitMaterials();
	self:InitHitTypes();
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnInit()
	self.fallHit={};
	self.explosionHit={};
	self.collisionHit={};
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnInit()
	self.fadeFrames = 0;
	self.curFadeTime = 0;
	self.fadeTime = 2;	-- fade time in seconds
	self.fading = true;
	self.fadingToBlack = false;

	if (not System.IsEditor()) then	
		Sound.SetMasterVolumeScale(0);
	else
	  -- full volume when starting the Editor
		Sound.SetMasterVolumeScale(1);
	end
	
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnClientConnect( channelId )
	local params =
	{
		name     = "Dude",
		class    = "Player",
		position = {x=0, y=0, z=0},
		rotation = {x=0, y=0, z=0},
		scale    = {x=1, y=1, z=1},
	};
	local player = Actor.CreateActor(channelId, params);
	
	if (not player) then
	  Log("OnClientConnect: Failed to spawn the player!");
	  return;
	end
	
	local spawnId = self.game:GetFirstSpawnLocation(0);
	if (spawnId) then
		local spawn=System.GetEntity(spawnId);
		if (spawn) then
			--set pos
			player:SetWorldPos(spawn:GetWorldPos(g_Vectors.temp_v1));
			--set angles
			player:SetWorldAngles(spawn:GetAngles(g_Vectors.temp_v1));
			spawn:Spawned(player);
			
			return;
		end
	end

	self.works[player.id]=nil;

	player.actor:SetSpectatorMode(1, NULL_ENTITY);
	
	System.Log("$1warning: No spawn points; using default spawn location!")
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnClientEnteredGame( channelId, player, loadingSaveGame )
end

--------------------------------------------------------------------------
function SinglePlayer:ProcessFallDamage(actor, fallspeed, freefall)--distance)

	do return end;

	local dead = (actor.IsDead and actor:IsDead());
	if (dead) then
		return;
	end

	local fataldamage = 105;--the damage applied for fatal distance. The more the distance is, the more the damage.
	
--	local safe 	= 2;				--a fall less than this distance(meters) is safe
--	local fatal = 12;				--a fall bigger than this distance(meters) is fatal
--			
--	if (distance <= safe) then
--		return;
--	end
			
--	local delta 		= fatal - safe;
--	local excursion = distance - safe;
--	
--	local damage = (1.0 - ((delta - excursion) / delta)) * fataldamage;
--	local dead = (actor.IsDead and actor:IsDead());

	local safeZVel = 5;
	
	if (fallspeed < 8) then
		return;
	end
	
	local fatalZVel = 17;	
	
	-- in strength mode you jump higher and get less/later damage from falling
	if(actor.actorStats.nanoSuitStrength > 50) then
		if(fallspeed < 10) then
			return;
		end
		fatalZVel = 20;
	end
	
	local deltaZVel = fatalZVel - safeZVel;
	local excursionZVel = fallspeed - safeZVel;
	local damage = (1.0 - ((deltaZVel - excursionZVel) / deltaZVel)) * fataldamage;
	
	if (actor.actorStats.inFreeFall==1) then
		damage=1000;
	end
	
	--Log("falldamage to "..actor:GetName()..":"..damage);

	self.fallHit.partId = -1;
	self.fallHit.pos = actor:GetWorldPos();
	self.fallHit.dir = g_Vectors.v001;
	self.fallHit.radius = 0;	
	self.fallHit.target = actor;
	self.fallHit.targetId = actor.id;
	self.fallHit.weapon = actor;
	self.fallHit.weaponId = actor.id
	self.fallHit.shooter = actor;
	self.fallHit.shooterId = actor.id
	self.fallHit.materialId = 0;
	self.fallHit.damage = damage;
	self.fallHit.typeId = self.game:GetHitTypeId("fall");
	self.fallHit.type = "fall";
	
	local deadly=false;
	
	if ((not dead) and actor.Server.OnHit(actor, self.fallHit)) then	
		self:ProcessDeath(self.fallHit);
		
		deadly=true;
	end
	
	local debugHits = self.game:DebugHits();
	
	if (debugHits>0) then
		self:LogHit(self.fallHit, debugHits>1, deadly);
	end	
end

----------------------------------------------------------------------------------------------------
-- how much damage does 1 point of energy absorbs?
function SinglePlayer:GetEnergyAbsorption(player)
	local suitMaxEnergy = 200.0; --keep in mind to change this if the suit max energy is altered
	return tonumber(System.GetCVar("g_suitArmorHealthValue"))/suitMaxEnergy;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetDamageAbsorption(actor, hit)
	if(hit.damage == 0 or hit.type=="punish") then
		return 0;
	end

	--TheOtherSide
	if (actor.actor:IsHumanMode()) then
		return 0;
	end
	--~TheOtherSide
	
	local nanoSuitMode = actor.actor:GetNanoSuitMode();
	if(nanoSuitMode == 3) then -- armor mode
		local suitMaxEnergy = 200.0; --keep in mind to change this if the suit max energy is altered
		local armorHealthVal = tonumber(System.GetCVar("g_suitArmorHealthValue"));
		local currentSuitEnergy = actor.actor:GetNanoSuitEnergy();
		local currentSuitArmor = (currentSuitEnergy / suitMaxEnergy) * armorHealthVal; -- Convert energy to health points
		-- Reduce energy based on damage. The left over will be reduced from the health.
		local suitArmorLeft = currentSuitArmor - hit.damage;
		local absorption = 0.0;
		if (suitArmorLeft < 0.0) then
			-- Only part of the hit was absorbted by the armor, no energy left anymore and return the remaining fraction.
			actor.actor:SetNanoSuitEnergy(0);
			absorption = 1 + suitArmorLeft/hit.damage;
		else
			-- Convert remaining health points back to energy
			actor.actor:SetNanoSuitEnergy((suitArmorLeft / armorHealthVal) * suitMaxEnergy);
			absorption = 1;
		end
		
		-- When in armor mode, absorb at least 30% of the damage.
		return math.max(0.3, absorption);
	end
	
	return 0;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:ProcessActorDamage(hit)

	local target=hit.target;
	local shooter=hit.shooter;
	local weapon=hit.weapon;
	local health = target.actor:GetHealth();
	
	if(target.Properties.bInvulnerable)then
		if(target.Properties.bInvulnerable==1)then
			return (health <= 0);
		end;
	end;
	
	if(hit.target.actor:IsPlayer() or hit.target.actor:IsHaveOwner()) then
		if(hit.type == "fire") then
			HUD.HitIndicator();
		end
		if(hit.explosion and (hit.target.actor.id == g_localActorId or (hit.target.actor.id == g_localActor.actor:GetSlaveId()))) then
			HUD.DamageIndicator(hit.weaponId or NULL_ENTITY, hit.shooterId or NULL_ENTITY, hit.dir, false);
		end
	end
	
	local dmgMult = 1.0;
	if (target and target.actor and (target.actor:IsPlayer() or hit.target.actor:IsHaveOwner())) then
		dmgMult = g_dmgMult;
	end

	local totalDamage = 0;
	
	local splayer=source and shooter.actor and (shooter.actor:IsPlayer() or hit.target.actor:IsHaveOwner());
	local sai=(not splayer) and shooter and shooter.actor;
	local tplayer=target and target.actor and (target.actor:IsPlayer() or hit.target.actor:IsHaveOwner());
	local tai=(not tplayer) and target and target.actor;
	
	if (not self:IsMultiplayer()) then
		if (sai and not tai) then
			-- AI vs. player
			totalDamage = AI.ProcessBalancedDamage(shooter.id, target.id, dmgMult*hit.damage, hit.type);
			totalDamage = totalDamage*(1-self:GetDamageAbsorption(target, hit));
				--totalDamage = dmgMult*hit.damage*(1-target:GetDamageAbsorption(hit.type, hit.damage));
		elseif (sai and tai) then
			-- AI vs. AI
			totalDamage = AI.ProcessBalancedDamage(shooter.id, target.id, dmgMult*hit.damage, hit.type);
			totalDamage = totalDamage*(1-self:GetDamageAbsorption(target, hit));
		else
			totalDamage = dmgMult*hit.damage*(1-self:GetDamageAbsorption(target, hit));
		end
	else
		totalDamage = dmgMult*hit.damage*(1-self:GetDamageAbsorption(target, hit));
	end

	--update the health
	health = math.floor(health - totalDamage);

	if (self.game:DebugCollisionDamage()>0) then	
	  Log("<%s> hit damage: %d // absorbed: %d // health: %d", target:GetName(), hit.damage, hit.damage*self:GetDamageAbsorption(target, hit), health);
	end
	
	if (health<=0) then --prevent death out of some reason
		if(target.Properties.Damage.bNoDeath and target.Properties.Damage.bNoDeath==1) then
			target.actor:Fall(hit.pos);
			return false;
		else
			if(target.id == g_localActorId) then
				if(System.GetCVar("g_PlayerFallAndPlay") == 1) then
					HUD.StartPlayerFallAndPlay();
					return false;
				--else
				--	if(System.GetCVar("g_difficultyLevel") < 2 or System.GetCVar("g_godMode") == 3) then --System.GetCVar("g_playerRespawns")
				--		if(hit.type == "bullet" or hit.type == "gaussbullet" or hit.type == "melee" or hit.type == "frag" or hit.type == "C4") then --
				--			HUD.FakeDeath();
				--			return false;
				--		end
				--	end
				end;
			else	--prevent friendly AIs from dying by player action //from grenade explosions (Bernds call)
				--if(hit.type == "frag") then
				if(hit.shooterId == g_localActorId) then
					if(not AI.Hostile(hit.target.id, g_localActorId, false)) then
						target.actor:Fall(hit.pos);
						return false;
					end
				end
				--end
			end;
		end;
	end
	
	--if the actor is god do some counts and reset the hp if necessary
	local isGod = target.actorStats.godMode;
	if (isGod and isGod > 0) then
	 	if (health <=0) then
	 		target.actor:SetHealth(0);  --is only called to count deaths in GOD mode within C++
			health = target.Properties.Damage.health;	
		end
	end
	
	target.actor:SetHealth(health);	
	
	if(health>0 and target.Properties.Damage.FallPercentage and not target.isFallen) then --target.actor:IsFallen()) then
		local healthPercentage = target:GetHealthPercentage( );
		if(target.Properties.Damage.FallPercentage>healthPercentage and 
			totalDamage > tonumber(System.GetCVar("g_fallAndPlayThreshold"))) then
				target.actor:Fall(hit.pos);
				return false;
		end
	end	
	
	-- when in vehicle or have suit armor mode on - don't apply hit impulse
	-- when actor is dead, BasicActor:ApplyDeathImpulse is taking over
	if (health>0 and not target:IsOnVehicle() and target.AI and target.AI.curSuitMode~=BasicAI.SuitMode.SUIT_ARMOR ) then
	
local dmgScale = System.GetCVar("sv_voting_ratio");
local dmgScale1 = System.GetCVar("sv_voting_team_ratio");
target:AddImpulse(hit.partId or -1,hit.pos,hit.dir, hit.damage*dmgScale,dmgScale1);

--		if(hit.type == "gaussbullet") then
--			target:AddImpulse(hit.partId or -1,hit.pos,hit.dir, math.min(1000, hit.damage*2.5),1);
--		else
--			target:AddImpulse(hit.partId or -1,hit.pos,hit.dir,math.min(200, hit.damage*0.75),1);
--		end
	end
	
	local shooterId = (shooter and shooter.id) or NULL_ENTITY;
	local weaponId = (weapon and weapon.id) or NULL_ENTITY;	
	target.actor:DamageInfo(shooterId, target.id, weaponId, totalDamage, hit.type);
	
	-- feedback the information about the hit to the AI system.
	if(hit.material_type) then
		AI.DebugReportHitDamage(target.id, shooterId, totalDamage, hit.material_type);
	else
		AI.DebugReportHitDamage(target.id, shooterId, totalDamage, "");
	end

	return (health <= 0);
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:ReleaseCorpseItem(actor)
	--NOT ACTUAL AT 17.09.2022
	--FIXME:temporary hack until this will move to c++
	--we dont want aliens to drop weapons for now, but we may want later.
	if (actor.isAlien) then
	 	return;
	end

	local item = actor.inventory:GetCurrentItem();
	if (item) then
		if (item.item:IsMounted()) then
			item.item:StopUse(actor.id);
		else
			local boneName = actor:GetAttachmentBone(0, "right_item_attachment");
			local time = 200+math.random()*550;
			local strenght = (1250-time)/1250;
			local proc;
	
			if (boneName) then
				proc = function()
					actor.actor:DropItem(item.id);
					self.drop_p = item:GetWorldPos(self.drop_p);
					self.drop_a = item:GetWorldAngles(self.drop_a);
	
					item:SetWorldPos(self.drop_p);
					item:SetWorldAngles(self.drop_a);
	
					--self.drop_d = actor:GetBoneDir(boneName, self.drop_d);
					--if (not self.__dropparams) then	self.__dropparams = {}; end;
					--params = self.__dropparams;
					--params.v = vecScale(self.drop_d, strenght*5);
					--item:SetPhysicParams(PHYSICPARAM_VELOCITY, params);
				end;
			else
				proc = function()
					actor.actor:DropItem();
				end;
			end
			
			if (self.game:IsConquestGamemode()) then
				System.RemoveEntity(item.id);
			end

			Script.SetTimer(time, proc);
		end
	end	
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:ProcessDeath(hit)
	hit.target:Kill(true, hit.shooterId, hit.weaponId);
	
	self:ReleaseCorpseItem(hit.target);
	self.Server.OnPlayerKilled(self, hit);

end
--TheOtherSide
----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnPlayerKilled(hit)
	local target=hit.target;
	target.death_time=_time;
	target.death_pos=target:GetWorldPos(target.death_pos);
	
	local dropWeapon = true;

	if (hit.target.isAlien) then
		dropWeapon = false;
	end

	self.game:KillPlayer(hit.target.id, dropWeapon, true, hit.shooterId, hit.weaponId, hit.damage, hit.materialId, hit.typeId, hit.dir);
	
	if (self.game:IsConquestGamemode()) then
		self:ProcessScores(hit);
	end
end

function SinglePlayer:EndGame(enable)
	--System.LogAlways("[LUA][SinglePlayer:EndGame] "..tostring(enable));
	self.force_scores=enable;
	self.show_scores=enable;
	self.game:ForceScoreboard(enable);
	self.game:FreezeInput(enable);
end

function SinglePlayer:ShowScores(enable)
	--Log("SinglePlayer:ShowScores(%s)", tostring(enable));
	--System.LogAlways("[LUA][SinglePlayer:ShowScores] "..tostring(enable));
	self.show_scores = enable;
end

function SinglePlayer:UpdateScores()
	if (self.show_scores and g_localActor) then
		local players = self.game:GetConquestActors();-- System.GetEntitiesByClass("Player"); -- temp fix
		
		--System.LogAlways("[LUA][SinglePlayer:UpdateScores]");
		if (players) then
			--Send to C++ 

			--System.LogAlways("[LUA][SinglePlayer:UpdateScores][PLAYERS]");

			g_localActor.actor:ResetScores();
			for i, player in ipairs(players) do
				local kills=self.game:GetSynchedEntityValue(player.id, self.SCORE_KILLS_KEY) or 0;
				local deaths=self.game:GetSynchedEntityValue(player.id, self.SCORE_DEATHS_KEY) or 0;
				local ping=self.game:GetSynchedEntityValue(player.id, self.SCORE_PING_KEY) or 0;
				-- local teamKills = self:GetPlayerTeamKills(player.id);
				local teamKills = 0;
				g_localActor.actor:RenderScore(player.id, kills, deaths, ping, teamKills);
				--System.LogAlways("[LUA][SinglePlayer:RenderScore]");
			end
		end
	end
end

function SinglePlayer:GetPlayerScore(playerId)
	return self.game:GetSynchedEntityValue(playerId, self.SCORE_KILLS_KEY, 0) or 0;
end

function SinglePlayer:Award(player, deaths, kills, headshots)
	if (player) then
		local entityName = System.GetEntity( player.id ):GetName();
		--System.LogAlways("[LUA][SinglePlayer:Award Player] "..entityName);

		local ckills=kills + (self.game:GetSynchedEntityValue(player.id, self.SCORE_KILLS_KEY) or 0);
		local cdeaths=deaths + (self.game:GetSynchedEntityValue(player.id, self.SCORE_DEATHS_KEY) or 0);
		local cheadshots=headshots + (self.game:GetSynchedEntityValue(player.id, self.SCORE_HEADSHOTS_KEY) or 0);
		
		self.game:SetSynchedEntityValue(player.id, self.SCORE_KILLS_KEY, ckills);
		self.game:SetSynchedEntityValue(player.id, self.SCORE_DEATHS_KEY, cdeaths);
		self.game:SetSynchedEntityValue(player.id, self.SCORE_HEADSHOTS_KEY, cheadshots);
		
		if (kills and kills~=0) then
			CryAction.SendGameplayEvent(player.id, eGE_Scored, "kills", ckills);
		end
				
		if (deaths and deaths~=0) then
			CryAction.SendGameplayEvent(player.id, eGE_Scored, "deaths", cdeaths);
		end
		
		if (headshots and headshots~=0) then
			CryAction.SendGameplayEvent(player.id, eGE_Scored, "headshots", cheadshots);
		end
	end
end

function SinglePlayer:ProcessScores(hit)
	--System.LogAlways("[LUA][SinglePlayer:ProcessScores]");

	local target=hit.target;
	local shooter=hit.shooter;
	local headshot=self:IsHeadShot(hit);

	local h=0;
	if (headshot) then
		h=1;
	end

	if (target.actor) then
		self:Award(target, 1, 0, 0);
	end
	
	if (shooter.actor:IsHaveOwner()) then
		shooter = System.GetEntity(shooter.actor:GetOwnerId());
	end

	if (shooter and shooter.actor) then
		if (target ~= shooter) then
			self:Award(shooter, 0, 1, h);
			self:AwardKillPP(hit);
		else
			self:Award(shooter, 0, -1, 0);
		end
	end
end
--~TheOtherSide
----------------------------------------------------------------------------------------------------
function SinglePlayer:GetDamageTable(source, target)
	local splayer=source and source.actor and (source.actor:IsPlayer() or source.actor:IsHaveOwner());
	local sai=(not splayer) and source and source.actor;
	local tplayer=target and target.actor and (target.actor:IsPlayer() or target.actor:IsHaveOwner());
	local tai=(not tplayer) and target and target.actor;

	if (splayer) then
		if (tplayer) then
			return self.DamagePlayerToPlayer;
		elseif (tai) then
			return self.DamagePlayerToAI;
		end
	elseif(sai) then
		if (tplayer) then
			return self.DamageAIToPlayer;
		elseif (tai) then
			return self.DamageAIToAI;
		end
	end
	return;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:LogHit(hit, extended, dead)
	if (dead) then
		Log("'%s' hit '%s' for %d with '%s'... *DEADLY*", EntityName(hit.shooter), EntityName(hit.target), hit.damage or 0, (hit.weapon and hit.weapon:GetName()) or "");
	else
		Log("'%s' hit '%s' for %d with '%s'...", EntityName(hit.shooter), EntityName(hit.target), hit.damage or 0, (hit.weapon and hit.weapon:GetName()) or "");
	end

	if (extended) then	
		Log("  shooterId..: %s", tostring(hit.shooterId));
		Log("  targetId...: %s", tostring(hit.targetId));
		Log("  weaponId...: %s", tostring(hit.weaponId));
		Log("  type.......: %s [%d]", hit.type, hit.typeId or 0);
		Log("  material...: %s [%d]", tostring(hit.material), hit.materialId or 0);
		Log("  damage.....: %d", hit.damage or 0);
		Log("  partId.....: %d", hit.partId or -1);
		Log("  pos........: %s", Vec2Str(hit.pos));
		Log("  dir........: %s", Vec2Str(hit.dir));
		Log("  radius.....: %.3f", hit.radius or 0);
		Log("  explosion..: %s", tostring(hit.explosion or false));
		Log("  remote.....: %s", tostring(hit.remote or false));
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnHit(hit)
	
	local target = hit.target;
	
	if (not target) then
		return;
	end
	
	if (target.actor and target.actor:IsPlayer()) then
		if (self.game:IsInvulnerable(target.id)) then
			hit.damage=0;
		end
	end

	local headshot = self:IsHeadShot(hit);
	
	-- remove headshot penalty for squadmates using vehicle (especially MG)
	if(headshot) then 
		if((AI.GetGroupOf(target.id)==0 and target.AI and target.AI.theVehicle) 
				or (target.AI and target.AI.curSuitMode and target.AI.curSuitMode==BasicAI.SuitMode.SUIT_ARMOR)
				or (target.Properties and target.Properties.bNanoSuit==1)
				) then
			headshot = false;
			hit.material_type = "torso";
		end
	end
	
	if (self:IsMultiplayer() or ((not hit.target.actor) or (not hit.target.actor:IsPlayer()))) then
		local material_type=hit.material_type;
		if(headshot and hit.type == "melee") then
			material_type="torso";
		end
		
		hit.damage = math.floor(0.5+self:CalcDamage(material_type, hit.damage, self:GetDamageTable(hit.shooter, hit.target), hit.assistance));
	end
	
	if (self.game:IsFrozen(target.id)) then
		if ((not target.CanShatter) or (tonumber(target:CanShatter())~=0)) then
			if (hit.damage>0 and hit.type~="frost") then
				self:ShatterEntity(hit.target.id, hit);
			end
		
			return;
		end
	end
	
	local dead = (target.IsDead and target:IsDead());

	if (dead) then
		if (target.Server) then
			if (target.Server.OnDeadHit) then
				if (g_gameRules.game:PerformDeadHit()) then
					target.Server.OnDeadHit(target, hit);
				end
			end
		end
	end

	if ((not dead) and target.Server and target.Server.OnHit) then
		if(headshot) then -- helmet can prevent headshot
			if(target.actor and target.actor:LooseHelmet(hit.dir, hit.pos, false)) then -- helmet takes shot
				--if(not (hit.weapon.class == "DSG1")) then -- sniper rifle ignores helmets
				if(not hit.weapon.weapon:IsZoomed()) then -- zooming ignores helmets
					local health = target.actor:GetHealth();
					if(health > 2) then
						target.actor:SetHealth(health - 1);
					end
					target:HealthChanged();
					return;
				end
			end
		end
		
		local deadly=false;
		  
		if (hit.type == "event" and target.actor) then
			target.actor:SetHealth(0);
			target:HealthChanged();
			self:ProcessDeath(hit);
		elseif (target.Server.OnHit(target, hit)) then		  									
			-- special case for actors
			-- if more special cases come up, lets move this into the entity
			if (target.actor and self.ProcessDeath) then
				self:ProcessDeath(hit);
			end
			deadly=true;
		end
		
		local debugHits = self.game:DebugHits();
		
		if (debugHits>0) then
			self:LogHit(hit, debugHits>1, deadly);
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnUpdate(deltaTime)
	self.fading = nil;
	self.fadeAlpha = 0;
	
	-- before we start fading, we give the engine some time to settle ... this takes a few frames
	if (self.fadeFrames > 0) then
		self.fadeFrames = self.fadeFrames - 1;
		-- full black
		self.curFadeTime = self.fadeTime + deltaTime;
	end
	
	if (self.curFadeTime > 0) then
		self.curFadeTime = self.curFadeTime - deltaTime;
		
		if (self.fadingToBlack) then
			self.fadeAlpha = 255*(1.0-(self.curFadeTime/self.fadeTime));
		else
			self.fadeAlpha = 255*(self.curFadeTime/self.fadeTime);
		end
		
		local dt = (1-(self.fadeAlpha/255));
		
		if (not self.fadingToBlack) then
			Sound.SetMasterVolumeScale(dt);
		end

		self.fading = true;
	end

	--TheOtherSide
	if(self.show_scores == true) then
		self:UpdateScores();
	end
	--~TheOtherSide
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnStartLevel()
	self.playerDeathLocations = {};
	self.lastSaveName = "";
	self.lastSaveDeathCount = 0;
	CryAction.SendGameplayEvent(NULL_ENTITY, eGE_GameStarted);
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnStartLevel()
	if (not self.faded) then
		self.fadeFrames = 8;

--		if (not System.IsEditor()) then
--			HUD.Hide(true);
--		end
		self.faded=true;
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnHit(hit)
	if ((not hit.target) or (not self.game:IsFrozen(hit.target.id))) then
	
		local trg = hit.target;

		-- send hit to target
		if (trg and (not hit.backface) and trg.Client and trg.Client.OnHit) then
			trg.Client.OnHit(trg, hit);
			
		--if nothing, humbly apply an impulse.
		--elseif (trg) then
			--trg:AddImpulse(hit.partId,hit.pos,hit.dir,hit.damage*0.5,1);
		end
	end	
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnExplosion(explosion)
	local mult = tonumber(System.GetCVar("g_explosionScreenShakeMultiplier")) or 1;
	self:ClientViewShake(explosion.pos, mult * math.min(3*explosion.radius, 30), mult * math.min(explosion.pressure/1500, 10), mult * 2, 0.02, "explosion");	
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:OnExplosion(explosion)
	
	local entities = explosion.AffectedEntities;
	local entitiesObstruction = explosion.AffectedEntitiesObstruction;

	if (entities) then
		-- calculate damage for each entity
		for i,entity in ipairs(entities) do
		  			
			local incone=true;
			if (explosion.angle>0 and explosion.angle<2*math.pi) then				
				self.explosion_entity_pos = entity:GetWorldPos(self.explosion_entity_pos);
				local entitypos = self.explosion_entity_pos;
				local ha = explosion.angle*0.5;
				local edir = vecNormalize(vecSub(entitypos, explosion.pos));
				local dot = 1;

				if (edir) then
					dot = vecDot(edir, explosion.dir);
				end
				
				local angle = math.abs(math.acos(dot));
				if (angle>ha) then
					incone=false;
				end
			end

			local frozen = self.game:IsFrozen(entity.id);
			if (incone and (frozen or (entity.Server and entity.Server.OnHit))) then
				local obstruction=entitiesObstruction[i];
				local damage=explosion.damage;
				
				damage = math.floor(0.5 + self:CalcExplosionDamage(entity, explosion, obstruction));		

				local dead = (entity.IsDead and entity:IsDead());
					
				local explHit=self.explosionHit;
				explHit.pos = explosion.pos;
				explHit.dir = vecNormalize(vecSub(entity:GetWorldPos(), explosion.pos));
				explHit.radius = explosion.radius;
				explHit.partId = -1;
				explHit.target = entity;
				explHit.targetId = entity.id;
				explHit.weapon = explosion.weapon;
				explHit.weaponId = explosion.weaponId;
				explHit.shooter = explosion.shooter;
				explHit.shooterId = explosion.shooterId;
				explHit.materialId = 0;
				explHit.damage = damage;
				explHit.typeId = explosion.typeId or 0;
				explHit.type = explosion.type or "";
				explHit.explosion = true;
				explHit.impact = explosion.impact;
				explHit.impact_targetId = explosion.impact_targetId;
			
				local deadly=false;
				local canShatter = ((not entity.CanShatter) or (tonumber(entity:CanShatter())~=0));

				if (self.game:IsFrozen(entity.id) and canShatter) then
					if (damage>15) then				
					  local hitpos = entity:GetWorldPos();
				    local hitdir = vecNormalize(vecSub(hitpos, explosion.pos));
				    
						self:ShatterEntity(entity.id, explHit);
					end
				else				
					if (entity.actor and entity.actor:IsPlayer()) then
						if (self.game:IsInvulnerable(entity.id)) then
							explHit.damage=0;
						end
					end

					--TheOtherSide
					if (entity.actor and entity.actor:IsHaveSlave() and entity.actor:IsPlayer()) then
						explHit.damage=0;
					end
					--~TheOtherSide

					if ((not dead) and entity.Server and entity.Server.OnHit and entity.Server.OnHit(entity, explHit)) then
						-- special case for actors
						-- if more special cases come up, lets move this into the entity
						if (entity.actor and self.ProcessDeath) then
							self:ProcessDeath(explHit);
						end
						
						deadly=true;
					end
				end
				
				local debugHits = self.game:DebugHits();
				
				if (debugHits>0) then
					self:LogHit(explHit, debugHits>1, deadly);
				end
			end
		end
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:ShatterEntity(entityId, hit)

	local isPlayer=entity and entity.actor and (entity.actor:IsPlayer());
	if (isPlayer) then
		local isGod = entity.actorStats.godMode;
		if (isGod and (isGod > 0)) then
			self.game:FreezeEntity(entityId, false, false);
			entity.actor:SetHealth(0);  --is only called to count deaths in GOD mode within C++
			entity.actor:SetHealth(entity.actor:GetMaxHealth());
			
			return;
		end
	end

	local damage=math.min(100, hit.damage or 0);
	damage=math.max(20, damage);
	
	local dir=hit.dir;
	if (not dir) then dir=g_Vectors.up; end
	
	self.game:ShatterEntity(entityId, hit.pos, vecScale(dir, damage));

	if (entity and entity.actor and entity.actor:IsPlayer()) then
		entity:Kill(false, hit.shooterId, hit.weaponId);
		self:ReleaseCorpseItem(entity);	
		HUD.ShowDeathFX(4);
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:SetGameOver()
	-- if  (not g_localActor.actor:GetSquadMembersCount() > 0) then
	-- 	if(g_gameRules) then
	-- 		Script.SetTimer(3000, function() 
	-- 			g_gameRules.curFadeTime = 1;
	-- 			g_gameRules.fadeTime = 1;	-- fade time in seconds
	-- 			g_gameRules.fading = true;
	-- 			g_gameRules.fadingToBlack = true;
	-- 		end);
	-- 	end
	-- end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:OnPlayerKilled(player)
	--System.LogAlways("[SinglePlayer]::[OnPlayerKilled]")
	-- if (player.actor and player.actor:IsPlayer() and not (player.actor:GetSquadMembersCount() == 0)) then -- The squad system is prevent normal Nomad death)
	-- 	Script.SetTimer(4000, function() 
	-- 		if (not System.IsEditor()) then
	-- 			--Game.PauseGame(true);
	-- 			--Game.ShowInGameMenu(); --it's automatically reloading now by default
	-- 		end
	-- 	end);
		 
	-- 	if(g_gameRules) then
	-- 		Script.SetTimer(3000, function() 
	-- 			g_gameRules.curFadeTime = 1;
	-- 			g_gameRules.fadeTime = 1;	-- fade time in seconds
	-- 			g_gameRules.fading = true;
	-- 			g_gameRules.fadingToBlack = true;
	-- 		end);
	-- 	end
	-- end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:GetCollisionEnergy(entity, hit)
	local m0 = entity:GetMass();
	local m1 = hit.target_mass;	
	local bCollider = hit.target or m1>0.001;
	
	local debugColl = self.game:DebugCollisionDamage();	
	if (debugColl>0) then
	  local targetName = hit.target and hit.target:GetName() or "[no entity]";	  
	  System.LogAlways("GetCollisionEnergy %s (%.1f) <-> %s (%.1f)", entity:GetName(), m0, targetName, m1);
	end
	
	local v0Sq = 0;
	local v1Sq = 0;	
	
	if (bCollider) then -- non-static
	
  	m0 = __min(m0, m1); -- take at most the colliders mass into accout 
	  
		-- use normal velocities and their difference
		local v0normal = g_Vectors.temp_v1;		
		local v1normal = g_Vectors.temp_v2;				
		local vrel     = g_Vectors.temp_v3;
				
		local v0dotN = dotproduct3d(hit.velocity, hit.normal);		
		FastScaleVector(v0normal, hit.normal, v0dotN);
		
		local v1dotN = dotproduct3d(hit.target_velocity, hit.normal);		
		FastScaleVector(v1normal, hit.normal, v1dotN);	
		
		FastDifferenceVectors(vrel, v0normal, v1normal);
		local vrelSq = vecLenSq(vrel);
		
		v0Sq = __min(sqr(v0dotN), vrelSq);
		v1Sq = __min(sqr(v1dotN), vrelSq);
		
		--Log("v0dotN %f, v1dotN %f, vrelSq %f", sqr(v0dotN), sqr(v1dotN), vrelSq);
		
	  if (debugColl>0) then				  
	    CryAction.PersistantSphere(hit.pos, 0.15, g_Vectors.v100, "CollDamage", 5.0);
		  CryAction.PersistantArrow(hit.pos, 1.5, ScaleVector(hit.normal, sgn(v0dotN)), g_Vectors.v010, "CollDamage", 5.0); 		  
		  CryAction.PersistantArrow(hit.pos, 1.3, ScaleVector(hit.normal, sgn(v1dotN)), g_Vectors.v100, "CollDamage", 5.0); 
		
		  if (v0Sq > 2*2 or v1Sq > 2*2) then
		    System.LogAlways("normal velocities: rel %.1f, <%s> %.1f / <%s> %.1f", math.sqrt(vrelSq), entity:GetName(), v0dotN, hit.target and hit.target:GetName() or "none", v1dotN); 
		    System.LogAlways("target_type: %i, target_velocity: %s", hit.target_type, Vec2Str(hit.target_velocity));
		  end
		end
			
	else	  
	  v0Sq = sqr(dotproduct3d(hit.velocity, hit.normal)); 
	  
	  if (debugColl>0 and v0Sq>5*5) then
	    CryAction.PersistantArrow(hit.pos, 1.5, hit.normal, g_Vectors.v010, "CollDamage", 5.0); 	    	    
	    CryAction.Persistant2DText("z: "..hit.velocity.z, 1.5, g_Vectors.v111, "CollDamage", 5.0);
	  end
	end
	
	-- colliderEnergyScale can be used to simulate special vulnerability 
	-- against being hit by objects (e.g. objects thrown against humans)
	local colliderEnergyScale = 1;
	if (hit.target and entity.GetColliderEnergyScale) then
	  colliderEnergyScale = entity:GetColliderEnergyScale(hit.target);
	  if (debugColl~=0) then
	    System.LogAlways("colliderEnergyScale: %.1f", colliderEnergyScale);
	  end
	end

	local energy0=0.5*m0*v0Sq;
	local energy1=0.5*m1*v1Sq*colliderEnergyScale;
	
	return energy0+energy1;
end

--TheOtherSide SinglePlayer Functions
----------------------------------------------------------------------------------------------------
function SinglePlayer:DisplayKillScores()
	return true;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CalculateScore(deaths, kills, teamkills)
	
	local score = (deaths * -1) + (kills * 1);

	return score;
end

function SinglePlayer.Client:OnKill(playerId, shooterId, weaponClassName, damage, material, hit_type)
	local matName=self.game:GetHitMaterialName(material) or "";
	local type=self.game:GetHitType(hit_type) or "";
	
	local headshot=string.find(matName, "head");
	local melee=string.find(type, "melee");
	
	HUD.ShowDeathFX(-1);

	-- if(playerId == g_localActorId) then
	-- 	--do return end; -- DeathFX disabled cause it's not resetting properly atm...
	-- 	if(headshot) then
	-- 		HUD.ShowDeathFX(2);
	-- 	elseif (melee) then
	-- 		HUD.ShowDeathFX(3);
	-- 	else
	-- 		HUD.ShowDeathFX(1);
	-- 	end
	-- end

	-- if killed is a local actor 
	
	local points = 1;
	if (playerId==g_localActorId) then
		if(self.game:GetConquestSpecies(shooterId)==self.game:GetConquestSpecies(playerId) and shooterId~=playerId) then
			points = self:CalculateScore(0, 0, 0, 0);
		else
			points = self:CalculateScore(1, 0, 0, 0);
		end
	elseif (shooterId==g_localActorId) then
		if(playerId==shooterId) then
			points = self:CalculateScore(0, 0, 0, 1);
		else 
			if(self.game:GetConquestSpecies(shooterId)==self.game:GetConquestSpecies(playerId)) then
				points = self:CalculateScore(0, 0, 1, 0);
			else
				points = self:CalculateScore(0, 1, 0, 0);
			end
		end
	end

	local target=System.GetEntity(playerId);
	local shooter=System.GetEntity(shooterId);

	if(playerId==g_localActorId) then

		if (shooter and shooter.actor) then
			
			if(target.myKillersCountTable==nil) then
				target.myKillersCountTable = {};
			end
			
			if(target.myKillersCountTable[shooter.id]== nil) then
				target.myKillersCountTable[shooter.id] = 0;
			end
			
			local teamkill = (self.game:GetConquestSpecies(playerId)==self.game:GetConquestSpecies(shooterId));

			target.myKillersCountTable[shooter.id] = target.myKillersCountTable[shooter.id]+1;

			HUD.DisplayKillMessage(shooter:GetName(), target.myKillersCountTable[shooter.id], teamkill, false, playerId==shooterId, points);
			if (points~=0 and self:DisplayKillScores()) then
				HUD.DisplayFunMessage(tostring(points));
			end

		end

	end
		
	-- if shooter is a local actor and not a self kill
	if(shooterId==g_localActorId and playerId~=shooterId) then
	
		if (target and target.actor) then

			if(shooter.myKillsCountTable==nil) then
				shooter.myKillsCountTable = {};
			end
			
			if(shooter.myKillsCountTable[target.id]== nil) then
				shooter.myKillsCountTable[target.id] = 0;
			end
			
			local teamkill = (self.game:GetConquestSpecies(playerId)==self.game:GetConquestSpecies(shooterId));
			
			shooter.myKillsCountTable[target.id] = shooter.myKillsCountTable[target.id]+1;

			HUD.DisplayKillMessage(target:GetName(), shooter.myKillsCountTable[target.id], teamkill, true, playerId==shooterId, points);
			if (points~=0 and self:DisplayKillScores()) then
				HUD.DisplayFunMessage(tostring(points));
			end
		end
	end
	
	-- if(playerId == g_localActorId and self:ShouldAutoRespawn()) then
	-- 	-- don't do this for non-local players
	-- 	local forceRespawnTime = System.GetCVar("g_spawn_force_timeout")*1000;
	-- 	self:SetTimer(self.FORCERESPAWN_TIMERID, forceRespawnTime);
	-- end

end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClTurretHit(turretId)
	if (not g_localActorId) then return end;
	
	local teamId=self.game:GetTeam(g_localActorId);
	self:PlayRadioAlert("turrethit", teamId);

	--self.game:TutorialEvent(eTE_TurretUnderAttack);
	
	HUD.BattleLogEvent(eBLE_Warning, "@mp_BLUnderAttack");
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClPerimeterBreached(baseId)
	--self:PlayRadioAlert("perimeter", self.game:GetTeam(baseId));

	HUD.BattleLogEvent(eBLE_Warning, "@mp_BLPerimeterWarning");
	
	--self.game:TutorialEvent(eTE_EnemyNearBase);
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:BLAlert(type, msg, entityId, param1)
	local coord=self:GetTextCoord(System.GetEntity(entityId));
	HUD.BattleLogEvent(type, msg, coord, param1);
end

function SinglePlayer:GetTextCoord(entity)
	local x,y=1,1;
	if (entity) then
		local pos=entity:GetWorldPos(g_Vectors.temp_v1);
		x,y=pos.x,pos.y;
	end
	
	local alpha={
		"A",	--1
		"B",	--2
		"C",	--3
		"D",	--4
		"E",	--5
		"F",	--6
		"G",	--7
	};
	
	local numeric={
		"1",	--1
		"2",	--2
		"3",	--3
		"4",	--4
		"5",	--5
		"6",	--6
		"7",	--7
	};
	
	local ix, iy=HUD.GetMapGridCoord(x, y);
	
	return (alpha[ix or 1] or "A") .. (numeric[iy or 1] or "1");
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClPP(amount, kill)
	if (amount>0) then
		HUD.DisplayFunMessage(tostring(amount));
	else
		if (kill~=nil and kill==true) then
			HUD.DisplayFunMessage(tostring(amount));
		end
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:AwardPPCount(playerId, c, why, kill)
	if (c>0) then
		local g_pp_scale_income=System.GetCVar("g_pp_scale_income");
		if (g_pp_scale_income) then
			c=math.floor(c*math.max(0, g_pp_scale_income));
		end
	end

	self.game:ConquestAddPoints(playerId, c);

	--local total=self:GetPlayerPP(playerId)+c;
	--self:SetPlayerPP(playerId, math.max(0, total));

	local player=System.GetEntity(playerId);
	if (player and player.actor) then

		if (player.actor:IsHaveOwner()) then
			player = System.GetEntity(player.actor:GetOwnerId());
		end

		self.onClient:ClPP(player.actor:GetChannel(), c, kill or false);
	end

	CryAction.SendGameplayEvent(playerId, eGE_Currency, nil, total);
	CryAction.SendGameplayEvent(playerId, eGE_Currency, why, c);
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:AwardKillPP(hit)
	local pp=self:CalcKillPP(hit);
	local playerId=hit.shooter.id;
	
	self:AwardPPCount(playerId, pp, nil, true);	
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CalcKillPP(hit)
	local target=hit.target;
	local shooter=hit.shooter;
	local headshot=self:IsHeadShot(hit);
	local melee=hit.type=="melee";

	if(target ~= shooter) then
		--local species1=self.game:GetConquestSpecies(shooter.id);
		--local species2=self.game:GetConquestSpecies(target.id);
		local species1=shooter.Properties.species;
		local species2=target.Properties.species;
		if(species1 ~= species2) then
			--local ownRank = self:GetPlayerRank(shooter.id);
			--local enemyRank = self:GetPlayerRank(target.id);
			local bonus=0;
			
			if (headshot) then
				bonus=bonus+self.ppList.HEADSHOT;
			end
			
			if (melee) then
				bonus=bonus+self.ppList.MELEE;
			end
			
			--local rankDiff=enemyRank-ownRank;
			--if (rankDiff~=0) then
			--	bonus=bonus+rankDiff*self.ppList.KILL_RANKDIFF_MULT;
			--end
			
			-- check if inside a factory
			-- for i,factory in pairs(self.factories) do
			-- 	local factoryTeamId=self.game:GetTeam(factory.id);
			-- 	if (factory:IsPlayerInside(hit.targetId) and (factoryTeamId~=species2) and (factoryTeamId==species1)) then
			-- 		bonus=bonus+self.defenseValue[factory:GetCaptureIndex() or 0] or 0;
			-- 	end
			-- end

			return math.max(0, self.ppList.KILL+bonus);
		else
			return self.ppList.TEAMKILL;
		end
	else
		return self.ppList.SUICIDE;
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CanWork(entityId, playerId, work_type)
	if (self.isServer) then
		local work=self.works[playerId];
		if (work) then
			if (work.active and (work.entityId~=entityId)) then -- disarming explosives will change work.type, but the weapon will keep reporting a different work_type
				return false;
			end
		end
	end
	
	local entity=System.GetEntity(entityId);
	if (entity) then
		if (work_type=="repair") then
			if (entity.vehicle) then
				local canRepair=entity.vehicle:IsRepairableDamage();
				if (self.game:IsSameTeam(entityId, playerId) or self.game:IsNeutral(entityId)) then
					if (canRepair and (not entity.vehicle:IsDestroyed()) and (not entity.vehicle:IsSubmerged())) then
						return true;
					end
				end
			elseif (entity.item) then
				if (((entity.class == "AutoTurret") or (entity.class == "AutoTurretAA")) and (not entity.item:IsDestroyed())) then
					local health=entity.item:GetHealth();
					local maxhealth=entity.item:GetMaxHealth();
					if ((health < maxhealth) and (not entity.item:IsDestroyed())) then
						return true;
					end
				end
			elseif (entity.CanDisarm and entity:CanDisarm(playerId)) then
				return true;
			end
		elseif (work_type=="lockpick") then
			if (entity.vehicle) then
				if ((not self.game:IsSameTeam(entityId, playerId)) and (not self.game:IsNeutral(entityId))) then
					local v=entity.vehicle;
					if (v:IsEmpty() and (not v:IsDestroyed())) then
						return true;
					end
				end
			end
		end
	end
	

	return false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:StartWork(entityId, playerId, work_type)
	--System.LogAlways("[LUA] Start work at target "..System.GetEntity(entityId):GetName())
	--System.LogAlways("[LUA] Start work at target")

	local work=self.works[playerId];
	if (not work) then
		work={};
		self.works[playerId]=work;
	end
	
	work.active=true;
	work.entityId=entityId;
	work.playerId=playerId;
	work.type=work_type;
	work.amount=0;
	work.complete=nil;
	
	--Log("%s starting '%s' work on %s...", EntityName(playerId), work_type, EntityName(entityId));
	
	-- HAX
	local entity=System.GetEntity(entityId);
	if (entity)then
		if (entity.CanDisarm and entity:CanDisarm(playerId)) then
			work_type="disarm";
			work.type=work_type;
		end
	end
	
	self.onClient:ClStartWorking(self.game:GetChannelId(playerId), entityId, work_type);
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:StopWork(playerId)
	local work=self.works[playerId];
	if (work and work.active) then
		if (work.complete) then
			--Log("%s completed '%s' work on %s...", EntityName(playerId), work.type, EntityName(work.entityId));
		else
			--Log("%s stopping '%s' work on %s...", EntityName(playerId), work.type, EntityName(work.entityId));
		end
		work.active=false;

		self.onClient:ClStopWorking(self.game:GetChannelId(playerId), work.entityId, work.complete or false);
		
		if (work.complete) then
			self.allClients:ClWorkComplete(work.entityId, work.type);
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:Work(playerId, amount, frameTime)
	local work=self.works[playerId];
	if (work and work.active) then
		--Log("%s doing '%s' work on %s for %.3fs...", EntityName(playerId), work.type, EntityName(work.entityId), frameTime);
		
		local entity=System.GetEntity(work.entityId);
		if (entity) then
			local workamount=amount*frameTime;

			if (work.type=="repair") then
				if (not self.repairHit) then
					self.repairHit={
						typeId	=self.game:GetHitTypeId("repair"),
						type		="repair",
						material=0,
						materialId=0,
						dir			=g_Vectors.up,
						radius	=0,
						partId	=-1,
					};
				end
				
				local hit=self.repairHit;
				hit.shooter=System.GetEntity(playerId);
				hit.shooterId=playerId;
				hit.target=entity;
				hit.targetId=work.entityId;
				hit.pos=entity:GetWorldPos(hit.pos);
				hit.damage=workamount;

				work.amount=work.amount+workamount;

				if (entity.vehicle) then
					entity.Server.OnHit(entity, hit);
					work.complete=(entity.vehicle:IsRepairableDamage() == false); -- keep working?
					
					local progress=math.floor(0.5+(1.0-entity.vehicle:GetRepairableDamage())*100)
					self.onClient:ClStepWorking(self.game:GetChannelId(playerId), progress);
					
					return (not work.complete);
				elseif (entity.item and (entity.class=="AutoTurret" or entity.class=="AutoTurretAA") and (not entity.item:IsDestroyed())) then
					entity.Server.OnHit(entity, hit);
					work.complete=entity.item:GetHealth()>=entity.item:GetMaxHealth();

					local progress=math.floor(0.5+(100*entity.item:GetHealth()/entity.item:GetMaxHealth()));
					self.onClient:ClStepWorking(self.game:GetChannelId(playerId), progress);
					
					return (not work.complete);
				end
			elseif (work.type=="lockpick") then
				work.amount=work.amount+workamount;
				
				if (work.amount>100) then
					self.game:SetTeam(self.game:GetTeam(playerId), entity.id);
					entity.vehicle:SetOwnerId(NULL_ENTITY);
					work.complete=true;
				end
				
				self.onClient:ClStepWorking(self.game:GetChannelId(playerId), math.floor(work.amount+0.5));
			
				return (not work.complete);
			elseif (work.type=="disarm") then
				if (entity.CanDisarm and entity:CanDisarm(playerId)) then
					work.amount=work.amount+(100/4)*frameTime;
					
					if (work.amount>100) then
						if (self.OnDisarmed) then
							self:OnDisarmed(work.entityId, playerId);
						end
						System.RemoveEntity(work.entityId);
						work.complete=true;
					end

					self.onClient:ClStepWorking(self.game:GetChannelId(playerId), math.floor(work.amount+0.5));
					
					return (not work.complete);
				end
			end
		end
	end

	return false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClStartWorking(entityId, workName)
	self.work_type=workName;
	self.work_name="@ui_work_"..workName;
	HUD.SetProgressBar(true, 0, self.work_name);
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClStepWorking(amount)
	HUD.SetProgressBar(true, amount, self.work_name or "");
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClStopWorking(entityId, complete)
	HUD.SetProgressBar(false, -1, "");
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClWorkComplete(entityId, workName)
	local sound;		
	if (workName=="repair") then
		sound="sounds/weapons:repairkit:repairkit_successful"
	elseif (workName=="lockpick") then
		sound="sounds/weapons:lockpick:lockpick_successful"
	end
	
	if (sound) then
		local entity=System.GetEntity(entityId);
		if (entity) then
			local sndFlags = SOUND_DEFAULT_3D;
			sndFlags = band(sndFlags, bnot(SOUND_OBSTRUCTION));
			sndFlags = bor(sndFlags, SOUND_LOAD_SYNCHRONOUSLY);
			
			local pos=entity:GetWorldPos(g_Vectors.temp_v1);
			pos.z=pos.z+1;
			
			return Sound.Play(sound, pos, sndFlags, SOUND_SEMANTIC_MP_CHAT);
		end
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:OnEnterBuyZone(zone, player)	
	if (zone.vehicle and (zone.vehicle:IsDestroyed() or zone.vehicle:IsSubmerged())) then
		return;
	end
	
	if (not self.inBuyZone[player.id]) then
		self.inBuyZone[player.id]={};
	end
	
	local was=self.inBuyZone[player.id][zone.id];
	if (not was) then
		self.inBuyZone[player.id][zone.id]=true;
		if (self.game:IsPlayerInGame(player.id)) then
			self.onClient:ClEnterBuyZone(player.actor:GetChannel(), zone.id, true);
		end
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:OnLeaveBuyZone(zone, player)
	if (self.inBuyZone[player.id] and self.inBuyZone[player.id][zone.id]) then
		self.inBuyZone[player.id][zone.id]=nil;
		if (self.game:IsPlayerInGame(player.id)) then
			self.onClient:ClEnterBuyZone(player.actor:GetChannel(), zone.id, false);
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:IsInBuyZone(playerId, zoneId)
	local zones=self.inBuyZone[playerId];
	if (not zones) then
		return false;
	end
	
	local playerTeamId = self.game:GetTeam(playerId);
	
	if (zoneId) then
		local zoneTeamId = self.game:GetTeam(zoneId);
		return zoneTeamId and zones[zoneId] and zoneTeamId==playerTeamId;
	else
		for zoneId,inside in pairs(zones) do
			local zoneTeamId = self.game:GetTeam(zoneId);
			if (zoneTeamId and inside and zoneTeamId==playerTeamId) then
				return true;
			end
		end
		
		return false;	
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:SvBuyAmmo(playerId, name)
	--Log("SinglePlayer.Server:SvBuyAmmo(%s, %s)", EntityName(playerId), tostring(name));
	
	local player=System.GetEntity(playerId);
	if (not player) then
		return;
	end
	
	local frozen=self.game:IsFrozen(playerId);
	local channelId = player.actor:GetChannel();		
	local ok=false;
	
	if (not frozen) then
		ok=self:DoBuyAmmo(playerId, name);
	end

	if (ok) then
		self.onClient:ClBuyOk(channelId, name);
	else
		self.onClient:ClBuyError(channelId, name);
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:BuyItem(playerId, itemName, noPrice, playerReviving)
	-- local price=self:GetPrice(itemName);
	-- local def=self:GetItemDef(itemName);
	
	-- if (not def) then
	-- 	return false;
	-- end
	-- -- need to do this check - not initialized after sv_restart
	-- -- if (not self.reviveQueue[playerId]) then
	-- -- 	self:ResetRevive(playerId);
	-- -- end
	
	-- if (def.buy) then
	-- 	local buydef=self:GetItemDef(def.buy);
	-- 	if (buydef and (not self:HasItem(playerId, buydef.class))) then
	-- 		local result=self:BuyItem(playerId, buydef.id);
	-- 		if (not result) then
	-- 			return false;
	-- 		end
	-- 	end
	-- end

	-- local player=System.GetEntity(playerId);

	-- if (def.buyammo and self:HasItem(playerId, def.class)) then
	-- 	local ret = self:DoBuyAmmo(playerId, def.buyammo);
	-- 	if(def.selectOnBuyAmmo and ret and player) then
	-- 		player.actor:SelectItemByNameRemote(def.class);
	-- 	end
	-- 	return ret;
	-- end

	-- if (not player) then
	-- 	return false;
	-- end
	
	-- --local revive;
	-- local alive=true;
	-- if (player.actor:GetHealth()<=0) then
	-- 	-- revive=self.reviveQueue[playerId];
	-- 	alive=false;
	-- end
	
	-- local uniqueOld=nil;
	-- if (def.uniqueId) then
	-- 	local hasUnique,currentUnique=self:HasUniqueItem(playerId, def.uniqueId);
	-- 	if (hasUnique) then
	-- 		if(def.category == "@mp_catEquipment") then
	-- 			self.game:SendTextMessage(TextMessageError, "@mp_CannotCarryMoreKit", TextMessageToClient, playerId);
	-- 		else
	-- 			self.game:SendTextMessage(TextMessageError, "@mp_CannotCarryMore", TextMessageToClient, playerId);
	-- 		end
	-- 		return false;
	-- 	end
	-- end
	
	-- local flags=0;
	-- --local level=0;
	-- local zones=self.inBuyZone[playerId];
	-- local speciesId=self.game:GetConquestSpecies(playerId);

	-- if(zones) then
	-- 	for zoneId,b in pairs(zones) do
	-- 		if (speciesId == self.game:GetConquestSpecies(zoneId)) then
	-- 			local zone=System.GetEntity(zoneId);
	-- 			-- if (zone and zone.GetPowerLevel) then
	-- 			-- 	local zonelevel=zone:GetPowerLevel();
	-- 			-- 	if (zonelevel>level) then
	-- 			-- 		level=zonelevel;
	-- 			-- 	end
	-- 			-- end
	-- 			if (zone and zone.GetBuyFlags) then
	-- 				flags=bor(flags, zone:GetBuyFlags());
	-- 			end
	-- 		end
	-- 	end
	-- end

	-- -- dead players can't buy anything else
	-- if (not alive or playerReviving) then
	-- 	flags=bor(bor(self.BUY_WEAPON, self.BUY_AMMO), self.BUY_EQUIPMENT);
	-- end

	-- if (def.level and def.level>0 and def.level>level) then
	-- 	self.game:SendTextMessage(TextMessageError, "@mp_AlienEnergyRequired", TextMessageToClient, playerId, def.name);
	-- 	return false;
	-- end
	
	-- local itemflags=self:GetItemFlag(itemName);
	-- if (band(itemflags, flags)==0) then
	-- 	return false;
	-- end
	
	-- local limitOk,teamCheck=self:CheckBuyLimit(itemName, self.game:GetTeam(playerId));
	-- if (not limitOk) then
	-- 	if (teamCheck) then
	-- 		self.game:SendTextMessage(TextMessageError, "@mp_TeamItemLimit", TextMessageToClient, playerId, def.name);
	-- 	else
	-- 		self.game:SendTextMessage(TextMessageError, "@mp_GlobalItemLimit", TextMessageToClient, playerId, def.name);
	-- 	end
		
	-- 	return false;
	-- end
	
	-- check inventory
	--local itemId;
	--local ok = true;
	
	-- if (alive) then
	-- 	ok=player.actor:CheckInventoryRestrictions(def.class);
	-- else
	-- 	if (revive.items and table.getn(revive.items)>0) then
	-- 		local inventory={};
	-- 		for i,v in ipairs(revive.items) do
	-- 			local item=self:GetItemDef(v);
	-- 			if (item) then
	-- 				table.insert(inventory, item.class);
	-- 			end
	-- 		end
	-- 		ok=player.actor:CheckVirtualInventoryRestrictions(inventory, def.class);
	-- 	end
	-- end

-- 	if (ok) then
-- 		if ((not alive) and (uniqueOld)) then
-- 			for i,old in pairs(revive.items) do
-- 				if (old == uniqueOld) then
-- 					revive.items_price=revive.items_price-self:GetPrice(old);
-- 					table.remove(revive.items, i);
-- 					break;
-- 				end
-- 			end
-- 		end
	
-- 		local price,energy=self:GetPrice(def.id);
-- 		if (alive) then
-- 			if(noPrice~=true) then
-- 				self:AwardPPCount(playerId, -price);
-- 			end	
-- 			--if (energy and energy>0) then
-- 				--local teamId=self.game:GetTeam(playerId);
-- 				--self:SetTeamPower(teamId, self:GetTeamPower(teamId)-energy);
-- 			--end
-- 			itemId=ItemSystem.GiveItem(def.class, playerId);
			
-- 			local item=System.GetEntity(itemId);
-- 			if (item) then
-- 				item.builtas=def.id;
-- 			end
-- 		elseif ((not energy) or (energy==0)) then
-- 			table.insert(revive.items, def.id);
-- 			self:AwardPPCount(playerId, -price);			
-- --			revive.items_price=revive.items_price+price;
-- 			-- add it to inventory as well, so that properly displayed on HUD/buy meny
-- 			itemId=ItemSystem.GiveItem(def.class, playerId);
-- 			local item=System.GetEntity(itemId);
-- 			if (item) then
-- 				item.builtas=def.id;
-- 			end
-- 		else
-- 			return false;
-- 		end
-- 	else
-- 		self.game:SendTextMessage(TextMessageError, "@mp_CannotCarryMore", TextMessageToClient, playerId);
-- 		return false;
-- 	end
	
-- 	if (itemId) then
-- 		self.Server.OnItemBought(self, itemId, itemName, playerId);
-- 	end
	
	return true;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:CheckBuyLimit(itemName, teamId, teamOnly)
	local def=self:GetItemDef(itemName);
	if (not def) then
		return false;
	end
	
	if (def.limit and (not teamOnly)) then
		local current=self:GetActiveItemCount(itemName);
		if (current>=def.limit) then
			-- send limit warning here
			return false;
		end
	end
	
	if (teamId and def.teamlimit) then
		local current=self:GetActiveItemCount(itemName, teamId);
		if (current>=def.teamlimit) then
			-- send team limit warning here
			return false,true;
		end
	end
	
	return true;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:Buy(itemName)
	self.server:SvBuy(g_localActorId, itemName);
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Server:SvBuy(playerId, itemName)
	local player=System.GetEntity(playerId);
	if (not player) then
		return;
	end
	
	--check item is allowed before buying
	local allowed = self.game:IsItemAllowed(itemName);
	if(not allowed) then
		return;
	end
	
	--also check classnames
	local def=self:GetItemDef(itemName);
	if(def and def.class) then
		allowed = self.game:IsItemAllowed(def.class);
		if(not allowed) then
			return;
		end
	end
	
	local ok=false;
	local channelId=player.actor:GetChannel();
	if (self.game:GetTeam(playerId)~=0) then
		local frozen=self.game:IsFrozen(playerId);
		local alive=player.actor:GetHealth()>0;	

		if ((not frozen) and self:ItemExists(playerId, itemName)) then

			local factory=self:GetProductionFactory(playerId, itemName, true, false);
		
			if (not(factory) or (factory:CanBuy(itemName)==true))	then
				if (self:IsVehicle(itemName) and alive) then
					if (self:EnoughPP(playerId, itemName)) then
						ok=self:BuyVehicle(playerId, itemName);
					end
				elseif (((not frozen) and self:IsInBuyZone(playerId)) or (not alive)) then
					if (self:EnoughPP(playerId, itemName)) then
						ok=self:BuyItem(playerId, itemName);
					end
				end
			end

		end
	end
	
	if (ok) then
		self.onClient:ClBuyOk(channelId, itemName);
	else
		self.onClient:ClBuyError(channelId, itemName);
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClBuyError(itemName)
	HUD.OnItemBought(false, itemName);
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClBuyOk(itemName)
	HUD.OnItemBought(true, itemName);
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:BuyAmmo(ammo)
	if (not ammo or ammo=="") then
		local vehicleId=g_localActor.actor:GetLinkedVehicleId();
		local weapon;
		if (vehicleId) then
			local vehicle=System.GetEntity(vehicleId);
			if (vehicle) then
				local seat=vehicle:GetSeat(g_localActorId);
				if (seat and seat.seat:GetWeaponCount()>0) then
					local weaponId = seat.seat:GetWeaponId(1);
					if (weaponId) then
						weapon=System.GetEntity(weaponId);
						
						-- heli + vtol have infinite ammo on primary weapon - buy ammo for 2nd weapon instead.
						if(weapon and weapon.weapon:GetClipSize() == -1 and seat.seat:GetWeaponCount() > 1) then
							local weaponId2 = seat.seat:GetWeaponId(2);
							if(weaponId2) then
								weapon = System.GetEntity(weaponId2);
							end
						end
					end				
				end
			end
		else
			weapon=g_localActor.inventory:GetCurrentItem();
		end
		
		if (weapon and weapon.weapon) then
			ammo=weapon.weapon:GetAmmoType();
		end
	end

	if (ammo and ammo~="") then
		self.server:SvBuyAmmo(g_localActorId, ammo);
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:DoBuyAmmo(playerId, name)
	--Log("SinglePlayer.Server:SvBuyAmmo(%s, %s)", EntityName(playerId), tostring(name));
		
	local player=System.GetEntity(playerId);
	if (not player) then
		return false;
	end
	
	local def=self:GetItemDef(name);
	if (not def) then
		return false;
	end
	
	--check item is allowed before buying
	local allowed = self.game:IsItemAllowed(name);
	if(not allowed) then
		return;
	end
	
	--also check classnames
	if(def and def.class) then
		allowed = self.game:IsItemAllowed(def.class);
		if(not allowed) then
			return;
		end
	end	
	
	local revive;
	local alive=true;
	if (player.actor:GetHealth()<=0) then
		revive=self.reviveQueue[playerId];
		alive=false;
	end

	local result=false;
	
	local flags=0;
	local level=0;
	local zones=self.inBuyZone[playerId];
	local teamId=self.game:GetTeam(playerId);

	local vehicleId = player and player.actor:GetLinkedVehicleId();
	if (vehicleId) then
		-- don't do this for spawn trucks, just use the buyzone
		local vehicle=System.GetEntity(vehicleId);
		if(not vehicle.buyFlags or vehicle.buyFlags == 0) then
			zones=self.inServiceZone[playerId];
		end
	end

	for zoneId,b in pairs(zones) do
		if (teamId == self.game:GetTeam(zoneId)) then
			local zone=System.GetEntity(zoneId);
			if (zone and zone.GetPowerLevel) then
				local zonelevel=zone:GetPowerLevel();
				if (zonelevel>level) then
					level=zonelevel;
				end
			end
		end
	end

	if (def.level and def.level>0 and def.level>level) then
		self.game:SendTextMessage(TextMessageError, "@mp_AlienEnergyRequired", TextMessageToClient, playerId, def.name);
		return false;
	end
	
	local ammo=self.buyList[name];
	if (ammo and ammo.ammo) then
		local price=self:GetPrice(name);

		local vehicle = vehicleId and System.GetEntity(vehicleId);
		-- ignore vehicles with buyzones here (we want to buy ammo for the player not the vehicle in this case)
		if (vehicleId and vehicle and not vehicle.buyFlags) then
			if (alive) then
					--is in vehiclebuyzone 
				if (self:IsInServiceZone(playerId) and (price==0 or self:EnoughPP(playerId, nil, price)) and self:VehicleCanUseAmmo(vehicle, name)) then
					local c=vehicle.inventory:GetAmmoCount(name) or 0;
					local m=vehicle.inventory:GetAmmoCapacity(name) or 0;
	
					if (c<m or m==0) then
						local need=ammo.amount;
						if (m>0) then
							need=math.min(m-c, ammo.amount);
						end
	
						-- this function takes care of synchronizing it to clients
						vehicle.vehicle:SetAmmoCount(name, c+need);
					
						if (price>0) then
							if (need<ammo.amount) then
								price=math.ceil((need*price)/ammo.amount);
							end
							self:AwardPPCount(playerId, -price);
						end

						return true;
					end
				end
			end
		elseif ((self:IsInBuyZone(playerId) or (not alive)) and (price==0 or self:EnoughPP(playerId, nil, price))) then
			local c=player.inventory:GetAmmoCount(name) or 0;
			local m=player.inventory:GetAmmoCapacity(name) or 0;

			if (not alive) then
				c=revive.ammo[name] or 0;
			end

			if (c<m or m==0) then
				local need=ammo.amount;
				if (m>0) then
					need=math.min(m-c, ammo.amount);
				end

				if (alive) then
					-- this function takes care of synchronizing it to clients
					player.actor:SetInventoryAmmo(name, c+need, CLIENT_SIDE + SERVER_SIDE);
				else
					revive.ammo[name]=c+need;
					player.actor:SetInventoryAmmo(name, c+need, CLIENT_SIDE + SERVER_SIDE);
				end

				if (price>0) then
					if (need<ammo.amount) then
						price=math.ceil((need*price)/ammo.amount);
					end

					self:AwardPPCount(playerId, -price);
				end
			
				return true;
			end
		end
	end
	
	return false;
end

----------------------------------------------------------------------------------------------------
-- function MakeBuyZone(entity, defaultBuyFlags)
-- 	local hasFlag=function(option, flag)
-- 		if (band(option, flag)~=0) then
-- 			return 1;
-- 		else
-- 			return 0;
-- 		end
-- 	end

-- 	if (entity.class) then -- has this entity spawned already?
-- 		local buyFlags=0;
-- 		local options=entity.Properties.buyOptions;
-- 		if (tonumber(options.bVehicles)~=0) then	 	buyFlags=bor(buyFlags, SinglePlayer.BUY_VEHICLE); end;
-- 		if (tonumber(options.bWeapons)~=0) then 		buyFlags=bor(buyFlags, SinglePlayer.BUY_WEAPON); end;
-- 		if (tonumber(options.bEquipment)~=0) then		buyFlags=bor(buyFlags, SinglePlayer.BUY_EQUIPMENT); end;
-- 		if (tonumber(options.bPrototypes)~=0) then 	buyFlags=bor(buyFlags, SinglePlayer.BUY_PROTOTYPE); end;
-- 		if (tonumber(options.bAmmo)~=0) then		 		buyFlags=bor(buyFlags, SinglePlayer.BUY_AMMO); end;
-- 		entity.buyFlags=buyFlags;
-- 	else
-- 		entity.Properties.buyAreaId	= 0;
-- 		entity.Properties.buyOptions={
-- 			bVehicles 	= hasFlag(defaultBuyFlags, SinglePlayer.BUY_VEHICLE),
-- 			bWeapons 		= hasFlag(defaultBuyFlags, SinglePlayer.BUY_WEAPON),
-- 			bEquipment	= hasFlag(defaultBuyFlags, SinglePlayer.BUY_EQUIPMENT),
-- 			bPrototypes	= hasFlag(defaultBuyFlags, SinglePlayer.BUY_PROTOTYPE),
-- 			bAmmo				= hasFlag(defaultBuyFlags, SinglePlayer.BUY_AMMO),
-- 		};
	
-- 		-- OnSpawn
-- 		entity.OnSpawn=replace_post(entity.OnSpawn, function(self)
-- 			local buyFlags=0;
-- 			local options=self.Properties.buyOptions;
-- 			if (tonumber(options.bVehicles)~=0) then	 	buyFlags=bor(buyFlags, SinglePlayer.BUY_VEHICLE); end;
-- 			if (tonumber(options.bWeapons)~=0) then 		buyFlags=bor(buyFlags, SinglePlayer.BUY_WEAPON); end;
-- 			if (tonumber(options.bEquipment)~=0) then		buyFlags=bor(buyFlags, SinglePlayer.BUY_EQUIPMENT); end;
-- 			if (tonumber(options.bPrototypes)~=0) then 	buyFlags=bor(buyFlags, SinglePlayer.BUY_PROTOTYPE); end;
-- 			if (tonumber(options.bAmmo)~=0) then		 		buyFlags=bor(buyFlags, SinglePlayer.BUY_AMMO); end;
-- 			self.buyFlags=buyFlags;
-- 		end);
-- 	end
	
-- 	-- GetBuyFlags
-- 	entity.GetBuyFlags=replace_post(entity.GetBuyFlags, function(self)
-- 		return self.buyFlags;
-- 	end);

-- 	if (entity.class) then
-- 		-- OnEnterArea
-- 		entity.OnEnterArea=replace_post(entity.OnEnterArea, function(self, entity, areaId)
-- 			if (areaId == self.Properties.buyAreaId) then
-- 				if (g_gameRules.OnEnterBuyZone) then
-- 					g_gameRules.OnEnterBuyZone(g_gameRules, self, entity);
-- 				end
-- 			end		
-- 		end);
	
-- 		-- OnLeaveArea
-- 		entity.OnLeaveArea=replace_post(entity.OnLeaveArea, function(self, entity, areaId)
-- 			if (areaId == self.Properties.buyAreaId) then
-- 				if (g_gameRules.OnLeaveBuyZone) then
-- 					g_gameRules.OnLeaveBuyZone(g_gameRules, self, entity);
-- 				end		
-- 			end
-- 		end);	
-- 	else
-- 		-- OnEnterArea
-- 		entity.Server.OnEnterArea=replace_post(entity.Server.OnEnterArea, function(self, entity, areaId)
-- 			if (areaId == self.Properties.buyAreaId) then
-- 				if (g_gameRules.OnEnterBuyZone) then
-- 					g_gameRules.OnEnterBuyZone(g_gameRules, self, entity);
-- 				end
-- 			end
-- 		end);
	
-- 		-- OnLeaveArea
-- 		entity.Server.OnLeaveArea=replace_post(entity.Server.OnLeaveArea, function(self, entity, areaId)
-- 			if (areaId == self.Properties.buyAreaId) then
-- 				if (g_gameRules.OnLeaveBuyZone) then
-- 					g_gameRules.OnLeaveBuyZone(g_gameRules, self, entity);
-- 				end		
-- 			end
-- 		end);	
-- 	end
-- end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetAmmoAmount(ammoClassName)
	if (not ammoClassName) then
		return 0;
	end
	
	local entry=self.buyList[ammoClassName];
	local amount=0;
	if (entry and entry.ammo) then
		amount = entry.amount
	end

	return amount;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetPrice(itemName)
	if (not itemName) then
		return 0;
	end
	
	local entry=self.buyList[itemName];
	local price=0;
	if (entry) then
		price=entry.price;
	end
	
	if (price>0) then
		g_pp_scale_price=System.GetCVar("g_pp_scale_price");
		if (g_pp_scale_price) then
			price=math.floor(price*math.max(0, g_pp_scale_price));
		end
	end
	
	-- if (energy and energy>0) then
	-- 	g_energy_scale_price=System.GetCVar("g_energy_scale_price");
	-- 	if (g_energy_scale_price) then
	-- 		energy=math.floor(energy*math.max(0, g_energy_scale_price));
	-- 	end
	-- end
	
	return price;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:GetItemName(itemName)
	if (not itemName) then
		return "";
	end
	
	local entry=self.buyList[itemName];
	if (entry) then
		return entry.name;
	end
	
	return itemName;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:GetItemFlag(itemName)
	local item=self.buyList[itemName];
	local flag=0;
	if (item.ammo) then flag=bor(flag, self.BUY_AMMO); end;
	if (item.equip) then flag=bor(flag, self.BUY_EQUIPMENT); end;
	if (item.proto) then flag=bor(flag, self.BUY_PROTOTYPE); end;
	if (item.vehicle) then flag=bor(flag, self.BUY_VEHICLE); end;
	if (item.weapon) then flag=bor(flag, self.BUY_WEAPON); end;
	
	return flag;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:GetItemDef(itemName)
	local entry=self.buyList[itemName];
	if (entry) then
		return entry;
	end
	return nil;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:ItemExists(playerId, itemName)
	return self.buyList[itemName]~=nil;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:IsVehicle(itemName)
	return self.buyList[itemName].vehicle;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:GetAutoBuyPrice()
	local player=System.GetEntity(g_localActorId);
	if (not player) then
		return 0;
	end
	
	local vehicleId=g_localActor.actor:GetLinkedVehicleId();
	local weapon;
	if (vehicleId) then
		local vehicle=System.GetEntity(vehicleId);
		if (vehicle) then
			local seat=vehicle:GetSeat(g_localActorId);
			if (seat and seat.seat:GetWeaponCount()>0) then
				local weaponId = seat.seat:GetWeaponId(1);
				if (weaponId) then
					weapon=System.GetEntity(weaponId);
				end				
			end
		end
	else
		weapon=g_localActor.inventory:GetCurrentItem();
	end
	
	if (weapon and weapon.weapon) then
		name=weapon.weapon:GetAmmoType();
	end
	
	local ammo=self.buyList[name];
	if (ammo and ammo.ammo) then
		local price=self:GetPrice(name);	
		local vehicleId = player and player.actor:GetLinkedVehicleId();
		if (vehicleId) then
			local vehicle=System.GetEntity(vehicleId);
			local c=vehicle.inventory:GetAmmoCount(name) or 0;
			local m=vehicle.inventory:GetAmmoCapacity(name) or 0;
			if (c<m or m==0) then
				local need=ammo.amount;
				if (m>0) then
					need=math.min(m-c, ammo.amount);
				end

				if (price>0) then
					if (need<ammo.amount) then
						price=math.ceil((need*price)/ammo.amount);
					end
					return price;
				end
			end
		else
			local c=player.inventory:GetAmmoCount(name) or 0;
			local m=player.inventory:GetAmmoCapacity(name) or 0;
			if (c<m or m==0) then
				local need=ammo.amount;
				if (m>0) then
					need=math.min(m-c, ammo.amount);
				end

				if (price>0) then
					if (need<ammo.amount) then
						price=math.ceil((need*price)/ammo.amount);
					end
					return price;
				end
			end
		end
	end

	return 0;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:VehicleCanUseAmmo2(vehicleId, ammo)
	local vehicle = System.GetEntity(vehicleId);
	
	if (vehicle) then
		for i,seat in pairs(vehicle.Seats) do
			local weaponCount = seat.seat:GetWeaponCount();
			for j = 1,weaponCount do
				local weaponId = seat.seat:GetWeaponId(j);
				if (weaponId) then
					local weapon = System.GetEntity(weaponId);
					if (weapon) then
						local weaponAmmo=weapon.weapon:GetAmmoType();
						if ((weaponAmmo==ammo) and (weapon.weapon:GetClipSize()~=-1)) then
							return true;
						end
					end
				end
			end
		end
	end
	
	return false;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:VehicleCanUseAmmo(vehicle, ammo)
	if (vehicle) then
		for i,seat in pairs(vehicle.Seats) do
			local weaponCount = seat.seat:GetWeaponCount();
			for j = 1,weaponCount do
				local weaponId = seat.seat:GetWeaponId(j);
				if (weaponId) then
					local weapon = System.GetEntity(weaponId);
					if (weapon) then
						local weaponAmmo=weapon.weapon:GetAmmoType();
						if ((weaponAmmo==ammo) and (weapon.weapon:GetClipSize()~=-1)) then
							return true;
						end
					end
				end
			end
		end
	end
	
	return false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:GetActiveItemCount(itemName, teamId)
	local def=self:GetItemDef(itemName);
	if (not def) then
		return 0;
	end
	
	if (not def.class) then
		return -1;
	end
	
	local count=0;
	local entities=System.GetEntitiesByClass(def.class);
	if (entities) then
		if (teamId) then
			for i,entity in pairs(entities) do
				if (entity and entity.builtas and entity.builtas==itemName) then
					if (self.game:GetTeam(entity.id)==teamId) then
						count=count+1;
					end
				end
			end
		else
			for i,entity in pairs(entities) do
				if (entity and entity.builtas and entity.builtas==itemName) then
					count=count+1;
				end
			end
		end
	end
	
	return count;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:HasItem(playerId, itemName)
	local player=System.GetEntity(playerId);
	local inventory=player and player.inventory;
	if (inventory) then
		if (inventory:GetItemByClass(itemName)) then
			return true;
		end
	end
	
	return false;
end


----------------------------------------------------------------------------------------------------
function SinglePlayer:HasUniqueItem(playerId, uniqueId)
	local player=System.GetEntity(playerId);
	if (not player) then
		return false;
	end
	
	local alive=(player.actor:GetHealth()>0);
	if (alive) then
		local inventory=player and player.inventory;
		if (inventory) then
			for item,def in pairs(self.buyList) do
				if (def.uniqueId and def.uniqueId==uniqueId) then
					local itemId=inventory:GetItemByClass(def.class);
					if (itemId) then
						local item=System.GetEntity(itemId);
						if (item and item.builtas==def.id) then
							return true, def.id;
						end
					end
				end
			end	
		end
	else
		local revive=self.reviveQueue[playerId];
		for i,v in pairs(revive.items) do
			local def=self:GetItemDef(v);
			if (def and def.uniqueId and def.uniqueId==uniqueId) then
				return true, def.id;
			end
		end
	end
	
	return false;
end

----------------------------------------------------------------------------------------------------
function SinglePlayer:PrecacheLevel()
	TeamInstantAction.PrecacheLevel(self);
	
	for i,v in pairs(self.buyList) do
		if (v.weapon or v.equip or v.proto) then
			if (v.class) then
			 	if (type(v.class)=="string" and v.class~="") then
					CryAction.CacheItemGeometry(v.class);
					CryAction.CacheItemSound(v.class);
				elseif (type(v.class)=="table") then
					for k,j in pairs(v.class) do
						CryAction.CacheItemGeometry(j);
						CryAction.CacheItemSound(j);
					end
				end
			end
		end
	end
end


----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClResetBuyZones()
	if (g_localActor and HUD) then
		HUD.ResetBuyZones();
		HUD.UpdateBuyList();
	end
end

----------------------------------------------------------------------------------------------------
function SinglePlayer.Client:ClEnterBuyZone(zoneId, enable)
	if (g_localActor and HUD) then
		HUD.EnteredBuyZone(zoneId, enable);
		HUD.UpdateBuyList();
	end
end
--~TheOtherSide SinglePlayer Functions