#pragma once

#include "TheOtherSideMP\Game\Modules\GenericModule.h"
#include <TheOtherSideMP\Actors\Player\TOSPlayer.h>

enum EZeusFlags
{
	eZF_CanRotateCamera = BIT(0),
};

/**
 * \brief Модуль для обработки ситуаций режима игры Zeus
 */
class CTOSZeusModule : public CTOSGenericModule
{
public:
	CTOSZeusModule();
	virtual ~CTOSZeusModule();

	//ITOSGameModule
	bool        OnInputEvent(const SInputEvent& event)  { return true; };
	bool        OnInputEventUI(const SInputEvent& event)  { return false; };
	void        OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	void        GetMemoryStatistics(ICrySizer* s) ;
	const char* GetName()  { return "CTOSGenericModule"; };
	void        Init() ;
	void        Update(float frametime) ;
	void        Serialize(TSerialize ser) ;

	int GetDebugLog()  { return m_debugLogMode; }

	void InitCVars(IConsole* pConsole)  {};
	void InitCCommands(IConsole* pConsole)  {};
	void ReleaseCVars()  {};
	void ReleaseCCommands()  {};
	//~ITOSGameModule

	void NetMakePlayerZeus(IActor* pPlayer);
	void ShowHUD(bool show);

	void SetZeusFlag(uint flag, bool value);
	bool GetZeusFlag(uint flag) const;

	void OnAction(const ActionId& action, int activationMode, float value);
protected:
	bool OnActionAttack2(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);

private:
	void ShowMouse(bool show);
	void ApplyZeusProperties(IActor* pPlayer);
	CTOSPlayer* m_zeus;
	uint m_zeusFlags;
	Vec2 m_anchoredMousePos;
};
