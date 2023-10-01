#pragma once

#include "ISystem.h"
#include "IRenderer.h"

#define STR_BLUE TOS_Debug::GetLogColor(ELogColor::blue)
#define STR_BROWN TOS_Debug::GetLogColor(ELogColor::brown)
#define STR_CYAN TOS_Debug::GetLogColor(ELogColor::cyan)
#define STR_GREEN TOS_Debug::GetLogColor(ELogColor::green)
#define STR_GREY TOS_Debug::GetLogColor(ELogColor::grey)
#define STR_PURPLE TOS_Debug::GetLogColor(ELogColor::purple)
#define STR_RED TOS_Debug::GetLogColor(ELogColor::red)
#define STR_YELLOW TOS_Debug::GetLogColor(ELogColor::yellow)

enum class ELogColor
{
	blue = 2,
	green = 3,
	red = 4,
	cyan = 5,
	yellow = 6,
	purple = 7,
	brown = 8,
	grey = 9
};

namespace TOS_Debug
{
	constexpr auto SIZE_HEADER = 1.3f;
	constexpr auto SIZE_COMMON = 1.1f;
	constexpr auto XOFFSET_COMMON = 60;
	constexpr auto YOFFSET_CONQ_CMDRS = 120;
	constexpr auto YOFFSET_CONQ_CHANNELS = 220;
	constexpr auto YOFFSET_CONQ_CMDR_SQUADS = 320;
	constexpr auto YOFFSET_AIACTION_TRACKER = 420;
	constexpr auto YOFFSET_SQUAD = 520;
	constexpr auto YOFFSET_CLIENTAREA = 520;

	// 29.09.2023 Crashing when log
	//inline void Log(const char* format, ...)
	//{
	//	if (gEnv && gEnv->pSystem)
	//	{
	//		string finalString;
	//		string env;
	//		string lang = "[C++]";

	//		if (gEnv->bServer)
	//			env = "[SERVER]";
	//		else if (gEnv->bClient)
	//			env = "[CLIENT]";

	//		if (strlen(format) < 3)
	//		{
	//			env = "";
	//			lang = "";
	//		}

	//		finalString = lang + env + format;
	//		const char* fs = finalString.c_str();

	//		va_list args;
	//		va_start(args, fs);
	//		gEnv->pLog->LogV(ILog::eAlways, fs, args);
	//		va_end(args);
	//	}
	//}

	// Summary
	//   Get string of type of action
	// Parameters
	//   type - type of action: 1 - FUNC CALL, 2 - CONSTR CALL, 3 - RMI RECEIVED.
	inline const char* GetAct(int type)
	{
		switch (type)
		{
		case 1:
			return "FUNC CALL";
		case 2:
			return "CONSTR CALL";
		case 3:
			return "RMI RECEIVED";
		default:
			return "UNDEFINED_DEBUG_ACTION";
			break;
		}
	}

	inline const char* GetEnv()
	{
		if (gEnv->bServer && !gEnv->bClient)
		{
			return "DEDICATED";
		}
		else if (gEnv->bServer && gEnv->bClient)
		{
			return "SERVERCLIENT";
		}
		else if (gEnv->bServer)
		{
			return "SERVER";
		}
		else if (gEnv->bClient)
		{
			return "CLIENT";
		}

		return "ENV_NULL";
	}

	inline void Draw2dText(float x, float y, float size, const char* format, ...)
	{
		if (gEnv && gEnv->pSystem)
		{
			float color[] = { 1,1,1,1 };

			va_list args;
			va_start(args, format);
			gEnv->pRenderer->Draw2dLabel(x, y, size, color,false ,format, args);
			va_end(args);
		}
	}

	inline const char* GetLogColor(ELogColor colorenum)
	{
		const char* color = 0;

		if (colorenum == ELogColor::blue)
		{
			color = "$2";
		}
		else if (colorenum == ELogColor::brown)
		{
			color = "$8";
		}
		else if (colorenum == ELogColor::cyan)
		{
			color = "$5";
		}
		else if (colorenum == ELogColor::green)
		{
			color = "$3";
		}
		else if (colorenum == ELogColor::grey)
		{
			color = "$9";
		}
		else if (colorenum == ELogColor::purple)
		{
			color = "$7";
		}
		else if (colorenum == ELogColor::red)
		{
			color = "$4";
		}
		else if (colorenum == ELogColor::yellow)
		{
			color = "$6";
		}


		return color;
	}
}