// ReSharper disable CppMsExtBindingRValueToLvalueReference
// ReSharper disable CppInconsistentNaming
// ReSharper disable CppVariableCanBeMadeConstexpr
#include "StdAfx.h"
#include "IAgent.h"

//#include "HUD/HUD.h"
//#include "HUD/HUDScopes.h"
//#include "HUD/HUDCrosshair.h"
//#include "HUD/HUDSilhouettes.h"

#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/TOSGame.h"

//#include "PlayerInput.h"

#include "MasterClient.h"

#include "GameActions.h"
#include "GameUtils.h"
#include "IPlayerInput.h"
#include "IViewSystem.h"
#include "MasterSynchronizer.h"
#include "Single.h"

#include "HUD/HUD.h"
#include "HUD/HUDCrosshair.h"

#include "TheOtherSideMP/Control/ControlSystem.h"
#include "TheOtherSideMP/Helpers/TOS_AI.h"
#include "TheOtherSideMP/Helpers/TOS_Console.h"
#include "TheOtherSideMP/HUD/TOSCrosshair.h"

// Akeeper 28.01.2024:
// Проблема: действия будут отпускаться 2 раза подряд. Может вызвать баги.
// Решение: если режим активации - отпустить, то вызывать другую функцию, которая будет вызывать уже эту
// с указанным временем нажатия.
#define ASSING_ACTION(pActor, actionId, activationMode, pressedDuration, checkActionId, func)\
if ( (actionId) == (checkActionId) )\
	func( (pActor), (actionId), (activationMode), value, (pressedDuration))\

CTOSMasterClient::CTOSMasterClient(CTOSPlayer* pPlayer)
	: m_pLocalDude(pPlayer),
	m_pSlaveEntity(nullptr),
	m_dudeFlags(0),
	m_actions(0)
{
	assert(pPlayer);
    m_deltaMovement.zero();
}

CTOSMasterClient::~CTOSMasterClient()
{
}

void CTOSMasterClient::OnEntityEvent(IEntity* pEntity, const SEntityEvent& event)
{
	if (m_pSlaveEntity != nullptr && pEntity == m_pSlaveEntity)
	{
		switch (event.event)
		{
		case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			PrePhysicsUpdate();
			break;
		}
		default:
			break;
		}
	}
}

void CTOSMasterClient::OnAction(const ActionId& action, const int activationMode, const float value)
{
	const CGameActions& rGA = g_pGame->Actions();

    //CRY_ASSERT_MESSAGE(m_pSlaveEntity, "Pointer to slave entity is null");
	if (!m_pSlaveEntity)
        return;

	const auto pSlaveActor = GetSlaveActor();
	//CRY_ASSERT_MESSAGE(m_pSlaveEntity, "Pointer to slave actor is null");
	if (!pSlaveActor)
        return;

	float pressedDuration = 0.0f;

	//m_pPlayerGruntInput->OnAction(action, activationMode, value);

	// Здесь описана логика удержания клавиши нажатой и как долго она была в таком состоянии
	// Используется, когда высота прыжка зависит от длительности нажатия на действие прыжка [jump]
	if (activationMode == eAAM_OnPress)
	{
		m_actionFlags[action] |= TOS_PRESSED;
	}
	else if (activationMode == eAAM_OnRelease)
	{
		if (m_actionFlags[action] & TOS_PRESSED)
		{
			pressedDuration = stl::find_in_map(m_actionPressedDuration, action, 0.0f);

			OnActionDelayReleased(action, pressedDuration);
		}

		m_actionFlags[action] &= ~TOS_PRESSED;
		m_actionFlags[action] &= ~TOS_HOLD;
	}

	if (activationMode == eAAM_OnHold)
	{
		m_actionFlags[action] |= TOS_HOLD;
	}

	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.attack1, OnActionAttack);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.special, OnActionSpecial);// it is melee
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.moveforward, OnActionMoveForward);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.moveback, OnActionMoveBack);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.moveleft, OnActionMoveLeft);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.moveright, OnActionMoveRight);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.jump, OnActionJump);
	ASSING_ACTION(pSlaveActor, action, activationMode, pressedDuration, rGA.sprint, OnActionSprint);
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool CTOSMasterClient::OnActionAttack(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur)
{
	const auto pInventory = pActor->GetInventory();
    CRY_ASSERT_MESSAGE(pInventory, "[OnActionAttack] pInventory pointer is NULL");
	if (!pInventory)
		return false;

	const auto pCurItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pInventory->GetCurrentItem());
	CRY_ASSERT_MESSAGE(pCurItem, "[OnActionAttack] pCurItem pointer is NULL");
    if (!pCurItem)
        return false;

	IWeapon* pWeapon = pCurItem->GetIWeapon();  // NOLINT(clang-diagnostic-misleading-indentation)
	CRY_ASSERT_MESSAGE(pWeapon, "[OnActionAttack] pWeapon pointer is NULL");
    if (!pWeapon)
        return false;

    pWeapon->OnAction(pActor->GetEntityId(), actionId, activationMode, value);

    return true;
}

bool CTOSMasterClient::OnActionSpecial(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur)
{
	const auto pInventory = pActor->GetInventory();
	CRY_ASSERT_MESSAGE(pInventory, "[OnActionSpecial] pInventory pointer is NULL");
	if (!pInventory)
		return false;

	const auto pCurItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pInventory->GetCurrentItem());
	CRY_ASSERT_MESSAGE(pCurItem, "[OnActionSpecial] pCurItem pointer is NULL");
	if (!pCurItem)
		return false;

	IWeapon* pWeapon = pCurItem->GetIWeapon();  // NOLINT(clang-diagnostic-misleading-indentation)
	CRY_ASSERT_MESSAGE(pWeapon, "[OnActionSpecial] pWeapon pointer is NULL");
	if (!pWeapon)
		return false;
	
	pWeapon->OnAction(m_pSlaveEntity->GetId(), actionId, activationMode, value);

    return true;
}

bool CTOSMasterClient::OnActionMoveForward(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value, float pressedDur)
{
	m_deltaMovement.x = m_deltaMovement.z = 0;
	m_deltaMovement.y = value * 2.0f - 1.0f;

	//pActor->ApplyMasterMovement(delta);
    m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionMoveBack(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value, float pressedDur)
{
	m_deltaMovement.x = m_deltaMovement.z = 0;
    m_deltaMovement.y = -(value * 2.0f - 1.0f);

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionMoveLeft(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value, float pressedDur)
{
	m_deltaMovement.x = -(value * 2.0f - 1.0f);
    m_deltaMovement.y = m_deltaMovement.z = 0;

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

    return true;
}

bool CTOSMasterClient::OnActionMoveRight(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value, float pressedDur)
{
	m_deltaMovement.x = value * 2.0f - 1.0f;
	m_deltaMovement.y = m_deltaMovement.z = 0;

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionJump(CTOSActor* pActor, const ActionId& actionId, const int activationMode, const float value, float pressedDur)
{
	//Akeeper 28/01/2024
	//Пояснение: заряжаемый прыжок доступен ТОЛЬКО когда актёр на твердой поверхности,
	// потому что сделать двойной прыжок за трупера при заряжаемом прыжке нужно быстро а не ждать,
	// пока пройдет заряд.
	const float holdTime = stl::find_in_map(m_actionPressedDuration, actionId, 0.0f);

	// Проверка наличия возможности заряженного прыжка и контакта с землей
	if (pActor->IsHaveChargingJump() && pActor->GetActorStats()->onGround > 0.0f)
	{
		// Получение времени удержания кнопки и задержки для прыжка
		const float jumpDelay = TOS_Console::GetSafeFloatVar("tos_tr_charging_jump_input_time");

		// Автопрыжок при удержании клавиши прыжка
		if (activationMode == eAAM_OnHold && holdTime > jumpDelay)
		{
			// Установка запроса на прыжок и обнуление длительности нажатия
			//m_movementRequest.SetJump(); // перемещен в PrePhysicsUpdate
			m_actions |= ACTION_JUMP;
			pActor->GetSlaveStats().chargingJumpPressDur = holdTime;
			m_actionPressedDuration[actionId] = 0;
		}
		// Обработка отпускания клавиши прыжка
		else if (activationMode == eAAM_OnRelease && holdTime <= jumpDelay)
		{
			m_actions |= ACTION_JUMP;
			//m_movementRequest.SetJump(); // перемещен в PrePhysicsUpdate
			pActor->GetSlaveStats().chargingJumpPressDur = 0;
		}
	}
	// Обработка прыжка без заряда
	else
	{
		// Автопрыжок при удержании клавиши прыжка
		if (activationMode == eAAM_OnHold)
		{
			// Установка запроса на прыжок и обнуление длительности нажатия
			//m_movementRequest.SetJump(); // перемещен в PrePhysicsUpdate
			m_actions |= ACTION_JUMP;
			m_actionPressedDuration[actionId] = 0;
		}

		// Установка запроса на прыжок при нажатии клавиши
		if ((activationMode == eAAM_OnHold || activationMode == eAAM_OnPress) && value > 0.0f)
		{
			//m_movementRequest.SetJump(); // перемещен в PrePhysicsUpdate
			m_actions |= ACTION_JUMP;
		}
		// Очистка запроса на прыжок при отпускании клавиши
		else if (activationMode == eAAM_OnRelease)
		{
			if (m_actions & ACTION_JUMP)
			{
				m_actions &= ~ACTION_JUMP;
			}

			//if (m_movementRequest.ShouldJump()) // перемещен в PrePhysicsUpdate
			//{
			//	m_movementRequest.ClearJump();
			//}
		}
	}

	return true;
}

bool CTOSMasterClient::OnActionHoldTest(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur)
{
	if (activationMode == eAAM_OnHold)
	{
		CryLogAlways("KEY HOLDING: VALUE = %f", value);
	}

	return true;
}

bool CTOSMasterClient::OnActionSprint(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur)
{
	if (value > 0.0f)
	{
		m_actions |= ACTION_SPRINT;
	}
	else
	{
		m_actions &= ~ACTION_SPRINT;
	}

	return true;
}

void CTOSMasterClient::PrePhysicsUpdate()
{
	const auto pSlaveActor = GetSlaveActor();
	if (!pSlaveActor)
		return;

	const auto pController = pSlaveActor->GetMovementController();
    assert(pController);

	if (m_actions & ACTION_JUMP)
	{
		m_movementRequest.SetJump();
		m_actions &= ~ACTION_JUMP;
	}

	m_movementRequest.SetLookTarget(m_lookfireInfo.lookTargetPos);
	m_movementRequest.SetFireTarget(m_lookfireInfo.fireTargetPos);

	pSlaveActor->ApplyActions(m_actions);

	// В сетевой игре отправка запроса отсюда работает прекрасно
	// В одиночной игре отправка запроса не работает
	SendMovementRequest(pController, m_movementRequest);

	if (m_movementRequest.ShouldJump())
		m_movementRequest.ClearJump();
}

void CTOSMasterClient::Update(float frametime)
{
	//m_pWorldCamera = &gEnv->pSystem->GetViewCamera();
    //CRY_ASSERT_MESSAGE(m_pWorldCamera, "[CTOSMasterClient] m_pWorldCamera pointer is null");

	const auto cam = gEnv->pSystem->GetViewCamera();
	m_cameraInfo.viewDir = cam.GetMatrix().GetColumn1() * 1000.0f;
	m_cameraInfo.worldPos = cam.GetMatrix().GetTranslation();
    m_cameraInfo.lookPointPos = m_cameraInfo.worldPos + m_cameraInfo.viewDir;

	const auto            rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
	static const unsigned entityFlags =
		ent_living |
		ent_rigid |
		ent_static |
		ent_terrain |
		ent_sleeping_rigid |
		ent_independent;

    UpdateCrosshair(m_pSlaveEntity, m_pLocalDude, rayFlags, entityFlags);

	const auto pSlaveActor = GetSlaveActor();
	if (!pSlaveActor)
		return;

	const auto pMovementController = pSlaveActor->GetMovementController();
	if (!pMovementController)
		return;

	SMovementState state;
	pMovementController->GetMovementState(state);

    UpdateMeleeTarget(m_pSlaveEntity, rayFlags, entityFlags, state);
	UpdateLookFire(m_pSlaveEntity, rayFlags, entityFlags, state);

	const auto pFireTargetEntity = TOS_GET_ENTITY(m_lookfireInfo.fireTargetId);
	const auto pMeleeTargetEntity = TOS_GET_ENTITY(m_meleeInfo.targetId);
	const auto pCrosshairTargetEntity = TOS_GET_ENTITY(m_crosshairInfo.targetId);

	if (IsFriendlyEntity(pFireTargetEntity, m_pSlaveEntity) ||
		IsFriendlyEntity(pMeleeTargetEntity, m_pSlaveEntity) ||
		IsFriendlyEntity(pCrosshairTargetEntity, m_pSlaveEntity))
	{
		pSlaveActor->GetSlaveStats().lookAtFriend = true;
	}
	else
	{
		pSlaveActor->GetSlaveStats().lookAtFriend = false;
	}

	//Debug
	if (gEnv->pSystem->IsDevMode() && gEnv->pConsole->GetCVar("tos_sv_mc_LookDebugDraw")->GetIVal() > 0)
	{
		IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin(string("MasterClient") + (m_pSlaveEntity ? m_pSlaveEntity->GetName() : "<undefined>"), true);

		auto red = ColorF(1, 0, 0, 1);
		auto green = ColorF(0, 1, 0, 1);
		auto blue = ColorF(0, 0, 1, 1);

		pPD->AddSphere(m_crosshairInfo.worldPos, 0.25f, red, 1.0f);
		pPD->AddSphere(m_meleeInfo.targetPos, 0.25f, green, 1.0f);
		pPD->AddSphere(m_lookfireInfo.lookTargetPos, 0.25f, blue, 1.0f);

		//float color[] = { 1,1,1,1 };
		//gEnv->pRenderer->Draw2dLabel(100, 100, 1.3f, color, false, "jumpCount = %i", pSlaveActor->GetSlaveStats().jumpCount);
	}

	// В одиночной игре отправка запроса работает только отсюда
	const auto pController = pSlaveActor->GetMovementController();
	assert(pController);

	// Возможно будет работать метод через 
	if (!gEnv->bMultiplayer)
		PrePhysicsUpdate();

	// Подсчет времени, сколько была нажата определенная клавиша в сек.
	std::map<ActionId, uint>::iterator it = m_actionFlags.begin();
	std::map<ActionId, uint>::iterator end = m_actionFlags.end();

	for (; it != end; ++it)
	{
		const ActionId action = it->first;
		const bool pressed = it->second & TOS_PRESSED;

		m_actionPressedDuration[action] += pressed ? frametime : 0.0f;

		if (!pressed)
			m_actionPressedDuration[action] = 0.0f;
	}

}

void CTOSMasterClient::UpdateCrosshair(const IEntity* pSlaveEntity, const IActor* pLocalDudeActor, int rayFlags, unsigned entityFlags)
{
	//Physics entity
	IPhysicalEntity* pDudePhysics = pLocalDudeActor->GetEntity()->GetPhysics();
	IPhysicalEntity* pPhys = (pSlaveEntity != nullptr) ? pSlaveEntity->GetPhysics() : pDudePhysics;

	//Calculate crosshair position and target from camera pos;
	if (gEnv->pPhysicalWorld->RayWorldIntersection(m_cameraInfo.worldPos, m_cameraInfo.viewDir, entityFlags, rayFlags, &m_crosshairInfo.rayHit, 1, pPhys))
	{
		m_crosshairInfo.worldPos = m_crosshairInfo.rayHit.pt;
		if (m_crosshairInfo.rayHit.pCollider)
		{
			const IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_crosshairInfo.rayHit.pCollider);
			if (pTargetEntity)
				m_crosshairInfo.lastTargetId = m_crosshairInfo.targetId = pTargetEntity->GetId();
			else 
                m_crosshairInfo.targetId = NULL;

			if (m_crosshairInfo.rayHit.bTerrain)
				m_crosshairInfo.targetId = NULL;
		}
		else
			m_crosshairInfo.targetId = NULL;
	}
	else
	{
		m_crosshairInfo.worldPos = m_cameraInfo.lookPointPos;
		m_crosshairInfo.targetId = 0;
	}
}

void CTOSMasterClient::UpdateLookFire(const IEntity* pSlaveEntity, const int rayFlags, const unsigned entityFlags, const SMovementState& state)
{
	Vec3 crosshairDir(m_crosshairInfo.worldPos - state.weaponPosition);
	crosshairDir = crosshairDir.GetNormalizedSafe() * 1000;

	const int hits = gEnv->pPhysicalWorld->RayWorldIntersection(state.weaponPosition, crosshairDir, entityFlags, rayFlags, &m_lookfireInfo.rayHit, 1, pSlaveEntity->GetPhysics());

	const IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_lookfireInfo.rayHit.pCollider);

	if (hits > 0)
	{
		if (m_lookfireInfo.rayHit.pCollider)
		{
			if (pTargetEntity)
			{
				m_lookfireInfo.fireTargetId = pTargetEntity->GetId();
			}
			else
				m_lookfireInfo.fireTargetId = NULL;

			if (m_lookfireInfo.rayHit.bTerrain)
				m_lookfireInfo.fireTargetId = NULL;
		}
		else
			m_lookfireInfo.fireTargetId = NULL;
	}
	else
		m_lookfireInfo.fireTargetId = NULL;

	if (m_lookfireInfo.rayHit.dist > 8.50f || pTargetEntity)
		m_lookfireInfo.fireTargetPos = m_crosshairInfo.rayHit.pt;
	else
		m_lookfireInfo.fireTargetPos = m_cameraInfo.lookPointPos;

	// Пусть персонаж будет смотреть туда куда стреляет.
	m_lookfireInfo.lookTargetPos = m_lookfireInfo.fireTargetPos;
}

void CTOSMasterClient::OnActionDelayReleased(const ActionId action, float pressedTimeLen)
{
	CryLog("[%s] is released with time %f", action, pressedTimeLen);
}

void CTOSMasterClient::SendMovementRequest(IMovementController* pController, CMovementRequest& request)
{
	assert(pController);
	if (!pController)
		return;
	
	int disableRequest = 0;
	ICVar* pCvar = gEnv->pConsole->GetCVar("tos_cl_DisableSlaveRequestMovement");
	if (pCvar)
		disableRequest = pCvar->GetIVal();

	if (disableRequest > 0)
		return;

	pController->RequestMovement(m_movementRequest);
}

void CTOSMasterClient::UpdateMeleeTarget(const IEntity* pSlaveEntity, const int rayFlags, const unsigned entityFlags, const SMovementState& state)
{
	if (!pSlaveEntity)
		return;

	const Vec3 toCrosshairDir(m_crosshairInfo.worldPos - state.weaponPosition);

	m_meleeInfo.hitCount = gEnv->pPhysicalWorld->RayWorldIntersection(state.weaponPosition, toCrosshairDir.GetNormalizedSafe() * 3, entityFlags, rayFlags, &m_meleeInfo.rayHit, 1, pSlaveEntity->GetPhysics());

	if (m_meleeInfo.hitCount != 0)
	{
		m_meleeInfo.targetPos = m_meleeInfo.rayHit.pt;

		if (m_meleeInfo.rayHit.pCollider)
		{
			const IEntity* pTargetEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_meleeInfo.rayHit.pCollider);
			if (pTargetEntity)
			{
				m_meleeInfo.targetId = pTargetEntity->GetId();
			}

			if (m_meleeInfo.rayHit.bTerrain)
				m_meleeInfo.targetId = 0;
		}
	}
	else
	{
		m_meleeInfo.targetPos = m_cameraInfo.lookPointPos;
		m_meleeInfo.targetId = 0;
	}
}

CWeapon* CTOSMasterClient::GetCurrentWeapon(const IActor* pActor) const
{
    assert(pActor);
    if (!pActor)
        return nullptr;

	const IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return nullptr;

	const EntityId itemId = pInventory->GetCurrentItem();
	if (!itemId)
		return nullptr;

	IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
	if (!pItem)
		return nullptr;

	auto* pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
	if (!pWeapon)
		return nullptr;

	return pWeapon;
}

bool CTOSMasterClient::IsFriendlyEntity(IEntity* pEntity, IEntity* pTarget) const
{
	//TODO: 10/21/2023, 08:55 Сделать общее хранилище, где и клиент и сервер будут знать фракцию ИИ объекта

	//Only for actors (not vehicles)
	if (pEntity && pEntity->GetAI() && pTarget)
	{
		if (!pEntity->GetAI()->IsHostile(pTarget->GetAI(), false))
			return true;
		return false;
	}

	//Special case (Animated objects), check for script table value "bFriendly"
	//Check script table (maybe is not possible to grab)
	if (pEntity)
	{
		SmartScriptTable props;
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (!pScriptTable || !pScriptTable->GetValue("Properties", props))
			return false;

		int isFriendly = 0;
		if (props->GetValue("bNoFriendlyFire", isFriendly) && isFriendly != 0)
			return true;
	}

	//for vehicles
	if (pEntity && pEntity->GetId())
	{
		IVehicle* pVehicle = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pEntity->GetId());
		if (pVehicle)
		{
			if (pTarget && pVehicle->HasFriendlyPassenger(pTarget))
				return true;
		}
	}

	return false;
	
}

void CTOSMasterClient::AnimationEvent(IEntity* pEntity, ICharacterInstance* pCharacter, const AnimEventInstance& event)
{
	assert(pEntity);
	assert(pCharacter);

    if (pEntity != m_pSlaveEntity)
        return;

    string eventName = event.m_EventName;

    if (eventName == "MeleeDamage")
    {
        //ProcessMeleeDamage();
    }
}

void CTOSMasterClient::StartControl(IEntity* pEntity, uint dudeFlags, bool fromFG, EFactionPriority priority)
{
	assert(pEntity);
    if (!pEntity)
		return;

	const auto pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	m_dudeFlags = dudeFlags;

	const auto dudeId = m_pLocalDude->GetEntityId();
	const auto slaveId = pEntity->GetId();

	if (gEnv->bClient)
	{
		CGameRules::ChangeTeamParams teamParams;
		teamParams.entityId = priority == eFP_Master ? slaveId : dudeId;
		teamParams.teamId = pGameRules->GetTeam(priority == eFP_Master ? dudeId : slaveId);
		pGameRules->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::SvRequestForceSetTeam(), teamParams, eRMI_ToServer, teamParams.entityId);

		CGameRules::ChangeSpeciesParams speciesParams;
		speciesParams.entityId = priority == eFP_Master ? slaveId : dudeId;;
		speciesParams.speciesIdx = pGameRules->GetSpecies(priority == eFP_Master ? dudeId : slaveId);
		pGameRules->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::SvRequestForceSetSpecies(), speciesParams, eRMI_ToServer, speciesParams.entityId);
	}

	auto pNextSlaveActor = static_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));

	//TODO: нужно протестить переключение м/у юнитами
	//const IEntity* const oldSlaveEnt = m_pSlaveEntity;
	if (m_pSlaveEntity == nullptr)
	{
		PrepareNextSlave(pNextSlaveActor);
	}
	else if (m_pSlaveEntity && m_pSlaveEntity != pEntity)
	{
		PreparePrevSlave(GetSlaveActor());
		PrepareNextSlave(pNextSlaveActor);
	}

	SetSlaveEntity(pEntity, pEntity->GetClass()->GetName());
	PrepareDude(true, m_dudeFlags);

	const auto pSlaveActor = GetSlaveActor();
	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		const auto pSlaveConsumer = pSlaveActor->GetEnergyConsumer();

		if (pSlaveConsumer)
			pHUD->TOSSetEnergyConsumer(pSlaveConsumer);
	}

	// Запросить права на изменение сущности, если вызов функции был из Flow Graph
	// Т.к. при вызове через FG не происходит передача прав мастеру на изменение контролируемого раба
	if (fromFG)
	{
		const auto pSynch = g_pTOSGame->GetMasterModule()->GetSynchronizer();
		assert(pSynch);
		if (pSynch)
		{
			NetDelegateAuthorityParams params1;
			params1.masterChannelId = m_pLocalDude->GetChannelId();
			params1.slaveId = m_pSlaveEntity->GetId();

			pSynch->RMISend(CTOSMasterSynchronizer::SvRequestDelegateAuthority(), params1, eRMI_ToServer);


			//const auto pSlaveEntClsCvar = gEnv->pConsole->GetCVar("tos_cl_SlaveEntityClass");
			//assert(pSlaveEntClsCvar);

			NetMasterAddingParams params2;
			params2.entityId = m_pLocalDude->GetEntityId();
			//params2.desiredSlaveClassName = pSlaveEntClsCvar->GetString();
			params2.desiredSlaveClassName = "Trooper";

			pSynch->RMISend(CTOSMasterSynchronizer::SvRequestMasterAdd(), params2, eRMI_ToServer);
		}
	}

	// Событие вызывает RMI, которая отправляется на сервер
	// Смотреть CTOSMasterModule::OnExtraGameplayEvent()
	auto void_dudeFlags = static_cast<void*>(new uint(m_dudeFlags));
	TOS_RECORD_EVENT(m_pSlaveEntity->GetId(), STOSGameEvent(eEGE_MasterClientOnStartControl, "", true, false, void_dudeFlags));
}

void CTOSMasterClient::StopControl(bool callFromFG /*= false*/)
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_MasterClientOnStopControl, callFromFG ? "from FG" : "", true));

	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		const auto pDudeConsumer = m_pLocalDude->GetEnergyConsumer();
		if (pDudeConsumer)
			pHUD->TOSSetEnergyConsumer(pDudeConsumer);
	}

	PrepareDude(false, m_dudeFlags);
	PreparePrevSlave(GetSlaveActor());
    ClearSlaveEntity();

	m_movementRequest.RemoveDeltaMovement();

	if (callFromFG)
	{
		const auto pSynch = g_pTOSGame->GetMasterModule()->GetSynchronizer();
		assert(pSynch);
		if (pSynch)
		{
			NetMasterAddingParams params2;
			params2.entityId = m_pLocalDude->GetEntityId();
			params2.desiredSlaveClassName = "<NOT_USED>";

			pSynch->RMISend(CTOSMasterSynchronizer::SvRequestMasterRemove(), params2, eRMI_ToServer);
		}
	}
}

bool CTOSMasterClient::SetSlaveEntity(IEntity* pEntity, const char* cls)
{
	assert(pEntity);
	m_pSlaveEntity = pEntity;
	const auto pSlaveActor = GetSlaveActor();
	assert(pSlaveActor);

	//string className = m_pSlaveEntity->GetClass()->GetName();
	//if (className == "Grunt" && pSlaveActor)
	//{
	//	CTOSPlayer* pPlayerGrunt = static_cast<CTOSPlayer*>(pSlaveActor);
	//	pPlayerGrunt->m_pPlayerInput.reset(new CPlayerInput(pPlayerGrunt));
	//	m_pPlayerGruntInput =  static_cast<CPlayerInput*>(pPlayerGrunt->m_pPlayerInput.get());
	//}
	//else
	//	m_pPlayerGruntInput = nullptr;

	TOS_RECORD_EVENT(m_pSlaveEntity->GetId(), STOSGameEvent(eEGE_MasterClientOnSetSlave, "", true));
	return true;
}

void CTOSMasterClient::ClearSlaveEntity()
{
	const auto pSlaveActor = GetSlaveActor();
	if (!pSlaveActor)
		return;
	// 21/10/2023, 14:40
	// В сетевой игре раб будет удаляться только при переходе в режим зрителя или выходе мастера из игры.
	// Во всех остальных случаях раб удаляться не будет.
	//pSlaveActor->NetMarkMeSlave(false);
	//m_pLocalDude->NetMarkMeMaster(false);

	m_pSlaveEntity = nullptr;
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_MasterClientOnClearSlave, "", true));
}

void CTOSMasterClient::UpdateView(SViewParams& viewParams) const
{
    assert(m_pSlaveEntity);
	if (!m_pSlaveEntity)
		return;

    viewParams.position = m_pSlaveEntity->GetWorldPos();

    static float currentFov = -1.0f;

	// Copied from ViewThirdPerson() in PlayerView.cpp
    static Vec3 target;
    static Vec3 current;

	Vec3 offsetX(0, 0, 0); //= pAlien->GetViewRotation().GetColumn0() * current.x;
	Vec3 offsetY(0, 0, 0);
    Vec3 offsetZ(0, 0, 0); //= pAlien->GetViewRotation().GetColumn2() * current.z;

    if (target)
    {
        current = target;
        Interpolate(current, target, 5.0f, viewParams.frameTime);
    }

    const EntityId controlledId = m_pSlaveEntity->GetId();
	const auto pControlledActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(controlledId));

    if (pControlledActor)
    {
        pControlledActor->UpdateMasterView(viewParams, offsetX, offsetY, offsetZ, target, current, currentFov);

		// Массив для хранения сущностей, которые следует пропустить при трассировке луча
		IPhysicalEntity* pSkipEntities[10];
		int nSkip = 0; // Количество сущностей для пропуска

		IItem* pItem = pControlledActor->GetCurrentItem();
		if (pItem)
		{
			const auto pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
			if (pWeapon)
			{
				// Получение сущностей для пропуска от оружия
				nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
			}
		}

		// Расчет длины смещения по оси Y
		const float oldLen = offsetY.len();

		// Вычисление начальной позиции для трассировки луча
		const Vec3 eyeOffsetView = pControlledActor->GetStanceInfo(pControlledActor->GetStance())->viewOffset;
		const Vec3 start = (pControlledActor->GetBaseMtx() * eyeOffsetView + viewParams.position + offsetX);

		// Определение безопасного расстояния от стен для камеры
		const float wallSafeDistance = 0.3f;

		// Создание сферы для трассировки луча
		primitives::sphere sphere;
		sphere.center = start;
		sphere.r = wallSafeDistance;

		// Переменная для хранения информации о контакте
		geom_contact* pContact = nullptr;

		// Выполнение трассировки луча в физическом мире
		const float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(
			sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
			&pContact, 0, rwi_stop_at_pierceable, nullptr, nullptr, 0, pSkipEntities, nSkip);

		// Обработка результатов трассировки луча
		if (hitDist > 0 && pContact)
		{
			// Корректировка смещения, если было обнаружено столкновение
			offsetY = pContact->pt - start;
			if (offsetY.len() > wallSafeDistance)
			{
				offsetY -= offsetY.GetNormalized() * wallSafeDistance;
			}
			// Корректировка позиции камеры на основе расстояния до столкновения
			current.y = current.y * (hitDist / oldLen);
		}

		// Обновление позиции камеры с учетом новых смещений
		viewParams.position += (offsetX + offsetY + offsetZ);
    }

	viewParams.rotation = m_pLocalDude->GetViewQuatFinal();
}

void CTOSMasterClient::PrepareDude(const bool toStartControl, const uint dudeFlags) const
{
	assert(m_pLocalDude);

	CNanoSuit* pSuit = m_pLocalDude->GetNanoSuit();
    assert(pSuit);

	const auto pHUD = g_pGame->GetHUD();

	//Fix the non-resetted Dude player movement after controlling the actor;
	if (m_pLocalDude->GetPlayerInput())
		m_pLocalDude->GetPlayerInput()->Reset();

	const auto pSynch = g_pTOSGame->GetMasterModule()->GetSynchronizer();
    assert(pSynch);

	if (toStartControl)
    {
        pSynch->RMISend(
            CTOSMasterSynchronizer::SvRequestSaveMCParams(), 
            NetMasterIdParams(m_pLocalDude->GetEntityId()), 
            eRMI_ToServer);

        m_pLocalDude->ResetScreenFX();

		auto pAI = m_pLocalDude->GetEntity()->GetAI();
		if (pAI)
			TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);

        if (dudeFlags & TOS_DUDE_FLAG_DISABLE_SUIT)
        {
			if (pSuit)
			{
				pSuit->SetMode(NANOMODE_DEFENSE);
				pSuit->SetModeDefect(NANOMODE_CLOAK, true);
				pSuit->SetModeDefect(NANOMODE_SPEED, true);
				pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
			}
        }

        if (dudeFlags & TOS_DUDE_FLAG_ENABLE_ACTION_FILTER)
        {
			g_pGameActions->FilterMasterControlSlave()->Enable(true);
        }

		//if (dudeFlags & TOS_DUDE_FLAG_HIDE_MODEL)
		//{
		m_pLocalDude->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
		//}

		if (dudeFlags & TOS_DUDE_FLAG_BEAM_MODEL)
		{
			const Vec3 slavePos = m_pSlaveEntity->GetWorldPos();
			const Quat slaveRot = m_pSlaveEntity->GetWorldRotation();

			m_pLocalDude->GetEntity()->SetWorldTM(Matrix34::CreateTranslationMat(slavePos), 0);

			// Привязка НЕ работает от клиента к серверу без исп. RMI
			//m_pSlaveEntity->AttachChild(m_pLocalDude->GetEntity(), ENTITY_XFORM_USER | IEntity::ATTACHMENT_KEEP_TRANSFORMATION);

			CTOSActor::NetAttachChild params;
			params.flags = ENTITY_XFORM_USER | IEntity::ATTACHMENT_KEEP_TRANSFORMATION;
			params.id = m_pLocalDude->GetEntityId();
			GetSlaveActor()->GetGameObject()->InvokeRMI(CTOSActor::SvRequestAttachChild(), params, eRMI_ToServer);

			SAnimatedCharacterParams anparams = m_pLocalDude->GetAnimatedCharacter()->GetParams();
			anparams.flags &= ~eACF_EnableMovementProcessing;
			anparams.flags |= eACF_NoLMErrorCorrection;
			m_pLocalDude->GetAnimatedCharacter()->SetParams(anparams);

			m_pLocalDude->SetViewRotation(slaveRot);
			//m_pLocalDude->SetAngles(Ang3(slaveRot));
		}

		IInventory* pInventory = m_pLocalDude->GetInventory();
		if (pInventory)
		{
			pInventory->HolsterItem(true);
			pInventory->RemoveAllItems();

			//if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
			//{
				//const string itemClassName = "Binoculars";

				//pClassRegistry->IteratorMoveFirst();
				//const IEntityClass* pEntityClass = pClassRegistry->FindClass(itemClassName);

				//if (pEntityClass)
					//g_pGame->GetIGameFramework()->GetIItemSystem()->GiveItem(pActor, itemClassName, false, false, false);
			//}
		}

        //if (dudeFlags & TOS_DUDE_FLAG_CLEAR_INVENTORY)
        //{
        //    //TODO Сохранение инвентаря Dude
        //}

		//g_pGameCVars->hud_enableAlienInterference = 0;
        //m_pLocalDude->ClearInterference();
        //gEnv->pConsole->GetCVar("hud_enableAlienInterference")->ForceSet("0");

        if (pHUD)
        {
            //LoadHUD(true); deprecated
            //m_pAbilitiesSystem->InitHUD(true);			
            //m_pAbilitiesSystem->ShowHUD(true);
            //m_pAbilitiesSystem->UpdateHUD();
            //m_pAbilitiesSystem->ReloadHUD();

            //SetAmmoHealthHUD();

            //g_pGame->GetHUD()->UpdateHealth(m_pControlledActor);
            //g_pGame->GetHUD()->m_animPlayerStats.Reload(true);

			const auto pHUDCrosshair = pHUD->GetCrosshair();
			pHUDCrosshair->SetOpacity(1.0f);
			pHUDCrosshair->SetCrosshair(g_pGameCVars->hud_crosshair);
        }

        if (!gEnv->bEditor)
        {
            // fix "Pure function error" 	

            /*CGameRules* pGR = g_pGame->GetGameRules();
            if (pGR && !m_isHitListener)
            {
                m_isHitListener = true;
                pGR->AddHitListener(this);
            }*/
        }
    }
    else
    {
		if (!gEnv->bMultiplayer)
		{
			pSynch->RMISend(
				CTOSMasterSynchronizer::SvRequestApplyMCSavedParams(),
				NetMasterIdParams(m_pLocalDude->GetEntityId()),
				eRMI_ToServer);
		}

		auto pAI = m_pLocalDude->GetEntity()->GetAI();
		if (pAI)
			TOS_AI::SendEvent(pAI, AIEVENT_ENABLE);

        if (dudeFlags & TOS_DUDE_FLAG_ENABLE_ACTION_FILTER)
        {
			g_pGameActions->FilterMasterControlSlave()->Enable(false);
        }

		bool zeus = m_pLocalDude->IsZeus();
		if (!zeus)
		{
			m_pLocalDude->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(false), eRMI_ToServer);
		}

		if (dudeFlags & TOS_DUDE_FLAG_BEAM_MODEL)
		{
			m_pLocalDude->GetEntity()->DetachThis(IEntity::ATTACHMENT_KEEP_TRANSFORMATION, 0);

			SAnimatedCharacterParams params = m_pLocalDude->GetAnimatedCharacter()->GetParams();
			params.flags |= eACF_EnableMovementProcessing;
			params.flags &= ~eACF_NoLMErrorCorrection;

			m_pLocalDude->GetAnimatedCharacter()->SetParams(params);
		}

        //m_pLocalDude->InitInterference();
		//gEnv->pConsole->GetCVar("hud_enableAlienInterference")->ForceSet("1");
        //g_pGameCVars->hud_enableAlienInterference = 1;
		//m_pLocalDude->ResetScreenFX();
		//gEnv->pSystem->GetI3DEngine()->SetPostEffectParam("AlienInterference_Amount", 0.0f);
		//SAFE_HUD_FUNC(StartInterference(0, 0, 100.0f, 3.f));

        if (m_pLocalDude->IsThirdPerson())
            m_pLocalDude->ToggleThirdPerson();

        // Выполнено - Нужно написать функцию для отображения дружественного перекрестия
		// Выполнено - Нужно написать функцию для смены имени текущего оружия

		if (pHUD)
		{
			const auto pHUDCrosshair = pHUD->GetCrosshair();
			pHUDCrosshair->ShowFriendCross(false);
		}
        SAFE_HUD_FUNC(TOSSetWeaponName(""))

        //g_pControlSystem->GetSquadSystem()->AnySquadClientLeft();

        //auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(m_pLocalDude, true);
        //if (pSquad && pSquad->GetLeader() != nullptr)
        //    pSquad->OnPlayerAdded();

        //Clean the OnUseData from player .lua script
        IScriptTable* pTable = m_pLocalDude->GetEntity()->GetScriptTable();
        if (pTable)
        {
            const ScriptAnyValue value = 0;
            Script::CallMethod(pTable, "SetOnUseData", value, value);
        }

        if (pSuit)
        {
	        // ReSharper disable once CppInconsistentNaming
	        const auto dudeHP = m_pLocalDude->GetHealth();
	        const auto spectatorMode = m_pLocalDude->GetSpectatorMode();
			const auto inSpectatorMode = spectatorMode > CActor::eASM_None && spectatorMode < CActor::eASM_Cutscene;
			
            if (dudeHP > 0 || inSpectatorMode)
            {
				pSuit->Reset(m_pLocalDude);
            }

            if (dudeFlags & TOS_DUDE_FLAG_DISABLE_SUIT)
            {
				if (dudeHP > 0 || inSpectatorMode)
				{
					pSuit->SetModeDefect(NANOMODE_CLOAK, false);
					pSuit->SetModeDefect(NANOMODE_SPEED, false);
					pSuit->SetModeDefect(NANOMODE_STRENGTH, false);

					pSuit->ActivateMode(NANOMODE_CLOAK, true);
					pSuit->ActivateMode(NANOMODE_SPEED, true);
					pSuit->ActivateMode(NANOMODE_STRENGTH, true);
				}
            }

            if (g_pGame->GetHUD())
            {
                //TODO: 10/11/2023, 07:35 Нужно написать функции, меняющие интерфейс жизней и инвентаря
                //SetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx");
                //SetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx");

				SAFE_HUD_FUNC(TOSSetAmmoHealthHUD(m_pLocalDude, "Libs/UI/HUD_AmmoHealthEnergySuit.gfx"));
				SAFE_HUD_FUNC(TOSSetInventoryHUD(m_pLocalDude, "Libs/UI/HUD_WeaponSelection.gfx"));

                //m_animScoutFlyInterface.Unload();

                //switch (pSuit->GetMode())
                //{
                //case NANOMODE_DEFENSE:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Armor");
                //    break;
                //case NANOMODE_SPEED:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Speed");
                //    break;
                //case NANOMODE_STRENGTH:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Strength");
                //    break;
                //case NANOMODE_CLOAK:
                //    g_pGame->GetHUD()->m_animPlayerStats.Invoke("setMode", "Cloak");
                //    break;
                //case NANOMODE_INVULNERABILITY:
                //case NANOMODE_DEFENSE_HIT_REACTION:
                //case NANOMODE_LAST:
                //    break;
                //}
            }

        }

        //if (dudeFlags & TOS_DUDE_FLAG_CLEAR_INVENTORY)
        //{
        //    //TODO Загрузка инвентаря
        //}

		SActorParams* pParams = m_pLocalDude->GetActorParams();
		pParams->vLimitRangeH = 0;
		pParams->vLimitRangeV = pParams->vLimitRangeVDown = pParams->vLimitRangeVUp = 0;

    }
}

bool CTOSMasterClient::PrepareNextSlave(CTOSActor* pNextActor) const
{
	if (!pNextActor)
		return false;

	//AI
	/////////////////////////////////////////////
	auto pAI = pNextActor->GetEntity()->GetAI();
	if (!pAI)
		return false;

	auto pAIActor = pAI->CastToIAIActor();
	if (!pAIActor)
		return false;

    //Save AI values
    AgentParameters params = pAI->CastToIAIActor()->GetParameters();

    //Re-register controlled actor in AI System as AI Player
    if (pAI->GetAIType() == AIOBJECT_PUPPET)
	{
		TOS_AI::RegisterAI(pNextActor->GetEntity(), true);
	}
	
	//Restore AI values to new ai pointer
    pAI = pNextActor->GetEntity()->GetAI();
    if (!pAI)
		return false;

	pAIActor = pAI->CastToIAIActor();
	if (!pAIActor)
		return false;

	pAIActor->SetParameters(params);

	//Weapons
	/////////////////////////////////////////////
	if (CWeapon* pWeapon = GetCurrentWeapon(pNextActor))
	{
		if (pWeapon->IsFiring())
			pWeapon->StopFire();
	}
}

bool CTOSMasterClient::PreparePrevSlave(CTOSActor* pPrevActor) const
{
	if (!pPrevActor)
		return false;

	if (pPrevActor->GetHealth() <= 0)
		return false;

    if (CWeapon* pWeapon = GetCurrentWeapon(pPrevActor))
    {
        if (pWeapon->IsFiring())
            pWeapon->StopFire();
    }

    if (const IAIObject* pAI = pPrevActor->GetEntity()->GetAI())
    {
		if (pAI->GetAIType() == AIOBJECT_PLAYER)
		{
			TOS_AI::RegisterAI(pPrevActor->GetEntity(), false);
		}
    }

    return true;
}
