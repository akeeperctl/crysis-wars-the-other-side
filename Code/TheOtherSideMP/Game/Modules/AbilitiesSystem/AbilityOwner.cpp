#include "StdAfx.h"

#include "IEntityProxy.h"
#include <IVehicleSystem.h>
#include "ICryAnimation.h"

#include "HUD/HUD.h"

#include "Game.h"
#include "GameRules.h"

#include "Scout.h"
#include "Hunter.h"
#include "Trooper.h"

#include "../Control/ControlSystem.h"
#include "../Squad/SquadSystem.h"

#include "../Helpers/TOS_AI.h"
#include "../Helpers/TOS_Vehicle.h"
#include "../Helpers/TOS_HUD.h"

#include "AbilitiesSystem.h"


int CAbilityOwner::AddAbility(const char* name)
{
    if (IsHaveAbility(name))
        return -1;

    CAbility ability;
    ability.index = static_cast<int>(m_abilities.size()) + 1;
    ability.name = name;
    ability.abilityOwnerId = m_ownerId;
    //ability.SetVisibleMode(eAbilityMode_Show);
    m_abilities.push_back(ability);

    g_pControlSystem->GetAbilitiesSystem()->OnAddingAbility(name, m_ownerId);

    return ability.index;
}

int CAbilityOwner::AddAbility(CAbility& ability)
{
    if (IsHaveAbility(ability))
        return -1;

    ability.index = static_cast<int>(m_abilities.size()) + 1;
    ability.abilityOwnerId = m_ownerId;
    m_abilities.push_back(ability);

    g_pControlSystem->GetAbilitiesSystem()->OnAddingAbility(ability.name.c_str(), m_ownerId);

    return ability.index;
}

bool CAbilityOwner::RemoveAbility(const char* name)
{
    auto it = m_abilities.begin();
    auto end = m_abilities.end();

    bool isFounded = false;

    for (; it != end; ++it)
    {
        string abilityName = it->name;
        if (abilityName == name)
        {
            isFounded = true;
            break;
        }
    }

    if (isFounded)
    {
        m_abilities.erase(it);

        it = m_abilities.begin();
        end = m_abilities.end();

        for (; it != end; ++it)
        {
            if (it->index > 0)
                it->index -= 1;
        }

        return true;
    }
    return false;
}

bool CAbilityOwner::RemoveAbility(const int index)
{
    auto it = m_abilities.begin();
    auto end = m_abilities.end();

    bool isFounded = false;

    for (; it != end; ++it)
    {
        if (it->index == index)
        {
            isFounded = true;
            break;
        }
    }

    if (isFounded)
    {
        m_abilities.erase(it);

        it = m_abilities.begin();
        end = m_abilities.end();
        for (; it != end; ++it)
        {
            if (it->index > 0)
                it->index -= 1;
        }

        return true;
    }
    return false;
}

bool CAbilityOwner::RemoveAbility(const CAbility& ability)
{
    auto it = m_abilities.begin();
    auto end = m_abilities.end();

    bool isFounded = false;

    for (; it != end; ++it)
    {
        string abilityName = it->name;
        if (abilityName == ability.name)
        {
            isFounded = true;
            break;
        }
    }

    if (isFounded)
    {
        m_abilities.erase(it);

        it = m_abilities.begin();
        end = m_abilities.end();
        
        for (; it != end; ++it)
        {
            if (it->index > 0)
                it->index -= 1;
        }

        return true;
    }
    return false;
}

bool CAbilityOwner::IsHaveAbility(const char* name)
{
    for (auto& ability : m_abilities)
    {
        if (ability.GetName() == name)
            return true;
    }

    return false;
}

bool CAbilityOwner::IsHaveAbility(const int index)
{
    auto it = m_abilities.begin();
    const auto end = m_abilities.end();
    
    for (; it != end; ++it)
    {
        if (index == it->index)
            return true;
    }

    return false;
}

bool CAbilityOwner::IsHaveAbility(const CAbility& ability)
{
    auto it = m_abilities.begin();
    const auto end = m_abilities.end();

    for (; it != end; ++it)
    {
        string& abilityName = it->name;
        if (abilityName == ability.name)
            return true;
    }

    return false;
}

CAbility* CAbilityOwner::GetAbility(const char* name)
{
    //auto it = m_abilities.begin();
    //auto end = m_abilities.end();

    for (auto& ability : m_abilities)
    {
        string abilityName = name;
        if (ability.name == abilityName)
            return &ability;
    }

    //CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "[CAbility &CAbilityOwner::GetAbility name] Get nullAbility");
    //CryLogAlways("[CAbility &CAbilityOwner::GetAbility id] Get nullAbility");
    return nullptr;
}

CAbility* CAbilityOwner::GetAbility(int index)
{
    //auto it = m_abilities.begin();
    //auto end = m_abilities.end();

    for (auto& ability : m_abilities)
    {
        if (ability.index == index)
            return &ability;
    }

    //CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "[CAbility &CAbilityOwner::GetAbility index] Get nullAbility");
    //CryLogAlways("[CAbility &CAbilityOwner::GetAbility id] Get nullAbility");
    return nullptr;
}

int CAbilityOwner::GetAbilityCount() const
{
    return static_cast<int>(m_abilities.size());
}

void CAbilityOwner::SetEntityId(const EntityId id)
{
    m_ownerId = id;
}

EntityId CAbilityOwner::GetEntityId() const
{
    return m_ownerId;
}

void CAbilityOwner::Reset()
{
    auto it = m_abilities.begin();
    const auto end = m_abilities.end();

    for (; it != end; ++it)
        it->Reset();

    m_abilities.clear();
    m_ownerId = 0;
}

void CAbilityOwner::Serialize(TSerialize ser)
{
    if (ser.GetSerializationTarget() != eST_Network)
    {
        ser.BeginGroup("CAbilityOwner");
        ser.Value("m_ownerId", m_ownerId);

        TAbilities abilities;

        if (ser.IsWriting())
        {
            abilities = m_abilities;
            ser.Value("m_abilities", abilities);
        }
        else
        {
            ser.Value("m_abilities", abilities);

            auto it = m_abilities.begin();
            const auto end = m_abilities.end();
            for (; it != end; ++it)
            {
                AddAbility(*it);
            }
        }

        auto it = m_abilities.begin();
        const auto end = m_abilities.end();
        for (; it != end; ++it)
        {
            it->Serialize(ser);
        }

        ser.EndGroup();
    }
}

void CAbilityOwner::GetMemoryStatistics(ICrySizer* s)
{
    s->Add(*this);
    s->AddContainer(m_abilities);

    auto it = m_abilities.begin();
    const auto end = m_abilities.end();

    for (; it != end; ++it)
        it->GetMemoryStatistics(s);
}


void CAbilityOwner::ToggleAbility(int index, EntityId targetId)
{
    auto pOwnerActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_ownerId));
    if (!pOwnerActor)
        return;

    if (pOwnerActor->GetHealth() <= 0)
        return;

    if (pOwnerActor->GetEntity()->IsHidden())
        return;

    auto pAbilities = g_pControlSystem->GetAbilitiesSystem();
    if (!pAbilities)
        return;

    auto pAbility = GetAbility(index);
    if (!pAbility)
        return;

    auto pOwnerTable = pOwnerActor->GetEntity()->GetScriptTable();
    if (!pOwnerTable)
        return;

    auto pTargetEntity = GET_ENTITY(targetId);
    
    string abilityName = pAbility->GetName();
    string className = pOwnerActor->GetEntity()->GetClass()->GetName();

    if (g_pControlSystem->GetAbilitiesSystem()->m_isDebugLog)
    {
        CryLogAlways("[C++][Actor %s][Activate Ability %s]",
                     pOwnerActor->GetEntity()->GetName(), GetAbility(index)->GetName().c_str());
    }

    if (className == "Trooper")
    {
        auto* pTrooper = dynamic_cast<CTrooper*>(pOwnerActor);

        if (abilityName == ABILITY_TROOPER_SHIELD)
        {
            //CAbility& shieldAbility = GetAbility(abilityName);
            if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                //Turn on a ability 
                if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian)
                {
                    pTrooper->m_shieldParams.canGuardianShieldProj = !pTrooper->m_shieldParams.canGuardianShieldProj;
                    pTrooper->m_shieldParams.canGuardianShieldProj = true;

                    //bool isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj;

                    pAbility->SetState(eAbilityState_Activated);
                    TOS_HUD::DisplayOverlayMessage("@abil_tr_shield_enabled", ColorF(0, 1, 0));
                }
                else
                {
                    TOS_HUD::DisplayOverlayMessage("@abil_you_cant_use_ability", ColorF(1, 0, 0));
                }
            }
            else if (pAbility->GetState() == eAbilityState_Activated)
            {
                //Turn off a ability
                //pSys->trooper.isProjectingShield = pTrooper->m_shieldParams.canGuardianShieldProj = false;
                pTrooper->m_shieldParams.canGuardianShieldProj = false;
                pAbility->SetState(eAbilityState_Cooldown);
                TOS_HUD::DisplayOverlayMessage("@abil_tr_shield_disabled", ColorF(0, 1, 0));
            }
        }
        if (abilityName == ABILITY_TROOPER_RAGE)
        {
            //CAbility& rageAbility = GetAbility(abilityName);
            if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                const bool isRageMode = pAbilities->trooper.isRageMode = (!pTrooper->m_rageMode.isActive && !pTrooper->
                    m_rageMode.isReloading);
                const float rageDuration = pAbilities->trooper.rageDuration;

                pTrooper->m_rageMode.ToggleMode(isRageMode, rageDuration);
                pAbility->SetState(eAbilityState_Activated);

                TOS_HUD::DisplayOverlayMessage("@abil_tr_rage_enabled", ColorF(0, 1, 0));
            }
            else if (pAbility->GetState() == eAbilityState_Activated)
            {
                pAbility->SetState(eAbilityState_Cooldown);

                TOS_HUD::DisplayOverlayMessage("@abil_tr_rage_disabled", ColorF(0, 1, 0));
            }
        }
        if (abilityName == ABILITY_TROOPER_EMP)
        {
            //CAbility& empAbility = GetAbility(abilityName);
            if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                const bool isGuardian = pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Guardian;
                const bool isLeader = pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Leader;
                const bool isElite = isGuardian || isLeader;

                float shockwaveTimer = pAbilities->trooper.shockwaveTimer;

                if (isElite && shockwaveTimer == 0.0f)
                {
                    float timerSec = 15.0f; //150

                    pAbilities->trooper.shockwaveTimer = timerSec;

                    ExplosionInfo info;
                    info.weaponId = info.shooterId = pOwnerActor->GetEntityId();
                    info.damage = 1;
                    info.pos = pOwnerActor->GetEntity()->GetWorldPos();
                    info.pos.z += 1.5f;
                    info.dir = pOwnerActor->GetEntity()->GetWorldTM().GetColumn2();
                    info.radius = 6;
                    info.angle = 0;
                    info.pressure = 0;
                    info.hole_size = 10;
                    info.type = 21; //g_pGame->GetGameRules()->GetHitTypeId("emp");
                    info.SetEffect("expansion_fx.weapons.emp_grenade", 0.8f, 0.0f);

                    g_pGame->GetGameRules()->ServerExplosion(info);

                    //if (IsHaveAbility(ABILITY_TROOPER_SHIELD))
                    //	GetAbility(ABILITY_TROOPER_SHIELD)->SetState(eAbilityState_Disabled_By_Condition);

                    auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(pOwnerActor, true);
                    if (pSquad && pSquad->GetLeader())
                    {
                        //Get the shield owners from the player squad members
                        for (auto& member : pSquad->GetAllMembers())
                        {
                            auto pMember = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(member.GetId());
                            if (pMember)
                            {
                                const auto leaderPos = pOwnerActor->GetEntity()->GetWorldPos();
                                const auto memberPos = pMember->GetEntity()->GetWorldPos();

                                if ((leaderPos - memberPos).GetLength() < 15.0f)
                                {
                                    const auto maxHealth = pMember->GetMaxHealth();
                                    const auto currentHealth = pMember->GetHealth();
                                    constexpr auto divider = 6;

                                    int addValue = maxHealth / divider;

                                    if (pTrooper->m_shieldParams.shieldType == CTrooper::eShieldType_Leader)
                                        addValue = maxHealth / 2;

                                    pMember->SetHealth(currentHealth + addValue);
                                }
                            }
                        }
                    }

                    pAbility->SetState(eAbilityState_Activated);
                }
            }
        }

        if (abilityName == ABILITY_TROOPER_CLOAK)
        {
            int cloakedLua = -1;

            if (pOwnerTable->GetValue("cloaked", cloakedLua))
            {
                int cloakValue = 0;
                if (cloakedLua == 0 && pAbility->GetState() == eAbilityState_Ready_To_Activate)
                {
                    cloakValue = 1;

                    pAbility->SetState(eAbilityState_Activated);
                }
                else if (cloakedLua == 1 && pAbility->GetState() == eAbilityState_Activated)
                {
                    cloakValue = 0;

                    pAbility->SetState(eAbilityState_Ready_To_Activate);
                }


                //const char* szSound = 0;
                //if (cloakValue == 1)
                //	szSound = "sounds/environment:sphere_sfx:hunter_shield_on";
                //else
                //	szSound = "sounds/environment:sphere_sfx:hunter_shield_off";

                //IEntitySoundProxy* pSoundProxy = (IEntitySoundProxy*)pClientActor->GetEntity()->GetProxy(ENTITY_PROXY_SOUND);
                //if (pSoundProxy)
                //	pSoundProxy->PlaySound(szSound, Vec3Constants<float>::fVec3_Zero, Vec3Constants<float>::fVec3_OneY, 0, eSoundSemantic_HUD);
                //

                Script::CallMethod(pOwnerTable, "Cloak", cloakValue);
            }
        }
    }
    else if (className == "Hunter")
    {
        auto pHunter = dynamic_cast<CHunter*>(pOwnerActor);
        if (abilityName == ABILITY_HUNTER_GRAB)
        {
            auto pCrossEntity = pTargetEntity ? pTargetEntity : g_pControlSystem->GetLocalControlClient()->GetLastCrosshairEntity();
            if (pCrossEntity)
            {
                IActor* pCrossActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pCrossEntity->GetId());
                IVehicle* pCrossVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(
                    pCrossEntity->GetId());

                if (pCrossActor || pCrossVehicle)
                {
                    IAIObject* pOwnerAI = pOwnerActor->GetEntity()->GetAI();
                    IAIObject* pCrossEntityAI = pCrossEntity->GetAI();

                    bool isActor = false;
                    bool isVehicle = false;

                    if ((pCrossActor && pCrossActor->GetHealth() > 0) && (pCrossEntityAI && pOwnerAI &&
                        pCrossEntityAI->IsHostile(pOwnerAI, false)))
                        isActor = true;
                    else if (pCrossVehicle)
                        isVehicle = true;
                    else
                        return;

                    float distance = (pOwnerActor->GetEntity()->GetWorldPos() - pCrossEntity->GetWorldPos()).
                        GetLength();
                    if (distance <= 30.0f)
                    {
                        auto pScriptTable = pOwnerActor->GetEntity()->GetScriptTable();
                        if (pScriptTable)
                        {
                            string schemeName = "";
                            if (isActor)
                                schemeName = "Mouth";
                            else if (isVehicle)
                            {
                                auto pChar = pHunter->GetEntity()->GetCharacter(0);
                                if (pChar)
                                {
                                    //Hunter not have left grab animation :(

                                    //int leftLegId = pChar->GetISkeletonPose()->GetJointIDByName("frontLegLeft12");
                                    //int rightLegId = pChar->GetISkeletonPose()->GetJointIDByName("frontLegRight12");

                                    const auto& leftLegPos = pHunter->GetBoneWorldPos("frontLegLeft12");
                                    const auto& rightLegPos = pHunter->GetBoneWorldPos("frontLegRight12");

                                    const float leftLegDist = (pCrossEntity->GetWorldPos() - leftLegPos).GetLength();
                                    const float rightLegDist = (pCrossEntity->GetWorldPos() - rightLegPos).GetLength();

                                    if (leftLegDist < rightLegDist)
                                        schemeName = "Left";
                                    else
                                        schemeName = "Right";
                                }
                            }

                            auto crosshairPos = g_pControlSystem->GetLocalControlClient()->GetCrosshairPos();

                            Script::CallMethod(pScriptTable, "AbilityHunterThrow", pCrossEntity->GetId(), crosshairPos,
                                               schemeName.c_str());
                        }
                    }
                }
            }
        }
        if (abilityName == ABILITY_HUNTER_SHIELD)
        {
            if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                if (!pHunter->GetEnergyParams().isHunterShieldEnabled)
                {
                    pAbility->SetState(eAbilityState_Activated);

                    TOS_HUD::DisplayOverlayMessage("@abil_ht_shield_enabled", ColorF(0, 1, 0));
                }

                pHunter->GetEnergyParams().isHunterShieldEnabled = true;
            }
            else if (pAbility->GetState() == eAbilityState_Activated)
            {
                pHunter->GetEnergyParams().isHunterShieldEnabled = false;
                pAbility->SetState(eAbilityState_Cooldown);

                TOS_HUD::DisplayOverlayMessage("@abil_ht_shield_disabled", ColorF(0, 1, 0));
            }

            auto* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
            if (pClientActor)
            {
                const char* szSound = nullptr;

                auto zero = Vec3Constants<float>::fVec3_Zero;
                auto oneY = Vec3Constants<float>::fVec3_OneY;

                auto pHeadShield = pHunter->GetEntity()->GetCharacter(0)->GetIAttachmentManager()->GetInterfaceByName(
                    "hunter_head_shield");

                if (pHunter->IsShieldEnabled())
                {
                    szSound = "sounds/environment:sphere_sfx:hunter_shield_on";

                    if (pHeadShield)
                        pHeadShield->HideAttachment(0);
                }
                else
                {
                    szSound = "sounds/environment:sphere_sfx:hunter_shield_off";

                    if (pHeadShield)
                        pHeadShield->HideAttachment(1);
                }

                auto pSoundProxy = dynamic_cast<IEntitySoundProxy*>(pClientActor->GetEntity()->GetProxy(
                    ENTITY_PROXY_SOUND));
                if (pSoundProxy)
                {
                    auto szIdleSound = "sounds/environment:sphere_sfx:hunter_shield_idle";
                    pSoundProxy->PlaySound(szSound, zero, oneY, FLAG_SOUND_2D | FLAG_SOUND_RELATIVE,
                                           eSoundSemantic_AI_Readability);

                    if (pHunter->IsShieldEnabled())
                        pHunter->m_shieldSoundId = pSoundProxy->PlaySound(
                            szIdleSound, zero, oneY, FLAG_SOUND_2D | FLAG_SOUND_RELATIVE | FLAG_SOUND_LOOP,
                            eSoundSemantic_AI_Readability);
                    else
                    {
                        pSoundProxy->StopSound(pHunter->m_shieldSoundId);
                        pHunter->m_shieldSoundId = 0;
                    }
                }
            }
        }
    }
    else if (className == "Scout")
    {
        auto pScout = dynamic_cast<CScout*>(pOwnerActor);
        if (abilityName == ABILITY_SCOUT_SPOTLIGHT)
        {
            auto pSearchBeam = gEnv->pEntitySystem->GetEntity(pScout->m_searchbeam.itemId);
            if (!pSearchBeam)
                pScout->InitSearchBeam();

            if (pSearchBeam)
            {
                if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
                {
                    pAbilities->scout.IsSearch = true;
                    g_pControlSystem->GetLocalControlClient()->GetGenericParams().canShoot = false;

                    pScout->EnableSearchBeam(true);
                    pAbility->SetState(eAbilityState_Activated);

                    TOS_HUD::DisplayOverlayMessage("@abil_sc_spotLight_enabled", ColorF(0, 1, 0));
                }
                else if (pAbility->GetState() == eAbilityState_Activated)
                {
                    pAbility->SetState(eAbilityState_Cooldown);
                    TOS_HUD::DisplayOverlayMessage("@abil_sc_spotLight_disabled", ColorF(0, 1, 0));
                }
            }
        }
        else if (abilityName == ABILITY_SCOUT_ANTIGRAV)
        {
            if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                pAbility->SetState(eAbilityState_Activated);
                TOS_HUD::DisplayOverlayMessage("@abil_sc_antigrav_enabled", ColorF(0, 1, 0));
            }
            else if (pAbility->GetState() == eAbilityState_Activated)
            {
                pAbility->SetState(eAbilityState_Cooldown);
                TOS_HUD::DisplayOverlayMessage("@abil_sc_antigrav_disabled", ColorF(0, 1, 0));
            }
        }
        else if (abilityName == ABILITY_SCOUT_OBJECTGRAB)
        {
            const auto pos = pOwnerActor->GetEntity()->GetWorldPos();

            //if (pAbility->GetState() == eAbilityState_Ready_To_Activate)
            {
                //pAbility->SetState(eAbilityState_Activated);

                auto pLCC = g_pControlSystem->GetLocalControlClient();

                auto pCrossEntity = pTargetEntity ? pTargetEntity : GET_ENTITY(pLCC->GetScoutAutoAimTargetId());
                if (pCrossEntity && pCrossEntity != pOwnerActor->GetEntity())
                {
                    //Handle grab here

                    IActor* pCrossActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(
                        pCrossEntity->GetId());
                    IVehicle* pCrossVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(
                        pCrossEntity->GetId());

                    bool isActor = false;
                    bool isVehicle = false;
                    bool isOther = false;

                    if (pCrossActor || pCrossVehicle)
                    {
                        if ((pCrossActor && pCrossActor->GetHealth() > 0))
                            isActor = true;
                        else if (pCrossVehicle)
                            isVehicle = true;
                    }
                    else
                    {
                        isOther = true;
                    }

                    const float dist = (pos - pCrossEntity->GetWorldPos()).GetLength();
                    if (dist <= 25.0f)
                    {
                        const auto alreadyGrabbed = stl::find(pOwnerActor->GetGrabStats()->grabbedIds,
                                                              pCrossEntity->GetId());
                        if (alreadyGrabbed)
                            return;

                        if (isVehicle)
                        {
                            //28.01.2023
                            //Akeeper: Транспорт не поддаётся физике после броска, пока с ним не столкнешься  // NOLINT(clang-diagnostic-invalid-utf8)
                            return;
                            //const auto isCar = TOS_Vehicle::IsCar(pCrossVehicle);
                            //if (!isCar)
                            //{
                            //	if (pLCC->GetControlledActor() == pOwnerActor)
                            //		g_pGame->GetHUD()->DisplayBigOverlayFlashMessage("@abil_you_cant_grab_big_vehicle", 5.0f);
                            //	return;
                            //}
                        }

                        if (isOther)
                        {
                            bool pickable = false;
                            TOS_Script::GetEntityProperty(pCrossEntity, "bPickable", pickable);

                            if (!pickable)
                            {
                                if (pLCC->GetControlledActor() == pOwnerActor)
                                {
                                    TOS_HUD::DisplayOverlayMessage("@abil_you_cant_grab_notpickable", ColorF(1, 0, 0));
                                    return;
                                }
                            }
                        }

                        const auto id = static_cast<int>(pCrossEntity->GetId());
                        if (Script::CallMethod(pOwnerTable, "GrabEntityInTentacle",ScriptHandle(id)))
                        {
                            //CryLogAlways("GrabEntityInTentacle %s", pCrossEntity->GetName());

                            if (isActor)
                            {
                                if (TOS_AI::IsCombatEnable(pCrossActor))
                                    TOS_AI::EnableCombat(pCrossActor, false, false, "Grabbed by ability");


                                auto pActor = dynamic_cast<CActor*>(pCrossActor);
                                const char* grabbedClsName = pActor->GetEntity()->GetClass()->GetName();

                                if (pActor->IsAlien())
                                {
                                    TOS_HUD::DisplayOverlayMessage("@abil_you_grab_alien", ColorF(0, 1, 0));
                                }
                                else if (strcmp(grabbedClsName, "Player") == 0 || strcmp(grabbedClsName, "Grunt") == 0)
                                {
                                    TOS_HUD::DisplayOverlayMessage("@abil_you_grab_human", ColorF(0, 1, 0));
                                }
                                else
                                {
                                    TOS_HUD::DisplayOverlayMessage("@abil_you_grab_something", ColorF(0, 1, 0));
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (!pOwnerActor->GetGrabStats()->grabbedIds.empty())
                    {
                        //Handle drop here
                        Script::CallMethod(pOwnerTable, "DropEntitiesFromTentacles");
                        TOS_HUD::DisplayOverlayMessage("@abil_you_drop_all_grabbed", ColorF(0, 1, 0));

                        for (auto id : pOwnerActor->GetGrabStats()->grabbedIds)
                        {
                            auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);

                            if (!TOS_AI::IsCombatEnable(pActor))
                                TOS_AI::EnableCombat(pActor, true, false, "UnGrabbed by ability");
                        }
                    }
                }
            }
            //else if (pAbility->GetState() == eAbilityState_Activated)
            //{
            //	pAbility->SetState(eAbilityState_Cooldown);
            //}
        }
    }
    else if (className == "Player" || className == "Grunt")
    {
        /*if (abilityName == ABILITY_HUMAN_VEHICLE_HEALING)
        {
    
        }*/
    }
}

void CAbilityOwner::DeactivateAbility(CAbility* pAbility)
{
    if (pAbility)
    {
        pAbility->SetState(eAbilityState_Cooldown);
    }
}
