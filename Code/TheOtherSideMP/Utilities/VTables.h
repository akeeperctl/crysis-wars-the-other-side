/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

Comrade, thx for helping to create this!
**************************************************************************/

#pragma once

constexpr size_t IAIOBJECT_VTABLE_ADDRESS = 0x000000003122bda0;
constexpr size_t IAIACTOR_VTABLE_ADDRESS = 0x0000000041acae44;
constexpr size_t IAISYSTEM_VTABLE_ADDRESS = 0x000000003121c290;

namespace VTables
{
	/// @brief Получить указатель на VTable по адресу экземпляра класса.
	/// ПРИМЕР: void* pVTable = GetPointer(0x000000003121c290);
	/// @param address0x - адрес экземпляра класса.
	/// @return указатель на VTable
	inline void* GetPointer(size_t address0x)
	{
		return reinterpret_cast<void**>(address0x);
	}	
	
	/// @brief Получить указатель на VTable из экземпляра класса.
	/// ПРИМЕР: void** pAISystemVTable = GetVTable(gEnv->pAISystem);
	/// @param ptr - указатель на экземпяр класса.
	/// @return указатель на VTable
	template <typename objectType>
	inline void** GetVTable(objectType ptr)
	{
		return *reinterpret_cast<void***>(ptr);
	}

	/// @brief Вызвать функцию из VTable по индексу
	/// @tparam returnType - тип возврата функции
	/// @tparam funcType - тип функции
	/// @param pVTable - указатель на VTable
	/// @param functionIndex - индекс функции в VTable
	/// @return результат с типом returnType
	template <typename returnType, typename funcType>
	inline returnType CallFunction(void** pVTable, size_t functionIndex)
	{
		funcType* pFuncPointer = reinterpret_cast<funcType&>(pVTable[functionIndex]);
		returnType result = reinterpret_cast<returnType>(pFuncPointer());

		return result;
	};

	/// @brief Вызвать функцию из VTable по индексу
	/// @tparam returnType - тип возврата функции
	/// @tparam funcType - тип функции
	/// @param pVTable - указатель на VTable
	/// @param functionIndex - индекс функции в VTable
	/// @param param1 - параметр вызываемой функции
	/// @return результат с типом returnType
	template <typename returnType,
		typename funcType,
		typename param1_type>
	inline returnType CallFunction(void** pVTable, size_t functionIndex, param1_type param1)
	{
		funcType* pFuncPointer = reinterpret_cast<funcType&>(pVTable[functionIndex]);
		returnType result = reinterpret_cast<returnType>(pFuncPointer(param1));

		return result;
	};
	
	template <typename returnType,
		typename funcType,
		typename param1_type,
		typename param2_type>
	inline returnType CallFunction(void** pVTable, size_t functionIndex, param1_type param1, param2_type param2)
	{
		funcType* pFuncPointer = reinterpret_cast<funcType&>(pVTable[functionIndex]);
		returnType result = reinterpret_cast<returnType>(pFuncPointer(param1, param2));

		return result;
	};

	/// @brief Вызвать функцию из VTable по индексу у экземпляра класса
	/// @tparam returnType - тип возврата функции
	/// @tparam funcType - тип функции
	/// @param pVTable - указатель на VTable
	/// @param functionIndex - индекс функции
	/// @param pClassObject - экземпляр класса, у которого будет вызвана функция из VTable
	/// @param param1 - параметр вызываемой функции
	/// @return 
	template <typename returnType,
		typename funcType,
		typename classObjectType,
		typename param1_type>
	inline returnType CallFunction(void** pVTable, size_t functionIndex, classObjectType pClassObject, param1_type param1)
	{
		funcType* pFuncPointer = reinterpret_cast<funcType&>(pVTable[functionIndex]);
		return reinterpret_cast<returnType>(pClassObject->*pFuncPointer)(param1);
	};

	template <typename returnType,
		typename funcType,
		typename classObjectType,
		typename param1_type,
		typename param2_type>
	inline returnType CallFunction(void** pVTable, size_t functionIndex, classObjectType pClassObject, param1_type param1, param2_type param2)
	{
		funcType* pFuncPointer = reinterpret_cast<funcType&>(pVTable[functionIndex]);
		return reinterpret_cast<returnType>(pClassObject->*pFuncPointer)(param1, param2);
	};
}