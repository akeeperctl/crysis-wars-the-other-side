#include <StdAfx.h>

#include "HUD/HUD.h"
#include "HUD/HUDScopes.h"
#include "HUD/HUDCrosshair.h"
#include "HUD/HUDSilhouettes.h"

#include "HUD/GameFlashAnimation.h"
#include "HUD/GameFlashLogic.h"
#include "Menus/FlashMenuObject.h"

#include "GameActions.h"
#include "GameCvars.h"
#include "GameUtils.h"

#include "Fists.h"
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
#include "TheOtherSide/Conqueror/ConquerorSystem.h"

#include "TheOtherSide/Helpers/TOS_HUD.h"
#include "TheOtherSide/Helpers/TOS_AI.h"
//#include "SquadSystem.h"
//#include "AbilitiesSystem.h"

#include "IPlayerInput.h"

//SControlClient* g_pControlSystem = new SControlClient();

namespace
{
    bool IsFriendlyEntity(IEntity* pEntity, const IActor* pTarget)
    {
        //Only for actors (not vehicles)
        if (pEntity && pEntity->GetAI() && pTarget)
        {
            if (!pEntity->GetAI()->IsHostile(pTarget->GetEntity()->GetAI(), false))
                return true;
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

TActionHandler<CControlClient> CControlClient::s_actionHandler;

CControlClient::CControlClient(CPlayer* _player)
    : m_isDebugLog(false),
      m_lastSpectatorMode(0),
      m_mpLastControlledId(0),
      m_finalFireTargetPos(0, 0, 0),
      m_lastDudeNanoMode(NANOMODE_DEFENSE),
      m_lastDudeRotation(ZERO),
      m_lastDudePosition(ZERO),
      m_lastDudeSpecies(0),
      m_lastDudeSuitEnergy(0.0f),
      m_storedTime(0.0f),
      m_meleeHits(0.0f),

      m_currentFov(60),
      m_crosshairPos(0, 0, 0),
      m_mustBeamDude(false),
      m_mustHideDude(false),
      m_canProceedActions(false),

      m_isHitListener(false),
      m_pLocalDude(_player),
      m_pControlledActor(nullptr),
      m_fireTargetId(0),

      m_meleeTargetId(0),
      m_crosshairTargetId(0),
      m_lastCrosshairTargetId(0),
      m_scoutAimTargetId(0),
      m_pAbilitiesSystem(nullptr)
{
    m_pAbilitiesSystem = g_pControlSystem->m_pAbilitiesSystem;

    //CGameRules* pGR = g_pGame->GetGameRules();
    //if (pGR && !m_isHitListener)
    //{
    //	m_isHitListener = true;
    //	pGR->AddHitListener(this);
    //}

    ResetParams();
}

CControlClient::~CControlClient()
{
    m_pAbilitiesSystem = nullptr;
}

//
void CControlClient::OnHit(const HitInfo& hitInfo)
{
}

void CControlClient::OnExplosion(const ExplosionInfo& expInfo)
{
}

void CControlClient::OnServerExplosion(const ExplosionInfo& expInfo)
{
}

void CControlClient::InitDudeToControl(const bool bToLink)
{
    if (m_pLocalDude)
    {
        //CPlayer::TAlienInterferenceParams lastInterferenceParams;

        CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();

        //Fix the non-resetted Dude player movement after controlling the actor;
        if (m_pLocalDude->GetPlayerInput())
            m_pLocalDude->GetPlayerInput()->Reset();

        //before link to the new actor
        if (bToLink)
        {
            m_lastDudePosition = m_pLocalDude->GetEntity()->GetWorldPos();
            m_lastDudeRotation = m_pLocalDude->GetViewRotation();

            if (gEnv->bServer)
            {
                IAIObject* pAI = m_pLocalDude->GetEntity()->GetAI();
                if (pAI)
                {
                    if (pAI->CastToIAIActor())
                    {
                        m_lastDudeSpecies = pAI->CastToIAIActor()->GetParameters().m_nSpecies;
                        //CryLogAlways("SControlClient::PrepareDude -->> save player species");
                    }

                    pAI->Event(AIEVENT_DISABLE, nullptr);
                }
            }

            m_pLocalDude->ResetScreenFX();

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
            //lastInterferenceParams = m_pLocalDude->m_interferenceParams;
            m_pLocalDude->ClearInterference();

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
                SActorParams* pParams = m_pLocalDude->GetActorParams();

                if (m_pLocalDude->GetHealth() > 0)
                {
                    m_pLocalDude->GetEntity()->SetPos(m_lastDudePosition);

                    //may be bugged, not checked at 19.12.2020 0:14
                    m_pLocalDude->SetViewRotation(m_lastDudeRotation);
                }


                m_pLocalDude->SetSlaveId(NULL);
                //m_pLocalDude->m_interferenceParams = lastInterferenceParams;
                m_pLocalDude->InitInterference();
                m_pLocalDude->ResetScreenFX();

                if (m_pLocalDude->IsThirdPerson())
                    m_pLocalDude->ToggleThirdPerson();

                SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(false))
                SAFE_HUD_FUNC(SetWeaponName(""))

                g_pControlSystem->GetSquadSystem()->AnySquadClientLeft();

                auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(m_pLocalDude, true);
                if (pSquad && pSquad->GetLeader() != nullptr)
                    pSquad->OnPlayerAdded();

                //Clean the OnUseData from player .lua script
                IScriptTable* pTable = m_pLocalDude->GetEntity()->GetScriptTable();
                if (pTable)
                {
                    const ScriptAnyValue value = 0;
                    Script::CallMethod(pTable, "SetOnUseData", value, value);
                }

                if (pSuit)
                {
                    if (m_pLocalDude->GetHealth() > 0)
                    {
                        pSuit->Reset(m_pLocalDude);

                        pSuit->SetModeDefect(NANOMODE_CLOAK, false);
                        pSuit->SetModeDefect(NANOMODE_SPEED, false);
                        pSuit->SetModeDefect(NANOMODE_STRENGTH, false);

                        pSuit->ActivateMode(NANOMODE_CLOAK, true);
                        pSuit->ActivateMode(NANOMODE_SPEED, true);
                        pSuit->ActivateMode(NANOMODE_STRENGTH, true);

                        pSuit->SetSuitEnergy(m_lastDudeSuitEnergy);
                        pSuit->SetMode(m_lastDudeNanoMode);
                    }

                    if (g_pGame->GetHUD())
                    {
                        SetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx");
                        SetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx");
                        m_animScoutFlyInterface.Unload();

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
                        case NANOMODE_CLOAK:
                            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Cloak");
                            break;
                        case NANOMODE_INVULNERABILITY:
                        case NANOMODE_DEFENSE_HIT_REACTION:
                        case NANOMODE_LAST:
                            break;
                        }
                    }

                    pParams->vLimitRangeH = 0;
                    pParams->vLimitRangeV = pParams->vLimitRangeVDown = pParams->vLimitRangeVUp = 0;
                }

                if (gEnv->bServer)
                {
                    IAIObject* pAI = m_pLocalDude->GetEntity()->GetAI();
                    if (pAI)
                    {
                        pAI->Event(AIEVENT_ENABLE, nullptr);
                        SetDudeSpecies(m_lastDudeSpecies);
                    }
                }
            }
        }
    }
}

void CControlClient::SetInventoryHUD(IActor* pActor, const char* file)
{
    const auto pHUD = g_pGame->GetHUD();
    if (!pHUD || !pActor)
        return;

    pHUD->m_animWeaponSelection.Unload();
    pHUD->m_animWeaponSelection.Load(file, eFD_Right, eFAF_Visible | eFAF_ThisHandler);

    TOS_HUD::ShowInventory(pActor, "null", "null");
}

void CControlClient::SetAmmoHealthHUD(IActor* pActor, const char* file)
{
    if (!g_pGame->GetHUD())
        return;

    //Health, Energy, Ammo
    g_pGame->GetHUD()->m_animPlayerStats.Unload();
    g_pGame->GetHUD()->m_animPlayerStats.Load(file, eFD_Right, eFAF_Visible | eFAF_ThisHandler);

    const int iHealth = (pActor->GetHealth() / pActor->GetMaxHealth()) * 100 + 1;
    float fEnergy = 0;

    if (auto* pNewActor = dynamic_cast<CActor*>(pActor))
    {
        if (pNewActor->IsAlien())
        {
            auto* pAlien = dynamic_cast<CAlien*>(pNewActor);
            fEnergy = (pAlien->GetAlienEnergy() / pAlien->GetMaxAlienEnergy()) * 100.0f + 1.0f;
        }
        else
        {
            const auto pPlayer = dynamic_cast<CPlayer*>(pActor);
            if (pPlayer->GetNanoSuit())
                fEnergy = pPlayer->GetNanoSuit()->GetSuitEnergy() * 0.5f + 1.0f;
        }
    }

    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setHealth", iHealth);
    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setEnergy", static_cast<int>(fEnergy));
}

void CControlClient::Update()
{
    if (s_actionHandler.GetNumHandlers() == 0)
    {
#define ADD_HANDLER(action, func) s_actionHandler.AddHandler(actions.action, &CControlClient::func)
        const CGameActions& actions = g_pGame->Actions();

        ADD_HANDLER(view_lock, OnActionViewLock);
        ADD_HANDLER(moveforward, OnActionMoveForward);
        ADD_HANDLER(moveback, OnActionMoveBack);
        ADD_HANDLER(moveleft, OnActionMoveLeft);
        ADD_HANDLER(moveright, OnActionMoveRight);

        //ADD_HANDLER(squad_test, OnActionTest);

#undef ADD_HANDLER
    }
    m_isDebugLog = g_pGameCVars->tos_debug_log_all == 1;

    assert(m_pControlledActor);
    if (!m_pControlledActor)
        return;

    const CGameActions& rGameActions = g_pGame->Actions();

    if (!m_pLocalDude)
        return;

    auto* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);

    //This not fix trooper "gliding" after load game
    //pAlien->GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameClientStatic | eEA_GameServerDynamic | eEA_GameServerStatic);

    SMovementState state;
    m_pControlledActor->GetMovementController()->GetMovementState(state);

    IActor* pTargetActor = nullptr;
    Vec3 vCrosshairDir(m_crosshairPos - state.weaponPosition);

    // Get on crosshair entity and her AI
    constexpr int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
    static constexpr unsigned entityFlags = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid |
        ent_independent;

    ray_hit rayHit;

    const int hits = gEnv->pPhysicalWorld->RayWorldIntersection(
        state.weaponPosition,
        vCrosshairDir.GetNormalizedSafe() *
        g_pGameCVars->ctrl_shootRange,
        entityFlags, rayFlags,
        &rayHit, 1,
        m_pControlledActor->GetEntity()->GetPhysics());
    
    if (hits != 0)
    {
        if (rayHit.pCollider)
        {
            IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(rayHit.pCollider);
            if (pTargetEntity)
            {
                m_fireTargetId = pTargetEntity->GetId();
            }
            else
                m_fireTargetId = NULL;

            if (rayHit.bTerrain)
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
        gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(state.weaponPosition, ColorB(0, 0, 255, 255),
                                                       state.weaponPosition + m_camViewDir * 500,
                                                       ColorB(0, 0, 255, 255));
        gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(state.weaponPosition, ColorB(0, 255, 0, 255),
                                                       state.weaponPosition + vCrosshairDir.GetNormalizedSafe() * 2,
                                                       ColorB(0, 255, 0, 255));
        gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_camPos, ColorB(255, 0, 0, 255), m_camPos + m_camViewDir,
                                                       ColorB(255, 0, 0, 255));
    }

    m_meleeHits = static_cast<float>(gEnv->pPhysicalWorld->RayWorldIntersection(
        state.weaponPosition,
        vCrosshairDir.GetNormalizedSafe() * 3,
        entityFlags,
        rayFlags,
        &m_meleeRayhit, 1,
        m_pControlledActor->GetEntity()->GetPhysics()));
    
    if (m_meleeHits != 0.0f)
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
        if (rayHit.dist > 8.50f || pTargetActor)
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

        static const char* weaponName = nullptr;
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

        SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(false))

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
                SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(true))
                //m_Generic.m_bTargetFriendly = true;
            }
        }
        if (GetMeleeTarget() && m_trooper.isAiming)
        {
            if (IsFriendlyEntity(GetMeleeTarget(), m_pControlledActor))
            {
                if ((!pWeapon->IsWeaponRaised() || !pWeapon->IsModifying()) && GetItemClassName() !=
                    "Trooper_Melee_PLAY")
                {
                    pWeapon->LowerWeapon(true);
                    pWeapon->StopFire();
                }

                if (pWeapon->IsWeaponLowered())
                {
                    SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(true))
                }
            }
        }

        //pWeapon->SetAimLocation(m_vCrosshairPos);
        //pWeapon->SetTargetLocation(m_vCrosshairPos);
    }

    //draw debug
    if (g_pGameCVars->ctrl_debug_draw == 1)
    {
        static float c[] = {1, 1, 1, 1};
        IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_meleeTargetId);
        IEntity* pEntity2 = gEnv->pEntitySystem->GetEntity(m_crosshairTargetId);
        gEnv->pRenderer->Draw2dLabel(20, 180, 1.15f, c, false, "m_MeleeTargetName %s",
                                     pEntity ? pEntity->GetName() : "");
        gEnv->pRenderer->Draw2dLabel(20, 200, 1.15f, c, false, "m_CrosshairTargetName %s",
                                     pEntity2 ? pEntity2->GetName() : "");
        gEnv->pRenderer->Draw2dLabel(20, 220, 1.15f, c, false, "m_FireTargetName %s",
                                     GetFireTarget() ? GetFireTarget()->GetName() : "");
        gEnv->pRenderer->Draw2dLabel(20, 240, 1.15f, c, false, "m_bFireTargetFriendly %s",
                                     m_generic.fireTargetIsFriend ? "true" : "false");
        gEnv->pRenderer->Draw2dLabel(20, 260, 1.15f, c, false, "m_bMeleeTargetFriendly %s",
                                     m_generic.meleeTargetIsFriend ? "true" : "false");

        gEnv->pRenderer->Draw2dLabel(20, 280, 1.15f, c, false, "m_MovementDir (%1.f,%1.f,%1.f)",
            static_cast<double>(m_generic.moveDir.x),
            static_cast<double>(m_generic.moveDir.y),
            static_cast<double>(m_generic.moveDir.z));
        
        gEnv->pRenderer->Draw2dLabel(20, 300, 1.15f, c, false, "m_bDisableMovement %s",
                                     m_generic.cannotMove ? "true" : "false");

        //gEnv->pRenderer->Draw2dLabel(20, 140, 1.3f, c, false, "deltaMovement (%f,%f,%f)", pAlien->GetAlienInput()->deltaMovement.x, pAlien->GetAlienInput()->deltaMovement.y, pAlien->GetAlienInput()->deltaMovement.z);
    }

    // if Alien die
    {
        if (m_pControlledActor->GetHealth() <= 0)
        {
            SAFE_HUD_FUNC(ShowDeathFX(1))
        }
        else
            SAFE_HUD_FUNC(ShowDeathFX(0))
    }

    //Update this before scout update
    if (!m_generic.cannotLookAt && !m_pAbilitiesSystem->trooper.isCeiling)
    {
        if (GetActorClassName() == "Hunter")
        {
            static Matrix33 rot;
            Vec3 v(0, 0, RAD2DEG(m_pLocalDude->GetAngles().z));

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
    if (GetActorClassName() == "Trooper")
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
    CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();
    if (pSuit)
    {
        if (m_pControlledActor->IsAlien())
            pSuit->SetSuitEnergy(pAlien->GetAlienEnergy());
    }
    
    //Hide the Nomad player when is controlling somebody
    if (m_mustHideDude)
    {
        SActorStats* pActorStats = m_pLocalDude->GetActorStats();
        if (pActorStats)
        {
            uint8 physicsProfile = m_pLocalDude->GetGameObject()->GetAspectProfile(eEA_Physics);

            if (physicsProfile != eAP_Spectator)
                m_pLocalDude->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

            if (!pActorStats->isHidden)
                pActorStats->isHidden = true;
        }
    }

    //Beam the Nomad player to the controlled actor position
    if (m_mustBeamDude)
    {
        Vec3 vControlledPos = {0, 0, 0};
        if (m_pControlledActor)
            vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();

        if (!vControlledPos.IsZero())
        {
            vControlledPos.z += 3;

            Matrix34 nomadMat34 = m_pLocalDude->GetEntity()->GetWorldTM();
            nomadMat34.SetTranslation(vControlledPos);
            m_pLocalDude->GetEntity()->SetWorldTM(nomadMat34);
        }
    }

    // movement and others
    if (m_generic.cannotMove == true && m_generic.canMoveMult != 0.0f)
        m_generic.canMoveMult = 0;
    else if (m_generic.cannotMove == false)
        m_generic.canMoveMult = 1.0f;

    if (m_pControlledActor->IsAlien())
    {
        const string className = m_pControlledActor->GetEntity()->GetClass()->GetName();

        //if (gEnv->bMultiplayer)
        //{
        //	if (gEnv->bClient)
        //	{
        //		Vec3 currentMovement(0, 0, 0);
        //		Vec3& currentLookDir = pAlien->GetAlienViewMtx().GetColumn1().GetNormalized();
        //		Vec3& currentPosition = pAlien->GetEntity()->GetWorldPos();

        //		if (className == "Scout")
        //			currentMovement = m_generic.moveDir * m_scout.speedMult * m_generic.canMoveMult;
        //		else if (className == "PlayerTrooper")
        //			currentMovement = m_generic.moveDir * m_trooper.speedMult * m_generic.canMoveMult;
        //		else if (className == "Alien")
        //			currentMovement = m_generic.moveDir * m_nakedAlien.speedMult * m_nakedAlien.aimSpeedMult * m_generic.canMoveMult;
        //		else if (className == "Hunter")
        //			currentMovement = m_generic.moveDir * m_generic.canMoveMult;

        //		//CryLogAlways("Client requesting movement");

        //		pAlien->GetGameObject()->InvokeRMI(CAlien::SvRequestSetMove(), CAlien::SMovementParams(currentLookDir, currentMovement, currentPosition), eRMI_ToServer);
        //	}
        //}
        //else
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

            if (!m_generic.cannotLookAt && !m_pAbilitiesSystem->scout.IsSearch && !m_pAbilitiesSystem->scout.
                isGyroEnabled)
            {
                Quat alienInvQuat = pAlien->GetEntity()->GetRotation().GetInverted();
                Quat alienQuat(pAlien->GetEntity()->GetRotation());
                Quat dudeViewQuat = m_pLocalDude->GetViewQuatFinal();

                const float rotRate = pAlien->m_rateParams.rotationRate;

                alienQuat = Quat::CreateSlerp(alienQuat, dudeViewQuat, rotRate * 2);
                m_scout.modelQuat = alienInvQuat * alienQuat;
            }
        }
        if (className == "Drone")
        {
            //BUGFIX: Trooper very speedly after Naked control
            if (m_lookRequest.HasStance())
            {
                m_lookRequest.ClearStance();
            }

            pAlien->SetAlienMove(m_generic.moveDir * m_scout.speedMult * m_generic.canMoveMult);

            //Disables continuation rotation when cannotLookAt is true
            m_scout.modelQuat.SetIdentity();

            if (!m_generic.cannotLookAt)
            {
                Quat alienInvQuat = pAlien->GetEntity()->GetRotation().GetInverted();
                Quat alienQuat(pAlien->GetEntity()->GetRotation());
                Quat dudeViewQuat = m_pLocalDude->GetViewQuatFinal();

                const float rotRate = pAlien->m_rateParams.rotationRate;

                alienQuat = Quat::CreateSlerp(alienQuat, dudeViewQuat, rotRate * 2);
                m_scout.modelQuat = alienInvQuat * alienQuat;
            }
        }
        else if (className == "Trooper")
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

            const Vec3 move = m_generic.moveDir * m_nakedAlien.speedMult * m_nakedAlien.aimSpeedMult * m_generic.
                canMoveMult;
            pAlien->SetAlienMove(move);

            //Set naked alien stance for true animation
            //m_lookRequest.SetStance(STANCE_PRONE);

            //Disables continuation rotation when cannotLookAt is true
            m_nakedAlien.modelQuat.SetIdentity();

            if (!m_generic.cannotLookAt)
            {
                //Naked alien have a specific rotation like in zeroG
                const Quat alienInvQuat = pAlien->GetEntity()->GetRotation().GetInverted();
                const Quat dudeViewQuat = m_pLocalDude->GetViewQuatFinal();

                m_nakedAlien.modelQuat = alienInvQuat * dudeViewQuat;
            }
        }
        else if (className == "Hunter" || className == "Pinger")
        {
            //BUGFIX: Trooper very speedly after Naked control
            if (m_lookRequest.HasStance())
            {
                m_lookRequest.ClearStance();
            }

            pAlien->SetAlienMove(m_generic.moveDir * m_generic.canMoveMult);
        }

        m_pControlledActor->GetGameObject()->ChangedNetworkState(IPlayerInput::INPUT_ASPECT);
    }

    //Only Look target and aim processing
    if (!m_generic.cannotLookAt)
    {
        m_pControlledActor->GetMovementController()->RequestMovement(m_lookRequest);
        m_pControlledActor->GetGameObject()->ChangedNetworkState(IPlayerInput::INPUT_ASPECT);
    }

    //pAlien->SetActorMovement(CAlien::SMovementRequestParams(m_Req));

    //Update entities usability
    UpdateUsability(GetMeleeTarget());

    /*if (gEnv->bServer && gEnv->bMultiplayer && g_pGame->GetGameRules())
    {
        int health = m_pControlledActor->GetHealth();
        m_pControlledActor->GetGameObject()->InvokeRMI(CAlien::ClSetHealth(), CAlien::SetHealthParams(health), eRMI_ToOwnClient | eRMI_NoLocalCalls);
    }*/

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

void CControlClient::UpdateUsability(const IEntity* pTarget) const
{
    if (pTarget)
    {
        const IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pTarget->GetId());
        const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pTarget->GetId());

        if (!pVehicle && !pItem)
        {
            IScriptTable* pTable = pTarget->GetScriptTable();
            if (pTable)
            {
                bool bIsUsable = false;
                auto sUseText = "";

                SmartScriptTable props;
                if (pTable->GetValue("Properties", props))
                    props->GetValue("UseText", sUseText);
                const string sText = sUseText;

                HSCRIPTFUNCTION IsUsable = nullptr;
                if (pTable->GetValueType("IsUsable") == svtFunction &&
                    pTable->GetValue("IsUsable", IsUsable))
                {
                    Script::CallReturn(gEnv->pScriptSystem, IsUsable, pTable, bIsUsable);
                    gEnv->pScriptSystem->ReleaseFunc(IsUsable);
                    //CryLogAlways("%s is Usable %1.f", pTarget->GetName(), (float)result);
                }

                const float fDist = (pTarget->GetWorldPos() - m_pControlledActor->GetEntity()->GetWorldPos()).
                    GetLength();
                if (fDist <= 3.f && bIsUsable)
                {
                    SAFE_HUD_FUNC(GetCrosshair()->SetUsability(1, sText.empty() ? "@use_door" : sText.c_str()))
                    //CryLogAlways("Usable Text: %s", sText.c_str());
                }
                else if (fDist > 3.f || !bIsUsable)
                    SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0))
            }
        }
    }
    else
    {
        SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0))
    }
}

void CControlClient::UpdateCrosshair()
{
    const CCamera& camera = gEnv->pSystem->GetViewCamera();
    m_camPos = camera.GetMatrix().GetTranslation();
    m_camViewDir = camera.GetViewdir() * g_pGameCVars->ctrl_shootRange;
    m_camViewCoords = m_camViewDir + m_camPos;

    // Get on crosshair entity and her AI
    static constexpr int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
    static constexpr unsigned entityFlags = ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid |
        ent_independent;

    //Physics entity
    IPhysicalEntity* pDudePhysics = m_pLocalDude->GetEntity()->GetPhysics();
    IPhysicalEntity* pPhys = (m_pControlledActor != nullptr)
                                 ? m_pControlledActor->GetEntity()->GetPhysics()
                                 : pDudePhysics;

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
    if (gEnv->pPhysicalWorld->RayWorldIntersection(m_camPos, m_camViewDir, entityFlags, rayFlags, &m_crosshairRayHit, 1,
                                                   pPhys))
    {
        m_crosshairPos = m_crosshairRayHit.pt;
        if (m_crosshairRayHit.pCollider)
        {
            IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_crosshairRayHit.pCollider);
            if (pTargetEntity)
            {
                m_lastCrosshairTargetId = m_crosshairTargetId = pTargetEntity->GetId();

                const IAIObject* pTargetAI = pTargetEntity->GetAI();
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

void CControlClient::UpdateHunter() const
{
    if (!m_pControlledActor || !m_pLocalDude)
        return;

    SActorParams* pParams = m_pLocalDude->GetActorParams();
    pParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
    pParams->vLimitRangeH = DEG2RAD(70);
}

void CControlClient::UpdateScout()
{
    if (!m_pControlledActor || !m_pLocalDude)
        return;

    auto* pScout = dynamic_cast<CScout*>(m_pControlledActor);
    SActorParams* pDudeParams = m_pLocalDude->GetActorParams();

    bool isAutoAiming = false;
    bool isHostileTarget = false;

    IEntity* pCrosshairTarget = gEnv->pEntitySystem->GetEntity(m_lastCrosshairTargetId);
    if (pCrosshairTarget)
    {
        IActor* pTargetActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pCrosshairTarget->GetId());
        IVehicle* pTargetVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(
            pCrosshairTarget->GetId());

        bool isActorAlive = pTargetActor && pTargetActor->GetHealth() > 0;
        bool isVehicleAlive = pTargetVehicle && !(pTargetVehicle->IsDestroyed()) && pTargetVehicle->GetStatus().
            passengerCount > 0;
        isHostileTarget = !IsFriendlyEntity(pCrosshairTarget, m_pControlledActor) && (isActorAlive || isVehicleAlive);
        //&& TOS_AI::IsHostile(pCrosshairTarget, m_pControlledActor->GetEntity(), false);

        if (isHostileTarget)
        {
            AABB targetBounds;
            pCrosshairTarget->GetWorldBounds(targetBounds);

            Vec3 targetCenterPos = targetBounds.GetCenter();

            //Distance method
            float distToTarget = (targetCenterPos - m_crosshairPos).GetLength();
            float threshold = 10.0f;

            isAutoAiming = distToTarget < threshold;
            if (isAutoAiming && g_pGameCVars->ctrl_scAimSupport > 0.0f)
            {
                m_lookRequest.SetFireTarget(targetCenterPos);
            }
        }

        if (CHUDSilhouettes* pSil = SAFE_HUD_FUNC_RET(GetSilhouettes()))
        {
            if (isAutoAiming && isHostileTarget)
            {
                pSil->SetSilhouette(pCrosshairTarget, 1.0f, 1.0f, 1.0f, 1.0f, -3.0f);
            }
            else
            {
                if (pSil->GetSilhouette(pCrosshairTarget))
                    pSil->ResetSilhouette(pCrosshairTarget->GetId());
            }
        }
    }

    m_scoutAimTargetId = pCrosshairTarget && isAutoAiming && isHostileTarget
                             ? m_scoutAimTargetId = pCrosshairTarget->GetId()
                             : 0;

    if (m_scoutAimTargetId != 0)
    {
        if (!m_scout.targetLocked)
        {
            m_scout.targetLocked = true;

            if (m_animScoutFlyInterface.IsLoaded())
                m_animScoutFlyInterface.Invoke("setScoutTargetLocked", 2);

            _smart_ptr<ISound> pBeep = gEnv->pSoundSystem->CreateSound("Sounds/interface:hud:target_lock", 0);
            if (pBeep)
            {
                pBeep->SetSemantic(eSoundSemantic_HUD);
                pBeep->Play();
            }

            //CryLogAlways("locked");
        }
    }
    else
    {
        if (m_scout.targetLocked)
        {
            m_scout.targetLocked = false;

            if (m_animScoutFlyInterface.IsLoaded())
                m_animScoutFlyInterface.Invoke("setScoutTargetLocked", 1);

            _smart_ptr<ISound> pBeep = gEnv->pSoundSystem->CreateSound("sounds/interface:suit:binocular_target_locked",
                                                                       0);
            if (pBeep)
            {
                pBeep->SetSemantic(eSoundSemantic_HUD);
                pBeep->Play();
            }

            //CryLogAlways("unlocked");
        }
    }


    if (m_pAbilitiesSystem->scout.IsSearch)
    {
        //Look forward when scout enable search mode
        if (GetActorClassName() == "Scout")
        {
            Matrix33 rot;
            Vec3 v(0, 0, RAD2DEG(m_pLocalDude->GetAngles().z));

            rot.SetRotationXYZ(Ang3(DEG2RAD(v)));
            m_lookRequest.SetLookTarget(pScout->GetEntity()->GetWorldPos() + 20.0f * rot.GetColumn1());
        }
    }

    if (!m_scout.canDodge) // When scout dodging
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
            pEntity->SetRotation(m_pLocalDude->GetViewQuatFinal());

            if (m_pLocalDude)
            {
                pDudeParams->vLimitDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
                pDudeParams->vLimitRangeH = DEG2RAD(50);
                pDudeParams->vLimitRangeVUp = DEG2RAD(1);
                pDudeParams->vLimitRangeVDown = DEG2RAD(-50);
            }

            if (m_crosshairRayHit.pCollider)
            {
                // IItem* pItem = m_pControlledActor->GetCurrentItem();
                // if (pItem)
                // {
                //     IPhysicalEntity* pSkipEntities[10];
                //     int nSkip = 0;
                //     
                //     auto* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
                //     if (pWeapon)
                //         nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
                // }

                const Vec3& center(m_crosshairRayHit.pt);
                constexpr float range = 5.f;

                const Vec3 min(center.x - range, center.y - range, center.z - range);
                const Vec3 max(center.x + range, center.y + range, center.z + range);

                IPhysicalWorld* pWorld = gEnv->pPhysicalWorld;
                IPhysicalEntity** ppList = nullptr;
                int numEntities = pWorld->GetEntitiesInBox(min, max, ppList, ent_all);
                for (int i = 0; i < numEntities; ++i)
                {
                    if (m_crosshairRayHit.dist < 35.f)
                    {
                        EntityId id = pWorld->GetPhysicalEntityId(ppList[i]);
                        IEntity* pCrosshairEnt = gEnv->pEntitySystem->GetEntity(id);

                        CHUDSilhouettes* pSil = SAFE_HUD_FUNC_RET(GetSilhouettes());

                        if (pCrosshairEnt && pSil)
                        {
                            IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
                            //Scanned actor
                            IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
                            //Scanned vehicle

                            IAIObject* pAI = pCrosshairEnt->GetAI();
                            IAIObject* pControlActorAI = m_pControlledActor->GetEntity()->GetAI();

                            if (pActor && pControlActorAI && pAI)
                            {
                                if (pAI->IsHostile(pControlActorAI, false))
                                {
                                    auto pScanPlayer = dynamic_cast<CPlayer*>(pActor);
                                    if (pScanPlayer)
                                    {
                                        CNanoSuit* pScanSuit = pScanPlayer->GetNanoSuit();
                                        if (pScanSuit && pScanSuit->GetMode() == NANOMODE_CLOAK)
                                        {
                                            pScanSuit->SetMode(NANOMODE_DEFENSE);
                                            continue;
                                        }
                                    }

                                    if (!pSil->GetSilhouette(pCrosshairEnt))
                                    {
                                        pSil->SetSilhouette(pCrosshairEnt, 255, 255, 255, 1.0f, 60.0f);

                                        if (g_pGameCVars->sqd_ScoutCanSearch)
                                        {
                                            auto* pSquad = g_pControlSystem->m_pSquadSystem->GetSquadFromMember(
                                                m_pControlledActor, true);
                                            if (pSquad && pSquad->HasClientLeader())
                                            {
                                                //for (auto& member : pSquad->GetAllMembers())
                                                //{
                                                    //member.SetSearchPos(pEntity->GetWorldPos());
                                                    //pSquad->ExecuteOrder(eSO_SearchEnemy, &member, eEOF_ExecutedByAI);
                                                //}
                                            }
                                            else
                                                SendPipeToAIGroup(
                                                    "rush_ai", pControlActorAI->GetGroupId(),
                                                    true,
                                                    pControlActorAI->GetEntity()->GetWorldPos());
                                        }
                                    }

                                    if (pActor->GetHealth() <= 0)
                                    {
                                        pSil->ResetSilhouette(id);
                                    }
                                }
                            }

                            if (pVehicle)
                            {
                                IAIObject* pVehAI = pVehicle->GetEntity()->GetAI();

                                if (pVehAI && pVehAI->IsHostile(pControlActorAI, false) && pVehicle->GetStatus().
                                    passengerCount > 0)
                                {
                                    if (!pSil->GetSilhouette(pCrosshairEnt))
                                    {
                                        pSil->SetSilhouette(pCrosshairEnt, 255, 255, 255, 1.0f, 60.0f);
                                        SendPipeToAIGroup("rush_ai", pControlActorAI->GetGroupId(), true,
                                                          pControlActorAI->GetEntity()->GetWorldPos());
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

        if (m_pLocalDude)
        {
            pDudeParams->vLimitDir.zero();
            pDudeParams->vLimitRangeH = 0.0f;
            pDudeParams->vLimitRangeVUp = pDudeParams->vLimitRangeVDown = 0.0f;
        }
    }

    if (m_animScoutFlyInterface.IsLoaded())
    {
        //Roll
        const Vec3 vWorldPos = pScout->GetEntity()->GetWorldPos();
        const float waterLevel = gEnv->p3DEngine->GetWaterLevel(&vWorldPos);

        float terrainZ = GetISystem()->GetI3DEngine()->GetTerrainZ(static_cast<int>(vWorldPos.x),
                                                                   static_cast<int>(vWorldPos.y));
        if (terrainZ < waterLevel)
            terrainZ = waterLevel;

        const float fRoll = RAD2DEG(pScout->GetEntity()->GetWorldAngles().y);

        m_animScoutFlyInterface.Invoke("setRoll", fRoll);

        //Pitch
        SMovementState sMovementState;
        pScout->GetMovementController()->GetMovementState(sMovementState);

        const int iAttitude = static_cast<int>(sMovementState.eyeDirection.z * 320);
        m_animScoutFlyInterface.Invoke("setPitch", iAttitude);

        //Values
        const auto height = static_cast<int>(vWorldPos.z - terrainZ);
        const auto speed = static_cast<int>(pScout->GetActorStats()->speed);
        const auto speedMul = static_cast<int>(m_scout.speedMult * 10.0f);

        SFlashVarValue args[3] = {height, speed, speedMul};
        m_animScoutFlyInterface.Invoke("setScoutValues", args, 3);
    }
}

void CControlClient::UpdateTrooper()
{
    auto pAlienTrooper = dynamic_cast<CTrooper*>(m_pControlledActor);

    if (!pAlienTrooper || !m_pLocalDude)
        return;

    IAIObject* pAITrooper = pAlienTrooper->GetEntity()->GetAI();

    static Vec3 vWeaponTarget(0, 0, 0); // Trooper melee target position
    if (m_meleeHits != 0.0f)
        vWeaponTarget = m_meleeRayhit.pt;
    else
        vWeaponTarget = m_camViewCoords;

    //when is ceiling
    SActorParams* pParams = m_pLocalDude->GetActorParams();
    uint flags = pAlienTrooper->GetEntity()->GetSlotFlags(0);

    //Trooper is using the binoculars -> play the search animation
    if (m_trooper.isBinocular && pAlienTrooper->GetStance() != STANCE_STEALTH)
    {
        if (!m_pAbilitiesSystem->trooper.isCeiling && !m_pAbilitiesSystem->trooper.doCeiling)
        {
            m_lookRequest.SetStance(STANCE_STEALTH);
        }
    }
    else if (!m_trooper.isBinocular && pAlienTrooper->GetStance() == STANCE_STEALTH)
    {
        if (!m_pAbilitiesSystem->trooper.isCeiling && !m_pAbilitiesSystem->trooper.doCeiling)
        {
            m_lookRequest.SetStance(STANCE_STAND);
        }
    }

    if (m_pAbilitiesSystem->trooper.isCeiling)
    {
        if (pAlienTrooper->GetHealth() > 0)
        {
            m_generic.cannotMove = true;

            //Disable trooper body
            flags &= ~ENTITY_SLOT_RENDER;
            pAlienTrooper->GetEntity()->SetSlotFlags(0, flags);

            if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_Ceiling)
            {
                //Change view limits
                if (m_pLocalDude)
                {
                    pParams->vLimitDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn1() * 2.f;
                    pParams->vLimitRangeH = DEG2RAD(50);
                    pParams->vLimitRangeVUp = DEG2RAD(1);
                    pParams->vLimitRangeVDown = DEG2RAD(-50);
                }

                IPhysicalEntity* pSkipEntities[10];
                int nSkip = 0;
                IItem* pItem = pAlienTrooper->GetCurrentItem();
                if (pItem)
                {
                    auto pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
                    if (pWeapon)
                        nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
                }

                ray_hit hit;
                Vec3 vTrooperUpDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn2() * 3.0f;

                gEnv->pPhysicalWorld->RayWorldIntersection(pAlienTrooper->GetEntity()->GetWorldPos(),
                                                                      vTrooperUpDir,
                                                                      ent_terrain | ent_static,
                                                                      rwi_ignore_noncolliding | rwi_stop_at_pierceable,
                                                                      &hit, 1, pSkipEntities, nSkip);

                if (!hit.pCollider)
                {
                    m_pAbilitiesSystem->trooper.isCeiling = false;
                    m_pAbilitiesSystem->trooper.doCeiling = false;
                    NetPlayAnimAction("idle", true);
                    //m_bTrooperIsCeiling = false;
                }
            }
            else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallLeft)
            {
                //Change view limits
                if (m_pLocalDude)
                {
                    pParams->vLimitDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn0() * 2.f;
                    pParams->vLimitRangeH = DEG2RAD(90); //horizont
                    pParams->vLimitRangeVUp = DEG2RAD(1); //up limit form center
                    pParams->vLimitRangeVDown = DEG2RAD(-25); //down limit form center
                }
            }
            else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallRight)
            {
                //Change view limits
                if (m_pLocalDude)
                {
                    pParams->vLimitDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn0() * -2.f;
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
            NetPlayAnimAction("idle", true);
            //m_bTrooperIsCeiling = false;
        }
    }

    //Allow melee
    if (!m_trooper.canMelee) // When trooper make a punch
    {
        if (CheckPassTime(TROOPER_MELEE_REST_TIME))
        {
            m_trooper.canMelee = true;
        }
    }
    else if (!m_trooper.canJumpMelee)
    {
        if (pAlienTrooper->GetActorStats()->onGround > 0.10f) //
        {
            m_trooper.canJumpMelee = true;
        }
    }

    //const auto vImpulseDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn(1) * 40.f;
    // Trooper melee attack impulse direction
    //Do melee flags from OnAction
    if (m_trooper.doMelee && !m_generic.meleeTargetIsFriend)
    {
        if (CheckPassTime(0.2f)) //time while playing animation
        {
            m_trooper.doMelee = false;

            if (auto pWeapon = GetCurrentWeapon(pAlienTrooper))
                pAlienTrooper->DoMeleeAttack(pWeapon, GetActorClassName(), m_meleeTargetId, vWeaponTarget,
                                             m_camViewDir);
        }
    }
    else if (m_trooper.doJumpMelee)
    {
        if (pAlienTrooper->GetActorStats()->onGround > 0.10f) //
        {
            m_trooper.doJumpMelee = false;

            if (auto pWeapon = GetCurrentWeapon(pAlienTrooper))
                pAlienTrooper->DoMeleeAttack(pWeapon, GetActorClassName(), m_meleeTargetId, vWeaponTarget,
                                             m_camViewDir);
        }
    }

    if (!m_trooper.canDodge)
    {
        if (CheckPassTime(TROOPER_DODGE_REST_TIME))
            m_trooper.canDodge = true;
    }

    if (!m_trooper.canJump)
    {
        if (CheckPassTime(TROOPER_JUMP_AFTER_DODGE_REST_TIME))
            m_trooper.canJump = true;
    }

    if (!m_pAbilitiesSystem->trooper.canCeiling)
    {
        if (pAlienTrooper->GetActorStats()->onGround > 0.0f)
            m_pAbilitiesSystem->trooper.canCeiling = true;
    }
    //Stay on ceiling
    if (pAlienTrooper->GetActorStats()->inAir > 0.25f && !m_trooper.doJumpMelee)
    {
        IPhysicalEntity* pSkipEntities[10];
        int nSkip = 0;
        IItem* pItem = pAlienTrooper->GetCurrentItem();
        if (pItem)
        {
            auto* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
            if (pWeapon)
                nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
        }

        ray_hit hit;
        constexpr int objTypes = ent_terrain | ent_static;
        constexpr int rayFlags = rwi_ignore_noncolliding | rwi_stop_at_pierceable;

        if (m_pAbilitiesSystem->trooper.doCeiling) //if ability key is pressed
        {
            SMovementState state;
            pAlienTrooper->GetMovementController()->GetMovementState(state);

            Vec3 xDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn0() * 100;
            Vec3 worldPos = pAlienTrooper->GetEntity()->GetWorldPos();

            if (m_actionsMap["moveleft"] == eAAM_OnPress && g_pGameCVars->ctrl_trWalls == 1.0f)
            {
                if (!m_pAbilitiesSystem->trooper.isCeiling)
                {
                    m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_WallLeft;

                    const Vec3 vTrooperLeftDir = xDir * -1;

                    const int leftHits = gEnv->pPhysicalWorld->RayWorldIntersection(
                        pAlienTrooper->GetEntity()->GetWorldPos(), vTrooperLeftDir, objTypes, rayFlags, &hit, 1,
                        pSkipEntities, nSkip);
                    if (leftHits > 0)
                    {
                        gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(worldPos, ColorB(0, 255, 0, 255),
                                                                       worldPos + vTrooperLeftDir,
                                                                       ColorB(0, 255, 0, 255));

                        if (hit.pCollider)
                        {
                            if (hit.dist <= 2.0f)
                            {
                                NetPlayAnimAction("CTRL_StayOnWallLeft", true);
                                m_pAbilitiesSystem->trooper.isCeiling = true;

                                //Matrix34 mat34 = pAlienTrooper->GetEntity()->GetWorldTM();
                                //const Vec3 hitPoint(hit.pt.x + 2, pAlienTrooper->GetEntity()->GetWorldPos().y,
                                                    //pAlienTrooper->GetEntity()->GetWorldPos().z);
                            }
                        }
                    }
                }
            }
            else if (m_actionsMap["moveright"] == eAAM_OnPress && g_pGameCVars->ctrl_trWalls == 1.0f)
            {
                if (!m_pAbilitiesSystem->trooper.isCeiling)
                {
                    m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_WallRight;

                    const Vec3 vTrooperRightDir = xDir;

                    const int rightHits = gEnv->pPhysicalWorld->RayWorldIntersection(
                        pAlienTrooper->GetEntity()->GetWorldPos(), vTrooperRightDir, objTypes, rayFlags, &hit, 1,
                        pSkipEntities, nSkip);
                    if (rightHits > 0)
                    {
                        gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(pAlienTrooper->GetEntity()->GetWorldPos(),
                                                                       ColorB(0, 255, 0, 255),
                                                                       pAlienTrooper->GetEntity()->GetWorldPos() +
                                                                       vTrooperRightDir, ColorB(0, 255, 0, 255));

                        if (hit.pCollider)
                        {
                            if (hit.dist <= 2.0f)
                            {
                                NetPlayAnimAction("CTRL_StayOnWallRight", true);
                                m_pAbilitiesSystem->trooper.isCeiling = true;

                                //Matrix34 mat34 = pAlienTrooper->GetEntity()->GetWorldTM();
                               // const Vec3 hitPoint(hit.pt.x - 2,
                                                    //pAlienTrooper->GetEntity()->GetWorldPos().y,
                                                    //pAlienTrooper->GetEntity()->GetWorldPos().z);
                            }
                        }
                    }
                }
            }
            else
            {
                if (!m_pAbilitiesSystem->trooper.isCeiling)
                    m_pAbilitiesSystem->trooper.eCeilingType = eCeilingType_Ceiling;

                const Vec3 vTrooperUpDir = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn2() * 100.f;
                const int ceilingHits = gEnv->pPhysicalWorld->RayWorldIntersection(
                    pAlienTrooper->GetEntity()->GetWorldPos(),
                    vTrooperUpDir, objTypes, rayFlags, &hit, 1, pSkipEntities, nSkip);
                //int rightHits = gEnv->pPhysicalWorld->RayWorldIntersection(pAlienTrooper->GetEntity()->GetWorldPos(), vTrooperUpDir, objTypes, rayFlags, &ceilingHit, 1, pSkipEntities, nSkip);

                if (ceilingHits > 0)
                {
                    if (hit.pCollider)
                    {
                        if (hit.dist <= 2.3f && hit.dist >= 2.0f)
                        {
                            NetPlayAnimAction("CTRL_StayOnCeiling", true);
                            m_pAbilitiesSystem->trooper.isCeiling = true;
                        }
                    }
                }
            }

            if (IInventory* pInventory = pAlienTrooper->GetInventory())
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
                            pAlienTrooper->SelectItem(shootWeaponId, false);
                    }
                }
            }
        }
        else if (!m_pAbilitiesSystem->trooper.doCeiling && m_pAbilitiesSystem->trooper.isCeiling)
        {
            //pe_status_dynamics status = pe_status_dynamics();
            //pAlienTrooper->GetEntity()->GetPhysics()->GetStatus(&status);
            NetPlayAnimAction("idle", true);

            pe_action_impulse impulseParams;

            if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallLeft)
                impulseParams.impulse = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn0() * 500.f;
            else if (m_pAbilitiesSystem->trooper.eCeilingType == eCeilingType_WallRight)
                impulseParams.impulse = pAlienTrooper->GetEntity()->GetWorldTM().GetColumn0() * -500.f;

            pAlienTrooper->GetEntity()->GetPhysics()->Action(&impulseParams);

            m_pAbilitiesSystem->trooper.isCeiling = false;

            //Enable trooper body
            flags |= ENTITY_SLOT_RENDER;
            pAlienTrooper->GetEntity()->SetSlotFlags(0, flags);

            m_generic.cannotMove = false;

            //Change view limits
            if (m_pLocalDude)
            {
                pParams->vLimitDir.zero();
                pParams->vLimitRangeH = 0.0f;
                pParams->vLimitRangeVUp = pParams->vLimitRangeVDown = 0.0f;
            }
        }
    }

    if (CWeapon* pWeapon = GetCurrentWeapon(pAlienTrooper))
    {
        //m_restTimer = 0;

        if (pWeapon->IsFiring() && GetCrosshairTargetAI() && GetCrosshairTargetAI()->IsHostile(pAITrooper))
        {
            if (IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(m_crosshairTargetId))
            {
                if (pVehicle->GetStatus().passengerCount == 0)
                    return;
            }

            if (m_generic.fSoundRestTimer == 0.0f)
            {
                auto* pSoundProxy = dynamic_cast<IEntitySoundProxy*>(pAlienTrooper->GetEntity()->CreateProxy(
                    ENTITY_PROXY_SOUND));
                if (pSoundProxy)
                {
                    auto pSound = gEnv->pSoundSystem->CreateSound("sounds/alien:trooper:scare", FLAG_SOUND_DEFAULT_3D);

                    if (pSound)
                    {
                        pSound->SetPosition(pAlienTrooper->GetEntity()->GetWorldPos());
                        pSound->SetSemantic(eSoundSemantic_AI_Readability);
                        pSound->SetVolume(1);

                        pSoundProxy->PlaySound(pSound);
                        m_generic.fSoundRestTimer = Random(12, 20);

                        if (g_pGameCVars->sqd_TrooperCanSay)
                        {
                            auto* pSquad = g_pControlSystem->m_pSquadSystem->GetSquadFromMember(pAlienTrooper, true);
                            if (pSquad && pSquad->HasClientLeader())
                            {
                                //for (auto& member : pSquad->GetAllMembers())
                                //{
                                    //member.SetSearchPos(m_crosshairPos);
                                    //pSquad->ExecuteOrder(eSO_SearchEnemy, &member, eEOF_ExecutedByAI);
                                //}
                            }
                            else
                                SendPipeToAIGroup("rush_ai", pAITrooper->GetGroupId(), true,
                                                  pAITrooper->GetEntity()->GetWorldPos());
                        }
                    }
                }
            }
        }

        if (m_generic.fSoundRestTimer != 0.0f)
        {
            m_generic.fSoundRestTimer -= gEnv->pTimer->GetFrameTime();
            if (m_generic.fSoundRestTimer < 0.0f)
                m_generic.fSoundRestTimer = 0.0f;
        }

        //if (!g_pControlSystem->GetEnabled())
        //	restTimer = 0;

        //float clr[] = { 1,1,1,1 };
        //gEnv->pRenderer->Draw2dLabel(20, 20, 2, clr, false, "restTimer: %.f", m_SoundRestTimer);
        //gEnv->pRenderer->Draw2dLabel(20, 40, 2, clr, false, "m_CrosshairTargetId: %.f", float(m_CrosshairTargetId));
    }

    if (m_pAbilitiesSystem->trooper.shockwaveTimer != 0.0f)
    {
        m_pAbilitiesSystem->trooper.shockwaveTimer -= gEnv->pTimer->GetFrameTime();

        if (m_pAbilitiesSystem->trooper.shockwaveTimer < 0.0f)
            m_pAbilitiesSystem->trooper.shockwaveTimer = 0.0f;
    }

    if (m_trooper.isSprinting)
    {
        static float sprintTimer = 1.0f;

        if (sprintTimer != 0.0f)
            sprintTimer -= gEnv->pTimer->GetFrameTime();

        if (sprintTimer < 0.0f)
            sprintTimer = 0.0f;

        if (sprintTimer == 0.0f)
        {
            SubEnergy(2.5f);
            sprintTimer = 1.0f;
        }
    }

    //draw debug
    if (g_pGameCVars->ctrl_debug_draw == 1)
    {
        static float clr[] = {1, 1, 1, 1};
        const auto timer = static_cast<double>(m_pAbilitiesSystem->trooper.shockwaveTimer);
        gEnv->pRenderer->Draw2dLabel(20, 400, 1.15f, clr, false, "m_pAbilitiesSystem->trooper.shockwaveTimer: %.f", timer);
    }
    //gEnv->pRenderer->Draw2dLabel(20, 40, 2, clr, false, "m_CrosshairTargetId: %.f", float(m_CrosshairTargetId));
}

void CControlClient::FullSerialize(TSerialize ser)
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

            bool isEnabled = g_pControlSystem->GetLocalEnabled();
            ser.Value("g_pControlSystem_bEnabled", isEnabled);

            ser.EnumValue("m_lastDudeNanoMode", m_lastDudeNanoMode, NANOMODE_SPEED, NANOMODE_LAST);
            ser.Value("m_lastDudeRotation", m_lastDudeRotation);
            ser.Value("m_lastDudePosition", m_lastDudePosition);
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

            bool bEnabled = false;
            EntityId _controlledId;

            ser.Value("m_pControlledActorId", _controlledId);
            ser.Value("g_pControlSystem_bEnabled", bEnabled);

            if (bEnabled)
            {
                //Because the Control System already serialized and pick up saved values;
                if (auto* pNewControlledActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->
                                                                               GetActor(_controlledId)))
                {
                    //CPlayer::TAlienInterferenceParams lastInterferenceParams = pDude->m_interferenceParams;
                    //pDude->m_interferenceParams.clear();

                    //Player save game -->> Player load game -->> setup default values from save game -->> ControlSystem interpret this values as defaults
                    {
                        ser.EnumValue("m_lastDudeNanoMode", m_lastDudeNanoMode, NANOMODE_SPEED, NANOMODE_LAST);
                        ser.Value("m_lastDudeRotation", m_lastDudeRotation);
                        ser.Value("m_lastDudePosition", m_lastDudePosition);
                        ser.Value("m_lastDudeSpecies", m_lastDudeSpecies);

                        if (m_pLocalDude)
                        {
                            CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();
                            if (pSuit)
                                pSuit->SetMode(m_lastDudeNanoMode);

                            SetDudeSpecies(m_lastDudeSpecies);
                        }
                    }

                    InitDudeToControl(true);
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
                    ser.Value("m_CrosshairTargetId", m_crosshairTargetId);
                    ser.Value("m_LastCrosshairTargetId", m_lastCrosshairTargetId);
                }
            }
            else
            {
                if (g_pControlSystem->GetLocalEnabled())
                    g_pControlSystem->StopLocal(false);
            }

            //CryLogAlways("Loaded m_bControlSystemEnabled in %d", int(benabled));
        }
        ser.EndGroup();

        /*if (m_pSquadSystem)
            m_pSquadSystem->FullSerialize(ser);*/
        if (g_pControlSystem->GetSquadSystem())
            g_pControlSystem->GetSquadSystem()->Serialize(ser);
    }
}

void CControlClient::ToggleTutorialMode(const bool mode)
{
    m_generic.isTutorialMode = mode;
}

bool CControlClient::GetTutorialMode() const
{
    return m_generic.isTutorialMode;
}

IMaterial* CControlClient::GetMaterial() const
{
    if (m_pControlledActor)
    {
        if (const IEntity* pEnt = m_pControlledActor->GetEntity())
        {
            if (auto* pProxy = dynamic_cast<IEntityRenderProxy*>(pEnt->GetProxy(ENTITY_PROXY_RENDER)))
            {
                if (IMaterial* pMat = pProxy->GetRenderMaterial(0))
                    return pMat;
            }
        }
    }
    return nullptr;
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

void CControlClient::ApplyMovement(const Vec3& delta)
{
    m_generic.moveDir.x = clamp_tpl(m_generic.moveDir.x + delta.x, -1.0f, 1.0f);
    m_generic.moveDir.y = clamp_tpl(m_generic.moveDir.y + delta.y, -1.0f, 1.0f);
    m_generic.moveDir.z = clamp_tpl(m_generic.moveDir.z + delta.z, -1.0f, 1.0f);
}

void CControlClient::ResetParams()
{
    m_nakedAlien.Reset();
    m_trooper.Reset();
    m_scout.Reset();
    m_hunter.Reset();
    m_generic.Reset();
}

void CControlClient::Reset(const bool toEditor)
{
    if (m_pControlledActor && m_pControlledActor->GetEntity() && !toEditor)
    {
        if (m_pControlledActor->IsAlien())
        {
            auto* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
            //if (pAlien->GetAlienInput())
            pAlien->GetAlienInput().deltaMovement.zero();
        }
        else
        {
            const auto* pPlayer = dynamic_cast<CPlayer*>(m_pControlledActor);
            if (pPlayer->GetPlayerInput())
                pPlayer->GetPlayerInput()->Reset();
        }

        if (gEnv->pAISystem && gEnv->pAISystem->IsEnabled())
        {
            if (m_pControlledActor->GetEntity()->GetAI() && m_pControlledActor->GetEntity()->GetAI()->GetAIType() !=
                AIOBJECT_PUPPET)
                SetActorAI(m_pControlledActor, false);
        }

        if (gEnv->bClient && g_pGame->GetGameRules())
        {
            const auto params = CGameRules::OwnerParams(0, m_pControlledActor->GetEntityId());
            g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::SvRequestSetOwner(), params, eRMI_ToServer);
        }

        if (gEnv->bMultiplayer)
            m_mpLastControlledId = m_pControlledActor->GetEntityId();

        m_pControlledActor = nullptr;
    }
    else
    {
        m_pControlledActor = nullptr;
    }

    SAFE_HUD_FUNC(SetWeaponName(""))
    //m_actions = ACTION_GYROSCOPE;//gyroscope is on by default
}

void CControlClient::ToggleDudeHide(const bool bToggle)
{
    if (!m_pLocalDude)
        return;

    if (m_pLocalDude == g_pGame->GetIGameFramework()->GetClientActor())
        m_pLocalDude->GetGameObject()->InvokeRMI(CActor::SvRequestHidePlayer(), CActor::PlayerHideParams(bToggle),
                                                 eRMI_ToServer);

    m_mustHideDude = bToggle;
}

void CControlClient::ToggleDudeBeam(const bool bBeam)
{
    const IActor* pNomadPlayer = g_pGame->GetIGameFramework()->GetClientActor();

    if (!pNomadPlayer)
        return;

    m_mustBeamDude = bBeam;
}

void CControlClient::SetDudeSpecies(const int species)
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

bool CControlClient::NetSpawnParticleEffect(const char* effectName) const
{
    if (gEnv->bClient)
    {
        const Vec3 pos = m_pControlledActor->GetEntity()->GetWorldPos();
        const string name = effectName;
        m_pControlledActor->GetGameObject()->InvokeRMI(CActor::SvRequestSpawnEffect(),
                                                       CActor::SNetSpawnParticleParams(false, pos, name),
                                                       eRMI_ToServer);
    }

    return false;
}

bool CControlClient::NetPlayAnimAction(const char* action, const bool looping) const
{
    if (!m_pControlledActor)
        return false;
    
    if (!m_pControlledActor->GetAnimatedCharacter())
        return false;

    if (gEnv->bClient)
    {
        //TODO: play animation on this client --> request to our server --> play animation on all clients include this
        const string actionType = looping ? "Action" : "Signal";

        m_pControlledActor->GetGameObject()->InvokeRMI(
            CActor::SvRequestPlayAction(),
            CActor::PlayActionParams(action, actionType), eRMI_ToServer);

        return true;
    }
    return false;
}

CWeapon* CControlClient::GetCurrentWeapon(const CActor* pActor)
{
    if (!pActor)
        return nullptr;

    const IInventory* pInventory = pActor->GetInventory();
    if (!pInventory)
        return nullptr;

    const EntityId itemId = pInventory->GetCurrentItem();
    if (!itemId)
        return nullptr;

    IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
    if (!pItem)
        return nullptr;

    auto* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
    if (!pWeapon)
        return nullptr;

    return pWeapon;
}

IAIObject* CControlClient::GetCrosshairTargetAI() const
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

    return nullptr;
}

void CControlClient::SetActorAI(const IActor* pActor, const bool bToPlayerAI) const
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

bool CControlClient::DoNextActor(CActor* pNextActor)
{
    if (pNextActor)
    {
        pNextActor->GetEntity()->SetFlags(pNextActor->GetEntity()->GetFlags() | (ENTITY_FLAG_TRIGGER_AREAS));
        pNextActor->GetActorParams()->viewFoVScale = m_pLocalDude->GetActorParams()->viewFoVScale;

        IAIObject* pNextActorAI = pNextActor->GetEntity()->GetAI();
        if (pNextActorAI)
        {
            if (pNextActorAI->CastToIAIActor())
            {
                //Save AI values
                AgentParameters params = pNextActorAI->CastToIAIActor()->GetParameters();
                const int species = params.m_nSpecies;
                SetDudeSpecies(species);

                //Re-register controlled actor in AI System as AI Player
                if (pNextActorAI->GetAIType() == AIOBJECT_PUPPET)
                    SetActorAI(pNextActor, true);

                //Restore AI values to new ai pointer
                IAIObject* pControlledAI = pNextActor->GetEntity()->GetAI();
                assert(pControlledAI);
                if (pControlledAI)
                {
                    assert(pControlledAI->CastToIAIActor());
                    if (pControlledAI->CastToIAIActor())
                        pControlledAI->CastToIAIActor()->SetParameters(params);
                }
            }

            const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pNextActor, true);
            if (pSquad && pSquad->GetLeader())
                pSquad->OnPlayerAdded();
        }

        if (CWeapon* pWeapon = GetCurrentWeapon(pNextActor))
        {
            if (pWeapon->IsFiring())
                pWeapon->StopFire();
        }

        if (m_pLocalDude)
        {
            //TODO: I may have fixed "Pure function error"
            if (gEnv->bClient)
            {
                const auto params = CGameRules::OwnerParams(
                    m_pLocalDude->GetEntityId(),
                    pNextActor->GetEntityId());
                
                g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(
                    CGameRules::SvRequestSetOwner(), params,
                    eRMI_ToServer);
            }

            m_pLocalDude->GetEntity()->SetRotation(pNextActor->GetEntity()->GetRotation());
        }
        return true;
    }
    return false;
}

bool CControlClient::DoPrevActor(CActor* pActor) const
{
    if (pActor && pActor->GetHealth() > 0)
    {
        if (gEnv->bClient)
        {
            const auto ownerParams = CGameRules::OwnerParams(0, pActor->GetEntityId());
            g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(
                CGameRules::SvRequestSetOwner(), ownerParams,
                eRMI_ToServer);
            
            const auto pAlien = dynamic_cast<CAlien*>(pActor);
            pAlien->SetAlienMove(Vec3(0, 0, 0));
            pAlien->SetDesiredDirection(Vec3(0, 1, 0));

            pAlien->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);
        }

        if (CWeapon* pWeapon = GetCurrentWeapon(pActor))
        {
            if (pWeapon->IsFiring())
                pWeapon->StopFire();
        }

        if (const IAIObject* pAI = pActor->GetEntity()->GetAI())
        {
            if (pAI->GetAIType() == AIOBJECT_PLAYER)
                SetActorAI(pActor, false);
        }
        return true;
    }
    return false;
}

void CControlClient::SendPipeToAIGroup(const char* name, int groupId, bool useInComm, Vec3 refPoint) const
{
    auto pNomadPlayer = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());

    CActor* pControlActor = m_pControlledActor;
    IAIObject* pControlAI = pControlActor->GetEntity()->GetAI();

    if (!pControlActor || !pNomadPlayer || !pControlAI)
        return;

    const bool hasRefPoint = refPoint != Vec3(0, 0, 0);

    IEntityItPtr pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
    if (pEntityIterator && name)
    {
        pEntityIterator->MoveFirst();
        IEntity* pEntity = nullptr;

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
                                float distance = (pControlActor->GetEntity()->GetWorldPos() - pEntity->GetWorldPos()).
                                    GetLength();

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

void CControlClient::SetActor(CActor* pNewActor) //ÎÑÒÎÐÎÆÍÎ!! ÃÎÂÍÎÊÎÄ!!!
{
    assert(pNewActor);
    if (!pNewActor)
        return;

    const CActor* pOldActor = m_pControlledActor;

    SAFE_HUD_FUNC(GetCrosshair()->SetUsability(0))
    SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(false))

    m_generic.zoomScale = 0;
    m_generic.moveDir.zero();

    m_trooper.isBinocular = false;
    m_generic.isUsingBinocular = false;
    m_generic.isAiming = false;

    m_lookRequest.ClearAimTarget();

    //Hunter->Trooper camera fix
    SActorParams* pParams = m_pLocalDude->GetActorParams();
    pParams->vLimitDir.zero();
    pParams->vLimitRangeH = 0;
    pParams->vLimitRangeV = 0;

    if (!pOldActor)
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
            g_pControlSystem->StopLocal(false);
            return;
        }
    }
    else if (pOldActor && pOldActor != pNewActor)
    {
        DoPrevActor(m_pControlledActor); //The previous actor after controlling must return to normal state

        m_pControlledActor = pNewActor;
        //Take New Actor under control
        DoNextActor(m_pControlledActor);
    }
    m_pAbilitiesSystem->AddAbilityOwner(m_pControlledActor);
    //auto pAbilityOwner = m_pAbilitiesSystem->GetAbilityOwner(m_pControlledActor->GetEntityId());

    //New Health hud that add the abilities
    SetAmmoHealthHUD(m_pControlledActor, "Libs/UI/HUD_AmmoHealthEnergyAbilities.gfx");
    SetInventoryHUD(m_pControlledActor, "Libs/UI/HUD_WeaponSelectionAliens.gfx");

    //if (m_lastControlledId != m_pControlledActor->GetEntityId())
    //	m_lastControlledId = m_pControlledActor->GetEntityId();

    CAlien* pCurrentAlien = nullptr;
    if (m_pControlledActor && m_pControlledActor->IsAlien())
        pCurrentAlien = dynamic_cast<CAlien*>(m_pControlledActor);

    if (pCurrentAlien)
    {
        pCurrentAlien->GetAlienInput().movementVector.Set(0, 0, 0);

        if (GetActorClassName() == "Alien") //Not useful in update but necessary
        {
            //m_generic.canShoot = false;
            m_nakedAlien.Reset();

            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Alien");
        }
        else if (GetActorClassName() == "Scout")
        {
            const auto pScout = dynamic_cast<CScout*>(m_pControlledActor);
            pScout->EnableSearchBeam(false);
            pScout->m_params.forceView = 49.f;

            m_scout.Reset();
            m_generic.canShoot = true;

            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Scout");

            m_animScoutFlyInterface.Load("Libs/UI/HUD_ScoutFlyInterface.swf", eFD_Center,
                             eFAF_Visible | eFAF_ThisHandler);
        }
        else if (GetActorClassName() == "Trooper")
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
        else if (GetActorClassName() == "Drone")
        {
            m_generic.canShoot = true;

            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Trooper");
        }
        else if (GetActorClassName() == "Pinger")
        {
            m_generic.canShoot = true;

            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Hunter");
        }
        else
        {
            m_generic.canShoot = true;
        }
    }
    //m_pAbilitiesSystem->ReloadHUD();
    m_pAbilitiesSystem->UpdateHUD();

    //if (gEnv->bServer)
    //	m_pControlledActor->GetEntity()->SetTimer(ESyncTimers_GiveWeapons, 100);
}

void CControlClient::UpdateView(SViewParams& viewParams)
{
    if (!m_pControlledActor || !m_pControlledActor->GetEntity())
        return;

    viewParams.position = m_pControlledActor->GetEntity()->GetWorldPos();

    const auto pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
    if (pAlien)
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

        Vec3 offsetY(0, 0, 0);

        if (m_pControlledActor->IsAlien())
        {
            const Matrix33 alienWorldMtx(pAlien->GetGameObject()->GetEntity()->GetWorldTM());

            if (GetActorClassName() == "Hunter")
            {
                target(g_pGameCVars->ctrl_hrTargetx, g_pGameCVars->ctrl_hrTargety, g_pGameCVars->ctrl_hrTargetz);
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1() *
                    g_pGameCVars->ctrl_hrForwardOffset;
                currentFov = g_pGameCVars->ctrl_hrFov;
            }
            else if (GetActorClassName() == "Scout")
            {
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
                target(g_pGameCVars->ctrl_scTargetx, g_pGameCVars->ctrl_scTargety, g_pGameCVars->ctrl_scTargetz);
                currentFov = g_pGameCVars->ctrl_scFov;
            }
            else if (GetActorClassName() == "Trooper")
            {
                if (!m_pAbilitiesSystem->trooper.isCeiling)
                    target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
                else
                {
                    target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety,
                           g_pGameCVars->ctrl_trTargetz - 2.f);
                }
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
                currentFov = g_pGameCVars->ctrl_trFov;
            }
            else if (GetActorClassName() == "Drone")
            {
                target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
                currentFov = g_pGameCVars->ctrl_trFov;
            }
            else if (GetActorClassName() == "Pinger")
            {
                target(g_pGameCVars->ctrl_pgTargetx, g_pGameCVars->ctrl_pgTargety, g_pGameCVars->ctrl_pgTargetz);
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1();
                currentFov = g_pGameCVars->ctrl_pgFov;
            }
            else if (GetActorClassName() == "Alien")
            {
                target(g_pGameCVars->ctrl_alTargetx, g_pGameCVars->ctrl_alTargety, g_pGameCVars->ctrl_alTargetz);
                offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
                currentFov = g_pGameCVars->ctrl_alFov;
            }
        }

        const Vec3 offsetX = pAlien->GetViewRotation().GetColumn0() * current.x;
        const Vec3 offsetZ = pAlien->GetViewRotation().GetColumn2() * current.z;

        //Get skip entities
        IPhysicalEntity* pSkipEntities[10];
        int nSkip = 0;
        IItem* pItem = pAlien->GetCurrentItem();
        if (pItem)
        {
            CWeapon* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
            if (pWeapon)
                nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
        }

        const float oldLen = offsetY.len();

        // Ray cast to camera with offset position to check colliding
        const Vec3 eyeOffsetView = pAlien->GetStanceInfo(pAlien->GetStance())->viewOffset;
        const Vec3 start = (pAlien->GetAlienBaseMtx() * eyeOffsetView + viewParams.position + offsetX);
        // +offsetZ;// + offsetX;// +offsetZ;

        static float wallSafeDistance = 0.3f; // how far to keep camera from walls

        primitives::sphere sphere;
        sphere.center = start;
        sphere.r = wallSafeDistance;

        geom_contact* pContact = nullptr;
        const float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(
            sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
            &pContact, 0, rwi_stop_at_pierceable, nullptr, nullptr, 0, pSkipEntities, nSkip);

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
                const IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pAlien->m_searchbeam.itemId);
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
            float m_pForwardOffset = g_pGameCVars->ctrl_hrForwardOffset;
            viewParams.fov = g_pGameCVars->ctrl_hrFov * gf_PI / 180.0f;
            Vec3 CamViewdir = gEnv->pSystem->GetViewCamera().GetViewdir() * -m_pDistance;
            Matrix33 viewMtx(m_pAlien->GetGameObject()->GetEntity()->GetWorldTM());
            viewParams.position = pActorWorldPos + (viewMtx.GetColumn(2) * m_pHeight + viewMtx.GetColumn(1) * m_pForwardOffset) + CamViewdir;
        }*/
    }
}

void CControlClient::StoreCurrTime()
{
    m_storedTime = gEnv->pTimer->GetCurrTime();
}

bool CControlClient::CheckPassTime(const float passedSeconds) const
{
    if (gEnv->pTimer->GetCurrTime() - m_storedTime > passedSeconds)
        return true;
    return false;
}

void CControlClient::SubEnergy(const float subtractValue) const
{
    auto* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
    if (pAlien)
    {
        const float energy = pAlien->GetAlienEnergy();
        pAlien->SetAlienEnergy(energy - subtractValue);
    }
}

IEntity* CControlClient::GetMeleeTarget() const
{
    if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_meleeTargetId))
        return pEntity;
    
    return nullptr;
}

IEntity* CControlClient::GetLastCrosshairEntity() const
{
    IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_lastCrosshairTargetId);
    return pEntity;
}

IEntity* CControlClient::GetCrosshairEntity() const
{
    IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_crosshairTargetId);
    return pEntity;
}

IEntity* CControlClient::GetFireTarget() const
{
    if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_fireTargetId))
        return pEntity;
    return nullptr;
}

void CControlClient::OnClientHit(const HitInfo& hitInfo) const
{
    // local client side

    if (m_pControlledActor)
    {
        //On screen effects
        if (IsTrooper())
        {
            IMaterialEffects* pMaterialEffects = gEnv->pGame->GetIGameFramework()->GetIMaterialEffects();
            const TMFXEffectId id = pMaterialEffects->GetEffectIdByName("player_fx", "player_damage_armormode");

            SMFXRunTimeEffectParams params;

            params.pos = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldPos();
            params.soundSemantic = eSoundSemantic_HUD;

            pMaterialEffects->ExecuteEffect(id, params);
        }

        //Scout & hunter shields
        {
            if (IsScout() || IsHunter())
            {
            }
        }
    }
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

void CControlClient::GetMemoryStatistics(ICrySizer* s) const
{
    s->Add(*this);
}

void CControlClient::OnActorDeath(IActor* _pActor)
{
    auto pConqueror = g_pControlSystem->GetConquerorSystem();
    if (!pConqueror)
        return;

    auto pDeathActor = dynamic_cast<CActor*>(_pActor);
    if (!pDeathActor)
        return;

    auto isGameMode = pConqueror->IsGamemode();
    auto isControlling = g_pControlSystem->GetLocalEnabled();

    if (pDeathActor == m_pControlledActor)
    {
        m_animScoutFlyInterface.SetVisible(false);
        
        if (!gEnv->bMultiplayer)
        {
            if (!GetTutorialMode())
            {
                auto* pControlSquad = g_pControlSystem->m_pSquadSystem->GetSquadFromMember(m_pControlledActor, true);
                if (pControlSquad && !isGameMode)
                {
                    if (pControlSquad->HasClientLeader() || pControlSquad->GetMembersCount() > 0)
                    {
                        if (m_pLocalDude->GetHealth() > 0)
                        {
                            static float timer = 1.0f;

                            if (timer != 0.0f)
                            {
                                timer -= gEnv->pTimer->GetFrameTime();
                                if (timer < 0.0f)
                                    timer = 0.0f;
                            }

                            if (timer == 0.0f)
                            {
                                auto* pAliveMember = pControlSquad->GetMemberAlive();
                                if (pAliveMember)
                                {
                                    if (auto* pMemberActor = pControlSquad->GetActor(pAliveMember->GetId()))
                                    {
                                        SetActor(pMemberActor);
                                        pControlSquad->SetLeader(pMemberActor, false);
                                        timer = 1.0f;

                                        CryLogAlways(
                                            "[Squad Leader][Dude is Alive][Controlled is Dead]--->> [Select Alive Member][Setup New Leader][Control Him]");
                                    }
                                }
                            }
                        }
                    }
                    else if (pControlSquad->HasClientMember() && pControlSquad->GetMembersCount() > 0)
                    {
                        if (m_pLocalDude->GetHealth() > 0)
                        {
                            static float timer = 1.0f;

                            if (timer != 0.0f)
                            {
                                timer -= gEnv->pTimer->GetFrameTime();
                                if (timer < 0.0f)
                                    timer = 0.0f;
                            }

                            if (timer == 0.0f)
                            {
                                auto* pAliveMember = pControlSquad->GetMemberAlive();
                                if (pAliveMember)
                                {
                                    if (auto* pMember = pControlSquad->GetActor(pAliveMember->GetId()))
                                    {
                                        SetActor(pMember);
                                        timer = 1.0f;

                                        CryLogAlways(
                                            "[Squad Member][Dude is Alive][Controlled is Dead]--->>[Select Alive Member][Control Him]");
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (m_pLocalDude->GetHealth() > 0)
                    {
                        //Fix the binocular non hiding after Dude die
                        if (SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
                            SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(0))

                        //if not squad controlled is dead, dude is alive --> kill dude
                        HitInfo hitInfo(
                            m_pLocalDude->GetEntityId(),
                            m_pLocalDude->GetEntityId(),
                            m_pLocalDude->GetEntityId(),
                            -1, 0, 0, -1, 0, ZERO, ZERO, ZERO);

                        hitInfo.SetDamage(15000);
                        g_pGame->GetGameRules()->ClientHit(hitInfo);

                        CryLogAlways("[Not Squad][Dude is Alive][Controlled is Dead]--->>[Kill Dude]");

                        //Fix the Dude stucking in controlled actor after death
                        ToggleDudeBeam(false);

                        Vec3 vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();
                        vControlledPos.z += 15.f;

                        Matrix34 nomadMat34 = m_pLocalDude->GetEntity()->GetWorldTM();
                        nomadMat34.SetTranslation(vControlledPos);
                        m_pLocalDude->GetEntity()->SetWorldTM(nomadMat34);
                    }
                }
            }
            else if (GetTutorialMode())
            {
                if (m_pLocalDude->GetHealth() > 0)
                {
                    //--> if not squad controlled is dead, player is alive
                    const int maxHealth = m_pControlledActor->GetMaxHealth();

                    m_pControlledActor->StandUp();
                    m_pControlledActor->Revive(true);
                    m_pControlledActor->SetHealth(maxHealth);

                    if (m_pControlledActor->GetEntity()->GetAI())
                        SetActorAI(m_pControlledActor, true);

                    CryLogAlways(
                        "[Tutorial Mode][Not Squad][Dude is Alive][Controlled is Dead]--->>[Revive Controlled]");
                }
            }
        }
        else
        {
            if (isControlling)
            {
                InitDudeToControl(false);
                Reset(false);
            }

            if (gEnv->bClient)
            {
                HitInfo hit;
                hit.targetId = m_pLocalDude->GetEntityId();
                hit.SetDamage(100000);
                //hit.remote = true;

                g_pGame->GetGameRules()->ClientHit(hit);
            }
        }
    }
    else if (pDeathActor == m_pLocalDude)
    {
        if (!gEnv->bMultiplayer)
        {
            if (m_pControlledActor && m_pControlledActor->GetHealth() > 0)
            {
                //Fix the binocular non hiding after Dude die
                if (SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
                    SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(0))

                //not squad Dude is dead, controlled is alive

                IScriptTable* pTable = m_pControlledActor->GetEntity()->GetScriptTable();
                if (pTable)
                    Script::CallMethod(pTable, "Event_Kill");

                // fix the player stucking in controlled actor after death	
                ToggleDudeBeam(false);

                Vec3 vControlledPos = m_pControlledActor->GetEntity()->GetWorldPos();
                vControlledPos.z += 15.f;

                Matrix34 nomadMat34 = pDeathActor->GetEntity()->GetWorldTM();
                nomadMat34.SetTranslation(vControlledPos);
                pDeathActor->GetEntity()->SetWorldTM(nomadMat34);

                CryLogAlways("[NOT Squad][Dude is Dead][Controlled is Alive]--->>[Kill Controlled]");
            }
        }
        else
        {
            if (isControlling)
            {
                InitDudeToControl(false);
                Reset(false);
            }
        }


        //if (m_pLocalPlayer->GetHealth() > 0.1f)
        //{
        //	pActor->StandUp();
        //	pActor->Revive(true);
        //	pActor->SetHealth(200);

        //	if (!pActor->IsThirdPerson())
        //		pActor->ToggleThirdPerson();

        //	pActor->GetActorStats()->isHidden = true;
        //	pActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

        //	g_pGame->GetHUD()->RebootHUD();
        //	g_pGame->GetHUD()->GetScopes()->m_animBinoculars.Reload();

        //	IScriptTable* pTable = m_pControlledActor->GetEntity()->GetScriptTable();
        //	if (pTable)
        //		Script::CallMethod(pTable, "Event_Kill");
        //	CryLogAlways("In squad dude is Dead, controlled actor is Alive --->> revive dude, kill controlled actor ***");
        //}
    }
}

void CControlClient::OnChangedSpectatorMode(const IActor* pActor, const uint8 mode, EntityId targetId, bool resetAll)
{
    if (gEnv->bMultiplayer)
    {
        if (pActor->GetEntityId() == m_pLocalDude->GetEntityId())
        {
            const bool isControllingSomebody = m_pControlledActor != nullptr;

            if (m_lastSpectatorMode != mode)
                m_lastSpectatorMode = mode;

            switch (mode)
            {
            case CActor::eASM_None:
                CryLogAlways("[C++][CLIENT][ControlClient][OnChangedSpectatorMode][eASM_None]");
                break;
            case CActor::eASM_FirstMPMode:
                CryLogAlways("[C++][CLIENT][ControlClient][OnChangedSpectatorMode][eASM_FirstMPMode]");
                break;
            case CActor::eASM_Free:
                CryLogAlways("[C++][CLIENT][ControlClient][OnChangedSpectatorMode][eASM_Free]");
                break;
            case CActor::eASM_LastMPMode:
                CryLogAlways("[C++][CLIENT][ControlClient][OnChangedSpectatorMode][eASM_LastMPMode]");
                break;
            case CActor::eASM_Cutscene:
                CryLogAlways("[C++][CLIENT][ControlClient][OnChangedSpectatorMode][eASM_Cutscene]");
                break;
            default:
                break;
            }

            if (isControllingSomebody)
            {
                if (mode != CActor::eASM_None)
                {
                    //Controlling -> Spectator -> Stop Control -> ???
                    InitDudeToControl(false);
                    Reset(false);
                    //g_pGame->GetGameRules()->StopControl(m_pLocalPlayer->GetEntityId());
                }
            }
            else
            {
                if (m_mpLastControlledId && mode == CActor::eASM_None)
                {
                    //Spectator->Continue control (if alienPlayerFlag)
                    InitDudeToControl(true);

                    const auto* lastActor = dynamic_cast<CActor*>(
                        g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(
                        m_mpLastControlledId));
                    
                    g_pGame->GetGameRules()->StartControl(lastActor->GetEntityId(), m_pLocalDude->GetEntityId(), false);
                    /*SetActor(lastActor);
                    ToggleDudeBeam(true);
                    ToggleDudeHide(true);*/
                }
            }
        }
    }
}

EntityId CControlClient::GetCrosshairEntityId() const
{
    return m_crosshairTargetId;
}

EntityId CControlClient::GetScoutAutoAimTargetId() const
{
    return m_scoutAimTargetId;
}
