/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
#include <IGame.h>

class CScriptBind_Zeus
{
public:
	CScriptBind_Zeus(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_Zeus();

	void RegisterMethods();


	// Methods
	int OnOrderComplete(IFunctionHandler* pH, ScriptHandle executorId);

private:
	ISystem* m_pSystem;
	IGameFramework* m_pGF;
	IScriptSystem* m_pSS;
};