#pragma once

#include "../GenericModule.h"

// Description: 
//    A module designed to resurrect entities removed during the work of console command sv_restart
// Note:
//    As of 10/01/2023 Use only with archetypes.
//    Because I didn’t figure out how and where
//    to save the entire set of script parameters
//    in the computer’s memory, as well as how to load it from there later.
class CTOSRestartResurrectionModule final : public CTOSGenericModule  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSRestartResurrectionModule();
	~CTOSRestartResurrectionModule() override;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override {return "CTOSRestartResurrectionModule";};
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//~ITOSGameModule

private:

};