/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include "ZeusSynchronizer.h"
#include <TheOtherSideMP/Helpers/TOS_AI.h>
#include <TheOtherSideMP/Helpers/TOS_Inventory.h>
#include <TheOtherSideMP/Helpers/TOS_NET.h>
#include <TheOtherSideMP/Helpers/TOS_Entity.h>

// ПОКА НЕ ИСПОЛЬЗУЕТСЯ
void CTOSZeusModule::Network::SetPP(int amount)
{
	if (!gEnv->bServer)
		return;

	CGameRules* pGameRules = g_pGame->GetGameRules();
	IScriptTable* pScriptTable = pGameRules->GetEntity()->GetScriptTable();
	if (pScriptTable)
	{
		//FIXME: НУЖНО СИНХРОНИТЬ И НЕ ИСПОЛЬЗОВТАЬ ЛОК. АКТЕРА
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

void CTOSZeusModule::Network::ServerEntitySpawned(EntityId id, const Vec3& pos, int clientChannelId)
{
	auto pSpawned = TOS_GET_ENTITY(id);
	assert(pSpawned != nullptr);

	char buffer[64];
	sprintf(buffer, "%d", id);
	pSpawned->SetName(string(pSpawned->GetName()) + "_" + buffer);

	// Извещаем клиента, о том, что он может перемещать заспавненную сущность
	auto pSync = static_cast<CTOSZeusSynchronizer*>(pParent->GetSynchronizer());
	assert(pSync != nullptr);

	CTOSZeusSynchronizer::NetSpawnedInfo info;
	info.spawnedId = id;
	info.spawnedPos = pos;
	pSync->RMISend(CTOSZeusSynchronizer::ClSpawnEntity(), info, eRMI_ToClientChannel, clientChannelId);
}

void CTOSZeusModule::Network::MakeZeus(IActor* pPlayer, bool bMake)
{
	auto pSync = static_cast<CTOSZeusSynchronizer*>(pParent->GetSynchronizer());

	CTOSZeusSynchronizer::NetMakeParams params;
	params.bMake = bMake;
	params.playerChannelId = pPlayer->GetChannelId();

	if (gEnv->bClient)
		pSync->GetGameObject()->InvokeRMI(CTOSZeusSynchronizer::SvRequestMakeZeus(), params, eRMI_ToServer);

	//// Сбрасываем статы
	//pTOSPlayer->GetActorStats()->inAir = 0.0f;
	//pTOSPlayer->GetActorStats()->onGround = 0.0f;

	//// Отбираем оружие
	//IInventory* pInventory = pTOSPlayer->GetInventory();
	//assert(pInventory != nullptr);
	//pInventory->HolsterItem(true);
	//pInventory->RemoveAllItems();

	//// Скрываем игрока
	//if (gEnv->bClient && pTOSPlayer->IsClient())
	//{
	//	pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);

	//	// Убираем лишние действия
	//	g_pGameActions->FilterZeus()->Enable(true);

	//	// Скрываем HUD игрока
	//	pParent->GetHUD().ShowPlayerHUD(false);
	//	pParent->GetHUD().ShowZeusMenu(true);

	//	//Включаем мышь
	//	pParent->GetLocal().ShowMouse(true);
	//	pParent->GetLocal().SetFlag(EFlag::CanUseMouse, true);
	//	pParent->GetLocal().SetPlayer(pTOSPlayer);
	//}
	//
	//if (gEnv->bServer)
	//{
	//	pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients | eRMI_NoLocalCalls);
	//	TOS_Inventory::GiveItem(pTOSPlayer, "NightVision", false, false, false);

	//	// Становимся неуязвимым к урону
	//	pTOSPlayer->m_isZeus = true;
	//	pTOSPlayer->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);

	//	// Откл. ИИ для перса зевса
	//	auto pAI = pTOSPlayer->GetEntity()->GetAI();
	//	if (pAI)
	//		TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);
	//}

	//pTOSPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

	//// Режим полета со столкновениями
	//pTOSPlayer->SetFlyMode(1);

	//// убираем нанокостюм
	//CNanoSuit* pSuit = pTOSPlayer->GetNanoSuit();
	//if (pSuit)
	//{
	//	pSuit->SetMode(NANOMODE_DEFENSE);
	//	pSuit->SetModeDefect(NANOMODE_CLOAK, true);
	//	pSuit->SetModeDefect(NANOMODE_SPEED, true);
	//	pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
	//}

	//if (pTOSPlayer->GetAnimatedCharacter())
	//{
	//	pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
	//	pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "CTOSZeusModule::MakeZeus");
	//}
}
