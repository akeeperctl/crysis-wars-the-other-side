#include "StdAfx.h"
#include "GameActions.h"
#include "ZeusSynchronizer.h"
#include "ZeusModule.h"
#include <TheOtherSideMP/Helpers/TOS_AI.h>
#include <TheOtherSideMP/Helpers/TOS_Entity.h>
#include <TheOtherSideMP/Helpers/TOS_Inventory.h>
#include <TheOtherSideMP/Helpers/TOS_NET.h>
#include <TheOtherSideMP/Helpers/TOS_Vehicle.h>

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestMakeZeus)
{
	// Здесь пишем всё, что должно выполниться на сервере

	//TODO:
	// 2) Через меню паузы можно кликнуть по объектам...

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

		RMISend(CTOSZeusSynchronizer::ClMakeZeus(), params, eRMI_ToClientChannel, params.playerChannelId);

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
		spawnParams.pCallback = std::bind(
			&CTOSZeusModule::Network::ServerEntitySpawned, 
			&pZeusModule->GetNetwork(), 
			std::placeholders::_1, 
			std::placeholders::_2,
			std::placeholders::_3);

		spawnParams.clientChannelId = params.playerChannelId;
		spawnParams.hide = true;
		spawnParams.spawnDelay = 1.0f;
		spawnParams.saveParams = false;
		spawnParams.vanilla.bStaticEntityId = false; // true - вылетает в редакторе и медленно работает O(n), false O(1)
		spawnParams.vanilla.bIgnoreLock = false; // spawn lock игнор

		auto pPlayer = TOS_GET_ACTOR_CHANNELID(params.playerChannelId);
		if (pPlayer)
			spawnParams.authorityPlayerName = pPlayer->GetEntity()->GetName();

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

		bool bSpawned = TOS_Entity::SpawnDelay(spawnParams, true);
		if (!bSpawned)
		{
			CryLogError("[Zeus] entity with class '%s' spawn failed!", psClassName->c_str());
			return true;
		}
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, ClSpawnEntity)
{
	// Здесь пишем всё, что должно выполниться на клиенте

	CryLog("[C++][%s][%s][ClSpawnEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pZeusModule = g_pTOSGame->GetZeusModule();
	assert(pZeusModule != nullptr);

	pZeusModule->GetLocal().m_dragging = true;
	pZeusModule->GetHUD().m_menuSpawnHandling = true;

	//TODO: не выделяется сущность после спавна
	pZeusModule->GetLocal().SelectEntity(params.spawnedId);
	pZeusModule->GetLocal().ClickEntity(params.spawnedId, params.spawnedPos);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestTransformEntity)
{
	CryLog("[C++][%s][%s][SvRequestTransformEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (!pEntity)
		return true;

	auto pPhys = pEntity->GetPhysics();
	if (pPhys)
	{
		pe_action_awake awake;
		awake.bAwake = 1;
		pPhys->Action(&awake);
	}

	Vec3 pos = params.pos;
	Quat rot = Quat::CreateRotationVDir(params.dir);
	pEntity->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), rot, pos));
	pEntity->SetRotation(rot);

	RMISend(ClTransformEntity(), params, eRMI_ToAllClients | eRMI_NoLocalCalls);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, ClTransformEntity)
{
	CryLog("[C++][%s][%s][SvRequestTransformEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (!pEntity)
		return true;

	auto pPhys = pEntity->GetPhysics();
	if (pPhys)
	{
		pe_action_awake awake;
		awake.bAwake = 1;
		pPhys->Action(&awake);
	}

	Vec3 pos = params.pos;
	Quat rot = Quat::CreateRotationVDir(params.dir);
	pEntity->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), rot, pos));
	pEntity->SetRotation(rot);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestRemoveEntity)
{
	CryLog("[C++][%s][%s][SvRequestRemoveEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pActor = TOS_GET_ACTOR(params.id);
	if (pActor)
	{
		auto pActorVeh = pActor->GetLinkedVehicle();
		if (pActorVeh)
		{
			TOS_Vehicle::Exit(pActor, false, true);
		}
	}

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (pEntity)
	{
		pEntity->Hide(true);
		pEntity->Activate(false);
	}

	TOS_Entity::RemoveEntityDelayed(params.id, 2);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestKillEntity)
{
	CryLog("[C++][%s][%s][SvRequestKillEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	string hitType = "event";

	HitInfo info;
	info.SetDamage(99999.0f);
	info.shooterId = params.shooterId;
	info.targetId = params.targetId;
	info.type = g_pGame->GetGameRules()->GetHitTypeId(hitType.c_str());

	g_pGame->GetGameRules()->ServerHit(info);

	auto pVehicle = TOS_GET_VEHICLE(params.targetId);
	if (pVehicle)
		TOS_Vehicle::Destroy(pVehicle);


	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestExecuteOrder)
{
	CryLog("[C++][%s][%s][SvRequestExecuteOrder]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	IScriptSystem* pSS = gEnv->pScriptSystem;
	if (pSS->ExecuteFile("Scripts/AI/TOS/TOSHandleOrder.lua", true, true))
	{
		SmartScriptTable executorInfo;
		SmartScriptTable orderInfo;

		executorInfo.Create(gEnv->pScriptSystem);
		orderInfo.Create(gEnv->pScriptSystem);

		CScriptSetGetChain executorChain(executorInfo);
		CScriptSetGetChain orderChain(orderInfo);

		executorChain.SetValue("entityId", params.id);
		executorChain.SetValue("maxCount", params.maxCount); // макс. кол-во исполнителей
		executorChain.SetValue("index", params.index); // текущий номер исполнителя
		
		orderChain.SetValue("goalPipeId", params.id); // так надо
		orderChain.SetValue("pos", params.pos);
		orderChain.SetValue("targetId", params.targetId);

		pSS->BeginCall("HandleOrder");
		pSS->PushFuncParam(executorInfo);
		pSS->PushFuncParam(orderInfo);
		pSS->EndCall();
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestHideEntity)
{
	CryLog("[C++][%s][%s][SvRequestHideEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (pEntity)
		pEntity->Hide(params.bHide);

	RMISend(ClHideEntity(), params, eRMI_ToAllClients | eRMI_NoLocalCalls);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, ClHideEntity)
{
	CryLog("[C++][%s][%s][ClHideEntity]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (pEntity)
		pEntity->Hide(params.bHide);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestAIMakeHostile)
{
	CryLog("[C++][%s][%s][SvRequestAIMakeHostile]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	auto pEntity = TOS_GET_ENTITY(params.id);
	if (pEntity)
	{
		bool hostile = params.bHostile;
		TOS_Script::GetEntityProperty(pEntity, "bSpeciesHostility", hostile);
		TOS_AI::MakeHostile(pEntity->GetAI(), hostile);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSZeusSynchronizer, SvRequestCopyEntity)
{
	if (gEnv->bServer)
	{
		CryLog("[C++][%s][%s][SvRequestCopyEntity]",
			TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

		auto pZeusModule = g_pTOSGame->GetZeusModule();
		assert(pZeusModule != nullptr);

		STOSEntityDelaySpawnParams spawnParams;
		spawnParams.pCallback = std::bind(
			&CTOSZeusModule::Network::ServerEntityCopied,
			&pZeusModule->GetNetwork(),
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);

		spawnParams.clientChannelId = params.playerChannelId;
		spawnParams.hide = true;
		spawnParams.spawnDelay = 1.0f;
		spawnParams.saveParams = false;
		spawnParams.vanilla.bStaticEntityId = false; // true - вылетает в редакторе и медленно работает O(n), false O(1)
		spawnParams.vanilla.bIgnoreLock = false; // spawn lock игнор

		auto pPlayer = TOS_GET_ACTOR_CHANNELID(params.playerChannelId);
		if (pPlayer)
			spawnParams.authorityPlayerName = pPlayer->GetEntity()->GetName();

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

		bool bSpawned = TOS_Entity::SpawnDelay(spawnParams, true);
		if (!bSpawned)
		{
			CryLogError("[Zeus] entity with class '%s' copy failed!", psClassName->c_str());
			return true;
		}
	}

	return true;
}