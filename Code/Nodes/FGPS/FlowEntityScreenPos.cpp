/////////////////////////////////////////////////////////////////
// FlowEntityScreenPos.cpp
//
// Purpose: Flow nodes for calculating screen position of entity
//
// History:
//	- 3/25/08 : File created - Kevin
/////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Cry_Camera.h"
#include "IUIDraw.h"
#include "Nodes/G2FlowBaseNode.h"
#include "Game.h"

// Positions
enum ePositions
{
	POSITION_MIDDLE = 0,
	POSITION_TOP,
	POSITION_BOTTOM,
};

////////////////////////////////////////////////////
// GetEntityScreenPos
//
// Purpose: Gets the position of an entity on the screen
//
// In:	pEntity - Entity object
//		nPos - Position (see ePositions)
//
// Out:	vScreenPos - Screen position
//
// Returns TRUE if entity is on the screen
////////////////////////////////////////////////////
bool GetEntityScreenPos(IEntity* pEntity, int nPos, Vec3& vScreenPos)
{
	vScreenPos.Set(0, 0, 0);
	if (NULL == pEntity) return false;

	// Calculate center position
	AABB aabb;
	pEntity->GetWorldBounds(aabb);
	Vec3 vHalf = (aabb.max - aabb.min) * 0.5f;
	Vec3 vPos(aabb.min.x + vHalf.x, aabb.min.y + vHalf.y, aabb.min.z);
	switch (nPos)
	{
	case POSITION_MIDDLE:
		vPos.z += vHalf.z;
		break;
	case POSITION_TOP:
		vPos.z = aabb.max.z;
		break;
		// POSITION_BOTTOM is default
	};

	// Check if object is visible
	IRenderer* pRenderer = gEnv->pSystem->GetIRenderer();
	CCamera camera = pRenderer->GetCamera();
	bool bAllIn = false;
	bool bVisible = (CULL_EXCLUSION != camera.IsAABBVisible_FH(aabb, &bAllIn));
	if (true == bVisible)
	{
		const float fWidth = pRenderer->GetWidth();
		const float fHeight = pRenderer->GetHeight();

		// Convert to screen
		gEnv->pSystem->GetIRenderer()->ProjectToScreen(vPos.x, vPos.y, vPos.z, &vScreenPos.x, &vScreenPos.y, &vScreenPos.z);
		vScreenPos.x *= 0.01f * fWidth;
		vScreenPos.y *= 0.01f * fHeight;
	}
	return (bVisible);
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
class CFlowNode_EntityScreenPos : public CFlowBaseNode, public IGameFrameworkListener
{
private:
	bool m_bVisible, m_bEnabled;
	Vec3 m_vLastPos;
	SActivationInfo m_actInfo;

public:
	////////////////////////////////////////////////////
	CFlowNode_EntityScreenPos(SActivationInfo* pActInfo)
	{
		m_bVisible = false;
		m_bEnabled = true;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EntityScreenPos(void)
	{
		if (g_pGame && g_pGame->GetIGameFramework())
			g_pGame->GetIGameFramework()->UnregisterListener(this);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_EntityScreenPos(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.Value("m_bEnabled", m_bEnabled);
		ser.Value("m_bVisible", m_bVisible);
		ser.Value("m_vLastPos", m_vLastPos);

		if (ser.IsReading())
		{
			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework() && m_bEnabled)
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
	}

	enum EInputPorts
	{
		EIP_Area = 0,
		EIP_Enable,
		EIP_Disable,
	};

	enum EOutputPorts
	{
		EOP_Pos = 0,
		EOP_Visible,
	};

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<int>("Area", 0, _HELP("Position area"), 0, _HELP("enum_int:Middle=0,Top=1,Bottom=2")),
			InputPortConfig_Void("Enable", _HELP("Enable input checking")),
			InputPortConfig_Void("Disable", _HELP("Disable input checking")),
			{0}
		};
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<Vec3>("Pos", _HELP("Screen position of entity")),
			OutputPortConfig<bool>("Visible", _HELP("Returns if entity is visible on the screen")),
			{0}
		};
		config.nFlags = EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Returns screen position of entity as it changes");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			// Reset pos
			m_vLastPos.Set(0, 0, 0);
			m_bVisible = false;
			m_bEnabled = true;

			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework())
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
		break;

		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Enable) && false == m_bEnabled)
			{
				m_bEnabled = true;
				if (g_pGame && g_pGame->GetIGameFramework())
					g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
			}
			if (IsPortActive(pActInfo, EIP_Disable) && true == m_bEnabled)
			{
				m_bEnabled = false;
				if (g_pGame && g_pGame->GetIGameFramework())
					g_pGame->GetIGameFramework()->UnregisterListener(this);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	// ~IGameFrameworkListener
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(const char* nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	virtual void OnPostUpdate(float fDeltaTime)
	{
		if (false == m_bEnabled) return;

		// Get screen pos
		Vec3 vScreenPos;
		bool bVisible = GetEntityScreenPos(m_actInfo.pEntity, GetPortInt(&m_actInfo, EIP_Area), vScreenPos);
		if (bVisible != m_bVisible || vScreenPos != m_vLastPos)
		{
			m_vLastPos = vScreenPos;
			m_bVisible = bVisible;

			// Send it out
			ActivateOutput(&m_actInfo, EOP_Pos, vScreenPos);
			ActivateOutput(&m_actInfo, EOP_Visible, m_bVisible);
		}
	}
};

////////////////////////////////////////////////////
class CFlowNode_GetEntityScreenPos : public CFlowBaseNode, public IGameFrameworkListener
{
private:
	bool m_bRequested;
	SActivationInfo m_actInfo;

public:
	////////////////////////////////////////////////////
	CFlowNode_GetEntityScreenPos(SActivationInfo* pActInfo)
	{
		m_bRequested = false;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_GetEntityScreenPos(void)
	{
		if (g_pGame && g_pGame->GetIGameFramework())
			g_pGame->GetIGameFramework()->UnregisterListener(this);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetEntityScreenPos(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.Value("m_bRequested", m_bRequested);

		if (ser.IsReading())
		{
			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework())
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
	}

	enum EInputPorts
	{
		EIP_Area = 0,
		EIP_Get,
	};

	enum EOutputPorts
	{
		EOP_Pos = 0,
		EOP_Visible,
	};

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<int>("Area", 0, _HELP("Position area"), 0, _HELP("enum_int:Middle=0,Top=1,Bottom=2")),
			InputPortConfig_Void("Get", _HELP("Get entity's screen position")),
			{0}
		};
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<Vec3>("Pos", _HELP("Screen position of entity")),
			OutputPortConfig<bool>("Visible", _HELP("Returns if entity is visible on the screen")),
			{0}
		};
		config.nFlags = EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Gets the screen position of entity");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			m_bRequested = false;
			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework())
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
		break;

		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Get))
				m_bRequested = true;
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	// ~IGameFrameworkListener
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(const char* nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	virtual void OnPostUpdate(float fDeltaTime)
	{
		if (true == m_bRequested)
		{
			m_bRequested = false;

			// Get screen pos
			Vec3 vScreenPos;
			bool bVisible = GetEntityScreenPos(m_actInfo.pEntity, GetPortInt(&m_actInfo, EIP_Area), vScreenPos);

			ActivateOutput(&m_actInfo, EOP_Pos, vScreenPos);
			ActivateOutput(&m_actInfo, EOP_Visible, bVisible);
		}
	}
};

////////////////////////////////////////////////////
class CFlowNode_EntityScreenText : public CFlowBaseNode, public IGameFrameworkListener
{
private:
	bool m_bEnabled;
	SActivationInfo m_actInfo;

public:
	////////////////////////////////////////////////////
	CFlowNode_EntityScreenText(SActivationInfo* pActInfo)
	{
		m_bEnabled = true;
	}

	////////////////////////////////////////////////////
	virtual ~CFlowNode_EntityScreenText(void)
	{
		if (g_pGame && g_pGame->GetIGameFramework())
			g_pGame->GetIGameFramework()->UnregisterListener(this);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_EntityScreenText(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.Value("m_bEnabled", m_bEnabled);

		if (ser.IsReading())
		{
			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework() && m_bEnabled)
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
	}

	enum EInputPorts
	{
		EIP_Text = 0,
		EIP_AlphaColor,
		EIP_Color,
		EIP_Area,
		EIP_Enable,
		EIP_Disable,
	};

	enum EOutputPorts
	{
	};

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<string>("Text", _HELP("Text to display")),
			InputPortConfig<float>("Alpha", 1.0f, _HELP("Alpha component of text")),
			InputPortConfig<Vec3>("Color", Vec3(1,1,1), _HELP("Color of text")),
			InputPortConfig<int>("Area", 0, _HELP("Position area"), 0, _HELP("enum_int:Middle=0,Top=1,Bottom=2")),
			InputPortConfig_Void("Enable", _HELP("Enable input checking")),
			InputPortConfig_Void("Disable", _HELP("Disable input checking")),
			{0}
		};
		static const SOutputPortConfig outputs[] =
		{
			{0}
		};
		config.nFlags = EFLN_TARGET_ENTITY;
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Displays text centered over entity's screen position");
		config.SetCategory(EFLN_APPROVED);
	}

	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
		{
			// Reset pos
			m_bEnabled = true;

			m_actInfo = *pActInfo;

			// Listener
			if (g_pGame && g_pGame->GetIGameFramework())
				g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
		}
		break;

		case eFE_Activate:
		{
			if (IsPortActive(pActInfo, EIP_Enable) && false == m_bEnabled)
			{
				m_bEnabled = true;
				if (g_pGame && g_pGame->GetIGameFramework())
					g_pGame->GetIGameFramework()->RegisterListener(this, "FlowNode_EntityScreenPos", FRAMEWORKLISTENERPRIORITY_DEFAULT);
			}
			if (IsPortActive(pActInfo, EIP_Disable) && true == m_bEnabled)
			{
				m_bEnabled = false;
				if (g_pGame && g_pGame->GetIGameFramework())
					g_pGame->GetIGameFramework()->UnregisterListener(this);
			}
		}
		break;
		}
	}

	////////////////////////////////////////////////////
	// ~IGameFrameworkListener
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(const char* nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	virtual void OnPostUpdate(float fDeltaTime)
	{
		if (false == m_bEnabled) return;
		IUIDraw* pDraw = NULL;
		if (g_pGame)
			if (IGameFramework* pFW = g_pGame->GetIGameFramework())
				pDraw = pFW->GetIUIDraw();
		if (NULL == pDraw) return;

		// Get screen pos
		Vec3 vScreenPos;
		if (GetEntityScreenPos(m_actInfo.pEntity, GetPortInt(&m_actInfo, EIP_Area), vScreenPos))
		{
			// Get text and area info
			IFFont* pFont = gEnv->pCryFont->GetFont("default");
			string szText = GetPortString(&m_actInfo, EIP_Text);
			float fA = GetPortFloat(&m_actInfo, EIP_AlphaColor);
			Vec3 vColor = GetPortVec3(&m_actInfo, EIP_Color);

			// Convert to right coordinate system
			IRenderer* pRenderer = gEnv->pSystem->GetIRenderer();
			const float fWidthH = pRenderer->GetWidth() * 0.5f;
			const float fHeightH = pRenderer->GetHeight() * 0.5f;
			const float fScale = (300.0f / fHeightH);
			vScreenPos.x = (vScreenPos.x - fWidthH) * fScale;
			vScreenPos.y = (vScreenPos.y - fHeightH) * fScale;

			// Display centered
			pDraw->PreRender();
			pDraw->DrawText(pFont, vScreenPos.x, vScreenPos.y, 0, 0, szText, fA, vColor.x, vColor.y, vColor.z,
				UIDRAWHORIZONTAL_CENTER, UIDRAWVERTICAL_CENTER, UIDRAWHORIZONTAL_CENTER, UIDRAWVERTICAL_CENTER);
			pDraw->PostRender();
		}
	}
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("Entity:EntityScreenPos", CFlowNode_EntityScreenPos);
REGISTER_FLOW_NODE("Entity:GetScreenPos", CFlowNode_GetEntityScreenPos);
REGISTER_FLOW_NODE("Entity:EntityScreenText", CFlowNode_EntityScreenText);