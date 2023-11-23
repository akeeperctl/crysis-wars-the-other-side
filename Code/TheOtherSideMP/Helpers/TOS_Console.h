#pragma once

#define CHECK_CONSOLE_VAR_POINTER(varName, returnStatement)\
	CRY_ASSERT_MESSAGE(gEnv->pConsole, "gEnv->pConsole pointer is nullptr");\
	CRY_ASSERT_MESSAGE(gEnv->pConsole->GetCVar((varName)), "Console var pointer is nullptr");\
	if (!gEnv->pConsole || !gEnv->pConsole->GetCVar((varName)))\
		return (returnStatement)\


namespace TOS_Console
{
	inline float GetSafeFloatVar(const char* varName)
	{
		CHECK_CONSOLE_VAR_POINTER(varName, 0.0f);

		return gEnv->pConsole->GetCVar(varName)->GetFVal();
	}

	inline int GetSafeIntVar(const char* varName)
	{
		CHECK_CONSOLE_VAR_POINTER(varName, 0);

		return gEnv->pConsole->GetCVar(varName)->GetIVal();
	}

}
