#pragma once
#include "HUD/HUD.h"

namespace TOS_HUD
{
	//Надпись чуть выше середины. Можно настроить цвет
	//Пример: DisplayOverlayFlashMessage("@conq_you_can_switch_team_count", ColorF(0.0f, 1.0f, 0), true, times);
	inline void DisplayOverlayMessage(const char* label, const ColorF& col = Col_White, bool formatWStringWithParams = false, const char* paramLabel1 = 0, const char* paramLabel2 = 0, const char* paramLabel3 = 0, const char* paramLabel4 = 0)
	{
		SAFE_HUD_FUNC(DisplayOverlayFlashMessage(label, col, formatWStringWithParams, paramLabel1, paramLabel2, paramLabel3, paramLabel4));
	}

	//Надпись внизу, которая отображает подсказки
	inline void DisplayHintMessage(const char* label, float duration, int posX, int posY, ColorF col)
	{
		SAFE_HUD_FUNC(DisplayBigOverlayFlashMessage(label, duration, posX, posY, col));
	}

	//Понятия не имею что это
	//Должен быть загружен m_animMPMessages
	inline void DisplayMPMessage(const char* msg)
	{
		SAFE_HUD_FUNC(DisplayFunMessage(msg));
	}

	//Отображение записи в баттл логе
	//Должен быть загружен m_animBattleLog
	//Пример: BattleLogEvent(eBLE_Information, "@mp_BLYouPickedup", displayName);
	inline void DisplayBattleLogEvent(int type, const char* msg, const char* p0, const char* p1, const char* p2, const char* p3)
	{
		SAFE_HUD_FUNC(BattleLogEvent(type, msg, p0, p1, p2, p3))
	}

	//Отобразить текущие пушки в инвенторе на HUD
	//Должен быть загружен m_animWeaponSelection
	inline void ShowInventory(IActor* pActor, const char* category, const char* curClass)
	{
		SAFE_HUD_FUNC(TOSShowInventoryOverview(pActor, category, curClass, false));
	}
}