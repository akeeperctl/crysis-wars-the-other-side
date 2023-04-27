#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDObituary.h"
#include "HUD/HUDScopes.h"
#include "HUD/HUDCrosshair.h"
#include "HUD/HUDSilhouettes.h"
#include "HUD/HUDPowerStruggle.h"

#include "stdafx.h"
#include "Cry_Camera.h"
#include "IUIDraw.h"
#include "Nodes/G2FlowBaseNode.h"
#include "Game.h"

#include <StringUtils.h>
#include <IRenderAuxGeom.h>
#include "Nodes/FGPS/FlowSystemCvarsC3.h"

// display a debug message in the HUD
class CFlowNode_DisplayDebugMessage : public CFlowBaseNode
{
	enum
	{
		INP_Show = 0,
		INP_Hide,
		INP_Message,
		INP_DisplayTime,
		INP_X,
		INP_Y,
		INP_FontSize,
		INP_Color,
		INP_Centered,
	};

	enum
	{
		OUT_Show = 0,
		OUT_Hide
	};

public:
	CFlowNode_DisplayDebugMessage(SActivationInfo* pActInfo)
		: m_isVisible(false)
		, m_isPermanent(false)
		, m_showTimeLeft(0)
	{

	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_DisplayDebugMessage(pActInfo);
	}

	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.Value("m_isVisible", m_isVisible);
		ser.Value("m_isPermanent", m_isPermanent);
		ser.Value("m_showTimeLeft", m_showTimeLeft);
	}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		// declare input ports
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_AnyType("Show",       _HELP("Show message")),
			InputPortConfig_AnyType("Hide",       _HELP("Hide message")),
			InputPortConfig<string>("message",    _HELP("Display this message on the hud")),
			InputPortConfig<float>("DisplayTime", 0.f,                                      _HELP("How much time the message will be visible. 0 = permanently visible.")),

			// this floating point input port is the x position where the message should be displayed
			InputPortConfig<float>("posX",        50.0f,                                    _HELP("Input x text position")),
			// this floating point input port is the y position where the message should be displayed
			InputPortConfig<float>("posY",        50.0f,                                    _HELP("Input y text position")),
			// this floating point input port is the font size of the message that should be displayed
			InputPortConfig<float>("fontSize",    2.0f,                                     _HELP("Input font size")),
			InputPortConfig<Vec3>("clr_Color",    Vec3(1.f,                                 1.f,                                               1.f),0, _HELP("color")),
			InputPortConfig<bool>("centered",     false,                                    _HELP("centers the text around the coordinates")),
			{ 0 }
		};
		static const SOutputPortConfig out_ports[] =
		{
			OutputPortConfig_AnyType("Show", _HELP("")),
			OutputPortConfig_AnyType("Hide", _HELP("")),
			{ 0 }
		};
		// we set pointers in "config" here to specify which input and output ports the node contains
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.SetCategory(EFLN_DEBUG);
		config.sDescription = _HELP("If an entity is not provided, the local player will be used instead");
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (!InputEntityIsLocalPlayer(pActInfo))
			return;

		switch (event)
		{
		case eFE_Initialize:
			m_isPermanent = false;
			m_isVisible = false;
			m_showTimeLeft = 0;
			break;

		case eFE_Activate:
			if (IsPortActive(pActInfo, INP_Show))
			{
				m_showTimeLeft = GetPortFloat(pActInfo, INP_DisplayTime);
				m_isPermanent = m_showTimeLeft == 0;
				if (!m_isVisible)
				{
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					m_isVisible = true;
				}
				ActivateOutput(pActInfo, OUT_Show, true);
			}

			if (IsPortActive(pActInfo, INP_Hide) && m_isVisible)
			{
				m_isVisible = false;
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				ActivateOutput(pActInfo, OUT_Hide, true);
			}

			if (IsPortActive(pActInfo, INP_DisplayTime))
			{
				m_showTimeLeft = GetPortFloat(pActInfo, INP_DisplayTime);
				m_isPermanent = m_showTimeLeft == 0;
			}
			break;

			

		case eFE_Update:
			if (!m_isPermanent)
			{
				m_showTimeLeft -= gEnv->pTimer->GetFrameTime();
				if (m_showTimeLeft <= 0)
				{
					m_isVisible = false;
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
					ActivateOutput(pActInfo, OUT_Hide, true);
				}
			}

				IRenderer* pRenderer = gEnv->pRenderer;

				IFFont *pFont = gEnv->pCryFont->GetFont("default");

				// Get correct coordinates
				float x = GetPortFloat(pActInfo, INP_X);
				float y = GetPortFloat(pActInfo, INP_Y);

				int aspectRatioFlag = 0;

				Vec3 vScreenPos;
				if ((x < 1.f || y < 1.f) && gEnv->pRenderer)
				{
					IRenderer *pRenderer = gEnv->pSystem->GetIRenderer();
					const float fWidthH = pRenderer->GetWidth()*0.5f;
					const float fHeightH = pRenderer->GetHeight()*0.5f;
					const float fScale = (300.0f / fHeightH);
					vScreenPos.x = (vScreenPos.x - fWidthH)*fScale;
					vScreenPos.y = (vScreenPos.y - fHeightH)*fScale;

				}

				IUIDraw *pDraw = NULL;
				if (g_pGame)
					if (IGameFramework *pFW = g_pGame->GetIGameFramework())
						pDraw = pFW->GetIUIDraw();
				if (NULL == pDraw) return;

				SDrawTextInfo ti;
				float scale = GetPortFloat(pActInfo, INP_FontSize);
				Vec3 color = GetPortVec3(pActInfo, INP_Color);
				pDraw->PreRender();
				//pDraw->DrawText(Vec3(x, y, 0.5f), scale, color, flags, GetPortString(pActInfo, INP_Message).c_str());
				pDraw->DrawText(pFont, vScreenPos.x, vScreenPos.y, 0, 0, GetPortString(pActInfo, INP_Message).c_str(), 0, 255, 255, 255,
					UIDRAWHORIZONTAL_CENTER, UIDRAWVERTICAL_CENTER, UIDRAWHORIZONTAL_CENTER, UIDRAWVERTICAL_CENTER);
				pDraw->PostRender();
			break;
		}
	}

	virtual void GetMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}

protected:

	bool  m_isVisible;
	bool  m_isPermanent;
	float m_showTimeLeft;

};

REGISTER_FLOW_NODE("Debug:DisplayMessage", CFlowNode_DisplayDebugMessage);