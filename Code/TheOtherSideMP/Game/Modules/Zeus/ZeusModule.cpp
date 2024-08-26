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
#include <Cry_Camera.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>

CTOSZeusModule::CTOSZeusModule()
	: m_zeus(nullptr),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO),
	m_worldMousePos(ZERO),
	m_worldProjectedMousePos(ZERO),
	m_selectStartPos(ZERO),
	m_selectStopPos(ZERO),
	m_mouseIPos(ZERO),
	m_lastClickedEntityId(0),
	m_curClickedEntityId(0),
	m_mouseDownDurationSec(0),
	m_ctrlModifier(false),
	m_select(false)
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

			if (CanSelectMultiplyWithBox())
				GetSelectedEntities();
			else
			{
				if (!m_ctrlModifier && m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0)
					m_selectedEntities.clear();

				if (m_curClickedEntityId != 0)
					stl::push_back_unique(m_selectedEntities, m_curClickedEntityId);
			}
		}
	}
}

void CTOSZeusModule::GetSelectedEntities()
{
	//clear previously stored entity id's if left or right CTRL is not pressed
	if (!m_ctrlModifier)
		m_selectedEntities.clear();

	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIt->IsEnd())
	{
		if (IEntity* pEntity = pIt->Next())
		{
			//skip useless entities (gamerules, fists etc.)
			if (IPhysicalEntity* physEnt = pEntity->GetPhysics())
			{
				IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();

				if (!pClientActor)
					return;

				//skip the client actor entity
				if (physEnt == pClientActor->GetEntity()->GetPhysics())
					continue;

				AABB worldBounds;
				pEntity->GetWorldBounds(worldBounds);

				//skip further calculations if the entity is not visible at all...
				if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
					continue;

				Vec3 wpos = pEntity->GetWorldPos();
				Quat rot = pEntity->GetWorldRotation();
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
						if (!m_ctrlModifier)
						{
							m_selectedEntities.push_back(pEntity->GetId());
						}
						else
						{
							//check for previously added entity id's first
							stl::push_back_unique(m_selectedEntities, pEntity->GetId());
						}
					}
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
				m_ctrlModifier = false;
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

void CTOSZeusModule::Update(float frametime)
{
	if (!m_zeus)
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

	m_select ? m_mouseDownDurationSec += frametime : m_mouseDownDurationSec = 0.0f;

	if (m_zeus->IsClient())
	{
		const auto pPhys = m_zeus->GetEntity()->GetPhysics();
		const Vec3 camWorldPos = gEnv->pSystem->GetViewCamera().GetPosition();
		const Vec3 camToMouseDir = (m_worldMousePos - camWorldPos).GetNormalizedSafe() * 5000;
		const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
		const unsigned entityFlags = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;
		ray_hit ray;

		int hitNum = gEnv->pPhysicalWorld->RayWorldIntersection(m_worldMousePos, camToMouseDir, entityFlags, rayFlags, &ray, 1, pPhys, 0);
		if (hitNum)
		{
			m_worldProjectedMousePos = ray.pt;
		}

		// Отрисовка позиции мыши в реальном мире
		///////////////////////////////////////////////////////////////////////
		IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin("ZeusModule", true);
		const auto blue = ColorF(0, 0, 1, 1);

		//pPD->AddSphere(m_worldMousePos, 0.25f, green, 1.0f);
		pPD->AddSphere(m_worldProjectedMousePos, 0.20f, blue, 1.0f);

		// Отрисовка границ выделенной сущности
		///////////////////////////////////////////////////////////////////////
		//if (m_lastClickedEntityId != 0)
		//{
		//	IRenderAuxGeom* pRAG = gEnv->pRenderer->GetIRenderAuxGeom();
		//	pRAG->SetRenderFlags(e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone);

		//	AABB entityBox;
		//	auto pEntity = TOS_GET_ENTITY(m_lastClickedEntityId);
		//	pEntity->GetWorldBounds(entityBox);
		//	pRAG->DrawAABB(entityBox, true, ColorB(0, 0, 255, 65), eBBD_Faceted);
		//}

		// Отрисовка границ выделения
		///////////////////////////////////////////////////////////////////////
		if (m_select && CanSelectMultiplyWithBox())
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

		// Отрисовка границ выделенных сущностей
		///////////////////////////////////////////////////////////////////////
		auto it = m_selectedEntities.cbegin();
		auto end = m_selectedEntities.cend();
		for (; it != end; it++)
		{
			auto pEntity = TOS_GET_ENTITY(*it);
			if (!pEntity)
				continue;

			IRenderAuxGeom* pRAG = gEnv->pRenderer->GetIRenderAuxGeom();
			pRAG->SetRenderFlags(e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone);

			AABB entityBox;
			pEntity->GetWorldBounds(entityBox);
			pRAG->DrawAABB(entityBox, true, ColorB(0, 0, 255, 65), eBBD_Faceted);
		}

		// Отрисовка границ выделенных сущностей
		///////////////////////////////////////////////////////////////////////

		float color[] = {1,1,1,1};
		gEnv->pRenderer->Draw2dLabel(100, 100, 1.2f, color, false, "m_ctrlModifier = %i", int(m_ctrlModifier));
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
