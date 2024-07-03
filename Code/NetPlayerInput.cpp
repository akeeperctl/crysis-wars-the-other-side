#include "StdAfx.h"
#include "NetPlayerInput.h"
#include "Player.h"
#include "Game.h"
#include "GameCVars.h"


/*
moveto (P1);                            // move pen to startpoint
for (int t=0; t < steps; t++)
{
float s = (float)t / (float)steps;    // scale s to go from 0 to 1
float h1 =  2s^3 - 3s^2 + 1;          // calculate basis function 1
float h2 = -2s^3 + 3s^2;              // calculate basis function 2
float h3 =   s^3 - 2*s^2 + s;         // calculate basis function 3
float h4 =   s^3 -  s^2;              // calculate basis function 4
vector p = h1*P1 +                    // multiply and sum all funtions
h2*P2 +                    // together to build the interpolated
h3*T1 +                    // point along the curve.
h4*T2;
lineto (p)                            // draw to calculated point on the curve
}*/

static Vec3 HermiteInterpolate( float s, const Vec3& p1, const Vec3& t1, const Vec3& p2, const Vec3& t2 )
{
	float s2 = s*s;
	float s3 = s2*s;
	float h1 = 2*s3 - 3*s2 + 1.0f;
	float h2 = -2*s3 + 3*s2;
	float h3 = s3 - 2*s2 + s;
	float h4 = s3 - s2;
	return h1*p1 + h2*p2 + h3*t1 + h4*t2;
}








CNetPlayerInput::CNetPlayerInput( CPlayer * pPlayer ) : m_pPlayer(pPlayer)
{
}

void CNetPlayerInput::PreUpdate()
{
	IPhysicalEntity* pPhysEnt = m_pPlayer->GetEntity()->GetPhysics();
	if (!pPhysEnt)
	    return;

	CMovementRequest moveRequest;
	SMovementState moveState;
	m_pPlayer->GetMovementController()->GetMovementState(moveState);

	// Получаем обратную кватернионную ротацию и нормализуем её
	Quat invWorldRot = m_pPlayer->GetBaseQuat().GetInverted().GetNormalized();
	// Применяем обратную ротацию к движению и нормализуем результат
	Vec3 deltaMovement = invWorldRot * m_curInput.deltaMovement;
	deltaMovement.NormalizeSafe(ZERO);
	deltaMovement *= m_curInput.deltaMovement.GetLength();
	moveRequest.AddDeltaMovement(deltaMovement);

	if (IsDemoPlayback())
	{
	    // Получаем локальное направление взгляда
	    Vec3 localVDir = m_pPlayer->GetViewQuatFinal().GetInverted() * m_curInput.lookDirection;
	    localVDir.NormalizeSafe();

	    // Вычисляем углы поворота и применяем их с учётом времени кадра
	    Ang3 deltaAngles(asin(localVDir.z), 0, cry_atan2f(-localVDir.x, localVDir.y));
	    moveRequest.AddDeltaRotation(deltaAngles * gEnv->pTimer->GetFrameTime());
	}

	Vec3 distantTarget = moveState.eyePosition + 1000.0f * m_curInput.lookDirection;
	Vec3 lookTarget = distantTarget;

	// Проверяем, является ли клиент видимым
	if (gEnv->bClient && m_pPlayer->GetGameObject()->IsProbablyVisible())
	{
	    // Инициализация для проверки пересечения луча
	    ray_hit hit;
	    static const int obj_types = ent_all; // Все типы объектов
	    static const unsigned int flags = rwi_stop_at_pierceable|rwi_colltype_any;
	    
	    // Проверка пересечения луча для определения цели взгляда
	    bool rayHitAny = gEnv->pPhysicalWorld->RayWorldIntersection(
	        moveState.eyePosition, 150.0f * m_curInput.lookDirection, 
	        obj_types, flags, &hit, 1, pPhysEnt
	    ) != 0;
	    
	    // Если есть пересечение, обновляем цель взгляда
	    if (rayHitAny)
	    {
	        lookTarget = hit.pt;
	    }

	    // Расстояния для разных стоек
	    static float proneDist = 1.0f; // Лёжа
	    static float crouchDist = 0.6f; // Сидя
	    static float standDist = 0.3f; // Стоя

	    // Выбор расстояния в зависимости от стойки
	    float dist = (m_pPlayer->GetStance() == STANCE_CROUCH) ? crouchDist :
	                 (m_pPlayer->GetStance() == STANCE_PRONE) ? proneDist : standDist;

	    // Проверка дистанции до цели взгляда
	    if ((lookTarget - moveState.eyePosition).GetLength2D() < dist)
	    {
	        Vec3 eyeToTarget2d = lookTarget - moveState.eyePosition;
	        eyeToTarget2d.z = 0.0f;
	        eyeToTarget2d.NormalizeSafe();
	        eyeToTarget2d *= dist;

	        // Повторная проверка пересечения луча
	        ray_hit newhit;
	        rayHitAny = gEnv->pPhysicalWorld->RayWorldIntersection(
	            moveState.eyePosition + eyeToTarget2d, 3 * Vec3(0,0,-1), 
	            obj_types, flags, &newhit, 1, pPhysEnt
	        ) != 0;
	        
	        // Обновление цели взгляда, если есть новое пересечение
	        if (rayHitAny)
	        {
	            lookTarget = newhit.pt;
	        }
	    }

	    // Убеждаемся, что цель взгляда находится на безопасном расстоянии
	    Vec3 dir = lookTarget - moveState.eyePosition;
	    static float minDist = 1.5f; // Минимальное расстояние
	    if (dir.GetLengthSquared() < minDist)
	    {
	        lookTarget = moveState.eyePosition + dir.GetNormalizedSafe();
	    }

	    // Визуализация позиции глаз для сравнения (отключено)
	    // gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(moveState.eyePosition, 0.04f, ColorF(0.3f,0.2f,0.7f,1.0f));
	}

	// Установка целей для движения и прицеливания
	moveRequest.SetLookTarget(lookTarget);
	moveRequest.SetAimTarget(lookTarget);

	// Установка цели для тела, если движение достаточно велико
	if (m_curInput.deltaMovement.GetLengthSquared() > sqr(0.2f)) // 0.2f - почти неподвижность
	{
	    moveRequest.SetBodyTarget(distantTarget);
	}
	else
	{
	    moveRequest.ClearBodyTarget(); // Очистка цели для тела, если движение минимально
	}
	moveRequest.SetAllowStrafing(true);

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);

	float lean=0.0f;
	if (m_curInput.leanl)
		lean-=1.0f;
	if (m_curInput.leanr)
		lean+=1.0f;

	if (fabsf(lean)>0.01f)
		moveRequest.SetLean(lean);
	else
		moveRequest.ClearLean();

	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);

	if (m_curInput.sprint)
		m_pPlayer->m_actions |= ACTION_SPRINT;
	else
		m_pPlayer->m_actions &= ~ACTION_SPRINT;

	if (m_curInput.leanl)
		m_pPlayer->m_actions |= ACTION_LEANLEFT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANLEFT;

	if (m_curInput.leanr)
		m_pPlayer->m_actions |= ACTION_LEANRIGHT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANRIGHT;

	// debug..
	if (g_pGameCVars->g_debugNetPlayerInput & 2)
	{
		IPersistantDebug * pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin( string("update_player_input_") + m_pPlayer->GetEntity()->GetName(), true );

		Vec3 wp = m_pPlayer->GetEntity()->GetWorldPos();
		wp.z += 2.0f;

		pPD->AddSphere( moveRequest.GetLookTarget(), 0.5f, ColorF(1,0,1,0.3f), 1.0f );
		pPD->AddSphere( moveRequest.GetMoveTarget(), 0.5f, ColorF(1,1,0,0.3f), 1.0f );
		pPD->AddDirection( m_pPlayer->GetEntity()->GetWorldPos() + Vec3(0,0,2), 1, m_curInput.deltaMovement, ColorF(1,0,0,0.3f), 1.0f );
	}

	//m_curInput.deltaMovement.zero();
}

void CNetPlayerInput::Update()
{
	if (gEnv->bServer && (g_pGameCVars->sv_input_timeout>0) && ((gEnv->pTimer->GetFrameStartTime()-m_lastUpdate).GetMilliSeconds()>=g_pGameCVars->sv_input_timeout))
	{
		m_curInput.deltaMovement.zero();
		m_curInput.sprint=m_curInput.leanl=m_curInput.leanr=false;
		m_curInput.stance=STANCE_NULL;

		m_pPlayer->GetGameObject()->ChangedNetworkState( INPUT_ASPECT );
	}
}

void CNetPlayerInput::PostUpdate()
{
}

void CNetPlayerInput::SetState( const SSerializedPlayerInput& input )
{
	DoSetState(input);

	m_lastUpdate = gEnv->pTimer->GetCurrTime();
}

void CNetPlayerInput::GetState( SSerializedPlayerInput& input )
{
	input = m_curInput;
}

void CNetPlayerInput::Reset()
{
	SSerializedPlayerInput i(m_curInput);
	i.leanl=i.leanr=i.sprint=false;
	i.deltaMovement.zero();

	DoSetState(i);

	m_pPlayer->GetGameObject()->ChangedNetworkState(IPlayerInput::INPUT_ASPECT);
}

void CNetPlayerInput::DisableXI(bool disabled)
{
}


void CNetPlayerInput::DoSetState(const SSerializedPlayerInput& input )
{
	// Функция используется при считывании сериализованных параметров

	m_curInput = input;
	m_pPlayer->GetGameObject()->ChangedNetworkState( INPUT_ASPECT );

	CMovementRequest moveRequest;
	moveRequest.SetStance( (EStance)m_curInput.stance );

	if(IsDemoPlayback())
	{
		Vec3 localVDir(m_pPlayer->GetViewQuatFinal().GetInverted() * m_curInput.lookDirection);
		Ang3 deltaAngles(asin(localVDir.z),0,cry_atan2f(-localVDir.x,localVDir.y));
		moveRequest.AddDeltaRotation(deltaAngles*gEnv->pTimer->GetFrameTime());
	}

	moveRequest.SetLookTarget( m_pPlayer->GetEntity()->GetWorldPos() + 10.0f * m_curInput.lookDirection );
	moveRequest.SetAimTarget(moveRequest.GetLookTarget());

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);
	moveRequest.SetAllowStrafing(true);

	float lean=0.0f;
	if (m_curInput.leanl)
		lean-=1.0f;
	if (m_curInput.leanr)
		lean+=1.0f;
	moveRequest.SetLean(lean);

	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);

	// debug..
	if (g_pGameCVars->g_debugNetPlayerInput & 1)
	{
		IPersistantDebug * pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin( string("net_player_input_") + m_pPlayer->GetEntity()->GetName(), true );
		pPD->AddSphere( moveRequest.GetLookTarget(), 0.5f, ColorF(1,0,1,1), 1.0f );
		//			pPD->AddSphere( moveRequest.GetMoveTarget(), 0.5f, ColorF(1,1,0,1), 1.0f );

		Vec3 wp(m_pPlayer->GetEntity()->GetWorldPos() + Vec3(0,0,2));
		pPD->AddDirection( wp, 1.5f, m_curInput.deltaMovement, ColorF(1,0,0,1), 1.0f );
		pPD->AddDirection( wp, 1.5f, m_curInput.lookDirection, ColorF(0,1,0,1), 1.0f );
	}
}
