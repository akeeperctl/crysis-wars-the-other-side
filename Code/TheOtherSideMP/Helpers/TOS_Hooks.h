/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
#include "TheOtherSideMP\Utilities\VTables.h"
#include "CryMP\Library\WinAPI.h"

// EXAMPLES
//////////////////////////////////////////////////////////////////////////

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

//inline void InitCanAcquireTargetHook(CAIActor* pCAIActor, IAIActor* pIAIActor, IAIObject* pAI)
//{
	//auto pAIVTable1 = reinterpret_cast<void**>(aiVTableAdress); // 
	//void** pAIVTable2 = *reinterpret_cast<void***>(pCAIObject); // 
	//void** pCAIActorVTable = *reinterpret_cast<void***>(pCAIActor); // 0x311ba5d0
	//void** pIAIActorVTable = *reinterpret_cast<void***>(pIAIActor); // 0x311ba598
	//void** pAISystemVTable = *reinterpret_cast<void***>(gEnv->pAISystem); // 0x311b0fa0

	// IAIObject::IsHostile 35
	// IAISystem::ExecuteAIAction 146
	// CAIActor::IsObserver 14

	//auto pFuncPointer = &CAIObject::IsObservable;
	//int index = IndexFinder::getIndexOf(pFuncPointer);

	//auto pVTableFuncPointer = reinterpret_cast<IsObservableFunc&>(pAIVTable1[11]);
	//bool value = (pCAIObject->*pVTableFuncPointer)();

	//CallVTableFunction<bool, IsObservableFunc>(pAIVTable1, 1, pCAIObject);
	
	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc>(pAISystemVTable[index]);
	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc*>(&pAISystemVTable[index]);
	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc>(pAISystemVTable[index]);
	//int result = pVTableFunctionPointer(gEnv->pAISystem);

	// index of IAIActor::CanAcquireTarget
	//constexpr unsigned int FUNC_INDEX = 11;

	// vtable hook
	//CanAcquireTargetFunc newFunc = &AIActorHook::CanAcquireTarget;
	//WinAPI::FillMem(&pAIActorVTable[FUNC_INDEX], &reinterpret_cast<void*&>(newFunc), sizeof(void*));
//}

//////////////////////////////////////////////////////////////////////////
// ~EXAMPLES

namespace TOS_Hooks
{
	/// @brief Заменить функцию в виртуальной таблице слева на функцию справа
	/// ПРИМЕР: TOS_Hooks::ReplaceFunction(&pVTable[11], &IAIActorHook::CanAcquireTarget);
	/// @param oldFuncVTableElement - указатель Void** на VTable
	/// @param newFuncAddress - адрес новой функции
	template <typename funcType1, typename funcType2>
	inline void ReplaceFunction(funcType1 oldFuncVTableAddress, funcType2 newFuncAddress)
	{
		WinAPI::FillMem(oldFuncVTableAddress, &reinterpret_cast<void*&>(newFuncAddress), sizeof(void*));
	}
}
