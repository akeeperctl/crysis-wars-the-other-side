/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

// ReSharper disable CppInconsistentNaming
#pragma once

#include "IEntity.h"
#include "IGameplayRecorder.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "TOSGame.h"

// Example
//   TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_SynchronizerCreated, "MasterModule", true));
#define TOS_RECORD_EVENT(entityId, tosGameEventExample) \
if (g_pTOSGame)\
	g_pTOSGame->GetEventRecorder()->RecordEvent((entityId), (tosGameEventExample)) \

// Description
//   Initialize the event vars
// Values
//   entName, eventName, eventDesc, pGO
#define TOS_INIT_EVENT_VALUES(pEntity, _event) \
const string entName = (pEntity) ? (pEntity)->GetName() : "";\
const EntityId entId = (pEntity) ? (pEntity)->GetId() : 0;\
const string eventName = g_pTOSGame->GetEventRecorder()->GetStringFromEnum((_event).event);\
	  string eventDesc = (_event).description;\
const auto pGO = (pEntity) ? g_pGame->GetIGameFramework()->GetGameObject((pEntity)->GetId()) : nullptr\


enum EExtraGameplayEvent
{
	//If update this, plz update GetStringFromEnum func

	eEGE_MainMenuOpened = 36,

	eEGE_ActorGrabbed, //NOT USED
	eEGE_ActorDropped, //NOT USED
	eEGE_ActorGrab, //NOT USED
	eEGE_ActorDrop, //NOT USED
	eEGE_ActorPostInit,
	eEGE_InitClient,
	eEGE_ActorRelease,
	eEGE_ActorDead,


	//eEGE_MasterEnterSpectator, //NOT USED
	eEGE_MasterAdd,
	eEGE_MasterRemove,

	eEGE_SlaveStartObey, //NOT USED
	eEGE_SlaveStopObey, //NOT USED

	eEGE_EditorGameEnter,
	eEGE_EditorGameExit,

	eEGE_SynchronizerCreated,
	eEGE_SynchronizerDestroyed,
	eEGE_SynchronizerRegistered,

	eEGE_PlayerJoinedGame, // Игрок нажал кнопку "Присоединится" и появился в игре. Не срабатывает автоматически когда игрок в игре, но после sv_restart 
	eEGE_PlayerJoinedSpectator, // Игрок нажал кнопку "Зритель" и перешёл в режим зрителя
	eEGE_PlayerJoinedCutscene,

	eEGE_GamerulesReset, // log off
	eEGE_GamerulesStartGame, // log off
	eEGE_GamerulesEventInit, // log off
	eEGE_GamerulesPostInit, // log off
	eEGE_GamerulesPostInitClient, // log off
	eEGE_GamerulesInit, // log off
	eEGE_GamerulesDestroyed, //NOT USED

	eEGE_GameModuleInit,

	eEGE_EntityOnSpawn, // Сущность была заспавнена с помощью pEntitySystem->SpawnEntity()
	eEGE_EntityOnRemove,
	eEGE_EntityRemovedForced, // Не важно какая сущность была удалена с помощью CTOSEntitySpawnModule::RemoveEntityForced()

	eEGE_TOSEntityOnSpawn, // Сущность была заспавнена с помощью CTOSEntitySpawnModule::SpawnEntity()
	eEGE_TOSEntityOnRemove,
	eEGE_TOSEntityMarkForRecreation,
	eEGE_TOSEntityRecreated,

	eEGE_TOSEntityScheduleDelegateAuthority,
	eEGE_TOSEntityAuthorityDelegated,

	eEGE_ForceStartControl,

	eEGE_EntitiesPreReset,
	eEGE_EntitiesPostReset,

	eEGE_MasterClientOnSetSlave, ///< Вызывается с локальной машины, когда у мастер-клиента изменяется указатель на контролируемого раб
	eEGE_MasterClientOnClearSlave, ///< Вызывается с локальной машины, когда у мастер-клиента указатель на контролируемого раба становится равен 0
	eEGE_MasterClientOnStartControl, ///< Вызывается с локальной машины, когда мастер-клиент начинает контролировать сущность раба
	eEGE_MasterClientOnStopControl, ///< Вызывается с локальной машины, когда мастер-клиент завершает контролировать сущность раба

	eEGE_SlaveEntityOnRemove,

	eEGE_GameChannelDestroyed,
	eEGE_ConfigureGameChannel,

	eEGE_ClientConnect, // Вызывается только на сервере в мультиплеере
	eEGE_ClientDisconnect, // Вызывается только на сервере в мультиплеере
	eEGE_ClientEnteredGame, // Вызывается и на сервере и на клиенте

	eEGE_UpdateContextViewState,
	eEGE_UpdateChannelConnectionState,

	eEGE_VehicleDestroyed,

	eEGE_ActorRevived,
	eEGE_OnLevelLoadingStart,
	eEGE_OnServerStartRestarting,
	eEGE_ActorExitVehicle,
	eEGE_ActorEnterVehicle,

	eEGE_HUDInit,
	eEGE_HUDHandleFSCommand,
	eEGE_HUDInGamePostUpdate,
	eEGE_HUDUnloadSimpleAssets,

	eEGE_EntityFactionChanged,
	eEGE_FactionReactionChanged,

	//eEGE_TOSGame_Init,

	//eEGE_VehicleStuck,
	eEGE_Last,
};

struct STOSGameEvent
{
	STOSGameEvent() :
		event(0),
		description(nullptr),
		value(0),
		extra_data(nullptr),
		int_value(0),
		console_log(false),
		vanilla_recorder(false)
	{}

	explicit STOSGameEvent(const GameplayEvent& _event) :
		event(_event.event),
		description(_event.description),
		value(_event.value),
		extra_data(_event.extra),
		int_value(0),
		console_log(false),
		vanilla_recorder(false)
	{}

	explicit STOSGameEvent(const uint8 evt, const char* desc = nullptr, const bool log = false, const bool vanilla = false, void* xtra = nullptr, const float val = 0.0f, const int int_val = 0) :
		event(evt),
		description(desc),
		value(val),
		extra_data(xtra),
		int_value(int_val),
		console_log(log),
		vanilla_recorder(vanilla)
	{}

	uint8 event;
	const char* description;
	float value;
	void* extra_data;
	int int_value;
	bool console_log;
	bool vanilla_recorder; // события, которые обозначены в IGameplayRecorder
};


/**
 * TOS Game Event Recorder
 * На сервере и клиенте: записывает и реагирует на все события в течение работы игры
 * RecordEvent() - записывает событие
 * Автоудаление: отсутствует.
 */
class CTOSGameEventRecorder
{

public:
	CTOSGameEventRecorder();

	const char* GetStringFromEnum(const int value) const
	{
		switch (value)
		{
			//eGE - Vanilla events
			case eGE_Connected:
				return "Connected";
			case eGE_Disconnected:
				return "Disconnected";
			case eGE_GameReset:
				return "GameReset";
			case eGE_GameStarted:
				return "GameStarted";
			case eGE_GameEnd:
				return "GameEnd";
			case eGE_Renamed:
				return "Renamed";
			case eGE_ChangedTeam:
				return "ChangedTeam";
			case eGE_Died:
				return "Died";
			case eGE_Scored:
				return "Scored";
			case eGE_Currency:
				return "Currency";
			case eGE_Rank:
				return "Rank";
			case eGE_Spectator:
				return "Spectator";
			case eGE_ScoreReset:
				return "ScoreReset";
			case eGE_AttachedAccessory:
				return "AttachedAccessory";
			case eGE_ZoomedIn:
				return "ZoomedIn";
			case eGE_ZoomedOut:
				return "ZoomedOut";
			case eGE_Kill:
				return "Kill";
			case eGE_Death:
				return "Death";
			case eGE_Revive:
				return "Revive";
			case eGE_SuitModeChanged:
				return "SuitModeChanged";
			case eGE_Hit:
				return "Hit";
			case eGE_Damage:
				return "Damage";
			case eGE_WeaponHit:
				return "WeaponHit";
			case eGE_WeaponReload:
				return "WeaponReload";
			case eGE_WeaponShot:
				return "WeaponShot";
			case eGE_WeaponMelee:
				return "WeaponMelee";
			case eGE_WeaponFireModeChanged:
				return "WeaponFireModeChanged";
			case eGE_Explosion:
				return "Explosion";
			case eGE_ItemSelected:
				return "ItemSelected";
			case eGE_ItemPickedUp:
				return "ItemPickedUp";
			case eGE_ItemDropped:
				return "ItemDropped";
			case eGE_ItemBought:
				return "ItemBought";
			case eGE_EnteredVehicle:
				return "EnteredVehicle";
			case eGE_LeftVehicle:
				return "LeftVehicle";

				//eEGE - TOS events
			case eEGE_MainMenuOpened:
				return "MainMenuOpened";
			case eEGE_ActorGrabbed:
				return "ActorGrabbed";
			case eEGE_ActorDropped:
				return "ActorDropped";
			case eEGE_ActorGrab:
				return "ActorGrab";
			case eEGE_ActorDrop:
				return "ActorDrop";
			case eEGE_ActorRelease:
				return "ActorRelease";
			case eEGE_ActorPostInit:
				return "ActorPostInit";
			case eEGE_InitClient:
				return "InitClient";
			case eEGE_ActorDead:
				return "ActorDead";
			case eEGE_GamerulesReset:
				return "GamerulesReset";
			case eEGE_MasterClientOnStartControl:
				return "MasterClientOnStartControl";
			case eEGE_MasterClientOnStopControl:
				return "MasterClientOnStopControl";
			case eEGE_SlaveStartObey:
				return "SlaveStartObey";
			case eEGE_SlaveStopObey:
				return "SlaveStopObey";
			case eEGE_EditorGameEnter:
				return "EditorGameEnter";
			case eEGE_EditorGameExit:
				return "EditorGameExit";
			case eEGE_PlayerJoinedGame:
				return "PlayerJoinedGame";
			case eEGE_PlayerJoinedSpectator:
				return "PlayerJoinedSpectator";
			case eEGE_MasterAdd:
				return "MasterAdd";
			case eEGE_MasterRemove:
				return "MasterRemove";
			case eEGE_SynchronizerCreated:
				return "SynchronizerCreated";
			case eEGE_SynchronizerDestroyed:
				return "SynchronizerDestroyed";
			case eEGE_GamerulesDestroyed:
				return "GamerulesDestroyed";
			case eEGE_GamerulesStartGame:
				return "GamerulesStartGame";
			case eEGE_GamerulesEventInit:
				return "GamerulesEventInit";
			case eEGE_GamerulesPostInit:
				return "GamerulesPostInit";
			case eEGE_GamerulesInit:
				return "GamerulesInit";
			case eEGE_GameModuleInit:
				return "GameModuleInit";
			case eEGE_EntityOnSpawn:
				return "EntityOnSpawn";
			case eEGE_EntityOnRemove:
				return "EntityOnRemove";
			case eEGE_TOSEntityOnSpawn:
				return "TOSEntityOnSpawn";
			case eEGE_TOSEntityOnRemove:
				return "TOSEntityOnRemove";
			case eEGE_TOSEntityMarkForRecreation:
				return "TOSEntityMarkForRecreation";
			case eEGE_TOSEntityRecreated:
				return "TOSEntityRecreated";
			case eEGE_TOSEntityScheduleDelegateAuthority:
				return "TOSEntityScheduleDelegateAuthority";
			case eEGE_TOSEntityAuthorityDelegated:
				return "TOSEntityAuthorityDelegated";
			case eEGE_ForceStartControl:
				return "ForceStartControl";
			case eEGE_EntitiesPreReset:
				return "EntitiesPreReset";
			case eEGE_EntitiesPostReset:
				return "EntitiesPostReset";
			case eEGE_MasterClientOnSetSlave:
				return "MasterClientOnSetSlave";
			case eEGE_MasterClientOnClearSlave:
				return "MasterClientOnClearSlave";
			case eEGE_SlaveEntityOnRemove:
				return "SlaveEntityOnRemove";
			case eEGE_PlayerJoinedCutscene:
				return "PlayerJoinedCutscene";
			case eEGE_GameChannelDestroyed:
				return "GameChannelDestroyed";
			case eEGE_ConfigureGameChannel:
				return "ConfigureGameChannel";
			case eEGE_ClientConnect:
				return "ClientConnect";
			case eEGE_ClientDisconnect:
				return "ClientDisconnect";
			case eEGE_ClientEnteredGame:
				return "ClientEnteredGame";
			case eEGE_GamerulesPostInitClient:
				return "GamerulesPostInitClient";
			case eEGE_UpdateContextViewState:
				return "UpdateContextViewState";
			case eEGE_UpdateChannelConnectionState:
				return "UpdateChannelConnectionState";
			case eEGE_SynchronizerRegistered:
				return "SynchronizerRegistered";
			case eEGE_EntityRemovedForced:
				return "EntityRemovedForced";
			case eEGE_VehicleDestroyed:
				return "VehicleDestroyed";
			case eEGE_ActorRevived:
				return "ActorRevived";
			case eEGE_OnLevelLoadingStart:
				return "OnLevelLoadingStart";
			case eEGE_OnServerStartRestarting:
				return "OnServerStartRestarting";
			case eEGE_ActorExitVehicle:
				return "ActorExitVehicle";
			case eEGE_ActorEnterVehicle:
				return "ActorEnterVehicle";
			case eEGE_HUDInit:
				return "HUDInit";
			case eEGE_HUDHandleFSCommand:
				return "HUDHandleFSCommand";
			case eEGE_HUDInGamePostUpdate:
				return "HUDInGamePostUpdate";
			case eEGE_HUDUnloadSimpleAssets:
				return "HUDUnloadSimpleAssets";
			case eEGE_EntityFactionChanged:
				return "EntityFactionChanged";
			case eEGE_FactionReactionChanged:
				return "FactionReactionChanged";
			case eEGE_Last:
				return "Last";
			default:
				return "<UNDEFINED>";
		}
	}

	static void RecordEvent(EntityId id, const STOSGameEvent& event);
};