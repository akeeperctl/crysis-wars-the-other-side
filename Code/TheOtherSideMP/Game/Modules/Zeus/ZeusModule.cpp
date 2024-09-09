#include "StdAfx.h"
#include "HUD/HUD.h"
#include "HUD/HUDSilhouettes.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include <TheOtherSideMP\HUD\TOSCrosshair.h>
#include <TheOtherSideMP\Helpers\TOS_AI.h>
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_NET.h>
#include <TheOtherSideMP\Helpers\TOS_Inventory.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include <TheOtherSideMP\Helpers\TOS_Vehicle.h>
#include <Cry_Camera.h>

const uint DEFAULT_MOUSE_ENT_FLAGS = ent_terrain; //ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;
const uint DRAGGING_MOUSE_ENT_FLAGS = ent_terrain;
enum eScreenIconType
{
	eFTI_Grey = 0,
	eFTI_Blue,
	eFTI_Red,
	eFTI_Yellow,
};

CTOSZeusModule::CTOSZeusModule()
	: m_zeus(nullptr),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO),
	m_worldMousePos(ZERO),
	m_worldProjectedMousePos(ZERO),
	m_selectStartPos(ZERO),
	m_clickedSelectStartPos(ZERO),
	m_selectStopPos(ZERO),
	m_mouseIPos(ZERO),
	m_draggingDelta(ZERO),
	m_lastClickedEntityId(0),
	m_curClickedEntityId(0),
	m_mouseOveredEntityId(0),
	m_mouseDownDurationSec(0),
	m_draggingMoveStartTimer(0),
	m_updateIconOnScreenTimer(0),
	m_mouseRayEntityFlags(DEFAULT_MOUSE_ENT_FLAGS),
	m_shiftModifier(false),
	m_ctrlModifier(false),
	m_altModifier(false),
	m_debugZModifier(false),
	m_select(false),
	m_dragging(false)
{}

CTOSZeusModule::~CTOSZeusModule()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->RemoveListener(this);
}

bool CTOSZeusModule::OnInputEvent(const SInputEvent& event)
{
	if (event.deviceId == EDeviceId::eDI_Keyboard)
	{
		if (event.keyId == EKeyId::eKI_LCtrl)
		{
			if (event.state == eIS_Pressed)
				m_ctrlModifier = true;
			else if (event.state == eIS_Released)
				m_ctrlModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_LAlt)
		{
			if (event.state == eIS_Pressed)
			{
				m_altModifier = true;
				m_select = false;
			}
			else if (event.state == eIS_Released)
			{
				m_altModifier = false;
			}
		}
		else if (event.keyId == EKeyId::eKI_Z)
		{
			if (event.state == eIS_Pressed)
				m_debugZModifier = true;
			else if (event.state == eIS_Released)
				m_debugZModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_LShift)
		{
			if (event.state == eIS_Pressed)
				m_shiftModifier = true;
			else if (event.state == eIS_Released)
				m_shiftModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_End)
		{
			ExecuteCommand(eZC_KillSelected);
		}
		else if (event.keyId == EKeyId::eKI_Delete)
		{
			ExecuteCommand(eZC_RemoveSelected);
		}
	}

	return true;
}

bool CTOSZeusModule::CanSelectMultiplyWithBox() const
{
	return m_mouseDownDurationSec > TOS_Console::GetSafeFloatVar("tos_sv_zeus_mass_selection_hold_sec", 0.2f);
}

EntityId CTOSZeusModule::GetMouseEntityId()
{
	if (!m_zeus)
		return 0;

	const auto vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
	const auto vDir = (m_worldProjectedMousePos - vCamPos).GetNormalizedSafe();
	auto pPhys = m_zeus->GetEntity()->GetPhysics();

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

void CTOSZeusModule::DeselectEntities()
{
	m_selectedEntities.clear();
}

void CTOSZeusModule::HandleOnceSelection(EntityId id)
{
	m_lastClickedEntityId = m_curClickedEntityId;
	m_curClickedEntityId = id;

	// Одиночное выделение
	if (!m_ctrlModifier)
	{
		// При каждом клике без CTRL снимаем выделение если кликнули не на последнюю кликнутую сущность
		if (m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0)
			DeselectEntities();

		// После чистки всех выделенных сущностей, выделяем кликнутую сущность
		if (m_curClickedEntityId != 0)
		{
			m_selectedEntities.insert(m_curClickedEntityId);
		}
	}

	auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	if (pEntity)
		m_clickedSelectStartPos = pEntity->GetWorldPos();
}

void CTOSZeusModule::OnEntityIconPressed(IEntity* pEntity)
{
	if (!pEntity)
		return;

	HandleOnceSelection(pEntity->GetId());
}

void CTOSZeusModule::SaveEntitiesStartPositions()
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

void CTOSZeusModule::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_mouseIPos.x = iX;
	m_mouseIPos.y = iY;

	auto mod_iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, mod_iY, 0.0f, &m_worldMousePos.x, &m_worldMousePos.y, &m_worldMousePos.z);

	auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		SFlashCursorEvent::ECursorState eCursorState = SFlashCursorEvent::eCursorMoved;
		if (HARDWAREMOUSEEVENT_LBUTTONDOWN == eHardwareMouseEvent)
		{
			eCursorState = SFlashCursorEvent::eCursorPressed;
		}
		else if (HARDWAREMOUSEEVENT_LBUTTONUP == eHardwareMouseEvent)
		{
			eCursorState = SFlashCursorEvent::eCursorReleased;
		}

		if (m_animZeusScreenIcons.IsLoaded())
		{
			int x(iX), y(iY);
			m_animZeusScreenIcons.GetFlashPlayer()->ScreenToClient(x, y);
			m_animZeusScreenIcons.GetFlashPlayer()->SendCursorEvent(SFlashCursorEvent(eCursorState, x, y));
		}

		if (!pHUD->IsHaveModalHUD())
		{
			if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOWN)
			{
				m_select = true;
				m_selectStartPos = Vec2i(iX, iY);

				HandleOnceSelection(GetMouseEntityId());
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONUP)
			{
				m_select = false;
				m_selectStopPos = Vec2i(iX, iY);

				if (!m_dragging && CanSelectMultiplyWithBox())
					GetSelectedEntities();
				else
				{
					// Множественное выделение c зажатым модификатором
					if (m_ctrlModifier)
					{
						if (!m_dragging && m_curClickedEntityId != 0)
						{
							if (m_selectedEntities.count(m_curClickedEntityId) > 0)
							{
								stl::binary_erase(m_selectedEntities, m_curClickedEntityId);
							}
							else
							{
								m_selectedEntities.insert(m_curClickedEntityId);
							}
						}
					}
				}

				// Пинаем физику выделенных сущностей после того как закончили их перетаскивать
				if (m_dragging)
				{
					for (auto it = m_selectedEntities.begin(); it != m_selectedEntities.end(); it++)
					{
						auto pEntity = TOS_GET_ENTITY(*it);
						if (!pEntity)
							continue;

						auto pPhys = pEntity->GetPhysics();
						if (!pPhys)
							continue;

						pe_action_awake awake;
						awake.bAwake = 1;

						pPhys->Action(&awake);
					}
				}

				m_dragging = false;
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_MOVE)
			{
				// При перетаскивании кликнутая сущность должна быть выделена
				const auto clickedIter = stl::binary_find(m_selectedEntities.begin(), m_selectedEntities.end(), m_curClickedEntityId);
				const bool clickedSelected = clickedIter != m_selectedEntities.end();

				// Мышь находится в диапазоне иконки кликнутой сущности. True - да
				const bool clickedOveredByMouse = m_mouseOveredEntityId == m_curClickedEntityId;

				if (m_select && m_curClickedEntityId != 0 && clickedSelected && clickedOveredByMouse)
				{
					// Перед тем как начать перемещение сущностей...
					if (m_dragging == false)
					{
						// Сохраняем начальное положение каждой выделенной сущности
						SaveEntitiesStartPositions();

						// Запуск таймера 
						m_draggingMoveStartTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_dragging_move_start_delay", 0.05f);
					}

					m_dragging = true;
				}
			}

		}
	}
}

static bool IsPhysicsAllowed(IEntity* pEntity)
{
	if (!pEntity)
		return false;

	IPhysicalEntity* physEnt = pEntity->GetPhysics();
	if (!physEnt)
		return false;

	// допустимые сущности
	const auto type = physEnt->GetType();
	if (!(type == PE_LIVING || type == PE_RIGID || type == PE_STATIC || type == PE_WHEELEDVEHICLE || PE_ARTICULATED))
		return false;
}

void CTOSZeusModule::GetSelectedEntities()
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIt->IsEnd())
	{
		if (IEntity* pEntity = pIt->Next())
		{
			// выделяем только родительскую сущность
			if (TOS_Console::GetSafeIntVar("tos_sv_zeus_always_select_parent", 1) && pEntity->GetParent())
				pEntity = pEntity->GetParent();

			if (m_debugZModifier == false)
			{
				// пропускаем мусор (при удалении сущности, в редакторе остается мусор от сущности, который можно выделить повторно)
				if (pEntity->IsGarbage())
					continue;

				if (!IsPhysicsAllowed(pEntity))
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
				m_selectedEntities.insert(pEntity->GetId());
			}
		}
	}
}

void CTOSZeusModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	auto pHUD = g_pGame->GetHUD();
	bool noModalOrNoHUD = !pHUD || (pHUD && !pHUD->IsHaveModalHUD());

	switch (event.event)
	{
		case eEGE_ActorRevived:
		{
			if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
			{
				Reset();
				if (noModalOrNoHUD)
					ShowMouse(false);
			}
			break;
		}
		case eEGE_MasterClientOnStartControl:
		{
			if (m_zeus)
			{
				ShowHUD(true);
				if (noModalOrNoHUD)
					ShowMouse(false);

				SetZeusFlag(eZF_Possessing, true);
			}
			break;
		}
		case eEGE_MasterClientOnStopControl:
		{
			if (m_zeus)
			{
				ApplyZeusProperties(m_zeus);
				SetZeusFlag(eZF_Possessing, false);
			}
			break;
		}
		case eEGE_HUDInit:
		{
			HUDInit();
			break;
		}
		case eEGE_HUDInGamePostUpdate:
		{
			HUDInGamePostUpdate(event.value);
			break;
		}
		case eEGE_HUDUnloadSimpleAssets:
		{
			HUDUnloadSimpleAssets(event.int_value);
			break;
		}
		//case eEGE_ActorEnterVehicle:
		//{
		//	if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
		//	{
		//		if (noModalOrNoHUD)
		//			ShowMouse(false);
		//	}

		//	break;
		//}
		//case eEGE_ActorExitVehicle:
		//{
		//	if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
		//	{				
		//		//ApplyZeusProperties(m_zeus);
		//	}
		//	break;
		//}
		default:
			break;
	}
}

void CTOSZeusModule::Reset()
{
	if (m_zeus)
	{
		m_zeus->m_isZeus = false;
		m_zeus = nullptr;
	}
	m_zeusFlags = 0;
	m_select = false;
	m_dragging = false;
	m_ctrlModifier = false;
	m_altModifier = false;
	m_shiftModifier = false;
	m_debugZModifier = false;
	m_selectedEntities.clear();
	m_selectStartEntitiesPositions.clear();
	m_storedEntitiesPositions.clear();
	m_lastClickedEntityId = m_curClickedEntityId = 0;
	m_mouseOveredEntityId = 0;
	m_updateIconOnScreenTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_update_delay", 0.1f);
}

void CTOSZeusModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_selectedEntities);
	s->AddContainer(m_selectStartEntitiesPositions);
	s->AddContainer(m_storedEntitiesPositions);
	s->AddContainer(m_onScreenIcons);
}

void CTOSZeusModule::Init()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->AddListener(this);

	Reset();
}

int CTOSZeusModule::MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags)
{
	if (!m_zeus || !m_zeus->GetEntity() || !m_zeus->GetEntity()->GetPhysics())
		return 0;

	const Vec3 camWorldPos = gEnv->pSystem->GetViewCamera().GetPosition();
	Vec3 camToMouseDir = (mouseWorldPos - camWorldPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();

	float clickedDistance = 10.0f; // дистанция до кликнутой сущности
	const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);

	IPhysicalEntity* pSkipEnts[2] = {m_zeus->GetEntity()->GetPhysics(), nullptr};
	if (pClickedEntity)
	{
		clickedDistance = pClickedEntity->GetWorldPos().GetDistance(mouseWorldPos);
		camToMouseDir = camToMouseDir.GetNormalizedSafe() * clickedDistance;

		pSkipEnts[1] = pClickedEntity->GetPhysics();
	}


	const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
	const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_stop_at_pierceable);

	return gEnv->pPhysicalWorld->RayWorldIntersection(mouseWorldPos, camToMouseDir, entityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
}

void CTOSZeusModule::Update(float frametime)
{
	if (!m_zeus || !m_zeus->IsClient())
		return;

	//int frameID = gEnv->pRenderer->GetFrameID();

	// Привязка мыши к позиции, когда крутится камера
	///////////////////////////////////////////////////////////////////////
	if (m_zeusFlags & eZF_CanRotateCamera && !(m_zeusFlags & eZF_Possessing))
	{
		auto pMouse = gEnv->pHardwareMouse;

		if (m_anchoredMousePos == Vec2(0, 0))
		{
			Vec2 mousePos;
			pMouse->GetHardwareMousePosition(&mousePos.x, &mousePos.y);

			m_anchoredMousePos = mousePos;
		}

		pMouse->SetHardwareMousePosition(m_anchoredMousePos.x, m_anchoredMousePos.y);
	}
	else
	{
		if (m_anchoredMousePos != Vec2(0, 0))
			m_anchoredMousePos.zero();
	}

	if (m_select)
		m_mouseDownDurationSec += frametime;
	else
		m_mouseDownDurationSec = 0.0f;

	MouseProjectToWorld(m_mouseRay, m_worldMousePos, m_mouseRayEntityFlags);
	m_worldProjectedMousePos = m_mouseRay.pt;
	//if (!m_altModifier)
	//{
	//	auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	//	if (pEntity && m_worldProjectedMousePos.z < pEntity->GetWorldPos().z)
	//	{
	//		m_worldProjectedMousePos.z = pEntity->GetWorldPos().z;
	//	}
	//}

	// Обработка таймера задержки перемещения выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_dragging && m_draggingMoveStartTimer > 0.0f)
		m_draggingMoveStartTimer -= frametime;

	if (m_draggingMoveStartTimer <= 0.0f)
		m_draggingMoveStartTimer = 0.0f;

	pe_status_dynamics zeus_dyn;
	const auto pZeusPhys = m_zeus->GetEntity()->GetPhysics();
	if (pZeusPhys)
		pZeusPhys->GetStatus(&zeus_dyn);

	const bool zeusMoving = zeus_dyn.v.len() > 0.1f;

	//Перемещение выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_dragging && !zeusMoving && m_draggingMoveStartTimer == 0.0f)
	{
		const bool autoEntitiesHeight = TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_entities_auto_height", 1); // Расчет высоты для каждой сущности отдельно
		const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);

		m_mouseRayEntityFlags = DRAGGING_MOUSE_ENT_FLAGS;
		m_draggingDelta = m_worldProjectedMousePos - m_clickedSelectStartPos;

		auto it = m_selectedEntities.begin();
		auto end = m_selectedEntities.end();
		while (it != end)
		{
			auto pEntity = TOS_GET_ENTITY(*it);
			if (pEntity)
			{
				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
				if (pActor)
				{
					// Пропускаем убитых актеров
					if (TOS_Console::GetSafeIntVar("tos_sv_zeus_can_drag_dead_bodies", 0) < 1 && pActor->GetHealth() < 0.0f)
						continue;
				}

				Matrix34 mat34 = pEntity->GetWorldTM();
				const Vec3 curPos = mat34.GetTranslation();
				const Vec3 startPos = m_selectStartEntitiesPositions[*it];

				if (!m_shiftModifier)
				{
					Vec3 newPos;
					if (m_altModifier)
					{
						// Перемещение по Z
						float height = startPos.z + m_draggingDelta.z;
						m_storedEntitiesPositions[*it].z = height;

						newPos = Vec3(curPos.x, curPos.y, height);
					}
					else
					{
						// Перемещение по X, Y с автоприлипанием к поверхности
						newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, m_storedEntitiesPositions[*it].z);

						if (autoEntitiesHeight)
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

					mat34.SetTranslation(newPos);
					pEntity->SetWorldTM(mat34);
				}
				else
				{
					mat34.SetTranslation(Vec3(curPos.x, curPos.y, m_storedEntitiesPositions[*it].z));
					pEntity->SetWorldTM(mat34);

					Vec3 toMouseDir = (m_worldProjectedMousePos - curPos).GetNormalizedSafe();
					Quat rot = Quat::CreateRotationVDir(toMouseDir);
					pEntity->SetRotation(rot);
				}
			}
			it++;
		}
	}
	else
	{
		m_mouseRayEntityFlags = DEFAULT_MOUSE_ENT_FLAGS;
	}

	// Отрисовка границ выделения
	///////////////////////////////////////////////////////////////////////
	if (m_select && CanSelectMultiplyWithBox() && !m_dragging)
	{
		if (IRenderAuxGeom* pGeom = gEnv->pRenderer->GetIRenderAuxGeom())
		{
			//calculate the four selection boundary points
			Vec3 vTopLeft(m_selectStartPos.x, m_selectStartPos.y, 0.0f);
			Vec3 vTopRight(m_mouseIPos.x, m_selectStartPos.y, 0.0f);
			Vec3 vBottomLeft(m_selectStartPos.x, m_mouseIPos.y, 0.0f);
			Vec3 vBottomRight(m_mouseIPos.x, m_mouseIPos.y, 0.0f);

			gEnv->pRenderer->Set2DMode(true, gEnv->pRenderer->GetWidth(), gEnv->pRenderer->GetHeight());

			//set boundary color: white
			ColorB col(255, 255, 255, 255);

			pGeom->DrawLine(vTopLeft, col, vTopRight, col);
			pGeom->DrawLine(vTopRight, col, vBottomRight, col);
			pGeom->DrawLine(vTopLeft, col, vBottomLeft, col);
			pGeom->DrawLine(vBottomLeft, col, vBottomRight, col);

			gEnv->pRenderer->Set2DMode(false, 0, 0);
		}
	}

	// Отрисовка флэш иконок под сущностями
	///////////////////////////////////////////////////////////////////////
	if (m_updateIconOnScreenTimer > 0.0f)
		m_updateIconOnScreenTimer -= frametime;

	if (m_updateIconOnScreenTimer <= 0.0f)
	{
		m_updateIconOnScreenTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_update_delay", 0.1f);

		IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
		if (!pClientActor)
			return;

		IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
		while (!pIt->IsEnd())
		{
			int color = eFTI_Grey;
			int icon = eZSI_Base;

			if (IEntity* pEntity = pIt->Next())
			{
				if (pEntity->IsGarbage())
					continue;

				if (pEntity->IsHidden())
					continue;

				if (!IsPhysicsAllowed(pEntity))
					continue;

				// пропускаем сущность актера клиента
				if (pEntity->GetId() == pClientActor->GetEntityId())
					continue;

				AABB worldBounds;
				pEntity->GetWorldBounds(worldBounds);

				//skip further calculations if the entity is not visible at all...
				if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
					continue;

				IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pEntity->GetId());
				if (pVehicle)
				{
					//TODO Танк, VTOL!
					auto movType = pVehicle->GetMovement()->GetMovementType();
					if (movType == IVehicleMovement::eVMT_Land)
						icon = eZSI_Car;
					else if (movType == IVehicleMovement::eVMT_Air)
						icon = eZSI_Helicopter;
					else if (movType == IVehicleMovement::eVMT_Sea || IVehicleMovement::eVMT_Amphibious)
						icon = eZSI_Boat;

					// не пустые тс должны быть желтые
					if (pVehicle->GetStatus().passengerCount > 0)
						color = eFTI_Yellow;

					if (pVehicle->IsDestroyed())
						color = eFTI_Grey;
				}

				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
				if (pActor)
				{
					icon = eZSI_Unit;

					if (pActor->GetHealth() > 0)
						color = eFTI_Yellow;
				}

				IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pEntity->GetId());
				if (pItem)
				{
					icon = eZSI_Circle;
				}

				HUDUpdateZeusUnitIcon(pEntity->GetId(), color, icon, Vec3(0, 0, 0));
			}
		}
	}

	// Отрисовка квадрата выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	for (auto it = m_selectedEntities.cbegin(); it != m_selectedEntities.cend(); it++)
	{
		const auto pEntity = TOS_GET_ENTITY(*it);
		if (!pEntity)
			continue;

		IRenderAuxGeom* pRAG = gEnv->pRenderer->GetIRenderAuxGeom();
		pRAG->SetRenderFlags(e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone);

		AABB entityBox;
		pEntity->GetLocalBounds(entityBox);
		OBB box;
		box.SetOBBfromAABB(pEntity->GetWorldRotation(), entityBox);

		//pRAG->DrawAABB(entityBox, false, ColorB(0, 0, 255, 65), eBBD_Extremes_Color_Encoded);
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawOBB(box, pEntity->GetWorldPos(), false, ColorB(255, 255, 255, 255), eBBD_Faceted);
	}

	// Отрисовка отладки
	///////////////////////////////////////////////////////////////////////
	UpdateDebug(zeusMoving, zeus_dyn.v);
}

void CTOSZeusModule::UpdateDebug(bool zeusMoving, const Vec3& zeusDynVec)
{
	IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
	pPD->Begin("ZeusModule", true);
	const auto blue = ColorF(0, 0, 1, 1);
	const auto green = ColorF(0, 1, 0, 1);
	const auto red = ColorF(1, 0, 0, 1);

	// Отрисовка отладки позиции мыши в реальном мире
	///////////////////////////////////////////////////////////////////////
	pPD->AddSphere(m_worldProjectedMousePos, 0.20f, blue, 1.0f);

	if (TOS_Console::GetSafeIntVar("tos_cl_zeus_dragging_draw_debug", 0) > 0 && m_dragging)
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
	float color[] = {1,1,1,1};
	const int startY = 100;
	const int deltaY = 20;

	const auto pLastClickedEntity = TOS_GET_ENTITY(m_lastClickedEntityId);
	const auto pCurrClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 0, 1.3f, color, false, "lastClickedEntity = %s", pLastClickedEntity ? pLastClickedEntity->GetName() : "");
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 1, 1.3f, color, false, "currClickedEntity  = %s", pCurrClickedEntity ? pCurrClickedEntity->GetName() : "");

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 2, 1.3f, color, false, "m_draggingMoveStartTimer = %f", m_draggingMoveStartTimer);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 3, 1.3f, color, false, "m_mouseIPos = (%i, %i)", m_mouseIPos.x, m_mouseIPos.y);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 4, 1.3f, color, false, "m_mouseDownDurationSec = %1.f", m_mouseDownDurationSec);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 5, 1.3f, color, false, "m_select = %i", int(m_select));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 6, 1.3f, color, false, "m_dragging = %i", int(m_dragging));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 7, 1.3f, color, false, "m_ctrlModifier = %i", int(m_ctrlModifier));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 8, 1.3f, color, false, "m_debugZModifier = %i", int(m_debugZModifier));

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 9, 1.3f, color, false, "zeusDynVec = (%1.f,%1.f,%1.f)", zeusDynVec.x, zeusDynVec.y, zeusDynVec.z);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 10, 1.3f, color, false, "zeusMoving = %i", zeusMoving);

	if (!m_selectedEntities.empty())
	{
		TOS_Debug::DrawEntitiesName2DLabel(m_selectedEntities, "Selected Entities: ", 100, startY + deltaY * 11, deltaY);
	}
}

void CTOSZeusModule::Serialize(TSerialize ser)
{

}

void CTOSZeusModule::NetMakePlayerZeus(IActor* pPlayer)
{
	if (!pPlayer)
		return;

	if (m_zeus)
		return;

	m_zeus = static_cast<CTOSPlayer*>(pPlayer);
	ApplyZeusProperties(m_zeus);
}

void CTOSZeusModule::ShowHUD(bool show)
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

void CTOSZeusModule::SetZeusFlag(uint flag, bool value)
{
	m_zeusFlags = value ? (m_zeusFlags | flag) : (m_zeusFlags & ~flag);
}

bool CTOSZeusModule::GetZeusFlag(uint flag) const
{
	return (m_zeusFlags & flag) != 0;
}

void CTOSZeusModule::ShowMouse(bool show)
{
	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		show ? pMouse->IncrementCounter() : pMouse->DecrementCounter();
		pMouse->ConfineCursor(show);
	}

	//auto pHUD = g_pGame->GetHUD();
	//if (pHUD)
	//{
	//	show ? pHUD->CursorIncrementCounter() : pHUD->CursorDecrementCounter();
	//}
}

void CTOSZeusModule::ApplyZeusProperties(IActor* pPlayer)
{
	auto pTOSPlayer = static_cast<CTOSPlayer*>(pPlayer);

	// Сбрасываем статы
	pTOSPlayer->GetActorStats()->inAir = 0.0f;
	pTOSPlayer->GetActorStats()->onGround = 0.0f;

	if (gEnv->bServer)
	{
		// Становимся неуязвимым к урону
		pTOSPlayer->m_isZeus = true;
		pTOSPlayer->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}

	// Отбираем оружие
	IInventory* pInventory = pTOSPlayer->GetInventory();
	if (pInventory)
	{
		pInventory->HolsterItem(true);
		pInventory->RemoveAllItems();
		TOS_Inventory::GiveItem(pTOSPlayer, "NightVision", false, false, false);
	}

	// Скрываем игрока
	if (gEnv->bClient)
		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
	else if (gEnv->bServer)
		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients | eRMI_NoLocalCalls);

	pTOSPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

	// Режим полета со столкновениями
	pTOSPlayer->SetFlyMode(1);

	// убираем нанокостюм
	CNanoSuit* pSuit = pTOSPlayer->GetNanoSuit();
	if (pSuit)
	{
		pSuit->SetMode(NANOMODE_DEFENSE);
		pSuit->SetModeDefect(NANOMODE_CLOAK, true);
		pSuit->SetModeDefect(NANOMODE_SPEED, true);
		pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
	}

	if (pTOSPlayer->IsClient())
	{
		// Убираем лишние действия
		g_pGameActions->FilterZeus()->Enable(true);

		// Скрываем HUD
		ShowHUD(false);
	}

	// Становимся невидимым для ИИ
	auto pAI = pTOSPlayer->GetEntity()->GetAI();
	if (pAI)
	{
		TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);
	}

	pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
	pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "CTOSZeusModule::ApplyZeusProperties");

	//Включаем мышь
	ShowMouse(true);
}

bool CTOSZeusModule::ExecuteCommand(EZeusCommands command)
{
	bool needUpdateIter = true;

	auto it = m_selectedEntities.begin();
	auto end = m_selectedEntities.end();
	while (it != end)
	{
		EntityId id = *it;
		IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
		//IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);

		switch (command)
		{
			case eZC_KillSelected:
			{
				string hitType = "normal";

				HitInfo info;
				info.SetDamage(99999.0f);
				info.targetId = id;
				info.type = g_pGame->GetGameRules()->GetHitTypeId(hitType.c_str());

				g_pGame->GetGameRules()->ClientHit(info);

				if (pVehicle)
					TOS_Vehicle::Destroy(pVehicle);

				break;
			}
			case eZC_RemoveSelected:
			{
				gEnv->pEntitySystem->RemoveEntity(id);
				it = m_selectedEntities.erase(it);
				needUpdateIter = false;

				break;
			}
			default:
				break;
		}

		if (needUpdateIter)
			it++;
	}


	return true;
}
