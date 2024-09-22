/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include <Nodes/G2FlowBaseNode.h>
#include <TheOtherSideMP\Helpers\TOS_Vehicle.h>

class CFlowNode_VehicleGetFreeSeat : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_IsGunner,
	};

	enum EOutputPorts
	{
		EOP_SeatId,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleGetFreeSeat(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleGetFreeSeat(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<bool>("IsGunner", _HELP("Get only free seats which have gun")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Get free seat of vehicle");
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
					auto* pVehicle = pActInfo->pEntity ?
						g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId()) : 0;

					if (pVehicle)
					{
						const auto isGunner = GetPortBool(pActInfo, EIP_IsGunner);
						const int index = isGunner ?
							TOS_Vehicle::RequestGunnerSeatIndex(pVehicle) :
							TOS_Vehicle::RequestFreeSeatIndex(pVehicle);

						ActivateOutput(pActInfo, EOP_SeatId, index);
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleGetFreeSeat(pActInfo);
	}
};

class CFlowNode_VehicleEnterNew : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
		EIP_SeatId
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleEnterNew(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleEnterNew(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("Vehicle", _HELP("")),
			InputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Failed", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Put the actor in vehicle fast");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Activate:
			{
				if (!pActInfo->pEntity)
					return;

				const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
				if (!pActor)
					return;

				if (IsPortActive(pActInfo, EIP_Sync))
				{
					auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));
					if (pVehicle)
					{
						const auto pSeat = pVehicle->GetSeatById(GetPortInt(pActInfo, EIP_SeatId));
						if (pSeat)
						{
							//if (pSeat->Enter(pActor->GetEntityId(), false))
							if (TOS_Vehicle::Enter(pActor, pVehicle, true))
							{
								ActivateOutput(pActInfo, EOP_Success, 1);
							}
							else
							{
								ActivateOutput(pActInfo, EOP_Failed, 1);
							}
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleEnterNew(pActInfo);
	}
};

//class CFlowNode_VehicleStatus : public CFlowBaseNode
//{
//	enum EInputPorts
//	{
//		EIP_Sync,
//	};
//
//	enum EOutputPorts
//	{
//		EOP_Velocity,
//		EOP_Speed,
//		EOP_Flipped,
//		EOP_Altitude,
//		EOP_SubmergedRatio,
//		EOP_PassengerCount,
//		EOP_Power,
//	};
//
//public:
//	////////////////////////////////////////////////////
//	CFlowNode_VehicleStatus(SActivationInfo* pActInfo)
//	{}
//
//	////////////////////////////////////////////////////
//	virtual ~CFlowNode_VehicleStatus(void)
//	{}
//
//	////////////////////////////////////////////////////
//	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
//	{}
//
//	////////////////////////////////////////////////////
//	virtual void GetConfiguration(SFlowNodeConfig& config)
//	{
//		static const SInputPortConfig inputs[] =
//		{
//			InputPortConfig_AnyType("Sync", _HELP("")),
//			{0}
//		};
//
//		static const SOutputPortConfig outputs[] =
//		{
//			OutputPortConfig<Vec3>("Velocity", _HELP("")),
//			OutputPortConfig<float>("Speed", _HELP("")),
//			OutputPortConfig<float>("Flipped", _HELP("")),
//			OutputPortConfig<float>("Altitude", _HELP("")),
//			OutputPortConfig<float>("SubmergedRatio", _HELP("")),
//			OutputPortConfig<int>("PassengersCount", _HELP("")),
//			OutputPortConfig<float>("Power", _HELP("")),
//			{0}
//		};
//
//		config.nFlags |= EFLN_TARGET_ENTITY;
//		config.pInputPorts = inputs;
//		config.pOutputPorts = outputs;
//		config.sDescription = _HELP("");
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	////////////////////////////////////////////////////
//	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
//	{
//		switch (event)
//		{
//			case eFE_Activate:
//			{
//				if (!pActInfo->pEntity)
//					return;
//
//				if (IsPortActive(pActInfo, EIP_Sync))
//				{
//					auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pActInfo->pEntity->GetId());
//					if (pVehicle)
//					{
//						const auto velocity = pVehicle->GetStatus().vel;
//						const auto speed = pVehicle->GetStatus().speed;
//						const auto flipped = pVehicle->GetStatus().flipped;
//						const auto altitude = pVehicle->GetStatus().altitude;
//						const auto submergedRatio = pVehicle->GetStatus().submergedRatio;
//						const auto passengerCount = pVehicle->GetStatus().passengerCount;
//
//						const auto pInfo = g_pControlSystem->GetVehicleMovementInfo(pVehicle->GetEntityId());
//						if (pInfo)
//						{
//							const auto& action = pInfo->GetMovementAction();
//
//							ActivateOutput(pActInfo, EOP_Power, action.power);
//						}
//
//						ActivateOutput(pActInfo, EOP_Velocity, velocity);
//						ActivateOutput(pActInfo, EOP_Speed, speed);
//						ActivateOutput(pActInfo, EOP_Flipped, flipped);
//						ActivateOutput(pActInfo, EOP_Altitude, altitude);
//						ActivateOutput(pActInfo, EOP_SubmergedRatio, submergedRatio);
//						ActivateOutput(pActInfo, EOP_PassengerCount, passengerCount);
//					}
//				}
//			}
//			break;
//		}
//	}
//
//	////////////////////////////////////////////////////
//	virtual void GetMemoryStatistics(ICrySizer* s)
//	{
//		s->Add(*this);
//	}
//
//	////////////////////////////////////////////////////
//	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
//	{
//		return new CFlowNode_VehicleStatus(pActInfo);
//	}
//};

class CFlowNode_VehicleExitNew : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_TransitionEnabled,
		EIP_Force,
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Fail,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleExitNew(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleExitNew(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<bool>("TransitionEnabled", _HELP("")),
			InputPortConfig<bool>("Force", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Fail", _HELP("")),
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
				if (!pActInfo->pEntity)
					return;

				const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
				if (!pActor)
					return;

				if (IsPortActive(pActInfo, EIP_Sync))
				{
					const auto pVehicle = TOS_Vehicle::GetVehicle(pActor);
					if (pVehicle)
					{
						const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
						if (pSeat)
						{
							const auto transEnabled = GetPortBool(pActInfo, EIP_TransitionEnabled);
							const auto force = GetPortBool(pActInfo, EIP_Force);

							if (pSeat->Exit(transEnabled, force))
							{
								ActivateOutput(pActInfo, EOP_Success, 1);
							}
							else
							{
								ActivateOutput(pActInfo, EOP_Fail, 1);
							}
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleExitNew(pActInfo);
	}
};

class CFlowNode_VehicleChangeSeat : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
		EIP_SeatId
	};

	enum EOutputPorts
	{
		EOP_Success,
		EOP_Failed
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_VehicleChangeSeat(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_VehicleChangeSeat(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("Vehicle", _HELP("")),
			InputPortConfig<int>("SeatId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("Success", _HELP("")),
			OutputPortConfig_AnyType("Failed", _HELP("")),
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
				if (!pActInfo->pEntity)
					return;

				const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
				if (!pActor)
					return;

				if (IsPortActive(pActInfo, EIP_Sync))
				{
					auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));
					if (pVehicle)
					{
						const auto pSeat = pVehicle->GetSeatById(GetPortInt(pActInfo, EIP_SeatId));
						if (pSeat && !pSeat->GetPassenger())
						{
							TOS_Vehicle::ChangeSeat(pActor, pSeat->GetSeatId(), false);
							ActivateOutput(pActInfo, EOP_Success, 1);
						}
						else
						{
							ActivateOutput(pActInfo, EOP_Failed, 1);
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_VehicleChangeSeat(pActInfo);
	}
};

class CFlowNode_IsInVehicle : public CFlowBaseNode
{
	enum EInputPorts
	{
		EIP_Sync,
		EIP_VehicleId,
	};

	enum EOutputPorts
	{
		EOP_IsDriver,
		EOP_IsGunner,
		EOP_IsPassenger,
	};

public:
	////////////////////////////////////////////////////
	CFlowNode_IsInVehicle(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_IsInVehicle(void)
	{}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_AnyType("Sync", _HELP("")),
			InputPortConfig<EntityId>("VehicleId", _HELP("")),
			{0}
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig_AnyType("IsGunner", _HELP("")),
			OutputPortConfig_AnyType("IsDriver", _HELP("")),
			OutputPortConfig_AnyType("IsPassenger", _HELP("")),
			{0}
		};

		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Put the actor in vehicle fast");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
			case eFE_Activate:
			{
				if (!pActInfo->pEntity)
					return;

				const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pActInfo->pEntity->GetId());
				if (!pActor)
					return;

				if (IsPortActive(pActInfo, EIP_Sync))
				{
					const auto* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(GetPortEntityId(pActInfo, EIP_VehicleId));
					if (pVehicle)
					{
						TOS_Vehicle::ActorIsDriver(pActor) ? ActivateOutput(pActInfo, EOP_IsDriver, 1) : void();
						TOS_Vehicle::ActorIsGunner(pActor) ? ActivateOutput(pActInfo, EOP_IsGunner, 1) : void();
						TOS_Vehicle::ActorIsPassenger(pActor) ? ActivateOutput(pActInfo, EOP_IsPassenger, 1) : void();
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

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_IsInVehicle(pActInfo);
	}
};

REGISTER_FLOW_NODE("Vehicle:GetFreeSeat", CFlowNode_VehicleGetFreeSeat);
REGISTER_FLOW_NODE("Vehicle:EnterNew", CFlowNode_VehicleEnterNew);
//REGISTER_FLOW_NODE("Vehicle:Status", CFlowNode_VehicleStatus);
REGISTER_FLOW_NODE("Vehicle:ExitNew", CFlowNode_VehicleExitNew);
REGISTER_FLOW_NODE("Vehicle:ChangeSeat", CFlowNode_VehicleChangeSeat);
REGISTER_FLOW_NODE("Vehicle:IsInVehicle", CFlowNode_IsInVehicle);
