/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include <IEntitySystem.h>
#include <IScriptSystem.h>

namespace TOS_Script
{
	template<typename T>bool GetEntityProperty(IEntity* pEntity, const char* name, T& value)
	{
		if (!pEntity)
			return false;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue("Properties", props))
			return props->GetValue(name, value);
		return false;
	}

	template<typename T>bool GetEntityProperty(IEntity* pEntity, const char* table, const char* name, T& value)
	{
		if (!pEntity)
			return false;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue("Properties", props))
		{
			SmartScriptTable subprop;
			if (props->GetValue(table, subprop))
				return subprop->GetValue(name, value);
		}
		return false;
	}

	template<typename T>bool GetEntityScriptValue(IEntity* pEntity, const char* propertiesTable, const char* name, T& value)
	{
		if (!pEntity)
			return false;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue(propertiesTable, props))
		{
			props->GetValue(name, value);
			return true;
		}

		return false;
	}

	template<typename T>bool GetEntityScriptValue(IEntity* pEntity, const char* name, T& value)
	{
		if (!pEntity)
			return false;

		IScriptTable* pScriptTable = pEntity->GetScriptTable();

		return pScriptTable && pScriptTable->GetValue(name, value);
	}

	template<typename T>bool SetEntityScriptValue(IEntity* pEntity, const char* name, const T& value)
	{
		if (!pEntity)
			return false;

		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable)
		{
			pScriptTable->SetValue(name, value);
			return true;
		}

		return false;
	}


	template<typename T>bool SetEntityScriptValue(IEntity* pEntity, const char* propertiesTable, const char* name, const T& value)
	{
		if (!pEntity)
			return false;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue(propertiesTable, props))
		{
			props->SetValue(name, value);
			return true;
		}

		return false;
	}

	template<typename T>bool SetEntityScriptValue(IEntity* pEntity, const char* propertiesTable, const char* table, const char* name, T& value)
	{
		if (!pEntity)
			return false;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue(propertiesTable, props))
		{
			SmartScriptTable subprop;
			if (props->GetValue(table, subprop))
			{
				subprop->SetValue(name, value);
				return true;
			}

		}

		return false;
	}

	template<typename T>void SetEntityProperty(IEntity* pEntity, const char* table, const char* name, T& value)
	{
		if (!pEntity)
			return;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue("Properties", props))
		{
			SmartScriptTable subprop;
			if (props->GetValue(table, subprop))
				subprop->SetValue(name, value);
		}
	}


	template<typename T>void SetEntityProperty(IEntity* pEntity, const char* name, const T& value)
	{
		if (!pEntity)
			return;

		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->GetValue("Properties", props))
			props->SetValue(name, value);
	}

	inline void RegisterFunction(const char* sTableName, const char* sFuncName, IScriptTable::FunctionFunctor function)
	{
		auto pSS = gEnv->pScriptSystem;
		if (!pSS)
			return;

		bool tableNameNotFound = false;

		SmartScriptTable table;
		if (!pSS->GetGlobalValue(sTableName, table))
		{
			tableNameNotFound = true;
			table = pSS->CreateTable();
			table->AddRef();
		}

		if (!table.GetPtr())
			return;

		if (tableNameNotFound)
		{
			pSS->SetGlobalValue(sTableName, table);
		}

		IScriptTable::SUserFunctionDesc fd;
		fd.sGlobalName = sTableName;
		fd.sFunctionName = sFuncName;
		fd.pFunctor = function;
		fd.nParamIdOffset = 0;
		table->AddFunction(fd);
	}

	template <typename Callee, typename Func>
	void RegisterTemplateFunction(const char* sTableName, const char* sFuncName, const char* sFuncParams, Callee& callee, const Func& func)
	{
		auto pSS = gEnv->pScriptSystem;
		if (!pSS)
			return;

		bool tableNameNotFound = false;

		SmartScriptTable table;
		if (!pSS->GetGlobalValue(sTableName, table))
		{
			tableNameNotFound = true;
			table = pSS->CreateTable();
			table->AddRef();
		}

		if (!table.GetPtr())
			return;

		if (tableNameNotFound)
			pSS->SetGlobalValue(sTableName, table);


		typedef Callee* Callee_pointer;
		Callee_pointer pCalleePtr = &callee;
		unsigned char pBuffer[sizeof(Callee_pointer) + sizeof(func)];
		memcpy(pBuffer, &pCalleePtr, sizeof(Callee_pointer));
		memcpy(pBuffer + sizeof(Callee_pointer), &func, sizeof(func));

		IScriptTable::SUserFunctionDesc fd;
		fd.sGlobalName = sTableName;
		fd.sFunctionName = sFuncName;
		fd.sFunctionParams = sFuncParams;
		fd.pUserDataFunc = ScriptTemplateCallHelper::CallDispatcher<Callee, Func>::Dispatch;
		fd.nDataSize = sizeof(Callee_pointer) + sizeof(func);
		fd.pDataBuffer = pBuffer;
		fd.nParamIdOffset = 0;

		table->AddFunction(fd);
	}
}