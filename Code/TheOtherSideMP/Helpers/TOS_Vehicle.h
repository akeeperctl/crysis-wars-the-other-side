#pragma once
#include "IActorSystem.h"
#include "IVehicleSystem.h"
#include "../Control System/ControlSystem.h"
//#include "../Conqueror/ConquerorSystem.h"

namespace TOS_Vehicle
{
	inline void Exit(const IActor* pActor, bool transitionEnabled, bool force, Vec3 exitPos = Vec3(0,0,0))
	{
		if (!pActor)
			return;

		const auto pVeh = pActor->GetLinkedVehicle();
		if (!pVeh)
			return;

		const auto pSeat = pVeh->GetSeatForPassenger(pActor->GetEntityId());
		if (!pSeat)
			return;

		pSeat->Exit(transitionEnabled, force, exitPos);
	}

	inline int RequestFreeSeatIndex(IVehicle* pVehicle)
	{
		auto freeSeatIndex = -1;

		if (pVehicle)
		{
			HSCRIPTFUNCTION RequestSeatFunc = 0;
			const auto pTable = pVehicle->GetEntity()->GetScriptTable();

			if (pTable && pTable->GetValue("RequestSeat", RequestSeatFunc))
			{
				Script::CallReturn(gEnv->pScriptSystem, RequestSeatFunc, pTable, freeSeatIndex);
				gEnv->pScriptSystem->ReleaseFunc(RequestSeatFunc);
			}
		}

		//if (freeSeatIndex == -1)
			//CryLogAlways("%s[C++][WARNING][RequestFreeSeatIndex return -1]", STR_YELLOW);

		return freeSeatIndex;
	}

	inline int RequestGunnerSeatIndex(IVehicle* pVehicle)
	{
		int gunnerSeatIndex = -1;

		if (pVehicle)
		{
			HSCRIPTFUNCTION RequestSeatFunc = 0;
			const auto pTable = pVehicle->GetEntity()->GetScriptTable();

			if (pTable && pTable->GetValue("RequestGunnerSeat", RequestSeatFunc))
			{
				Script::CallReturn(gEnv->pScriptSystem, RequestSeatFunc, pTable, gunnerSeatIndex);
				gEnv->pScriptSystem->ReleaseFunc(RequestSeatFunc);

				//CryLogAlways("GUNNER SEAT %i", gunnerSeatIndex);
			}
		}

		if (gunnerSeatIndex == -1)
			CryLogAlways("%s[C++][WARNING][RequestGunnerSeatIndex return -1]", STR_YELLOW);

		return gunnerSeatIndex;
	}

	inline bool ActorIsPassenger(IActor* pActor)
	{
		if (pActor)
		{
			const auto pVehicle = pActor->GetLinkedVehicle();
			if (pVehicle)
			{
				const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
				if (pSeat)
					return !pSeat->IsDriver() && !pSeat->IsGunner();
			}
		}

		return false;
	}

	inline bool ActorIsDriver(IActor* pActor)
	{
		if (pActor)
		{
			const auto pVehicle = pActor->GetLinkedVehicle();
			if (pVehicle)
			{
				const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
				if (pSeat)
					return pSeat->IsDriver();
			}
		}

		return false;
	}

	inline bool ActorIsGunner(IActor* pActor)
	{
		if (pActor)
		{
			const auto pVehicle = pActor->GetLinkedVehicle();
			if (pVehicle)
			{
				const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
				if (pSeat)
					return pSeat->IsGunner();
			}
		}

		return false;
	}

	inline bool ActorInVehicle(IActor* pActor)
	{
		if (pActor)
		{
			const auto pVehicle = pActor->GetLinkedVehicle();
			if (pVehicle)
			{
				const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
				if (pSeat)
					return true;
			}
		}

		return false;
	}

	inline IVehicle* GetVehicle(IActor* pActor)
	{
		if (pActor)
		{
			const auto pVehicle = pActor->GetLinkedVehicle();
			if (pVehicle)
			{
				const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
				if (pSeat)
					return pVehicle;
			}
		}

		return nullptr;
	}

	inline IVehicle* GetVehicle(IEntity* pEntity)
	{
		if (pEntity)
			return g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pEntity->GetId());

		return nullptr;
	}

	inline void ChangeSeat(IActor* pActor, int seatIndex, bool isAnimationEnabled)
	{
		if (pActor && pActor->GetLinkedVehicle())
		{
			HSCRIPTFUNCTION ActorChangeSeat = 0;
			const auto pTable = pActor->GetEntity()->GetScriptTable();

			if (pTable && pTable->GetValue("ActorChangeSeat", ActorChangeSeat))
				Script::CallMethod(pTable, ActorChangeSeat, seatIndex, isAnimationEnabled);
		}
	}

	inline void Destroy(IVehicle* pVehicle)
	{
		if (pVehicle)
		{
			const auto id = pVehicle->GetEntityId();
			const auto& pos = pVehicle->GetEntity()->GetWorldPos();

			pVehicle->OnHit(id, id, 18000, pos, 1, "normal", false);
		}
	}

	inline IAIObject* GetAI(IVehicle* pVehicle)
	{
		if (pVehicle)
			return pVehicle->GetEntity()->GetAI();

		return nullptr;
	}

	inline bool IsHavePassengers(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return false;

		auto passengerCount = 0;
		const auto seats = pVehicle->GetSeatCount();

		for (auto i = 0; i < seats; i++)
		{
			const auto pSeat = pVehicle->GetSeatById(i);
			if (!pSeat)
				continue;

			if (pSeat->IsDriver())
				continue;

			if (pSeat->IsGunner())
				continue;

			passengerCount++;
		}

		return passengerCount != 0;
	}

	inline bool DriverSelectWeapon(IVehicle* pVehicle, int index)
	{
		if (!pVehicle)
			return false;

		HSCRIPTFUNCTION DriverSelectWeaponFunc = 0;
		const auto pTable = pVehicle->GetEntity()->GetScriptTable();

		if (pTable && pTable->GetValue("DriverSelectWeaponByIndex", DriverSelectWeaponFunc))
		{
			return Script::CallMethod(pTable, DriverSelectWeaponFunc, index);
		}

		return false;
	}

	inline int GetSeatWeaponCount(IVehicle* pVehicle, const TVehicleSeatId seatId)
	{
		if (!pVehicle)
			return 0;

		int count = 0;

		HSCRIPTFUNCTION GetSeatWeaponCountFunc = 0;
		const auto pTable = pVehicle->GetEntity()->GetScriptTable();

		if (pTable && pTable->GetValue("GetSeatWeaponCount", GetSeatWeaponCountFunc))
		{
			Script::CallReturn(gEnv->pScriptSystem, GetSeatWeaponCountFunc, pTable, seatId, count);
			gEnv->pScriptSystem->ReleaseFunc(GetSeatWeaponCountFunc);
		}

		return count;
	}

	inline int GetSeatWeaponCount(IActor* pActor)
	{
		if (!pActor)
			return 0;

		const auto pVehicle = pActor->GetLinkedVehicle();
		if (!pVehicle)
			return 0;

		const auto pSeat = pVehicle->GetSeatForPassenger(pActor->GetEntityId());
		if (!pSeat)
			return 0;

		const int count = GetSeatWeaponCount(pVehicle, pSeat->GetSeatId());

		return count;
	}

	inline bool IsAir(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return false;

		return pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air;
	}

	inline bool IsLand(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return false;

		return pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Land;
	}
	
	//inline bool IsPLV(IVehicle* pVehicle)
	//{
	//	const auto pConqueror = g_pControlSystem->GetConquerorSystem();
	//	if (!pConqueror)
	//		return false;

	//	if (!pVehicle)
	//		return false;

	//	const auto classType = pConqueror->GetVehicleClassType(pVehicle);

	//	return classType == eVCT_PLV;
	//}

	//inline bool IsCar(const IVehicle* pVehicle)
	//{
	//	const auto pConqueror = g_pControlSystem->GetConquerorSystem();
	//	if (!pConqueror)
	//		return false;

	//	if (!pVehicle)
	//		return false;

	//	const auto classType = pConqueror->GetVehicleClassType(pVehicle);

	//	return classType == eVCT_Cars;
	//}

	//inline bool IsTank(IVehicle* pVehicle)
	//{
	//	const auto pConqueror = g_pControlSystem->GetConquerorSystem();
	//	if (!pConqueror)
	//		return false;

	//	if (!pVehicle)
	//		return false;

	//	const auto classType = pConqueror->GetVehicleClassType(pVehicle);

	//	return classType == eVCT_Tanks;
	//}

	inline bool IsSea(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return false;

		return pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Sea;
	}

	inline bool IsHaveFreeSeats(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return false;

		const int seatsCount = pVehicle->GetSeatCount();
		const int passengerCount = pVehicle->GetStatus().passengerCount;

		return passengerCount < seatsCount;
	}

	inline float GetEnterRadius(IVehicle* pVehicle)
	{
		if (!pVehicle)
			return 0.0f;

		AABB bounds;
		pVehicle->GetEntity()->GetWorldBounds(bounds);

		// rad < 4 --> rad = 4 or rad > 10 --> rad = 10

		float maximum = 10.0f;
		if (TOS_Vehicle::IsAir(pVehicle))
		{
			if (!pVehicle->GetMovement()->IsEngineDisabled())
				maximum += 5.0f;
		}

		return min(max(4.0f, bounds.GetRadius()), maximum);
	}
}