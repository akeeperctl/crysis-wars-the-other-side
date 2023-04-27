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
}