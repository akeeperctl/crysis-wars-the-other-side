#pragma once

#include "TheOtherSideMP\Game\Modules\GenericModule.h"
#include <TheOtherSideMP\Actors\Player\TOSPlayer.h>

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
	bool		NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;

	int GetDebugLog()  { return m_debugLogMode; }

	void InitCVars(IConsole* pConsole)  {};
	void InitCCommands(IConsole* pConsole)  {};
	void ReleaseCVars()  {};
	void ReleaseCCommands()  {};
	//~ITOSGameModule

	void NetMakePlayerZeus(IActor* pPlayer);
	void ShowHUD(bool show);
private:
	void ApplyZeusProperties(IActor* pPlayer);
	CTOSPlayer* m_zeus;
};
