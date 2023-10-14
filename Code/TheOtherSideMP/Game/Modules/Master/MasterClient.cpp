#include "StdAfx.h"

//#include "HUD/HUD.h"
//#include "HUD/HUDScopes.h"
//#include "HUD/HUDCrosshair.h"
//#include "HUD/HUDSilhouettes.h"

#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/TOSGame.h"

//#include "PlayerInput.h"

#include "MasterClient.h"

#include "GameUtils.h"
#include "IPlayerInput.h"
#include "IViewSystem.h"
#include "MasterSynchronizer.h"
#include "Single.h"

#include "HUD/HUD.h"
#include "HUD/HUDCrosshair.h"

#include "TheOtherSideMP/Actors/Aliens/TOSAlien.h"
#include "TheOtherSideMP/HUD/TOSCrosshair.h"
#include "TheOtherSideMP/Helpers/TOS_AI.h"

CTOSMasterClient::CTOSMasterClient(CTOSPlayer* pPlayer)
	: m_pLocalDude(pPlayer),
	m_pSlaveEntity(nullptr),
	m_pHUDCrosshair(nullptr)
{
    assert(pPlayer);

    m_pHUDCrosshair = dynamic_cast<CTOSHUDCrosshair*>(g_pGame->GetHUD()->GetCrosshair());
    assert(m_pHUDCrosshair);



	if (gEnv->bClient)
	{
		//const char* clientChannelName = g_pGame->GetIGameFramework()->GetClientChannel()->GetName();
		//const char* netChName = _player->GetEntity()->;

		//CryLogAlways(" ");
		//CryLogAlways("[C++][CallConstructor][CTOSMasterClient] Player: %s, ClientChName: %s",
		//	_player->GetEntity()->GetName(), clientChannelName);
		//[C++][CallConstructor][CTOSMasterClient] Player: Akeeper, ClientChName: lmlicenses.wip4.adobe.com:64100

		//g_pTOSGame->GetModuleMasterSystem()->MasterAdd(m_pLocalDude->GetEntity());

		//InvokeRMI(ClTempRadarEntity(), params, eRMI_ToClientChannel, GetChannelId(*it));

		//auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
		//assert(pSender);

		//MasterAddingParams params;
		//params.entityId = m_pLocalDude->GetEntityId();

		//pSender->RMISend(CTOSMasterRMISender::SvRequestMasterAdd(), params, eRMI_ToServer);
	}
}

CTOSMasterClient::~CTOSMasterClient()
{
	//g_pTOSGame->GetModuleMasterSystem()->MasterRemove(m_pLocalDude->GetEntity());

	// delete this;

	//Case 1 not work
	//if (gEnv->bClient)
	//{
	//	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
	//	assert(pSender);

	//	MasterAddingParams params;
	//	params.entityId = m_pLocalDude->GetEntityId();

	//	pSender->RMISend(CTOSMasterRMISender::SvRequestMasterRemove(), params, eRMI_ToServer);
	//}
}

void CTOSMasterClient::StartControl(IEntity* pEntity)
{
	assert(pEntity);

    PrepareDude(true);
    SetSlaveEntity(pEntity, pEntity->GetClass()->GetName());


	TOS_RECORD_EVENT(m_pSlaveEntity->GetId(), STOSGameEvent(eEGE_MasterClientStartControl, "", true));
}

void CTOSMasterClient::StopControl()
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_MasterClientStopControl, "", true));

	PrepareDude(false);
    ClearSlaveEntity();
}

bool CTOSMasterClient::SetSlaveEntity(IEntity* pEntity, const char* cls)
{
	assert(pEntity);
	m_pSlaveEntity = pEntity;


	TOS_RECORD_EVENT(m_pSlaveEntity->GetId(), STOSGameEvent(eEGE_MasterClientSetSlave, "", true));
	return true;
}

void CTOSMasterClient::ClearSlaveEntity()
{
	m_pSlaveEntity = nullptr;

	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_MasterClientClearSlave, "", true));
}

void CTOSMasterClient::UpdateView(SViewParams& viewParams) const
{
    assert(m_pSlaveEntity);

    viewParams.position = m_pSlaveEntity->GetWorldPos();

    static float currentFov = -1.0f;

	// Copied from ViewThirdPerson() in PlayerView.cpp
    static Vec3 target;
    static Vec3 current;

	Vec3 offsetX(0, 0, 0); //= pAlien->GetViewRotation().GetColumn0() * current.x;
	Vec3 offsetY(0, 0, 0);
    Vec3 offsetZ(0, 0, 0); //= pAlien->GetViewRotation().GetColumn2() * current.z;

    if (target)
    {
        current = target;
        Interpolate(current, target, 5.0f, viewParams.frameTime);
    }

    const EntityId controlledId = m_pSlaveEntity->GetId();
	const string   controlledCls = m_pSlaveEntity->GetClass()->GetName();

	const auto pControlledActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(controlledId));
    if (pControlledActor)
    {

    	//if (controlledCls == "Hunter")
     //   {
     //       target(g_pGameCVars->ctrl_hrTargetx, g_pGameCVars->ctrl_hrTargety, g_pGameCVars->ctrl_hrTargetz);
     //       offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1() *
     //           g_pGameCVars->ctrl_hrForwardOffset;
     //       currentFov = g_pGameCVars->ctrl_hrFov;
     //   }
     //   else if (controlledCls == "Scout")
     //   {
     //       offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
     //       target(g_pGameCVars->ctrl_scTargetx, g_pGameCVars->ctrl_scTargety, g_pGameCVars->ctrl_scTargetz);
     //       currentFov = g_pGameCVars->ctrl_scFov;
     //   }
     //   else if (controlledCls == "Drone")
     //   {
     //       target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
     //       offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
     //       currentFov = g_pGameCVars->ctrl_trFov;
     //   }
     //   else if (controlledCls == "Pinger")
     //   {
     //       target(g_pGameCVars->ctrl_pgTargetx, g_pGameCVars->ctrl_pgTargety, g_pGameCVars->ctrl_pgTargetz);
     //       offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1();
     //       currentFov = g_pGameCVars->ctrl_pgFov;
     //   }
     //   else if (controlledCls == "Alien")
     //   {
     //       target(g_pGameCVars->ctrl_alTargetx, g_pGameCVars->ctrl_alTargety, g_pGameCVars->ctrl_alTargetz);
     //       offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
     //       currentFov = g_pGameCVars->ctrl_alFov;
     //   }

        pControlledActor->UpdateMasterView(viewParams, offsetX, offsetY, offsetZ, target, current, currentFov);



        // Старт кода, который я скопипастил из ControlClient.cpp

    	//Get skip entities
	    IPhysicalEntity* pSkipEntities[10];  // NOLINT(modernize-avoid-c-arrays)
	    int nSkip = 0;
	    IItem* pItem = pControlledActor->GetCurrentItem();
	    if (pItem)
	    {
		    const auto pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
	        if (pWeapon)
	            nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
	    }

	    const float oldLen = offsetY.len();

	    // Ray cast to camera with offset position to check colliding
	    const Vec3 eyeOffsetView = pControlledActor->GetStanceInfo(pControlledActor->GetStance())->viewOffset;
	    const Vec3 start = (pControlledActor->GetBaseMtx() * eyeOffsetView + viewParams.position + offsetX);
	    // +offsetZ;// + offsetX;// +offsetZ;

	    constexpr float wallSafeDistance = 0.3f; // how far to keep camera from walls

	    primitives::sphere sphere;
	    sphere.center = start;
	    sphere.r = wallSafeDistance;

	    geom_contact* pContact = nullptr;
	    const float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(
	        sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
	        &pContact, 0, rwi_stop_at_pierceable, nullptr, nullptr, 0, pSkipEntities, nSkip);

	    if (hitDist > 0 && pContact /*&& !m_pAbilitiesSystem->trooper.isCeiling*/)
	    {
	        offsetY = pContact->pt - start;
	        if (offsetY.len() > 0.3f)
	        {
	            offsetY -= offsetY.GetNormalized() * 0.3f;
	        }
	        current.y = current.y * (hitDist / oldLen);
	    }

        viewParams.position += (offsetX + offsetY + offsetZ);

        // Конец кода, который я скопипастил из ControlClient.cpp
    }


    //const auto pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
    //if (pAlien)
    //{
    //    static float currentFov = -1;

    //    //Copied from ViewThirdPerson() in PlayerView.cpp
    //    static Vec3 target;
    //    static Vec3 current;
    //    if (target)
    //    {
    //        current = target;
    //        Interpolate(current, target, 5.0f, viewParams.frameTime);
    //    }

    //    Vec3 offsetY(0, 0, 0);

    //    if (m_pControlledActor->IsAlien())
    //    {
    //        const Matrix33 alienWorldMtx(pAlien->GetGameObject()->GetEntity()->GetWorldTM());

    //        if (GetActorClassName() == "Hunter")
    //        {
    //            target(g_pGameCVars->ctrl_hrTargetx, g_pGameCVars->ctrl_hrTargety, g_pGameCVars->ctrl_hrTargetz);
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1() *
    //                g_pGameCVars->ctrl_hrForwardOffset;
    //            currentFov = g_pGameCVars->ctrl_hrFov;
    //        }
    //        else if (GetActorClassName() == "Scout")
    //        {
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
    //            target(g_pGameCVars->ctrl_scTargetx, g_pGameCVars->ctrl_scTargety, g_pGameCVars->ctrl_scTargetz);
    //            currentFov = g_pGameCVars->ctrl_scFov;
    //        }
    //        else if (GetActorClassName() == "Trooper")
    //        {
    //            if (!m_pAbilitiesSystem->trooper.isCeiling)
    //                target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
    //            else
    //            {
    //                target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety,
    //                       g_pGameCVars->ctrl_trTargetz - 2.f);
    //            }
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
    //            currentFov = g_pGameCVars->ctrl_trFov;
    //        }
    //        else if (GetActorClassName() == "Drone")
    //        {
    //            target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
    //            currentFov = g_pGameCVars->ctrl_trFov;
    //        }
    //        else if (GetActorClassName() == "Pinger")
    //        {
    //            target(g_pGameCVars->ctrl_pgTargetx, g_pGameCVars->ctrl_pgTargety, g_pGameCVars->ctrl_pgTargetz);
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1();
    //            currentFov = g_pGameCVars->ctrl_pgFov;
    //        }
    //        else if (GetActorClassName() == "Alien")
    //        {
    //            target(g_pGameCVars->ctrl_alTargetx, g_pGameCVars->ctrl_alTargety, g_pGameCVars->ctrl_alTargetz);
    //            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
    //            currentFov = g_pGameCVars->ctrl_alFov;
    //        }
    //    }

    //    const Vec3 offsetX = pAlien->GetViewRotation().GetColumn0() * current.x;
    //    const Vec3 offsetZ = pAlien->GetViewRotation().GetColumn2() * current.z;

    //    //Get skip entities
    //    IPhysicalEntity* pSkipEntities[10];
    //    int nSkip = 0;
    //    IItem* pItem = pAlien->GetCurrentItem();
    //    if (pItem)
    //    {
    //        CWeapon* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
    //        if (pWeapon)
    //            nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
    //    }

    //    const float oldLen = offsetY.len();

    //    // Ray cast to camera with offset position to check colliding
    //    const Vec3 eyeOffsetView = pAlien->GetStanceInfo(pAlien->GetStance())->viewOffset;
    //    const Vec3 start = (pAlien->GetAlienBaseMtx() * eyeOffsetView + viewParams.position + offsetX);
    //    // +offsetZ;// + offsetX;// +offsetZ;

    //    static float wallSafeDistance = 0.3f; // how far to keep camera from walls

    //    primitives::sphere sphere;
    //    sphere.center = start;
    //    sphere.r = wallSafeDistance;

    //    geom_contact* pContact = nullptr;
    //    const float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(
    //        sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
    //        &pContact, 0, rwi_stop_at_pierceable, nullptr, nullptr, 0, pSkipEntities, nSkip);

    //    if (hitDist > 0 && pContact && !m_pAbilitiesSystem->trooper.isCeiling)
    //    {
    //        offsetY = pContact->pt - start;
    //        if (offsetY.len() > 0.3f)
    //        {
    //            offsetY -= offsetY.GetNormalized() * 0.3f;
    //        }
    //        current.y = current.y * (hitDist / oldLen);
    //    }

    //    if (m_pAbilitiesSystem->trooper.isCeiling)
    //    {
    //        Vec3 pos = pAlien->GetLocalEyePos() + pAlien->GetEntity()->GetWorldPos();
    //        pos.z += 0.3f;
    //        viewParams.position = pos;
    //    }
    //    else if (m_pAbilitiesSystem->scout.IsSearch)
    //    {
    //        if (pAlien->m_searchbeam.itemId != NULL)
    //        {
    //            const IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pAlien->m_searchbeam.itemId);
    //            if (pEntity)
    //            {
    //                Vec3 pos = pEntity->GetWorldTM().GetTranslation();
    //                pos.z -= 1.5f;

    //                viewParams.position = pos;
    //            }
    //        }
    //    }
    //    else
    //    {
    //        viewParams.position += (offsetX + offsetY + offsetZ);
    //    }

    //    if (!m_generic.isUsingBinocular)
    //    {
    //        viewParams.fov = currentFov * gf_PI / 180.0f;
    //        m_currentFov = currentFov;
    //    }
    //    else
    //    {
    //        viewParams.fov = m_currentFov * gf_PI / 180.0f;
    //    }

    //    //Old
    //    /*Vec3 pActorWorldPos = m_pAlien->GetEntity()->GetWorldPos();
    //    string m_pClassName = m_pAlien->GetEntity()->GetClass()->GetName();

    //    if (m_pClassName == "Alien")
    //    {
    //        viewParams.fov = g_pGameCVars->ctrl_alFov * gf_PI / 180.0f;
    //        Matrix33 viewMtx(m_pAlien->GetAlienViewMtx());
    //        viewParams.position = pActorWorldPos +(viewMtx.GetColumn(1) * -m_pDistance + viewMtx.GetColumn(2) * +m_pHeight);
    //    }
    //    else if (m_pClassName == "Scout")
    //    {
    //        viewParams.fov = g_pGameCVars->ctrl_scFov * gf_PI / 180.0f;
    //        Matrix33 viewMtx(m_pAlien->GetAlienViewMtx());
    //        viewParams.position = pActorWorldPos + (viewMtx.GetColumn(1) * -m_pDistance + viewMtx.GetColumn(2) * +m_pHeight);
    //    }
    //    else if (m_pClassName == "PlayerTrooper")
    //    {
    //        viewParams.fov = g_pGameCVars->ctrl_trFov * gf_PI / 180.0f;
    //        Vec3 CamViewdir = gEnv->pSystem->GetViewCamera().GetViewdir() * -m_pDistance;
    //        Matrix33 viewMtx(m_pAlien->GetGameObject()->GetEntity()->GetWorldTM());
    //        viewParams.position = pActorWorldPos + (viewMtx.GetColumn(2) * m_pHeight) + CamViewdir;
    //    }
    //    else if (m_pClassName == "Hunter")
    //    {
    //        float m_pForwardOffset = g_pGameCVars->ctrl_hrForwardOffset;
    //        viewParams.fov = g_pGameCVars->ctrl_hrFov * gf_PI / 180.0f;
    //        Vec3 CamViewdir = gEnv->pSystem->GetViewCamera().GetViewdir() * -m_pDistance;
    //        Matrix33 viewMtx(m_pAlien->GetGameObject()->GetEntity()->GetWorldTM());
    //        viewParams.position = pActorWorldPos + (viewMtx.GetColumn(2) * m_pHeight + viewMtx.GetColumn(1) * m_pForwardOffset) + CamViewdir;
    //    }*/
    //}

	viewParams.rotation = m_pLocalDude->GetViewQuatFinal();
}

void CTOSMasterClient::PrepareDude(const bool toStartControl) const
{
	assert(m_pLocalDude);

	CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();
    assert(pSuit);

	//Fix the non-resetted Dude player movement after controlling the actor;
	if (m_pLocalDude->GetPlayerInput())
		m_pLocalDude->GetPlayerInput()->Reset();

	const auto pSynch = g_pTOSGame->GetMasterModule()->GetSynchronizer();
    assert(pSynch);

	if (toStartControl)
    {
        pSynch->RMISend(
            CTOSMasterSynchronizer::SvRequestSaveMCParams(), 
            NetMasterIdParams(m_pLocalDude->GetEntityId()), 
            eRMI_ToServer);

        m_pLocalDude->ResetScreenFX();

		if (pSuit)
		{
			pSuit->SetMode(NANOMODE_DEFENSE);
			pSuit->SetModeDefect(NANOMODE_CLOAK, true);
			pSuit->SetModeDefect(NANOMODE_SPEED, true);
			pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
		}

		//g_pGameCVars->hud_enableAlienInterference = 0;
        //m_pLocalDude->ClearInterference();
        //gEnv->pConsole->GetCVar("hud_enableAlienInterference")->ForceSet("0");


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

            if (m_pHUDCrosshair)
            {
                m_pHUDCrosshair->SetOpacity(1.0f);
                m_pHUDCrosshair->SetCrosshair(g_pGameCVars->hud_crosshair);
            }
        }

        if (!gEnv->bEditor)
        {
            // fix "Pure function error" 	

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
		pSynch->RMISend(
			CTOSMasterSynchronizer::SvRequestApplyMCSavedParams(),
			NetMasterIdParams(m_pLocalDude->GetEntityId()),
			eRMI_ToServer);
        
        SActorParams* pParams = m_pLocalDude->GetActorParams();

        //m_pLocalDude->InitInterference();
		//gEnv->pConsole->GetCVar("hud_enableAlienInterference")->ForceSet("1");
        //g_pGameCVars->hud_enableAlienInterference = 1;
		//m_pLocalDude->ResetScreenFX();
		//gEnv->pSystem->GetI3DEngine()->SetPostEffectParam("AlienInterference_Amount", 0.0f);
		//SAFE_HUD_FUNC(StartInterference(0, 0, 100.0f, 3.f));

        if (m_pLocalDude->IsThirdPerson())
            m_pLocalDude->ToggleThirdPerson();

        // Выполнено - Нужно написать функцию для отображения дружественного перекрестия
		// Выполнено - Нужно написать функцию для смены имени текущего оружия

        m_pHUDCrosshair->ShowFriendCross(false);
       SAFE_HUD_FUNC(TOSSetWeaponName(""))

        //g_pControlSystem->GetSquadSystem()->AnySquadClientLeft();

        //auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(m_pLocalDude, true);
        //if (pSquad && pSquad->GetLeader() != nullptr)
        //    pSquad->OnPlayerAdded();

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
			}


            if (g_pGame->GetHUD())
            {
                //TODO: 10/11/2023, 07:35 Нужно написать функции, меняющие интерфейс жизней и инвентаря
                //SetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx");
                //SetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx");

				SAFE_HUD_FUNC(TOSSetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx"));
				SAFE_HUD_FUNC(TOSSetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx"));

                //m_animScoutFlyInterface.Unload();

                //switch (pSuit->GetMode())
                //{
                //case NANOMODE_DEFENSE:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Armor");
                //    break;
                //case NANOMODE_SPEED:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Speed");
                //    break;
                //case NANOMODE_STRENGTH:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Strength");
                //    break;
                //case NANOMODE_CLOAK:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Cloak");
                //    break;
                //case NANOMODE_INVULNERABILITY:
                //case NANOMODE_DEFENSE_HIT_REACTION:
                //case NANOMODE_LAST:
                //    break;
                //}
            }

            pParams->vLimitRangeH = 0;
            pParams->vLimitRangeV = pParams->vLimitRangeVDown = pParams->vLimitRangeVUp = 0;
        }
    }
}
