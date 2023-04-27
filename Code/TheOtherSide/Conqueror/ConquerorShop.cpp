#include "StdAfx.h"

#include "HUD/HUD.h"
#include "HUD/HUDPowerStruggle.h"

#include "ConquerorShop.h"
#include "ConquerorSystem.h"
#include "../Control/ControlSystem.h"
#include "../Helpers/TOS_Vehicle.h"
#include "GameRules.h"
#include "Actor.h"

#define RULES_TABLE g_pGame->GetGameRules()->GetEntity()->GetScriptTable()

CConquerorShop::CConquerorShop()
{
	m_pConquerorSystem = g_pControlSystem->GetConquerorSystem();
}

CConquerorShop::CConquerorShop(CConquerorSystem* _pConqSys)
{
	m_pConquerorSystem = _pConqSys;
}

void CConquerorShop::Buy(CActor* pActor, const char* classname)
{
	if (!pActor)
		return;

	auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	auto pPSHUD = pHUD->GetPowerStruggleHUD();
	if (!pPSHUD)
		return;

	CHUDPowerStruggle::SItem itemdef;
	if (pPSHUD->GetItemFromName(classname, itemdef))
	{
		if (itemdef.bAmmoType)
			BuyAmmo(pActor, classname);
		else
			BuyOther(pActor, classname);		
	}
}

//void CConquerorShop::Init()
//{
//
//
//}

void CConquerorShop::BuyAmmo(CActor* pActor, const char* weaponClassName)
{
	auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	if (!pActor)
		return;

	if (pActor->GetHealth() <= 0)
		return;

	auto pVehicle = TOS_Vehicle::GetVehicle(pActor);

	auto pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	const auto curItemId = pInventory->GetCurrentItem();
	
	const auto pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(curItemId);
	if (!pItem)
		return;

	if (pItem->GetEntity()->GetClass() == CItem::sFistsClass)
		return;

	const auto pWeapon = pItem->GetIWeapon();
	if (!pWeapon)
		return;

	const auto curFireMode = pWeapon->GetCurrentFireMode();
	
	const auto pFireMode = pWeapon->GetFireMode(curFireMode);
	if (!pFireMode)
		return;

	const auto pAmmoType = pFireMode->GetAmmoType();
	const auto ammoCount = pFireMode->GetAmmoCount();
	const auto ammoCapacity = pInventory->GetAmmoCapacity(pAmmoType);
	const auto ammoShopAmount = GetShopAmmoAmount(pAmmoType->GetName());

	auto price = GetItemPrice(weaponClassName);
	if ((ammoCount < ammoCapacity) || (ammoCapacity == 0))
	{
		auto need = ammoShopAmount;
		if (ammoCapacity > 0)
			need = min(ammoCapacity - ammoCount, ammoShopAmount);

		pInventory->SetAmmoCount(pAmmoType, ammoCount + need);

		if (price > 0)
		{
			if (need < ammoShopAmount)
				price = ceil((need * price) / ammoShopAmount);

			m_pConquerorSystem->AddPointsToActor(pActor->GetEntity(), -price);
		}
	}
}

void CConquerorShop::BuyOther(CActor* pActor, const char* weaponClassName)
{
	auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	auto pPSHUD = pHUD->GetPowerStruggleHUD();
	if (!pPSHUD)
		return;

	if (!pActor)
		return;

	if (pActor->GetHealth() <= 0)
		return;

	auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	const auto pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	//const auto ok = pActor->CheckInventoryRestrictions(weaponClassName);
	//if (!ok)
	//{
	//	//self.game:SendTextMessage(TextMessageError, "@mp_CannotCarryMore", TextMessageToClient, playerId);
	//	g_pGame->GetGameRules()->SendTextMessage(eTextMessageError, "@mp_CannotCarryMore", eRMI_ToClientChannel, pActor->GetChannelId());
	//	return;
	//}
	
	CHUDPowerStruggle::SItem itemdef;
	if (pPSHUD->GetItemFromName(weaponClassName, itemdef))
	{
		auto price = GetItemPrice(weaponClassName);
		if (price > 0)
			m_pConquerorSystem->AddPointsToActor(pActor->GetEntity(), -price);

		CryLogAlways("[C++][%s buy %s]", pActor->GetEntity()->GetName(), itemdef.strClass);
		g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemdef.strClass, true, true, true);
	}
}

int CConquerorShop::GetItemPrice(const char* className)
{
	int price = 0;

	auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return price;

	HSCRIPTFUNCTION GetPriceFunc;
	RULES_TABLE->GetValue("GetPrice", GetPriceFunc);

	Script::CallReturn(gEnv->pScriptSystem, GetPriceFunc, RULES_TABLE, className, price);

	return price;
}

int CConquerorShop::GetShopAmmoAmount(const char* ammoClassName)
{
	int amount = 0;

	auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return amount;

	HSCRIPTFUNCTION GetAmountFunc;
	RULES_TABLE->GetValue("GetAmmoAmount", GetAmountFunc);

	Script::CallReturn(gEnv->pScriptSystem, GetAmountFunc, RULES_TABLE, ammoClassName, amount);

	return amount;
}

bool CConquerorShop::VehicleCanUseAmmo(IVehicle* pVeh, const char* ammoClassName)
{
	if (!pVeh)
		return false;

	bool can = 0;

	auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return can;

	HSCRIPTFUNCTION VehicleCanUseAmmo2Func;
	RULES_TABLE->GetValue("VehicleCanUseAmmo2", VehicleCanUseAmmo2Func);

	Script::CallReturn(gEnv->pScriptSystem, VehicleCanUseAmmo2Func, RULES_TABLE, ammoClassName, can);

	return can;
}
