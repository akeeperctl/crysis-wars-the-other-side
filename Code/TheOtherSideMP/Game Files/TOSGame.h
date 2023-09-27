#pragma once

#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IVehicleSystem.h>
#include <IGameplayRecorder.h>

class STOSCvars;
class CAIActionTracker;
class CControlClient;
class CGameFlashAnimation;
class CAbilitiesSystem;

class CTOSGameEventRecorder;
class STOSGameEvent;

struct IHardwareMouseEventListener;
struct IHitListener;
struct ITOSGameModule;

enum EExtraGameplayEvent;

class CTOSGame
{
public:
	CTOSGame();
	~CTOSGame();

	friend class CTOSGameEventRecorder;

	void Init();
	void Shutdown();
	void Update(float frameTime, int frameId);

	//Events
	
	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener
	// 
	//IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event);
	virtual bool OnInputEventUI(const SInputEvent& event) { return false; }
	//~IInputEventListener

	//IGameplayListener
	virtual void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event);
	//~IGameplayListener

	//TOSEventRecorder->RecordEvent->calling this
	//IGameplayRecorder->Event->calling this
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);

	//ILevelSystemListener
	//virtual void OnLevelNotFound(const char* levelName);
	//virtual void OnLoadingStart(ILevelInfo* pLevel);
	//virtual void OnLoadingComplete(ILevel* pLevel);
	//virtual void OnLoadingError(ILevelInfo* pLevel, const char* error);
	//virtual void OnLoadingProgress(ILevelInfo* pLevel, int progressAmount);
	////~ILevelSystemListener

	//// IItemSystemListener
	//virtual void OnSetActorItem(IActor* pActor, IItem* pItem);
	//virtual void OnDropActorItem(IActor* pActor, IItem* pItem);
	//virtual void OnSetActorAccessory(IActor* pActor, IItem* pItem) {};
	//virtual void OnDropActorAccessory(IActor* pActor, IItem* pItem) {};
	// ~IItemSystemListener

	//void		OnMainMenuEnter();
	//void		OnActorDeath(IActor* pActor);
	//void		OnActorGrabbed(IActor* pActor, EntityId grabberId);
	//void		OnActorDropped(IActor* pActor, EntityId droppedId);
	//void		OnActorGrab(IActor* pActor, EntityId grabId);
	//void		OnActorDrop(IActor* pActor, EntityId dropId);
	//void		OnEnterVehicle(IActor* pActor, IVehicle* pVehicle, const char* strSeatName, bool bThirdPerson);
	//void		OnExitVehicle(IActor* pActor);
	//void		OnGameRulesReset();
	//void		OnVehicleStuck(IVehicle* pVehicle, bool stuck);
	// 

	//~Events

	CTOSGameEventRecorder* GetEventRecorder() const;


private:

	CAIActionTracker* m_pAIActionTracker;
	CControlClient* m_pLocalControlClient;
	CTOSGameEventRecorder* m_pEventRecorder;

	std::vector<ITOSGameModule*> m_modules;
	std::vector<ITOSGameModule*> m_flowgraphModules;

	Vec3 m_mouseScreenPos;
	Vec3 m_mouseWorldPos;
};

extern class CTOSGame* g_pTOSGame;