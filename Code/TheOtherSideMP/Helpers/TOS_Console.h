#pragma once

#define CHECK_CONSOLE_VAR_POINTER(varName, returnStatement)\
	CRY_ASSERT_MESSAGE(gEnv->pConsole, "gEnv->pConsole pointer is nullptr");\
	CRY_ASSERT_MESSAGE(gEnv->pConsole->GetCVar((varName)), "Console var pointer is nullptr");\
	if (!gEnv->pConsole || !gEnv->pConsole->GetCVar((varName)))\
		return (returnStatement)\


namespace TOS_Console
{
	inline float GetSafeFloatVar(const char* varName, const float defaultValue = 0.0f)
	{
		CHECK_CONSOLE_VAR_POINTER(varName, defaultValue);

		return gEnv->pConsole->GetCVar(varName)->GetFVal();
	}

	inline int GetSafeIntVar(const char* varName, const int defaultValue = 1)
	{
		CHECK_CONSOLE_VAR_POINTER(varName, defaultValue);

		return gEnv->pConsole->GetCVar(varName)->GetIVal();
	}

	inline string GetSafeStringVar(const char* varName, const char* defaultValue = "")
	{
		CHECK_CONSOLE_VAR_POINTER(varName, string(defaultValue));

		return gEnv->pConsole->GetCVar(varName)->GetString();
	}

}
