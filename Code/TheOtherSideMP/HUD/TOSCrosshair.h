// ReSharper disable CppImplicitDefaultConstructorNotAvailable
// ReSharper disable CppInconsistentNaming
#pragma once
#include "HUD/HUDCrosshair.h"

/**
 * \brief Экспериментальный класс.
	\note Используется для переопределения оригинального класса прицела
	\note Подразумевалось, что это облегчит работу с интерфейсом прицеливания.
	\note 10/12/2023, 17:57: класс пока что не особо помагает, наоборот с ним больше мороки.
 */
class CTOSHUDCrosshair  : public CHUDCrosshair
{
	friend class CHUD;
	friend class CTOSMasterClient;
	//friend class CWeapon;
	//friend class CVehicleWeapon;
	//friend class CPlayer;
	//friend class CTOSPlayer;
	//friend class COffHand;
	//friend class CItem;
	//friend class CScriptBind_HUD;

public:
	explicit CTOSHUDCrosshair(CHUD* pHUD);

	// CHUDCrosshair
	void Update(float fDeltaTime) ;
	void UpdateCrosshair() ;
	// ~CHUDCrosshair


	void ShowFriendCross(bool show);

protected:
private:
};
