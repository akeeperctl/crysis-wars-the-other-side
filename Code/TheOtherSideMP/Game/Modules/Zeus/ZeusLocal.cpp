#include "StdAfx.h"
#include "ZeusModule.h"
#include "ZeusSynchronizer.h"
#include <TheOtherSideMP/Helpers/TOS_AI.h>
#include <TheOtherSideMP/Helpers/TOS_Entity.h>
#include <TheOtherSideMP/Helpers/TOS_Console.h>
#include <TheOtherSideMP/Helpers/TOS_Screen.h>
#include <TheOtherSideMP/Helpers/TOS_Vehicle.h>
#include <TheOtherSideMP/Helpers/TOS_STL.h>

static CTOSZeusModule::SOBBWorldPos* CreateBoxForEntity(EntityId id)
{
	OBB box;
	AABB entityBox;
	Vec3 worldPos(ZERO);

	auto pEntity = TOS_GET_ENTITY(id);
	if (!pEntity)
		return nullptr;

	pEntity->GetLocalBounds(entityBox);
	box.SetOBBfromAABB(pEntity->GetRotation(), entityBox);

	worldPos = pEntity->GetWorldPos();

	auto wbox = new CTOSZeusModule::SOBBWorldPos();

	if (box.c.IsZero())
	{
		pEntity->GetWorldBounds(entityBox);
		box = OBB::CreateOBBfromAABB(pEntity->GetRotation(), entityBox);
	}

	wbox->obb = box;
	wbox->wPos = worldPos;

	return wbox;
}

CTOSZeusModule::Local::Local(CTOSZeusModule* _pParent)
	:
	pParent(_pParent),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO),
	m_worldMousePos(ZERO),
	m_worldProjectedMousePos(ZERO),
	m_selectStartPos(ZERO),
	m_clickedSelectStartPos(ZERO),
	m_selectStopPos(ZERO),
	m_mouseIPos(ZERO),
	m_draggingDelta(ZERO),
	m_orderPos(ZERO),
	m_orderTargetId(0),
	m_lastClickedEntityId(0),
	m_curClickedEntityId(0),
	m_dragTargetId(0),
	m_mouseOveredEntityId(0),
	m_mouseDownDurationSec(0),
	m_draggingMoveStartTimer(0),
	m_mouseRayEntityFlags(ZEUS_DEFAULT_MOUSE_ENT_FLAGS),
	m_shiftModifier(false),
	m_ctrlModifier(false),
	m_doubleClick(false),
	m_copying(false),
	m_altModifier(false),
	m_debugZModifier(false),
	m_select(false),
	m_dragging(false),
	m_spaceFreeCam(false),
	m_mouseDisplayed(false)
{
}

void CTOSZeusModule::Local::SetFlag(EFlag flag, bool value)
{
	const int iFlag = int(flag);
	m_zeusFlags = value ? (m_zeusFlags | iFlag) : (m_zeusFlags & ~iFlag);
}

bool CTOSZeusModule::Local::GetFlag(EFlag flag) const
{
	return (m_zeusFlags & int(flag)) != 0;
}

void CTOSZeusModule::Local::ShowMouse(bool show)
{
	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		m_mouseDisplayed = show;
		m_mouseDisplayed ? pMouse->IncrementCounter() : pMouse->DecrementCounter();
		pMouse->ConfineCursor(m_mouseDisplayed);
	}
}

void CTOSZeusModule::Local::Reset()
{
	m_zeusFlags = 0;
	m_select = false;
	m_dragging = false;
	m_doubleClick = false;
	m_copying = false;
	m_ctrlModifier = false;
	m_altModifier = false;
	m_shiftModifier = false;
	m_debugZModifier = false;
	m_spaceFreeCam = false;
	m_mouseDisplayed = false;

	m_orderInfo.Create(gEnv->pScriptSystem);
	m_executorInfo.Create(gEnv->pScriptSystem);

	m_doubleClickLastSelectedEntities.clear();
	m_selectedEntities.clear();
	m_selectStartEntitiesPositions.clear();
	m_storedEntitiesPositions.clear();
	m_orders.clear();
	m_boxes.clear();

	m_lastClickedEntityId = m_curClickedEntityId = 0;
	m_dragTargetId = 0;
	m_mouseOveredEntityId = 0;
}

bool CTOSZeusModule::Local::CanSelectMultiplyWithBox() const
{
	return m_mouseDownDurationSec > TOS_Console::GetSafeFloatVar("tos_sv_zeus_mass_selection_hold_sec", 0.2f);
}

EntityId CTOSZeusModule::Local::GetMouseEntityId() const
{
	if (!pParent->GetPlayer())
		return 0;

	const auto vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
	const auto vDir = (m_worldProjectedMousePos - vCamPos).GetNormalizedSafe();
	auto pPhys = pParent->GetPlayer()->GetEntity()->GetPhysics();

	ray_hit hit;
	const auto queryFlags = ent_all;
	const unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
	const float fRange = gEnv->p3DEngine->GetMaxViewDistance();

	if (gEnv->pPhysicalWorld && gEnv->pPhysicalWorld->RayWorldIntersection(vCamPos, vDir * fRange, queryFlags, flags, &hit, 1, pPhys))
	{
		if (gEnv->p3DEngine->RefineRayHit(&hit, vDir * fRange))
		{
			if (const auto pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider))
				return pEntity->GetId();
		}
	}

	return 0;
}

void CTOSZeusModule::Local::DeselectEntities()
{
	m_doubleClickLastSelectedEntities.clear();
	m_selectedEntities.clear();
	m_boxes.clear();
}

std::set<EntityId>::iterator CTOSZeusModule::Local::DeselectEntity(EntityId id)
{
	auto it = std::lower_bound(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (it != m_selectedEntities.end() && *it == id)
	{
		it = m_selectedEntities.erase(it);
		m_boxes.erase(id);
		return it;
	}
}

void CTOSZeusModule::Local::SelectEntity(EntityId id)
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (pEntity)
	{
		m_selectedEntities.insert(id);
		m_boxes[id] = CreateBoxForEntity(id);
	}
}

void CTOSZeusModule::Local::ClickEntity(EntityId id, const Vec3& clickPos)
{
	m_curClickedEntityId = id;
	m_mouseOveredEntityId = id;
	m_selectStartEntitiesPositions[id] = clickPos;
	m_storedEntitiesPositions[id] = clickPos;
	m_clickedSelectStartPos = clickPos;
}

bool CTOSZeusModule::Local::IsSelectedEntity(EntityId id)
{
	auto it = std::lower_bound(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (it != m_selectedEntities.end() && *it == id)
		return true;

	return false;
}

void CTOSZeusModule::Local::HandleOnceSelection(EntityId id)
{
	// при копировании нельзя выделять новые сущности
	// т.к. сначала нужно переместить скопированные на места
	if (!m_copying)
	{
		m_lastClickedEntityId = m_curClickedEntityId;
		m_curClickedEntityId = id;

		// Одиночное выделение
		if (!m_ctrlModifier)
		{
			//const auto clickedIter = stl::binary_find(m_selectedEntities.begin(), m_selectedEntities.end(), m_curClickedEntityId);
			//const bool clickedSelected = clickedIter != m_selectedEntities.end();

			// При каждом клике без CTRL снимаем выделение если кликнули не на последнюю кликнутую сущность
			//if (m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0)
			if ((m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0))
				DeselectEntities();

			// После чистки всех выделенных сущностей, выделяем кликнутую сущность
			if (SelectionFilter(m_curClickedEntityId) && m_curClickedEntityId != 0)
				SelectEntity(m_curClickedEntityId);
		}
	}

	auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	if (pEntity)
		m_clickedSelectStartPos = pEntity->GetWorldPos();
}

void CTOSZeusModule::Local::OnEntityIconPressed(IEntity* pEntity)
{
	if (!pEntity || m_spaceFreeCam)
		return;

	HandleOnceSelection(pEntity->GetId());
}

void CTOSZeusModule::Local::SaveEntitiesStartPositions()
{
	m_selectStartEntitiesPositions.clear();

	for (auto it = m_selectedEntities.begin(); it != m_selectedEntities.end(); it++)
	{
		EntityId id = *it;

		auto pos = stl::find_in_map(m_selectStartEntitiesPositions, id, Vec3(ZERO));
		if (pos.IsZero())
		{
			auto pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				m_selectStartEntitiesPositions[id] = pEntity->GetWorldPos();
				m_storedEntitiesPositions[id] = pEntity->GetWorldPos();
			}
		}
	}
}

bool CTOSZeusModule::Local::SelectionFilter(EntityId id) const
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (!pEntity)
	{
		return false;
	}

	if (m_debugZModifier)
		return true;

	if (pEntity->IsGarbage())
		return false;

	IItem* pItem = TOS_GET_ITEM(id);
	if (pItem)
	{
		if (pItem->GetEntity()->GetParent())
			return false;
		else if (pItem->GetOwnerId())
			return false;
	}

	const string className = pEntity->GetClass()->GetName();
	auto it = s_classToConsoleVar.find(className);

	// Проверяем, есть ли сопоставление для текущего класса
	if (it != s_classToConsoleVar.end())
	{
		// Получаем консольную переменную для текущего класса
		const string consoleVarName = it->second;

		// Проверяем значение консольной переменной
		if (TOS_Console::GetSafeIntVar(consoleVarName.c_str(), 1) == 1)
		{
			return false;
		}
	}

	// Проверяем класс по умолчанию
	if (pEntity->GetClass() == gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass())
	{
		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_selection_ignore_default", 1) == 1)
		{
			return false;
		}
	}

	return true;
}

void CTOSZeusModule::Local::GetSelectedEntities()
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIt->IsEnd())
	{
		if (IEntity* pEntity = pIt->Next())
		{
			// выделяем только родительскую сущность
			if (TOS_Console::GetSafeIntVar("tos_sv_zeus_selection_always_select_parent", 1) == 1 && pEntity->GetParent())
				pEntity = pEntity->GetParent();

			if (!SelectionFilter(pEntity->GetId()))
				continue;

			if (m_debugZModifier == false)
			{
				// пропускаем мусор (при удалении сущности, в редакторе остается мусор от сущности, который можно выделить повторно)
				if (!pParent->IsPhysicsAllowed(pEntity))
					continue;
			}

			IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
			if (!pClientActor)
				return;

			// пропускаем сущность актера клиента
			if (pEntity->GetId() == pClientActor->GetEntityId())
				continue;

			AABB worldBounds;
			pEntity->GetWorldBounds(worldBounds);

			//skip further calculations if the entity is not visible at all...
			if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
				continue;

			const Vec3 wpos = pEntity->GetWorldPos();
			const Quat rot = pEntity->GetWorldRotation();
			AABB localBounds;

			pEntity->GetLocalBounds(localBounds);

			//get min and max values of the entity bounding box (local positions)
			static Vec3 points[2];
			points[0] = wpos + rot * localBounds.min;
			points[1] = wpos + rot * localBounds.max;

			static Vec3 pointsProjected[2];

			//project the bounding box min max values to screen positions
			for (int i = 0; i < 2; ++i)
			{
				gEnv->pRenderer->ProjectToScreen(points[i].x, points[i].y, points[i].z, &pointsProjected[i].x, &pointsProjected[i].y, &pointsProjected[i].z);
				const float fWidth = gEnv->pRenderer->GetWidth();
				const float fHeight = gEnv->pRenderer->GetHeight();

				//scale projected values to the actual screen resolution
				pointsProjected[i].x *= 0.01f * fWidth;
				pointsProjected[i].y *= 0.01f * fHeight;
			}

			// Check if the projected bounding box min max values are fully or partly inside the screen selection 
			bool isXOverlap =
				// Полное пересечение по X
				(m_selectStartPos.x <= pointsProjected[0].x && pointsProjected[1].x <= m_selectStopPos.x) ||
				// Полное пересечение по X (справа налево)
				(m_selectStartPos.x >= pointsProjected[0].x && pointsProjected[1].x >= m_selectStopPos.x) ||
				// Частичное пересечение по X
				(m_selectStartPos.x <= pointsProjected[0].x && pointsProjected[0].x <= m_selectStopPos.x) ||
				(m_selectStartPos.x >= pointsProjected[0].x && m_selectStopPos.x <= pointsProjected[1].x) ||
				(m_selectStartPos.x <= pointsProjected[1].x && m_selectStopPos.x >= pointsProjected[1].x);

			bool isYOverlap =
				// Полное пересечение по Y
				(m_selectStartPos.y <= pointsProjected[0].y && pointsProjected[1].y <= m_selectStopPos.y) ||
				// Полное пересечение по Y (снизу вверх)
				(m_selectStartPos.y >= pointsProjected[0].y && pointsProjected[1].y >= m_selectStopPos.y) ||
				// Частичное пересечение по Y
				(m_selectStartPos.y <= pointsProjected[0].y && m_selectStopPos.y >= pointsProjected[0].y) ||
				(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[0].y) ||
				(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[1].y);

			if (isXOverlap && isYOverlap)
			{
				// finally we have an entity id
				// if left or right CTRL is not pressed we can directly add every entity id, old entity id's are already deleted
				//m_selectedEntities.insert(pEntity->GetId());
				SelectEntity(pEntity->GetId());
			}
		}
	}
}

void CTOSZeusModule::Local::UpdateUnitIcons(IActor* pClientActor)
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIt->IsEnd())
	{
		EIconColor color = EIconColor::Grey;
		EIcon icon = EIcon::Base;

		if (IEntity* pEntity = pIt->Next())
		{
			const auto id = pEntity->GetId();

			if (!SelectionFilter(id))
				continue;

			if (pEntity->IsHidden())
			{
				const auto selectedIter = stl::binary_find(m_selectedEntities.cbegin(), m_selectedEntities.cend(), id);
				const bool selected = selectedIter != m_selectedEntities.cend();
				if (!(selected && (m_copying || pParent->GetHUD().m_menuSpawnHandling)))
				{
					continue;
				}
			}

			if (!pParent->IsPhysicsAllowed(pEntity))
				continue;

			// пропускаем сущность актера клиента
			if (id == pClientActor->GetEntityId())
				continue;

			//skip further calculations if the entity is not visible at all...
			AABB worldBounds;
			pEntity->GetWorldBounds(worldBounds);
			if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
				continue;

			IVehicle* pVehicle = TOS_GET_VEHICLE(id);
			if (pVehicle)
			{
				//TODO Танк, VTOL!
				const auto movType = pVehicle->GetMovement()->GetMovementType();
				if (movType == IVehicleMovement::eVMT_Land)
					icon = EIcon::Car;
				else if (movType == IVehicleMovement::eVMT_Air)
					icon = EIcon::Helicopter;
				else if (movType == IVehicleMovement::eVMT_Sea || movType == IVehicleMovement::eVMT_Amphibious)
					icon = EIcon::Boat;

				// не пустые тс должны быть желтые
				if (pVehicle->GetStatus().passengerCount > 0)
					color = EIconColor::Yellow;

				if (pVehicle->IsDestroyed())
					color = EIconColor::Grey;
			}

			const IActor* pActor = TOS_GET_ACTOR(id);
			if (pActor)
			{
				icon = EIcon::Unit;

				if (pActor->GetHealth() > 0)
					color = EIconColor::Yellow;

				auto pLinkedVehicle = TOS_Vehicle::GetVehicle(pActor);
				if (pLinkedVehicle)
					continue;
			}

			IItem* pItem = TOS_GET_ITEM(id);
			if (pItem)
			{
				icon = EIcon::Circle;

				const auto pPickupClass = TOS_GET_ENTITY_CLASS("CustomAmmoPickup");

				if (pItem->GetEntity()->GetClass() == pPickupClass)
					icon = EIcon::Ammo;
				else if (pItem->GetIWeapon())
					icon = EIcon::Rifle;
			}

			pParent->GetHUD().CreateUnitIcon(id, int(color), int(icon), Vec3(0, 0, 0));
		}
	}
}

void CTOSZeusModule::Local::UpdateOrderIcons()
{
	auto it = m_selectedEntities.cbegin();
	auto end = m_selectedEntities.cend();
	for (; it != end; it++)
	{
		auto foundIt = m_orders.find(*it);
		if (foundIt != m_orders.end())
		{
			SOrder* order = &foundIt->second;
			auto pTarget = TOS_GET_ENTITY(order->targetId);
			if (pTarget)
				order->pos = pTarget->GetWorldPos();

			pParent->GetHUD().CreateOrderIcon(order->pos);
			pParent->GetHUD().CreateOrderLine(foundIt->first, order->pos);
		}
	}
}

void CTOSZeusModule::Local::CreateOrder(EntityId executorId, const SOrder& info)
{
	m_orders[executorId] = info;
}

void CTOSZeusModule::Local::StopOrder(EntityId executorId)
{
	IScriptSystem* pSS = gEnv->pScriptSystem;
	if (pSS->ExecuteFile("Scripts/AI/TOS/TOSHandleOrder.lua", true, false))
	{
		pSS->BeginCall("StopOrder");
		pSS->PushFuncParam(ScriptHandle(executorId));
		pSS->EndCall();
	}
}

void CTOSZeusModule::Local::RemoveOrder(EntityId executorId)
{
	m_orders.erase(executorId);
}

void CTOSZeusModule::Local::UpdateDebug(bool zeusMoving, const Vec3& zeusDynVec)
{
	if (TOS_Console::GetSafeIntVar("tos_cl_zeus_dragging_draw_debug", 0) > 0)
	{
		IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin("ZeusModule", true);
		const auto blue = ColorF(0, 0, 1, 1);
		const auto green = ColorF(0, 1, 0, 1);
		const auto red = ColorF(1, 0, 0, 1);
		const auto orderColor = ColorF(0.7, 0.3, 1, 1);

		// Отрисовка позиции мыши в реальном мире
		///////////////////////////////////////////////////////////////////////
		pPD->AddSphere(m_worldProjectedMousePos, 0.20f, blue, 1.0f);

		// Отрисовка позиции приказа в реальном мире
		///////////////////////////////////////////////////////////////////////
		const auto pOrderTargerEntity = TOS_GET_ENTITY(m_orderTargetId);

		pPD->AddSphere(m_orderPos, 0.40f, orderColor, 1.0f);
		Vec3 ordScreenPos(ZERO);
		TOS_Screen::ProjectToScreen(m_orderPos, ordScreenPos);
		pPD->AddText(ordScreenPos.x, ordScreenPos.y, 1.3f, orderColor, 1.0f, "Order position: (%1.f, %1.f, %1.f)", m_orderPos.x, m_orderPos.y, m_orderPos.z);
		pPD->AddText(ordScreenPos.x, ordScreenPos.y + 20, 1.3f, orderColor, 1.0f, "Order target: %s", pOrderTargerEntity ? pOrderTargerEntity->GetName() : "NONE");

		if (m_dragging)
		{
			// Отрисовка фактических координат сущности
			const auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
			if (pEntity)
			{
				const Vec3 startPos = m_selectStartEntitiesPositions[pEntity->GetId()];
				Vec3 newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, startPos.z);
				if (m_altModifier)
				{
					newPos.z += m_draggingDelta.z;
				}

				pPD->AddSphere(newPos, 0.20f, red, 1.0f);
				pPD->AddLine(startPos, newPos, red, 1.0f);
			}

			// Отрисовка точки начала выделения сущностей
			///////////////////////////////////////////////////////////////////////
			pPD->AddText(m_mouseIPos.x, m_mouseIPos.y, 1.4f, green, 1.0f, "m_draggingDelta (%1.f, %1.f, %1.f)",
				m_draggingDelta.x, m_draggingDelta.y, m_draggingDelta.z);
			pPD->AddSphere(m_clickedSelectStartPos, 0.20f, green, 1.0f);
			pPD->AddLine(m_clickedSelectStartPos, m_worldProjectedMousePos, green, 1.0f);
		}

		// Вывод текстовой отладки
		///////////////////////////////////////////////////////////////////////
		float color[] = { 1,1,1,1 };
		const int startY = 100;
		const int deltaY = 20;

		const auto pLastClickedEntity = TOS_GET_ENTITY(m_lastClickedEntityId);
		const auto pCurrClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
		const auto pDragTargetEntity = TOS_GET_ENTITY(m_dragTargetId);
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 0, 1.3f, color, false, "lastClickedEntity = %s", pLastClickedEntity ? pLastClickedEntity->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 1, 1.3f, color, false, "currClickedEntity = %s", pCurrClickedEntity ? pCurrClickedEntity->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 2, 1.3f, color, false, "dragTargetEntity  = %s", pDragTargetEntity ? pDragTargetEntity->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 3, 1.3f, color, false, "orderTargerEntity  = %s", pOrderTargerEntity ? pOrderTargerEntity->GetName() : "");

		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 4, 1.3f, color, false, "m_draggingMoveStartTimer = %f", m_draggingMoveStartTimer);

		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 5, 1.3f, color, false, "m_mouseIPos = (%i, %i)", m_mouseIPos.x, m_mouseIPos.y);
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 6, 1.3f, color, false, "m_mouseDownDurationSec = %1.f", m_mouseDownDurationSec);

		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 7, 1.3f, color, false, "m_doubleClick = %i", int(m_doubleClick));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 8, 1.3f, color, false, "m_select = %i", int(m_select));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 9, 1.3f, color, false, "m_dragging = %i", int(m_dragging));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 10, 1.3f, color, false, "m_copying = %i", int(m_copying));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 11, 1.3f, color, false, "m_ctrlModifier = %i", int(m_ctrlModifier));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 12, 1.3f, color, false, "m_altModifier = %i", int(m_altModifier));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 13, 1.3f, color, false, "m_debugZModifier = %i", int(m_debugZModifier));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 14, 1.3f, color, false, "m_menuSpawnHandling = %i", int(pParent->GetHUD().m_menuSpawnHandling));
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 15, 1.3f, color, false, "m_spaceFreeCam = %i", int(m_spaceFreeCam));

		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 16, 1.3f, color, false, "zeusDynVec = (%1.f,%1.f,%1.f)", zeusDynVec.x, zeusDynVec.y, zeusDynVec.z);
		gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 17, 1.3f, color, false, "zeusMoving = %i", zeusMoving);

		if (!m_selectedEntities.empty())
		{
			TOS_Debug::DrawEntitiesName2DLabel(m_selectedEntities, "Selected Entities: ", 100, startY + deltaY * 18, deltaY);
		}

		if (!m_doubleClickLastSelectedEntities.empty())
		{
			TOS_Debug::DrawEntitiesName2DLabel(m_doubleClickLastSelectedEntities, "DC Selected Entities: ", 300, startY + deltaY * 19, deltaY);
		}

		//if (!m_orders.empty())
		//{
		//	TOS_Debug::DrawEntitiesName2DLabelMap(m_orders, "Orders: ", 100, startY + deltaY * 20, deltaY);
		//}

		// Вывод текстовой отладки кликнутой сущности
		///////////////////////////////////////////////////////////////////////
		auto pDebugEntity = pCurrClickedEntity;
		if (!pDebugEntity)
		{
			pDebugEntity = TOS_GET_ENTITY(*m_selectedEntities.begin());
		}

		if (pDebugEntity)
		{
			Vec3 screenPos(ZERO);
			TOS_Screen::ProjectToScreen(pDebugEntity->GetWorldPos(), screenPos);

			auto pParentEnt = pDebugEntity->GetParent();

			pPD->AddText(screenPos.x, screenPos.y + 20 * 0, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Name = %s", pDebugEntity->GetName());
			pPD->AddText(screenPos.x, screenPos.y + 20 * 1, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "ID = %i", pDebugEntity->GetId());
			pPD->AddText(screenPos.x, screenPos.y + 20 * 2, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "IsGarbage = %i", int(pDebugEntity->IsGarbage()));
			pPD->AddText(screenPos.x, screenPos.y + 20 * 3, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "IsRemovable = %i", int((ENTITY_FLAG_UNREMOVABLE & pDebugEntity->GetFlags()) == 0));
			pPD->AddText(screenPos.x, screenPos.y + 20 * 4, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "IsActive = %i", int(pDebugEntity->IsActive()));
			pPD->AddText(screenPos.x, screenPos.y + 20 * 5, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Physics = %i", int(pDebugEntity->GetPhysics() != nullptr));
			pPD->AddText(screenPos.x, screenPos.y + 20 * 6, 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Parent = %s", pParentEnt ? pParentEnt->GetName() : "NONE");

			int actorDelta = 7;
			int itemDelta = 7;

			auto pActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(pDebugEntity->GetId()));
			auto pItem = static_cast<CItem*>(TOS_GET_ITEM(pDebugEntity->GetId()));

			if (pActor)
			{
				pPD->AddText(screenPos.x, screenPos.y + 20 * (actorDelta + 0), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Health = %i", pActor->GetHealth());
				pPD->AddText(screenPos.x, screenPos.y + 20 * (actorDelta + 1), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Stance = %i", int(pActor->GetStance()));
			}
			else if (pItem)
			{
				auto id = pItem->GetOwnerId();
				auto pOwner = TOS_GET_ENTITY(id);
				auto pActorOwner = static_cast<CTOSActor*>(TOS_GET_ACTOR(id));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 0), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "Owner = %s(%i)", pOwner ? pOwner->GetName() : "NONE", id);
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 1), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "dropped = %i", int(pItem->GetStats().dropped));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 2), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "flying = %i", int(pItem->GetStats().flying));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 3), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "mounted = %i", int(pItem->GetStats().mounted));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 4), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "pickable = %i", int(pItem->GetStats().pickable));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 5), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "selectable = %i", int(pItem->GetStats().selectable));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 6), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "selected = %i", int(pItem->GetStats().selected));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 7), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "used = %i", int(pItem->GetStats().used));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 8), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "updating = %i", int(pItem->GetStats().updating));
				pPD->AddText(screenPos.x, screenPos.y + 20 * (itemDelta + 9), 1.2f, ColorF(1, 1, 1, 1), 1.0f, "inOwnerInventory = %i", pActorOwner ? (pActorOwner->GetInventory()->FindItem(pItem->GetEntityId())) : -1);
			}
		}

	}
}

int CTOSZeusModule::Local::MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags, bool boxDistanceAdjustment)
{
	if (!TOS_GET_CLIENT_ACTOR)
		return 0;
	if (!TOS_GET_CLIENT_ACTOR->GetEntity())
		return 0;
	if (!TOS_GET_CLIENT_ACTOR->GetEntity()->GetPhysics())
		return 0;

	const Vec3 camWorldPos = gEnv->pSystem->GetViewCamera().GetPosition();
	Vec3 camToMouseDir = (mouseWorldPos - camWorldPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();

	float clickedBoxDistance = 300.0f; // дистанция до кликнутой сущности

	IPhysicalEntity* pSkipEnts[2] = { TOS_GET_CLIENT_ACTOR->GetEntity()->GetPhysics(), nullptr };
	const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	if (pClickedEntity)
		pSkipEnts[1] = pClickedEntity->GetPhysics();

	if (boxDistanceAdjustment)
	{
		clickedBoxDistance = m_clickedSelectStartPos.GetDistance(mouseWorldPos);
		camToMouseDir = camToMouseDir.GetNormalizedSafe() * clickedBoxDistance;


		//auto it = m_boxes.find(m_curClickedEntityId);
		//if (it != m_boxes.end())
		//{
		//	//clickedBoxDistance = pClickedEntity->GetWorldPos().GetDistance(mouseWorldPos);
		//	clickedBoxDistance = it->second->wPos.GetDistance(mouseWorldPos);
		//	camToMouseDir = camToMouseDir.GetNormalizedSafe() * clickedBoxDistance;
		//}
	}

	const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
	const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_stop_at_pierceable);

	return gEnv->pPhysicalWorld->RayWorldIntersection(mouseWorldPos, camToMouseDir, entityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
}

bool CTOSZeusModule::Local::UpdateDraggedEntity(EntityId id, const IEntity* pClickedEntity, IPhysicalEntity* pZeusPhys, std::map<EntityId, _smart_ptr<SOBBWorldPos>>& container, bool heightAutoCalc)
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (!pEntity)
		return false;

	const string className = pEntity->GetClass()->GetName();

	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
	if ((pActor && pActor->GetHealth() < 0.0f) || className == "DeadBody")
	{
		// Пропускаем убитых актеров при условии
		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_ignore_dead_bodies", 0) < 1)
			return false;
	}

	Matrix34 mat34 = pEntity->GetWorldTM();
	//const Vec3 curPos = mat34.GetTranslation();
	Vec3 curPos = container[id]->wPos;
	if (curPos.IsZero())
		curPos = mat34.GetTranslation();

	const Vec3 startPos = m_selectStartEntitiesPositions[id];

	if (pParent->GetHUD().m_menuSpawnHandling)
	{
		ray_hit ray;
		MouseProjectToWorld(ray, m_worldMousePos, ent_all, false);

		Vec3 newPos(ray.pt);
		newPos.z += 0.1f;

		container[id]->wPos = newPos;
	}
	else
	{
		if (!m_shiftModifier)
		{
			Vec3 newPos(ZERO);
			if (m_altModifier)
			{
				// Перемещение по Z
				float height = startPos.z + m_draggingDelta.z;
				m_storedEntitiesPositions[id].z = height;

				newPos = Vec3(curPos.x, curPos.y, height);
			}
			else
			{
				// Перемещение по X, Y с автоприлипанием к поверхности
				newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, m_storedEntitiesPositions[id].z);

				if (heightAutoCalc)
				{
					const Vec3 entToGroundDir = (Vec3(curPos.x, curPos.y, curPos.z - 2) - curPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();
					const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
					ray_hit ray;

					IPhysicalEntity* pSkipEnts[3];
					pSkipEnts[0] = pZeusPhys;
					pSkipEnts[1] = pEntity->GetPhysics();

					if (pClickedEntity)
						pSkipEnts[2] = pClickedEntity->GetPhysics();

					const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
					const int hitNum = gEnv->pPhysicalWorld->RayWorldIntersection(Vec3(curPos.x, curPos.y, curPos.z + 1), entToGroundDir, m_mouseRayEntityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
					if (hitNum)
						newPos.z = ray.pt.z + 0.01f;
				}
			}

			container[id]->wPos = newPos;
		}
		else
		{
			const Vec3 toMouseDir = (m_worldProjectedMousePos - curPos).GetNormalizedSafe();
			const Quat rot = Quat::CreateRotationVDir(toMouseDir);
			container[id]->wPos = curPos;
			container[id]->obb.m33 = Matrix33(rot);
		}
	}

	return true;
}

bool CTOSZeusModule::Local::ExecuteCommand(ECommand command)
{
	bool needUpdateIter = true;
	std::set<EntityId> copiedEntities;

	auto it = m_selectedEntities.begin();
	auto end = m_selectedEntities.end();
	while (it != end)
	{
		const EntityId id = *it;
		const int index = TOS_STL::GetIndexFromMapKey(m_selectedEntities, id);

		IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
		IActor* pActor = TOS_GET_ACTOR(id);

		switch (command)
		{
		case ECommand::KillSelected:
		{
			string hitType = "event";

			HitInfo info;
			info.SetDamage(99999.0f);
			info.targetId = id;
			info.type = g_pGame->GetGameRules()->GetHitTypeId(hitType.c_str());

			g_pGame->GetGameRules()->ClientHit(info);

			if (pVehicle)
				TOS_Vehicle::Destroy(pVehicle);

			break;
		}
		case ECommand::RemoveSelected:
		{
			m_dragging = false;
			m_select = false;
			m_copying = false;

			if (pActor)
			{
				auto pActorVeh = pActor->GetLinkedVehicle();
				if (pActorVeh)
				{
					TOS_Vehicle::Exit(pActor, false, true);
				}
			}

			auto pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				pEntity->Hide(true);
				pEntity->Activate(false);
			}

			TOS_Entity::RemoveEntityDelayed(id, 2);

			it = DeselectEntity(id);
			m_doubleClickLastSelectedEntities.erase(id);

			needUpdateIter = false;

			break;
		}
		case ECommand::CopySelected:
		{
			// Get the entity from the given ID
			const auto pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				auto pSync = static_cast<CTOSZeusSynchronizer*>(pParent->GetSynchronizer());
				assert(pSync != nullptr);

				auto& method = CTOSZeusSynchronizer::SvRequestSpawnEntity();
				auto netParams = CTOSZeusSynchronizer::NetSpawnParams();
				netParams.className = pEntity->GetClass()->GetName();
				netParams.pos = pEntity->GetWorldPos();
				netParams.dir = pEntity->GetWorldRotation().GetColumn1();

				pSync->RMISend(method, netParams, eRMI_ToServer);

				// Initialize variables
				// int savedItemCount = 0;
				// std::map<int, std::string> savedItems;
				// std::string currentItemClass;
				// EStance selectedStance = STANCE_STAND;

				// Get the actor associated with the entity
				//CTOSActor* pActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(id));
				//if (pActor)
					//selectedStance = pActor->GetStance();

				// Prepare spawn parameters
				//STOSEntitySpawnParams params;
				//params.vanilla.bStaticEntityId = false; // Set to true for faster performance but slower editor
				//params.vanilla.bIgnoreLock = false; // Ignore spawn lock
				//params.vanilla.nFlags = pEntity->GetFlags();
				//params.vanilla.nFlags &= ~ENTITY_FLAG_UNREMOVABLE;
				//params.vanilla.pClass = pEntity->GetClass();
				//params.vanilla.vPosition = pEntity->GetWorldPos();
				//params.vanilla.qRotation = pEntity->GetRotation();

				//// Set archetype if available
				//if (pEntity->GetArchetype())
				//	params.vanilla.pArchetype = pEntity->GetArchetype();

				//// Get script table properties
				//const auto pScriptTable = pEntity->GetScriptTable();
				//if (pScriptTable)
				//	pScriptTable->GetValue("Properties", params.properties);

				// Spawn the entity 
				//const auto pSpawned = TOS_Entity::Spawn(params, false);
				//if (pSpawned)
				//{
					// Deselect the original entity and update the iterator
					//it = DeselectEntity(id);
					//needUpdateIter = false;

					//// Hide the spawned entity and set it as the current dragging entity
					//pSpawned->Hide(true);
					//m_dragging = true;

					//// Get the ID of the spawned entity and set its stance
					//const auto spawnedId = pSpawned->GetId();
					//auto pSpawnedActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(spawnedId));
					//if (pSpawnedActor)
					//{
					//	pSpawnedActor->SetStance(selectedStance);
					//	TOS_AI::SetStance(pSpawnedActor->GetEntity()->GetAI(), selectedStance);
					//}

					//// Set the name of the spawned entity
					//char buffer[16];
					//sprintf(buffer, "%i", spawnedId);
					//pSpawned->SetName(string(pSpawned->GetClass()->GetName()) + "_" + buffer);

					//// Update tracking variables
					//m_lastClickedEntityId = id;
					//m_curClickedEntityId = spawnedId;
					//m_mouseOveredEntityId = spawnedId;

					//// Store positions
					//const auto pos = pSpawned->GetWorldPos();
					//m_selectStartEntitiesPositions[spawnedId] = pos;
					//m_storedEntitiesPositions[spawnedId] = pos;
					//m_clickedSelectStartPos = pos;

					// Add the spawned entity to the copied entities set
					//copiedEntities.insert(spawnedId);
				//}
			}
			break;
		}
		case ECommand::OrderSelected:
		{
			auto pAI = TOS_GET_ENTITY(id)->GetAI();
			if (pAI && pAI->IsEnabled())
			{
				MouseProjectToWorld(m_mouseRay, m_worldMousePos, m_mouseRayEntityFlags, false);
				m_orderPos = m_mouseRay.pt;

				m_orderTargetId = GetMouseEntityId();
				auto pOrderTargetEnt = TOS_GET_ENTITY(m_orderTargetId);
				if (pOrderTargetEnt)
					m_orderPos = pOrderTargetEnt->GetWorldPos();

				SOrder order;
				order.pos = m_orderPos;
				order.targetId = m_orderTargetId;
				CreateOrder(id, order);

				IScriptSystem* pSS = gEnv->pScriptSystem;
				if (pSS->ExecuteFile("Scripts/AI/TOS/TOSHandleOrder.lua", true, true))
				{
					CScriptSetGetChain executorChain(m_executorInfo);
					executorChain.SetValue("entityId", id);
					executorChain.SetValue("maxCount", int(m_selectedEntities.size())); // макс. кол-во исполнителей
					executorChain.SetValue("index", int(index)); // текущий номер исполнителя

					CScriptSetGetChain orderChain(m_orderInfo);
					orderChain.SetValue("goalPipeId", id); // так надо
					orderChain.SetValue("pos", order.pos);
					orderChain.SetValue("targetId", order.targetId);

					pSS->BeginCall("HandleOrder");
					pSS->PushFuncParam(m_executorInfo);
					pSS->PushFuncParam(m_orderInfo);
					pSS->EndCall();
				}
			}

			break;
		}
		default:
			break;
		}

		if (needUpdateIter)
			it++;
	}

	//if (command == ECommand::CopySelected)
	//{
	//	for (auto it = copiedEntities.cbegin(); it != copiedEntities.cend(); it++)
	//	{
	//		SelectEntity(*it);
	//	}
	//}

	return true;
}

