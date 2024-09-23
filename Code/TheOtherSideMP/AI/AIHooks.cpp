/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

Файл реализации хуков ИИ Системы и всё, что с ней связано
**************************************************************************/

#include "StdAfx.h"
#include "AIHooks.h"

#ifdef _WIN64
	constexpr size_t IAIOBJECT_VTABLE_ADDRESS = 0x000000003122bda0;
	constexpr size_t IAIACTOR_VTABLE_ADDRESS = 0x0000000041acae44;
	constexpr size_t IAISYSTEM_VTABLE_ADDRESS = 0x000000003121c290;
#elif _WIN32
	constexpr size_t IAIOBJECT_VTABLE_ADDRESS = 0x311ba5d0;
	constexpr size_t IAIACTOR_VTABLE_ADDRESS = 0x311ba598;
	constexpr size_t IAISYSTEM_VTABLE_ADDRESS = 0x311b0fa0;
#endif


// Определение функций хука обязательно должно быть вне структуры
// Иначе при получении адреса целевой функции будет выдаваться адрес совсем другой функции
/////////////////////////////////////////////////////////////

void TOS_Hooks::AI::ApplyHooks()
{
	auto pVTable1 = Utils::VTables::GetVTableFromAddress(IAIACTOR_VTABLE_ADDRESS);
	auto index1 = IndexFinder::getIndexOf(&IAIActor::CanAcquireTarget);
	TOS_Hooks::ReplaceFunction(&pVTable1[index1], &IAIActorHook::CanAcquireTarget);

	auto pVTable2 = Utils::VTables::GetVTableFromAddress(IAIOBJECT_VTABLE_ADDRESS);
	auto index2 = IndexFinder::getIndexOf(&IAIObject::IsHostile);
	TOS_Hooks::ReplaceFunction(&pVTable2[index2], &IAIObjectHook::IsHostile);
}

bool TOS_Hooks::AI::IAIActorHook::CanAcquireTarget(IAIObject* pOther) const
{
	//CryLogAlways("IAIActorHook::CanAcquireTarget");
	return false;
};

bool TOS_Hooks::AI::IAIObjectHook::IsHostile(const IAIObject* pOther, bool bUsingAIIgnorePlayer) const
{
	//CryLogAlways("IAIObjectHook::IsHostile");
	return false;
};
