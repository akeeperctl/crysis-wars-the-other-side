#include "StdAfx.h"
#include "TOSTrooper.h"

#include "GameUtils.h"

CTOSTrooper::CTOSTrooper()
{
}

CTOSTrooper::~CTOSTrooper()
{
}

void CTOSTrooper::PostInit(IGameObject* pGameObject)
{
	CTrooper::PostInit(pGameObject);
}

void CTOSTrooper::Update(SEntityUpdateContext& ctx, const int updateSlot)
{
	CTrooper::Update(ctx,updateSlot);
}

bool CTOSTrooper::NetSerialize(const TSerialize ser, const EEntityAspects aspect, const uint8 profile, const int flags)
{
	if (!CTrooper::NetSerialize(ser, aspect, profile, flags))
		return false;



	return true;
}

void CTOSTrooper::ProcessMovement(const float frameTime)
{
	CTrooper::ProcessMovement(frameTime);

	ProcessJumpFlyControl(m_moveRequest.velocity, frameTime);
}

void CTOSTrooper::ProcessJumpFlyControl(const Vec3& move, const float frameTime)
{
	if (move.x > 0 || move.y > 0)
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
		Matrix33 baseMtxZ;
		baseMtxZ.SetRotationXYZ(angles);

		Vec3       actual = baseMtxZ * Vec3(0, 0, 0);
		const Vec3 goal = baseMtxZ * desiredVelocityClamped;

		Interpolate(actual, goal, 5.0f, frameTime);

		actionMove.dir = actual;

		if (m_stats.speed <= 12.0f)
			GetEntity()->GetPhysics()->Action(&actionMove);

		//Matrix rotation debug
		constexpr bool isDebugEnabled = false;
		if (isDebugEnabled)
		{
			static float c[] = { 1,1,1,1 };
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 0, 255, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn0(), ColorB(0, 0, 255, 255));
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(255, 0, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn1(), ColorB(255, 0, 0, 255));
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 255, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn2(), ColorB(0, 255, 0, 255));
		}
	}
}

void CTOSTrooper::UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov)
{
	//CTrooper::UpdateMasterView(viewParams, offsetY, target, currentFov);

	//const Matrix33 alienWorldMtx(GetEntity()->GetWorldTM());

	//if (!m_pAbilitiesSystem->trooper.isCeiling)
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
