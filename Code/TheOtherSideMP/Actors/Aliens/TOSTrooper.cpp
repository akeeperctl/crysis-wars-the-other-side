#include "StdAfx.h"
#include "TOSTrooper.h"

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

void CTOSTrooper::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CTrooper::Update(ctx,updateSlot);
}

bool CTOSTrooper::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CTrooper::NetSerialize(ser, aspect, profile, flags))
		return false;

	return true;
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
