#pragma once

#include "IGameplayRecorder.h"
#include "IEntity.h"
#include "TOSGame.h"

// Example:
//   TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_SynchronizerCreated, "MasterModule", true));
#define TOS_RECORD_EVENT(entityId, tosGameEventExample) \
if (g_pTOSGame)\
	g_pTOSGame->GetEventRecorder()->RecordEvent(entityId, tosGameEventExample) \

// Summary
//   Initialize the event vars
// Remarks
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

	eEGE_GamerulesReset,

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

	eEGE_GamerulesPostInit,
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
		case eEGE_GamerulesPostInit:
			return "eEGE_GamerulesPostInit";
			break;
		case eEGE_GamerulesDestroyed:
			return "eEGE_GamerulesDestroyed";
			break;
		case eEGE_Last:
			return "eEGE_Last";
			break;
		}

		return "EExtraGameplayEvent UNDEFINED";
	}
	void RecordEvent(EntityId id, const STOSGameEvent &event);
};