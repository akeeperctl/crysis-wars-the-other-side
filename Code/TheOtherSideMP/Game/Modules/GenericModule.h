#pragma once

#include "Game.h"
#include "ITOSGameModule.h"

#include "TheOtherSideMP/Game/TOSGameEventRecorder.h"

class CTOSGenericModule : public ITOSGameModule // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterModule;

	CTOSGenericModule();
	~CTOSGenericModule() ;

	//ITOSGameModule
	bool        OnInputEvent(const SInputEvent& event)  { return true; };
	bool        OnInputEventUI(const SInputEvent& event)  { return false; };
	void        OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) ;
	void        GetMemoryStatistics(ICrySizer* s) ;
	const char* GetName()  { return "CTOSGenericModule"; };
	void        Init() ;
	void        Update(float frametime) ;
	void        Serialize(TSerialize ser) ;
	bool		NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;

	int GetDebugLog()  { return m_debugLogMode; }

	void InitCVars(IConsole* pConsole)  {};
	void InitCCommands(IConsole* pConsole)  {};
	void ReleaseCVars()  {};
	void ReleaseCCommands()  {};

	//~ITOSGameModule

	virtual CTOSGenericSynchronizer* GetSynchronizer() const;

	//template <class CSynchType>
	//CSynchType* CreateSynchonizer(const char* entityName, const char* extensionName);

protected:
	/**
	 * \brief Регистрация синхронизатора для этого модуля.
		\n смотреть \a m_pSynchonizer
	 */
	void RegisterSynchronizer(CTOSGenericSynchronizer* pSynch);

	CTOSGenericSynchronizer* m_pSynchonizer;

	int m_debugLogMode; // режим отладки модуля (1 - вкл, 0 - выкл)

private:
};

// Not actual anymore
//template <class CSynchType>
//CSynchType* CTOSGenericModule::CreateSynchonizer(const char* entityName, const char* extensionName)
//{
//	//const char* extName = extensionName;
//	//CSynchType* pSynchExt = nullptr;
//
//	//const IEntity* pSynchEntity = gEnv->pEntitySystem->FindEntityByName(entityName);
//	//if (pSynchEntity)
//	//{
//	//	IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchEntity->GetId());
//	//	if (pGO)
//	//	{
//	//		// pGO->RegisterAsPredicted();
//
//	//		pSynchExt = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
//	//		assert(pSynchExt);
//
//	//		//m_pSynchonizer->SetModule(this);
//	//		m_pSynchonizer = pSynchExt;
//
//	//		TOS_RECORD_EVENT(pSynchEntity->GetId(), STOSGameEvent(eEGE_SynchronizerCreated, "", true));
//
//	//		return pSynchExt;
//	//	}
//	//}
//
//	//SEntitySpawnParams params;
//	//params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
//	//params.bStaticEntityId = true;
//	//params.bIgnoreLock = true; // i dont know why i use it
//	//params.sName = entityName;
//	//params.nFlags |= ENTITY_FLAG_NO_PROXIMITY;// | ENTITY_FLAG_UNREMOVABLE; Флаг ENTITY_FLAG_UNREMOVABLE удалён, ибо не работает при sv_restart
//
//	//pSynchEntity = gEnv->pEntitySystem->SpawnEntity(params);
//	//assert(pSynchEntity);
//
//	//if (!pSynchEntity) 
//	//	return nullptr;
//
//	////IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchronizer->GetId());
//	//IGameObject* pGO = g_pGame->GetIGameFramework()->GetIGameObjectSystem()->CreateGameObjectForEntity(pSynchEntity->GetId());
//	//if (pGO)
//	//{
//	//	// Вообще эффекта не имеет
//	//	// pGO->RegisterAsPredicted(); 
//
//	//	//m_pSynchonizer = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
//	//	//assert(m_pSynchonizer);
//	//	pSynchExt = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
//	//	assert(pSynchExt);
//
//
//	//	//m_pSynchonizer->SetModule(this);
//
//	//	pSynchExt->GetGameObject()->ForceUpdate(true);
//	//	m_pSynchonizer = pSynchExt;
//
//	//	TOS_RECORD_EVENT(pSynchEntity->GetId(), STOSGameEvent(eEGE_SynchronizerCreated, "", true));
//	//}
//
//	//return pSynchExt;
//
//	return nullptr;
//}
