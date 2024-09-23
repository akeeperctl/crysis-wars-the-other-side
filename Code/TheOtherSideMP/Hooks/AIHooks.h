/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

Файл реализации хуков ИИ Системы и всё, что с ней связано
**************************************************************************/

#pragma once
#include "IAgent.h"
#include "ISystem.h"
#include "TheOtherSideMP/Utilities/VTables.h"
#include "TheOtherSideMP/Utilities/IndexFinder.h"
#include "TheOtherSideMP/Helpers/TOS_Hooks.h"

namespace TOS_Hooks
{
	namespace AI
	{
		struct IAIActorHook
		{
			bool CanAcquireTarget(IAIObject* pOther) const;
		};

		struct IAIObjectHook
		{
			bool IsHostile(const IAIObject* pOther, bool bUsingAIIgnorePlayer=true) const;
		};

		void ApplyHooks();
	}
};

