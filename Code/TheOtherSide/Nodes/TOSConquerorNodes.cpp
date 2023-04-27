/*************************************************************************
Copyright (C), CryTechLab, 2022.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 26:04:2022   10.04: Created by Akeeper

*************************************************************************/

// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppClangTidyClangDiagnosticSwitch
#include "StdAfx.h"

#include "TOSNodesIncludes.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Conqueror/StrategicArea.h"
#include "TheOtherSide/Conqueror/RequestsAndResponses/Request.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "HUD/HUD.h"
#include "TheOtherSide/Helpers/TOS_AI.h"

class CFlowNode_SpeciesGetInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_CheckSpecies,
	};

	enum EOutputPorts
	{
		EOP_SpeciesAgentCount, //GetAgentCountBySpecies
		EOP_EnemiesCount, //GetSpeciesEnemiesCount
		EOP_CaptureProgress, //GetCaptureProgress
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SpeciesGetInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SpeciesGetInfo()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:0=0,1=1,2=2";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<int>("Species",0 , _HELP(""),"CheckSpecies"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("AgentCount",_HELP(""),"AgentCount"),
			OutputPortConfig<int>("EnemiesCount",_HELP(""),"EnemiesCount"),
			OutputPortConfig<float>("CaptureProgress",_HELP(""),"CaptureProgress"),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			IEntity* pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			if (IsPortActive(pActInfo,EIP_Sync))
			{
				CStrategicArea* pArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pEntity->GetId(), 0);
				if (pArea)
				{
					const ESpeciesType species = ESpeciesType(GetPortInt(pActInfo, EIP_CheckSpecies));

					const int agentCount = pArea->GetAgentCount(species);
					const int enemiesCount = pArea->GetEnemiesCount(species);
					const float captureProgress = pArea->GetCaptureProgress(species);

					ActivateOutput(pActInfo, EOP_SpeciesAgentCount, agentCount);
					ActivateOutput(pActInfo, EOP_EnemiesCount, enemiesCount);
					ActivateOutput(pActInfo, EOP_CaptureProgress, captureProgress);
				}
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SpeciesGetInfo(pActInfo);
	}
};

class CFlowNode_AreaGetInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_ActorCount, //GetActorCount
		EOP_CaptureState, //GetCaptureState
		EOP_IsCapturable, //IsCapturable
		EOP_Species,
		//EOP_Team,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_AreaGetInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_AreaGetInfo()
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("ActorCount",_HELP(""),"ActorCount"),
			OutputPortConfig<int>("CaptureState",_HELP("0=NOTCAPTURED, 1=CAPTURED, 2=CONTESTED, 4=CAPTURING, 8=UNCAPTURING"),"CaptureState"),
			OutputPortConfig<bool>("IsCapturable",_HELP(""),"IsCapturable"),
			OutputPortConfig<int>("Species",_HELP(""),"Species"),
			//OutputPortConfig<int>("Team",_HELP(""),"Team"),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			IEntity* pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				CStrategicArea* pArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pEntity->GetId(), 0);
				if (pArea)
				{
					const int actorCount = pArea->GetActorCount();
					const ECaptureState state = pArea->GetCaptureState();
					const bool bIsCapturable = pArea->IsCapturable();
					const int species = (int)pArea->GetSpecies();
					const int team = pArea->GetTeam();

					ActivateOutput(pActInfo, EOP_ActorCount, actorCount);
					ActivateOutput(pActInfo, EOP_CaptureState, (int)state);
					ActivateOutput(pActInfo, EOP_IsCapturable, bIsCapturable);
					ActivateOutput(pActInfo, EOP_Species, species);
					//ActivateOutput(pActInfo, EOP_Team, team);
				}
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AreaGetInfo(pActInfo);
	}
};

class CFlowNode_GameRulesInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_Name,
		EOP_EntityId,
		//EOP_IsMultiplayer,
		EOP_GameState,
		EOP_GameStateName,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GameRulesInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GameRulesInfo()
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Name",_HELP(""),"Name"),
			OutputPortConfig<EntityId>("EntityId",_HELP(""),"EntityId"),
			//OutputPortConfig<bool>("IsMultiplayer",_HELP(""),"IsMultiplayer"),
			OutputPortConfig<int>("GameState",_HELP(""),"GameState"),
			OutputPortConfig<string>("GameStateName",_HELP(""),"GameStateName"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_DEBUG);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			CGameRules* pGR = g_pGame->GetGameRules();
			if (!pGR)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				string		gameStateName = pGR->GetCurrentStateName();
				string		className =		pGR->GetEntity()->GetClass()->GetName();
				EntityId	entityId =		pGR->GetEntityId();
				int			gameState =		pGR->GetCurrentStateId();

				ActivateOutput(pActInfo, EOP_Name, className);
				ActivateOutput(pActInfo, EOP_EntityId, entityId);
				ActivateOutput(pActInfo, EOP_GameState, gameState);
				ActivateOutput(pActInfo, EOP_GameStateName, gameStateName);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GameRulesInfo(pActInfo);
	}
};

class CFlowNode_GetLobbyInfo : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
	};

	enum EOutputPorts
	{
		EOP_State,
		EOP_AnimEntityId,
		EOP_SelectedSpecies,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_GetLobbyInfo(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetLobbyInfo()
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
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("State",_HELP(""),"State"),
			OutputPortConfig<EntityId>("AnimEntityId",_HELP(""),"AnimEntityId"),

			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			//CGameRules* pGR = g_pGame->GetGameRules();
			//if (!pGR)
			//	return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				SConquerLobbyInfo info = g_pControlSystem->GetConquerorSystem()->GetLobbyInfo();

				int state = (int)info.state;
				EntityId entityID = info.modelEntityId;

				ActivateOutput(pActInfo, EOP_State, state);
				ActivateOutput(pActInfo, EOP_AnimEntityId, entityID);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetLobbyInfo(pActInfo);
	}
};

class CFlowNode_ConquestGamemode : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Start,
		EIP_LobbyAnimEntityId,
		//EIP_MaxBotCount

	};

	enum EOutputPorts
	{
		EOP_Started,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_ConquestGamemode(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ConquestGamemode()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:4=4,8=8,16=16,32=32,64=64,128=128";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Start", _HELP("")),
			InputPortConfig<EntityId>("LobbyAnimEntityId", EntityId(0), _HELP("The entity that will change the character model in the lobby"), "LobbyAnimEntityId"),
			//InputPortConfig<int>("MaxBotCount", 4, _HELP(""), "MaxBotCount", ui_config),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Started",_HELP(""),"Started"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("The main node that launches the Conquest game mode");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			const auto entityId = GetPortEntityId(pActInfo, EIP_LobbyAnimEntityId);
			auto pEntity = GET_ENTITY(entityId);

			if (pEntity && IsPortActive(pActInfo, EIP_Start))
			{
				SConquerLobbyInfo info;
				info.isConquest = true;
				info.state = EConquerLobbyState::IN_LOBBY;
				info.modelEntityId = pEntity->GetId();
				info.modelPos = pEntity->GetWorldPos();
				info.modelScale = pEntity->GetScale();

				//g_pControlSystem->GetConquerorSystem()->SetBotsMaxCount(botCount);
				g_pControlSystem->GetConquerorSystem()->InitGamemodeFromFG(info);
				ActivateOutput(pActInfo, EOP_Started, true);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ConquestGamemode(pActInfo);
	}
};

class CFlowNode_MakeAreaCaptured : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Species,

	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_MakeAreaCaptured(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_MakeAreaCaptured()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:4=4,8=8,16=16,32=32,64=64,128=128";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<int>("Species", 0, _HELP(""), "Species"),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP(""),"Done"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			ESpeciesType species = (ESpeciesType)GetPortInt(pActInfo, EIP_Species);

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto pArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pEntity->GetId(), 0, true);
				if (!pArea)
					return;

				pArea->SetCaptured(species);

				if (pArea->GetSpecies() == species)
					ActivateOutput(pActInfo, EOP_Done, 1);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MakeAreaCaptured(pActInfo);
	}
};

class CFlowNode_MakeAreaNeutral : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Species,

	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_MakeAreaNeutral(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_MakeAreaNeutral()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:4=4,8=8,16=16,32=32,64=64,128=128";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP(""),"Done"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Makes a strategic zone neutral");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto pArea = g_pControlSystem->GetConquerorSystem()->GetStrategicArea(pEntity->GetId(), 0, true);
				if (!pArea)
					return;

				pArea->SetNeutral(false);

				if (pArea->GetSpecies() == eST_NEUTRAL)
					ActivateOutput(pActInfo, EOP_Done, 1);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MakeAreaNeutral(pActInfo);
	}
};

class CFlowNode_VehicleCheckClass : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Class,

	};

	enum EOutputPorts
	{
		EOP_True,
		EOP_False,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleCheckClass(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleCheckClass()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:Air=0,Car=1,PLV=2,Tank=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<int>("Class", 0, _HELP(""), "Class", ui_config),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("True",_HELP(""),"True"),
			OutputPortConfig_AnyType("False",_HELP(""),"False"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto pVehicle = TOS_Vehicle::GetVehicle(pEntity);
				if (pVehicle)
				{
					const auto checkClass = GetPortInt(pActInfo, EIP_Class);
					if (checkClass == 0)
					{
						if (TOS_Vehicle::IsAir(pVehicle))
							ActivateOutput(pActInfo, EOP_True, 1);
						else
							ActivateOutput(pActInfo, EOP_False, 1);
					}
					else if (checkClass == 1)
					{
						if (TOS_Vehicle::IsCar(pVehicle))
							ActivateOutput(pActInfo, EOP_True, 1);
						else
							ActivateOutput(pActInfo, EOP_False, 1);
					}
					else if (checkClass == 2)
					{
						if (TOS_Vehicle::IsPLV(pVehicle))
							ActivateOutput(pActInfo, EOP_True, 1);
						else
							ActivateOutput(pActInfo, EOP_False, 1);
					}
					else if (checkClass == 3)
					{
						if (TOS_Vehicle::IsTank(pVehicle))
							ActivateOutput(pActInfo, EOP_True, 1);
						else
							ActivateOutput(pActInfo, EOP_False, 1);
					}
				}
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleCheckClass(pActInfo);
	}
};

class CFlowNode_ActorGetVehicle : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,

	};

	enum EOutputPorts
	{
		EOP_VehicleId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_ActorGetVehicle(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorGetVehicle()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:Air=0,Car=1,PLV=2,Tank=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<EntityId>("VehicleId",_HELP(""),"VehicleId"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (!pActor)
				return;

			if (IsPortActive(pActInfo, EIP_Sync))
			{
				auto pVehicle = TOS_Vehicle::GetVehicle(pActor);
				if (pVehicle)
					ActivateOutput(pActInfo, EOP_VehicleId, pVehicle->GetEntityId());
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorGetVehicle(pActInfo);
	}
};

class CFlowNode_ActorPoints : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
		EIP_Add,
		EIP_Points,
	};

	enum EOutputPorts
	{
		EOP_Points,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_ActorPoints(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_ActorPoints()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:Air=0,Car=1,PLV=2,Tank=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Get", _HELP("")),
			InputPortConfig_AnyType("Add", _HELP("")),
			InputPortConfig<int>("Points", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Points",_HELP(""),"Points"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (!pActor)
				return;

			auto pConqueror = g_pControlSystem->GetConquerorSystem();
			if (!pConqueror)
				return;

			if (IsPortActive(pActInfo, EIP_Get))
			{
				const auto points = pConqueror->GetActorPoints(pEntity);
				ActivateOutput(pActInfo, EOP_Points, points);
			}

			if (IsPortActive(pActInfo, EIP_Add))
			{
				const auto points = GetPortInt(pActInfo, EIP_Points);
				pConqueror->AddPointsToActor(pEntity, points);
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ActorPoints(pActInfo);
	}
};

class CFlowNode_SpeciesReinforcements : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Get,
		EIP_Add,
		EIP_Set,
		EIP_Points,
		EIP_Species,
	};

	enum EOutputPorts
	{
		EOP_Points,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_SpeciesReinforcements(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_SpeciesReinforcements()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:Air=0,Car=1,PLV=2,Tank=3";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Get", _HELP("")),
			InputPortConfig_AnyType("Add", _HELP("")),
			InputPortConfig_AnyType("Set", _HELP("")),
			InputPortConfig<int>("Points", 0, _HELP("")),
			InputPortConfig<int>("Species", -1, _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("Points",_HELP(""),"Points"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			auto pConqueror = g_pControlSystem->GetConquerorSystem();
			if (!pConqueror)
				return;

			ESpeciesType species = (ESpeciesType)GetPortInt(pActInfo, EIP_Species);
			const auto points = GetPortInt(pActInfo, EIP_Points);

			if (IsPortActive(pActInfo, EIP_Add))
				pConqueror->AddSpeciesReinforcements(species, points, 1);

			if (IsPortActive(pActInfo, EIP_Get))
			{
				const auto points = pConqueror->GetSpeciesReinforcements(species);
				ActivateOutput(pActInfo, EOP_Points, points);
			}

			if (IsPortActive(pActInfo, EIP_Set))
				pConqueror->SetSpeciesReinforcements(species, points);
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SpeciesReinforcements(pActInfo);
	}
};


class CFlowNode_MatchStatus : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_MatchEndTrigger,
	};

	enum EOutputPorts
	{
		EOP_OnMatchStart,
		EOP_OnMatchEndNear,
		EOP_OnMatchEndInvalid,
		EOP_OnMatchWon,
		EOP_OnMatchLost,
		EOP_OnBattleStart,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_MatchStatus(SActivationInfo* pActInfo)
	{
		if (pActInfo && pActInfo->pGraph)
		{
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
		}

		m_matchStarted = false;
		m_matchEndNear = false;
		m_matchEnd = false;
		m_battleStart = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_MatchStatus()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		if (ser.GetSerializationTarget() != eST_Network)
		{
			SER_VALUE(m_matchEnd);
			SER_VALUE(m_matchEndNear);
			SER_VALUE(m_matchStarted);
			SER_VALUE(m_battleStart);
		}
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		//const char* ui_config = "enum_int:4=4,8=8,16=16,32=32,64=64,128=128";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<int>("MatchEndTrigger", 25, _HELP("Number of local player's species reinforcements remaining at which to trigger the OnMatchEndNear output")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("OnMatchStarted",_HELP("Triggered when local player enters the game")),
			OutputPortConfig_AnyType("OnMatchEndNear",_HELP("Triggered when game-ending condition is near")),
			OutputPortConfig_AnyType("OnMatchEndInvalid",_HELP("Triggered when game-ending condition is invalidated")),
			OutputPortConfig_AnyType("OnMatchWon", _HELP("Triggered when local player's species wins the game")),
			OutputPortConfig_AnyType("OnMatchLost", _HELP("Triggered when local player's species loses the game")),
			OutputPortConfig_AnyType("OnBattleStart", _HELP("Triggered when state of game equal InBattle")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		//config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Initialize:
		{
			m_actInfo = *pActInfo;

			m_matchEnd = false;
			m_matchEndNear = false;
			m_matchStarted = false;
			m_battleStart = false;
		}
		break;
		case eFE_Activate:
		break;
		case eFE_Update:
		{
			auto pConqueror = g_pControlSystem->GetConquerorSystem();
			if (!pConqueror)
				return;

			auto pDude = g_pGame->GetIGameFramework()->GetClientActor();
			if (!pDude)
				return;

			const auto isConquest = pConqueror->IsGamemode();
			if (!isConquest)
				return;

			const auto species = pConqueror->GetClientSpecies();
			const auto nReinforcements = pConqueror->GetSpeciesReinforcements(species);
			const auto count = GetPortInt(pActInfo, EIP_MatchEndTrigger);

			const auto isGameOver = pConqueror->IsGameOver();
			const auto winnerSpecies = pConqueror->GetWinnerSpecies();
			//const auto nMaxReinforcements = pConquest->GetMaxSpeciesReinforcements(species);
			//const auto nCommanders = pConquest->GetCommandersCount();
			//const auto nLimit = pConquest->GetSpeciesChangeLimit();
			//const auto nChangeCount = pConquest->GetSpeciesChangeCount();

			const auto endNearTrigger = nReinforcements <= count;
			const auto endInvalidTrigger = nReinforcements > count;

			if (m_matchStarted)
			{
				if (endInvalidTrigger && m_matchEndNear)
				{
					ActivateOutput(pActInfo, EOP_OnMatchEndInvalid, 1);
					m_matchEndNear = false;
				}
				else if (endNearTrigger && !m_matchEndNear)
				{
					ActivateOutput(pActInfo, EOP_OnMatchEndNear, 1);
					m_matchEndNear = true;
				}

				if (isGameOver && !m_matchEnd)
				{
					ActivateOutput(pActInfo, (winnerSpecies == species) ? EOP_OnMatchWon : EOP_OnMatchLost, 1);
					m_matchEnd = true;
				}
			}

			auto info = pConqueror->GetLobbyInfo();
			if (info.state == EConquerLobbyState::IN_GAME)
			{
				if (!m_matchStarted)
				{
					ActivateOutput(pActInfo, EOP_OnMatchStart, 1);
					m_matchStarted = true;
				}
			}

			if (pConqueror->GetGameStatus() == eGS_Battle && !m_battleStart)
			{
				ActivateOutput(pActInfo, EOP_OnBattleStart, 1);
				m_battleStart = true;
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

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MatchStatus(pActInfo);
	}

	SActivationInfo m_actInfo;
	bool m_matchStarted;
	bool m_matchEndNear;
	bool m_matchEnd;
	bool m_battleStart;
};

class CFlowNode_BattleLogEvent : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Type,
		EIP_Message,
		EIP_Param1,
		EIP_Param2,
		EIP_Param3,
	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_BattleLogEvent(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_BattleLogEvent()
	{

	}
	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		const char* ui_config = "enum_int:Currency=1,Information=2,Warning=3,System=4";

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<int>("Type", 0, _HELP(""), "Type", ui_config),
			InputPortConfig<string>("Message", string("Hello World!"), _HELP("Use (Message %1 %2 %3). %1 - param 1 and etc")),
			InputPortConfig<string>("Param1", string("") , _HELP("")),
			InputPortConfig<string>("Param2", string("") , _HELP("")),
			InputPortConfig<string>("Param3", string("") , _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Done",_HELP(""),"Done"),

			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Sync))
			{
				int type = GetPortInt(pActInfo, EIP_Type);
				string message = GetPortString(pActInfo, EIP_Message);
				string param1 = GetPortString(pActInfo, EIP_Param1);
				string param2 = GetPortString(pActInfo, EIP_Param2);
				string param3 = GetPortString(pActInfo, EIP_Param3);

				SAFE_HUD_FUNC(BattleLogEvent(type, message.c_str(), param1.c_str(), param2.c_str(), param3.c_str()));
				ActivateOutput(pActInfo, EOP_Done, 1);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_BattleLogEvent(pActInfo);
	}
};

class CFlowNode_RARCreateRequest : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_TargetId,
		EIP_Type,

	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	explicit CFlowNode_RARCreateRequest(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	~CFlowNode_RARCreateRequest() override
	{

	}
	////////////////////////////////////////////////////
	void Serialize(SActivationInfo* pActInfo, TSerialize ser) override
	{
	}

	////////////////////////////////////////////////////
	void GetConfiguration(SFlowNodeConfig& config) override
	{
		constexpr auto ui_config = _UICONFIG("enum_int:NotDefined=0,AlienTaxsee=1");
		
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("Trigger for an input entity [Actor squad leader] to create request with input [Type] and [TargetId]")),
			InputPortConfig<EntityId>("TargetId", _HELP("Entity id of target entity")),
			InputPortConfig<int>("Type", 0, _HELP(""), "Type", ui_config),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Created",_HELP(""),"Created"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
		{
			const auto pConquerorSystem = g_pControlSystem->GetConquerorSystem();
			if (!pConquerorSystem)
				return;
				
			const auto pRarSystem = g_pControlSystem->GetConquerorSystem()->GetRAR();
			if (!pRarSystem)
				return;

			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto targetId = GetPortEntityId(pActInfo, EIP_TargetId);
			const auto requestType = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type));
			
			const auto pRequest = pRarSystem->CreateRequest(pEntity->GetId(), targetId, requestType);
			if (pRequest)
				ActivateOutput(pActInfo, EOP_Done, 1);
				
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowNode_RARCreateRequest(pActInfo);
	}
};

class CFlowNode_RARExecutor : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_Type1,
		EIP_Type2,
		EIP_Type3,
		EIP_Type4,
		EIP_Type5,

	};

	enum EOutputPorts
	{
		EOP_Done,
	};

public:
	////////////////////////////////////////////////////
	explicit CFlowNode_RARExecutor(SActivationInfo* pActInfo)
	{
	}

	////////////////////////////////////////////////////
	~CFlowNode_RARExecutor() override
	{

	}
	////////////////////////////////////////////////////
	void Serialize(SActivationInfo* pActInfo, TSerialize ser) override
	{
	}

	////////////////////////////////////////////////////
	void GetConfiguration(SFlowNodeConfig& config) override
	{
		constexpr auto ui_config = _UICONFIG("enum_int:NotDefined=0,AlienTaxsee=1");
		
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("Trigger for an input entity [Actor] to begin requests executor")),
			InputPortConfig<int>("Type1", 0, _HELP("Define the executor type 1"), "Type1", ui_config),
			InputPortConfig<int>("Type2", 0, _HELP("Define the executor type 2"), "Type2", ui_config),
			InputPortConfig<int>("Type3", 0, _HELP("Define the executor type 3"), "Type3", ui_config),
			InputPortConfig<int>("Type4", 0, _HELP("Define the executor type 4"), "Type4", ui_config),
			InputPortConfig<int>("Type5", 0, _HELP("Define the executor type 5"), "Type5", ui_config),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Created",_HELP(""),"Created"),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Define for input entity [Actor] to begin requests executor");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override
	{
		if (!pActInfo)
			return;

		switch (event)
		{
		case eFE_Activate:
			const auto pConquerorSystem = g_pControlSystem->GetConquerorSystem();
			if (!pConquerorSystem)
				return;
				
			const auto pRarSystem = g_pControlSystem->GetConquerorSystem()->GetRAR();
			if (!pRarSystem)
				return;

			const auto pEntity = pActInfo->pEntity;
			if (!pEntity)
				return;

			const auto requestType1 = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type1));
			const auto requestType2 = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type2));
			const auto requestType3 = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type3));
			const auto requestType4 = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type4));
			const auto requestType5 = static_cast<ERequestType>(GetPortInt(pActInfo, EIP_Type5));
				
			const std::vector<ERequestType> requestType = {requestType1, requestType2,requestType3,requestType4,requestType5};
			const auto species = static_cast<ESpeciesType>(TOS_AI::GetSpecies(pEntity->GetAI(),false));
			
			pRarSystem->AddExecutor(species, pEntity->GetId(), requestType);

			ActivateOutput(pActInfo, EOP_Done, 1);
			
		break;
		}
	}

	////////////////////////////////////////////////////
	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowNode_RARExecutor(pActInfo);
	}
};


//REGISTER_FLOW_NODE("ConquerorSystem:GetSpeciesInfo", CFlowNode_SpeciesGetInfo);
REGISTER_FLOW_NODE("ConquerorSystem:GetAreaInfo", CFlowNode_AreaGetInfo);
//REGISTER_FLOW_NODE("ConquerorSystem:GameRulesInfo", CFlowNode_GameRulesInfo);
REGISTER_FLOW_NODE("ConquerorSystem:GetLobbyInfo", CFlowNode_GetLobbyInfo);
REGISTER_FLOW_NODE("ConquerorSystem:BattleLogEvent", CFlowNode_BattleLogEvent);
REGISTER_FLOW_NODE("ConquerorSystem:ConquestGamemode", CFlowNode_ConquestGamemode);
//REGISTER_FLOW_NODE("ConquerorSystem:SpawnActor", CFlowNode_SpawnActor);
REGISTER_FLOW_NODE("ConquerorSystem:MakeAreaCaptured", CFlowNode_MakeAreaCaptured);
REGISTER_FLOW_NODE("ConquerorSystem:MakeAreaNeutral", CFlowNode_MakeAreaNeutral);
REGISTER_FLOW_NODE("ConquerorSystem:MatchStatus", CFlowNode_MatchStatus);
REGISTER_FLOW_NODE("Vehicle:VehicleCheckClass", CFlowNode_VehicleCheckClass);
REGISTER_FLOW_NODE("Game:ActorGetVehicle", CFlowNode_ActorGetVehicle);
REGISTER_FLOW_NODE("ConquerorSystem:ActorPoints", CFlowNode_ActorPoints);
REGISTER_FLOW_NODE("ConquerorSystem:SpeciesReinforcements", CFlowNode_SpeciesReinforcements);
REGISTER_FLOW_NODE("ConquerorSystem:RARCreateRequest", CFlowNode_RARCreateRequest);
REGISTER_FLOW_NODE("ConquerorSystem:RARCreateExecutor", CFlowNode_RARExecutor);