/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include <IHardwareMouse.h>
#include <IGameFramework.h>
#include <IVehicleSystem.h>
#include <IGameplayRecorder.h>
#include <ILevelSystem.h>
#include <IInput.h>
#include "ScriptBinds\ScriptBind_Custom.h"
#include "FGPS\FGPluginLoader.h"

class CGameFlashAnimation;

class CTOSAbilitiesSystem;
class CTOSAIActionTracker;
class CTOSGameEventRecorder;
class CTOSMasterModule;
class CTOSEntitySpawnModule;
class CTOSZeusModule;
class CTOSFactionsModule;

struct STOSCvars;
struct STOSGameEvent;
struct ITOSGameModule;
struct ITOSGameEventListener;
struct IHardwareMouseEventListener;
struct IHitListener;

// ReSharper disable once CppInconsistentNaming
/**
 * \brief указатель на функцию
 */
typedef void (*func)();

//enum EExtraGameplayEvent;

// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class CTOSGame :  // NOLINT(cppcoreguidelines-special-member-functions)
	public IGameplayListener,
	public IInputEventListener,
	public IEntitySystemSink,
	public IScriptTableDumpSink,
	public ILevelSystemListener
{
public:
	CTOSGame();
	virtual ~CTOSGame();

	friend class CTOSGameEventRecorder;
	friend struct STOSCvars;

	void Init();
	void Shutdown();
	void Update(float frameTime, int frameId);
	void InitScriptBinds();
	void ReleaseScriptBinds();

	//Events
	// ILevelSystemListener
	void OnLevelNotFound(const char* levelName);
	void OnLoadingStart(ILevelInfo* pLevel);
	void CacheAssets();
	void OnLoadingComplete(ILevel* pLevel);
	void OnLoadingError(ILevelInfo* pLevel, const char* error);
	void OnLoadingProgress(ILevelInfo* pLevel, int progressAmount);

	// ILevelSystemListener


	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener

	//IInputEventListener
	bool OnInputEvent(const SInputEvent& event);
	bool OnInputEventUI(const SInputEvent& event)
	{
		return false;
	}
	//~IInputEventListener

	//IGameplayListener
	void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event);
	//~IGameplayListener

	//TOSEventRecorder->RecordEvent->calling this
	//IGameplayRecorder->Event->calling this
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) const;

	//IEntitySystemSink
	bool OnBeforeSpawn(SEntitySpawnParams& params);
	void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params);
	bool OnRemove(IEntity* pEntity);
	void OnEvent(IEntity* pEntity, SEntityEvent& event);
	//~IEntitySystemSink

	//IScriptTableDumpSink
	void OnElementFound(const char* sName, ScriptVarType type);
	void OnElementFound(int nIdx, ScriptVarType type);
	//~IScriptTableDumpSink

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

	/**
	 * \brief Получить актуального актёра игрока. Это может быть как Dude(стандартный персонаж)
		\n так и раб(контролируемый персонаж в модуле Master)
	 * \return Указатель на актёра.
	 */
	IActor* GetActualClientActor() const;

	CTOSGameEventRecorder* GetEventRecorder() const;
	CTOSMasterModule* GetMasterModule() const;
	CTOSZeusModule* GetZeusModule() const;
	CTOSEntitySpawnModule* GetEntitySpawnModule() const;
	CTOSFactionsModule* GetFactionsModule() const;

	bool ModuleAdd(ITOSGameModule* pModule, bool flowGraph);
	bool ModuleRemove(ITOSGameModule* pModule, bool flowGraph);

	bool RegisterGameEventListener(ITOSGameEventListener* pModule);
	bool UnregisterGameEventListener(ITOSGameEventListener* pListener);

private:

	void UpdateChannelConnectionState();
	void UpdateContextViewState();

	Vec3 m_mouseScreenPos;
	Vec3 m_mouseWorldPos;
	uint m_lastChannelConnectionState;
	uint m_lastContextViewState;
	string m_modVersion;

	CFGPluginLoader* m_pFGPluginLoader;
	CTOSGameEventRecorder* m_pEventRecorder;

	CScriptBind_Custom* m_pCustomScriptBind;

	std::vector<ITOSGameEventListener*> m_gameEventListeners;
	std::vector<ITOSGameModule*> m_modules;
	std::vector<ITOSGameModule*> m_flowgraphModules;

	//Modules
	CTOSMasterModule* m_pModuleMaster;
	CTOSZeusModule* m_pModuleZeus;
	CTOSEntitySpawnModule* m_pModuleEntitySpawn;
	CTOSFactionsModule* m_pModuleFactions;
};

extern class CTOSGame* g_pTOSGame;