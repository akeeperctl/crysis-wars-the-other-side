#include "StdAfx.h"
#include "ScriptBind_AITracker.h"
#include "TheOtherSideMP\Helpers\TOS_AI.h"
#include <TheOtherSideMP\Helpers\TOS_Entity.h>

CScriptBind_AITracker::CScriptBind_AITracker(ISystem* pSystem, IGameFramework* pGameFramework)
	: m_pSystem(pSystem),
	m_pSS(pSystem->GetIScriptSystem()),
	m_pGameFW(pGameFramework)
{
	Init(m_pSS, m_pSystem);
	SetGlobalName("AITracker");
	RegisterMethods();
	RegisterGlobals();
}

CScriptBind_AITracker::~CScriptBind_AITracker()
{

}

void CScriptBind_AITracker::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_AITracker::

	SCRIPT_REG_TEMPLFUNC(ExecuteAIAction, "user, object, actionName, maxAlertness, goalPipeId, combatFlag, luaCallback, desiredPipeName");

#undef SCRIPT_REG_CLASSNAME
}

//------------------------------------------------------------------------
void CScriptBind_AITracker::RegisterGlobals()
{
	m_pSS->SetGlobalValue("IGNORE_COMBAT_DURING_ACTION", eAAEF_IgnoreCombatDuringAction);
	m_pSS->SetGlobalValue("JOIN_COMBAT_PAUSE_ACTION", eAAEF_JoinCombatPauseAction);
}

int CScriptBind_AITracker::ExecuteAIAction(IFunctionHandler* pH, ScriptHandle user, ScriptHandle object, const char* actionName, const float maxAlertness, int actionGoalPipeId, const int combatFlag, const char* luaCallback, const char* desiredGoalName)
{
	EntityId userId = user.n;
	EntityId objectId = object.n;

	auto pObject = TOS_GET_ENTITY(objectId);
	auto pUser = TOS_GET_ENTITY(userId);
	auto pUserAI = pUser ? pUser->GetAI() : nullptr;

	int usedGoalPipeId = TOS_AI::ExecuteAIAction(pUserAI, pObject, actionName, maxAlertness, actionGoalPipeId, EAAEFlag(combatFlag), desiredGoalName, "", luaCallback);

	return pH->EndFunction(usedGoalPipeId);
}
