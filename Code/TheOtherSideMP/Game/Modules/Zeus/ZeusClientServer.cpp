/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "ZeusModule.h"

void CTOSZeusModule::NetMakePlayerZeus(IActor* pPlayer)
{
	if (!pPlayer)
		return;

	if (m_zeus)
		return;

	m_zeus = static_cast<CTOSPlayer*>(pPlayer);
	MakeZeus(m_zeus);
}

// ПОКА НЕ ИСПОЛЬЗУЕТСЯ
void CTOSZeusModule::NetSetZeusPP(int amount)
{
	if (!gEnv->bServer || !m_zeus)
		return;

	CGameRules* pGameRules = g_pGame->GetGameRules();
	IScriptTable* pScriptTable = pGameRules->GetEntity()->GetScriptTable();
	if (pScriptTable)
	{
		pGameRules->SetSynchedEntityValue(m_zeus->GetEntityId(), TSynchedKey(ZEUS_PP_AMOUNT_KEY), amount);
	}
}

int CTOSZeusModule::NetGetZeusPP()
{
	if (!m_zeus)
		return 0;

	int pp = 0;
	CGameRules* pGameRules = g_pGame->GetGameRules();
	IScriptTable* pScriptTable = pGameRules->GetEntity()->GetScriptTable();
	if (pScriptTable)
	{
		pGameRules->GetSynchedEntityValue(m_zeus->GetEntityId(), TSynchedKey(ZEUS_PP_AMOUNT_KEY), pp);
	}

	return pp;
}
// ~ПОКА НЕ ИСПОЛЬЗУЕТСЯ
