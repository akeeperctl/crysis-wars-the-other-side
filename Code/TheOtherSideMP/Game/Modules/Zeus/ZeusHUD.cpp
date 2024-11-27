/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "IFlashPlayer.h"

#include "ZeusModule.h"
#include "HUD/HUD.h"
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include <TheOtherSideMP\Helpers\TOS_Screen.h>
#include <TheOtherSideMP\Helpers\TOS_STL.h>

void CTOSZeusModule::HUDInit()
{
	m_animZeusUnitIcons.Load("Libs/UI/HUD_Zeus_Unit_Icon.swf", eFD_Center, eFAF_Visible);
	m_animZeusUnitIcons.GetFlashPlayer()->SetFSCommandHandler(this);

	m_animZeusOrderIcons.Load("Libs/UI/HUD_Zeus_Order_Icon.swf", eFD_Center, eFAF_Visible);
	m_animZeusOrderIcons.GetFlashPlayer()->SetFSCommandHandler(this);

	m_animZeusMenu.Load("Libs/UI/HUD_Zeus_Menu.swf", eFD_Right, eFAF_Visible);
	m_animZeusMenu.GetFlashPlayer()->SetFSCommandHandler(this);
}

void CTOSZeusModule::HandleFSCommand(const char* pCommand, const char* pArgs)
{
	auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	string sCommand = pCommand;
	string sArgs = pArgs;

	//Zeus on screen icon
	if (sCommand == "MousePressOnUnit")
	{
		auto pEntity = TOS_GET_ENTITY(atoi(sArgs));
		if (pEntity)
		{
			OnEntityIconPressed(pEntity);
		}
	}
	else if (sCommand == "MouseOverUnit")
	{
		m_mouseOveredEntityId = atoi(sArgs);
	}
	else if (sCommand == "MouseOutUnit")
	{
		m_mouseOveredEntityId = 0;
	}
	else if (sCommand == "CryLogAlways")
	{
		CryLogAlways("[Flash] %s", sArgs);
	}

	//Zeus menu
	else if (sCommand == "UpdateBuyList")
	{
		m_menuCurrentPage = atoi(sArgs);
		HUDUpdateZeusMenuItemList(sArgs);
	}
	else if (sCommand == "PDATabChanged")
	{
		pHUD->PlaySound(ESound_TabChanged);
	}
	else if (sCommand == "PDA")
	{
		if (sArgs == "TabChanged")
		{
			pHUD->PlaySound(ESound_TabChanged);
		}
		else if (sArgs == "SelfClose")
		{
			HUDShowZeusMenu(false);
		}
		//else if (sArgs == "OverMenu")
		//{
		//	pHUD->SetModalHUD(&m_animZeusMenu);
		//	SetZeusFlag(eZF_CanUseMouse, false);
		//}
		//else if (sArgs == "OutMenu")
		//{
		//	pHUD->SetModalHUD(nullptr);
		//	SetZeusFlag(eZF_CanUseMouse, true);
		//}
	}
	else if (sCommand == "Spawn")
	{
		STOSEntitySpawnParams params;
		params.vanilla.bStaticEntityId = false; // true - вылетает в редакторе и медленно работает O(n), false O(1)
		params.vanilla.bIgnoreLock = false; // spawn lock игнор

		const string* const psClassName = &sArgs;
		IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(psClassName->c_str());

		const string name = string("Zeus_") + psClassName->c_str();
		params.vanilla.sName = name;
		params.vanilla.pClass = pClass;

		const auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(psClassName->c_str());
		if (pArchetype)
			params.vanilla.pArchetype = pArchetype;

		if (!pClass && !pArchetype)
		{
			CryLogError("[Zeus] not defined entity class '%s'", psClassName->c_str());
			return;
		}

		// Получаем координаты вперед от камеры
		const Vec3 pos = gEnv->pSystem->GetViewCamera().GetPosition();
		const Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
		const float maxDistance = 300.0f;

		// Только вращение по Z
		Vec3 spawnPos = pos + (dir * maxDistance);
		//Quat spawnRot = Quat::CreateRotationVDir(dir);
		//spawnRot.v.x = 0;
		//spawnRot.v.y = 0;
		// FIXME: ротация транспорта после Hide(false) почему то становится в 000, хотя бокс изначально повернут правильно

		const int rayFlags = rwi_stop_at_pierceable | rwi_colltype_any;
		const unsigned entityFlags = ent_all;
		
		auto pPhys = m_zeus->GetEntity()->GetPhysics();
		ray_hit rayHit;

		const int hitted = gEnv->pPhysicalWorld->RayWorldIntersection(pos, dir * maxDistance, entityFlags, rayFlags, &rayHit, 1, pPhys);
		if (hitted)
		{
			spawnPos = rayHit.pt;
		}

		//const auto vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
		//const auto vDir = dir;
		//auto pPhys = m_zeus->GetEntity()->GetPhysics();

		//ray_hit hit;
		//const auto queryFlags = ent_all;
		//const unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
		//const float fRange = gEnv->p3DEngine->GetMaxViewDistance();

		//if (gEnv->pPhysicalWorld && gEnv->pPhysicalWorld->RayWorldIntersection(vCamPos, vDir * fRange, queryFlags, flags, &hit, 1, pPhys))
		//{
		//	if (gEnv->p3DEngine->RefineRayHit(&hit, vDir * fRange))
		//	{
		//		spawnPos = hit.pt;
		//	}
		//}

		//m_pPersistantDebug->Begin("ZeusMenuSpawn", true);
		//m_pPersistantDebug->AddLine(pos, spawnPos, ColorF(1.0f,1.0f,1.0f,1.0f), 15.0f);
		//m_pPersistantDebug->AddSphere(spawnPos, 1.0f, ColorF(0.3f,0.3f,0.3f,1.0f), 15.0f);

		params.vanilla.vPosition = spawnPos;
		// params.vanilla.qRotation = spawnRot;

		IEntity* pSpawned = TOS_Entity::Spawn(params, false);
		if (!pSpawned)
		{
			CryLogError("[Zeus] entity with class '%s' spawn failed!", psClassName->c_str());
			return;
		}

		pSpawned->Hide(true);

		const EntityId spawnedId = pSpawned->GetId();

		char buffer[64];
		sprintf(buffer, "%d", spawnedId);
		pSpawned->SetName(name + "_" + buffer);

		m_dragging = true;
		m_menuSpawnHandling = true;

		SelectEntity(spawnedId);
		m_curClickedEntityId = spawnedId;
		m_mouseOveredEntityId = spawnedId;

		m_selectStartEntitiesPositions[spawnedId] = spawnPos;
		m_storedEntitiesPositions[spawnedId] = spawnPos;
		m_clickedSelectStartPos = spawnPos;
	}
}

void CTOSZeusModule::HUDInGamePostUpdate(float frametime)
{
	HUDFlashUpdateUnitIcons();
	HUDFlashUpdateOrderIcons();
}

void CTOSZeusModule::HUDCreateOrderIcon(const Vec3& worldPos)
{
	const auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	if (pHUD->GetModalHUD() == &pHUD->m_animScoreBoard)
		return;

	auto pAnim = &m_animZeusOrderIcons;

	//const int	iMinDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_near_distance", 10);
	//const int	iMaxDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_far_distance", 500);
	//const float	fMinSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_near_size", 1.4f);
	//const float	fMaxSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_far_size", 0.7f);

	//const auto pPlayerActor = static_cast<CTOSActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	//const float fDist = (worldPos - pPlayerActor->GetEntity()->GetWorldPos()).len();
	float fSize = 1.0f;

	//if (fDist <= iMinDist)
	//{
	//	fSize = fMinSize;
	//}
	//else if (fDist >= iMaxDist)
	//{
	//	fSize = fMaxSize;
	//}
	//else if (iMaxDist > iMinDist)
	//{
	//	const float fA = ((float)iMaxDist - fDist);
	//	const float fB = (float)(iMaxDist - iMinDist);
	//	const float fC = (fMinSize - fMaxSize);
	//	fSize = ((fA / fB) * fC) + fMaxSize;
	//}

	Vec3 orderScreenPos(ZERO);
	gEnv->pRenderer->ProjectToScreen(worldPos.x, worldPos.y, worldPos.z, &orderScreenPos.x, &orderScreenPos.y, &orderScreenPos.z);

	float fScaleX = 0.0f;
	float fScaleY = 0.0f;
	float fHalfUselessSize = 0.0f;
	pHUD->GetProjectionScale(pAnim, &fScaleX, &fScaleY, &fHalfUselessSize);

	const float offsetX = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetX", 0.0f);
	const float offsetY = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetY", 0.0f);

	const float transX = orderScreenPos.x * fScaleX + fHalfUselessSize + offsetX;
	const float transY = orderScreenPos.y * fScaleY + offsetY;

	m_orderIcons.push_back(SOrderIcon(transX, transY, fSize));

}

void CTOSZeusModule::HUDCreateOrderLine(EntityId executor, const Vec3& orderWorldPos)
{
	auto pExecutor = gEnv->pEntitySystem->GetEntity(executor);
	if (!pExecutor)
		return;

	AABB bounds; 
	pExecutor->GetWorldBounds(bounds);

	const Vec3 startPos = bounds.GetCenter();
	const Vec3 endPos = orderWorldPos;
	const ColorB color = ColorB(167, 167, 167);

	gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(startPos, color, endPos, color);
}

void CTOSZeusModule::HUDUnloadSimpleAssets(bool unload)
{
	if (!unload)
	{
		m_animZeusUnitIcons.Reload();
		m_animZeusUnitIcons.GetFlashPlayer()->SetFSCommandHandler(this);

		m_animZeusOrderIcons.Reload();
		m_animZeusOrderIcons.GetFlashPlayer()->SetFSCommandHandler(this);

		m_animZeusMenu.Reload();
		m_animZeusMenu.GetFlashPlayer()->SetFSCommandHandler(this);
	}
	else
	{
		m_animZeusUnitIcons.Unload();
		m_animZeusOrderIcons.Unload();
		m_animZeusMenu.Unload();
	}
}

void CTOSZeusModule::HUDUpdateZeusMenuItemList(const char* szPageIdx)
{
	if (!g_pGame->GetIGameFramework()->GetClientActor())
		return;

	IFlashPlayer* pFlashPlayer = m_animZeusMenu.GetFlashPlayer();

	const int iPage = atoi(szPageIdx);
	string strPageName;

	if (m_menuItems.empty())
		MenuLoadItems();

	auto pTabBegin = m_menuItems.cbegin();
	auto pTabEnd = m_menuItems.cend();
	for (; pTabBegin != pTabEnd; pTabBegin++)
	{
		if (pTabBegin->first.iIndex == iPage)
		{	
			strPageName = pTabBegin->first.strName;

			const std::vector<SItem>* pItemsList = &pTabBegin->second;
			std::vector<string> itemArray;

			char tempBuf[256];

			for (auto iter = pItemsList->cbegin(); iter != pItemsList->cend(); ++iter)
			{
				SItem item = (*iter);
				const char* sReason = "ready";

				itemArray.push_back(item.strName.c_str());
				itemArray.push_back(item.strDesc.c_str());

				_snprintf(tempBuf, sizeof(tempBuf), "%d", item.iPrice);
				tempBuf[sizeof(tempBuf) - 1] = '\0';
				itemArray.push_back(tempBuf);

				itemArray.push_back(sReason);

				_snprintf(tempBuf, sizeof(tempBuf), "%d", item.iInventoryID);
				tempBuf[sizeof(tempBuf) - 1] = '\0';
				itemArray.push_back(tempBuf);
				if (!item.strCategory.empty())
				{
					itemArray.push_back(item.strCategory);
				}
				else
				{
					itemArray.push_back("");
				}

				_snprintf(tempBuf, sizeof(tempBuf), "%d", item.iCount);
				tempBuf[sizeof(tempBuf) - 1] = '\0';
				itemArray.push_back(tempBuf);

				_snprintf(tempBuf, sizeof(tempBuf), "%d", item.iMaxCount);
				tempBuf[sizeof(tempBuf) - 1] = '\0';
				itemArray.push_back(tempBuf);

				itemArray.push_back(item.strClass.c_str());
			}

			int size = itemArray.size();
			if (size)
			{
				std::vector<const char*> pushArray;
				pushArray.reserve(size);
				for (int i(0); i < size; ++i)
				{
					pushArray.push_back(itemArray[i].c_str());
				}

				pFlashPlayer->SetVariableArray(FVAT_ConstStrPtr, "m_allValues", 0, &pushArray[0], size);
			}
			break;
		}
	}


	pFlashPlayer->Invoke0("updateList");

	pTabBegin = m_menuItems.cbegin();
	pTabEnd = m_menuItems.cend();
	for (; pTabBegin != pTabEnd; pTabBegin++)
	{
		SFlashVarValue setTabNameArgs[2] = { pTabBegin->first.iIndex, pTabBegin->first.strName };
		pFlashPlayer->Invoke("setTabName", setTabNameArgs, 2);

		SFlashVarValue activateTabArgs[2] = { pTabBegin->first.iIndex, true };
		pFlashPlayer->Invoke("activateTab", activateTabArgs, 2);
	}
}

bool CTOSZeusModule::HUDShowZeusMenu(bool show)
{
	if (show && gEnv->pGame->GetIGameFramework()->GetIViewSystem()->IsPlayingCutScene())
		return false;

	m_menuShow = show;
	m_animZeusMenu.Invoke(m_menuShow ? "showPDA" : "hidePDA");

	auto pHUD = g_pGame->GetHUD();
	if (show && pHUD)
		pHUD->ShowPDA(false, false);
}

bool CTOSZeusModule::HUDIsShowZeusMenu() const
{
	return m_menuShow;
}

bool CTOSZeusModule::MenuLoadItems()
{
	XmlNodeRef rootNode = GetISystem()->LoadXmlFile(m_menuFilename.c_str());
	if (!rootNode)
	{
		CryLogError("[Zeus] Failed to open menu XML file '%s'.", m_menuFilename.c_str());
		return false;
	}

	CryLog("Parsing Zeus menu XML file '%s'", m_menuFilename);
	CryLog("---------------------------");

	m_menuItems.clear();

	const float scale = g_pGameCVars->g_pp_scale_price;

	const char* szRootName = rootNode->getTag();
	if (!stricmp(szRootName, "Root"))
	{
		const int32 tabsCount = rootNode->getChildCount();
		for (int32 tabIdx = 0; tabIdx < tabsCount; ++tabIdx)
		{
			// <tab name="Characters">
			const XmlNodeRef tabNode = rootNode->getChild(tabIdx);
			XmlString tabName;

			if (!tabNode->getAttr("name", tabName) || tabName.empty())
			{
				CryLogError("[Zeus] Missing or empty 'name' attribute for '%s' tag in file '%s' at line %d...", tabNode->getTag(), m_menuFilename.c_str(), tabNode->getLine());
				return false;
			}

			// нумерация tab начинается с 1
			STab tab;
			tab.strName = tabName;
			tab.iIndex = tabIdx + 1;

			// item - в данном случае элемент меню, а не оружие.
			const int itemsCount = tabNode->getChildCount();
			for (int32 itemIdx = 0; itemIdx < itemsCount; ++itemIdx)
			{
				// <Item category="@zeus_catUSA" name="SMG Soldier" class="Grunt" price="1000"/>
				const XmlNodeRef itemNode = tabNode->getChild(itemIdx);
				XmlString itemCategory;
				XmlString itemName;
				XmlString itemClass;
				XmlString itemPrice;
				XmlString itemAmount; // опционально

				if (!itemNode->getAttr("category", itemCategory) || itemCategory.empty())
				{
					CryLogError("[Zeus] Missing or empty 'category' attribute for '%s' tag in file '%s' at line %d...", itemNode->getTag(), m_menuFilename.c_str(), itemNode->getLine());
					return false;
				}

				if (!itemNode->getAttr("name", itemName) || itemName.empty())
				{
					CryLogError("[Zeus] Missing or empty 'name' attribute for '%s' tag in file '%s' at line %d...", itemNode->getTag(), m_menuFilename.c_str(), itemNode->getLine());
					return false;
				}

				if (!itemNode->getAttr("class", itemClass) || itemClass.empty())
				{
					CryLogError("[Zeus] Missing or empty 'class' attribute for '%s' tag in file '%s' at line %d...", itemNode->getTag(), m_menuFilename.c_str(), itemNode->getLine());
					return false;
				}

				if (!itemNode->getAttr("price", itemPrice) || itemPrice.empty())
				{
					CryLogError("[Zeus] Missing or empty 'price' attribute for '%s' tag in file '%s' at line %d...", itemNode->getTag(), m_menuFilename.c_str(), itemNode->getLine());
					return false;
				}

				itemNode->getAttr("amount", itemAmount);

				int iItemPrice = 0;
				int iItemAmount = 1;

				if (!itemPrice.empty())
					iItemPrice = atoi(itemPrice);
				if (!itemAmount.empty())
					iItemAmount = atoi(itemAmount);

				SItem item;
				// Не используется здесь
				item.level = 0.0f;
				item.isUnique = 0;
				item.iMaxCount = 1;
				item.uniqueLoadoutGroup = 0;
				item.uniqueLoadoutCount = 0;
				item.iInventoryID = 0;
				item.bVehicleType = false;
				item.bAmmoType = false;
				item.loadout = false;
				item.special = false;
				item.isWeapon = false;

				// Используется здесь
				item.iCount = iItemAmount;
				item.iPrice = int(iItemPrice * scale);
				item.strCategory = itemCategory;
				item.strClass = itemClass;
				item.strName = itemName;

				m_menuItems[tab].push_back(item);
				CryLog("[Zeus] Item '%s' reading successfully!", item.strName);
			}
		}
	}
	else
	{
		CryLogError("[Zeus] Unexpected tag '%s' in file '%s' at line %d...", szRootName, m_menuFilename.c_str(), rootNode->getLine());
		return false;
	}

	return true;
}

void CTOSZeusModule::HUDCreateUnitIcon(EntityId unitEntityId, int friendly, int iconType, const Vec3 localOffset)
{
	const auto pAnim = &m_animZeusUnitIcons;

	const auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	if (pHUD->GetModalHUD() == &pHUD->m_animScoreBoard)
		return;

	const IEntity* pObjectiveEntity = GetISystem()->GetIEntitySystem()->GetEntity(unitEntityId);
	if (!pObjectiveEntity)
		return;

	Vec3 vWorldPos = pObjectiveEntity->GetWorldPos();
	// При перетаскивании бокса, иконка должна отображаться по координатам бокса
	if (m_dragging)
	{
		auto it = m_boxes.find(pObjectiveEntity->GetId());
		if (it != m_boxes.end())
			vWorldPos = it->second->wPos;
	}

	vWorldPos += localOffset;

	Vec3 vEntityScreenSpace;
	gEnv->pRenderer->ProjectToScreen(vWorldPos.x, vWorldPos.y, vWorldPos.z, &vEntityScreenSpace.x, &vEntityScreenSpace.y, &vEntityScreenSpace.z);

	float fScaleX = 0.0f;
	float fScaleY = 0.0f;
	float fHalfUselessSize = 0.0f;
	pHUD->GetProjectionScale(pAnim, &fScaleX, &fScaleY, &fHalfUselessSize);

	const int	iMinDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_near_distance", 10);
	const int	iMaxDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_far_distance", 500);
	const float	fMinSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_near_size", 1.4f);
	const float	fMaxSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_far_size", 0.7f);

	const auto pPlayerActor = static_cast<CTOSActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	const float fDist = (vWorldPos - pPlayerActor->GetEntity()->GetWorldPos()).len();
	float fSize = 1.0f;

	if (fDist <= iMinDist)
	{
		fSize = fMinSize;
	}
	else if (fDist >= iMaxDist)
	{
		fSize = fMaxSize;
	}
	else if (iMaxDist > iMinDist)
	{
		const float fA = ((float)iMaxDist - fDist);
		const float fB = (float)(iMaxDist - iMinDist);
		const float fC = (fMinSize - fMaxSize);
		fSize = ((fA / fB) * fC) + fMaxSize;
	}

	const float offsetX = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetX", 0.0f);
	const float offsetY = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetY", 0.0f);

	const float transX = vEntityScreenSpace.x * fScaleX + fHalfUselessSize + offsetX;
	const float transY = vEntityScreenSpace.y * fScaleY + offsetY;

	const float rotation = 0.0f;
	const int healthValue = 100;

	const auto iter = stl::binary_find(m_selectedEntities.cbegin(), m_selectedEntities.cend(), unitEntityId);
	bool selected = iter != m_selectedEntities.cend();
	if (!selected && m_dragTargetId > 0)
		selected = unitEntityId == m_dragTargetId;

	SUnitIcon icon(unitEntityId, transX, transY, (int)iconType, friendly, fDist, fSize * fSize, -rotation, healthValue, (int)selected);

	static const wchar_t* localizedText = L"";
	localizedText = pHUD->LocalizeWithParams(pObjectiveEntity->GetName(), true);
	icon.text.append(localizedText);

	m_unitIcons.push_back(icon);
}

void CTOSZeusModule::HUDFlashUpdateUnitIcons()
{
	auto pAnim = &m_animZeusUnitIcons;
	auto icons = &m_unitIcons;

	//const auto pClient = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	//const bool isZeus = pClient && pClient->IsZeus();
	//const bool visible = isZeus && TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_force_show", 1);
	if (icons->size())
	{
		pAnim->SetVisible(true);

		const int parametersCount = 9;

		//CW original code
		std::vector< CryFixedStringT<128> > iconList;
		iconList.reserve(parametersCount * icons->size());
		char tempBuf[128];
		auto it = icons->begin();
		for (; it != icons->end(); ++it)
		{
			SUnitIcon icon = (*it);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", (int)icon.id);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.x);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.y);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.icontype);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.friendly);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.distance);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.size);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.rotation);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.healthValue);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.selected);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);
		}

		int size = iconList.size();
		if (size)
		{
			std::vector<const char*> pushArray;
			pushArray.reserve(size);
			for (int i(0); i < size; ++i)
			{
				pushArray.push_back(iconList[i].c_str());
			}

			pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstStrPtr, "m_allValues", 0, &pushArray[0], pushArray.size());
		}


		std::vector< CryFixedWStringT<64> > iconListWString;
		iconList.reserve(icons->size());

		for (auto it = icons->begin(); it != icons->end(); ++it)
		{
			SUnitIcon icon = (*it);
			iconListWString.push_back(icon.text);
		}

		size = iconListWString.size();
		if (size)
		{
			std::vector<const wchar_t*> pushArray;
			pushArray.reserve(size);
			for (int i(0); i < size; ++i)
			{
				pushArray.push_back(iconListWString[i].c_str());
			}

			pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstWstrPtr, "m_allNames", 0, &pushArray[0], pushArray.size());
		}

		pAnim->Invoke("updateMissionObjectives");
		//~ CW original code

		// Commented CW original code
		//const char* description = "";
		//if (m_pHUDRadar)
		//	description = m_pHUDRadar->GetObjectiveDescription(m_objectiveNearCenter);

		//SFlashVarValue args[2] = {(int)m_objectiveNearCenter, description};
		//m_animMissionObjective.Invoke("setNearCenter", args, 2);
		//m_objectiveNearCenter = 0;
		// ~Commented CW original code
	}
	else if (pAnim->GetVisible())
	{
		pAnim->SetVisible(false);
	}

	icons->clear();
}

void CTOSZeusModule::HUDFlashUpdateOrderIcons()
{
	auto pAnim = &m_animZeusOrderIcons;
	auto icons = &m_orderIcons;

	if (icons->size())
	{
		pAnim->SetVisible(true);
		const int parametersCount = 3;

		//CW original code
		std::vector< CryFixedStringT<128> > iconList;
		iconList.reserve(parametersCount * icons->size());

		char tempBuf[128];
		auto it = icons->begin();
		for (; it != icons->end(); ++it)
		{
			SOrderIcon icon = (*it);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.x);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.y);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.size);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);
		}

		int size = iconList.size();
		if (size)
		{
			std::vector<const char*> pushArray;
			pushArray.reserve(size);
			for (int i(0); i < size; ++i)
			{
				pushArray.push_back(iconList[i].c_str());
			}

			pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstStrPtr, "m_allValues", 0, &pushArray[0], pushArray.size());
		}

		//std::vector< CryFixedWStringT<64> > iconListWString;
		//iconList.reserve(icons->size());

		//for (auto it = icons->begin(); it != icons->end(); ++it)
		//{
		//	SOrderIcon icon = (*it);
		//	iconListWString.push_back(icon.text);
		//}

		//size = iconListWString.size();
		//if (size)
		//{
		//	std::vector<const wchar_t*> pushArray;
		//	pushArray.reserve(size);
		//	for (int i(0); i < size; ++i)
		//	{
		//		pushArray.push_back(iconListWString[i].c_str());
		//	}

		//	pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstWstrPtr, "m_allNames", 0, &pushArray[0], pushArray.size());
		//}

		pAnim->Invoke("updateIcons");
	}
	else if (pAnim->GetVisible())
	{
		pAnim->SetVisible(false);
	}

	icons->clear();
}

void CTOSZeusModule::HUDShowPlayerHUD(bool show)
{
	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		// HP
		pHUD->ShowPlayerStats(show);

		pHUD->ShowRadar(show);
		pHUD->ShowCrosshair(show);
	}
}