/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include "ISystem.h"
#include "IRenderer.h"

#define DRAW_2D_TEXT(x,y,size,format,...)\
	if (gEnv && gEnv->pSystem)\
	{\
		float color[] = { 1,1,1,1 };\
		gEnv->pRenderer->Draw2dLabel(x, y, size, color, false, format, __VA_ARGS__);\
	}\

enum ELogColor
{
	eLC_BLUE = 2,
	eLC_GREEN = 3,
	eLC_RED = 4,
	eLC_CYAN = 5,
	eLC_YELLOW = 6,
	eLC_PURPLE = 7,
	eLC_BROWN = 8,
	eLC_GREY = 9
};

//////////////////////////////////////////////////////////////////////////
// Error log of data with verbosity.
void CryLogError(const char *, ...) PRINTF_PARAMS(1, 2);
inline void CryLogError( const char *format,... )
{
//	return;
	if (gEnv && gEnv->pSystem)
	{
		va_list args;
		va_start(args,format);
		gEnv->pLog->LogV( ILog::eError,format,args );
		va_end(args);
	}
}

//////////////////////////////////////////////////////////////////////////
// Warning log of data with verbosity.
void CryLogWarning(const char *, ...) PRINTF_PARAMS(1, 2);
inline void CryLogWarning( const char *format,... )
{
//	return;
	if (gEnv && gEnv->pSystem)
	{
		va_list args;
		va_start(args,format);
		gEnv->pLog->LogV( ILog::eWarning,format,args );
		va_end(args);
	}
}

namespace TOS_Debug
{
	const auto SIZE_HEADER = 1.3f;
	const auto SIZE_COMMON = 1.1f;
	const auto XOFFSET_COMMON = 60;
	const auto YOFFSET_CONQ_CMDRS = 120;
	const auto YOFFSET_CONQ_CHANNELS = 220;
	const auto YOFFSET_CONQ_CMDR_SQUADS = 320;
	const auto YOFFSET_AIACTION_TRACKER = 420;
	const auto YOFFSET_SQUAD = 520;
	const auto YOFFSET_CLIENTAREA = 520;

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

	inline void DumpEntityFlags(const IEntity* pEntity)
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

	/// @brief 
	///   Отображает имена сущностей в 2D-тексте на экране.
	/// @tparam Container 
	///   Тип контейнера, содержащего идентификаторы сущностей.
	/// @param startX 
	///   Начальная координата X для отображения текста.
	/// @param startY 
	///   Начальная координата Y для отображения текста.
	/// @param deltaY 
	///   Вертикальное расстояние между строками текста.
	/// @param headerTextSize 
	///   Размер шрифта для заголовка.
	/// @param enumTextSize 
	///   Размер шрифта для имен сущностей.
	/// @param header 
	///   Текст заголовка, отображаемый вверху списка.
	/// @param container 
	///   Контейнер, содержащий идентификаторы сущностей для отображения.
	template <typename Container>
	inline void DrawEntitiesName2DLabel(const Container& container, const char* header, int startX = 100, int startY = 100, int deltaY = 20, float headerTextSize = 1.4f, float enumTextSize = 1.3f)
	{
		float color[] = {1,1,1,1};
		gEnv->pRenderer->Draw2dLabel(startX, startY, headerTextSize, color, false, "%s", header);

		for (auto it = container.cbegin(); it != container.cend(); it++)
		{
			const auto pEntity = gEnv->pEntitySystem->GetEntity(*it);
			if (!pEntity)
				continue;

			const char* name = pEntity->GetName();
			const int index = std::distance(container.cbegin(), it);

			const int y = startY + deltaY + index * deltaY;
			gEnv->pRenderer->Draw2dLabel(startX, y, enumTextSize, color, false, "	%i) %s", index, name);
		}
	}

	// Описание
	//   Получить строку с типом действия
	// Параметры
	//   type - тип действия: 1 - ВЫЗОВ ФУНКЦИИ, 2 - ВЫЗОВ КОНСТРУКТОРА, 3 - ПОЛУЧЕН RMI.
	inline const char* GetAct(int type)
	{
		switch (type)
		{
			case 1:
				return "func call";
			case 2:
				return "constr call";
			case 3:
				return "rmi received";
			default:
				return "undefined";
				break;
		}
	}

	inline const char* GetEnv()
	{
		if (gEnv->bServer && !gEnv->bClient)
		{
			return "D SV";
		}
		else if (gEnv->bServer && gEnv->bClient)
		{
			return "SV+CL";
		}
		else if (gEnv->bServer)
		{
			return "SV";
		}
		else if (gEnv->bClient)
		{
			return "CL";
		}

		return "NONE";
	}

	/// @brief Возвращает строковое представление цвета для логирования.
	/// 
	/// Эта функция принимает параметр перечисления ELogColor и возвращает строку,
	/// которая используется для визуального выделения сообщений в логах с помощью цвета.
	/// 
	/// @param colorenum Перечисление ELogColor, представляющее цвет.
	/// @return Строка, представляющая цвет в формате "$N", где N — код цвета.
	///
	/// @li Синий: "\$2"
	/// @li Зеленый: "\$3"
	/// @li Красный: "\$4"
	/// @li Голубой: "\$5"
	/// @li Желтый: "\$6"
	/// @li Фиолетовый: "\$7"
	/// @li Коричневый: "\$8"
	/// @li Серый: "\$9"
	inline const char* GetLogColor(ELogColor colorenum)
	{
		const char* color = 0;

		if (colorenum == ELogColor::eLC_BLUE)
		{
			color = "$2";
		}
		else if (colorenum == ELogColor::eLC_BROWN)
		{
			color = "$8";
		}
		else if (colorenum == ELogColor::eLC_CYAN)
		{
			color = "$5";
		}
		else if (colorenum == ELogColor::eLC_GREEN)
		{
			color = "$3";
		}
		else if (colorenum == ELogColor::eLC_GREY)
		{
			color = "$9";
		}
		else if (colorenum == ELogColor::eLC_PURPLE)
		{
			color = "$7";
		}
		else if (colorenum == ELogColor::eLC_RED)
		{
			color = "$4";
		}
		else if (colorenum == ELogColor::eLC_YELLOW)
		{
			color = "$6";
		}
		return color;
	}
}

const char* const TOS_COLOR_BLUE = TOS_Debug::GetLogColor(ELogColor::eLC_BLUE);
const char* const TOS_COLOR_BROWN = TOS_Debug::GetLogColor(ELogColor::eLC_BROWN);
const char* const TOS_COLOR_CYAN = TOS_Debug::GetLogColor(ELogColor::eLC_CYAN);
const char* const TOS_COLOR_GREEN = TOS_Debug::GetLogColor(ELogColor::eLC_GREEN);
const char* const TOS_COLOR_GREY = TOS_Debug::GetLogColor(ELogColor::eLC_GREY);
const char* const TOS_COLOR_PURPLE = TOS_Debug::GetLogColor(ELogColor::eLC_PURPLE);
const char* const TOS_COLOR_RED = TOS_Debug::GetLogColor(ELogColor::eLC_RED);
const char* const TOS_COLOR_YELLOW = TOS_Debug::GetLogColor(ELogColor::eLC_YELLOW);
