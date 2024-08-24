#include "StdAfx.h"
#include "TOSTrooper.h"

#include "GameUtils.h"
#include "NetInputChainDebug.h"

#include "TheOtherSideMP/Extensions/EnergyСonsumer.h"
#include "TheOtherSideMP/Helpers/TOS_Console.h"
#include <TheOtherSideMP/Helpers/TOS_NET.h>
#include <CompatibilityAlienMovementController.h>
#include <utility>

CTOSTrooper::CTOSTrooper() {};

CTOSTrooper::~CTOSTrooper() {};

void CTOSTrooper::PostInit(IGameObject* pGameObject)
{
	CTrooper::PostInit(pGameObject);
	m_chargingJump = true;

	if (m_pEnergyConsumer)
	{
		m_pEnergyConsumer->SetRegenStartDelayMP(TOS_Console::GetSafeFloatVar("tos_tr_regen_energy_start_delay_mp"));
		m_pEnergyConsumer->SetRegenStartDelaySP(TOS_Console::GetSafeFloatVar("tos_tr_regen_energy_start_delay_sp"));
		m_pEnergyConsumer->SetRegenStartDelay20Boundary(TOS_Console::GetSafeFloatVar("tos_tr_regen_energy_start_delay_20boundary"));
		m_pEnergyConsumer->SetRechargeTimeSP(TOS_Console::GetSafeFloatVar("tos_tr_regen_energy_recharge_time_sp"));
		m_pEnergyConsumer->SetRechargeTimeMP(TOS_Console::GetSafeFloatVar("tos_tr_regen_energy_recharge_time_mp"));
	}

}

void CTOSTrooper::PostPhysicalize()
{
	CTOSAlien::PostPhysicalize();

	// Это фиксит проблему с инерцией и дерганой синхронизацией
	if (m_pAnimatedCharacter)
	{
		SAnimatedCharacterParams params = m_pAnimatedCharacter->GetParams();
		params.SetInertia(m_params.inertia, m_params.inertiaAccel);
		params.timeImpulseRecover = GetTimeImpulseRecover();
		params.flags |= eACF_EnableMovementProcessing | eACF_ZCoordinateFromPhysics | eACF_ConstrainDesiredSpeedToXY;
		m_pAnimatedCharacter->SetParams(params);
	}
}

void CTOSTrooper::Update(SEntityUpdateContext& ctx, const int updateSlot)
{
	const float regenStartDelay = m_pEnergyConsumer->GetRegenStartDelay();

	NETINPUT_TRACE(GetEntityId(), regenStartDelay);
	NETINPUT_TRACE(GetEntityId(), m_input.deltaMovement);
	NETINPUT_TRACE(GetEntityId(), m_input.viewDir);
	NETINPUT_TRACE(GetEntityId(), m_input.actions);
	NETINPUT_TRACE(GetEntityId(), m_input.viewVector);
	NETINPUT_TRACE(GetEntityId(), m_netBodyInfo.desiredSpeed);
	NETINPUT_TRACE(GetEntityId(), m_netBodyInfo.deltaMov);
	NETINPUT_TRACE(GetEntityId(), m_netBodyInfo.lookTarget);
	NETINPUT_TRACE(GetEntityId(), GetEntity()->GetWorldPos());
	NETINPUT_TRACE(GetEntityId(), m_stats.inAir);
	NETINPUT_TRACE(GetEntityId(), m_stats.onGround);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.bFreeFall);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.velocity);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.curVelocity);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.state);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.duration);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.prevInAir);
	NETINPUT_TRACE(GetEntityId(), m_jumpParams.remainingTime);
	NETINPUT_TRACE(GetEntityId(), m_lastTimeOnGround.GetSeconds());

	IEntityRenderProxy* pRenderProxy = (IEntityRenderProxy*)(GetEntity()->GetProxy(ENTITY_PROXY_RENDER));
	if ((pRenderProxy == nullptr) || !pRenderProxy->IsCharactersUpdatedBeforePhysics())
		PrePhysicsUpdate();

	CTrooper::Update(ctx, updateSlot);

	//IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();
	//if (pPhysEnt)
	//{
	//	pe_status_dynamics dynStat;
	//	pe_status_living livStat;

	//	int dynStatType = dynStat.type;
	//	memset(&dynStat, 0, sizeof(pe_status_dynamics));
	//	dynStat.type = dynStatType;

	//	int livStatType = livStat.type;
	//	memset(&livStat, 0, sizeof(pe_status_living));
	//	livStat.groundSlope = Vec3(0, 0, 1);
	//	livStat.type = livStatType;

	//	pPhysEnt->GetStatus(&dynStat);
	//	pPhysEnt->GetStatus(&livStat);

	//	NETINPUT_TRACE(GetEntityId(), dynStat.mass);
	//	NETINPUT_TRACE(GetEntityId(), dynStat.a);
	//	NETINPUT_TRACE(GetEntityId(), dynStat.v);
	//	NETINPUT_TRACE(GetEntityId(), livStat.bFlying);
	//	NETINPUT_TRACE(GetEntityId(), livStat.bStuck);
	//	NETINPUT_TRACE(GetEntityId(), livStat.timeFlying);
	//	NETINPUT_TRACE(GetEntityId(), livStat.velRequested);
	//	NETINPUT_TRACE(GetEntityId(), livStat.velUnconstrained);
	//	NETINPUT_TRACE(GetEntityId(), livStat.groundHeight);
	//}

	NETINPUT_TRACE(GetEntityId(), InZeroG());
	NETINPUT_TRACE(GetEntityId(), IsSlave());

	EAutoDisablePhysicsMode adpm = eADPM_WhenAIDeactivated; // Значение по умолчанию

	if (m_stats.isRagDoll || IsLocalSlave() || (gEnv->bMultiplayer && gEnv->bServer)) 
	{
		adpm = eADPM_Never;
	}
	else if (IsPlayer() || IsSlave()) 
	{
		adpm = eADPM_WhenInvisibleAndFarAway;
	}

	GetGameObject()->SetAutoDisablePhysicsMode(adpm);
	//~TheOtherSide
}

bool CTOSTrooper::NetSerialize(const TSerialize ser, const EEntityAspects aspect, const uint8 profile, const int flags)
{
	if (!CTrooper::NetSerialize(ser, aspect, profile, flags))
		return false;

	return true;
}

void CTOSTrooper::ProcessMovement(const float frameTime)
{
	//TheOtherSide
	// Обработка прыжка
	const auto pMovementController = static_cast<CCompatibilityAlienMovementController*>(GetMovementController());
	auto& currentRequest = pMovementController->GetCurrentMovementRequest();
	if (currentRequest.ShouldJump())
	{
		currentRequest.ClearJump();

		STOSSlaveStats* pSlaveStats = &GetSlaveStats();

		const float onGround = m_stats.onGround;

		if (onGround > 0.25f && IsSlave())
		{
			m_jumpParams.bTrigger = true;
			m_jumpParams.bUseLandAnim = true; // ПОФИКШЕН РАССИНХРОН bUseLandAnim == True + RMI
			m_jumpParams.duration = 0.4f; // подбиралось эмпирически. Через 0.4 сек переход из flying в approach landing
			
			const float	jumpPressDur = pSlaveStats->chargingJumpPressDur;
			const float	jumpForce = TOS_Console::GetSafeFloatVar("tos_tr_jump_force", 4000.0f);
			//const float	jumpForceAdd = TOS_Console::GetSafeFloatVar("tos_tr_double_jump_force", 4.0f);
			const float chargingTime = TOS_Console::GetSafeFloatVar("tos_tr_charging_jump_input_time", 0.20f);
			const float chargingMul = TOS_Console::GetSafeFloatVar("tos_tr_charged_jump_mul", 2.7f);
			//const float	finalOnceJumpForce = jumpPressDur > chargingTime ? jumpForce + jumpForceAdd : jumpForce;
			const float	mult = jumpPressDur > chargingTime ? chargingMul : 1.0f;

			const float doubleJumpCost = TOS_Console::GetSafeFloatVar("tos_tr_double_jump_energy_cost");
			const float energy = TOS_SAFE_GET_ENERGY(this);

			Vec3 jumpVec(0, 0, 0);

			const Vec3& upDir = GetEntity()->GetWorldTM().GetColumn(2);
			const Vec3& forwardDir = GetEntity()->GetWorldTM().GetColumn(1);
			const Vec3& rightDir = GetEntity()->GetWorldTM().GetColumn(0);

			pe_action_impulse impulse;

			// Первый прыжок, который от земли
			if (pSlaveStats->jumpCount == 0)
			{
				// прыжки, используя анимации и углы в итоге получается так, 
				//что приш. прыгает не туда куда хочет игрок
				m_jumpParams.bUseAdvancedStartAnim = true;
				m_jumpParams.bUseInstantJumping = true;

				pe_status_dynamics dynStat;
				const auto         pPhysEnt = GetEntity()->GetPhysics();
				pPhysEnt->GetStatus(&dynStat);

				//pSlaveStats->jumpCount++; временное

				const float gravityMagnitude = m_stats.gravity.len();
				float jumpTime = 0.0f;

				// Если сила тяжести не равна нулю, рассчитать время прыжка
				if (gravityMagnitude > 0.0f) 
				{
				    jumpTime = cry_sqrtf(2.0f * gravityMagnitude * jumpForce * mult) / gravityMagnitude - m_stats.inAir * 0.5f;
				}

				jumpVec.z = upDir.z * gravityMagnitude * jumpTime;
				//impulse.impulse = jumpVec;
				//pPhysEnt->Action(&impulse);
				m_jumpParams.velocity += jumpVec + dynStat.v;

				pSlaveStats->chargingJumpPressDur = 0.0f;
			}
			//else if (pSlaveStats->jumpCount > 0 && m_stats.inAir > 0.0f && energy > doubleJumpCost)
			//{
			//	// Двойной прыжок

			//	if (currentRequest.HasDeltaMovement() && !currentRequest.GetDeltaMovement().IsZero())
			//	{
			//		jumpVec += currentRequest.GetDeltaMovement().x * jumpForce * rightDir / 1.5f;
			//		jumpVec += currentRequest.GetDeltaMovement().y * jumpForce * forwardDir / 1.5f;
			//	}
			//	else
			//	{
			//		jumpVec = forwardDir * jumpForce; //300.f;
			//	}
			//	jumpVec.z = upDir.z * 2.5f; // 250.f;

			//	//TODO
			//	//NetSpawnParticleEffect("alien_special.Trooper.doubleJumpAttack");

			//	//TODO
			//	//SubEnergy(TROOPER_JUMP_ENERGY_COST);

			//	m_jumpParams.velocity += jumpVec;
			//	//m_pAnimatedCharacter->AddMovement(animCharRequest);

			//	TOS_SAFE_ADD_ENERGY(this, -doubleJumpCost);

			//	pSlaveStats->jumpCount = 0;

			//	//TODO
			//	//The controlled trooper cannot to do jump attack after double jump
			//	//m_trooper.canJumpMelee = false;
			//}

			if (IsLocalSlave())
			{
				GetGameObject()->InvokeRMI(CTOSActor::SvRequestTOSJump(), CActor::NoParams(), eRMI_ToServer);
			}
		}
	}

	CTrooper::ProcessMovement(frameTime);
	ProcessJumpFlyControl(m_moveRequest.velocity, frameTime);

	// Сброс параметров для следующего кадра
	m_jumpParams.bRelative = false;
	m_jumpParams.bUseInstantJumping = false;
	m_jumpParams.duration = 0.0f;
}

void CTOSTrooper::ProcessJumpFlyControl(const Vec3& move, const float frameTime)
{
	if (!move.IsZero())
	{
		Vec3 desiredVelocityClamped = m_input.deltaMovement;

		const float desiredVelocityMag = desiredVelocityClamped.GetLength();
		if (desiredVelocityMag > 1.0f)
			desiredVelocityClamped /= desiredVelocityMag;

		pe_action_move actionMove;
		actionMove.iJump = 2;

		//углы поворота добываются в функциях GetAngles в виде Radian. 
		//Для удобства работы с углами, специально для юзера они преобразуются в градусы/Degrees (0-360)  

		const Ang3 angles(GetViewMtx());
		Matrix33   baseMtxZ;
		baseMtxZ.SetRotationXYZ(angles);

		Vec3       actual = baseMtxZ * Vec3(0, 0, 0);
		const Vec3 goal   = baseMtxZ * desiredVelocityClamped;

		Interpolate(actual, goal, 5.0f, frameTime);

		actionMove.dir = actual;

		if (m_stats.speed <= 8.0f)
			GetEntity()->GetPhysics()->Action(&actionMove);

		//Matrix rotation debug
		const bool isDebugEnabled = false;
		if (isDebugEnabled)
		{
			static float c[] = {1, 1, 1, 1};
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 0, 255, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn0(), ColorB(0, 0, 255, 255));
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(255, 0, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn1(), ColorB(255, 0, 0, 255));
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 255, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn2(), ColorB(0, 255, 0, 255));
		}
	}
}

void CTOSTrooper::UpdateStats(float frameTime)
{
	CTrooper::UpdateStats(frameTime);

	const auto pPhysEnt = GetEntity()->GetPhysics();

	//if (gEnv->pSystem->IsDedicated())
	//{
	//	if (pPhysEnt)
	//	{
	//		// leipzig: force inactive (was active on ded. servers)
	//		pe_player_dynamics paramsGet;
	//		if (pPhysEnt->GetParams(&paramsGet))
	//		{
	//			if (!paramsGet.bActive && m_pAnimatedCharacter)
	//			{
	//				m_pAnimatedCharacter->ForceRefreshPhysicalColliderMode();
	//				m_pAnimatedCharacter->RequestPhysicalColliderMode(eColliderMode_Undefined, eColliderModeLayer_Game, "CTOSTrooper::UpdateStats");
	//			}
	//		}			
	//	}
	//}

	if (pPhysEnt)
	{
		pe_player_dynamics paramsGet;
		if (pPhysEnt->GetParams(&paramsGet))
		{
			const auto collideMode = GetAnimatedCharacter()->GetPhysicalColliderMode();
			const auto charParams  = GetAnimatedCharacter()->GetParams();

			NETINPUT_TRACE(GetEntityId(), paramsGet.bActive);
			NETINPUT_TRACE(GetEntityId(), paramsGet.bNetwork);
			NETINPUT_TRACE(GetEntityId(), paramsGet.collTypes);
			NETINPUT_TRACE(GetEntityId(), paramsGet.iRequestedTime);
			NETINPUT_TRACE(GetEntityId(), paramsGet.kAirControl);
			NETINPUT_TRACE(GetEntityId(), paramsGet.kAirResistance);
			NETINPUT_TRACE(GetEntityId(), paramsGet.kInertia);
			NETINPUT_TRACE(GetEntityId(), charParams.inertia);
			NETINPUT_TRACE(GetEntityId(), m_params.speedInertia);
			NETINPUT_TRACE(GetEntityId(), paramsGet.timeImpulseRecover);
			NETINPUT_TRACE(GetEntityId(), paramsGet.type);
			NETINPUT_TRACE(GetEntityId(), collideMode);
		}
	}
}

void CTOSTrooper::UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov)
{
	//CTrooper::UpdateMasterView(viewParams, offsetY, target, currentFov);

	//const Matrix33 alienWorldMtx(GetEntity()->GetWorldTM());

	//if (esSystem->trooper.isCeiling)
	//	target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety, g_pGameCVars->ctrl_trTargetz);
	//else
	//{
	//	target(g_pGameCVars->ctrl_trTargetx, g_pGameCVars->ctrl_trTargety,
	//		g_pGameCVars->ctrl_trTargetz - 2.f);
	//}
	//offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y; //Used by all aliens in this mod
	//currentFov = g_pGameCVars->ctrl_trFov;

	currentFov = 75.0f;
	target(0.7f, -2.8f, 1.75f);

	offsetX = GetViewRotation().GetColumn0() * current.x;
	offsetY = gEnv->pSystem->GetViewCamera().GetViewdir() * current.y;
	offsetZ = GetViewRotation().GetColumn2() * current.z;
}

bool CTOSTrooper::ApplyActions(int actions)
{
	CTOSAlien::ApplyActions(actions);

	return true;
}

//void CTOSTrooper::ProcessJump(const CMovementRequest& request)
//{
//	//throw std::logic_error("Функция не актуальна");
//
//	TOS_CHECK_CONSUMER_EXISTING(this);
//
//	//pe_action_impulse impulse;
//	SCharacterMoveRequest animCharRequest;
//	animCharRequest.jumping = true;
//	animCharRequest.type = eCMT_JumpInstant; //eCMT_JumpAccumulate;//eCMT_JumpInstant; //eCMT_Impulse //eCMT_JumpAccumulate;
//
//	Vec3 jumpVec(0, 0, 0);
//
//	const Vec3& upDir      = GetEntity()->GetWorldTM().GetColumn(2);
//	const Vec3& forwardDir = GetEntity()->GetWorldTM().GetColumn(1);
//	const Vec3& rightDir   = GetEntity()->GetWorldTM().GetColumn(0);
//
//	STOSSlaveStats* pSlaveStats = &GetSlaveStats();
//	const auto      pActorStats = GetActorStats();
//
//	if (pActorStats)
//	{
//		const float     onGround  = pActorStats->onGround;
//		const float		jumpPressDur = pSlaveStats->chargingJumpPressDur;
//		const float		jumpForce = 6.0f;
//		const float		finalOnceJumpForce = jumpPressDur > TOS_Console::GetSafeFloatVar("tos_tr_charging_jump_input_time") ? jumpForce + 4.0f : jumpForce;
//
//		const float doubleJumpCost = TOS_Console::GetSafeFloatVar("tos_tr_double_jump_energy_cost");
//		const float energy = TOS_SAFE_GET_ENERGY(this);
//
//		// Одиночный прыжок
//		if (onGround > 0.25f)
//		{
//			pSlaveStats->jumpCount++;
//			jumpVec.z = upDir.z * finalOnceJumpForce; //400.0f
//
//			//GetEntity()->GetPhysics()->Action(&impulse);
//			animCharRequest.velocity += jumpVec;
//			m_pAnimatedCharacter->AddMovement(animCharRequest);
//
//			pSlaveStats->chargingJumpPressDur = 0.0f;
//
//			//TODO
//			//NetPlayAnimAction("CTRL_JumpStart", false);
//		}
//		else if (pSlaveStats->jumpCount > 0 && pActorStats->inAir > 0.0f && energy > doubleJumpCost)
//		{
//			// Двойной прыжок
//
//			if (request.HasDeltaMovement() && !request.GetDeltaMovement().IsZero())
//			{
//				//jumpVec += request.GetDeltaMovement().x * 300.f * rightDir / 1.5f;
//				//jumpVec += request.GetDeltaMovement().y * 300.f * forwardDir / 1.5f;
//
//				jumpVec += request.GetDeltaMovement().x * jumpForce * rightDir / 1.5f;
//				jumpVec += request.GetDeltaMovement().y * jumpForce * forwardDir / 1.5f;
//			}
//			else
//			{
//				jumpVec = forwardDir * jumpForce; //300.f;
//			}
//			jumpVec.z = upDir.z * 2.5f; // 250.f;
//
//			//TODO
//			//NetSpawnParticleEffect("alien_special.Trooper.doubleJumpAttack");
//
//			//TODO
//			//SubEnergy(TROOPER_JUMP_ENERGY_COST);
//
//			animCharRequest.velocity += jumpVec;
//			m_pAnimatedCharacter->AddMovement(animCharRequest);
//
//			TOS_SAFE_ADD_ENERGY(this, -doubleJumpCost);
//
//			pSlaveStats->jumpCount = 0;
//
//			//TODO
//			//The controlled trooper cannot to do jump attack after double jump
//			//m_trooper.canJumpMelee = false;
//		}
//	}
//}