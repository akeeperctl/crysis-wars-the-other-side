// ReSharper disable CppMsExtBindingRValueToLvalueReference
// ReSharper disable CppInconsistentNaming
// ReSharper disable CppVariableCanBeMadeConstexpr
#include "StdAfx.h"

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

#include "TheOtherSideMP/Actors/Aliens/TOSTrooper.h"
#include "TheOtherSideMP/Control/ControlSystem.h"
#include "TheOtherSideMP/Helpers/TOS_AI.h"
#include "TheOtherSideMP/HUD/TOSCrosshair.h"

#define ASSING_ACTION(pActor, actionId, checkActionId, func)\
if ((actionId) == (checkActionId))\
	func((pActor), (actionId), activationMode, value)\

CTOSMasterClient::CTOSMasterClient(CTOSPlayer* pPlayer)
	: m_pLocalDude(pPlayer),
	m_pSlaveEntity(nullptr),
	m_dudeFlags(0)
	//m_pWorldCamera(&gEnv->pSystem->GetViewCamera())
{
	assert(pPlayer);

    // в редакторе вылетает
  //  if (!gEnv->bEditor)
  //  {
		//m_pHUDCrosshair = g_pGame->GetHUD()->GetCrosshair();
		//assert(m_pHUDCrosshair);
  //  }

    m_deltaMovement.zero();

	if (gEnv->bClient)
	{
		//const char* clientChannelName = g_pGame->GetIGameFramework()->GetClientChannel()->GetName();
		//const char* netChName = _player->GetEntity()->;

		//CryLogAlways(" ");
		//CryLogAlways("[C++][CallConstructor][CTOSMasterClient] Player: %s, ClientChName: %s",
		//	_player->GetEntity()->GetName(), clientChannelName);
		//[C++][CallConstructor][CTOSMasterClient] Player: Akeeper, ClientChName: lmlicenses.wip4.adobe.com:64100

		//g_pTOSGame->GetModuleMasterSystem()->MasterAdd(m_pLocalDude->GetEntity());

		//InvokeRMI(ClTempRadarEntity(), params, eRMI_ToClientChannel, GetChannelId(*it));

		//auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
		//assert(pSender);

		//MasterAddingParams params;
		//params.entityId = m_pLocalDude->GetEntityId();

		//pSender->RMISend(CTOSMasterRMISender::SvRequestMasterAdd(), params, eRMI_ToServer);
	}
}

CTOSMasterClient::~CTOSMasterClient()
{
	//g_pTOSGame->GetModuleMasterSystem()->MasterRemove(m_pLocalDude->GetEntity());

	// delete this;

	//Case 1 not work
	//if (gEnv->bClient)
	//{
	//	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();
	//	assert(pSender);

	//	MasterAddingParams params;
	//	params.entityId = m_pLocalDude->GetEntityId();

	//	pSender->RMISend(CTOSMasterRMISender::SvRequestMasterRemove(), params, eRMI_ToServer);
	//}
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

	ASSING_ACTION(pSlaveActor, action, rGA.attack1, OnActionAttack);
	ASSING_ACTION(pSlaveActor, action, rGA.special, OnActionSpecial);// it is melee
	ASSING_ACTION(pSlaveActor, action, rGA.moveforward, OnActionMoveForward);
	ASSING_ACTION(pSlaveActor, action, rGA.moveback, OnActionMoveBack);
	ASSING_ACTION(pSlaveActor, action, rGA.moveleft, OnActionMoveLeft);
	ASSING_ACTION(pSlaveActor, action, rGA.moveright, OnActionMoveRight);
	ASSING_ACTION(pSlaveActor, action, rGA.jump, OnActionJump);
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool CTOSMasterClient::OnActionAttack(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value)
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

bool CTOSMasterClient::OnActionSpecial(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value)
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

bool CTOSMasterClient::OnActionMoveForward(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value)
{
	m_deltaMovement.x = m_deltaMovement.z = 0;
	m_deltaMovement.y = value * 2.0f - 1.0f;

	//pActor->ApplyMasterMovement(delta);
    m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionMoveBack(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value)
{
	m_deltaMovement.x = m_deltaMovement.z = 0;
    m_deltaMovement.y = -(value * 2.0f - 1.0f);

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionMoveLeft(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value)
{
	m_deltaMovement.x = -(value * 2.0f - 1.0f);
    m_deltaMovement.y = m_deltaMovement.z = 0;

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

    return true;
}

bool CTOSMasterClient::OnActionMoveRight(CTOSActor* pActor, const ActionId& actionId, int activationMode, const float value)
{
	m_deltaMovement.x = value * 2.0f - 1.0f;
	m_deltaMovement.y = m_deltaMovement.z = 0;

	//pActor->ApplyMasterMovement(delta);
	m_movementRequest.AddDeltaMovement(m_deltaMovement);

	return true;
}

bool CTOSMasterClient::OnActionJump(CTOSActor* pActor, const ActionId& actionId, const int activationMode, const float value)
{
    if (activationMode == eAAM_OnPress && value > 0.0f)
    {
		m_movementRequest.SetJump();
    }
    else if (activationMode == eAAM_OnRelease)
    {
        if (m_movementRequest.ShouldJump())
        {
			m_movementRequest.ClearJump();
        }
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

	// В сетевой игре отправка запроса отсюда работает прекрасно
	// В одиночной игре не отправка запроса не работает
	SendMovementRequest(pController);
}

void CTOSMasterClient::Update(float frametime)
{
	//m_pWorldCamera = &gEnv->pSystem->GetViewCamera();
    //CRY_ASSERT_MESSAGE(m_pWorldCamera, "[CTOSMasterClient] m_pWorldCamera pointer is null");

	const auto cam = gEnv->pSystem->GetViewCamera();
	m_cameraInfo.viewDir = cam.GetMatrix().GetColumn1() * 1000.0f;
	m_cameraInfo.worldPos = cam.GetMatrix().GetTranslation();
    m_cameraInfo.lookPointPos = m_cameraInfo.worldPos + m_cameraInfo.viewDir;

	constexpr auto            rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
	static constexpr unsigned entityFlags =
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
	if (gEnv->pConsole->GetCVar("tos_sv_mc_LookDebugDraw")->GetIVal() > 0)
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

	if (!gEnv->bMultiplayer)
		SendMovementRequest(pController);
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

void CTOSMasterClient::SendMovementRequest(IMovementController* pController)
{
	assert(pController);
	if (!pController)
		return;

	m_movementRequest.SetLookTarget(m_lookfireInfo.lookTargetPos);
	m_movementRequest.SetFireTarget(m_lookfireInfo.fireTargetPos);

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
/*
void CTOSMasterClient::ProcessMeleeDamage() const
{
    //TODO:
    //Получить цель в прицеле targetId
    //Нанести урон


	const IEntity* pEntityTarget = gEnv->pEntitySystem->GetEntity(m_meleeInfo.targetId);
    CRY_ASSERT_MESSAGE(pEntityTarget, "[CTOSMasterClient::ProcessMeleeDamage] pEntityTarget equal nullptr");
	if (!pEntityTarget)
		return;

    const auto targetId = pEntityTarget->GetId();

	const auto pWeapon = GetCurrentWeapon(GetSlaveActor());
	CRY_ASSERT_MESSAGE(pWeapon, "[CTOSMasterClient::ProcessMeleeDamage] pWeapon equal nullptr");
	if (!pWeapon)
		return;

	const IActor* pActorTarget = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(targetId);
	const IVehicle* pVehicleTarget = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(targetId);

    const string slaveCls = m_pSlaveEntity->GetClass()->GetName();

	if (slaveCls == "Trooper")
	{
		const Vec3 slavePos = m_pSlaveEntity->GetWorldPos();
		const Vec3 toTargetDir(m_meleeInfo.targetPos - slavePos);
		constexpr float attackRadius = 2.5f;

		const float dist = toTargetDir.GetLength();
		if (dist < attackRadius)
		{
			//Vec3 dirx = GetEntity()->GetWorldTM().GetColumn(0);
			//Vec3 diry = GetEntity()->GetWorldTM().GetColumn(1);
			//Vec3 dirz = GetEntity()->GetWorldTM().GetColumn(2);
			//Vec3 damageBoxOffset(0, 0, 0);

			const float damage = 80.0f; //g_pGameCVars->g_trooperMeleeDamage;
			const float damageMultiplier = 1.2f;//g_pGameCVars->g_trooperMeleeDamageMultiplier;

			bool isHaveNanosuit = false;

			SmartScriptTable meleeTable;
			IScriptTable* pTargetTable = pEntityTarget->GetScriptTable();
			if (pTargetTable)
			{
				SmartScriptTable propertiesTable;
				if (pTargetTable->GetValue("Properties", propertiesTable))
					propertiesTable->GetValue("bNanoSuit", isHaveNanosuit);
			}

			//pos.x += dirx.x * damageBoxOffset.x + diry.x * damageBoxOffset.y + dirz.x * damageBoxOffset.z;
			//pos.y += dirx.y * damageBoxOffset.x + diry.y * damageBoxOffset.y + dirz.y * damageBoxOffset.z;
			//pos.z += dirx.z * damageBoxOffset.x + diry.z * damageBoxOffset.y + dirz.z * damageBoxOffset.z;

			//const EntityId slaveId = m_pSlaveEntity->GetId();

			//HitInfo hit;
			//hit.shooterId = slaveId;
			//hit.weaponId = slaveId;
			//hit.targetId = targetId;
			//hit.type = g_pGame->GetGameRules()->GetHitTypeId("melee");

			//if (pVehicleTarget)
			//{
			//	const float vehicleMultiplier = 0.35f;
			//	hit.SetDamage(damage * damageMultiplier * vehicleMultiplier);
			//}
			//else if (pActorTarget)
			//{
			//	if (!isHaveNanosuit)
			//	{
			//		constexpr float noNanosuitMultiplier = 4.0f;
			//		hit.SetDamage(damage * damageMultiplier * noNanosuitMultiplier);
			//	}
			//	else if (isHaveNanosuit)
			//	{
			//		hit.SetDamage(damage * damageMultiplier);
			//	}
			//}

			//For melee point impulse
			//IFireMode* meleeFM = pWeapon->GetMeleeFireMode();
			//if (meleeFM)
				//meleeFM->NetShootEx(m_meleeInfo.targetPos, m_cameraInfo.viewDir, Vec3(0, 0, 0), Vec3(0, 0, 0), 0, 0);

			//For dealing melee damage to target
			//g_pGame->GetGameRules()->ClientHit(hit);

			//Попробуем по-другому
			//pWeapon->RequestMeleeAttack(true, m_meleeInfo.targetPos, m_cameraInfo.viewDir, 0); //TODO: не добавляется melee firemode у оружия трупера


			//SNetCamShakeParams params;
			//params.angle = 45;
			//params.duration = 0.3f;
			//params.frequency = 0.13f;
			//params.shift = 0;
			//params.pos = Vec3{ 0,0,0 };

			//if (pActorTarget && pActorTarget->IsPlayer())
			//{
			//	if (gEnv->bClient)
			//		pActorTarget->GetGameObject()->InvokeRMI(SvRequestCameraShake(), params, eRMI_ToServer);
			//}
		}
	}

}
*/

//void CTOSMasterClient::Update(IEntity* pEntity)
//{
//	if (m_pSlaveEntity != nullptr && pEntity == m_pSlaveEntity)
//	{
//		//m_deltaMovement.zero();
//	}
//		
//}

void CTOSMasterClient::StartControl(IEntity* pEntity, uint dudeFlags /*= 0*/, bool callFromFG /*= false*/)
{
	assert(pEntity);

    m_dudeFlags = dudeFlags;

	// 13.01.2023 Akeeper: Я изменил порядок вызова функций.
	// Подписанные номера это старая последовательность вызова по возрастанию
    SetSlaveEntity(pEntity, pEntity->GetClass()->GetName());//2
	PrepareDude(true, m_dudeFlags);//1

	const auto pSlaveActor = GetSlaveActor();
	const auto pHUD = g_pGame->GetHUD();

	if (pHUD)
	{
		const auto pSlaveConsumer = pSlaveActor->GetEnergyConsumer();

		if (pSlaveConsumer)
			pHUD->TOSSetEnergyConsumer(pSlaveConsumer);
	}

	const auto pGameRules = g_pGame->GetGameRules();
	if (pGameRules)
	{
		const int dudeTeam = pGameRules->GetTeam(m_pLocalDude->GetEntityId());
		//pGameRules->ChangeTeam(pSlaveActor, dudeTeam);

		if (gEnv->bClient)
		{
			CGameRules::ChangeTeamParams params;
			params.entityId = m_pSlaveEntity->GetId();
			params.teamId = dudeTeam;

			pGameRules->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::SvRequestForceSetTeam(), params, eRMI_ToServer, params.entityId);
		}
		else
		{
			//pGameRules->SetTeam(dudeTeam, m_pSlaveEntity->GetId());
		}
	}

	// Запросить права на изменение сущности, если вызов функции был из Flow Graph
	// Т.к. при вызове через FG не происходит передача прав мастеру на изменение контролируемого раба
	if (callFromFG)
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
	TOS_RECORD_EVENT(m_pSlaveEntity->GetId(), STOSGameEvent(eEGE_MasterClientOnStartControl, "", true));
}

void CTOSMasterClient::StopControl(bool callFromFG /*= false*/)
{
	TOS_RECORD_EVENT(0, STOSGameEvent(eEGE_MasterClientOnStopControl, "", true));

	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		const auto pDudeConsumer = m_pLocalDude->GetEnergyConsumer();
		if (pDudeConsumer)
			pHUD->TOSSetEnergyConsumer(pDudeConsumer);
	}

	PrepareDude(false, m_dudeFlags);
    ClearSlaveEntity();

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

	pSlaveActor->NetMarkMeSlave(true);

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
	pSlaveActor->NetMarkMeSlave(false);

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
	const string   controlledCls = m_pSlaveEntity->GetClass()->GetName();

	const auto pControlledActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(controlledId));
    if (pControlledActor)
    {
		/*
    	if (controlledCls == "Hunter")
        {
            target(g_pGameCVars->ctrl_hrTargetx, g_pGameCVars->ctrl_hrTargety, g_pGameCVars->ctrl_hrTargetz);
            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1() *
                g_pGameCVars->ctrl_hrForwardOffset;
            currentFov = g_pGameCVars->ctrl_hrFov;
        }
        else if (controlledCls == "Scout")
        {
            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
            target(g_pGameCVars->ctrl_scTargetx, g_pGameCVars->ctrl_scTargety, g_pGameCVars->ctrl_scTargetz);
            currentFov = g_pGameCVars->ctrl_scFov;
        }
        else if (controlledCls == "Drone")
        {
            target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
            currentFov = g_pGameCVars->ctrl_trFov;
        }
        else if (controlledCls == "Pinger")
        {
            target(g_pGameCVars->ctrl_pgTargetx, g_pGameCVars->ctrl_pgTargety, g_pGameCVars->ctrl_pgTargetz);
            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y + alienWorldMtx.GetColumn1();
            currentFov = g_pGameCVars->ctrl_pgFov;
        }
        else if (controlledCls == "Alien")
        {
            target(g_pGameCVars->ctrl_alTargetx, g_pGameCVars->ctrl_alTargety, g_pGameCVars->ctrl_alTargetz);
            offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
            currentFov = g_pGameCVars->ctrl_alFov;
        }
        */

        pControlledActor->UpdateMasterView(viewParams, offsetX, offsetY, offsetZ, target, current, currentFov);



        // Старт кода, который я скопипастил из ControlClient.cpp

    	//Get skip entities
	    IPhysicalEntity* pSkipEntities[10];  // NOLINT(modernize-avoid-c-arrays)
	    int nSkip = 0;
	    IItem* pItem = pControlledActor->GetCurrentItem();
	    if (pItem)
	    {
		    const auto pWeapon = dynamic_cast<CWeapon*>(pItem->GetIWeapon());
	        if (pWeapon)
	            nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
	    }

	    const float oldLen = offsetY.len();

	    // Ray cast to camera with offset position to check colliding
	    const Vec3 eyeOffsetView = pControlledActor->GetStanceInfo(pControlledActor->GetStance())->viewOffset;
	    const Vec3 start = (pControlledActor->GetBaseMtx() * eyeOffsetView + viewParams.position + offsetX);
	    // +offsetZ;// + offsetX;// +offsetZ;

	    constexpr float wallSafeDistance = 0.3f; // how far to keep camera from walls

	    primitives::sphere sphere;
	    sphere.center = start;
	    sphere.r = wallSafeDistance;

	    geom_contact* pContact = nullptr;
	    const float hitDist = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(
	        sphere.type, &sphere, offsetY, ent_static | ent_terrain | ent_rigid | ent_sleeping_rigid,
	        &pContact, 0, rwi_stop_at_pierceable, nullptr, nullptr, 0, pSkipEntities, nSkip);

	    if (hitDist > 0 && pContact /*&& !m_pAbilitiesSystem->trooper.isCeiling*/)
	    {
	        offsetY = pContact->pt - start;
	        if (offsetY.len() > 0.3f)
	        {
	            offsetY -= offsetY.GetNormalized() * 0.3f;
	        }
	        current.y = current.y * (hitDist / oldLen);
	    }

        viewParams.position += (offsetX + offsetY + offsetZ);

        // Конец кода, который я скопипастил из ControlClient.cpp
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

		if (dudeFlags & TOS_DUDE_FLAG_HIDE_MODEL)
		{
			m_pLocalDude->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
		}

		if (dudeFlags & TOS_DUDE_FLAG_BEAM_MODEL)
		{
			const Vec3 slavePos = m_pSlaveEntity->GetWorldPos();
			const Quat slaveRot = m_pSlaveEntity->GetWorldRotation();

			m_pLocalDude->GetEntity()->SetWorldTM(Matrix34::CreateTranslationMat(slavePos), 0);
			m_pLocalDude->GetEntity()->SetRotation(slaveRot);

			// Привязка работает от клиента к серверу без исп. RMI
			m_pSlaveEntity->AttachChild(m_pLocalDude->GetEntity(), IEntity::ATTACHMENT_KEEP_TRANSFORMATION);
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
		pSynch->RMISend(
			CTOSMasterSynchronizer::SvRequestApplyMCSavedParams(),
			NetMasterIdParams(m_pLocalDude->GetEntityId()),
			eRMI_ToServer);

        if (dudeFlags & TOS_DUDE_FLAG_ENABLE_ACTION_FILTER)
        {
			g_pGameActions->FilterMasterControlSlave()->Enable(false);
        }

		if (dudeFlags & TOS_DUDE_FLAG_HIDE_MODEL)
		{
			m_pLocalDude->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(false), eRMI_ToServer);
		}

		if (dudeFlags & TOS_DUDE_FLAG_BEAM_MODEL)
		{
			m_pLocalDude->GetEntity()->DetachThis(IEntity::ATTACHMENT_KEEP_TRANSFORMATION, 0);
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