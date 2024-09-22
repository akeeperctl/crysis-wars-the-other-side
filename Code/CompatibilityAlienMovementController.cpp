// ReSharper disable CppTooWideScope
#include "StdAfx.h"
#include "CompatibilityAlienMovementController.h"
#include "Alien.h"
#include <IItemSystem.h>

//TheOtherSide
#include "TheOtherSideMP/Actors/Aliens/TOSTrooper.h"
#include "TheOtherSideMP/Helpers/TOS_NET.h"
//~TheOtherSide

CCompatibilityAlienMovementController::CCompatibilityAlienMovementController( CAlien * pAlien ) : m_pAlien(pAlien), m_atTarget(false)
{
}

void CCompatibilityAlienMovementController::Reset()
{
}

bool CCompatibilityAlienMovementController::Update( float frameTime, SActorFrameMovementParams& params )
{
	ICharacterInstance* pCharacter = m_pAlien->GetEntity()->GetCharacter(0);
	if (!pCharacter)
		return false;

	// Params здесь не используется.
	// Заполняется m_currentMovementState
	UpdateCurMovementState(params);

	return true;
}

void CCompatibilityAlienMovementController::Release()
{
	delete this;
}

bool CCompatibilityAlienMovementController::RequestMovement( CMovementRequest& request )
{
	//SMovementState state;
	//GetMovementState(state);

	//Vec3 currentEyePos = state.eyePosition;
	Vec3 currentPos = m_pAlien->GetEntity()->GetWorldPos();
	//Vec3 currentForw = m_pAlien->GetEntity()->GetWorldRotation() * FORWARD_DIRECTION;

	CAlien::SMovementRequestParams os (request);

	if (request.HasMoveTarget())
		os.vMoveDir = (request.GetMoveTarget() - currentPos).GetNormalizedSafe(FORWARD_DIRECTION);


	if (request.HasForcedNavigation())
	{
		os.vMoveDir = request.GetForcedNavigation();
		os.fDesiredSpeed = os.vMoveDir.GetLength();
		os.vMoveDir.NormalizeSafe();
	}

	//TheOtherSide

	if (request.HasDeltaMovement())
	{
		os.vDeltaMovement = request.GetDeltaMovement();
		//CryLogAlways("Controller: os.vDeltaMovement = (%f,%f,%f)", os.vDeltaMovement.x, os.vDeltaMovement.y, os.vDeltaMovement.z);
	}

	if (request.ShouldJump())
	{
		// Задаем SetJump для реагирование на него в будущем
		m_currentMovementRequest.SetJump();

		//auto pTrooper = static_cast<CTOSTrooper*>(m_pAlien);
		//if (pTrooper)
		//{
		//	pTrooper->ProcessJump(request);
		//	request.ClearJump();
		//}

		request.ClearJump();

		//TODO: обработать прыжок как движение вверх у Scout и Alien
	}
	//~TheOtherSide

	IAnimationGraphState *pAnimationGraphState = m_pAlien->GetAnimationGraphState();
	//CRY_ASSERT(pAnimationGraphState); // Hey, we can't assume we get a state!

	if(pAnimationGraphState)
	{
		if (const SAnimationTarget* pTarget = pAnimationGraphState->GetAnimationTarget())
		{
			// Если цель анимации готовится, настраиваем точное позиционирование
			if (pTarget->preparing)
			{
				os.bExactPositioning = true;
				PATHPOINT p;
				p.vPos = pTarget->position;
				p.vDir = pTarget->orientation * FORWARD_DIRECTION;
				os.remainingPath.push_back(p);
			}

			// Определяем, двигается ли AI в трехмерном пространстве
			bool b3D = false;
			IAIActor* pAIActor = CastToIAIActorSafe(m_pAlien->GetEntity()->GetAI());
			if (pAIActor)
				b3D = pAIActor->GetMovementAbility().b3DMove;

			// Вычисляем расстояние до цели анимации
			Vec3 targetDisp(pTarget->position - currentPos);
			float distance = b3D ? targetDisp.GetLength() : targetDisp.GetLength2D();

			// Вычисляем направление к цели анимации
			Vec3 targetDir(pTarget->orientation.GetColumn1());
			float diffRot = targetDir.Dot(m_pAlien->GetEntity()->GetWorldTM().GetColumn1());

			// Если расстояние меньше 2 метров, настраиваем скорость и направление движения
			if (distance < 2.0f)
			{
				if (os.fDesiredSpeed == 0.0f)
				{
					os.fDesiredSpeed = max(0.1f, (distance / 2) * 0.8f);
					os.vMoveDir = pTarget->position - currentPos;
					if (!b3D)
						os.vMoveDir.z = 0;
					os.vMoveDir.NormalizeSafe(FORWARD_DIRECTION);
				}

				// Замедляем скорость, если пришелец близок к цели, но не достаточно ориентирован
				if (diffRot < 0.7f)
				{
					os.fDesiredSpeed *= max(distance / 2, 0.f);
				}
			}

			// Регулируем скорость движения в соответствии с временем кадра
			float frameTime = gEnv->pTimer->GetFrameTime();
			if (frameTime > 0)
			{
				float expectedSpeed = distance / frameTime;
				if (os.fDesiredSpeed > expectedSpeed)
					os.fDesiredSpeed = expectedSpeed;
			}

			// Разрешаем активацию анимации, если расстояние меньше начального
			static float startDistance = 5.0f;
			pTarget->allowActivation = distance < startDistance;
		}
	}

	m_pAlien->SetActorMovement(os);

	if (pAnimationGraphState)
	{
		// Обработка цели актора, если она есть
		if (request.HasActorTarget())
		{
			// Получаем параметры цели актора
			const SActorTargetParams& p = request.GetActorTarget();
			SAnimationTargetRequest req;

			// Устанавливаем позицию и радиус для запроса анимационной цели
			req.position = p.location;
			req.positionRadius = p.locationRadius;

			// Устанавливаем направление и радиус для запроса анимационной цели
			req.direction = p.direction;
			req.directionRadius = p.directionRadius;

			// Устанавливаем радиус подготовки и проекцию конца для запроса анимационной цели
			req.prepareRadius = 3.0f;
			req.projectEnd = p.projectEnd;

			// Создаем триггер анимации и устанавливаем его параметры
			IAnimationSpacialTrigger* pTrigger = pAnimationGraphState->SetTrigger(req, p.triggerUser, p.pQueryStart, p.pQueryEnd);
			
			if (!p.vehicleName.empty())
			{
				pTrigger->SetInput("Vehicle", p.vehicleName.c_str());
				pTrigger->SetInput("VehicleSeat", p.vehicleSeat);
			}

			// Устанавливаем желаемую скорость и угол поворота для триггера анимации
			pTrigger->SetInput("DesiredSpeed", p.speed);
			pTrigger->SetInput("DesiredTurnAngleZ", 0);
			
			// Если анимация задана, устанавливаем соответствующий входной сигнал
			if (!p.animation.empty())
			{
				pTrigger->SetInput(p.signalAnimation ? "Signal" : "Action", p.animation.c_str());
			}
		}
		// Если есть запрос на удаление цели актора, очищаем триггер
		else if (request.RemoveActorTarget())
		{
			pAnimationGraphState->ClearTrigger(eAGTU_AI);
		}
	}

	m_atTarget = os.eActorTargetPhase == eATP_Finished;

	if (request.HasFireTarget())
		m_currentMovementRequest.SetFireTarget( request.GetFireTarget() );
	else if (request.RemoveFireTarget())
		m_currentMovementRequest.ClearFireTarget();

	if (request.HasAimTarget())
		m_currentMovementRequest.SetAimTarget( request.GetAimTarget() );
	else if (request.RemoveAimTarget())
		m_currentMovementRequest.ClearAimTarget();

	return true;
}
void CCompatibilityAlienMovementController::UpdateCurMovementState(const SActorFrameMovementParams& params)
{
	// Заполнение структуры информацией о пришельце
	CAlien::SBodyInfo bodyInfo;
	m_pAlien->GetActorInfo(bodyInfo);

	// Получение текущего состояния движения
	SMovementState& state(m_currentMovementState);

	// Установка состояния и размеров стойки
	state.stance = bodyInfo.stance;
	state.m_StanceSize = bodyInfo.m_stanceSizeAABB;
	state.m_ColliderSize = bodyInfo.m_colliderSizeAABB;

	// Установка направлений взгляда и анимации глаз
	state.eyeDirection = bodyInfo.vEyeDir;
	state.animationEyeDirection = bodyInfo.vEyeDirAnim;

	// Установка позиций глаз и оружия
	state.eyePosition = bodyInfo.vEyePos;
	state.weaponPosition = bodyInfo.vFirePos;

	// Установка направлений движения и вверх
	state.movementDirection = bodyInfo.vFwdDir;
	state.upDirection = bodyInfo.vUpDir;

	// Проверка, достиг ли пришелец цели движения
	state.atMoveTarget = m_atTarget;

	// Установка направления тела пришельца
	state.bodyDirection = m_pAlien->GetEntity()->GetWorldRotation() * Vec3(0, 1, 0);

	// Расчет направления прицеливания
	if (m_currentMovementRequest.HasAimTarget())
		state.aimDirection = (m_currentMovementRequest.GetAimTarget() - state.weaponPosition).GetNormalizedSafe();
	else
		state.aimDirection = bodyInfo.vFireDir.GetNormalizedSafe();

	// Расчет направления стрельбы
	if (m_currentMovementRequest.HasFireTarget())
		state.fireDirection = (m_currentMovementRequest.GetFireTarget() - state.weaponPosition).GetNormalizedSafe(state.aimDirection);
	else
		state.fireDirection = state.aimDirection;

	// Проверка, жив ли пришелец
	state.isAlive = (m_pAlien->GetHealth() > 0);

	// FIXME: Состояние прицеливания всегда истинно - возможно, это временная заглушка
	state.isAiming = true;

	// Проверка, стреляет ли пришелец
	state.isFiring = (m_pAlien->GetActorStats()->inFiring > 0.001f);

	// Установка цели стрельбы, если она есть
	if (m_currentMovementRequest.HasFireTarget())
		state.fireTarget = m_currentMovementRequest.GetFireTarget();
}


bool CCompatibilityAlienMovementController::GetStanceState(EStance stance, float lean, bool defaultPose, SStanceState& state)
{
	const SStanceInfo*	pStance = m_pAlien->GetStanceInfo(stance);
	if(!pStance)
		return false;

	if(defaultPose)
	{
		state.pos.Set(0,0,0);
		state.bodyDirection = FORWARD_DIRECTION;
		state.upDirection(0,0,1);
		state.weaponPosition = m_pAlien->GetWeaponOffsetWithLean(stance, lean, m_pAlien->GetEyeOffset());
		state.aimDirection = FORWARD_DIRECTION;
		state.fireDirection = FORWARD_DIRECTION;
		state.eyePosition = pStance->GetViewOffsetWithLean(lean);
		state.eyeDirection = FORWARD_DIRECTION;
		state.m_StanceSize = pStance->GetStanceBounds();
		state.m_ColliderSize = pStance->GetColliderBounds();
	}
	else
	{
		// TODO: the directions are like not to match. Is the AI even using them?
		CAlien::SBodyInfo bodyInfo;
		m_pAlien->GetActorInfo( bodyInfo );

		Matrix34	tm = m_pAlien->GetEntity()->GetWorldTM();

		state.pos = m_pAlien->GetEntity()->GetWorldPos();
		state.bodyDirection = bodyInfo.vFwdDir;
		state.upDirection = bodyInfo.vUpDir;
		state.weaponPosition = tm.TransformPoint(m_pAlien->GetWeaponOffsetWithLean(stance, lean, m_pAlien->GetEyeOffset()));
		state.aimDirection = bodyInfo.vFireDir;
		state.fireDirection = bodyInfo.vFireDir;
		state.eyePosition = tm.TransformPoint(pStance->GetViewOffsetWithLean(lean));
		state.eyeDirection = bodyInfo.vEyeDir;
		state.m_StanceSize = pStance->GetStanceBounds();
		state.m_ColliderSize = pStance->GetColliderBounds();
	}

	return true;
}