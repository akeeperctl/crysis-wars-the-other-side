#include "stdafx.h"

#include "HUD/HUD.h"
#include "Menus/FlashMenuObject.h"

#include "Trooper.h"
#include "Scout.h"
#include "Hunter.h"

#include "../Control/ControlSystem.h"
#include "../Squad/SquadSystem.h"

#include "AbilitiesSystem.h"


//TActionHandler<CAbilitiesSystem> CAbilitiesSystem::s_actionHandler;

CAbilitiesSystem::CAbilitiesSystem()
    : m_isDebugLog(false)
{
    Reset();
}

CAbilitiesSystem::~CAbilitiesSystem()
{
    if (g_pControlSystem)
        g_pControlSystem->RemoveChild(this, false);
}

void CAbilitiesSystem::OnMainMenuEnter()
{
    Reset();
}

void CAbilitiesSystem::Update(const float frameTime)
{
    m_isDebugLog = g_pGameCVars->abil_debug_log == 1;

    UpdateEffectsHUD();
    
    for (auto& abilityOwner : m_abilityOwners)
    {
        if (auto* pActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(
            abilityOwner.m_ownerId)))
        {
            const string actorClass = pActor->GetEntity()->GetClass()->GetName();

            //Debug
            for (auto& ability : abilityOwner.m_abilities)
            {
                if (pActor == g_pControlSystem->GetClientActor() && g_pGameCVars->abil_debug_draw_info > 0)
                {
                    static float color[] = {1, 1, 1, 1};
                    const float size = 1.2f;
                    const float scale = 30.0f;
                    const float xOffset = 60.0f;
                    const float yOffset = 30.0f;
                    const auto index = static_cast<float>(ability.index);

                    gEnv->pRenderer->Draw2dLabel(xOffset, yOffset + scale * index, size, color, false,
                                                 "Ability %i, Name %s, State %i, Visible Mode %i",
                                                 ability.index, ability.name.c_str(), ability.state, ability.visMode);
                }

                if (actorClass == "Trooper")
                {
                    const auto pTrooper = dynamic_cast<CTrooper*>(pActor);

                    if (ability.name == ABILITY_TROOPER_RAGE)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);

                                const int maxHealth = pActor->GetMaxHealth() / 2;
                                const int curHealth = pActor->GetHealth();

                                if (curHealth > maxHealth)
                                    ability.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                if (pTrooper->m_rageMode.rageMaxDuration == 0.0f)
                                    ability.SetState(eAbilityState_Cooldown);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                /*if (pTrooper->m_rageMode.rageMaxDuration != 0)
                                    pTrooper->m_rageMode.rageMaxDuration = 0;*/
                                pTrooper->m_rageMode.ToggleMode(false);

                                if (pTrooper->m_rageMode.reloadDuration == 0.0f)
                                    ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                const int maxHealth = pActor->GetMaxHealth() / 2;
                                const int curHealth = pActor->GetHealth();

                                if (curHealth <= maxHealth)
                                    ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                    else if (ability.name == ABILITY_TROOPER_EMP)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                ability.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                if (trooper.shockwaveTimer == 0.0f)
                                    ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                    else if (ability.name == ABILITY_TROOPER_SHIELD)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);

                                const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pActor);
                                if (pSquad && pSquad->GetLeader())
                                {
                                    /*if (squad.GetMembersCount() > 0)
                                        {
                                            if (squad.GetMinDistance() > pTrooper->m_shieldParams.GetRange())
                                                shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
                                        }
                                        else*/
                                    if (pSquad->GetMembersCount() == 0)
                                        ability.SetState(eAbilityState_Disabled_By_Condition);
                                }
                                else if (!pSquad)
                                    ability.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pActor);
                                if (pSquad && pSquad->GetLeader())
                                {
                                    /*if (squad.GetMembersCount() > 0)
                                    {
                                        if (squad.GetMinDistance() < pTrooper->m_shieldParams.GetRange())
                                            shieldAbility.SetState(eAbilityState_Disabled_By_Condition);
                                    }
                                    else*/
                                    if (pSquad->GetMembersCount() == 0)
                                        ability.SetState(eAbilityState_Disabled_By_Condition);

                                    if (pTrooper->m_shieldParams.canGuardianShieldProj != true)
                                        pTrooper->m_shieldParams.canGuardianShieldProj = true;
                                }
                            }
                            break;
                        case eAbilityState_Cooldown:
                        //ability.SetState(eAbilityState_Disabled_By_Condition);
                        //break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                if (pTrooper->m_shieldParams.canGuardianShieldProj != false)
                                    pTrooper->m_shieldParams.canGuardianShieldProj = false;

                                const auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromLeader(pActor);
                                if (pSquad && pSquad->GetLeader())
                                {
                                    if (pSquad->GetMembersCount() > 0)
                                    {
                                        //if (squad.GetMinDistance() < pTrooper->m_shieldParams.GetRange())
                                        ability.SetState(eAbilityState_Ready_To_Activate);
                                    }
                                }
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            CryLogAlways("Default Ability State");
                            break;
                        }
                    }
                    else if (ability.name == ABILITY_TROOPER_CLOAK)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                //float energy = pTrooper->GetAlienEnergy();
                                //pTrooper->SetAlienEnergy(clamp(energy + recharge * frametime, 0.0f, NANOSUIT_ENERGY));
                            }
                            break;
                        case eAbilityState_Cooldown:
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                }
                else if (actorClass == "Scout")
                {
                    const auto pScout = dynamic_cast<CScout*>(pActor);

                    if (ability.name == ABILITY_SCOUT_SPOTLIGHT)
                    {
                        //if (!gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
                        //ability.SetState(eAbilityState_Disabled_By_Condition);
                        if (!gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
                            pScout->InitSearchBeam();

                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            {
                                scout.IsSearch = false;
                                //m_bDisableLookAt = false;
                                g_pControlSystem->GetLocalControlClient()->GetGenericParams().canShoot = true;

                                pScout->EnableSearchBeam(false);

                                ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                if (gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
                                    ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                    else if (ability.name == ABILITY_SCOUT_ANTIGRAV)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);

                                if (scout.isGyroEnabled == false)
                                    ability.SetState(eAbilityState_Disabled_By_Condition);

                                //if (!gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId))
                                //	antigravAbility.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                if (scout.isGyroEnabled == true)
                                    scout.isGyroEnabled = false;
                            }
                            break;
                        case eAbilityState_Cooldown:
                        //{
                        //	ability.SetState(eAbilityState_Disabled_By_Condition);
                        //}
                        //break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                scout.isGyroEnabled = true;
                                ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                    else if (ability.name == ABILITY_SCOUT_OBJECTGRAB)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            {
                                ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                }
                else if (actorClass == "Hunter")
                {
                    const auto pHunter = dynamic_cast<CHunter*>(pActor);

                    if (ability.name == ABILITY_HUNTER_GRAB)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            {
                                ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);
                            }
                            break;
                        case eAbilityState_ForSync: break;
                        default: ;
                        }
                    }
                    else if (ability.name == ABILITY_HUNTER_SHIELD)
                    {
                        switch (ability.GetState())
                        {
                        case eAbilityState_Ready_To_Activate:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Show)
                                    ability.SetVisibleMode(eAbilityMode_Show);

                                if (pHunter->GetAlienEnergy() <= 0.0f)
                                    ability.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Activated:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Activated)
                                    ability.SetVisibleMode(eAbilityMode_Activated);

                                //Passive energy drain of hunter's shield now disabled
                                /*const float maxShieldEnergy = pHunter->GetMaxAlienEnergy();
                                const float currentShieldEnergy = pHunter->GetAlienEnergy();
                                const float divider = 100.0f;
                                const float deltaEnergy = maxShieldEnergy / divider;
    
                                float frametime = gEnv->pTimer->GetFrameTime();
                                static float timer = 1.0f;
    
                                if (timer > 0.0f)
                                    timer -= frametime;
    
                                if (timer < 0.0f)
                                {
                                    pHunter->SetAlienEnergy(currentShieldEnergy - deltaEnergy);
                                    timer = 1.0f;
                                }*/

                                if (pHunter->GetAlienEnergy() <= 0.0f)
                                    ability.SetState(eAbilityState_Disabled_By_Condition);
                            }
                            break;
                        case eAbilityState_Cooldown:
                            {
                                ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_Disabled_By_Condition:
                            {
                                if (ability.GetVisibleMode() != eAbilityMode_Disabled)
                                    ability.SetVisibleMode(eAbilityMode_Disabled);

                                pHunter->GetEnergyParams().isHunterShieldEnabled = false;

                                if (pHunter->GetAlienEnergy() > 0.2f)
                                    ability.SetState(eAbilityState_Ready_To_Activate);
                            }
                            break;
                        case eAbilityState_ForSync:
                        default:
                            break;
                        }
                    }
                }
                else if (actorClass == "Grunt" || actorClass == "Player")
                {
                    const auto pPlayer = dynamic_cast<CPlayer*>(pActor);

                    if (ability.name == ABILITY_HUMAN_VEHICLE_HEALING)
                    {
                        if (!pPlayer->IsPlayer())
                        {
                            //This is a passive ability for AI, so it must be enabled at all times.
                            if (ability.GetState() != eAbilityState_Activated)
                                ability.SetState(eAbilityState_Activated);

                            //UI for this ability now is not used 04.09.2022
                            //if (ability.GetVisibleMode() != eAbilityMode_Activated)
                            //	ability.SetVisibleMode(eAbilityMode_Activated);

                            const auto pVehicle = pPlayer->GetLinkedVehicle();
                            if (pVehicle && pVehicle->GetDamageRatio() > 0)
                            {
                                const float constDelay = 0.9f; //in sec
                                static float healingUpDelay = constDelay;

                                if (healingUpDelay != 0.0f)
                                    healingUpDelay -= frameTime;

                                if (healingUpDelay <= 0)
                                {
                                    //Time to healing the vehicle

                                    HitInfo info;
                                    info.SetDamage(Random(5.f, 11.f));
                                    info.type = g_pGame->GetIGameFramework()->GetIGameRulesSystem()->
                                                         GetCurrentGameRules()->GetHitTypeId("repair");
                                    info.targetId = pVehicle->GetEntityId();
                                    info.shooterId = pPlayer->GetEntityId();

                                    g_pGame->GetGameRules()->ClientHit(info);

                                    healingUpDelay = constDelay;
                                }
                            }
                        }
                        else
                        {
                            //For client player see OnAddingAbility
                        }
                    }
                }
            }
        }
    }
}

void CAbilitiesSystem::Serialize(TSerialize ser)
{
    if (ser.GetSerializationTarget() != eST_Network && !gEnv->bEditor)
    {
        if (ser.IsReading())
            Reset();

        ser.BeginGroup("CAbilitiesSystem");

        trooper.Serialize(ser);
        scout.Serialize(ser);
        hunter.Serialize(ser);

        for (auto& owner : m_abilityOwners)
            owner.Serialize(ser);

        ser.EndGroup();
    }
}

void CAbilitiesSystem::UpdateHUD()
{
    if (!g_pGame->GetHUD())
        return;

    //if (!m_animAbilitiesHUD.IsLoaded())
    //	return;

    const CGameFlashAnimation* animAmmo = &g_pGame->GetHUD()->m_animPlayerStats;
    if (!animAmmo->IsLoaded())
        return;

    UpdateEffectsHUD();
    UpdateVisibleModeHUD();
}

void CAbilitiesSystem::UpdateEffectsHUD()
{
    if (!g_pGame->GetHUD())
        return;

    //if (!m_animAbilitiesHUD.IsLoaded())
    //	return;

    const auto pAnimAmmo = &g_pGame->GetHUD()->m_animPlayerStats;
    if (!pAnimAmmo->IsLoaded())
        return;

    const auto pCC = g_pControlSystem->GetLocalControlClient();
    if (pCC)
    {
        auto pPlayer = pCC->GetControlledActor();
        if (!pPlayer)
            pPlayer = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

        if (pPlayer)
        {
            if (IsAbilityOwner(pPlayer->GetEntityId()))
            {
                const auto pOwner = GetAbilityOwner(pPlayer->GetEntityId());
                if (pOwner->GetAbilityCount() > 0)
                {
                    const int count = pOwner->GetAbilityCount();

                    for (int i = 0; i <= count; i++)
                    {
                        const auto pAbility = pOwner->GetAbility(i);
                        if (pAbility)
                        {
                            const SFlashVarValue argsEffect[2] = {i, pAbility->name.c_str()};
                            pAnimAmmo->Invoke("setIconEffect", argsEffect, 2);
                        }
                    }
                }
            }
        }
    }
}

void CAbilitiesSystem::UpdateVisibleModeHUD()
{
    if (!g_pGame->GetHUD())
        return;

    const auto pAnimPlayerStats = &g_pGame->GetHUD()->m_animPlayerStats;
    if (!pAnimPlayerStats->IsLoaded())
        return;

    const auto pPlayer = g_pControlSystem->GetClientActor();
    if (pPlayer)
    {
        const auto pOwner = GetAbilityOwner(pPlayer->GetEntityId());
        if (pOwner)
        {
            for (auto& ability : pOwner->m_abilities)
            {
                const char* visMode = nullptr;
                bool isActivatedMode = false;

                switch (ability.GetVisibleMode())
                {
                case eAbilityMode_Hide:
                    visMode = VISIBLE_MODE_HIDE;
                    break;
                case eAbilityMode_Show:
                    visMode = VISIBLE_MODE_SHOW;
                    break;
                case eAbilityMode_Disabled:
                    visMode = VISIBLE_MODE_DISABLED;
                    break;
                case eAbilityMode_Activated:
                    visMode = VISIBLE_MODE_ACTIVATED;
                    isActivatedMode = true;
                    break;
                case eAbilityMode_ForSync:
                default:
                    break;
                }

                if (!isActivatedMode)
                {
                    const SFlashVarValue argsMode[2] = {ability.index, visMode};
                    pAnimPlayerStats->Invoke("setIconMode", argsMode, 2);
                }
                else
                    pAnimPlayerStats->Invoke("setIconActivated", ability.index);
            }
        }
    }
}

CAbilityOwner* CAbilitiesSystem::AddAbilityOwner(const IActor* pActor)
{
    if (pActor)
    {
        // auto it = m_abilityOwners.begin();
        // const auto end = m_abilityOwners.end();
        //
        // for (; it != end; ++it)
        // {
        //     auto& owner = *it;
        //
        //     if (it->m_ownerId == pActor->GetEntityId())
        //         return &owner;
        // }

        if (!IsAbilityOwner(pActor))
        {
            CAbilityOwner newOwner(pActor->GetEntityId());
            m_abilityOwners.push_back(newOwner);

            return &newOwner;
        }
    }

    return nullptr;
}

bool CAbilitiesSystem::RemoveAbilityOwner(const CAbilityOwner& owner)
{
    auto it = m_abilityOwners.begin();
    const auto end = m_abilityOwners.end();

    for (; it != end; ++it)
    {
        if (it->m_ownerId == owner.m_ownerId)
        {
            m_abilityOwners.erase(it);
            return true;
        }
    }

    return false;
}

bool CAbilitiesSystem::OnInputLeftAbility(const EntityId entityId, const EInputState activationMode, float value)
{
    if (activationMode == eIS_Pressed)
    {
        const auto pOwner = GetAbilityOwner(entityId);
        if (pOwner && pOwner->GetAbilityCount() != 0)
        {
            pOwner->ToggleAbility(1, 0);
            return true;
        }
    }

    return false;
}

bool CAbilitiesSystem::OnInputCenterAbility(const EntityId entityId, const EInputState activationMode, float value)
{
    if (activationMode == eIS_Pressed)
    {
        const auto pOwner = GetAbilityOwner(entityId);
        if (pOwner && pOwner->GetAbilityCount() != 0)
        {
            pOwner->ToggleAbility(2, 0);
            return true;
        }
    }

    return false;
}

bool CAbilitiesSystem::OnInputRightAbility(const EntityId entityId, const EInputState activationMode, float value)
{
    if (activationMode == eIS_Pressed)
    {
        //bool isHumanMode = false;

        //auto pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId));
        //if (pPlayer)
        //isHumanMode = pPlayer->IsHumanMode();

        const auto pOwner = GetAbilityOwner(entityId);
        if ((pOwner && pOwner->GetAbilityCount() != 0) /*&& isHumanMode*/)
        {
            pOwner->ToggleAbility(3, 0);
            return true;
        }
    }

    return false;
}

CAbilityOwner* CAbilitiesSystem::GetAbilityOwner(const IActor* pActor)
{
    if (pActor)
    {
        for (auto& owner : m_abilityOwners)
        {
            if (owner.m_ownerId == pActor->GetEntityId())
                return &owner;
        }
    }

    return nullptr;
}

CAbilityOwner* CAbilitiesSystem::GetAbilityOwner(const EntityId id)
{
    if (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
    {
        for (auto& owner : m_abilityOwners)
        {
            if (owner.m_ownerId == id)
                return &owner;
        }
    }

    return nullptr;
}

bool CAbilitiesSystem::IsAbilityOwner(const IActor* pActor)
{
    if (pActor)
    {
        auto it = m_abilityOwners.begin();
        const auto end = m_abilityOwners.end();

        for (; it != end; ++it)
        {
            if (it->m_ownerId == pActor->GetEntityId())
                return true;
        }
    }

    return false;
}

bool CAbilitiesSystem::IsAbilityOwner(const EntityId id)
{
    if (g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id))
    {
        auto it = m_abilityOwners.begin();
        const auto end = m_abilityOwners.end();

        for (; it != end; ++it)
        {
            if (it->m_ownerId == id)
                return true;
        }
    }

    return false;
}

void CAbilitiesSystem::OnGameRulesReset()
{
    Reset();

    //CryLogAlways("[C++][CAbilitiesSystem][OnGameRulesReset]");
}

void CAbilitiesSystem::OnActorDeath(IActor* pActor)
{
    if (pActor)
    {
        const auto pAbilityOwner = GetAbilityOwner(pActor);
        if (pAbilityOwner)
        {
            for (auto& ability : pAbilityOwner->m_abilities)
            {
                pAbilityOwner->DeactivateAbility(&ability);

                //ability.SetState(eAbilityState_Cooldown);
                //ability.SetVisibleMode(eAbilityMode_Hide);
            }
        }
    }
}

void CAbilitiesSystem::Reset()
{
    trooper.Reset();
    scout.Reset();
    hunter.Reset();

    m_abilityOwners.clear();
}

void CAbilitiesSystem::STrooperAbilities::Reset()
{
    shockwaveTimer = 0;
    rageDuration = 30.0f;

    eCeilingType = eCeilingType_Ceiling;

    isRageMode = false;
    //isProjectingShield = false;

    canCeiling = false;
    doCeiling = false;
    isCeiling = false;
}

void CAbilitiesSystem::STrooperAbilities::Serialize(TSerialize ser)
{
    ser.BeginGroup("TrooperAbilities");
    ser.Value("isRageMode", isRageMode);
    //ser.Value("isProjectingShield", isProjectingShield);
    ser.Value("rageDuration", rageDuration);
    ser.Value("shockwaveTimer", shockwaveTimer);
    ser.Value("canCeiling", canCeiling);
    ser.Value("doCeiling", doCeiling);
    ser.Value("isCeiling", isCeiling);
    ser.EndGroup();
}

void CAbilitiesSystem::SScoutAbilities::Reset()
{
    abilityPressNum = 0;

    IsSearch = false;
    isGyroEnabled = true;
}

void CAbilitiesSystem::SScoutAbilities::Serialize(TSerialize ser)
{
    ser.BeginGroup("ScoutAbilities");
    ser.Value("abilityPressNum", abilityPressNum);
    ser.Value("IsSearch", IsSearch);
    ser.Value("isGyroEnabled", isGyroEnabled);
    ser.EndGroup();
}

void CAbilitiesSystem::SHunterAbilities::Reset()
{
    IsShieldEnabled = false;
}

void CAbilitiesSystem::SHunterAbilities::Serialize(TSerialize ser)
{
    ser.BeginGroup("HunterAbilities");
    ser.Value("IsShieldEnabled", IsShieldEnabled);
    ser.EndGroup();
}

//int CAbilityOwner::AddAbility(const char* name, int index)
//{
//	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
//	if (!pPlayer)
//		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
//
//	if (IsHaveAbility(name))
//		return -1;
//
//	CAbility ability;
//	ability.m_index = index;
//
//	m_abilities.push_back(ability);
//	return index;
//}

void CAbilitiesSystem::GetMemoryStatistics(ICrySizer* s)
{
    s->Add(*this);
    s->AddContainer(m_abilityOwners);

    auto it = m_abilityOwners.begin();
    const auto end = m_abilityOwners.end();

    for (; it != end; ++it)
        it->GetMemoryStatistics(s);
}

void CAbilitiesSystem::OnActorGrabbed(IActor* pActor, EntityId grabberId)
{
}

void CAbilitiesSystem::OnActorDropped(IActor* pActor, EntityId droppedId)
{
}

void CAbilitiesSystem::OnActorGrab(IActor* pActor, EntityId grabId)
{
}

void CAbilitiesSystem::OnActorDrop(IActor* pActor, EntityId dropId)
{
}

void CAbilitiesSystem::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle)
{
}

void CAbilitiesSystem::OnExitVehicle(IActor* pActor)
{
}

bool CAbilitiesSystem::OnInputEvent(const SInputEvent& event)
{
    auto* pDude = dynamic_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
    if (!pDude)
        return false;

    const auto pControlClient = pDude->GetControlClient();
    if (!pControlClient)
        return false;

    auto pPlayer = pControlClient->GetControlledActor();
    if (!pPlayer)
        pPlayer = pDude;

    if (gEnv->pConsole->IsOpened())
        return false;

    if (event.keyName.c_str() && event.keyName.c_str()[0] == 0)
        return false;

    const auto inGamePaused = g_pGame->GetIGameFramework()->IsGamePaused();
    const auto playerId = pPlayer->GetEntityId();

    if (!inGamePaused && pPlayer->GetHealth() > 0)
    {
        if (event.deviceId == eDI_Keyboard)
        {
            if (event.keyId == eKI_X)
            {
                OnInputLeftAbility(playerId, event.state, event.value);
            }
            if (event.keyId == eKI_C)
            {
                OnInputCenterAbility(playerId, event.state, event.value);
            }
            if (event.keyId == eKI_V)
            {
                OnInputRightAbility(playerId, event.state, event.value);
            }
        }
    }
    return false;
}

void CAbilitiesSystem::Init()
{
    g_pControlSystem->AddChild(this, false);
}

void CAbilitiesSystem::OnAddingAbility(const char* abilityName, const EntityId ownerId)
{
    if (ownerId == g_pGame->GetIGameFramework()->GetClientActorId())
    {
        const auto pOwner = GetAbilityOwner(ownerId);
        if (pOwner)
        {
            const string ability(abilityName);

            if (ability == ABILITY_HUMAN_VEHICLE_HEALING)
            {
                //Disable because repairKit not work at third person view

                //auto pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId));
                //g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pPlayer,"RepairKit",true,true,true);
            }

            if (ability == ABILITY_SCOUT_SPOTLIGHT)
            {
                const auto pScout = dynamic_cast<CScout*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId));
                if (!pScout->InitSearchBeam())
                    CryLogAlways("%s[C++][Error][Scout %s][Can't init search beam]", STR_RED,
                                 pScout->GetEntity()->GetName());
            }
        }
    }
}
