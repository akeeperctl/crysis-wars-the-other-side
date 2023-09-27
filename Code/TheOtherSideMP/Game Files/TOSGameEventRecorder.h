#pragma once

#include "IGameplayRecorder.h"
#include "IEntity.h"

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

	eEGE_SlaveStartObey,
	eEGE_SlaveStopObey,

	eEGE_EditorGameEnter,
	eEGE_EditorGameExit,
	
	eEGE_EnterGame,
	eEGE_EnterSpectator,

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

class CTOSGameEventRecorder
{

public:
	CTOSGameEventRecorder();

	const char* GetStringFromEnum(int value);
	void RecordEvent(IEntity *pEntity, const STOSGameEvent &event);
};