#include "StdAfx.h"
#include "HUD/HUD.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include <TheOtherSideMP\HUD\TOSCrosshair.h>

CTOSZeusModule::CTOSZeusModule()
{
	m_zeus = nullptr;
}

CTOSZeusModule::~CTOSZeusModule()
{
}

void CTOSZeusModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{

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

}

void CTOSZeusModule::Serialize(TSerialize ser)
{

}

bool CTOSZeusModule::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

void CTOSZeusModule::NetMakePlayerZeus(IActor* pPlayer)
{
	if (!pPlayer)
		return;

	m_zeus = static_cast<CTOSPlayer*>(pPlayer);

	// Отбираем оружие
	IInventory* pInventory = m_zeus->GetInventory();
	if (pInventory)
	{
		pInventory->HolsterItem(true);
		pInventory->RemoveAllItems();
	}

	// Скрываем игрока
	if (gEnv->bClient)
		m_zeus->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
	else if (gEnv->bServer)
	{
		m_zeus->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);
		m_zeus->GetGameObject()->InvokeRMI(CTOSActor::ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients | eRMI_NoLocalCalls);	
	}

	// Режим полета со столкновениями
	m_zeus->SetFlyMode(1);

	// убираем нанокостюм
	CNanoSuit* pSuit = m_zeus->GetNanoSuit();
	if (pSuit)
	{
		pSuit->SetMode(NANOMODE_DEFENSE);
		pSuit->SetModeDefect(NANOMODE_CLOAK, true);
		pSuit->SetModeDefect(NANOMODE_SPEED, true);
		pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
	}

	// Убираем лишние действия
	g_pGameActions->FilterZeus()->Enable(true);

	// Скрываем прицел
	const auto pHUD = g_pGame->GetHUD();
	const auto pHUDCrosshair = pHUD->GetCrosshair();
	pHUDCrosshair->SetOpacity(0);

	// Скрыть HP
	// Скрыть Радар
	// Стать Неуязвимым
	// Стать Невидимым для ИИ
}
