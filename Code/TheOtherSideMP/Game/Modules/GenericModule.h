#pragma once

#include "ITOSGameModule.h"

class CTOSGenericModule: public ITOSGameModule  // NOLINT(cppcoreguidelines-special-member-functions)
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
	//~ITOSGameModule

	virtual CTOSGenericSynchronizer* GetSynchronizer() const;
	virtual CTOSGenericSynchronizer* CreateSynchonizer(const char* entityName, const char* extensionName);

protected:
	CTOSGenericSynchronizer* m_pSynchonizer;

private:

};