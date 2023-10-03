#pragma once

#include "Game.h"
#include "ITOSGameModule.h"

class CTOSGenericModule : public ITOSGameModule // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterModule;

	CTOSGenericModule();
	~CTOSGenericModule() override;

	//ITOSGameModule
	bool OnInputEvent(const SInputEvent& event) override { return true; };
	bool OnInputEventUI(const SInputEvent& event) override { return false; };
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	void GetMemoryStatistics(ICrySizer* s) override;
	const char* GetName() override { return "CTOSGenericModule"; };
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;

	int GetDebugLog() override { return m_debugLogMode; }
	//~ITOSGameModule

	virtual CTOSGenericSynchronizer* GetSynchronizer() const;

	template <class CSynchType>
	CSynchType* CreateSynchonizer(const char* entityName, const char* extensionName);

protected:
	CTOSGenericSynchronizer* m_pSynchonizer;

private:
	int m_debugLogMode; // режим отладки модуля (1 - вкл, 0 - выкл)
};


template <class CSynchType>
CSynchType* CTOSGenericModule::CreateSynchonizer(const char* entityName, const char* extensionName)
{
	const char* extName = extensionName;
	CSynchType* pSynchExt = nullptr;

	const IEntity* pSynchEntity = gEnv->pEntitySystem->FindEntityByName(entityName);
	if (pSynchEntity)
	{
		IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchEntity->GetId());
		if (pGO)
		{
			pSynchExt = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
			assert(pSynchExt);

			//m_pSynchonizer->SetModule(this);
			m_pSynchonizer = pSynchExt;
			return pSynchExt;
		}
	}

	SEntitySpawnParams params;
	params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	params.bStaticEntityId = true;
	params.bIgnoreLock = true; // i dont know why i use it
	params.sName = entityName;
	params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;

	pSynchEntity = gEnv->pEntitySystem->SpawnEntity(params);
	assert(pSynchEntity);

	if (!pSynchEntity) return nullptr;

	//IGameObject* pGO = g_pGame->GetIGameFramework()->GetGameObject(pSynchronizer->GetId());
	IGameObject* pGO = g_pGame->GetIGameFramework()->GetIGameObjectSystem()->CreateGameObjectForEntity(pSynchEntity->GetId());
	if (pGO)
	{
		//m_pSynchonizer = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
		//assert(m_pSynchonizer);
		pSynchExt = dynamic_cast<CSynchType*>(pGO->AcquireExtension(extName));
		assert(pSynchExt);


		//m_pSynchonizer->SetModule(this);

		pSynchExt->GetGameObject()->ForceUpdate(true);
		m_pSynchonizer = pSynchExt;
	}

	return pSynchExt;
}
