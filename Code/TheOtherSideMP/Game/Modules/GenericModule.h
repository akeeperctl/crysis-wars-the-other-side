#pragma once

#include "ITOSGameModule.h"

class CTOSGenericModule: public ITOSGameModule
{
public:
	CTOSGenericModule();
	~CTOSGenericModule();

	//ITOSGameModule
	bool OnInputEvent(const SInputEvent& event) override { return true; };
	bool OnInputEventUI(const SInputEvent& event) override { return false; };
	CTOSGenericSynchronizer* GetSynchronizer() const override;
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	void GetMemoryStatistics(ICrySizer* s) override;
	const char* GetName() override { return "CTOSGenericModule"; };
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//~ITOSGameModule

protected:
private:
};