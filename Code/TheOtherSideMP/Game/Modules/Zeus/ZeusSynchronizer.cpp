#include "StdAfx.h"
#include "GameActions.h"
#include "ZeusSynchronizer.h"
#include "ZeusModule.h"
#include <TheOtherSideMP/Helpers/TOS_AI.h>
#include <TheOtherSideMP/Helpers/TOS_Entity.h>
#include <TheOtherSideMP/Helpers/TOS_Inventory.h>
#include <TheOtherSideMP/Helpers/TOS_NET.h>

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestMakeZeus)
{
	// Здесь пишем всё, что должно выполниться на сервере

	//TODO:
	// 2) Через меню паузы можно кликнуть по объектам...
	// 6) Спавн транспорта вызывает Malformed Packet 0_o, при этом оружие спавнится нормально

	if (gEnv->bServer)
	{
		CryLog("[C++][%s][%s][SvRequestMakeZeus]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		auto pZeusModule = g_pTOSGame->GetZeusModule();
		auto pTOSPlayer = static_cast<CTOSPlayer*>(TOS_GET_ACTOR_CHANNELID(params.playerChannelId));
		assert(pZeusModule != nullptr);
		assert(pTOSPlayer != nullptr);

		// Сбрасываем статы
		pTOSPlayer->GetActorStats()->inAir = 0.0f;
		pTOSPlayer->GetActorStats()->onGround = 0.0f;

		// Становимся неуязвимым к урону
		pTOSPlayer->SetMeZeus(true);
		pTOSPlayer->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
		pTOSPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

		// Откл. ИИ для перса зевса
		auto pAI = pTOSPlayer->GetEntity()->GetAI();
		if (pAI)
			TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);

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

		if (pTOSPlayer->GetAnimatedCharacter())
		{
			pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
			pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(
				eColliderMode_Spectator, 
				eColliderModeLayer_Game, 
				"CTOSZeusModule::MakeZeus");
		}

		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::ClClearInventory(), CActor::NoParams(), eRMI_ToAllClients);
		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients);

		auto pSync = static_cast<CTOSZeusSynchronizer*>(pZeusModule->GetSynchronizer());
		assert(pSync != nullptr);
		pSync->RMISend(CTOSZeusSynchronizer::ClMakeZeus(), params, eRMI_ToClientChannel, params.playerChannelId);

		TOS_Inventory::GiveItem(pTOSPlayer, "NightVision", false, false, false);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, ClMakeZeus)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		CryLog("[C++][%s][%s][ClMakeZeus]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		auto pZeusModule = g_pTOSGame->GetZeusModule();
		auto pTOSPlayer = pZeusModule->GetPlayer();
		//auto pTOSPlayer = static_cast<CTOSPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());

		assert(pZeusModule != nullptr);
		assert(pTOSPlayer != nullptr);

		// Убираем лишние действия
		g_pGameActions->FilterZeus()->Enable(true);

		// Скрываем HUD игрока
		pZeusModule->GetHUD().ShowPlayerHUD(false);
		pZeusModule->GetHUD().ShowZeusMenu(true);

		//Включаем мышь
		pZeusModule->GetLocal().ShowMouse(true);
		pZeusModule->GetLocal().SetFlag(CTOSZeusModule::EFlag::CanUseMouse, true);

		//Включаем режим зевса
		pZeusModule->GetLocal().SetFlag(CTOSZeusModule::EFlag::Zeusing, true);
		// pZeusModule->SetPlayer(pTOSPlayer);

		pTOSPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

		// Режим полета со столкновениями
		pTOSPlayer->SetFlyMode(1);
		pTOSPlayer->SetMeZeus(true);

		// убираем нанокостюм
		CNanoSuit* pSuit = pTOSPlayer->GetNanoSuit();
		if (pSuit)
		{
			pSuit->SetMode(NANOMODE_DEFENSE);
			pSuit->SetModeDefect(NANOMODE_CLOAK, true);
			pSuit->SetModeDefect(NANOMODE_SPEED, true);
			pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
		}

		if (pTOSPlayer->GetAnimatedCharacter())
		{
			pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
			pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(
				eColliderMode_Spectator, 
				eColliderModeLayer_Game, 
				"CTOSZeusModule::MakeZeus");
		}
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestSpawnEntity)
{
	if (gEnv->bServer)
	{
		CryLog("[C++][%s][%s][SvRequestSpawnEntity]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		auto pZeusModule = g_pTOSGame->GetZeusModule();
		assert(pZeusModule != nullptr);

		STOSEntityDelaySpawnParams spawnParams;
		//spawnParams.pCallback = pZeusModule->GetNetwork().ServerEntitySpawned;
		spawnParams.pCallback = std::bind(
			&CTOSZeusModule::Network::ServerEntitySpawned, 
			&pZeusModule->GetNetwork(), 
			std::placeholders::_1, 
			std::placeholders::_2,
			std::placeholders::_3);

		spawnParams.spawnDelay = 1.0f;
		spawnParams.vanilla.bStaticEntityId = false; // true - вылетает в редакторе и медленно работает O(n), false O(1)
		spawnParams.vanilla.bIgnoreLock = false; // spawn lock игнор

		//auto pPlayer = TOS_GET_ACTOR_CHANNELID(params.playerChannelId);
		//if (pPlayer)
			//spawnParams.authorityPlayerName = pPlayer->GetEntity()->GetName();

		const string* const psClassName = &params.className;
		IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(psClassName->c_str());

		const string name = string("zeus_") + psClassName->c_str();
		spawnParams.vanilla.sName = name;
		spawnParams.vanilla.pClass = pClass;
		spawnParams.vanilla.vPosition = params.pos;

		const auto pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(psClassName->c_str());
		if (pArchetype)
			spawnParams.vanilla.pArchetype = pArchetype;

		if (!pClass && !pArchetype)
		{
			CryLogError("[Zeus] not defined entity class '%s'", psClassName->c_str());
			return true;
		}

		bool bSpawned = TOS_Entity::SpawnDelay(spawnParams, false);
		// IEntity* pSpawned = gEnv->pEntitySystem->SpawnEntity(spawnParams.vanilla, false);
		if (!bSpawned)
		{
			CryLogError("[Zeus] entity with class '%s' spawn failed!", psClassName->c_str());
			return true;
		}

		//pSpawned->Hide(true);
		// TODO: 01/12/2024 malformed packet при спавне техники какого хуя 
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, ClSpawnEntity)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	if (gEnv->bClient)
	{
		CryLog("[C++][%s][%s][ClSpawnEntity]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		auto pZeusModule = g_pTOSGame->GetZeusModule();
		assert(pZeusModule != nullptr);

		pZeusModule->GetLocal().m_dragging = true;
		pZeusModule->GetHUD().m_menuSpawnHandling = true;

		pZeusModule->GetLocal().SelectEntity(params.spawnedId);
		pZeusModule->GetLocal().ClickEntity(params.spawnedId, params.spawnedPos);
	}

	return true;
}
