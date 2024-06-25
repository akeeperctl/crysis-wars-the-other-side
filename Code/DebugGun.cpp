/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 09:01:2006   14:00 : Created by Michael Rauh

*************************************************************************/
#include "StdAfx.h"
#include "DebugGun.h"

#include <IActorSystem.h>
#include <IVehicleSystem.h>

#include "Actor.h"
#include "Game.h"
#include "GameCVars.h"

const float HIT_RANGE = 2000.0f;

//------------------------------------------------------------------------
CDebugGun::CDebugGun()
{
	m_pAIDebugDraw    = gEnv->pConsole->GetCVar("ai_DebugDraw");
	m_aiDebugDrawPrev = m_pAIDebugDraw->GetIVal();
	m_fireMode        = 0;

	for (int i = 15; i >= 0; --i)
	{
		m_fireModes.push_back( std::make_pair("pierceability", static_cast<float>(i)) );
	}
}

//------------------------------------------------------------------------
void CDebugGun::OnAction(const EntityId actorId, const ActionId& actionId, const int activationMode, const float value)
{
	if (actionId == "attack1")
	{
		if (activationMode == eAAM_OnPress)
			Shoot(true);
	}
	else if (actionId == "zoom")
	{
		if (activationMode == eAAM_OnPress)
			Shoot(false);
	}
	else if (actionId == "firemode")
	{
		++m_fireMode;

		if (m_fireMode == m_fireModes.size())
			m_fireMode = 0;

		//SGameObjectEvent evt("HUD_TextMessage", eGOEF_ToAll, IGameObjectSystem::InvalidExtensionID, (void*)m_fireModes[m_fireMode].c_str());
		//SendHUDEvent(evt);
	}
	else
	{
		CWeapon::OnAction(actorId, actionId, activationMode, value);
	}
}

//------------------------------------------------------------------------
void CDebugGun::Update(SEntityUpdateContext& ctx, int update)
{
	if (!IsSelected())
		return;

	static float           drawColor[4] = {1, 1, 1, 1};
	static const float fontLarge    = 1.4f;

	IRenderer*      pRenderer = gEnv->pRenderer;
	IRenderAuxGeom* pAuxGeom  = pRenderer->GetIRenderAuxGeom();
	pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);

	pRenderer->Draw2dLabel(pRenderer->GetWidth() / 5.f, pRenderer->GetHeight() - 35, fontLarge, drawColor, false, "Firemode: %s (%.1f)", m_fireModes[m_fireMode].first.c_str(), m_fireModes[m_fireMode].second);

	ray_hit rayhit;
	int     hits = 0;

	unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
	if (m_fireModes[m_fireMode].first == "pierceability")
		flags = static_cast<unsigned int>(m_fireModes[m_fireMode].second) & rwi_pierceability_mask;

	// use cam, no need for firing pos/dir
	const CCamera& cam = GetISystem()->GetViewCamera();

	if (hits = gEnv->pPhysicalWorld->RayWorldIntersection(cam.GetPosition() + cam.GetViewdir(), cam.GetViewdir() * HIT_RANGE, ent_all, flags, &rayhit, 1))
	{
		static const float font = 1.2f;
		static const int   dx   = 5;
		static const int   dy   = 15;

		IMaterialManager*    pMatMan        = gEnv->p3DEngine->GetMaterialManager();
		IActorSystem*        pActorSystem   = g_pGame->GetIGameFramework()->GetIActorSystem();
		IVehicleSystem*      pVehicleSystem = g_pGame->GetIGameFramework()->GetIVehicleSystem();

		const int x = static_cast<int>(pRenderer->GetWidth() * 0.5f) + dx;
		int       y = static_cast<int>(pRenderer->GetHeight() * 0.5f) + dx - dy;

		// draw normal
		const ColorB colNormal(200, 0, 0, 128);
		const Vec3   end = rayhit.pt + 0.75f * rayhit.n;
		pAuxGeom->DrawLine(rayhit.pt, colNormal, end, colNormal);
		pAuxGeom->DrawCone(end, rayhit.n, 0.1f, 0.2f, colNormal);

		const IEntity* pEntity = static_cast<IEntity*>(rayhit.pCollider->GetForeignData(PHYS_FOREIGN_ID_ENTITY));
		if (pEntity)
			pRenderer->Draw2dLabel(x, y += dy, fontLarge, drawColor, false, pEntity->GetName());

		// material
		const char* matName = pMatMan->GetSurfaceType(rayhit.surface_idx)->GetName();

		if (matName[0])
			pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%s (%i)", matName, rayhit.surface_idx);

		pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.1f m", rayhit.dist);

		if (pEntity)
		{
			IScriptTable* pScriptTable = pEntity->GetScriptTable();

			// physics 
			if (IPhysicalEntity* pPhysEnt = pEntity->GetPhysics())
			{
				pe_status_dynamics status;
				if (pPhysEnt->GetStatus(&status))
				{
					if (status.mass > 0.f)
						pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.1f kg", status.mass);

					pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "pe_type: %i", pPhysEnt->GetType());

					if (status.submergedFraction > 0.f)
						pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.2f submerged", status.submergedFraction);

					if (status.v.len2() > 0.0001f)
						pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.2f m/s", status.v.len());
				}
			}

			if (pScriptTable)
			{
				HSCRIPTFUNCTION func = nullptr;
				if (pScriptTable->GetValue("GetFrozenAmount", func) && func)
				{
					float frozen = 0.f;
					Script::CallReturn(gEnv->pScriptSystem, func, pScriptTable, frozen);
					gEnv->pScriptSystem->ReleaseFunc(func);

					if (frozen > 0.f)
						pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "Frozen: %.2f", frozen);
				}
			}

			// class-specific stuff
			if (const IActor* pActor = pActorSystem->GetActor(pEntity->GetId()))
			{
				pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%i health", pActor->GetHealth());
			}
			else if (IVehicle* pVehicle = pVehicleSystem->GetVehicle(pEntity->GetId()))
			{
				const SVehicleStatus& status = pVehicle->GetStatus();

				pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.0f%% health", 100.f * status.health);
				pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%i passengers", status.passengerCount);

				if (pVehicle->GetMovement() && pVehicle->GetMovement()->IsPowered())
					pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "Running");
			}
			else
			{
				if (pScriptTable)
				{
					HSCRIPTFUNCTION func = nullptr;
					if (pScriptTable->GetValue("GetHealth", func) && func)
					{
						float health = 0.f;
						if (Script::CallReturn(gEnv->pScriptSystem, func, pScriptTable, health))
							pRenderer->Draw2dLabel(x, y += dy, font, drawColor, false, "%.0f health", health);
						gEnv->pScriptSystem->ReleaseFunc(func);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CDebugGun::Shoot(const bool bPrimary)
{
	CWeapon::StartFire();

	ResetAnimation();

	// console cmd      

	string cmd = (bPrimary) ? g_pGameCVars->i_debuggun_1->GetString() : g_pGameCVars->i_debuggun_2->GetString();
	cmd += " ";

	unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;

	if (m_fireModes[m_fireMode].first == "pierceability")
		flags = static_cast<unsigned int>(m_fireModes[m_fireMode].second) & rwi_pierceability_mask;

	IPhysicalWorld*  pWorld = gEnv->pPhysicalWorld;
	IPhysicalEntity* pSkip  = GetOwnerActor()->GetEntity()->GetPhysics();
	ray_hit          rayhit;
	int              hits = 0;

	const CCamera& cam = GetISystem()->GetViewCamera();
	const Vec3     pos = cam.GetPosition() + cam.GetViewdir();
	const Vec3     dir = cam.GetViewdir() * HIT_RANGE;

	const IEntity* pEntity = nullptr;

	if (hits = pWorld->RayWorldIntersection(pos, dir, ent_all, flags, &rayhit, 1, &pSkip, 1))
		pEntity = static_cast<IEntity*>(rayhit.pCollider->GetForeignData(PHYS_FOREIGN_ID_ENTITY));

	cmd.append(pEntity ? pEntity->GetName() : "0");

	// if we execute an AI command take care of ai_debugdraw
	if (cmd.substr(0, 3) == "ai_")
	{
		if (pEntity && m_pAIDebugDraw->GetIVal() == 0)
			m_pAIDebugDraw->Set(1);
		else if (!pEntity && m_aiDebugDrawPrev == 0 && m_pAIDebugDraw->GetIVal() == 1)
			m_pAIDebugDraw->Set(0);
	}

	gEnv->pConsole->ExecuteString(cmd.c_str());

	// if 2nd button hits a vehicle, enable movement profiling  
	if (!bPrimary)
	{
		static IVehicleSystem* pVehicleSystem = g_pGame->GetIGameFramework()->GetIVehicleSystem();

		string vehicleCmd = "v_debugVehicle ";
		vehicleCmd.append((pEntity && pVehicleSystem->GetVehicle(pEntity->GetId())) ? pEntity->GetName() : "0");

		gEnv->pConsole->ExecuteString(vehicleCmd.c_str());
	}

	OnShoot(GetOwnerId(), 0, nullptr, pos, dir, Vec3(ZERO));
}

//------------------------------------------------------------------------
void CDebugGun::Select(const bool select)
{
	CWeapon::Select(select);

	// save ai_debugDraw val
	if (select)
		m_aiDebugDrawPrev = m_pAIDebugDraw->GetIVal();
}
