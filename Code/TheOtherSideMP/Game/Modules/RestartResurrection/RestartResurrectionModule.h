#pragma once

#include "../GenericModule.h"

// Description: 
//    A module designed to resurrect entities removed during the work of console command sv_restart   
class CTOSRestartResurrectionModule : public CTOSGenericModule
{
public:
	CTOSRestartResurrectionModule();
	~CTOSRestartResurrectionModule();

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override {return "CTOSRestartResurrectionModule";};
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//~ITOSGameModule

private:

};