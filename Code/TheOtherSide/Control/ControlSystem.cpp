#include <StdAfx.h>

#include "HUD/HUD.h"
#include "Player.h"

#include "GameCVars.h"
#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
#include "TheOtherSide/Conqueror/ConquerorChannel.h"
#include "TheOtherSide/AI Files/AIActionTracker.h"

#include "VehicleMovementBase.h"
#include "TheOtherSide/Helpers/TOS_Vehicle.h"
#include "TheOtherSide/Helpers/TOS_HUD.h"

#include "windows.h"

CControlSystem::CControlSystem()
{
	m_vehiclesMovementInfo.clear();
	m_vehiclesStuckTime.clear();
	m_vehiclesStuckFlag.clear();
	m_childs.clear();
	m_fgChilds.clear();
	m_pLocalNetControlClient = nullptr;
	m_pLocalControlClient = nullptr;
	m_pAIActionTracker = new CAIActionTracker();
	m_pSquadSystem = new CSquadSystem();
	m_pAbilitiesSystem = new CAbilitiesSystem();
	m_pConquerorSystem = new CConquerorSystem();
	m_isDebugLog = false;
	m_mouseWorldPos = Vec3(ZERO);
	m_screenMouseX = m_screenMouseY = 0;
};
CControlSystem::~CControlSystem()
{
};

void CControlSystem::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_screenMouseX = iX;
	m_screenMouseY = iY;

	iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, iY, 0.0f, &m_mouseWorldPos.x, &m_mouseWorldPos.y, &m_mouseWorldPos.z);

	//CryLogAlways("[CControlSystem::OnHardwareMouseEvent][WorldPos] x%1.f, y%1.f, z%1.f", m_mouseWorldPos.x, m_mouseWorldPos.y, m_mouseWorldPos.z);
}

bool CControlSystem::OnInputEvent(const SInputEvent& event)
{
	if (gEnv->bEditor && g_pGame->GetIGameFramework()->IsEditing())
		return false;

	for (const auto pChild : m_childs)
	{
		if (pChild)
		{
			pChild->OnInputEvent(event);
		}
	}

	for (const auto pFGChild : m_fgChilds)
	{
		pFGChild->OnInputEvent(event);
	}

	//if (m_pSquadSystem)
	//{
	//	m_pSquadSystem->OnInputEvent(event);
	//}

	//if (m_pConquerorSystem)
	//{
	//	m_pConquerorSystem->OnInputEvent(event);
	//}

	//if (m_pAbilitiesSystem)
	//{
	//	m_pAbilitiesSystem->OnInputEvent(event);
	//}

	return false;
}

void CControlSystem::OnEnterVehicle(IActor* pActor, IVehicle* pVehicle, const char* strSeatName, bool bThirdPerson)
{
	for (const auto pChild : m_childs)
		pChild->OnEnterVehicle(pActor, pVehicle);

	//if (m_pSquadSystem)
	//{
	//	m_pSquadSystem->OnEnterVehicle(pActor, pVehicle);
	//}

	//if (m_pConquerorSystem)
	//{
	//	m_pConquerorSystem->OnEnterVehicle(pActor, pVehicle);
	//}
}

void CControlSystem::OnExitVehicle(IActor* pActor)
{
	for (const auto pChild : m_childs)
		pChild->OnExitVehicle(pActor);

	//if (m_pSquadSystem)
	//{
	//	m_pSquadSystem->OnExitVehicle(pActor);
	//}

	//if (m_pConquerorSystem)
	//{
	//	m_pConquerorSystem->OnExitVehicle(pActor);
	//}
}

void CControlSystem::Init(const char* version)
{
	if (m_pAIActionTracker)
		m_pAIActionTracker->Init();

	if (m_pSquadSystem)
		m_pSquadSystem->Init();

	if (m_pAbilitiesSystem)
		m_pAbilitiesSystem->Init();

	if (m_pConquerorSystem)
		m_pConquerorSystem->Init();

	g_pGame->GetIGameFramework()->GetILevelSystem()->AddListener(this);
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->RegisterListener(this);
	g_pGame->GetIGameFramework()->GetIItemSystem()->RegisterListener(this);
	gEnv->pInput->AddEventListener(this);

	if (g_pGameCVars && gEnv->pSystem->IsDevMode())
		g_pGameCVars->tos_show_version = 1;

	CryLogAlways("TheOtherSide initialization [Build %s]", version);
	//gEnv->pHardwareMouse->AddListener(this);
}

void CControlSystem::OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event)
{
	switch (event.event)
	{
	case eGE_GameStarted:
		if (m_isDebugLog)
			CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_GameStarted][Value %f]",event.value);
		break;
	case eGE_Connected:
		if (m_isDebugLog && pEntity)
			CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_Connected][Entity Name %s]", pEntity->GetName());
		break;
	case eGE_Disconnected:
		if (m_isDebugLog && pEntity)
			CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_Disconnected][Entity Name %s]", pEntity->GetName());
		break;
	case eGE_Spectator:
		if (m_isDebugLog && pEntity)
			CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_Spectator][Entity Name %s]", pEntity->GetName());
		break;
	case eGE_Revive:
		if (m_isDebugLog && pEntity)
			CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_Revive][Entity Name %s]", pEntity->GetName());
		break;
	//case eGE_PreGameStarted:
	//	if (m_isDebugLog)
	//		CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_PreGameStarted][Value %f]",event.value);
	//	break;
	//case eGE_PreGameEnd:
	//	if (m_isDebugLog)
	//		CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_PreGameEnd][Value %f]", event.value);
	//	break;
	//case eGE_OnChangeDesiredActor:
	//	if (m_pLocalControlClient && pEntity)
	//		m_pLocalControlClient->m_mpLastControlledId = pEntity->GetId();
	//	if (m_isDebugLog && pEntity)
	//		CryLogAlways("[C++][ControlSystem][OnGameplayEvent][eGE_OnChangeDesiredActor][Entity Name %s, ActorName %s]", pEntity->GetName(), event.description);
	//	break;
	}
}

void CControlSystem::OnLevelNotFound(const char* levelName)
{

}

void CControlSystem::OnLoadingStart(ILevelInfo* pLevel)
{
	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnLevelLoadingStart(pLevel);

	for (const auto pListener : m_childs)
		pListener->OnGameRulesReset();

	if (m_isDebugLog)
		CryLogAlways("[ControlSystem][OnLoadingStart][Level Name] %s", pLevel->GetName());
}

void CControlSystem::OnLoadingComplete(ILevel* pLevel)
{
	if (m_isDebugLog)
		CryLogAlways("[ControlSystem][OnLoadingComplete][Level Name] %s", pLevel->GetLevelInfo()->GetName());
}

void CControlSystem::OnLoadingError(ILevelInfo* pLevel, const char* error)
{
	if (m_isDebugLog)
		CryLogAlways("[ControlSystem][OnLoadingError][Level Name: %s][Error: %s] ", pLevel->GetName(), error);
}

void CControlSystem::OnLoadingProgress(ILevelInfo* pLevel, int progressAmount)
{

}

void CControlSystem::OnMainMenuEnter()
{
	for (const auto pChild : m_childs)
		pChild->OnMainMenuEnter();
}

void CControlSystem::OnActorDeath(IActor* pActor)
{
	if (m_pLocalControlClient)
		m_pLocalControlClient->OnActorDeath(pActor);

	for (const auto pChild : m_childs)
		pChild->OnActorDeath(pActor);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnActorDeath(pActor);

	//if (m_pAbilitiesSystem)
	//	m_pAbilitiesSystem->OnActorDeath(pActor);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnActorDeath(pActor);
}

void CControlSystem::OnActorGrabbed(IActor* pActor, EntityId grabberId)
{
	for (const auto pChild : m_childs)
		pChild->OnActorGrabbed(pActor, grabberId);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnActorGrabbed(pActor, grabberId);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnActorGrabbed(pActor, grabberId);
}

void CControlSystem::OnActorDropped(IActor* pActor, EntityId grabberId)
{
	for (const auto pChild : m_childs)
		pChild->OnActorDropped(pActor, grabberId);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnActorDropped(pActor, grabberId);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnActorDropped(pActor, grabberId);
}

void CControlSystem::OnActorGrab(IActor* pActor, EntityId grabId)
{
	for (const auto pChild : m_childs)
		pChild->OnActorGrab(pActor, grabId);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnActorGrab(pActor, grabId);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnActorGrab(pActor, grabId);
}

void CControlSystem::OnActorDrop(IActor* pActor, EntityId dropId)
{
	for (const auto pChild : m_childs)
		pChild->OnActorDrop(pActor, dropId);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnActorDrop(pActor, dropId);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnActorDrop(pActor, dropId);
}

void CControlSystem::OnGameRulesReset()
{
	m_vehiclesStuckFlag.clear();
	m_vehiclesStuckTime.clear();

	g_pControlSystem->StopLocal(true);

	for (const auto pChild : m_childs)
		pChild->OnGameRulesReset();

	//issue #58 additional bug fix
	gEnv->pAISystem->AllocGoalPipeId();

	if (gEnv->bEditor)
	{
		const auto pMusicSystem = gEnv->pMusicSystem;
		if (pMusicSystem)
			pMusicSystem->EndTheme(EThemeFade_FadeOut);
	}

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->OnGameRulesReset();

	//if (m_pSquadSystem)
	//	m_pSquadSystem->OnGameRulesReset();

	//if (m_pAbilitiesSystem)
	//	m_pAbilitiesSystem->OnGameRulesReset();
}

void CControlSystem::OnVehicleStuck(IVehicle* pVehicle, bool stuck)
{
	if (m_pConquerorSystem)
		m_pConquerorSystem->OnVehicleStuck(pVehicle, stuck);
}

bool CControlSystem::GetLocalEnabled()
{
	 return (m_pLocalControlClient && m_pLocalControlClient->GetControlledEntity());
}

void CControlSystem::StartLocal(IActor* pActor, bool dudeHide, bool dudeBeam)
{
	const auto* pLocalActor = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pLocalActor)
		return;

	if (!m_pLocalControlClient)
		return;

	m_pLocalControlClient->InitDudeToControl(true);
	m_pLocalControlClient->SetActor(dynamic_cast<CActor*>(pActor));
	m_pLocalControlClient->ToggleDudeBeam(dudeBeam);
	m_pLocalControlClient->ToggleDudeHide(dudeHide);

	CConquerorChannel* pChannel = m_pConquerorSystem->GetConquerorChannel(pLocalActor->GetEntity());
	if (pChannel)
		pChannel->SetControlledEntity(pActor->GetEntityId());

	for (const auto pChild : m_childs)
	{
		pChild->OnStartControl(pActor);
	}

	for (const auto pChild : m_fgChilds)
	{
		pChild->OnStartControl(pActor);
	}
}

void CControlSystem::StopLocal(bool toEditor)
{
	const auto* pDude = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pDude)
		return;

	if (!m_pLocalControlClient)
		return;

	if (GetLocalEnabled())
	{
		//for (auto pChild : m_childs)
		//{
		//	if (strcmp(pChild->GetChildName(), "CFlowNode_OnStartControl") == 0)
		//		pChild->OnStopControl(m_pLocalControlClient->GetControlledActor());
		//}

		for (const auto pChild : m_childs)
		{
			pChild->OnStopControl(m_pLocalControlClient->GetControlledActor());
		}

		for (const auto pChild : m_fgChilds)
		{
			pChild->OnStopControl(m_pLocalControlClient->GetControlledActor());
		}

		m_pLocalControlClient->InitDudeToControl(false);
		m_pLocalControlClient->Reset(toEditor);

		CConquerorChannel* pChannel = m_pConquerorSystem->GetConquerorChannel(pDude->GetEntity());
		if (pChannel)
			pChannel->SetControlledEntity(0);
	}
}

void CControlSystem::ResetLocal()
{
	
}

void CControlSystem::Update(float frametime)
{
	m_isDebugLog = g_pGameCVars->tos_debug_log_all == 1;

	if (m_pLocalControlClient)
	{
		//Update crosshair position and etc.
		m_pLocalControlClient->UpdateCrosshair();

		if (GetLocalEnabled())
			m_pLocalControlClient->Update();
	}

	for (const auto pChild : m_childs)
		pChild->Update(frametime);

	for (const auto& infoPair : m_vehiclesMovementInfo)
	{
		const auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(infoPair.first);
		if (!pVehicle)
			continue;

		const auto pInfo = infoPair.second;
		if (!pInfo)
			continue;

		pe_status_vehicle status;
		if (!pVehicle->GetEntity()->GetPhysics())
			continue;

		if (!pVehicle->GetEntity()->GetPhysics()->GetStatus(&status))
			continue;

		const float speed = status.vel.len();
		const auto& action = pInfo->GetMovementAction();

		const auto isPedaling = action.power != 0;
		const auto isSteering = action.rotateYaw != 0;
		if (isPedaling)
		{
			if (speed < 2.0f)
			{
				//Vehicle get stuck if do this line for 3 seconds
				m_vehiclesStuckTime[pVehicle->GetEntityId()] += frametime;
			}
		}
		else
		{
			m_vehiclesStuckTime[pVehicle->GetEntityId()] = 0;
		}
	}

	for (auto& idPair : m_vehiclesStuckTime)
	{
		const auto stucked = m_vehiclesStuckFlag[idPair.first] == 1;

		if (idPair.second >= 3.0f)
		{
			if (!stucked)
			{
				m_vehiclesStuckFlag[idPair.first] = 1;

				const auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(idPair.first);
				if (pVehicle)
					OnVehicleStuck(pVehicle, true);
			}
		}
		else if (idPair.second == 0)
		{
			if (stucked)
			{
				m_vehiclesStuckFlag[idPair.first] = 0;

				const auto pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(idPair.first);
				if (pVehicle)
					OnVehicleStuck(pVehicle, false);
			}
		}
	}
}

void CControlSystem::RegisterLocalControlClient(CControlClient* _pControlClient)
{
	m_pLocalControlClient = _pControlClient;
}

void CControlSystem::RegisterNetLocalControlClient(CNetControlClient* pControlClient)
{
	m_pLocalNetControlClient = pControlClient;
}

void CControlSystem::AddChild(IControlSystemChild* pChild, bool FG)
{
	if (!pChild)
		return;

	stl::push_back_unique(FG ? m_fgChilds : m_childs, pChild);
}

void CControlSystem::RemoveChild(IControlSystemChild* pChild, bool FG)
{
	stl::find_and_erase(FG ? m_fgChilds : m_childs, pChild);
}

CActor* CControlSystem::GetClientActor()
{
	CActor* pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (g_pControlSystem->GetLocalControlClient() && g_pControlSystem->GetLocalControlClient()->GetControlledActor())
		pPlayer = g_pControlSystem->GetLocalControlClient()->GetControlledActor();

	return pPlayer;
}

void CControlSystem::AddVehicleMovementInfo(EntityId vehicleId, CVehicleMovementBase* info)
{
	m_vehiclesMovementInfo[vehicleId] = info;
	m_vehiclesStuckFlag[vehicleId] = 0;
	m_vehiclesStuckTime[vehicleId] = 0;
}

CVehicleMovementBase* CControlSystem::GetVehicleMovementInfo(EntityId vehicleId)
{
	return m_vehiclesMovementInfo[vehicleId];
}

bool CControlSystem::IsVehicleStuck(const IVehicle* pVehicle) const
{
	const auto iter = m_vehiclesStuckFlag.find(pVehicle->GetEntityId());
	if (iter != m_vehiclesStuckFlag.end())
		return iter->second == 1;

	return false;
}

void CControlSystem::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	for (const auto pChild : m_childs)
		pChild->GetMemoryStatistics(s);

	if (m_pLocalControlClient)
		m_pLocalControlClient->GetMemoryStatistics(s);

	//if (m_pAbilitiesSystem)
	//	m_pAbilitiesSystem->GetMemoryStatistics(s);

	//if (m_pSquadSystem)
	//	m_pSquadSystem->GetMemoryStatistics(s);

	//if (m_pConquerorSystem)
	//	m_pConquerorSystem->GetMemoryStatistics(s);
}

void CControlSystem::Shutdown()
{
	SAFE_DELETE(m_pSquadSystem);
	SAFE_DELETE(m_pAbilitiesSystem);
	SAFE_DELETE(m_pConquerorSystem);

	g_pGame->GetIGameFramework()->GetILevelSystem()->RemoveListener(this);
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->UnregisterListener(this);
	g_pGame->GetIGameFramework()->GetIItemSystem()->UnregisterListener(this);
	gEnv->pInput->RemoveEventListener(this);
	//gEnv->pHardwareMouse->RemoveListener(this);
}

Vec3 CControlSystem::GetMouseScreenPos()
{
	return Vec3(m_screenMouseX,m_screenMouseY,0);
}

Vec3 CControlSystem::GetMouseWorldPos()
{
	return m_mouseWorldPos;
}

EntityId CControlSystem::GetMouseEntityID()
{
	auto pClientActor = g_pControlSystem->GetLocalControlClient()->GetControlledActor();
	if (!pClientActor)
		pClientActor = dynamic_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (!pClientActor)
		return 0;

	const auto pClientVehicle = TOS_Vehicle::GetVehicle(pClientActor);

	const auto vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
	const auto vDir = (m_mouseWorldPos - vCamPos).GetNormalizedSafe();

	const auto pPhysicalEnt = pClientActor->GetEntity() ? pClientActor->GetEntity()->GetPhysics() : NULL;
	if (!pPhysicalEnt)
		return 0;

	static IPhysicalEntity* pSkipEnts[2] = {nullptr, nullptr};
	pSkipEnts[0] = pPhysicalEnt;
	int numSkipped = 1;

	if (pClientVehicle)
	{
		pSkipEnts[1] = pClientVehicle->GetEntity()->GetPhysics();
		numSkipped++;
	}

	const auto queryFlags = ent_all;

	static ray_hit hit;
	static const unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
	const float fRange = gEnv->p3DEngine->GetMaxViewDistance();

	if (gEnv->pPhysicalWorld && gEnv->pPhysicalWorld->RayWorldIntersection(vCamPos, vDir * fRange, queryFlags, flags, &hit, 1, pSkipEnts, numSkipped))
	{
		if (gEnv->p3DEngine->RefineRayHit(&hit, vDir * fRange))
		{
			if (m_pSquadSystem)
				m_pSquadSystem->m_storedMouseWorldPos = hit.pt;

			if (const auto pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider))
				return pEntity->GetId();
		}
	}

	return 0;
}

void CControlSystem::OnSetActorItem(IActor* pActor, IItem* pItem)
{
	if (!pActor || !pItem || !GetLocalControlClient())
		return;

	const auto pControlledActor = GetLocalControlClient()->GetControlledActor();

	if (pControlledActor == pActor)
	{
		const char* curClass = NULL;
		const char* curCategory = NULL;

		const IEntity* pItemEntity = pItem->GetEntity();
		if (pItemEntity)
		{
			const IEntityClass* pItemClass = pItemEntity->GetClass();
			if (pItemClass)
			{
				curClass = pItemClass->GetName();
				if (!strcmp(curClass, "Detonator"))
				{
					curClass = "C4";
				}
				curCategory = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItemCategory(curClass);
			}
		}

		if (curCategory && curClass)
		{
			//TheOtherSide
			TOS_HUD::ShowInventory(pActor, curCategory, curClass);
			//~TheOtherSide
		}
	}
}

void CControlSystem::OnDropActorItem(IActor* pActor, IItem* pItem)
{
	if (!pActor)
		return;
}
