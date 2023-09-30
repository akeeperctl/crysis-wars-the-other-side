#pragma once

#include "IActorSystem.h"
#include "IInput.h"
struct STOSGameEvent;
class CTOSGenericSynchronizer;


struct ITOSGameModule
{
	//virtual void OnMainMenuEnter() = 0;
	//virtual void OnGameRulesReset() = 0;
	//virtual void OnActorDeath(IActor* pActor) = 0;
	//virtual void OnActorGrabbed(IActor* pActor, EntityId grabberId) = 0;
	//virtual void OnActorDropped(IActor* pActor, EntityId droppedId) = 0;
	//virtual void OnActorGrab(IActor* pActor, EntityId grabId) = 0;
	//virtual void OnActorDrop(IActor* pActor, EntityId dropId) = 0;
	//virtual void OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) = 0;
	//virtual void OnExitVehicle(IActor* pActor) = 0;

	//virtual void OnStartControl(const IActor* pActor) { };
	//virtual void OnStopControl(const IActor* pActor) { };

	virtual bool OnInputEvent(const SInputEvent& event) { return true; };
	virtual bool OnInputEventUI(const SInputEvent& event) { return false; };

	virtual CTOSGenericSynchronizer* GetSynchronizer() const = 0;
	virtual void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) = 0;
	virtual void GetMemoryStatistics(ICrySizer* s) = 0;
	virtual const char* GetName() = 0;
	virtual void Init() = 0;
	virtual void Update(float frametime) = 0;
	virtual void Serialize(TSerialize ser) = 0;

	bool operator == (ITOSGameModule* pModule)
	{
		if (!pModule)
			return false;

		return strcmp(pModule->GetName(), this->GetName()) == 0;
	}

	bool operator != (ITOSGameModule* pModule)
	{
		if (!pModule)
			return false;

		return strcmp(pModule->GetName(), this->GetName()) != 0;
	}
};