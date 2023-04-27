/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// FlowSpawnEntity.cpp
//
// Purpose: FG node to spawn an entity
//
// History:
//	- 6/05/08 : File created - KAK
/////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include <IEntitySystem.h>
#include <IGameObject.h>
#include <Actor.h>

/////////////////////////////////////////////////////////////////
class CFlowNode_SpawnEntity : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Spawn,
		EIP_ClassName,
		EIP_Name,
		EIP_Pos,
		EIP_Rot,
		EIP_Scale,
		EIP_Properties,
		EIP_PropertiesInstance,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_Succeeded,
		EOP_Failed,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SpawnEntity(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SpawnEntity(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Spawn", _HELP("Spawn an entity using the values below")),
			InputPortConfig<string>("Class", "", _HELP("Entity class name i.e., \"BasicEntity\""), 0, 0),
			InputPortConfig<string>("Name", "", _HELP("Entity's name"), 0, 0),
			InputPortConfig<Vec3>("Pos", _HELP("Initial position")),
			InputPortConfig<Vec3>("Rot", _HELP("Initial rotation")),
			InputPortConfig<Vec3>("Scale", Vec3(1,1,1), _HELP("Initial scale")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Done", _HELP("Called when job is done")),
			OutputPortConfig<EntityId>("Succeeded", _HELP("Called when entity is spawned")),
			OutputPortConfig_Void("Failed", _HELP("Called when entity fails to spawn")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Spawns an entity with the specified properties");
		config.SetCategory(EFLN_ADVANCED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Spawn))
			{
				// Get properties
				string className(GetPortString(pActInfo, EIP_ClassName));
				string name(GetPortString(pActInfo, EIP_Name));
				Vec3 pos(GetPortVec3(pActInfo, EIP_Pos));
				Vec3 rot(GetPortVec3(pActInfo, EIP_Rot));
				Vec3 scale(GetPortVec3(pActInfo, EIP_Scale));

				// Define
				SEntitySpawnParams params;
				params.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className.c_str());
				params.sName = name.c_str();
				params.vPosition = pos;
				params.vScale = scale;

				Matrix33 mat;
				Ang3 ang(DEG2RAD(rot.x), DEG2RAD(rot.y), DEG2RAD(rot.z));
				mat.SetRotationXYZ(ang);
				params.qRotation = Quat(mat);

				// Create
				IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(params, true);
				pEntity->GetPhysics()->Release();
				if (NULL == pEntity)
					ActivateOutput(pActInfo, EOP_Failed, true);
				else
					ActivateOutput(pActInfo, EOP_Succeeded, pEntity->GetId());
				ActivateOutput(pActInfo, EOP_Done, true);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SpawnEntity(pActInfo);
	}
};

/////////////////////////////////////////////////////////////////
class CFlowNode_SpawnArchetypeEntity : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Spawn,
		EIP_ArchetypeName,
		EIP_Name,
		EIP_Pos,
		EIP_Rot,
		EIP_Scale,
	};

	enum EOutputPorts
	{
		EOP_Done,
		EOP_Succeeded,
		EOP_Failed,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SpawnArchetypeEntity(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SpawnArchetypeEntity(void)
	{
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Spawn", _HELP("Spawn an entity using the values below")),
			InputPortConfig<string>("Archetype", "", _HELP("Entity archetype name"), 0, 0),
			InputPortConfig<string>("Name", "", _HELP("Entity's name"), 0, 0),
			InputPortConfig<Vec3>("Pos", _HELP("Initial position")),
			InputPortConfig<Vec3>("Rot", _HELP("Initial rotation")),
			InputPortConfig<Vec3>("Scale", Vec3(1,1,1), _HELP("Initial scale")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_Void("Done", _HELP("Called when job is done")),
			OutputPortConfig<EntityId>("Succeeded", _HELP("Called when entity is spawned")),
			OutputPortConfig_Void("Failed", _HELP("Called when entity fails to spawn")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Spawns an archetype entity with the specified properties");
		config.SetCategory(EFLN_ADVANCED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Spawn))
			{
				// Get properties
				string archName(GetPortString(pActInfo, EIP_ArchetypeName));
				string name(GetPortString(pActInfo, EIP_Name));
				Vec3 pos(GetPortVec3(pActInfo, EIP_Pos));
				Vec3 rot(GetPortVec3(pActInfo, EIP_Rot));
				Vec3 scale(GetPortVec3(pActInfo, EIP_Scale));

				// Define
				IEntity* pEntity = NULL;
				SEntitySpawnParams params;
				IEntityArchetype* pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archName.c_str());
				if (NULL != pArchetype)
				{
					params.pArchetype = pArchetype;
					params.sName = name.empty() ? pArchetype->GetName() : name.c_str();
					params.vPosition = pos;
					params.vScale = scale;

					Matrix33 mat;
					Ang3 ang(DEG2RAD(rot.x), DEG2RAD(rot.y), DEG2RAD(rot.z));
					mat.SetRotationXYZ(ang);
					params.qRotation = Quat(mat);
					// Create
					pEntity = gEnv->pEntitySystem->SpawnEntity(params, true);
				}
				if (NULL == pEntity)
					ActivateOutput(pActInfo, EOP_Failed, true);
				else
					ActivateOutput(pActInfo, EOP_Succeeded, pEntity->GetId());
				ActivateOutput(pActInfo, EOP_Done, true);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SpawnArchetypeEntity(pActInfo);
	}
};

//////////////////////////////////////////////////// 	gEnv->pEntitySystem->RemoveEntity
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("Entity:Spawn", CFlowNode_SpawnEntity);
REGISTER_FLOW_NODE("Entity:SpawnArchetype", CFlowNode_SpawnArchetypeEntity);