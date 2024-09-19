/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
-------------------------------------------------------------------------
Description: Бинд луа функций к AITracker'у
*************************************************************************/
# pragma once

#include <IScriptSystem.h>
#include <ScriptHelpers.h>


struct IGameFramework;

// <title Actor>
// Syntax: Actor
class CScriptBind_AITracker :
	public CScriptableBase
{
public:
	CScriptBind_AITracker(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_AITracker();

	void RegisterMethods();
	void RegisterGlobals();

	int ExecuteAIAction(IFunctionHandler* pH, ScriptHandle user, ScriptHandle object, const char* actionName, const float maxAlertness, int actionGoalPipeId, const int combatFlag, const char* luaCallback, const char* desiredGoalName);

protected:
	SmartScriptTable m_pParams;

	IScriptSystem* m_pSS;
	ISystem* m_pSystem;
	IGameFramework* m_pGameFW;

};
