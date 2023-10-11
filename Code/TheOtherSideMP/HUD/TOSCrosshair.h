// ReSharper disable CppImplicitDefaultConstructorNotAvailable
// ReSharper disable CppInconsistentNaming
#pragma once
#include "HUD/HUDCrosshair.h"

class CTOSHUDCrosshair final : CHUDCrosshair
{
	friend class CHUD;
	friend class CTOSMasterClient;

public:
	explicit CTOSHUDCrosshair(CHUD* pHUD);

	// CHUDCrosshair
	void Update(float fDeltaTime) override;
	void UpdateCrosshair() override;
	// ~CHUDCrosshair


	void ShowFriendCross(bool show);

protected:
private:
};
