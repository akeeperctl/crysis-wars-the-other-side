#pragma once

#include "ISystem.h"
#include "IRenderer.h"

#define DRAW_2D_TEXT(x,y,size,format,...)\
	if (gEnv && gEnv->pSystem)\
	{\
		float color[] = { 1,1,1,1 };\
		gEnv->pRenderer->Draw2dLabel(x, y, size, color, false, format, __VA_ARGS__);\
	}\

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

	inline void LogEntityFlags(const IEntity* pEntity)
	{
		assert(pEntity);
		if (!pEntity)
			return;

		CryLogAlways("Flags of %s entity:", pEntity->GetName());

		if (pEntity->CheckFlags(ENTITY_FLAG_CASTSHADOW))
			CryLogAlways("	%s", "ENTITY_FLAG_CASTSHADOW");
		if (pEntity->CheckFlags(ENTITY_FLAG_UNREMOVABLE))
			CryLogAlways("	%s", "ENTITY_FLAG_UNREMOVABLE");
		if (pEntity->CheckFlags(ENTITY_FLAG_GOOD_OCCLUDER))
			CryLogAlways("	%s", "ENTITY_FLAG_GOOD_OCCLUDER");
		if (pEntity->CheckFlags(ENTITY_FLAG_WRITE_ONLY))
			CryLogAlways("	%s", "ENTITY_FLAG_WRITE_ONLY");
		if (pEntity->CheckFlags(ENTITY_FLAG_NOT_REGISTER_IN_SECTORS))
			CryLogAlways("	%s", "ENTITY_FLAG_NOT_REGISTER_IN_SECTORS");
		if (pEntity->CheckFlags(ENTITY_FLAG_CALC_PHYSICS))
			CryLogAlways("	%s", "ENTITY_FLAG_CALC_PHYSICS");
		if (pEntity->CheckFlags(ENTITY_FLAG_CLIENT_ONLY))
			CryLogAlways("	%s", "ENTITY_FLAG_CLIENT_ONLY");
		if (pEntity->CheckFlags(ENTITY_FLAG_SERVER_ONLY))
			CryLogAlways("	%s", "ENTITY_FLAG_SERVER_ONLY");
		if (pEntity->CheckFlags(ENTITY_FLAG_CUSTOM_VIEWDIST_RATIO))
			CryLogAlways("	%s", "ENTITY_FLAG_CUSTOM_VIEWDIST_RATIO");
		if (pEntity->CheckFlags(ENTITY_FLAG_CALCBBOX_USEALL))
			CryLogAlways("	%s", "ENTITY_FLAG_CALCBBOX_USEALL");
		if (pEntity->CheckFlags(ENTITY_FLAG_VOLUME_SOUND))
			CryLogAlways("	%s", "ENTITY_FLAG_VOLUME_SOUND");
		if (pEntity->CheckFlags(ENTITY_FLAG_HAS_AI))
			CryLogAlways("	%s", "ENTITY_FLAG_HAS_AI");
		if (pEntity->CheckFlags(ENTITY_FLAG_TRIGGER_AREAS))
			CryLogAlways("	%s", "ENTITY_FLAG_TRIGGER_AREAS");
		if (pEntity->CheckFlags(ENTITY_FLAG_NO_SAVE))
			CryLogAlways("	%s", "ENTITY_FLAG_NO_SAVE");
		if (pEntity->CheckFlags(ENTITY_FLAG_NET_PRESENT))
			CryLogAlways("	%s", "ENTITY_FLAG_NET_PRESENT");
		if (pEntity->CheckFlags(ENTITY_FLAG_CLIENTSIDE_STATE))
			CryLogAlways("	%s", "ENTITY_FLAG_CLIENTSIDE_STATE");
		if (pEntity->CheckFlags(ENTITY_FLAG_SEND_RENDER_EVENT))
			CryLogAlways("	%s", "ENTITY_FLAG_SEND_RENDER_EVENT");
		if (pEntity->CheckFlags(ENTITY_FLAG_NO_PROXIMITY))
			CryLogAlways("	%s", "ENTITY_FLAG_NO_PROXIMITY");
		if (pEntity->CheckFlags(ENTITY_FLAG_ON_RADAR))
			CryLogAlways("	%s", "ENTITY_FLAG_ON_RADAR");
		if (pEntity->CheckFlags(ENTITY_FLAG_UPDATE_HIDDEN))
			CryLogAlways("	%s", "ENTITY_FLAG_UPDATE_HIDDEN");
		if (pEntity->CheckFlags(ENTITY_FLAG_NEVER_NETWORK_STATIC))
			CryLogAlways("	%s", "ENTITY_FLAG_NEVER_NETWORK_STATIC");
		if (pEntity->CheckFlags(ENTITY_FLAG_IGNORE_PHYSICS_UPDATE))
			CryLogAlways("	%s", "ENTITY_FLAG_IGNORE_PHYSICS_UPDATE");
		if (pEntity->CheckFlags(ENTITY_FLAG_SPAWNED))
			CryLogAlways("	%s", "ENTITY_FLAG_SPAWNED");
		if (pEntity->CheckFlags(ENTITY_FLAG_SLOTS_CHANGED))
			CryLogAlways("	%s", "ENTITY_FLAG_SLOTS_CHANGED");
		if (pEntity->CheckFlags(ENTITY_FLAG_MODIFIED_BY_PHYSICS))
			CryLogAlways("	%s", "ENTITY_FLAG_MODIFIED_BY_PHYSICS");
		if (pEntity->CheckFlags(ENTITY_FLAG_OUTDOORONLY))
			CryLogAlways("	%s", "ENTITY_FLAG_OUTDOORONLY");
		if (pEntity->CheckFlags(ENTITY_FLAG_SEND_NOT_SEEN_TIMEOUT))
			CryLogAlways("	%s", "ENTITY_FLAG_SEND_NOT_SEEN_TIMEOUT");
		if (pEntity->CheckFlags(ENTITY_FLAG_RECVWIND))
			CryLogAlways("	%s", "ENTITY_FLAG_RECVWIND");
		if (pEntity->CheckFlags(ENTITY_FLAG_LOCAL_PLAYER))
			CryLogAlways("	%s", "ENTITY_FLAG_LOCAL_PLAYER");
		if (pEntity->CheckFlags(ENTITY_FLAG_AI_HIDEABLE))
			CryLogAlways("	%s", "ENTITY_FLAG_AI_HIDEABLE");
	}

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

const char* const TOS_COLOR_BLUE = TOS_Debug::GetLogColor(ELogColor::blue);
const char* const TOS_COLOR_BROWN = TOS_Debug::GetLogColor(ELogColor::brown);
const char* const TOS_COLOR_CYAN = TOS_Debug::GetLogColor(ELogColor::cyan);
const char* const TOS_COLOR_GREEN = TOS_Debug::GetLogColor(ELogColor::green);
const char* const TOS_COLOR_GREY = TOS_Debug::GetLogColor(ELogColor::grey);
const char* const TOS_COLOR_PURPLE = TOS_Debug::GetLogColor(ELogColor::purple);
const char* const TOS_COLOR_RED = TOS_Debug::GetLogColor(ELogColor::red);
const char* const TOS_COLOR_YELLOW = TOS_Debug::GetLogColor(ELogColor::yellow);
