#include "StdAfx.h"

//#include "HUD/HUD.h"
//#include "HUD/HUDScopes.h"
//#include "HUD/HUDCrosshair.h"
//#include "HUD/HUDSilhouettes.h"

#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/TOSGame.h"

//#include "PlayerInput.h"

#include "MasterClient.h"
#include "MasterSynchronizer.h"

CTOSMasterClient::CTOSMasterClient(CTOSPlayer* _player) :
	m_pLocalDude(_player)
{
	if (gEnv->bClient)
	{
		//const char* clientChannelName = g_pGame->GetIGameFramework()->GetClientChannel()->GetName();
		//const char* netChName = _player->GetEntity()->;

		//CryLogAlways(" ");
		//CryLogAlways("[C++][CallConstructor][CTOSMasterClient] Player: %s, ClientChName: %s",
		//	_player->GetEntity()->GetName(), clientChannelName);
		//[C++][CallConstructor][CTOSMasterClient] Player: Akeeper, ClientChName: lmlicenses.wip4.adobe.com:64100

		//TODO: Invoke RMI to Server from Client
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
	//TODO: Invoke RMI to Server from Client
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

//void CMasterClient::InitDudeMaster(bool toStart)
//{
//    if (m_pLocalDude)
//    {
//        //CPlayer::TAlienInterferenceParams lastInterferenceParams;
//
//        CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();
//
//        //Fix the non-resetted Dude player movement after controlling the actor;
//        if (m_pLocalDude->GetPlayerInput())
//            m_pLocalDude->GetPlayerInput()->Reset();
//
//        //before link to the new actor
//        if (toStart)
//        {
//            m_lastDudePosition = m_pLocalDude->GetEntity()->GetWorldPos();
//            m_lastDudeRotation = m_pLocalDude->GetViewRotation();
//
//            if (gEnv->bServer)
//            {
//                IAIObject* pAI = m_pLocalDude->GetEntity()->GetAI();
//                if (pAI)
//                {
//                    if (pAI->CastToIAIActor())
//                    {
//                        m_lastDudeSpecies = pAI->CastToIAIActor()->GetParameters().m_nSpecies;
//                        //CryLogAlways("SControlClient::PrepareDude -->> save player species");
//                    }
//
//                    pAI->Event(AIEVENT_DISABLE, nullptr);
//                }
//            }
//
//            //I dont know, it is work?
//            m_pLocalDude->ResetScreenFX();
//
//            if (pSuit)
//            {
//                m_lastDudeSuitEnergy = pSuit->GetSuitEnergy();
//                m_lastDudeNanoMode = pSuit->GetMode();
//                pSuit->SetMode(NANOMODE_DEFENSE);
//                pSuit->SetModeDefect(NANOMODE_CLOAK, true);
//                pSuit->SetModeDefect(NANOMODE_SPEED, true);
//                pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
//            }
//
//            // Turning off a Alien screen interference effects
//            //lastInterferenceParams = m_pLocalDude->m_interferenceParams;
//            m_pLocalDude->ClearInterference();
//
//            if (g_pGame->GetHUD())
//            {
//                //LoadHUD(true); deprecated
//                //m_pAbilitiesSystem->InitHUD(true);			
//                //m_pAbilitiesSystem->ShowHUD(true);
//                //m_pAbilitiesSystem->UpdateHUD();
//                //m_pAbilitiesSystem->ReloadHUD();
//
//                //SetAmmoHealthHUD();
//
//                //g_pGame->GetHUD()->UpdateHealth(m_pControlledActor);
//                //g_pGame->GetHUD()->m_animPlayerStats.Reload(true);
//
//                CHUDCrosshair* pCrosshair = g_pGame->GetHUD()->GetCrosshair();
//                if (pCrosshair)
//                {
//                    pCrosshair->SetOpacity(1.0f);
//                    pCrosshair->SetCrosshair(g_pGameCVars->hud_crosshair);
//                }
//            }
//
//            if (!gEnv->bEditor)
//            {
//                //TODO: fix "Pure function error" 	
//
//                /*CGameRules* pGR = g_pGame->GetGameRules();
//                if (pGR && !m_isHitListener)
//                {
//                    m_isHitListener = true;
//                    pGR->AddHitListener(this);
//                }*/
//            }
//        }
//        else
//        {
//            //after unlink
//
//            //The Player after unlink
//            {
//                SActorParams* pParams = m_pLocalDude->GetActorParams();
//
//                if (m_pLocalDude->GetHealth() > 0)
//                {
//                    m_pLocalDude->GetEntity()->SetPos(m_lastDudePosition);
//
//                    //may be bugged, not checked at 19.12.2020 0:14
//                    m_pLocalDude->SetViewRotation(m_lastDudeRotation);
//                }
//
//
//                m_pLocalDude->SetSlaveEntityId(0);
//                //m_pLocalDude->m_interferenceParams = lastInterferenceParams;
//                m_pLocalDude->InitInterference();
//                m_pLocalDude->ResetScreenFX();
//
//                if (m_pLocalDude->IsThirdPerson())
//                    m_pLocalDude->ToggleThirdPerson();
//
//                SAFE_HUD_FUNC(GetCrosshair()->ShowFriendCross(false))
//                SAFE_HUD_FUNC(SetWeaponName(""))
//
//               // g_pControlSystem->GetSquadSystem()->AnySquadClientLeft();
//
//                //auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(m_pLocalDude, true);
//                //if (pSquad && pSquad->GetLeader() != nullptr)
//                //    pSquad->OnPlayerAdded();
//
//                //Clean the OnUseData from player .lua script
//                IScriptTable* pTable = m_pLocalDude->GetEntity()->GetScriptTable();
//                if (pTable)
//                {
//                    const ScriptAnyValue value = 0;
//                    Script::CallMethod(pTable, "SetOnUseData", value, value);
//                }
//
//                if (pSuit)
//                {
//                    if (m_pLocalDude->GetHealth() > 0)
//                    {
//                        pSuit->Reset(m_pLocalDude);
//
//                        pSuit->SetModeDefect(NANOMODE_CLOAK, false);
//                        pSuit->SetModeDefect(NANOMODE_SPEED, false);
//                        pSuit->SetModeDefect(NANOMODE_STRENGTH, false);
//
//                        pSuit->ActivateMode(NANOMODE_CLOAK, true);
//                        pSuit->ActivateMode(NANOMODE_SPEED, true);
//                        pSuit->ActivateMode(NANOMODE_STRENGTH, true);
//
//                        pSuit->SetSuitEnergy(m_lastDudeSuitEnergy);
//                        pSuit->SetMode(m_lastDudeNanoMode);
//                    }
//
//                    if (g_pGame->GetHUD())
//                    {
//                        SetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx");
//                        SetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx");
//                        m_animScoutFlyInterface.Unload();
//
//                        switch (pSuit->GetMode())
//                        {
//                        case NANOMODE_DEFENSE:
//                            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Armor");
//                            break;
//                        case NANOMODE_SPEED:
//                            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Speed");
//                            break;
//                        case NANOMODE_STRENGTH:
//                            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Strength");
//                            break;
//                        case NANOMODE_CLOAK:
//                            g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Cloak");
//                            break;
//                        case NANOMODE_INVULNERABILITY:
//                        case NANOMODE_DEFENSE_HIT_REACTION:
//                        case NANOMODE_LAST:
//                            break;
//                        }
//                    }
//
//                    pParams->vLimitRangeH = 0;
//                    pParams->vLimitRangeV = pParams->vLimitRangeVDown = pParams->vLimitRangeVUp = 0;
//                }
//
//                if (gEnv->bServer)
//                {
//                    IAIObject* pAI = m_pLocalDude->GetEntity()->GetAI();
//                    if (pAI)
//                    {
//                        pAI->Event(AIEVENT_ENABLE, nullptr);
//                        SetDudeSpecies(m_lastDudeSpecies);
//                    }
//                }
//            }
//        }
//    }
//}

