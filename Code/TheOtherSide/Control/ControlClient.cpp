#include <StdAfx.h>

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDScopes.h"
#include "HUD/HUDCrosshair.h"
#include "HUD/HUDSilhouettes.h"

#include "HUD/GameFlashAnimation.h"
#include "HUD/GameFlashLogic.h"
#include "Menus/FlashMenuObject.h"

#include "GameActions.h"
#include "GameCvars.h"
#include "GameUtils.h"

#include "Weapon.h"
#include "Single.h"

#include "Player.h"

#include "Alien.h"
#include "Scout.h"
#include "Trooper.h"
#include "Hunter.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"
//#include "SquadSystem.h"
//#include "AbilitiesSystem.h"

#include "IPlayerInput.h"

//SControlClient* g_pControlSystem = new SControlClient();

namespace
{
	bool IsFriendlyEntity(IEntity* pEntity, IActor* pTarget)
	{
		//Only for actors (not vehicles)
		if (pEntity && pEntity->GetAI() && pTarget)
		{
			if (!pEntity->GetAI()->IsHostile(pTarget->GetEntity()->GetAI(), false))
				return true;
			else
				return false;
		}

		//Special case (Animated objects), check for script table value "bFriendly"
		//Check script table (maybe is not possible to grab)
		if (pEntity)
		{
			SmartScriptTable props;
			IScriptTable* pScriptTable = pEntity->GetScriptTable();
			if (!pScriptTable || !pScriptTable->GetValue("Properties", props))
				return false;

			int isFriendly = 0;
			if (props->GetValue("bNoFriendlyFire", isFriendly) && isFriendly != 0)
				return true;
		}

		//for vehicles
		if (pEntity && pEntity->GetId())
		{
			IVehicle* pVehicle = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pEntity->GetId());
			if (pVehicle)
			{
				if (pTarget->GetEntity() && pVehicle->HasFriendlyPassenger(pTarget->GetEntity()))
					return true;
			}
		}

		return false;
	}
}

TActionHandler<SControlClient> SControlClient::s_actionHandler;

#define TORADIAN gf_PI / 180.0f

SControlClient::SControlClient()
	: m_pControlledActor(0),
	m_crosshairPos(0, 0, 0),
	m_finalFireTargetPos(0, 0, 0),
	m_mustBeamDude(0),
	m_mustHideDude(0),
	m_currentFov(60),

	m_fireTargetId(0),
	m_meleeTargetId(0),
	m_crosshairTargetId(0),
	m_lastCrosshairTargetId(0),

	m_lastDudeNanoMode(NANOMODE_DEFENSE),
	m_lastDudeRotation(ZERO),
	m_lastDudePosition(ZERO),
	m_lastDudeSpecies(0),

	m_pAbilitiesSystem(nullptr),
	m_isHitListener(0)
{
	if (!g_pControlSystem->pSquadSystem)
		g_pControlSystem->pSquadSystem = new SSquadSystem();

	if (!g_pControlSystem->pAbilitiesSystem)
		g_pControlSystem->pAbilitiesSystem = new CAbilitiesSystem();
	m_pAbilitiesSystem = g_pControlSystem->pAbilitiesSystem;

	CGameRules* pGR = g_pGame->GetGameRules();
	if (pGR && !m_isHitListener)
	{
		m_isHitListener = true;
		pGR->AddHitListener(this);
	}

	ResetParams();
}

SControlClient::~SControlClient()
{
	//Don't change the sequence of code lines!!!
	if (g_pGame->GetGameRules() && m_isHitListener)
	{
		g_pGame->GetGameRules()->RemoveHitListener(this);
		m_isHitListener = false;
	}
		
	m_pAbilitiesSystem = nullptr;
	//SAFE_DELETE(m_pSquadSystem);
}
//
void SControlClient::OnHit(const HitInfo& hitInfo)
{
	//Multiplayer: code run on the local client side

	//When The Player are controlling a actor
    if (m_pControlledActor && m_pControlledActor->GetHealth() > 0.1f) 
	{

		if (hitInfo.damage > 1.5f)
		{
			if (hitInfo.shooterId == m_pControlledActor->GetEntityId() && hitInfo.shooterId != hitInfo.targetId)
			{
				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId);
				IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(hitInfo.targetId);

				if ((pVehicle && !pVehicle->IsDestroyed()) || (pActor && pActor->GetHealth() > 0.1f))
				{
					CHUD* pHUD = g_pGame->GetHUD();
					if (pHUD)
						pHUD->IndicateHit();
				}
			}
		}
	}
	else
	{
		// When The Player are not controlling a actor

		IActor* pPlayer = g_pGame->GetIGameFramework()->GetClientActor();
		if (pPlayer)
		{
			if (hitInfo.damage > 1.5f)
			{
				if (hitInfo.shooterId == pPlayer->GetEntityId() && hitInfo.shooterId != hitInfo.targetId)
				{
					IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId);
					IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(hitInfo.targetId);

					if ((pVehicle && !pVehicle->IsDestroyed()) || (pActor && pActor->GetHealth() > 0.1f))
					{
						CHUD* pHUD = g_pGame->GetHUD();
						if (pHUD)
							pHUD->IndicateHit();
					}
				}
			}
		}
	}
}

void SControlClient::OnExplosion(const ExplosionInfo& expInfo)
{
	return;
}

void SControlClient::OnServerExplosion(const ExplosionInfo& expInfo)
{
	return;
}

void SControlClient::InitDudeToControl(bool bToLink)
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		CPlayer::TAlienInterferenceParams lastInterferenceParams;

		CNanoSuit* pSuit = pDude->GetNanoSuit();

		//Fix the non-resetted Dude player movement after controlling the actor;
		if (pDude->GetPlayerInput())
			pDude->GetPlayerInput()->Reset();

		//before link to the new actor
		if (bToLink)
		{
			m_lastDudePosition = pDude->GetEntity()->GetWorldPos();
			m_lastDudeRotation = pDude->GetViewRotation();

			if (gEnv->bServer)
			{
				IAIObject* pAI = pDude->GetEntity()->GetAI();
				if (pAI)
				{
					if (pAI->CastToIAIActor())
					{
						m_lastDudeSpecies = pAI->CastToIAIActor()->GetParameters().m_nSpecies;
						//CryLogAlways("SControlClient::PrepareDude -->> save player species");
					}

					pAI->Event(AIEVENT_DISABLE, 0);
				}
			}

			pDude->ResetScreenFX();

			if (pSuit)
			{
				m_lastDudeSuitEnergy = pSuit->GetSuitEnergy();
				m_lastDudeNanoMode = pSuit->GetMode();
				pSuit->SetMode(NANOMODE_DEFENSE);
				pSuit->SetModeDefect(NANOMODE_CLOAK, true);
				pSuit->SetModeDefect(NANOMODE_SPEED, true);
				pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
			}

			// Turning off a Alien screen interference effects
			lastInterferenceParams = pDude->m_interferenceParams;
			pDude->m_interferenceParams.clear();

			if (g_pGame->GetHUD())
			{
				//LoadHUD(true); deprecated
				//m_pAbilitiesSystem->InitHUD(true);			
				//m_pAbilitiesSystem->ShowHUD(true);
				//m_pAbilitiesSystem->UpdateHUD();
				//m_pAbilitiesSystem->ReloadHUD();

				//SetAmmoHealthHUD();

				//g_pGame->GetHUD()->UpdateHealth(m_pControlledActor);
				//g_pGame->GetHUD()->m_animPlayerStats.Reload(true);

				CHUDCrosshair* pCrosshair = g_pGame->GetHUD()->GetCrosshair();
				if (pCrosshair)
				{
					pCrosshair->SetOpacity(1.0f);
					pCrosshair->SetCrosshair(g_pGameCVars->hud_crosshair);
				}
			}

			if (!gEnv->bEditor)
			{
				//TODO: fix "Pure function error" 	

				/*CGameRules* pGR = g_pGame->GetGameRules();
				if (pGR && !m_isHitListener)
				{
					m_isHitListener = true;
					pGR->AddHitListener(this);
				}*/
			}		
		}
		else 
		{
			//after unlink

			//The Player after unlink
			{
				SActorParams* pParams = pDude->GetActorParams();

				pDude->GetEntity()->SetPos(m_lastDudePosition);

				//may be bugged, not checked at 19.12.2020 0:14
				pDude->SetViewRotation(m_lastDudeRotation);

				pDude->SetSlaveId(NULL);
				pDude->m_interferenceParams = lastInterferenceParams;
				pDude->ResetScreenFX();

				if (pDude->IsThirdPerson())
					pDude->ToggleThirdPerson();

				SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(false));
				SAFE_HUD_FUNC(SetWeaponName(""));

				//HUD
				//LoadHUD(false); deprecated
				//m_pAbilitiesSystem->ShowHUD(false);
				g_pControlSystem->GetSquadSystem()->RemoveHUD();

				SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pDude, 1);
				if (squad.GetLeader() != 0)
					squad.OnPlayerAdded();

				//Clean the OnUseData from player .lua script
				IScriptTable* pTable = pDude->GetEntity()->GetScriptTable();
				if (pTable)
				{
					ScriptAnyValue value = 0;
					Script::CallMethod(pTable, "SetOnUseData", value, value);
				}

				if (pSuit)
				{
					pSuit->Reset(pDude);

					pSuit->SetModeDefect(NANOMODE_CLOAK, false);
					pSuit->SetModeDefect(NANOMODE_SPEED, false);
					pSuit->SetModeDefect(NANOMODE_STRENGTH, false);

					pSuit->ActivateMode(NANOMODE_CLOAK, true);
					pSuit->ActivateMode(NANOMODE_SPEED, true);
					pSuit->ActivateMode(NANOMODE_STRENGTH, true);

					pSuit->SetSuitEnergy(m_lastDudeSuitEnergy);
					pSuit->SetMode(m_lastDudeNanoMode);

					if (g_pGame->GetHUD())
					{
						SetAmmoHealthHUD(pDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx");
						//g_pGame->GetHUD()->m_animPlayerStats.Load("Libs/UI/HUD_AmmoHealthEnergySuit.gfx", eFD_Right, eFAF_Visible | eFAF_ThisHandler);

						//const int currentHealth = pDude->GetHealth();
						//if (currentHealth)
						//	pDude->SetHealth(currentHealth);

						//g_pGame->GetHUD()->UpdateHealth(pDude);
						//g_pGame->GetHUD()->m_animPlayerStats.Reload(true);

						switch (pSuit->GetMode())
						{
						case NANOMODE_DEFENSE:
							g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Armor");
							break;
						case NANOMODE_SPEED:
							g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Speed");
							break;
						case NANOMODE_STRENGTH:
							g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Strength");
							break;
						default:
							g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Cloak");
							break;
						}
					}

					pParams->vLimitRangeH = 0;
					pParams->vLimitRangeV = pParams->vLimitRangeVDown = pParams->vLimitRangeVUp = 0;
				}

				if (gEnv->bServer)
				{
					IAIObject* pAI = pDude->GetEntity()->GetAI();
					if (pAI)
					{
						pAI->Event(AIEVENT_ENABLE, 0);
						SetDudeSpecies(m_lastDudeSpecies);
						//CryLogAlways("SControlClient::PrepareDude -->> restore player species");
					}
				}

				//Pure function error 2
				//I cant fix this error
				if (!gEnv->bEditor)
				{
					/*CGameRules* pGR = g_pGame->GetGameRules();
					if (pGR && m_isHitListener)
					{
						pGR->RemoveHitListener(this);
						m_isHitListener = false;
					}*/
				}
				
			}
		}
	}
}

void SControlClient::SetAmmoHealthHUD(IActor* pActor, const char* file)
{
	g_pGame->GetHUD()->m_animPlayerStats.Unload();
	g_pGame->GetHUD()->m_animPlayerStats.Load(file, eFD_Right, eFAF_Visible | eFAF_ThisHandler);

	float fHealth = (pActor->GetHealth() / (float)pActor->GetMaxHealth()) * 100.0f + 1.0f;
	float fEnergy = 0;

	if (CActor* pNewActor = static_cast<CActor*>(pActor))
	{
		if (pNewActor->IsAlien())
		{
			CAlien* pAlien = static_cast<CAlien*>(pNewActor);
			fEnergy = (pAlien->GetAlienEnergy() / (float)pAlien->GetMaxAlienEnergy()) * 100.0f + 1.0f;
		}
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*> (pActor);
			if (pPlayer->GetNanoSuit())
				fEnergy = pPlayer->GetNanoSuit()->GetSuitEnergy() * 0.5f + 1.0f;
		}
	}

	g_pGame->GetHUD()->m_animPlayerStats.Invoke("setHealth", (int)fHealth);
	g_pGame->GetHUD()->m_animPlayerStats.Invoke("setEnergy", (int)fEnergy);
}

void SControlClient::Update(float frametime)
{
	if (s_actionHandler.GetNumHandlers() == 0)
	{
#define ADD_HANDLER(action, func) s_actionHandler.AddHandler(actions.action, &SControlClient::func)
		const CGameActions& actions = g_pGame->Actions();

		ADD_HANDLER(view_lock, OnActionViewLock);
		ADD_HANDLER(moveforward, OnActionMoveForward);
		ADD_HANDLER(moveback, OnActionMoveBack);
		ADD_HANDLER(moveleft, OnActionMoveLeft);
		ADD_HANDLER(moveright, OnActionMoveRight);

		//ADD_HANDLER(squad_test, OnActionTest);

#undef ADD_HANDLER
	}
	assert(m_pControlledActor);
	if (!m_pControlledActor)
		return;

	const CGameActions& rGameActions = g_pGame->Actions();

	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pDude)
		return;

	CAlien* pAlien = static_cast<CAlien*>(m_pControlledActor);

	//This not fix trooper "gliding" after load game
	//pAlien->GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameClientStatic | eEA_GameServerDynamic | eEA_GameServerStatic);

	SMovementState state;
	m_pControlledActor->GetMovementController()->GetMovementState(state);

	IActor* pTargetActor = 0;
	Vec3 vCrosshairDir(m_crosshairPos - state.weaponPosition);

	// Get on crosshair entity and her AI
	const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
	static const unsigned entityFlags = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;

	ray_hit rhit;

	const float hits = gEnv->pPhysicalWorld->RayWorldIntersection(state.weaponPosition, vCrosshairDir.GetNormalizedSafe() * g_pGameCVars->ctrl_shootRange, entityFlags, rayFlags,
		&rhit, 1, m_pControlledActor->GetEntity()->GetPhysics());
	if (hits != 0)
	{
		if (rhit.pCollider)
		{
			IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(rhit.pCollider);
			if (pTargetEntity)
			{
				m_fireTargetId = pTargetEntity->GetId();
			}
			else
				m_fireTargetId = NULL;

			if (rhit.bTerrain)
				m_fireTargetId = NULL;
		}
		else
			m_fireTargetId = NULL;
	}
	else
		m_fireTargetId = NULL;

	//draw debug
	if (g_pGameCVars->ctrl_debug_draw == 1)
	{
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(state.weaponPosition, ColorB(0, 0, 255, 255), state.weaponPosition + m_camViewDir * 500, ColorB(0, 0, 255, 255));
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(state.weaponPosition, ColorB(0, 255, 0, 255), state.weaponPosition + vCrosshairDir.GetNormalizedSafe() * 2, ColorB(0, 255, 0, 255));
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_camPos, ColorB(255, 0, 0, 255), m_camPos + m_camViewDir, ColorB(255, 0, 0, 255));
	}

	m_meleeHits = gEnv->pPhysicalWorld->RayWorldIntersection(state.weaponPosition, vCrosshairDir.GetNormalizedSafe() * 3, entityFlags, rayFlags,
		&m_meleeRayhit, 1, m_pControlledActor->GetEntity()->GetPhysics());
	if (m_meleeHits != 0)
	{
		if (m_meleeRayhit.pCollider)
		{
			IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_meleeRayhit.pCollider);
			if (pTargetEntity)
			{
				m_meleeTargetId = pTargetEntity->GetId();

				//pTargetActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pTargetEntity->GetId());
				IAIObject* pTargetAI = pTargetEntity->GetAI();
				m_generic.isTargetHaveAI = pTargetAI ? true : false;
			}

			if (m_meleeRayhit.bTerrain)
				m_meleeTargetId = NULL;
		}
	}
	else
		m_meleeTargetId = 0;

	if (GetActorClassName() != "Scout")
	{
		if (m_crosshairRayHit.dist > 4.50f || pTargetActor)
			m_finalFireTargetPos = m_crosshairRayHit.pt;
		else
			m_finalFireTargetPos = m_camViewCoords;
	}
	else
	{
		if (rhit.dist > 8.50f || pTargetActor)
			m_finalFireTargetPos = m_crosshairRayHit.pt;
		else
			m_finalFireTargetPos = m_camViewCoords;
	}
	m_lookRequest.SetFireTarget(m_finalFireTargetPos);

	CWeapon* pWeapon = GetCurrentWeapon(m_pControlledActor);
	if (pWeapon)
	{
		if (!m_generic.canShoot)
			pWeapon->OnAction(m_pControlledActor->GetEntityId(), rGameActions.attack1, eAAM_OnRelease, 1.0f);

		static const char* weaponName = 0;
		if (GetItemClassName() == "FastLightMOAC_PLAY" || GetItemClassName() == "FastLightMOAC_MKII")
			weaponName = "Trooper MOAC";
		else if (GetItemClassName() == "FastLightMOAR_PLAY")
			weaponName = "Trooper MOAR";
		else if (GetItemClassName() == "Trooper_Melee_PLAY")
			weaponName = "Trooper Melee";
		else if (GetItemClassName() == "CommanderMOAC")
			weaponName = "Trooper Guardian";
		else if (GetItemClassName() == "CommanderMOAR")
			weaponName = "Trooper Leader";
		else if (GetItemClassName() == "LightMOAC_PLAY")
			weaponName = "Alien MOAC";
		else if (GetItemClassName() == "MOAR_PLAY")
			weaponName = "Hunter MOAR";
		else if (GetItemClassName() == "SingularityCannon_PLAY")
			weaponName = "Hunter Cannon";
		else if (GetItemClassName() == "ScoutMOAC_PLAY")
			weaponName = "Scout MOAC";
		else if (GetItemClassName() == "Scout_MOAR_PLAY")
			weaponName = "Scout MOAR";
		else if (GetItemClassName() == "ScoutSingularity_PLAY")
			weaponName = "Scout Cannon";
		else if (GetItemClassName() == "Scout_Beam")
			weaponName = "Scout Beam";
		SAFE_HUD_FUNC(SetWeaponName(weaponName))

			pWeapon->UpdateCrosshair(gEnv->pTimer->GetFrameTime());
		pWeapon->LowerWeapon(false);

		/*if (!pWeapon->IsWeaponLowered())
			m_Generic.m_bTargetFriendly = false;*/

		SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(false));

		const static int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
		const static int entFlags = ent_living | ent_rigid | ent_static | ent_sleeping_rigid | ent_independent;

		//Lower weapon
		if (GetFireTarget() == GetCrosshairEntity() || GetFireTarget() == GetMeleeTarget())
			m_generic.fireTargetIsFriend = IsFriendlyEntity(GetCrosshairEntity(), m_pControlledActor);

		if (GetMeleeTarget())
			m_generic.meleeTargetIsFriend = IsFriendlyEntity(GetMeleeTarget(), m_pControlledActor);
		else
			m_generic.meleeTargetIsFriend = false;

		if ((IsFriendlyEntity(GetFireTarget(), m_pControlledActor) ||
			IsFriendlyEntity(GetCrosshairEntity(), m_pControlledActor)) &&
			GetActorClassName() != "Hunter" &&
			m_generic.isShooting)
		{
			if ((!pWeapon->IsWeaponRaised() || !pWeapon->IsModifying()) && GetItemClassName() != "Trooper_Melee_PLAY")
			{
				pWeapon->LowerWeapon(true);
				pWeapon->StopFire();
			}

			if (pWeapon->IsWeaponLowered())
			{
				SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(true));
				//m_Generic.m_bTargetFriendly = true;
			}
		}
		if (GetMeleeTarget() && m_trooper.isAiming)
		{
			if (IsFriendlyEntity(GetMeleeTarget(), m_pControlledActor))
			{
				if ((!pWeapon->IsWeaponRaised() || !pWeapon->IsModifying()) && GetItemClassName() != "Trooper_Melee_PLAY")
				{
					pWeapon->LowerWeapon(true);
					pWeapon->StopFire();
				}

				if (pWeapon->IsWeaponLowered())
				{
					SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(true));
				}
			}
		}

		//pWeapon->SetAimLocation(m_vCrosshairPos);
		//pWeapon->SetTargetLocation(m_vCrosshairPos);
	}

	//draw debug
	if (g_pGameCVars->ctrl_debug_draw == 1)
	{
		static float c[] = { 1,1,1,1 };
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_meleeTargetId);
		IEntity* pEntity2 = gEnv->pEntitySystem->GetEntity(m_crosshairTargetId);
		gEnv->pRenderer->Draw2dLabel(20, 180, 1.15f, c, false, "m_MeleeTargetName %s", pEntity ? pEntity->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(20, 200, 1.15f, c, false, "m_CrosshairTargetName %s", pEntity2 ? pEntity2->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(20, 220, 1.15f, c, false, "m_FireTargetName %s", GetFireTarget() ? GetFireTarget()->GetName() : "");
		gEnv->pRenderer->Draw2dLabel(20, 240, 1.15f, c, false, "m_bFireTargetFriendly %s", m_generic.fireTargetIsFriend ? "true" : "false");
		gEnv->pRenderer->Draw2dLabel(20, 260, 1.15f, c, false, "m_bMeleeTargetFriendly %s", m_generic.meleeTargetIsFriend ? "true" : "false");

		gEnv->pRenderer->Draw2dLabel(20, 280, 1.15f, c, false, "m_MovementDir (%1.f,%1.f,%1.f)", m_generic.moveDir.x, m_generic.moveDir.y, m_generic.moveDir.z);
		gEnv->pRenderer->Draw2dLabel(20, 300, 1.15f, c, false, "m_bDisableMovement %s", m_generic.cannotMove ? "true" : "false");

		//gEnv->pRenderer->Draw2dLabel(20, 140, 1.3f, c, false, "deltaMovement (%f,%f,%f)", pAlien->GetAlienInput()->deltaMovement.x, pAlien->GetAlienInput()->deltaMovement.y, pAlien->GetAlienInput()->deltaMovement.z);
	}

	// if Alien die
	{
		if (m_pControlledActor->GetHealth() <= 0)
		{
			SAFE_HUD_FUNC(ShowDeathFX(1));
		}
		else
			SAFE_HUD_FUNC(ShowDeathFX(0));
	}

	//Update this before scout update
	if (!m_generic.cannotLookAt && !m_pAbilitiesSystem->trooper.isCeiling)
	{
		if (GetActorClassName() == "Hunter")
		{
			static Matrix33 rot;
			Vec3 v(0, 0, RAD2DEG(pDude->GetAngles().z));

			rot.SetRotationXYZ(Ang3(DEG2RAD(v)));
			m_lookRequest.SetLookTarget(pAlien->GetEntity()->GetWorldPos() + 20.0f * rot.GetColumn1());
		}
		else
		{
			CCamera& camera = gEnv->pSystem->GetViewCamera();
			m_lookRequest.SetLookTarget(state.eyePosition + 20.0f * camera.GetMatrix().GetColumn1());
		}
	}

	//Timers updating;
	if (GetActorClassName() == "PlayerTrooper")
	{
		UpdateTrooper();
	}
	else if (GetActorClassName() == "Scout")
	{
		UpdateScout();
	}
	else if (GetActorClassName() == "Hunter")
	{
		UpdateHunter();
	}
	else if (GetActorClassName() == "Alien")
	{

		//if (m_generic.isAiming == 1)
		{
			//CCamera& camera = gEnv->pSystem->GetViewCamera();
			//m_lookRequest.SetAimTarget(state.eyePosition + 20.0f * camera.GetMatrix().GetColumn1());

			
		}
		//else
		{
			//m_lookRequest.ClearAimTarget();
		}
	}

	//Energy on hud
	CNanoSuit* pSuit = pDude->GetNanoSuit();
	if (pSuit)
	{
		if (m_pControlledActor->IsAlien())
			pSuit->SetSuitEnergy(pAlien->GetAlienEnergy());
	}

	//Death mechanics
	UpdateDeath(pDude);

	//Hide the Nomad player when is controlling somebody
	if (m_mustHideDude)
	{
		SActorStats* pActorStats = pDude->GetActorStats();
		if (pActorStats)
		{
			uint8 physicsProfile = pDude->GetGameObject()->GetAspectProfile(eEA_Physics);

			if (physicsProfile != eAP_Spectator)
				pDude->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

			if (!pActorStats->isHidden)
				pActorStats->isHidden = true;
		}
	}

	//Beam the Nomad player to the controlled actor position
	if (m_mustBeamDude)
	{
		Vec3 vControlledPos = { 0,0,0 };
		if (m_pControlledActor)
			vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();

		if (vControlledPos != (0, 0, 0))
		{
			vControlledPos.z += 3;

			Matrix34 nomadMat34 = pDude->GetEntity()->GetWorldTM();
			nomadMat34.SetTranslation(vControlledPos);
			pDude->GetEntity()->SetWorldTM(nomadMat34);
		}
	}

	// movement and others
	if (m_generic.cannotMove == true && m_generic.canMoveMult != 0)
		m_generic.canMoveMult = 0;
	else if (m_generic.cannotMove == false)
		m_generic.canMoveMult = 1.0f;

	if (m_pControlledActor->IsAlien())
	{
		const string className = m_pControlledActor->GetEntity()->GetClass()->GetName();

		if (gEnv->bMultiplayer)
		{
			if (gEnv->bClient)
			{
				Vec3 currentMovement(0, 0, 0);
				Vec3& currentLookDir = pAlien->GetAlienViewMtx().GetColumn1().GetNormalized();
				Vec3& currentPosition = pAlien->GetEntity()->GetWorldPos();

				if (className == "Scout")
					currentMovement = m_generic.moveDir * m_scout.speedMult * m_generic.canMoveMult;
				else if (className == "PlayerTrooper")
					currentMovement = m_generic.moveDir * m_trooper.speedMult * m_generic.canMoveMult;
				else if (className == "Alien")
					currentMovement = m_generic.moveDir * m_nakedAlien.speedMult * m_nakedAlien.aimSpeedMult * m_generic.canMoveMult;
				else if (className == "Hunter")
					currentMovement = m_generic.moveDir * m_generic.canMoveMult;

				//CryLogAlways("Client requesting movement");

				pAlien->GetGameObject()->InvokeRMI(CAlien::SvRequestSetMove(), CAlien::SMovementParams(currentLookDir, currentMovement, currentPosition), eRMI_ToServer);
			}
		}
		else
		{
			if (className == "Scout")
			{
				//BUGFIX: Trooper very speedly after Naked control
				if (m_lookRequest.HasStance())
				{
					m_lookRequest.ClearStance();
				}

				pAlien->SetAlienMove(m_generic.moveDir * m_scout.speedMult * m_generic.canMoveMult);

				//Disables continuation rotation when cannotLookAt is true
				m_scout.modelQuat.SetIdentity();

				if (!m_generic.cannotLookAt && !m_pAbilitiesSystem->scout.IsSearch && !m_pAbilitiesSystem->scout.isGyroEnabled)
				{
					Quat& alienInvQuat = pAlien->GetEntity()->GetRotation().GetInverted();
					Quat alienQuat(pAlien->GetEntity()->GetRotation());
					Quat dudeViewQuat = pDude->GetViewQuatFinal();

					const float rotRate = pAlien->m_rateParams.rotationRate;

					alienQuat = Quat::CreateSlerp(alienQuat, dudeViewQuat, rotRate * 2);
					m_scout.modelQuat = alienInvQuat * alienQuat;
				}
			}
			else if (className == "PlayerTrooper")
			{
				//BUGFIX: Trooper very speedly after Naked control
				if (m_lookRequest.HasStance())
				{
					m_lookRequest.ClearStance();
				}
				
				const Vec3 move = m_generic.moveDir * m_trooper.speedMult * m_generic.canMoveMult;
				pAlien->SetAlienMove(move);
			}
			else if (className == "Alien")
			{
				//BUGFIX: Trooper very speedly after Naked control
				if (m_lookRequest.HasStance())
				{
					m_lookRequest.ClearStance();
				}

				const Vec3 move = m_generic.moveDir * m_nakedAlien.speedMult * m_nakedAlien.aimSpeedMult * m_generic.canMoveMult;
				pAlien->SetAlienMove(move);

				//Set naked alien stance for true animation
				//m_lookRequest.SetStance(STANCE_PRONE);

				//Disables continuation rotation when cannotLookAt is true
				m_nakedAlien.modelQuat.SetIdentity();

				if (!m_generic.cannotLookAt)
				{
					//Naked alien have a specific rotation like in zeroG
					const Quat alienInvQuat = pAlien->GetEntity()->GetRotation().GetInverted();
					const Quat dudeViewQuat = pDude->GetViewQuatFinal();

					m_nakedAlien.modelQuat = alienInvQuat * dudeViewQuat;
				}
			}
			else if (className == "Hunter")
			{
				//BUGFIX: Trooper very speedly after Naked control
				if (m_lookRequest.HasStance())
				{
					m_lookRequest.ClearStance();
				}

				pAlien->SetAlienMove(m_generic.moveDir * m_generic.canMoveMult);
			}
		}
	}

	//Only Look target and aim processing
	if (!m_generic.cannotLookAt)
		m_pControlledActor->GetMovementController()->RequestMovement(m_lookRequest);

	//pAlien->SetActorMovement(CAlien::SMovementRequestParams(m_Req));

	//Update entities usability
	UpdateUsability(GetMeleeTarget());

	//Interpolation tests
	//{
	//	static float actualValue = 0.0f;
	//	static float goalValue = 100.0f;
	//	static float speed = 1.0f;

	//	if (actualValue != goalValue)
	//		Interpolate(actualValue, goalValue, speed, frametime);
	//	else
	//		actualValue = 0.0f;

	//	static float c[] = { 1,1,1,1 };
	//	gEnv->pRenderer->Draw2dLabel(60, 180, 2.0f, c, false, "Interpolation (actual: %f, goal: %f, speed: %f, frametime: %f)",actualValue,goalValue,speed,frametime);
	//}
}

void SControlClient::UpdateUsability(IEntity* pTarget)
{
	if (pTarget)
	{
		IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pTarget->GetId());
		IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pTarget->GetId());

		if (!pVehicle && !pItem)
		{
			IScriptTable* pTable = pTarget->GetScriptTable();
			if (pTable)
			{
				bool bIsUsable = false;
				const char* sUseText = "";

				SmartScriptTable props;
				if (pTable->GetValue("Properties", props))
					props->GetValue("UseText", sUseText);
				string sText = sUseText;

				HSCRIPTFUNCTION IsUsable = 0;
				if (pTable->GetValueType("IsUsable") == svtFunction &&
					pTable->GetValue("IsUsable", IsUsable))
				{
					Script::CallReturn(gEnv->pScriptSystem, IsUsable, pTable, bIsUsable);
					gEnv->pScriptSystem->ReleaseFunc(IsUsable);
					//CryLogAlways("%s is Usable %1.f", pTarget->GetName(), (float)result);
				}

				const float fDist = (pTarget->GetWorldPos() - m_pControlledActor->GetEntity()->GetWorldPos()).GetLength();
				if (fDist <= 3.f && bIsUsable)
				{
					SAFE_HUD_FUNC(GetCrosshair()->SetUsability(1, sText == "" ? "@use_door" : sText.c_str()));
					//CryLogAlways("Usable Text: %s", sText.c_str());
				}
				else if (fDist > 3.f || !bIsUsable)
					SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0));
			}
		}
	}
	else
	{
		SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0));
	}
}

void SControlClient::UpdateCrosshair()
{
	CCamera& camera = gEnv->pSystem->GetViewCamera();
	m_camPos = camera.GetMatrix().GetTranslation();
	m_camViewDir = camera.GetViewdir() * g_pGameCVars->ctrl_shootRange;
	m_camViewCoords = m_camViewDir + m_camPos;

	// Get on crosshair entity and her AI
	static const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
	static const unsigned entityFlags = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;

	//Physics entity
	IPhysicalEntity* pDudePhysics = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetPhysics();
	IPhysicalEntity* pPhys = (m_pControlledActor != 0) ? m_pControlledActor->GetEntity()->GetPhysics() : pDudePhysics;

	//Calculate fire direction from weapon position
	/*if (gEnv->pPhysicalWorld->RayWorldIntersection(m_CamPos, m_CamViewDir, entityFlags, rayFlags, &m_CrosshairRayHit, 1, pPhys))
	{
		m_vCrosshairPos = m_CrosshairRayHit.pt;
		if (m_CrosshairRayHit.pCollider)
		{
			IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_CrosshairRayHit.pCollider);
			if (pTargetEntity)
			{
				m_LastCrosshairTargetId = m_CrosshairTargetId = pTargetEntity->GetId();

				IAIObject* pTargetAI = pTargetEntity->GetAI();
				m_Generic.m_isTargetAI = pTargetAI ? true : false;
			}
			else
				m_CrosshairTargetId = NULL;

			if (m_CrosshairRayHit.bTerrain)
				m_CrosshairTargetId = NULL;
		}
		else
			m_CrosshairTargetId = NULL;
	}*/

	//Calculate crosshair position and target from camera pos;
	if (gEnv->pPhysicalWorld->RayWorldIntersection(m_camPos, m_camViewDir, entityFlags, rayFlags, &m_crosshairRayHit, 1, pPhys))
	{
		m_crosshairPos = m_crosshairRayHit.pt;
		if (m_crosshairRayHit.pCollider)
		{
			IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_crosshairRayHit.pCollider);
			if (pTargetEntity)
			{
				m_lastCrosshairTargetId = m_crosshairTargetId = pTargetEntity->GetId();

				IAIObject* pTargetAI = pTargetEntity->GetAI();
				m_generic.isTargetHaveAI = pTargetAI ? true : false;
			}
			else
				m_crosshairTargetId = NULL;

			if (m_crosshairRayHit.bTerrain)
				m_crosshairTargetId = NULL;
		}
		else
			m_crosshairTargetId = NULL;
	}
	else
	{
		m_crosshairPos = m_camViewCoords;
		m_crosshairTargetId = 0;
	}
}

void SControlClient::UpdateHunter()
{
	CActor* pDudePlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (!m_pControlledActor || !pDudePlayer)
		return;

	if (pDudePlayer)
	{
		SActorParams* pParams = pDudePlayer->GetActorParams();

		pParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
		pParams->vLimitRangeH = DEG2RAD(70);
	}
}

void SControlClient::UpdateScout()
{
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (!m_pControlledActor || !pDude)
		return;

	SActorParams* pDudeParams = pDude->GetActorParams();
	CScout* pScout = static_cast<CScout*>(m_pControlledActor);

	if (g_pGameCVars->ctrl_ScoutAimSupport == 1)
	{
		IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntity(m_lastCrosshairTargetId);
		if (pTargetEntity)
		{
			IActor* pTargetActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pTargetEntity->GetId());
			IVehicle* pTargetVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pTargetEntity->GetId());

			bool isActorAlive = pTargetActor && !(pTargetActor->GetHealth() < 0.0f);
			bool isVehicleAlive = pTargetVehicle && !(pTargetVehicle->IsDestroyed());
			bool isAutoAiming = false;
			bool isHostileTarget = !IsFriendlyEntity(pTargetEntity,m_pControlledActor);

			if (isActorAlive && isHostileTarget || isVehicleAlive)
			{
				AABB targetBounds;
				pTargetEntity->GetWorldBounds(targetBounds);

				Vec3 targetCenterPos = targetBounds.GetCenter();

				//Distance method
				float distToTarget = (targetCenterPos - m_crosshairPos).GetLength();
				float threshold = 10.0f;

				isAutoAiming = distToTarget < threshold;
				if (isAutoAiming)
					m_lookRequest.SetFireTarget(targetCenterPos);
			}
				
			if (CHUDSilhouettes* pSil = SAFE_HUD_FUNC_RET(GetSilhouettes()))
			{
				if (isAutoAiming)
					pSil->SetSilhouette(pTargetEntity, 1.0f, 1.0f, 1.0f, 1.0f, -3.0f);
				else
				{
					if (pSil->GetSilhouette(pTargetEntity))
						pSil->ResetSilhouette(pTargetEntity->GetId());
				}
			}
		}
	}

	if (m_pAbilitiesSystem->scout.IsSearch)
	{
		//Look forward when scout enable search mode
		if (GetActorClassName() == "Scout")
		{
			Matrix33 rot;
			Vec3 v(0, 0, RAD2DEG(pDude->GetAngles().z));

			rot.SetRotationXYZ(Ang3(DEG2RAD(v)));
			m_lookRequest.SetLookTarget(pScout->GetEntity()->GetWorldPos() + 20.0f * rot.GetColumn1());
		}
	}

	if (!m_scout.canDodge)// When scout dodging
	{
		if (CheckPassTime(SCOUT_DODGE_REST_TIME))
		{
			m_scout.canDodge = true;
		}
	}

	if (m_pAbilitiesSystem->scout.IsSearch)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId);
		if (pEntity)
		{
			pEntity->SetRotation(pDude->GetViewQuatFinal());

			if (pDude)
			{
				pDudeParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
				pDudeParams->vLimitRangeH = DEG2RAD(50);
				pDudeParams->vLimitRangeVUp = DEG2RAD(1);
				pDudeParams->vLimitRangeVDown = DEG2RAD(-50);
			}

			if (m_crosshairRayHit.pCollider)
			{
				IPhysicalEntity* pSkipEntities[10];
				int nSkip = 0;
				IItem* pItem = m_pControlledActor->GetCurrentItem();
				if (pItem)
				{
					CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
					if (pWeapon)
						nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
				}

				const Vec3& center(m_crosshairRayHit.pt);
				const float range = 5.f;
				const float rangeSq = range * range;

				const Vec3 min(center.x - range, center.y - range, center.z - range);
				const Vec3 max(center.x + range, center.y + range, center.z + range);

				IPhysicalWorld* pWorld = gEnv->pPhysicalWorld;
				IPhysicalEntity** ppList = NULL;
				int	numEnts = pWorld->GetEntitiesInBox(min, max, ppList, ent_all);
				for (int i = 0; i < numEnts; ++i)
				{
					if (m_crosshairRayHit.dist < 35.f)
					{
						EntityId id = pWorld->GetPhysicalEntityId(ppList[i]);
						IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);

						CHUDSilhouettes* pSil = SAFE_HUD_FUNC_RET(GetSilhouettes());

						if (pEntity && pSil)
						{
							IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id); //Scanned actor
							IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id); //Scanned vehicle

							IAIObject* pAI = pEntity->GetAI();
							IAIObject* pControlActorAI = m_pControlledActor->GetEntity()->GetAI();

							if (pActor && pControlActorAI && pAI)
							{
								if (pAI->IsHostile(pControlActorAI, false))
								{
									CPlayer* pScanPlayer = static_cast<CPlayer*>(pActor);
									if (pScanPlayer)
									{
										CNanoSuit* pScanSuit = pScanPlayer->GetNanoSuit();
										if (pScanSuit && pScanSuit->GetMode() == NANOMODE_CLOAK)
										{
											pScanSuit->SetMode(NANOMODE_DEFENSE);
											continue;
										}
									}

									if (!pSil->GetSilhouette(pEntity))
									{
										pSil->SetSilhouette(pEntity, 255, 255, 255, 1.0f, 60.0f);

										if (g_pGameCVars->sqd_ScoutCanSearch)
										{
											SSquad& squad = g_pControlSystem->pSquadSystem->GetSquadFromMember(m_pControlledActor, 1);
											if (squad.isPlayerLeader())
											{
												TMembers::iterator it = squad.GetAllMembers().begin();
												TMembers::iterator end = squad.GetAllMembers().end();

												for (; it != end; it++)
												{
													it->searchPos = pEntity->GetWorldPos();
													squad.ExecuteOrder(eSquadOrders_SearchEnemy, *it);
												}
											}
											else
												SendPipeToAIGroup("rush_ai", pControlActorAI->GetGroupId(), true, pControlActorAI->GetEntity()->GetWorldPos());
										}
									}

									if (pActor->GetHealth() <= 0.0f)
									{
										pSil->ResetSilhouette(id);
									}
								}
							}

							if (pVehicle)
							{
								IAIObject* pAI = pVehicle->GetEntity()->GetAI();

								if (pAI && pAI->IsHostile(pControlActorAI, false) && pVehicle->GetStatus().passengerCount > 0)
								{
									if (!pSil->GetSilhouette(pEntity))
									{
										pSil->SetSilhouette(pEntity, 255, 255, 255, 1.0f, 60.0f);
										SendPipeToAIGroup("rush_ai", pControlActorAI->GetGroupId(), true, pControlActorAI->GetEntity()->GetWorldPos());
									}

									if (pVehicle->IsDestroyed())
										pSil->ResetSilhouette(id);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		//Avoid turning on search light from the AI
		if (pScout->m_searchbeam.isActive == true)
		{
			pScout->EnableSearchBeam(false);
		}

		if (pDude)
		{
			pDudeParams->vLimitDir.zero();
			pDudeParams->vLimitRangeH = 0.0f;
			pDudeParams->vLimitRangeVUp = pDudeParams->vLimitRangeVDown = 0.0f;
		}
	}
}

void SControlClient::UpdateTrooper()
{
	CActor* pDudePlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (!m_pControlledActor || !pDudePlayer)
		return;

	IScriptTable* pTable = m_pControlledActor->GetEntity()->GetScriptTable();
	IAIObject* pAITrooper = m_pControlledActor->GetEntity()->GetAI();

	static Vec3 vWeaponTarget(0, 0, 0);// Trooper melee target position
	if (m_meleeHits != 0)
		vWeaponTarget = m_meleeRayhit.pt;
	else
		vWeaponTarget = m_camViewCoords;

	//when is ceiling
	SActorParams* pParams = pDudePlayer->GetActorParams();
	uint flags = m_pControlledActor->GetEntity()->GetSlotFlags(0);

	//Trooper is using the binoculars -> play the search animation
	if (m_trooper.isBinocular && m_pControlledActor->GetStance() != STANCE_STEALTH)
	{
		if (!m_pAbilitiesSystem->trooper.isCeiling && !m_pAbilitiesSystem->trooper.doCeiling)
		{
			m_lookRequest.SetStance(STANCE_STEALTH);
		}
	}
	else if (!m_trooper.isBinocular && m_pControlledActor->GetStance() == STANCE_STEALTH)
	{
		if (!m_pAbilitiesSystem->trooper.isCeiling && !m_pAbilitiesSystem->trooper.doCeiling)
		{
			m_lookRequest.SetStance(STANCE_STAND);
		}
	}

	if (m_pAbilitiesSystem->trooper.isCeiling)
	{
		if (m_pControlledActor->GetHealth() > 0.1f)
		{
			m_generic.cannotMove = true;

			//Disable trooper body
			flags &= ~ENTITY_SLOT_RENDER;
			m_pControlledActor->GetEntity()->SetSlotFlags(0, flags);

			if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_Ceiling)
			{
				//Change view limits
				if (pDudePlayer)
				{
					pParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
					pParams->vLimitRangeH = DEG2RAD(50);
					pParams->vLimitRangeVUp = DEG2RAD(1);
					pParams->vLimitRangeVDown = DEG2RAD(-50);
				}

				IPhysicalEntity* pSkipEntities[10];
				int nSkip = 0;
				IItem* pItem = m_pControlledActor->GetCurrentItem();
				if (pItem)
				{
					CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
					if (pWeapon)
						nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
				}

				ray_hit hit;
				Vec3 vTrooperUpDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn2() * 3.0f;

				int hits = gEnv->pPhysicalWorld->RayWorldIntersection(m_pControlledActor->GetEntity()->GetWorldPos(), vTrooperUpDir, ent_terrain | ent_static,
					rwi_ignore_noncolliding | rwi_stop_at_pierceable, &hit, 1, pSkipEntities, nSkip);

				if (!hit.pCollider)
				{
					m_pAbilitiesSystem->trooper.isCeiling = false;
					m_pAbilitiesSystem->trooper.doCeiling = false;
					PlayAnimAction("idle", true);
					//m_bTrooperIsCeiling = false;
				}
			}
			else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallLeft)
			{
				//Change view limits
				if (pDudePlayer)
				{
					pParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn0() * 2.f;
					pParams->vLimitRangeH = DEG2RAD(90);//horizont
					pParams->vLimitRangeVUp = DEG2RAD(1);//up limit form center
					pParams->vLimitRangeVDown = DEG2RAD(-25);//down limit form center
				}
			}
			else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallRight)
			{
				//Change view limits
				if (pDudePlayer)
				{
					pParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn0() * -2.f;
					pParams->vLimitRangeH = DEG2RAD(90);
					pParams->vLimitRangeVUp = DEG2RAD(1);
					pParams->vLimitRangeVDown = DEG2RAD(-25);
				}
			}
		}
		else
		{
			m_pAbilitiesSystem->trooper.isCeiling = false;
			m_pAbilitiesSystem->trooper.doCeiling = false;
			PlayAnimAction("idle", true);
			//m_bTrooperIsCeiling = false;
		}
	}

	//Allow melee
	if (!m_trooper.canMelee)// When trooper make a punch
	{
		if (CheckPassTime(TROOPER_MELEE_REST_TIME))
		{
			m_trooper.canMelee = true;
		}
	}
	else if (!m_trooper.canJumpMelee)
	{
		if (m_pControlledActor->GetActorStats()->onGround > 0.10f) //
		{
			m_trooper.canJumpMelee = true;
		}
	}

	const Vec3 vImpulseDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(1) * 40.f;// Trooper melee attack impulse direction
	//Do melee flags from OnAction
	if (m_trooper.doMelee && !m_generic.meleeTargetIsFriend)
	{
		if (CheckPassTime(0.2f))//time while playing animation
		{
			m_trooper.doMelee = false;

			if (CWeapon* pWeapon = GetCurrentWeapon(m_pControlledActor))
			{
				if (pWeapon->GetMeleeFireMode())
				{
					const float distanceToTarget = (vWeaponTarget - m_pControlledActor->GetEntity()->GetWorldPos()).GetLength();
					if (distanceToTarget <= TROOPER_MELEE_DAMAGE_DISTANCE)
					{
						IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_meleeTargetId);
						IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_meleeTargetId);

						if (pActor && pActor != g_pGame->GetIGameFramework()->GetClientActor())
						{
							if (pActor->GetHealth() > 0.0f)
							{
								Script::CallMethod(pTable, "Ctrl_DoMelee", vImpulseDir, pActor->GetEntityId());
								IPhysicalEntity* pPhys = pActor->GetEntity()->GetPhysics();

								//CryLogAlways("Actor");

								if (pPhys)
								{
									pe_action_impulse impulse;
									impulse.impulse = (pActor->GetEntity()->GetWorldPos() - m_pControlledActor->GetEntity()->GetWorldPos()) * 500.f;
									pPhys->Action(&impulse);

									//CryLogAlways("Impulse");
								}

								SAFE_HUD_FUNC(IndicateHit(false, NULL, false));
							}
						}
						else
							pWeapon->GetMeleeFireMode()->NetShootEx(vWeaponTarget, m_camViewDir, Vec3(0, 0, 0), Vec3(0, 0, 0), 0, 0);

						if (pVehicle)
							SAFE_HUD_FUNC(IndicateHit(false, NULL, false));
					}
				}
			}
		}
	}
	else if (m_trooper.doJumpMelee)
	{
		if (m_pControlledActor->GetActorStats()->onGround > 0.10f)//
		{
			m_trooper.doJumpMelee = false;

			if (CWeapon* pWeapon = GetCurrentWeapon(m_pControlledActor))
			{
				if (pWeapon->GetMeleeFireMode())
				{
					IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_meleeTargetId);
					IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_meleeTargetId);
					Vec3 vDirToTarget = vWeaponTarget - m_pControlledActor->GetEntity()->GetWorldPos();

					const float distanceToTarget = (vDirToTarget).GetLength();
					if (distanceToTarget <= TROOPER_MELEE_DAMAGE_DISTANCE)
					{
						if (!m_generic.fireTargetIsFriend)
						{
							if (pActor && pActor->GetHealth() > 0.0f && pActor != g_pGame->GetIGameFramework()->GetClientActor())
							{
								Script::CallMethod(pTable, "Ctrl_DoMelee", vImpulseDir, pActor->GetEntityId());
								IPhysicalEntity* pPhys = pActor->GetEntity()->GetPhysics();

								//CryLogAlways("Actor");

								if (pPhys)
								{
									pe_action_impulse impulse;
									impulse.impulse = vDirToTarget * 500.f;
									pPhys->Action(&impulse);

									//CryLogAlways("Impulse");
								}
								SAFE_HUD_FUNC(IndicateHit(false, NULL, false));
							}
							else
							{
								pWeapon->GetMeleeFireMode()->NetShootEx(vWeaponTarget, m_camViewDir, Vec3(0, 0, 0), Vec3(0, 0, 0), 0, 0);
							}

							if (pVehicle)
								SAFE_HUD_FUNC(IndicateHit(false, NULL, false));
						}
					}
				}
			}
		}
	}

	if (!m_trooper.canDodge)
	{
		if (CheckPassTime(TROOPER_DODGE_REST_TIME))
			m_trooper.canDodge = true;
	}

	if (!m_pAbilitiesSystem->trooper.canCeiling)
	{
		if (!m_pControlledActor->GetActorStats()->onGround)
		{
			m_pAbilitiesSystem->trooper.canCeiling = true;
		}
	}
	//Stay on ceiling
	if (m_pControlledActor->GetActorStats()->inAir > 0.25f && !m_trooper.doJumpMelee)
	{
		IPhysicalEntity* pSkipEntities[10];
		int nSkip = 0;
		IItem* pItem = m_pControlledActor->GetCurrentItem();
		if (pItem)
		{
			CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
			if (pWeapon)
				nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
		}

		ray_hit hit;
		const int objTypes = ent_terrain | ent_static;
		const int rayFlags = rwi_ignore_noncolliding | rwi_stop_at_pierceable;

		if (m_pAbilitiesSystem->trooper.doCeiling)//if ability key is pressed
		{
			SMovementState state;
			m_pControlledActor->GetMovementController()->GetMovementState(state);

			Vec3 xdir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn0() * 100;
			Vec3 worldpos = m_pControlledActor->GetEntity()->GetWorldPos();

			if (m_actionsMap["moveleft"] == eAAM_OnPress && g_pGameCVars->ctrl_trWalls == 1)
			{
				if (!m_pAbilitiesSystem->trooper.isCeiling)
				{
					m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_WallLeft;

					const Vec3 vTrooperLeftDir = xdir * -1;

					const int leftHits = gEnv->pPhysicalWorld->RayWorldIntersection(m_pControlledActor->GetEntity()->GetWorldPos(), vTrooperLeftDir, objTypes, rayFlags, &hit, 1, pSkipEntities, nSkip);
					if (leftHits > 0)
					{
						gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(worldpos, ColorB(0, 255, 0, 255), worldpos + vTrooperLeftDir, ColorB(0, 255, 0, 255));

						if (hit.pCollider)
						{
							if (hit.dist <= 2.0)
							{
								PlayAnimAction("CTRL_StayOnWallLeft", true);
								m_pAbilitiesSystem->trooper.isCeiling = true;

								//Matrix34 mat34 = m_pControlledActor->GetEntity()->GetWorldTM();
								const Vec3 hitPoint(hit.pt.x + 2, m_pControlledActor->GetEntity()->GetWorldPos().y, m_pControlledActor->GetEntity()->GetWorldPos().z);

								//mat34.SetTranslation(hitPoint);
								//m_pControlledActor->GetEntity()->SetWorldTM(mat34);
							}
						}
					}
				}
			}
			else if (m_actionsMap["moveright"] == eAAM_OnPress && g_pGameCVars->ctrl_trWalls == 1)
			{
				if (!m_pAbilitiesSystem->trooper.isCeiling)
				{
					m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_WallRight;

					const Vec3 vTrooperRightDir = xdir;

					const int rightHits = gEnv->pPhysicalWorld->RayWorldIntersection(m_pControlledActor->GetEntity()->GetWorldPos(), vTrooperRightDir, objTypes, rayFlags, &hit, 1, pSkipEntities, nSkip);
					if (rightHits > 0)
					{
						gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_pControlledActor->GetEntity()->GetWorldPos(), ColorB(0, 255, 0, 255), m_pControlledActor->GetEntity()->GetWorldPos() + vTrooperRightDir, ColorB(0, 255, 0, 255));

						if (hit.pCollider)
						{
							if (hit.dist <= 2.0)
							{
								PlayAnimAction("CTRL_StayOnWallRight", true);
								m_pAbilitiesSystem->trooper.isCeiling = true;

								//Matrix34 mat34 = m_pControlledActor->GetEntity()->GetWorldTM();
								const Vec3 hitPoint(hit.pt.x - 2,
									m_pControlledActor->GetEntity()->GetWorldPos().y,
									m_pControlledActor->GetEntity()->GetWorldPos().z);

								//mat34.SetTranslation(hitPoint);
								//m_pControlledActor->GetEntity()->SetWorldTM(mat34);
							}
						}
					}
				}
			}
			else
			{
				if (!m_pAbilitiesSystem->trooper.isCeiling)
					m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_Ceiling;

				const Vec3 vTrooperUpDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn2() * 100.f;
				const int ceilingHits = gEnv->pPhysicalWorld->RayWorldIntersection(m_pControlledActor->GetEntity()->GetWorldPos(),
					vTrooperUpDir, objTypes, rayFlags, &hit, 1, pSkipEntities, nSkip);
				//int rightHits = gEnv->pPhysicalWorld->RayWorldIntersection(m_pControlledActor->GetEntity()->GetWorldPos(), vTrooperUpDir, objTypes, rayFlags, &ceilingHit, 1, pSkipEntities, nSkip);

				if (ceilingHits > 0)
				{
					if (hit.pCollider)
					{
						if (hit.dist <= 2.3 && hit.dist >= 2.0)
						{
							PlayAnimAction("CTRL_StayOnCeiling", true);
							m_pAbilitiesSystem->trooper.isCeiling = true;
						}
					}
				}
			}

			if (IInventory* pInventory = m_pControlledActor->GetInventory())
			{
				if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
				{
					pClassRegistry->IteratorMoveFirst();

					IEntityClass* pEntityClass = pClassRegistry->FindClass("FastLightMOAC_PLAY");
					if (!pEntityClass)
						pEntityClass = pClassRegistry->FindClass("FastLightMOAR_PLAY");

					if (pEntityClass)
					{
						EntityId shootWeaponId = pInventory->GetItemByClass(pEntityClass);

						if (shootWeaponId)
							m_pControlledActor->SelectItem(shootWeaponId, false);
					}
				}
			}
		}
		else if (!m_pAbilitiesSystem->trooper.doCeiling && m_pAbilitiesSystem->trooper.isCeiling)
		{
			//pe_status_dynamics status = pe_status_dynamics();
			//m_pControlledActor->GetEntity()->GetPhysics()->GetStatus(&status);
			PlayAnimAction("idle", true);

			pe_action_impulse impulseParams;

			if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallLeft)
				impulseParams.impulse = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn0() * 500.f;
			else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallRight)
				impulseParams.impulse = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn0() * -500.f;

			m_pControlledActor->GetEntity()->GetPhysics()->Action(&impulseParams);

			m_pAbilitiesSystem->trooper.isCeiling = false;

			//Enable trooper body
			flags |= ENTITY_SLOT_RENDER;
			m_pControlledActor->GetEntity()->SetSlotFlags(0, flags);

			m_generic.cannotMove = false;

			//Change view limits
			if (pDudePlayer)
			{
				pParams->vLimitDir.zero();
				pParams->vLimitRangeH = 0.0f;
				pParams->vLimitRangeVUp = pParams->vLimitRangeVDown = 0.0f;
			}
		}
	}

	if (CWeapon* pWeapon = GetCurrentWeapon(m_pControlledActor))
	{
		//m_restTimer = 0;

		if (pWeapon->IsFiring() && GetCrosshairTargetAI() && GetCrosshairTargetAI()->IsHostile(pAITrooper))
		{
			if (IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_crosshairTargetId))
			{
				if (pVehicle->GetStatus().passengerCount == 0)
					return;
			}

			if (m_generic.fSoundRestTimer == 0)
			{
				IEntitySoundProxy* pSoundProxy = (IEntitySoundProxy*)m_pControlledActor->GetEntity()->CreateProxy(ENTITY_PROXY_SOUND);
				if (pSoundProxy)
				{
					ISound* pSound = gEnv->pSoundSystem->CreateSound("sounds/alien:trooper:scare", FLAG_SOUND_DEFAULT_3D);

					if (pSound)
					{
						pSound->SetPosition(m_pControlledActor->GetEntity()->GetWorldPos());
						pSound->SetSemantic(eSoundSemantic_AI_Readability);
						pSound->SetVolume(1);

						pSoundProxy->PlaySound(pSound);
						m_generic.fSoundRestTimer = Random(12, 20);

						if (g_pGameCVars->sqd_TrooperCanSay)
						{
							SSquad& squad = g_pControlSystem->pSquadSystem->GetSquadFromMember(m_pControlledActor, 1);
							//if (!squad.isPlayerLeader())
							//	squad = g_pControlSystem->pSquadSystem->GetSquadFromLeader(m_pControlledActor);

							if (squad.isPlayerLeader())
							{
								TMembers::iterator it = squad.GetAllMembers().begin();
								TMembers::iterator end = squad.GetAllMembers().end();

								for (; it != end; it++)
								{
									it->searchPos = m_crosshairPos;
									squad.ExecuteOrder(eSquadOrders_SearchEnemy, *it);
								}
							}
							else
								SendPipeToAIGroup("rush_ai", pAITrooper->GetGroupId(), true, pAITrooper->GetEntity()->GetWorldPos());
						}
					}
				}
			}
		}

		if (m_generic.fSoundRestTimer != 0)
		{
			m_generic.fSoundRestTimer -= gEnv->pTimer->GetFrameTime();
			if (m_generic.fSoundRestTimer < 0)
				m_generic.fSoundRestTimer = 0;
		}

		//if (!g_pControlSystem->GetEnabled())
		//	restTimer = 0;

		//float clr[] = { 1,1,1,1 };
		//gEnv->pRenderer->Draw2dLabel(20, 20, 2, clr, false, "restTimer: %.f", m_SoundRestTimer);
		//gEnv->pRenderer->Draw2dLabel(20, 40, 2, clr, false, "m_CrosshairTargetId: %.f", float(m_CrosshairTargetId));
	}

	if (m_pAbilitiesSystem->trooper.shockwaveTimer != 0)
	{
		m_pAbilitiesSystem->trooper.shockwaveTimer -= gEnv->pTimer->GetFrameTime();

		if (m_pAbilitiesSystem->trooper.shockwaveTimer < 0)
			m_pAbilitiesSystem->trooper.shockwaveTimer = 0;
	}
	//draw debug
	if (g_pGameCVars->ctrl_debug_draw == 1)
	{
		static float clr[] = { 1,1,1,1 };
		gEnv->pRenderer->Draw2dLabel(20, 400, 1.15f, clr, false, "m_pAbilitiesSystem->trooper.shockwaveTimer: %.f", m_pAbilitiesSystem->trooper.shockwaveTimer);
	}
	//gEnv->pRenderer->Draw2dLabel(20, 40, 2, clr, false, "m_CrosshairTargetId: %.f", float(m_CrosshairTargetId));
}

void SControlClient::UpdateDeath(IActor* pDude)
{
	CPlayer* pDudePlayer = static_cast<CPlayer*>(pDude);
	if (!GetTutorialMode())
	{
		//if (m_pSquadSystem && m_pSquadSystem->IsEnabled() && m_pSquadSystem->GetMembersCount() != 0)
		static const float clr[] = { 1,1,1,1 };

		SSquad& squad = g_pControlSystem->pSquadSystem->GetSquadFromMember(pDudePlayer, 1);
		//if (!squad.isPlayerLeader())
		//	squad = g_pControlSystem->pSquadSystem->GetSquadFromLeader(pDudePlayer);

		if (squad.isPlayerLeader() || squad.GetMembersCount() > 0)
		{
			if (m_pControlledActor->GetHealth() < 0.1f && pDudePlayer->GetHealth() > 0)
			{
				static float timer = 1.0f;

				if (timer != 0)
				{
					timer -= gEnv->pTimer->GetFrameTime();
					if (timer < 0)
						timer = 0;
				}

				if (timer == 0)
				{
					SMember& member = squad.GetMemberAlive();
					if (squad.GetActor(member.actorId))
					{
						CActor* pActor = static_cast<CActor*>(squad.GetActor(member.actorId));

						SetActor(pActor);
						squad.SetLeader(pActor);

						timer = 1.0f;
						CryLogAlways("Squad Leader Dude is Alive, controlled actor is dead --->> select the next alive actor from squad members, setup new leader and control him");
					}
				}
				//gEnv->pRenderer->Draw2dLabel(40, 240, 3, clr, false, "TIMER1 %1.f", timer);
			}

			if (pDudePlayer->GetHealth() < 0.1f && m_pControlledActor->GetHealth() > 0)
			{
				pDudePlayer->StandUp();
				pDudePlayer->Revive(true);
				pDudePlayer->SetHealth(200);

				if (!pDudePlayer->IsThirdPerson())
					pDudePlayer->ToggleThirdPerson();

				pDudePlayer->GetActorStats()->isHidden = true;
				pDudePlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

				g_pGame->GetHUD()->RebootHUD();
				g_pGame->GetHUD()->GetScopes()->m_animBinoculars.Reload();
				//g_pGame->GetHUD()->PlayerIdSet(pNomadPlayer->GetEntityId());

				//g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pNomadPlayer, "Binoculars", false,false,false);

				IScriptTable* pTable = m_pControlledActor->GetEntity()->GetScriptTable();
				if (pTable)
					Script::CallMethod(pTable, "Event_Kill");
				CryLogAlways("In squad dude is Dead, controlled actor is Alive --->> revive dude, kill controlled actor");
			}
		}
		else if (squad.isPlayerMember() && squad.GetMembersCount() > 0)
		{
			if (m_pControlledActor->GetHealth() < 0.1f && pDudePlayer->GetHealth() > 0)
			{
				static float timer = 1.0f;

				if (timer != 0)
				{
					timer -= gEnv->pTimer->GetFrameTime();
					if (timer < 0)
						timer = 0;
				}

				if (timer == 0)
				{
					SMember& member = squad.GetMemberAlive();
					if (squad.GetActor(member.actorId))
					{
						CActor* pActor = static_cast<CActor*>(squad.GetActor(member.actorId));

						SetActor(pActor);
						//squad.SetLeader(pActor);

						timer = 1.0f;
						CryLogAlways("Squad Member Dude is Alive, controlled actor is dead --->> select the next alive actor from squad members and control him");
					}
				}
				//gEnv->pRenderer->Draw2dLabel(40, 240, 3, clr, false, "TIMER1 %1.f", timer);
			}
		}
		else
		{
			if (m_pControlledActor->GetHealth() < 0.1f && pDudePlayer->GetHealth() > 0.1f) //--> if not squad controlled is dead, player is alive
			{
				if (gEnv->bServer)
				{
					HitInfo hitInfo(pDudePlayer->GetEntityId(), pDudePlayer->GetEntityId(), pDudePlayer->GetEntityId(), -1, 0, 0, -1, 0, ZERO, ZERO, ZERO);
					hitInfo.SetDamage(15000);

					if (CGameRules* pGameRules = g_pGame->GetGameRules())
						pGameRules->ServerHit(hitInfo);
				}
				CryLogAlways("In NOT squad dude is Alive, actor is Dead --->> kill player");

				//This block fix the Dude stucking in controlled actor after death
				{
					ToggleDudeBeam(false);

					Vec3 vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();
					vControlledPos.z += 15.f;

					Matrix34 nomadMat34 = pDudePlayer->GetEntity()->GetWorldTM();
					nomadMat34.SetTranslation(vControlledPos);
					pDudePlayer->GetEntity()->SetWorldTM(nomadMat34);
				}
			}
			else if (pDudePlayer->GetHealth() < 0.1f && m_pControlledActor->GetHealth() > 0.1f)// --> not squad Dude is dead, controlled is alive
			{
				IScriptTable* pTable = m_pControlledActor->GetEntity()->GetScriptTable();
				if (pTable)
					Script::CallMethod(pTable, "Event_Kill");

				//This block fix the player stucking in controlled actor after death
				{
					ToggleDudeBeam(false);

					Vec3 vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();
					vControlledPos.z += 15.f;

					Matrix34 nomadMat34 = pDudePlayer->GetEntity()->GetWorldTM();
					nomadMat34.SetTranslation(vControlledPos);
					pDudePlayer->GetEntity()->SetWorldTM(nomadMat34);
				}
				CryLogAlways("In NOT squad dude is Dead, controlled actor is Alive --->> kill controlled actor");
			}
		}
	}
	else
	{
		if (m_pControlledActor->GetHealth() < 0.1f && pDudePlayer->GetHealth() > 0.1f) //--> if not squad controlled is dead, player is alive
		{
			m_pControlledActor->StandUp();
			m_pControlledActor->Revive(true);
			m_pControlledActor->SetHealth(m_pControlledActor->GetMaxHealth());

			if (gEnv->bServer)
			{
				//if (m_pControlledActor->GetEntity()->GetAI() && m_pControlledActor->GetEntity()->GetAI()->GetAIType() != AIOBJECT_PLAYER)
				SetActorAI(m_pControlledActor, true);
			}

			CryLogAlways("In Tutorial Mode dude is Alive, actor is dead --->> revive actor");
		}
	}
}

void SControlClient::FullSerialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network && !gEnv->bEditor)
	{
		ser.BeginGroup("SControlClientFull");
		if (ser.IsWriting())
		{
			m_generic.Serialize(ser);
			m_nakedAlien.Serialize(ser);
			m_trooper.Serialize(ser);
			m_scout.Serialize(ser);
			m_hunter.Serialize(ser);

			ser.Value("g_pControlSystem_bEnabled", g_pControlSystem->isEnabled);

			ser.EnumValue("m_lastDudeNanoMode", m_lastDudeNanoMode, NANOMODE_SPEED, NANOMODE_LAST);
			ser.Value("m_lastDudeRotation", m_lastDudeRotation);
			ser.Value("m_vlastDudePosition", m_lastDudePosition);
			ser.Value("m_lastDudeSpecies", m_lastDudeSpecies);

			ser.Value("m_fStoredTime", m_storedTime);
			ser.Value("m_fLastPlayerEnergy", m_lastDudeSuitEnergy);
			ser.Value("m_fHits", m_meleeHits);
			ser.Value("m_PlayerLastPos", m_dudeLastPos);
			ser.Value("m_CamViewDir", m_camViewDir);
			ser.Value("m_CamPos", m_camPos);
			ser.Value("m_CamViewCoords", m_camViewCoords);
			ser.Value("m_vCrosshairPos", m_crosshairPos);
			ser.Value("m_bBeamPlayer", m_mustBeamDude);
			ser.Value("m_bHidePlayer", m_mustHideDude);

			ser.Value("m_bEnableActions", m_canProceedActions);

			//ser.Value("m_bTutorialMode", m_bTutorialMode);

			//ser.Value("m_isTargetAI", m_isTargetAI);
			ser.Value("m_CrosshairTargetId", m_crosshairTargetId);
			ser.Value("m_LastCrosshairTargetId", m_lastCrosshairTargetId);

			EntityId controlledId = 0;
			if (m_pControlledActor)
				controlledId = m_pControlledActor->GetEntityId();

			ser.Value("m_pControlledActorId", controlledId);
		}
		else if (ser.IsReading())
		{
			m_generic.Serialize(ser);
			m_generic.moveDir.zero();

			bool	bEnabled = false;
			EntityId _controlledId;

			ser.Value("m_pControlledActorId", _controlledId);
			ser.Value("g_pControlSystem_bEnabled", bEnabled);

			if (bEnabled)
			{
				//Because the Control System already serialized and pick up saved values;
				if (CActor* pNewControlledActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(_controlledId)))
				{
					//CPlayer::TAlienInterferenceParams lastInterferenceParams = pDude->m_interferenceParams;
					//pDude->m_interferenceParams.clear();

					//Player save game -->> Player load game -->> setup default values from save game -->> ControlSystem interpret this values as defaults
					{
						ser.EnumValue("m_lastDudeNanoMode", m_lastDudeNanoMode, NANOMODE_SPEED, NANOMODE_LAST);
						ser.Value("m_lastDudeRotation", m_lastDudeRotation);
						ser.Value("m_vlastDudePosition", m_lastDudePosition);
						ser.Value("m_lastDudeSpecies", m_lastDudeSpecies);

						CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
						if (pDude)
						{
							CNanoSuit* pSuit = pDude->GetNanoSuit();
							if (pSuit)
								pSuit->SetMode(m_lastDudeNanoMode);

							//pDude->GetEntity()->SetPos(m_vlastDudePosition);
							//pDude->SetViewRotation(m_lastDudeRotation);
							SetDudeSpecies(m_lastDudeSpecies);
						}
					}

					g_pControlSystem->Start();
					SetActor(pNewControlledActor);

					m_nakedAlien.Serialize(ser);
					m_trooper.Serialize(ser);
					m_scout.Serialize(ser);
					m_hunter.Serialize(ser);

					m_generic.cannotLookAt = false;

					ser.Value("m_fStoredTime", m_storedTime);

					ser.Value("m_fLastPlayerEnergy", m_lastDudeSuitEnergy);
					ser.Value("m_fHits", m_meleeHits);
					ser.Value("m_PlayerLastPos", m_dudeLastPos);
					ser.Value("m_CamViewDir", m_camViewDir);
					ser.Value("m_CamPos", m_camPos);
					ser.Value("m_CamViewCoords", m_camViewCoords);
					ser.Value("m_vCrosshairPos", m_crosshairPos);
					ser.Value("m_bBeamPlayer", m_mustBeamDude);
					ser.Value("m_bHidePlayer", m_mustHideDude);

					ser.Value("m_bEnableActions", m_canProceedActions);

					//ser.Value("m_bTutorialMode", m_bTutorialMode);

					//ser.Value("m_isTargetAI", m_isTargetAI);
					ser.Value("m_CrosshairTargetId", m_crosshairTargetId);
					ser.Value("m_LastCrosshairTargetId", m_lastCrosshairTargetId);

					//g_pControlSystem->Start();
				}
			}
			else
			{
				if (g_pControlSystem->GetEnabled())
				{
					g_pControlSystem->Stop();
				}
			}

			//CryLogAlways("Loaded m_bControlSystemEnabled in %d", int(benabled));
		}
		ser.EndGroup();

		/*if (m_pSquadSystem)
			m_pSquadSystem->FullSerialize(ser);*/
		if (g_pControlSystem->GetSquadSystem())
			g_pControlSystem->GetSquadSystem()->FullSerialize(ser);
	}
}

void SControlClient::ToggleTutorialMode(const bool mode)
{
	m_generic.isTutorialMode = mode;
}

bool SControlClient::GetTutorialMode() const
{
	return m_generic.isTutorialMode;
}

IMaterial* SControlClient::GetMaterial()
{
	if (m_pControlledActor)
	{
		if (IEntity* pEnt = m_pControlledActor->GetEntity())
		{
			if (IEntityRenderProxy* pProxy = (IEntityRenderProxy*)pEnt->GetProxy(ENTITY_PROXY_RENDER))
			{
				if (IMaterial* pMat = pProxy->GetRenderMaterial(0))
					return pMat;
			}
		}
	}
	return 0;
}

//EntityId SControlClient::GetClientId()
//{
//	return m_ClientId;
//}

//IActor* SControlClient::GetClientActor()
//{
//	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_ClientId);
//	if (pActor)
//		return pActor;
//
//	return 0;
//}
//
//void SControlClient::SetClientId(EntityId Id)
//{
//	m_ClientId = Id;
//}

void SControlClient::TestFunction()
{
}

void SControlClient::ApplyMovement(const Vec3& delta)
{
	m_generic.moveDir.x = clamp_tpl(m_generic.moveDir.x + delta.x, -1.0f, 1.0f);
	m_generic.moveDir.y = clamp_tpl(m_generic.moveDir.y + delta.y, -1.0f, 1.0f);
	m_generic.moveDir.z = clamp_tpl(m_generic.moveDir.z + delta.z, -1.0f, 1.0f);
}

void SControlClient::ResetParams()
{
	m_nakedAlien.Reset();
	m_trooper.Reset();
	m_scout.Reset();
	m_hunter.Reset();
	m_generic.Reset();
}

void SControlClient::Reset()
{
	if (m_pControlledActor)
	{
		if (m_pControlledActor->IsAlien())
		{
			CAlien* pAlien = static_cast<CAlien*>(m_pControlledActor);
			if (pAlien->GetAlienInput())
				pAlien->GetAlienInput()->deltaMovement.zero();
		}
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(m_pControlledActor);
			if (pPlayer->GetPlayerInput())
				pPlayer->GetPlayerInput()->Reset();
		}

		if (gEnv->pAISystem)
		{
			if (m_pControlledActor->GetEntity()->GetAI() && m_pControlledActor->GetEntity()->GetAI()->GetAIType() != AIOBJECT_PUPPET)
				SetActorAI(m_pControlledActor, false);
		}

		if (gEnv->bClient && gEnv->bMultiplayer)
			g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestSetOwner(), CGameRules::OwnerParams(0, m_pControlledActor->GetEntityId()), eRMI_ToServer);
		else
			m_pControlledActor->SetOwnerId(0);

		m_pControlledActor = 0;
	}

	SAFE_HUD_FUNC(SetWeaponName(""));
	//m_actions = ACTION_GYROSCOPE;//gyroscope is on by default
}

void  SControlClient::ToggleDudeHide(const bool bToggle)
{
	//Need server implementation

	CPlayer* pNomadActor = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pNomadActor)
	{
		SActorStats* pActorStats = pNomadActor->GetActorStats();
		if (pActorStats)
		{
			pActorStats->isHidden = bToggle;
			m_mustHideDude = bToggle;
			pNomadActor->GetGameObject()->SetAspectProfile(eEA_Physics, bToggle ? eAP_Spectator : eAP_Alive);
		}
	}
}

void SControlClient::ToggleDudeBeam(const bool bBeam)
{
	IActor* pNomadPlayer = g_pGame->GetIGameFramework()->GetClientActor();

	if (!pNomadPlayer)
		return;

	m_mustBeamDude = bBeam;
}

void SControlClient::SetDudeSpecies(const int species)
{
	if (IEntity* pPlayer = gEnv->pEntitySystem->GetEntity(g_pGame->GetIGameFramework()->GetClientActorId()))
	{
		if (IAIObject* pAI = pPlayer->GetAI())
		{
			AgentParameters playerParams = pAI->CastToIAIActor()->GetParameters();
			playerParams.m_nSpecies = species;

			if (pAI->CastToIAIActor())
				pAI->CastToIAIActor()->SetParameters(playerParams);
		}
	}
}

bool SControlClient::SpawnParticleEffect(const char* effectName)
{
	if (effectName)
	{
		IParticleEffect* pEffect = gEnv->p3DEngine->FindParticleEffect(effectName);
		if (pEffect && m_pControlledActor)
		{
			pEffect->Spawn(false, m_pControlledActor->GetEntity()->GetLocalTM());
			return true;
		}
	}

	return false;
}

bool SControlClient::PlayAnimAction(const char* action, bool looping)
{
	if (!m_pControlledActor)
		return false;
	if (!m_pControlledActor->GetAnimatedCharacter())
		return false;

	if (looping)
	{
		if (m_pControlledActor->GetAnimatedCharacter()->GetAnimationGraphState()->SetInput("Action", action))
			return true;
		else
			return false;
	}
	else
	{
		if (m_pControlledActor->GetAnimatedCharacter()->GetAnimationGraphState()->SetInput("Signal", action))
			return true;
		else
			return false;
	}
}

CWeapon* SControlClient::GetCurrentWeapon(CActor* pActor)
{
	if (!pActor)
		return 0;

	IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return 0;

	EntityId pItemId = pInventory->GetCurrentItem();
	if (!pItemId)
		return 0;

	IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pItemId);
	if (!pItem)
		return 0;

	CWeapon* pWeapon = static_cast<CWeapon*>(pItem->GetIWeapon());
	if (!pWeapon)
		return 0;

	return pWeapon;
}

IAIObject* SControlClient::GetCrosshairTargetAI()
{
	if (m_crosshairTargetId)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_crosshairTargetId);
		if (pEntity)
		{
			IAIObject* pAI = pEntity->GetAI();
			if (pAI)
				return pAI;
		}
	}

	return NULL;
}

void SControlClient::SetActorAI(IActor* pActor, bool bToPlayerAI)
{
	if (!pActor || !pActor->GetEntity())
		return;
	assert(pActor);
	assert(pActor->GetEntity());

	IScriptTable* pTable = pActor->GetEntity()->GetScriptTable();
	if (pTable)
	{
		Script::CallMethod(pTable, "RegAI", bToPlayerAI);
		// true - register actor as Player AI, false - register actor as normal AI puppet
	}
}

bool SControlClient::DoNextActor(CActor* pActor)
{
	if (pActor)
	{
		CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
		if (pDude)
		{
			//TODO: I may have fixed "Pure function error"
			if (gEnv->bMultiplayer && gEnv->bClient)
				g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestSetOwner(), CGameRules::OwnerParams(pDude->GetEntityId(), pActor->GetEntityId()), eRMI_ToServer);

			//Only on local client
			pDude->SetSlaveId(pActor->GetEntityId());
			pActor->SetOwnerId(pDude->GetEntityId());

			pDude->GetEntity()->SetRotation(pActor->GetEntity()->GetRotation());
		}

		IAIObject* pControlledAI = pActor->GetEntity()->GetAI();
		assert(pControlledAI);
		if (pControlledAI)
		{
			if (pControlledAI->CastToIAIActor())
			{
				//Save AI values
				AgentParameters params = pControlledAI->CastToIAIActor()->GetParameters();
				const int species = params.m_nSpecies;
				SetDudeSpecies(species);

				//Re-register controlled actor in AI System as AI Player
				if (pControlledAI->GetAIType() == AIOBJECT_PUPPET)
					SetActorAI(pActor, true);

				//Restore AI values to new ai pointer
				IAIObject* pControlledAI = pActor->GetEntity()->GetAI();
				assert(pControlledAI);
				if (pControlledAI)
				{
					//CryLogAlways("Restore AI values to new ai pointer");
					assert(pControlledAI->CastToIAIActor());
					if (pControlledAI->CastToIAIActor())
					{
						pControlledAI->CastToIAIActor()->SetParameters(params);
						//CryLogAlways("Restoring successful");
					}
				}
			}

			SSquad& squad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pActor, 1);
			if (squad.GetLeader())
				squad.OnPlayerAdded();
		}

		if (CWeapon* pWeapon = GetCurrentWeapon(pActor))
		{
			if (pWeapon->IsFiring())
				pWeapon->StopFire();
		}
		return true;
	}
	return false;
}

bool SControlClient::DoPrevActor(CActor* pActor)
{
	if (pActor)
	{
		if (gEnv->bClient)
		{
			if (gEnv->bMultiplayer)
				g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestSetOwner(), CGameRules::OwnerParams(0, pActor->GetEntityId()), eRMI_ToServer);
			//else
			//	pActor->SetOwnerId(0);

			pActor->GetGameObject()->InvokeRMI(CAlien::SvRequestSetMove(), CAlien::SMovementParams(Vec3(0, 1, 0), Vec3(0, 0, 0), pActor->GetEntity()->GetWorldPos()), eRMI_ToServer);
		}

		if (CWeapon* pWeapon = GetCurrentWeapon(pActor))
		{
			if (pWeapon->IsFiring())
				pWeapon->StopFire();
		}

		if (IAIObject* pAI = pActor->GetEntity()->GetAI())
		{
			if (pAI->GetAIType() == AIOBJECT_PLAYER)
				SetActorAI(pActor, false);
		}
		return true;
	}
	return false;
}

//bool SControlClient::OnShockwaveCreated(float reloadTimer)
//{
//	if (GetActorClassName() == "PlayerTrooper" || GetActorClassName() == "Trooper")
//		m_trooper.shockwaveTimer = reloadTimer;
//
//	return true;
//}

void SControlClient::SendPipeToAIGroup(const char* name, int groupId, bool useInComm, Vec3 refPoint)
{
	CPlayer* pNomadPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());

	CActor* pControlActor = m_pControlledActor;
	IAIObject* pControlAI = pControlActor->GetEntity()->GetAI();

	if (!pControlActor || !pNomadPlayer || !pControlAI)
		return;

	const bool hasRefPoint = refPoint != Vec3(0, 0, 0);

	IEntityItPtr pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	if (pEntityIterator && name)
	{
		pEntityIterator->MoveFirst();
		IEntity* pEntity = NULL;

		while (!pEntityIterator->IsEnd())
		{
			pEntity = pEntityIterator->Next();

			const bool isNotPlayer = pEntity != m_pControlledActor->GetEntity() &&
				pEntity != pNomadPlayer->GetEntity();

			if (pEntity && isNotPlayer)
			{
				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
				IAIObject* pEntityAI = pEntity->GetAI();

				if (pEntityAI && pActor)
				{
					IPipeUser* pPipeUser = pEntityAI->CastToIPipeUser();

					IAIActor* pAIActor = pEntityAI->CastToIAIActor();
					IAIActor* pControlAIAct = pControlAI->CastToIAIActor();

					IUnknownProxy* pAIProxy = pEntityAI->GetProxy();

					if (pAIActor && pControlAIAct && pPipeUser && pAIProxy)
					{
						//CryLogAlways("Actor name %s, groupId %.f", string(pEntity->GetName()), float(pAIActor->GetParameters().m_nGroup));
						//CryLogAlways("Player trooper, groupId %.f", string(pControlAI->GetEntity()->GetName()), float(pControlAIAct->GetParameters().m_nGroup));

						int iAlertness = pAIProxy->GetAlertnessState(); // 0 - relaxed, 1 - searching, 2 - in combat

						AgentParameters controlAIParams = pControlAIAct->GetParameters();
						AgentParameters entityAIParams = pAIActor->GetParameters();

						if (groupId == entityAIParams.m_nGroup && iAlertness <= 1)
						{
							if (hasRefPoint && GetCrosshairTargetAI())
								pPipeUser->SetRefPointPos(GetCrosshairTargetAI()->GetEntity()->GetWorldPos());

							if (useInComm)
							{
								float distance = (pControlActor->GetEntity()->GetWorldPos() - pEntity->GetWorldPos()).GetLength();

								if (distance <= controlAIParams.m_fCommRange)
									pPipeUser->SelectPipe(0, name, pEntityAI);
							}
							else
								pPipeUser->SelectPipe(0, name, pEntityAI);
						}
					}
				}
			}
		}
	}
}

void SControlClient::SetActor(CActor* pNewActor) //Œ—“Œ–Œ∆ÕŒ!! √Œ¬ÕŒ Œƒ!!!
{
	assert(pNewActor);
	if (!pNewActor)
		return;

	CActor* pCurrentActor = m_pControlledActor;
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());

	SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0));
	SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(false));
	m_generic.zoomScale = 0;
	m_generic.moveDir.zero();

	m_trooper.isBinocular = false;
	m_generic.isUsingBinocular = false;
	m_generic.isAiming = false;

	m_lookRequest.ClearAimTarget();

	//This cannot fix the bug
	//const CGameActions& rGameActions = g_pGame->Actions();
	//pDude->GetPlayerInput()->Reset();
	//OnAction(rGameActions.moveforward, eAAM_OnRelease, 0);
	//OnAction(rGameActions.moveback, eAAM_OnRelease, 0);
	//OnAction(rGameActions.moveleft, eAAM_OnRelease, 0);
	//OnAction(rGameActions.moveright, eAAM_OnRelease, 0);

	if (!pCurrentActor)
	{
		//First Actor
		m_pControlledActor = pNewActor;
		if (m_pControlledActor)
		{
			//Take New Actor under control
			DoNextActor(m_pControlledActor);
		}
		else
		{
			g_pControlSystem->Stop();
			return;
		}
	}
	else if (pCurrentActor && pCurrentActor != pNewActor)
	{
		DoPrevActor(m_pControlledActor);//The previous actor after controlling must return to normal state

		m_pControlledActor = pNewActor;
		//Take New Actor under control
		DoNextActor(m_pControlledActor);
	}
	m_pAbilitiesSystem->AddAbilityOwner(m_pControlledActor);
	CAbilityOwner& abilityOwner = m_pAbilitiesSystem->GetAbilityOwner(m_pControlledActor->GetEntityId());

	//New Health hud that add the abilities
	SetAmmoHealthHUD(m_pControlledActor, "Libs/UI/HUD_AmmoHealthEnergyAbilities.gfx");

	CAlien* pCurrentAlien = NULL;
	if (m_pControlledActor && m_pControlledActor->IsAlien())
		pCurrentAlien = static_cast<CAlien*>(m_pControlledActor);

	if (pCurrentAlien)
	{
		pCurrentAlien->GetAlienInput()->movementVector.Set(0, 0, 0);

		if (GetActorClassName() == "Alien")//Not useful in update but necessary
		{
			//m_generic.canShoot = false;
			m_nakedAlien.Reset();

			g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Alien");
		}
		else if (GetActorClassName() == "Scout")
		{
			CScout* pCurrentAlien = static_cast<CScout*>(m_pControlledActor);
			pCurrentAlien->EnableSearchBeam(false);
			pCurrentAlien->m_params.forceView = 49.f;

			m_scout.Reset();
			m_generic.canShoot = true;

			g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Scout");
		}
		else if (GetActorClassName() == "PlayerTrooper")
		{
			m_trooper.Reset();
			m_generic.canShoot = true;

			g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Trooper");
		}
		else if (GetActorClassName() == "Hunter")
		{
			m_hunter.Reset();
			m_generic.canShoot = true;

			g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Hunter");
		}
		else
		{
			m_generic.canShoot = true;
		}
	}
	m_pAbilitiesSystem->ReloadHUD();
	m_pAbilitiesSystem->UpdateHUD();
}

void SControlClient::UpdateView(SViewParams& viewParams)
{
	if (!m_pControlledActor)
		return;

	viewParams.position = m_pControlledActor->GetEntity()->GetWorldPos();

	CPlayer* pActorPlayer = static_cast<CPlayer*>(m_pControlledActor);
	CAlien* pAlien = static_cast<CAlien*>(m_pControlledActor);

	if (m_pControlledActor)
	{
		static float currentFov = -1;

		//Copied from ViewThirdPerson() in PlayerView.cpp
		static Vec3 target;
		static Vec3 current;
		if (target)
		{
			current = target;
			Interpolate(current, target, 5.0f, viewParams.frameTime);
		}

		// make sure we don't clip through stuff that much
		Vec3 offsetX(0, 0, 0);
		Vec3 offsetY(0, 0, 0);
		Vec3 offsetZ(0, 0, 0);

		if (m_pControlledActor->IsAlien())
		{
			Matrix33 alienWorldMtx(pAlien->GetGameObject()->GetEntity()->GetWorldTM());

			if (GetActorClassName() == "Hunter")
			{
				target(g_pGameCVars->ctrl_HunterTargetx, g_pGameCVars->ctrl_HunterTargety, g_pGameCVars->ctrl_HunterTargetz);
				offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1() * g_pGameCVars->ctrl_htForwardOffset;
				currentFov = g_pGameCVars->ctrl_htFov;
			}
			else if (GetActorClassName() == "Scout")
			{
				offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y;//Used by all aliens in this mod
				target(g_pGameCVars->ctrl_ScoutTargetx, g_pGameCVars->ctrl_ScoutTargety, g_pGameCVars->ctrl_ScoutTargetz);
				currentFov = g_pGameCVars->ctrl_scFov;
			}
			else if (GetActorClassName() == "PlayerTrooper")
			{
				if (!m_pAbilitiesSystem->trooper.isCeiling)
					target(g_pGameCVars->ctrl_TrooperTargetx, g_pGameCVars->ctrl_TrooperTargety, g_pGameCVars->ctrl_TrooperTargetz);
				else
				{
					target(g_pGameCVars->ctrl_TrooperTargetx, g_pGameCVars->ctrl_TrooperTargety, g_pGameCVars->ctrl_TrooperTargetz - 2.f);
				}
				offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y;//Used by all aliens in this mod
				currentFov = g_pGameCVars->ctrl_trFov;
			}
			else if (GetActorClassName() == "Alien")
			{
				target(g_pGameCVars->ctrl_AlienTargetx, g_pGameCVars->ctrl_AlienTargety, g_pGameCVars->ctrl_AlienTargetz);
				offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y;//Used by all aliens in this mod
				currentFov = g_pGameCVars->ctrl_alFov;
			}
		}

		offsetX = pAlien->GetViewRotation().GetColumn0() * current.x;
		offsetZ = pAlien->GetViewRotation().GetColumn2() * current.z;

		//Get skip entities
		ray_hit hit;
		IPhysicalEntity* pSkipEntities[10];
		int nSkip = 0;
		IItem* pItem = pAlien->GetCurrentItem();
		if (pItem)
		{
			CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
			if (pWeapon)
				nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
		}

		const float oldLen = offsetY.len();
		Vec3 start(0, 0, 0);

		// Ray cast to camera with offset position to check colliding
		Vec3 eyeOffsetView = pAlien->GetStanceInfo(pAlien->GetStance())->viewOffset;
		start = (pAlien->GetAlienBaseMtx() * eyeOffsetView + viewParams.position + offsetX);// +offsetZ;// + offsetX;// +offsetZ;

		static float wallSafeDistance = 0.3f; // how far to keep camera from walls

		primitives::sphere sphere;
		sphere.center = start;
		sphere.r = wallSafeDistance;

		geom_contact* pContact = 0;
		float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
			&pContact, 0, rwi_stop_at_pierceable, 0, 0, 0, pSkipEntities, nSkip);

		if (hitDist > 0 && pContact && !m_pAbilitiesSystem->trooper.isCeiling)
		{
			offsetY = pContact->pt - start;
			if (offsetY.len() > 0.3f)
			{
				offsetY -= offsetY.GetNormalized() * 0.3f;
			}
			current.y = current.y * (hitDist / oldLen);
		}

		if (m_pAbilitiesSystem->trooper.isCeiling)
		{
			Vec3 pos = pAlien->GetLocalEyePos() + pAlien->GetEntity()->GetWorldPos();
			pos.z += 0.3f;
			viewParams.position = pos;
		}
		else if (m_pAbilitiesSystem->scout.IsSearch)
		{
			if (pAlien->m_searchbeam.itemId != NULL)
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pAlien->m_searchbeam.itemId);
				if (pEntity)
				{
					Vec3 pos = pEntity->GetWorldTM().GetTranslation();
					pos.z -= 1.5f;

					viewParams.position = pos;
				}
			}
		}
		else
		{
			viewParams.position += (offsetX + offsetY + offsetZ);
		}

		if (!m_generic.isUsingBinocular)
		{
			viewParams.fov = currentFov * gf_PI / 180.0f;
			m_currentFov = currentFov;
		}
		else
		{
			viewParams.fov = m_currentFov * gf_PI / 180.0f;
		}

		//Old
		/*Vec3 pActorWorldPos = m_pAlien->GetEntity()->GetWorldPos();
		string m_pClassName = m_pAlien->GetEntity()->GetClass()->GetName();

		if (m_pClassName == "Alien")
		{
			viewParams.fov = g_pGameCVars->ctrl_alFov * gf_PI / 180.0f;
			Matrix33 viewMtx(m_pAlien->GetAlienViewMtx());
			viewParams.position = pActorWorldPos +(viewMtx.GetColumn(1) * -m_pDistance + viewMtx.GetColumn(2) * +m_pHeight);
		}
		else if (m_pClassName == "Scout")
		{
			viewParams.fov = g_pGameCVars->ctrl_scFov * gf_PI / 180.0f;
			Matrix33 viewMtx(m_pAlien->GetAlienViewMtx());
			viewParams.position = pActorWorldPos + (viewMtx.GetColumn(1) * -m_pDistance + viewMtx.GetColumn(2) * +m_pHeight);
		}
		else if (m_pClassName == "PlayerTrooper")
		{
			viewParams.fov = g_pGameCVars->ctrl_trFov * gf_PI / 180.0f;
			Vec3 CamViewdir = gEnv->pSystem->GetViewCamera().GetViewdir() * -m_pDistance;
			Matrix33 viewMtx(m_pAlien->GetGameObject()->GetEntity()->GetWorldTM());
			viewParams.position = pActorWorldPos + (viewMtx.GetColumn(2) * m_pHeight) + CamViewdir;
		}
		else if (m_pClassName == "Hunter")
		{
			float m_pForwardOffset = g_pGameCVars->ctrl_htForwardOffset;
			viewParams.fov = g_pGameCVars->ctrl_htFov * gf_PI / 180.0f;
			Vec3 CamViewdir = gEnv->pSystem->GetViewCamera().GetViewdir() * -m_pDistance;
			Matrix33 viewMtx(m_pAlien->GetGameObject()->GetEntity()->GetWorldTM());
			viewParams.position = pActorWorldPos + (viewMtx.GetColumn(2) * m_pHeight + viewMtx.GetColumn(1) * m_pForwardOffset) + CamViewdir;
		}*/
	}
}

void SControlClient::StoreCurrTime()
{
	m_storedTime = gEnv->pTimer->GetCurrTime();
}

bool SControlClient::CheckPassTime(float passedSeconds)
{
	if (gEnv->pTimer->GetCurrTime() - m_storedTime > passedSeconds)
	{
		return true;
	}
	else
		return false;
}

void SControlClient::SubEnergy(const float subtractValue)
{
	CAlien* pAlien = static_cast<CAlien*>(m_pControlledActor);
	if (!pAlien && !m_pControlledActor->IsAlien())
		return;
	pAlien->SetAlienEnergy(pAlien->GetAlienEnergy() - subtractValue);
}

IEntity* SControlClient::GetMeleeTarget()
{
	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_meleeTargetId))
		return pEntity;
	return 0;
}

IEntity* SControlClient::GetLastCrosshairEntity()
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_lastCrosshairTargetId);
	return pEntity;
}

IEntity* SControlClient::GetCrosshairEntity()
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_crosshairTargetId);
	return pEntity;
}

IEntity* SControlClient::GetFireTarget()
{
	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_fireTargetId))
		return pEntity;
	return nullptr;
}

void SControlClient::ShowClientHit(const HitInfo& hitInfo)
{
	if (m_pControlledActor) // local client side
	{
		//On screen effects
		{
			if (GetActorClassName() == "PlayerTrooper" || GetActorClassName() == "Alien")
			{
				IMaterialEffects* pMaterialEffects = gEnv->pGame->GetIGameFramework()->GetIMaterialEffects();
				TMFXEffectId id = pMaterialEffects->GetEffectIdByName("player_fx", "player_damage_armormode");

				SMFXRunTimeEffectParams params;

				params.pos = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldPos();
				params.soundSemantic = eSoundSemantic_HUD;

				pMaterialEffects->ExecuteEffect(id, params);
			}
		}

		//On hud effects
		{
			Vec3 dir(ZERO);

			if (IEntity* pShooter = gEnv->pEntitySystem->GetEntity(hitInfo.shooterId))
			{
				dir = (m_pControlledActor->GetEntity()->GetWorldPos() - pShooter->GetWorldPos());
				dir.NormalizeSafe();
			}

			SAFE_HUD_FUNC(IndicateDamage(NULL, dir, false));
			SAFE_HUD_FUNC(ShowTargettingAI(hitInfo.shooterId));
		}

		//Scout & hunter shields
		{
			if (GetActorClassName() == "Scout" || GetActorClassName() == "Hunter")
			{
			}
		}
	}
}

void SControlClient::ShowHitIndicator(const HitInfo& hitInfo)
{
}

void SAlienControlParams::Serialize(TSerialize ser)
{
	ser.BeginGroup("SAlienControlParams");
	ser.Value("m_fAlienSpeedMult", speedMult);
	ser.Value("m_fAlienAimMult", aimSpeedMult);
	ser.EndGroup();
}

void SScoutControlParams::Serialize(TSerialize ser)
{
	ser.BeginGroup("SScoutControlParams");
	ser.Value("m_fScoutSpeedMult", speedMult);
	ser.Value("m_bScoutAllowDodge", canDodge);
	ser.Value("m_bScoutDoDodge", doDodge);
	ser.EndGroup();
}

void STrooperControlParams::Serialize(TSerialize ser)
{
	ser.BeginGroup("STrooperControlParams");
	SER_VALUE(speedMult);
	SER_VALUE(isBinocular);
	SER_VALUE(canDodge);
	SER_VALUE(canJumpMelee);
	SER_VALUE(doJumpMelee);
	SER_VALUE(canMelee);
	SER_VALUE(doMelee);
	ser.EndGroup();
}

void SHunterControlParams::Serialize(TSerialize ser)
{
	ser.BeginGroup("SHunterControlParams");
	ser.EndGroup();
}

void SGenericControlParams::Serialize(TSerialize ser)
{
	SER_VALUE(canShoot);
	SER_VALUE(canUseCenterAbility);

	SER_VALUE(fireTargetIsFriend);
	//SER_VALUE(m_bDisableMovement);
	SER_VALUE(cannotLookAt);
	SER_VALUE(isTargetHaveAI);
	SER_VALUE(isTutorialMode);
	SER_VALUE(isUsingBinocular);

	SER_VALUE(zoomScale);
	SER_VALUE(jumpCount);

	SER_VALUE(fSoundRestTimer);

	//SER_VALUE(m_MovementDir);
	//ser.Value("m_bEnableControl", m_bEnableControl);// not implemented
}

void SControlClient::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}