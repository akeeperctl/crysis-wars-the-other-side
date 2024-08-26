#include "StdAfx.h"
#include "HUD/HUD.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include <TheOtherSideMP\HUD\TOSCrosshair.h>
#include <TheOtherSideMP\Helpers\TOS_AI.h>
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_NET.h>
#include <TheOtherSideMP\Helpers\TOS_Inventory.h>

CTOSZeusModule::CTOSZeusModule()
	: m_zeus(nullptr),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO)
{
}

CTOSZeusModule::~CTOSZeusModule()
{
}

void CTOSZeusModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	auto pHUD = g_pGame->GetHUD();
	bool noModalOrNoHUD = !pHUD || (pHUD && !pHUD->IsHaveModalHUD());

	switch (event.event)
	{
		case eEGE_ActorRevived:
		{
			if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
			{
				m_zeus->m_isZeus = false;
				m_zeus = nullptr;

				if (noModalOrNoHUD)
					ShowMouse(false);
			}
			break;
		}
		case eEGE_MasterClientOnStartControl:
		{
			if (m_zeus)
			{
				ShowHUD(true);
				if (noModalOrNoHUD)
					ShowMouse(false);
			}
			break;
		}
		case eEGE_ActorEnterVehicle:
		{
			if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
			{
				if (noModalOrNoHUD)
					ShowMouse(false);
			}

			break;
		}
		case eEGE_ActorExitVehicle:
		{
			if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
			{				
				ApplyZeusProperties(m_zeus);
			}
			break;
		}
		case eEGE_MasterClientOnStopControl:
		{
			if (m_zeus)
			{				
				ApplyZeusProperties(m_zeus);
			}
			break;
		}
		default:
			break;
	}
}

void CTOSZeusModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

void CTOSZeusModule::Init()
{

}

void CTOSZeusModule::Update(float frametime)
{

	if (m_zeusFlags & eZF_CanRotateCamera)
	{
		auto pMouse = gEnv->pHardwareMouse;

		if (m_anchoredMousePos == Vec2(0,0))
		{
			Vec2 mousePos;
			pMouse->GetHardwareMousePosition(&mousePos.x, &mousePos.y);

			m_anchoredMousePos = mousePos;
		}

		pMouse->SetHardwareMousePosition(m_anchoredMousePos.x, m_anchoredMousePos.y);
	}
	else
	{
		if (m_anchoredMousePos != Vec2(0,0))
			m_anchoredMousePos.zero();
	}
}

void CTOSZeusModule::Serialize(TSerialize ser)
{

}

void CTOSZeusModule::NetMakePlayerZeus(IActor* pPlayer)
{
	if (!pPlayer)
		return;

	if (m_zeus)
		return;

	m_zeus = static_cast<CTOSPlayer*>(pPlayer);
	ApplyZeusProperties(m_zeus);
}

void CTOSZeusModule::ShowHUD(bool show)
{
	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		// HP
		pHUD->ShowPlayerStats(show);

		pHUD->ShowRadar(show);
		pHUD->ShowCrosshair(show);
	}
}

void CTOSZeusModule::SetZeusFlag(uint flag, bool value)
{
	m_zeusFlags = value ? (m_zeusFlags | flag) : (m_zeusFlags & ~flag);

	if (flag == eZF_CanRotateCamera)
	{
		//TODO сделать неперемещаемость курсора во время поворота камеры
	}
}

bool CTOSZeusModule::GetZeusFlag(uint flag) const
{
    return (m_zeusFlags & flag) != 0;
}

void CTOSZeusModule::ShowMouse(bool show)
{
	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		show ? pMouse->IncrementCounter() : pMouse->DecrementCounter();
		pMouse->ConfineCursor(show);
	}
}

void CTOSZeusModule::ApplyZeusProperties(IActor* pPlayer)
{
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

		// Скрываем HUD
		ShowHUD(false);
	}

	// Становимся невидимым для ИИ
	auto pAI = pTOSPlayer->GetEntity()->GetAI();
	if (pAI)
	{
		TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);
	}

	pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
	pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "CTOSZeusModule::ApplyZeusProperties");

	//Включаем мышь
	ShowMouse(true);
}
