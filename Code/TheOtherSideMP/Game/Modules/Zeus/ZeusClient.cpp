/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "ZeusModule.h"
#include "GameActions.h"

//void CTOSZeusModule::OnAction(const ActionId& action, int activationMode, float value)
//{
//	const CGameActions& rGA = g_pGame->Actions();
//	float pressedDuration = 0.0f;
//
//	if (!m_zeus)
//		return;
//
//	ASSING_ACTION(m_zeus, action, activationMode, pressedDuration, rGA.attack2, OnActionAttack2);
//	// ASSING_ACTION(m_zeus, action, activationMode, pressedDuration, rGA.special, OnActionSpecial);// it is melee
//}
//
//bool CTOSZeusModule::OnActionAttack2(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur)
//{
//	auto pZM = g_pTOSGame->GetZeusModule();
//	if (!pZM)
//		return false;
//
//	if (pActor->IsZeus())
//	{
//		pZM->SetZeusFlag(eZF_CanRotateCamera, activationMode == eAAM_OnPress);
//	}
//
//	return true;
//}
