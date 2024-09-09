#include "StdAfx.h"
#include "ZeusModule.h"
#include "HUD/HUD.h"
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>

void CTOSZeusModule::HUDInit()
{
	m_animZeusScreenIcons.Load("Libs/UI/HUD_Zeus_Icon.swf", eFD_Center, eFAF_Visible);
	m_animZeusScreenIcons.GetFlashPlayer()->SetFSCommandHandler(this);
}

void CTOSZeusModule::HandleFSCommand(const char* pCommand, const char* pArgs)
{
	string sCommand = pCommand;
	string sArgs = pArgs;
	if (sCommand == "MousePressOnUnit")
	{
		auto pEntity = TOS_GET_ENTITY(atoi(sArgs));
		if (pEntity)
		{
			OnEntityIconPressed(pEntity);
		}
	}
	else if (sCommand == "MouseOverUnit")
	{
		m_mouseOveredEntityId = atoi(sArgs);
	}
	else if (sCommand == "MouseOutUnit")
	{
		m_mouseOveredEntityId = 0;
	}
	else if (sCommand == "Debug")
	{
		CryLogAlways("[Flash] %s", sArgs);
	}
}

void CTOSZeusModule::HUDInGamePostUpdate(float frametime)
{
	HUDUpdateAllZeusUnitIcons();
}

void CTOSZeusModule::HUDUnloadSimpleAssets(bool unload)
{
	if (!unload)
	{
		m_animZeusScreenIcons.Reload();
		m_animZeusScreenIcons.GetFlashPlayer()->SetFSCommandHandler(this);
	}
	else
	{
		m_animZeusScreenIcons.Unload();
	}
}

void CTOSZeusModule::HUDUpdateZeusUnitIcon(EntityId objective, int friendly, int iconType, const Vec3 localOffset)
{
	auto pAnim = &m_animZeusScreenIcons;

	auto pHUD = g_pGame->GetHUD();
	if (!pHUD)
		return;

	if (pHUD->GetModalHUD() == &pHUD->m_animScoreBoard)
		return;

	IEntity* pObjectiveEntity = GetISystem()->GetIEntitySystem()->GetEntity(objective);
	if (!pObjectiveEntity)
		return;

	const Vec3 vWorldPos = pObjectiveEntity->GetWorldPos() += localOffset;

	int healthValue = 100;

	Vec3 vEntityScreenSpace;
	gEnv->pRenderer->ProjectToScreen(vWorldPos.x, vWorldPos.y, vWorldPos.z, &vEntityScreenSpace.x, &vEntityScreenSpace.y, &vEntityScreenSpace.z);

	float fScaleX = 0.0f;
	float fScaleY = 0.0f;
	float fHalfUselessSize = 0.0f;
	pHUD->GetProjectionScale(pAnim, &fScaleX, &fScaleY, &fHalfUselessSize);

	float rotation = 0.0f;

	int		iMinDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_near_distance", 10);
	int		iMaxDist = TOS_Console::GetSafeIntVar("tos_sv_zeus_on_screen_far_distance", 500);
	float	fMinSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_near_size", 1.4f);
	float	fMaxSize = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_far_size", 0.7f);

	auto pPlayerActor = static_cast<CTOSActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	float fDist = (vWorldPos - pPlayerActor->GetEntity()->GetWorldPos()).len();
	float fSize = 1.0f;

	if (fDist <= iMinDist)
	{
		fSize = fMinSize;
	}
	else if (fDist >= iMaxDist)
	{
		fSize = fMaxSize;
	}
	else if (iMaxDist > iMinDist)
	{
		float fA = ((float)iMaxDist - fDist);
		float fB = (float)(iMaxDist - iMinDist);
		float fC = (fMinSize - fMaxSize);
		fSize = ((fA / fB) * fC) + fMaxSize;
	}

	const float offsetX = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetX", 0.0f);
	const float offsetY = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_offsetY", 0.0f);

	const float transX = vEntityScreenSpace.x * fScaleX + fHalfUselessSize + offsetX;
	const float transY = vEntityScreenSpace.y * fScaleY + offsetY;

	static const wchar_t* localizedText = L"";

	auto iter = stl::binary_find(m_selectedEntities.cbegin(), m_selectedEntities.cend(), objective);
	const bool selected = iter != m_selectedEntities.cend();

	SOnScreenIcon icon(objective, transX, transY, (int)iconType, friendly, fDist, fSize * fSize, -rotation, healthValue, (int)selected);
	localizedText = pHUD->LocalizeWithParams(pObjectiveEntity->GetName(), true);
	icon.text.append(localizedText);

	m_onScreenIcons.push_back(icon);
}
void CTOSZeusModule::HUDUpdateAllZeusUnitIcons()
{
	auto pAnim = &m_animZeusScreenIcons;
	auto icons = &m_onScreenIcons;

	//const auto pClient = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	//const bool isZeus = pClient && pClient->IsZeus();
	//const bool visible = isZeus && TOS_Console::GetSafeIntVar("tos_sv_zeus_force_on_screen_icons", 1);
	if (icons->size())
	{
		pAnim->SetVisible(true);

		const int parametersCount = 9;

		//CW original code
		std::vector< CryFixedStringT<128> > iconList;
		iconList.reserve(parametersCount * icons->size());
		char tempBuf[128];
		auto it = icons->begin();
		for (; it != icons->end(); ++it)
		{
			SOnScreenIcon icon = (*it);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", (int)icon.id);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.x);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.y);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.icontype);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.friendly);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.distance);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.size);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%f", icon.rotation);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.healthValue);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);

			_snprintf(tempBuf, sizeof(tempBuf), "%d", icon.selected);
			tempBuf[sizeof(tempBuf) - 1] = '\0';
			iconList.push_back(tempBuf);
		}

		int size = iconList.size();
		if (size)
		{
			std::vector<const char*> pushArray;
			pushArray.reserve(size);
			for (int i(0); i < size; ++i)
			{
				pushArray.push_back(iconList[i].c_str());
			}

			pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstStrPtr, "m_allValues", 0, &pushArray[0], pushArray.size());
		}


		std::vector< CryFixedWStringT<64> > iconListWString;
		iconList.reserve(icons->size());

		for (auto it = icons->begin(); it != icons->end(); ++it)
		{
			SOnScreenIcon icon = (*it);
			iconListWString.push_back(icon.text);
		}

		size = iconListWString.size();
		if (size)
		{
			std::vector<const wchar_t*> pushArray;
			pushArray.reserve(size);
			for (int i(0); i < size; ++i)
			{
				pushArray.push_back(iconListWString[i].c_str());
			}

			pAnim->GetFlashPlayer()->SetVariableArray(FVAT_ConstWstrPtr, "m_allNames", 0, &pushArray[0], pushArray.size());
		}

		pAnim->Invoke("updateMissionObjectives");
		//~ CW original code

		// Commented CW original code
		//const char* description = "";
		//if (m_pHUDRadar)
		//	description = m_pHUDRadar->GetObjectiveDescription(m_objectiveNearCenter);

		//SFlashVarValue args[2] = {(int)m_objectiveNearCenter, description};
		//m_animMissionObjective.Invoke("setNearCenter", args, 2);
		//m_objectiveNearCenter = 0;
		// ~Commented CW original code
	}
	else if (pAnim->GetVisible())
	{
		pAnim->SetVisible(false);
	}

	icons->clear();
}

