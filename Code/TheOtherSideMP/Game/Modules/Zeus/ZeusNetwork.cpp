/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include <TheOtherSideMP/Helpers/TOS_AI.h>
#include <TheOtherSideMP/Helpers/TOS_Inventory.h>
#include <TheOtherSideMP/Helpers/TOS_NET.h>

// ПОКА НЕ ИСПОЛЬЗУЕТСЯ
void CTOSZeusModule::Network::SetPP(int amount)
{
	if (!gEnv->bServer || !pParent->GetPlayer())
		return;

	CGameRules* pGameRules = g_pGame->GetGameRules();
	IScriptTable* pScriptTable = pGameRules->GetEntity()->GetScriptTable();
	if (pScriptTable)
	{
		pGameRules->SetSynchedEntityValue(pParent->GetPlayer()->GetEntityId(), TSynchedKey(ZEUS_PP_AMOUNT_KEY), amount);
	}
}

int CTOSZeusModule::Network::GetPP()
{
	if (!pParent->GetPlayer())
		return 0;

	int pp = 0;
	CGameRules* pGameRules = g_pGame->GetGameRules();
	IScriptTable* pScriptTable = pGameRules->GetEntity()->GetScriptTable();
	if (pScriptTable)
		pGameRules->GetSynchedEntityValue(pParent->GetPlayer()->GetEntityId(), TSynchedKey(ZEUS_PP_AMOUNT_KEY), pp);

	return pp;
}
// ~ПОКА НЕ ИСПОЛЬЗУЕТСЯ

void CTOSZeusModule::Network::MakeZeus(IActor* pPlayer, bool bMake)
{
	//TODO: Синхронизвать становление зевсом в сетевой игре
	// 1) Не появляется меню спавна сущностей
	// 2) Камера крутится вслед за мышкой всегда
	// 3) 

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

		// Скрываем HUD игрока
		pParent->GetHUD().ShowPlayerHUD(false);
		pParent->GetHUD().ShowZeusMenu(true);

		//Включаем мышь
		pParent->GetLocal().ShowMouse(true);
		pParent->GetLocal().SetFlag(EFlag::CanUseMouse, true);
		pParent->GetLocal().SetPlayer(pTOSPlayer);
	}

	// Становимся невидимым для ИИ
	auto pAI = pTOSPlayer->GetEntity()->GetAI();
	if (pAI)
	{
		TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);
	}

	pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
	pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "CTOSZeusModule::MakeZeus");
}
