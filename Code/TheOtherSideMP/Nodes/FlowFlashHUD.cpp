/////////////////////////////////////////////////////////////////
// FlowFlashHUD.cpp
//
// Purpose: Flow nodes for drawing flash HUD objects
//
// History:
//	- 3/29/09 : File created - Kevin
//  - 4/15/09 : Additional features added - James-Ryan
/////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "Game.h"
#include "HUD/HUD.h"
//#include "ExtendedManager.h"
//#include "Common/IGameFlashAnimation.h"
//#include "Common/IGameFlashAnimationFactory.h"
#include "IFlashPlayer.h"
////////////////////////////////////////////////////
class CFlowNode_FlashHUD : public CFlowBaseNode, public IFSCommandHandler, public IGameFrameworkListener
{
public:
	////////////////////////////////////////////////////
	CFlowNode_FlashHUD(SActivationInfo* pActInfo)
	{}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_FlashHUD(void)
	{
		if (m_pFlashAnim.IsLoaded())
		{
			m_pFlashAnim.Unload();
		}

		if (g_pGame && g_pGame->GetIGameFramework())
			g_pGame->GetIGameFramework()->UnregisterListener(this);

		m_FSCommands.clear();
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_FlashHUD(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{}

	enum EInputPorts
	{
		EIP_File = 0,
		EIP_Docking,
		EIP_Visible,
		EIP_Reload,
		EIP_ReInitVars,
		EIP_Invoke0,
		EIP_Invoke1,
		EIP_Invoke2,
		EIP_Invoke3,
		EIP_SetVar,
		EIP_IsVarAvail,
		EIP_FSCommand,
		EIP_Arg1,
		EIP_Arg2,
		EIP_Arg3
	};

	enum EOutputPorts
	{
		EOP_Loaded = 0,
		EOP_Available,
		EOP_Result,
		EOP_FSCommand
	};

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<string>("File", "", _HELP("Gfx file to load, relativie to \'Libs/UI/\'"), 0, 0),
			InputPortConfig<int>("Docking", 1, _HELP("Where to dock the animation"), 0, _UICONFIG("enum_int:Stretch=0,Center=1,Left=3,Right=4")),
			InputPortConfig<bool>("Visible", true, _HELP("Make animation visible")),
			InputPortConfig<bool>("Reload", false, _HELP("Reloads the flash file")),
			InputPortConfig<bool>("ReInitVars", false, _HELP("Reinitializes the flash variables")),
			InputPortConfig<string>("Invoke0", "", _HELP("Method to invoke with 0 arguments"), 0, 0),
			InputPortConfig<string>("Invoke1", "", _HELP("Method to invoke with 1 argument"), 0, 0),
			InputPortConfig<string>("Invoke2", "", _HELP("Method to invoke with 2 arguments"), 0, 0),
			InputPortConfig<string>("Invoke3", "", _HELP("Method to invoke with 3 arguments"), 0, 0),
			InputPortConfig<string>("SetVariable", "", _HELP("Sets a variable (with Arg1 as it's value) inside the flash movie"), 0, 0),
			InputPortConfig<string>("IsVarAvailable", "", _HELP("Checks if the variable exists in the current Flash file"), 0, 0),
			InputPortConfig<string>("FSCommand", "", _HELP("Adds a FSCommand to the internal FSCommand-List"), 0, 0),
			InputPortConfig_AnyType("Arg1",_HELP("Argument 1")),
			InputPortConfig_AnyType("Arg2",_HELP("Argument 2")),
			InputPortConfig_AnyType("Arg3",_HELP("Argument 3")),
			{0}
		};
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Loaded", _HELP("True if Flash animation was loaded, False on error")),
			OutputPortConfig<bool>("Available", _HELP("True if Flash variable exists")),
			OutputPortConfig_AnyType("Result", _HELP("Flash method invoke result")),
			OutputPortConfig<string>("FSCResult", _HELP("Argument which was sent by the fscommand in flash")),
			{0}
		};
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Draws a Flash animation on the HUD");
		config.SetCategory(EFLN_ADVANCED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (gEnv->pSystem->IsDedicated())
			return;

		m_actInfo = *pActInfo;

		switch (event)
		{
			case eFE_Initialize:
			{
				if (m_pFlashAnim.IsLoaded())
				{
					m_pFlashAnim.Unload();
				}

				// Load animation
				string sFile = GetPortString(pActInfo, EIP_File);
				if (!sFile.empty())
				{
					const bool bLoaded = (m_pFlashAnim.Load((string("Libs\\UI\\") + sFile).c_str(), eFD_Center, eFAF_ManualRender));
					m_pFlashAnim.GetFlashPlayer()->SetFSCommandHandler(this);
					ActivateOutput(pActInfo, EOP_Loaded, bLoaded);

					if (bLoaded)
					{
						m_pFlashAnim.SetDock((static_cast<EFlashDock>(1 << GetPortInt(pActInfo, EIP_Docking))));
						m_pFlashAnim.SetVisible(GetPortBool(pActInfo, EIP_Visible));

						if (g_pGame->GetHUD())
							g_pGame->GetHUD()->SetModalHUD(&m_pFlashAnim);
					}
				}

				if (g_pGame && g_pGame->GetIGameFramework())
					g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_HUDFlash", FRAMEWORKLISTENERPRIORITY_HUD);

				m_FSCommands.clear();
			}
			break;

			case eFE_Activate:
			{
				if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Docking))
					m_pFlashAnim.SetDock(static_cast<EFlashDock>(1 << GetPortInt(pActInfo, EIP_Docking)));
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Reload))
					m_pFlashAnim.Reload(true);
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_ReInitVars))
					m_pFlashAnim.ReInitVariables();
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_IsVarAvail))
				{
					if (m_pFlashAnim.IsAvailable(GetPortString(pActInfo, EIP_IsVarAvail)))
						ActivateOutput(pActInfo, EOP_Available, true);
					else
						ActivateOutput(pActInfo, EOP_Available, false);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Visible))
					m_pFlashAnim.SetVisible(GetPortBool(pActInfo, EIP_Visible));
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Invoke0))
				{
					SFlashVarValue* result = NULL;

					m_pFlashAnim.Invoke(GetPortString(pActInfo, EIP_Invoke0), NULL, 0, result);

					if (result)
						SetOutputResult(pActInfo, result);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Invoke1))
				{
					SFlashVarValue arg = 0;
					SFlashVarValue* result = NULL;

					GetFlashVarWithType(pActInfo, EIP_Arg1, arg);

					m_pFlashAnim.Invoke(GetPortString(pActInfo, EIP_Invoke1), &arg, 1, result);

					if (result)
						SetOutputResult(pActInfo, result);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Invoke2))
				{
					SFlashVarValue arg1 = 0;
					SFlashVarValue arg2 = 0;

					GetFlashVarWithType(pActInfo, EIP_Arg1, arg1);
					GetFlashVarWithType(pActInfo, EIP_Arg2, arg2);

					SFlashVarValue args[2] = {arg1,arg2};
					SFlashVarValue* result = NULL;

					m_pFlashAnim.Invoke(GetPortString(pActInfo, EIP_Invoke2), args, 2, result);

					if (result)
						SetOutputResult(pActInfo, result);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_Invoke3))
				{
					SFlashVarValue arg1 = 0;
					SFlashVarValue arg2 = 0;
					SFlashVarValue arg3 = 0;

					GetFlashVarWithType(pActInfo, EIP_Arg1, arg1);
					GetFlashVarWithType(pActInfo, EIP_Arg2, arg2);
					GetFlashVarWithType(pActInfo, EIP_Arg3, arg3);

					SFlashVarValue args[3] = {arg1,arg2,arg3};
					SFlashVarValue* result = NULL;

					m_pFlashAnim.Invoke(GetPortString(pActInfo, EIP_Invoke3), args, 3, result);

					if (result)
						SetOutputResult(pActInfo, result);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_SetVar))
				{
					SFlashVarValue val = 0;
					GetFlashVarWithType(pActInfo, EIP_Arg1, val);
					m_pFlashAnim.SetVariable(GetPortString(pActInfo, EIP_SetVar), val);
				}
				else if (m_pFlashAnim.IsLoaded() && IsPortActive(pActInfo, EIP_FSCommand))
				{
					string sCommand = GetPortString(pActInfo, EIP_FSCommand);
					if (strcmp(sCommand, ""))
					{
						if (stl::push_back_unique(m_FSCommands, sCommand))
						{
							if (gEnv->pConsole->GetCVar("fgps_debug"))
								CryLogAlways("[FGPS] Added FSCommand: %s", sCommand);
						}
					}
				}
			}
			break;
		}
	}

	void CFlowNode_FlashHUD::SetOutputResult(SActivationInfo* pActInfo, SFlashVarValue* result)
	{
		SFlashVarValue::Type type = result->GetType();
		string sResult = "";

		switch (type)
		{
			case 1:
				ActivateOutput(pActInfo, EOP_Result, result->GetBool());
				break;
			case 2:
				ActivateOutput(pActInfo, EOP_Result, result->GetInt());
				break;
			case 3:
				ActivateOutput(pActInfo, EOP_Result, result->GetUInt());
				break;
			case 5:
				ActivateOutput(pActInfo, EOP_Result, result->GetFloat());
				break;
			case 6:
				sResult = result->GetConstStrPtr();
				ActivateOutput(pActInfo, EOP_Result, sResult);
				break;
			default:

				if (gEnv->pConsole->GetCVar("fgps_debug"))
					CryLogAlways("[FGPS] ERROR: Unknown data type...");

				break;
		}
	}

	void CFlowNode_FlashHUD::GetFlashVarWithType(SActivationInfo* pActInfo, int port, SFlashVarValue& val)
	{
		const TFlowInputData& anyVal = GetPortAny(pActInfo, port);
		EFlowDataTypes type = GetPortType(pActInfo, port);
		string argTypeString;

		switch (type)
		{
			case eFDT_Int:
				int argTypeInt;
				anyVal.GetValueWithConversion(argTypeInt);

				val = argTypeInt;
				break;
			case eFDT_Float:
				float argTypeFloat;
				anyVal.GetValueWithConversion(argTypeFloat);

				val = argTypeFloat;
				break;
			case eFDT_String:
				anyVal.GetValueWithConversion(argTypeString);

				val = argTypeString.c_str();
				break;
			case eFDT_Bool:
				bool argTypeBool;
				anyVal.GetValueWithConversion(argTypeBool);

				val = argTypeBool;
				break;
			default:

				if (gEnv->pConsole->GetCVar("fgps_debug"))
					CryLogAlways("[FGPS] ERROR: Unknown data type...");

				break;
		}
	}

	virtual void HandleFSCommand(const char* pCommand, const char* pArgs)
	{
		for (std::vector<string>::iterator it = m_FSCommands.begin(); it != m_FSCommands.end(); ++it)
		{
			if (!strcmp(*it, pCommand))
			{
				ActivateOutput(&m_actInfo, EOP_FSCommand, static_cast<string>(pArgs));
				break;
			}
		}
	}

	////////////////////////////////////////////////////
	// ~IGameFrameworkListener
	virtual void OnSaveGame(ISaveGame* pSaveGame)
	{}
	virtual void OnLoadGame(ILoadGame* pLoadGame)
	{}
	virtual void OnLevelEnd(const char* nextLevel)
	{}
	virtual void OnActionEvent(const SActionEvent& event)
	{}
	virtual void OnPostUpdate(float fDeltaTime)
	{
		m_pFlashAnim.GetFlashPlayer()->Advance(fDeltaTime);
		m_pFlashAnim.GetFlashPlayer()->Render();
	}

private:
	CGameFlashAnimation  m_pFlashAnim;
	SActivationInfo		 m_actInfo;
	std::vector<string>	 m_FSCommands;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("HUD:Flash", CFlowNode_FlashHUD);