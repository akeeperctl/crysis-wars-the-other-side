#pragma once

#include "IGameplayRecorder.h"
#include "IEntity.h"
#include "TOSGame.h"

// Example
//   TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_SynchronizerCreated, "MasterModule", true));
#define TOS_RECORD_EVENT(entityId, tosGameEventExample) \
if (g_pTOSGame)\
	g_pTOSGame->GetEventRecorder()->RecordEvent(entityId, tosGameEventExample) \

// Description
//   Initialize the event vars
// Values
//   entName, eventName, eventDesc, pGO
#define TOS_INIT_EVENT_VALUES(pEntity, _event) \
const string entName = (pEntity) ? (pEntity)->GetName() : "";\
const EntityId entId = (pEntity) ? (pEntity)->GetId() : 0;\
const string eventName = g_pTOSGame->GetEventRecorder()->GetStringFromEnum(_event.event);\
const string eventDesc = (_event).description;\
const auto pGO = (pEntity) ? g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId()) : nullptr\


enum EExtraGameplayEvent
{
	//If update this, plz update GetStringFromEnum func

	eEGE_MainMenuOpened = 36,

	eEGE_ActorGrabbed,
	eEGE_ActorDropped,
	eEGE_ActorGrab,
	eEGE_ActorDrop,
	eEGE_ActorPostInit,
	eEGE_ActorInitClient,
	eEGE_ActorReleased,

	eEGE_MasterStartControl,
	eEGE_MasterStopControl,
	eEGE_MasterEnterSpectator,

	eEGE_MasterAdd,
	eEGE_MasterRemove,

	eEGE_SlaveStartObey,
	eEGE_SlaveStopObey,

	eEGE_EditorGameEnter,
	eEGE_EditorGameExit,

	eEGE_SynchronizerCreated,
	eEGE_SynchronizerDestroyed,
	
	eEGE_EnterGame,
	eEGE_EnterSpectator,

	eEGE_GamerulesReset,
	eEGE_GamerulesStartGame,
	eEGE_GamerulesEventInit,
	eEGE_GamerulesPostInit,
	eEGE_GamerulesInit,
	eEGE_GamerulesDestroyed,

	eEGE_GameModuleInit,

	eEGE_EntitySpawned,
	eEGE_EntityRemoved,

	//eEGE_TOSGame_Init,

	//eEGE_VehicleStuck,
	eEGE_Last,
};

struct STOSGameEvent
{
	STOSGameEvent():
		event(0),
		description(nullptr),
		value(0),
		extra(nullptr),
		int_value(0),
		console_log(false),
		vanilla_recorder(false)
	{
	};

	explicit STOSGameEvent(const GameplayEvent& _event):
		event(_event.event),
		description(_event.description),
		value(_event.value),
		extra(_event.extra),
		int_value(0),
		console_log(false),
		vanilla_recorder(false)
	{
	};

	explicit STOSGameEvent(const uint8 evt, const char* desc = nullptr, const bool log = false, const bool vanilla = false, const float val = 0.0f, const int int_val = 0, void* xtra = nullptr) : 
		event(evt), 
		description(desc), 
		value(val), 
		extra(xtra), 
		int_value(int_val), 
		console_log(log), 
		vanilla_recorder(vanilla) {};

	uint8 event;
	const char* description;
	float value;
	void* extra;
	int int_value;
	bool console_log;
	bool vanilla_recorder;
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
			return "eGE_Connected";
		case eGE_Disconnected:
			return "eGE_Disconnected";
		case eGE_GameReset:
			return "eGE_GameReset";
		case eGE_GameStarted:
			return "eGE_GameStarted";
		case eGE_GameEnd:
			return "eGE_GameEnd";
		case eGE_Renamed:
			return "eGE_Renamed";
		case eGE_ChangedTeam:
			return "eGE_ChangedTeam";
		case eGE_Died:
			return "eGE_Died";
		case eGE_Scored:
			return "eGE_Scored";
		case eGE_Currency:
			return "eGE_Currency";
		case eGE_Rank:
			return "eGE_Rank";
		case eGE_Spectator:
			return "eGE_Spectator";
		case eGE_ScoreReset:
			return "eGE_ScoreReset";
		case eGE_AttachedAccessory:
			return "eGE_AttachedAccessory";
		case eGE_ZoomedIn:
			return "eGE_ZoomedIn";
		case eGE_ZoomedOut:
			return "eGE_ZoomedOut";
		case eGE_Kill:
			return "eGE_Kill";
		case eGE_Death:
			return "eGE_Death";
		case eGE_Revive:
			return "eGE_Revive";
		case eGE_SuitModeChanged:
			return "eGE_SuitModeChanged";
		case eGE_Hit:
			return "eGE_Hit";
		case eGE_Damage:
			return "eGE_Damage";
		case eGE_WeaponHit:
			return "eGE_WeaponHit";
		case eGE_WeaponReload:
			return "eGE_WeaponReload";
		case eGE_WeaponShot:
			return "eGE_WeaponShot";
		case eGE_WeaponMelee:
			return "eGE_WeaponMelee";
		case eGE_WeaponFireModeChanged:
			return "eGE_WeaponFireModeChanged";
		case eGE_Explosion:
			return "eGE_Explosion";
		case eGE_ItemSelected:
			return "eGE_ItemSelected";
		case eGE_ItemPickedUp:
			return "eGE_ItemPickedUp";
		case eGE_ItemDropped:
			return "eGE_ItemDropped";
		case eGE_ItemBought:
			return "eGE_ItemBought";
		case eGE_EnteredVehicle:
			return "eGE_EnteredVehicle";
		case eGE_LeftVehicle:
			return "eGE_LeftVehicle";

		//eEGE - TOS events
		case eEGE_MainMenuOpened:
			return "eEGE_MainMenuOpened";
		case eEGE_ActorGrabbed:
			return "eEGE_ActorGrabbed";
		case eEGE_ActorDropped:
			return "eEGE_ActorDropped";
		case eEGE_ActorGrab:
			return "eEGE_ActorGrab";
		case eEGE_ActorDrop:
			return "eEGE_ActorDrop";
		case eEGE_ActorReleased:
			return "eEGE_ActorReleased";
		case eEGE_ActorPostInit:
			return "eEGE_ActorPostInit";
		case eEGE_ActorInitClient:
			return "eEGE_ActorInitClient";
		case eEGE_GamerulesReset:
			return "eEGE_GamerulesReset";
		case eEGE_MasterStartControl:
			return "eEGE_MasterStartControl";
		case eEGE_MasterStopControl:
			return "eEGE_MasterStopControl";
		case eEGE_MasterEnterSpectator:
			return "eEGE_MasterEnterSpectator";
		case eEGE_SlaveStartObey:
			return "eEGE_SlaveStartObey";
		case eEGE_SlaveStopObey:
			return "eEGE_SlaveStopObey";
		case eEGE_EditorGameEnter:
			return "eEGE_EditorGameEnter";
		case eEGE_EditorGameExit:
			return "eEGE_EditorGameExit";
		case eEGE_EnterGame:
			return "eEGE_EnterGame";
		case eEGE_EnterSpectator:
			return "eEGE_EnterSpectator";
		case eEGE_MasterAdd:
			return "eEGE_MasterAdd";
		case eEGE_MasterRemove:
			return "eEGE_MasterRemove";
		case eEGE_SynchronizerCreated:
			return "eEGE_SynchronizerCreated";
		case eEGE_SynchronizerDestroyed:
			return "eEGE_SynchronizerDestroyed";
		case eEGE_GamerulesDestroyed:
			return "eEGE_GamerulesDestroyed";
		case eEGE_GamerulesStartGame:
			return "eEGE_GamerulesStartGame";
		case eEGE_GamerulesEventInit:
			return "eEGE_GamerulesEventInit";
		case eEGE_GamerulesPostInit:
			return "eEGE_GamerulesPostInit";
		case eEGE_GamerulesInit:
			return "eEGE_GamerulesInit";
		case eEGE_GameModuleInit:
			return "eEGE_GameModuleInit";
		case eEGE_EntitySpawned:
			return "eEGE_EntitySpawned";
		case eEGE_EntityRemoved:
			return "eEGE_EntityRemoved";
		case eEGE_Last:
			return "eEGE_Last";

		default: 
			return "UNDEFINED";
		}
	}

	static void RecordEvent(EntityId id, const STOSGameEvent &event);
};