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

const uint DEFAULT_MOUSE_ENT_FLAGS = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;
const uint DRAGGING_MOUSE_ENT_FLAGS = ent_static | ent_terrain;

CTOSZeusModule::CTOSZeusModule()
	: m_zeus(nullptr),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO),
	m_worldMousePos(ZERO),
	m_worldProjectedMousePos(ZERO),
	m_selectStartPos(ZERO),
	m_worldProjectedSelectStartPos(ZERO),
	m_selectStopPos(ZERO),
	m_mouseIPos(ZERO),
	m_draggingDelta(ZERO),
	m_lastClickedEntityId(0),
	m_curClickedEntityId(0),
	m_mouseDownDurationSec(0),
	m_mouseRayEntityFlags(DEFAULT_MOUSE_ENT_FLAGS),
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
				m_altModifier = true;
			else if (event.state == eIS_Released)
				m_altModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_Z)
		{
			if (event.state == eIS_Pressed)
				m_debugZModifier = true;
			else if (event.state == eIS_Released)
				m_debugZModifier = false;
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

void CTOSZeusModule::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_mouseIPos.x = iX;
	m_mouseIPos.y = iY;

	auto mod_iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, mod_iY, 0.0f, &m_worldMousePos.x, &m_worldMousePos.y, &m_worldMousePos.z);

	auto pHUD = g_pGame->GetHUD();
	if (pHUD && !pHUD->IsHaveModalHUD())
	{
		if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOWN)
		{
			m_select = true;
			m_selectStartPos = Vec2i(iX, iY);

			// Сейчас позиция старта и позиция в мире мыши связаны
			const int hitNum = MouseProjectToWorld(m_mouseRay, m_worldMousePos, m_mouseRayEntityFlags);
			if (hitNum)
				m_worldProjectedSelectStartPos = m_mouseRay.pt;

			m_lastClickedEntityId = m_curClickedEntityId;
			m_curClickedEntityId = GetMouseEntityId();

			//const auto clickedEntityId = GetMouseEntityId();
			//if (clickedEntityId != m_lastClickedEntityId)
			//{
			//	const auto pSil = SAFE_HUD_FUNC_RET(GetSilhouettes());
			//	if (pSil)
			//	{
			//		auto pClickedEntity = TOS_GET_ENTITY(clickedEntityId);
			//		if (pClickedEntity)
			//		{
			//			pSil->SetSilhouette(pClickedEntity, 1.0f, 1.0f, 1.0f, 1.0f, 90000.0f);
			//		}

			//		pSil->ResetSilhouette(m_lastClickedEntityId);
			//	}
			//}

			//m_lastClickedEntityId = clickedEntityId;

		}
		else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONUP)
		{
			m_select = false;
			m_selectStopPos = Vec2i(iX, iY);

			if (CanSelectMultiplyWithBox() && !m_dragging)
				GetSelectedEntities();
			else
			{
				// Одиночное выделение
				if (!m_ctrlModifier)
				{
					// При каждом клике без CTRL снимаем выделение если кликнули не на последнюю кликнутую сущность
					if (m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0)
						m_selectedEntities.clear();

					// После чистки всех выделенных сущностей, выделяем кликнутую сущность
					if (m_curClickedEntityId != 0)
						m_selectedEntities.insert(m_curClickedEntityId);
				}
				// Множественное выделение
				else
				{
					if (m_curClickedEntityId != 0 && !m_dragging)
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

			m_dragging = false;
		}
		else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_MOVE)
		{
			// При перетаскивании кликнутая сущность должна быть выделена
			const auto iter = stl::binary_find(m_selectedEntities.begin(), m_selectedEntities.end(), m_curClickedEntityId);
			const bool clickedSelected = iter != m_selectedEntities.end();

			if (m_select && m_curClickedEntityId != 0 && clickedSelected)
			{
				// Сохраняем начальное положение каждой выделенной сущности
				if (m_dragging == false)
				{
					m_selectStartEntitiesPositions.clear();

					auto it = m_selectedEntities.begin();
					auto end = m_selectedEntities.end();
					while (it != end)
					{
						EntityId id = *it;

						auto pos = stl::find_in_map(m_selectStartEntitiesPositions, id, Vec3(ZERO));
						if (pos.IsZero())
						{
							auto pEntity = TOS_GET_ENTITY(id);
							if (pEntity)
							{
								m_selectStartEntitiesPositions[id] = pEntity->GetWorldPos();
							}
						}
						it++;
					}
				}
				m_dragging = true;
			}
		}
	}
}

void CTOSZeusModule::GetSelectedEntities()
{
	//clear previously stored entity id's if left CTRL is not pressed
	if (!m_ctrlModifier)
		m_selectedEntities.clear();

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

				//skip useless entities (gamerules, fists etc.)
				IPhysicalEntity* physEnt = pEntity->GetPhysics();
				if (!physEnt)
					continue;

				// допустимые сущности
				const auto type = physEnt->GetType();
				if (!(type == PE_LIVING || type == PE_RIGID || type == PE_STATIC || type == PE_WHEELEDVEHICLE))
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

			//check if the projected bounding box min max values are fully or partly inside the screen selection 
			if ((m_selectStartPos.x <= pointsProjected[0].x && pointsProjected[0].x <= m_selectStopPos.x) ||
				(m_selectStartPos.x >= pointsProjected[0].x && m_selectStopPos.x <= pointsProjected[1].x) ||
				(m_selectStartPos.x <= pointsProjected[1].x && m_selectStopPos.x >= pointsProjected[1].x) ||
				(m_selectStartPos.x <= pointsProjected[0].x && m_selectStopPos.x >= pointsProjected[1].x))
			{
				if ((m_selectStartPos.y <= pointsProjected[0].y && m_selectStopPos.y >= pointsProjected[0].y) ||
					(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[0].y) ||
					(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[1].y))
				{
					//finally we have an entity id
					//if left or right CTRL is not pressed we can directly add every entity id, old entity id's are already deleted
					m_selectedEntities.insert(pEntity->GetId());
				}
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
				m_zeus->m_isZeus = false;
				m_zeus = nullptr;
				m_zeusFlags = 0;
				m_select = false;
				m_dragging = false;
				m_ctrlModifier = false;
				m_altModifier = false;
				m_debugZModifier = false;
				m_selectedEntities.clear();

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

void CTOSZeusModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CTOSZeusModule::Init()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->AddListener(this);
}

int CTOSZeusModule::MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags)
{
	if (!m_zeus || !m_zeus->GetEntity() || !m_zeus->GetEntity()->GetPhysics())
		return 0;

	const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);

	IPhysicalEntity* pSkipEnts[2] = {m_zeus->GetEntity()->GetPhysics(), nullptr};
	if (pClickedEntity)
		pSkipEnts[1] = pClickedEntity->GetPhysics();

	const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
	const Vec3 camWorldPos = gEnv->pSystem->GetViewCamera().GetPosition();
	const Vec3 camToMouseDir = (mouseWorldPos - camWorldPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();
	const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);

	return gEnv->pPhysicalWorld->RayWorldIntersection(mouseWorldPos, camToMouseDir, entityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
}

void CTOSZeusModule::Update(float frametime)
{
	if (!m_zeus || !m_zeus->IsClient())
		return;

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

	pe_status_dynamics zeus_dyn;
	const auto pZeusPhys = m_zeus->GetEntity()->GetPhysics();
	if (pZeusPhys)
		pZeusPhys->GetStatus(&zeus_dyn);

	const bool zeusMoving = zeus_dyn.v.len() > 0.1f;

	//Перенос выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_dragging && !zeusMoving)
	{
		const bool individualEntHeight = true; // Расчет высоты для каждой сущности отдельно
		const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);

		m_mouseRayEntityFlags = DRAGGING_MOUSE_ENT_FLAGS;
		m_draggingDelta = m_worldProjectedMousePos - m_worldProjectedSelectStartPos;

		auto it = m_selectedEntities.begin();
		auto end = m_selectedEntities.end();
		while (it != end)
		{
			auto pEntity = TOS_GET_ENTITY(*it);
			if (pEntity)
			{
				Matrix34 mat34 = pEntity->GetWorldTM();
				const Vec3 curPos = mat34.GetTranslation();
				const Vec3 startPos = m_selectStartEntitiesPositions[*it];

				Vec3 newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, m_mouseRay.pt.z);

				if (individualEntHeight)
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

				mat34.SetTranslation(newPos);
				pEntity->SetWorldTM(mat34);
			}
			it++;
		}
	}
	else
	{
		m_mouseRayEntityFlags = DEFAULT_MOUSE_ENT_FLAGS;
	}

	const int hitNum = MouseProjectToWorld(m_mouseRay, m_worldMousePos, m_mouseRayEntityFlags);
	if (hitNum)
	{
		m_worldProjectedMousePos = m_mouseRay.pt;
	}

	// Отрисовка отладки
	///////////////////////////////////////////////////////////////////////
	IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
	pPD->Begin("ZeusModule", true);
	const auto blue = ColorF(0, 0, 1, 1);
	const auto green = ColorF(0, 1, 0, 1);
	const auto red = ColorF(1, 0, 0, 1);

	// Отрисовка отладки позиции мыши в реальном мире
	///////////////////////////////////////////////////////////////////////
	//pPD->AddSphere(m_worldMousePos, 0.25f, green, 1.0f);
	pPD->AddSphere(m_worldProjectedMousePos, 0.20f, blue, 1.0f);
	pPD->AddSphere(m_worldProjectedSelectStartPos, 0.20f, green, 1.0f);

	if (TOS_Console::GetSafeIntVar("tos_cl_zeus_dragging_draw_debug", 0) > 0 && m_dragging)
	{
		pPD->AddDirection(m_worldProjectedSelectStartPos, 1.0f, m_worldProjectedMousePos - m_worldProjectedSelectStartPos, green, 1.0f);

		const auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
		if (pEntity)
		{
			const Vec3 curPos = pEntity->GetWorldPos();
			const Vec3 newPos = Vec3(curPos.x + m_draggingDelta.x, curPos.y + m_draggingDelta.y, curPos.z);
			pPD->AddSphere(newPos, 0.20f, red, 1.0f);
			pPD->AddLine(curPos, newPos, red, 1.0f);
		}

		pPD->AddLine(m_worldProjectedSelectStartPos, m_worldProjectedMousePos, green, 1.0f);
		pPD->AddText(m_mouseIPos.x, m_mouseIPos.y, 1.4f, green, 1.0f, "delta (%1.f, %1.f, %1.f)",
					 m_draggingDelta.x, m_draggingDelta.y, m_draggingDelta.z);
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
		pEntity->GetWorldBounds(entityBox);
		pRAG->DrawAABB(entityBox, true, ColorB(0, 0, 255, 65), eBBD_Faceted);
	}

	// Вывод текстовой отладки
	///////////////////////////////////////////////////////////////////////
	float color[] = {1,1,1,1};
	const int startY = 100;
	const int deltaY = 20;
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 0, 1.3f, color, false, "m_ctrlModifier = %i", int(m_ctrlModifier));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 1, 1.3f, color, false, "m_debugZModifier = %i", int(m_debugZModifier));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 2, 1.3f, color, false, "zeus_dyn.v = (%1.f,%1.f,%1.f)", zeus_dyn.v.x, zeus_dyn.v.y, zeus_dyn.v.z);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 3, 1.3f, color, false, "zeusMoving = %i", zeusMoving);

	if (!m_selectedEntities.empty())
	{
		TOS_Debug::DrawEntitiesName2DLabel(m_selectedEntities, "Selected Entities: ", 100, startY + deltaY * 4, deltaY);
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
