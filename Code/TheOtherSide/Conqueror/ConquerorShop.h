#pragma once

class CGameRules;
class CConquerorSystem;
class CActor;

//struct SShopItem
//{
//	SShopItem()
//	{
//		name = 0;
//		price = 0;
//		className = 0;
//		category = 0;
//		uniqueId = 0;
//		ammoClass = 0;
//	}
//
//	const char* name;
//	float price;
//	const char* className;
//	const char* category;
//	int uniqueId;
//	const char* ammoClass;
//};

class CConquerorShop
{
public:
	CConquerorShop();
	CConquerorShop(CConquerorSystem* _pConqSys);

	//void Init();

	//bool AddItem(const SShopItem& item);
	//bool RemoveItem(const SShopItem& item);

	//bool DisableItem(const SShopItem& item);
	//bool EnableItem(const SShopItem& item);

	void Buy(CActor* pActor, const char* classname);
	void BuyAmmo(CActor* pActor, const char* weaponClassName);
	void BuyOther(CActor* pActor, const char* weaponClassName);

private:

	int GetItemPrice(const char* className);
	int GetShopAmmoAmount(const char* ammoClassName);
	bool VehicleCanUseAmmo(IVehicle* pVeh, const char* ammoClassName);

	//CGameRules* m_pGameRules;
	CConquerorSystem* m_pConquerorSystem;
	//std::vector<SShopItem> m_items;
};