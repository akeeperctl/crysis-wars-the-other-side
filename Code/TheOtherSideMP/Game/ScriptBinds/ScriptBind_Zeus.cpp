/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include <StdAfx.h>
#include <ISystem.h>
#include <TheOtherSideMP/Helpers/TOS_Script.h>
#include <TheOtherSideMP/Helpers/TOS_Entity.h>
#include <TheOtherSideMP/Game/Modules/Zeus/ZeusModule.h>
#include "ScriptBind_Zeus.h"

CScriptBind_Zeus::CScriptBind_Zeus(ISystem* pSystem, IGameFramework* pGameFramework)
{
	m_pSS = pSystem->GetIScriptSystem();
	RegisterMethods();
}

CScriptBind_Zeus::~CScriptBind_Zeus()
{
}

void CScriptBind_Zeus::RegisterMethods()
{
# define REGISTER_TEMPLATE(sGlobalName, sFuncName, sFuncParams)\
	TOS_Script::RegisterTemplateFunction(sGlobalName, #sFuncName, sFuncParams, *this, &CScriptBind_Zeus::sFuncName)

	// REGISTER_TEMPLATE("System", TOSSpawnEntity, "params");
	REGISTER_TEMPLATE("Zeus", OnOrderComplete, "executorId");
}

int CScriptBind_Zeus::OnOrderComplete(IFunctionHandler* pH, ScriptHandle executorId)
{
	IEntity* pExecutor = TOS_GET_ENTITY(executorId.n);
	if (!pExecutor)
		return pH->EndFunction();

	g_pTOSGame->GetZeusModule()->RemoveOrder(pExecutor->GetId());
	return pH->EndFunction();
}