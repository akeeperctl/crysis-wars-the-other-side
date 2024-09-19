/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

class CScriptBind_Custom
{
public:
	CScriptBind_Custom(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_Custom();

	void RegisterMethods();
	void MergeTable(IScriptTable* pDest, IScriptTable* pSrc);


	// Methods
	int HasAI(IFunctionHandler* pH, ScriptHandle entityId);
	int TOSSpawnEntity(IFunctionHandler* pH, SmartScriptTable params);

private:
	ISystem* m_pSystem;
	IGameFramework* m_pGF;
	IScriptSystem* m_pSS;
};