/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
#include "TheOtherSideMP\Utilities\VTables.h"
#include "CryMP\Library\WinAPI.h"

//struct AIActorHook
//{
//    bool CanAcquireTarget(IAIObject* pOther) const;
//};
//
//using CanAcquireTargetFunc = decltype(&AIActorHook::CanAcquireTarget);
//static CanAcquireTargetFunc g_originalCanAcquireTarget = nullptr;
//
//bool AIActorHook::CanAcquireTarget(IAIObject* pOther) const
//{
//    // add your code here
//    CryLogAlways("%s", __FUNCTION__);
//
//    return (this->*g_originalCanAcquireTarget)(pOther);
//}
//
//void InitCanAcquireTargetHook(IAIActor* pSomeAIActor)
//{
//    void** pAIActorVTable = *reinterpret_cast<void***>(pSomeAIActor);
//
//    // index of IAIActor::CanAcquireTarget
//    constexpr unsigned int FUNC_INDEX = 11;
//
//    g_originalCanAcquireTarget = reinterpret_cast<CanAcquireTargetFunc&>(pAIActorVTable[FUNC_INDEX]);
//
//    // vtable hook
//    CanAcquireTargetFunc newFunc = &AIActorHook::CanAcquireTarget;
//    WinAPI::FillMem(&pAIActorVTable[FUNC_INDEX], &reinterpret_cast<void*&>(newFunc), sizeof(void*));
//}

namespace TOS_Hooks
{
	// Get symbol name from address.
	//inline void GetSymbolNameFromAddr(DWORD SymbolAddress, string& csSymbolName)
	//{
	//	DWORD64 Displacement = 0;
	//	SYMBOL_INFO_PACKAGE SymbolInfo = {0};
	//	SymbolInfo.si.SizeOfStruct = sizeof(SYMBOL_INFO);
	//	SymbolInfo.si.MaxNameLen = sizeof(SymbolInfo.name);

	//	// Get symbol from address.
	//	::SymFromAddr(GetCurrentProcess(),
	//				  SymbolAddress,
	//				  &Displacement,
	//				  &SymbolInfo.si);

	//	csSymbolName = SymbolInfo.si.Name;
	//}

	/// @brief Заменить функцию в виртуальной таблице слева на функцию справа
	/// ПРИМЕР: TOS_Hooks::ReplaceFunction(&pVTable[11], &IAIActorHook::CanAcquireTarget);
	/// @param oldFuncVTableElement - указатель Void** на VTable
	/// @param newFuncAddress - адрес новой функции
	template <typename funcType1, typename funcType2>
	inline void ReplaceFunction(funcType1 oldFuncVTableAddress, funcType2 newFuncAddress)
	{
		//using CanAcquireTargetFunc = decltype(&IAIActorHook::CanAcquireTarget);
		//CanAcquireTargetFunc newFunc = &IAIActorHook::CanAcquireTarget;
		//constexpr unsigned int FUNC_INDEX = 11;

		//WinAPI::FillMem(&pVTable1[FUNC_INDEX], &reinterpret_cast<void*&>(newFunc), sizeof(void*));


		WinAPI::FillMem(oldFuncVTableAddress, &reinterpret_cast<void*&>(newFuncAddress), sizeof(void*));

		//	// vtable hook
//	//CanAcquireTargetFunc newFunc = &AIActorHook::CanAcquireTarget;
//	//WinAPI::FillMem(&pAIActorVTable[FUNC_INDEX], &reinterpret_cast<void*&>(newFunc), sizeof(void*));

	}
}
