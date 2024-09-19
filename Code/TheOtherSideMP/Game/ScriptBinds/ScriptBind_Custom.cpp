/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "TheOtherSideMP\Game\TOSGame.h"
#include "TheOtherSideMP\Helpers\TOS_Script.h"
#include "ScriptBind_Custom.h"

CScriptBind_Custom::CScriptBind_Custom(ISystem* pSystem, IGameFramework* pGameFramework)
{
	m_pSS = pSystem->GetIScriptSystem();

	RegisterMethods();
}

CScriptBind_Custom::~CScriptBind_Custom()
{
}

void CScriptBind_Custom::RegisterMethods()
{
# define REGISTER_TEMPLATE(sGlobalName, sFuncName, sFuncParams)\
	TOS_Script::RegisterTemplateFunction(sGlobalName, #sFuncName, sFuncParams, *this, &CScriptBind_Custom::sFuncName)

	REGISTER_TEMPLATE("System", TOSSpawnEntity, "params");
	REGISTER_TEMPLATE("AI", HasAI, "entityId");
}

/////////////////////////////////////////////////////////////////////////////////
void CScriptBind_Custom::MergeTable(IScriptTable* pDest, IScriptTable* pSrc)
{
	IScriptTable::Iterator it = pSrc->BeginIteration();

	while (pSrc->MoveNext(it))
	{
		if (pSrc->GetAtType(it.nKey) != svtNull)
		{
			if (pSrc->GetAtType(it.nKey) == svtObject)
			{
				SmartScriptTable tbl;

				if (pDest->GetAtType(it.nKey) != svtObject)
				{
					tbl = SmartScriptTable(m_pSS->CreateTable());
					pDest->SetAtAny(it.nKey, tbl);
				}
				else
				{
					tbl = SmartScriptTable(m_pSS, true);
					pDest->GetAt(it.nKey, tbl);
				}

				SmartScriptTable srcTbl;
				it.value.CopyTo(srcTbl);
				MergeTable(tbl, srcTbl);
			}
			else
			{
				pDest->SetAtAny(it.nKey, it.value);
			}
		}
		else if (pSrc->GetValueType(it.sKey) != svtNull)
		{
			if (pSrc->GetValueType(it.sKey) == svtObject)
			{
				SmartScriptTable tbl;

				if (pDest->GetValueType(it.sKey) != svtObject)
				{
					tbl = SmartScriptTable(m_pSS->CreateTable());
					pDest->SetValue(it.sKey, tbl);
				}
				else
				{
					tbl = SmartScriptTable(m_pSS, true);
					pDest->GetValue(it.sKey, tbl);
				}

				SmartScriptTable srcTbl;
				it.value.CopyTo(srcTbl);
				MergeTable(tbl, srcTbl);
			}
			else
			{
				pDest->SetValueAny(it.sKey, it.value);
			}
		}
	}

	pSrc->EndIteration(it);
}

//------------------------------------------------------------------------
int CScriptBind_Custom::HasAI(IFunctionHandler* pH, ScriptHandle entityId)
{
	const EntityId id = (EntityId)entityId.n;
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
	
	const bool hasAI = pEntity ? pEntity->GetAI() : false;
	return pH->EndFunction(hasAI);
}

int CScriptBind_Custom::TOSSpawnEntity(IFunctionHandler* pH, SmartScriptTable params)
{
	const char* entityClass = 0;
	const char* entityName = "";
	const char* archetypeName = 0;
	ScriptHandle entityId;
	SmartScriptTable propsTable(m_pSS, true);
	SmartScriptTable propsInstanceTable(m_pSS, true);

	Vec3 pos(0.0f, 0.0f, 0.0f);
	Vec3 dir(1.0f, 0.0f, 0.0f);
	Vec3 scale(1.0f, 1.0f, 1.0f);
	int flags = 0;
	bool props = false;
	bool propsInstance = false;

	{
		CScriptSetGetChain chain(params);

		chain.GetValue("id", entityId);
		chain.GetValue("class", entityClass);
		chain.GetValue("name", entityName);
		chain.GetValue("position", pos);
		chain.GetValue("orientation", dir);   //orientation unit vector
		chain.GetValue("scale", scale);
		chain.GetValue("flags", flags);
		chain.GetValue("archetype", archetypeName);

		if (params.GetPtr())
		{
			props = params.GetPtr()->GetValue("properties", propsTable);
			propsInstance = params.GetPtr()->GetValue("propertiesInstance", propsInstanceTable);
		}
	}

	ScriptHandle hdl;
	IEntity* pProtoEntity(NULL);

	if (pH->GetParamCount() > 1 && pH->GetParam(2, hdl))
	{
		pProtoEntity = gEnv->pEntitySystem->GetEntity((EntityId)hdl.n);
	}

	if (!entityClass)
	{
		return pH->EndFunction();
	}

	if (dir.IsZero(.1f))
	{
		dir = Vec3(1.0f, 0.0f, 0.0f);
		CryLogWarning("<CScriptBind_Custom> [TOSSpawnEntity] Zero orientation. Entity name %s", entityName);
	}
	else dir.NormalizeSafe();

	SEntitySpawnParams spawnParams;
	spawnParams.id = (EntityId)entityId.n;
	spawnParams.qRotation = Quat(Matrix33::CreateRotationVDir(dir));
	spawnParams.vPosition = pos;
	spawnParams.vScale = scale;
	spawnParams.sName = entityName;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(entityClass);

	if (archetypeName)
	{
		spawnParams.pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archetypeName);
	}

	if (!spawnParams.pClass)
	{
		CryLogError("<CScriptBind_Custom> [TOSSpawnEntity] No such entity class %s (entity name: %s)", entityClass, entityName);
		return pH->EndFunction();
	}
	// if there is a prototype - use some flags of prototype entity
	spawnParams.nFlags = flags |
		(pProtoEntity ?
		 pProtoEntity->GetFlags() & (ENTITY_FLAG_CASTSHADOW | ENTITY_FLAG_GOOD_OCCLUDER | ENTITY_FLAG_RECVWIND | ENTITY_FLAG_OUTDOORONLY)
		 : 0);

	IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams, !props);
	if (!pEntity)
		return pH->EndFunction();

	IScriptTable* pEntityTable = pEntity->GetScriptTable();

	if (props)
	{
		if (pEntityTable)
		{
			SmartScriptTable entityProps(m_pSS, false);

			if (pEntityTable->GetValue("Properties", entityProps))
			{
				MergeTable(entityProps, propsTable);
			}
			if (propsInstance && pEntityTable->GetValue("PropertiesInstance", entityProps))
			{
				MergeTable(entityProps, propsInstanceTable);
			}
		}

		gEnv->pEntitySystem->InitEntity(pEntity, spawnParams);
	}

	if (!pEntityTable)
	{
		CryLogError("<CScriptBind_Custom> [TOSSpawnEntity] Script table of spawned entity not created");
		return pH->EndFunction();
	}

	if (pEntity && pEntityTable)
	{
		return pH->EndFunction(pEntityTable);
	}

	return pH->EndFunction();
}
