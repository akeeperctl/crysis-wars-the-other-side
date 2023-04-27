#pragma once

#include <IGameFramework.h>
#include <IInput.h>

struct IControlSystemChild
{
	virtual void OnMainMenuEnter() = 0;
	virtual void OnGameRulesReset() = 0;
	virtual void OnActorDeath(IActor* pActor) = 0;
	virtual void OnActorGrabbed(IActor* pActor, EntityId grabberId) = 0;
	virtual void OnActorDropped(IActor* pActor, EntityId droppedId) = 0;
	virtual void OnActorGrab(IActor* pActor, EntityId grabId) = 0;
	virtual void OnActorDrop(IActor* pActor, EntityId dropId) = 0;
	virtual void OnEnterVehicle(IActor* pActor, IVehicle* pVehicle) = 0;
	virtual void OnExitVehicle(IActor* pActor) = 0;
	virtual bool OnInputEvent(const SInputEvent& event) { return true; };
	virtual bool OnInputEventUI(const SInputEvent& event) { return false; };
	virtual void Init() = 0;
	virtual void Update(float frametime) = 0;
	virtual void Serialize(TSerialize ser) = 0;
	virtual void GetMemoryStatistics(ICrySizer* s) = 0;
	virtual const char* GetChildName() = 0;
	virtual void OnStartControl(const IActor* pActor) { };
	virtual void OnStopControl(const IActor * pActor) { };

	bool operator == (IControlSystemChild* pChild)
	{
		if (!pChild)
			return false;

		return strcmp(pChild->GetChildName(), this->GetChildName()) == 0;
	}

	bool operator != (IControlSystemChild* pChild)
	{
		if (!pChild)
			return false;

		return strcmp(pChild->GetChildName(), this->GetChildName()) != 0;
	}
};