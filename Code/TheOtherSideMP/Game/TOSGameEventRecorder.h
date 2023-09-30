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
const char* entName = pEntity ? pEntity->GetName() : "";\
const EntityId entId = pEntity ? pEntity->GetId() : 0;\
const char* eventName = g_pTOSGame->GetEventRecorder()->GetStringFromEnum(_event.event);\
const char* eventDesc = _event.description;\
const auto pGO = pEntity ? g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId()) : nullptr\


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

	//eEGE_TOSGame_Init,

	//eEGE_VehicleStuck,
	eEGE_Last,
};

struct STOSGameEvent
{
	STOSGameEvent(): 
		event(0), 
		description(0), 
		value(0), 
		int_value(0), 
		console_log(0), 
		vanilla_recorder(0) {};

	STOSGameEvent(const GameplayEvent& _event): 
		event(_event.event), 
		description(_event.description), 
		value(_event.value), 
		int_value(0), 
		console_log(0), 
		vanilla_recorder(0) {};

	STOSGameEvent(uint8 evt, const char* desc = 0, bool log = false, bool vanilla = false, float val = 0.0f, int int_val = 0, void* xtra = 0) : 
		event(evt), 
		description(desc), 
		value(val), 
		int_value(int_val), 
		console_log(log), 
		vanilla_recorder(vanilla), 
		extra(xtra) {};

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

	const char* GetStringFromEnum(int value)
	{
		switch (value)
		{
		//eGE - Vanilla events
		case eGE_Connected:
			return "eGE_Connected";
			break;
		case eGE_Disconnected:
			return "eGE_Disconnected";
			break;
		case eGE_GameReset:
			return "eGE_GameReset";
			break;
		case eGE_GameStarted:
			return "eGE_GameStarted";
			break;
		case eGE_GameEnd:
			return "eGE_GameEnd";
			break;
		case eGE_Renamed:
			return "eGE_Renamed";
			break;
		case eGE_ChangedTeam:
			return "eGE_ChangedTeam";
			break;
		case eGE_Died:
			return "eGE_Died";
			break;
		case eGE_Scored:
			return "eGE_Scored";
			break;
		case eGE_Currency:
			return "eGE_Currency";
			break;
		case eGE_Rank:
			return "eGE_Rank";
			break;
		case eGE_Spectator:
			return "eGE_Spectator";
			break;
		case eGE_ScoreReset:
			return "eGE_ScoreReset";
			break;
		case eGE_AttachedAccessory:
			return "eGE_AttachedAccessory";
			break;
		case eGE_ZoomedIn:
			return "eGE_ZoomedIn";
			break;
		case eGE_ZoomedOut:
			return "eGE_ZoomedOut";
			break;
		case eGE_Kill:
			return "eGE_Kill";
			break;
		case eGE_Death:
			return "eGE_Death";
			break;
		case eGE_Revive:
			return "eGE_Revive";
			break;
		case eGE_SuitModeChanged:
			return "eGE_SuitModeChanged";
			break;
		case eGE_Hit:
			return "eGE_Hit";
			break;
		case eGE_Damage:
			return "eGE_Damage";
			break;
		case eGE_WeaponHit:
			return "eGE_WeaponHit";
			break;
		case eGE_WeaponReload:
			return "eGE_WeaponReload";
			break;
		case eGE_WeaponShot:
			return "eGE_WeaponShot";
			break;
		case eGE_WeaponMelee:
			return "eGE_WeaponMelee";
			break;
		case eGE_WeaponFireModeChanged:
			return "eGE_WeaponFireModeChanged";
			break;
		case eGE_Explosion:
			return "eGE_Explosion";
			break;
		case eGE_ItemSelected:
			return "eGE_ItemSelected";
			break;
		case eGE_ItemPickedUp:
			return "eGE_ItemPickedUp";
			break;
		case eGE_ItemDropped:
			return "eGE_ItemDropped";
			break;
		case eGE_ItemBought:
			return "eGE_ItemBought";
			break;
		case eGE_EnteredVehicle:
			return "eGE_EnteredVehicle";
			break;
		case eGE_LeftVehicle:
			return "eGE_LeftVehicle";
			break;

		//eEGE - TOS events
		case eEGE_MainMenuOpened:
			return "eEGE_MainMenuOpened";
			break;
		case eEGE_ActorGrabbed:
			return "eEGE_ActorGrabbed";
			break;
		case eEGE_ActorDropped:
			return "eEGE_ActorDropped";
			break;
		case eEGE_ActorGrab:
			return "eEGE_ActorGrab";
			break;
		case eEGE_ActorDrop:
			return "eEGE_ActorDrop";
			break;
		case eEGE_ActorReleased:
			return "eEGE_ActorReleased";
			break;		
		case eEGE_ActorPostInit:
			return "eEGE_ActorPostInit";
			break;
		case eEGE_ActorInitClient:
			return "eEGE_ActorInitClient";
			break;
		case eEGE_GamerulesReset:
			return "eEGE_GamerulesReset";
			break;
		case eEGE_MasterStartControl:
			return "eEGE_MasterStartControl";
			break;
		case eEGE_MasterStopControl:
			return "eEGE_MasterStopControl";
			break;
		case eEGE_MasterEnterSpectator:
			return "eEGE_MasterEnterSpectator";
			break;
		case eEGE_SlaveStartObey:
			return "eEGE_SlaveStartObey";
			break;
		case eEGE_SlaveStopObey:
			return "eEGE_SlaveStopObey";
			break;
		case eEGE_EditorGameEnter:
			return "eEGE_EditorGameEnter";
			break;
		case eEGE_EditorGameExit:
			return "eEGE_EditorGameExit";
			break;
		case eEGE_EnterGame:
			return "eEGE_EnterGame";
			break;
		case eEGE_EnterSpectator:
			return "eEGE_EnterSpectator";
			break;
		case eEGE_MasterAdd:
			return "eEGE_MasterAdd";
			break;
		case eEGE_MasterRemove:
			return "eEGE_MasterRemove";
			break;
		case eEGE_SynchronizerCreated:
			return "eEGE_SynchronizerCreated";
			break;
		case eEGE_SynchronizerDestroyed:
			return "eEGE_SynchronizerDestroyed";
			break;
		case eEGE_GamerulesDestroyed:
			return "eEGE_GamerulesDestroyed";
			break;
		case eEGE_GamerulesStartGame:
			return "eEGE_GamerulesStartGame";
			break;
		case eEGE_GamerulesEventInit:
			return "eEGE_GamerulesEventInit";
			break;
		case eEGE_GamerulesPostInit:
			return "eEGE_GamerulesPostInit";
			break;
		case eEGE_GamerulesInit:
			return "eEGE_GamerulesInit";
			break;
		case eEGE_Last:
			return "eEGE_Last";
			break;
		}

		return "EExtraGameplayEvent UNDEFINED";
	}
	void RecordEvent(EntityId id, const STOSGameEvent &event);
};