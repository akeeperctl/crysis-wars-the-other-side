#include "StdAfx.h"
#include "Drone.h"
#include "GameUtils.h"

#include <IViewSystem.h>
#include <IItemSystem.h>
#include <IPhysics.h>
#include <ICryAnimation.h>
#include <ISerialize.h>
#include <IRenderAuxGeom.h>
#include "Lam.h"

#include "CompatibilityAlienMovementController.h"

CDrone::CDrone()
{

}

CDrone::~CDrone()
{

}

void CDrone::UpdateGlow(float energy)
{

}

void CDrone::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CAlien::Update(ctx, updateSlot);
	UpdateGlow(GetAlienEnergy());
}

IGrabHandler* CDrone::CreateGrabHanlder()
{
	m_pGrabHandler = new CMultipleGrabHandler(this);
	return m_pGrabHandler;
}

bool CDrone::CreateCodeEvent(SmartScriptTable& rTable)
{
	const char* event = 0;
	if (!rTable->GetValue("event", event))
		return false;
	else
		return CAlien::CreateCodeEvent(rTable);
}

void CDrone::Revive(bool fromInit /*= false*/)
{
	CAlien::Revive(fromInit);

	//FIXME:deactivate transrot2k until the assets gets fixed
	if (m_pAnimatedCharacter)
	{
		SAnimatedCharacterParams acFlags = m_pAnimatedCharacter->GetParams();
		acFlags.flags |= eACF_NoTransRot2k;

		m_pAnimatedCharacter->SetParams(acFlags);
	}

	// setup searchbeam
	//static IEntityClass* pBeamClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ScoutSearchBeam");
	//m_searchbeam.itemId = GetInventory()->GetItemByClass(pBeamClass);

	//if (m_searchbeam.itemId)
	//{
	//	if (!m_searchbeam.pAttachment)
	//	{
	//		if (ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0))
	//		{
	//			IAttachmentManager* pAttachMan = pCharacter->GetIAttachmentManager();
	//			m_searchbeam.pAttachment = pAttachMan->GetInterfaceByName("searchlight_attachment");

	//			if (!m_searchbeam.pAttachment)
	//				m_searchbeam.pAttachment = pAttachMan->CreateAttachment("searchlight_attachment", CA_BONE, "weapon_bone");
	//		}
	//	}

	//	if (m_searchbeam.pAttachment)
	//	{
	//		CEntityAttachment* pEntityAttachment = new CEntityAttachment();
	//		pEntityAttachment->SetEntityId(m_searchbeam.itemId);
	//		m_searchbeam.pAttachment->AddBinding(pEntityAttachment);
	//	}
	//}
}

void CDrone::ProcessRotation(float frameTime)
{

}

void CDrone::ProcessSwimming(float frameTime)
{
	ProcessMovement(frameTime);
}

void CDrone::ProcessMovement(float frameTime)
{
	static bool showDebugInfo = false;
	static float color[] = { 1,1,1,1 };

	if (frameTime > 0.009f)
		frameTime = 0.009f;
	//frameTime = min(1.0f, frameTime);

	const float maxSpeed = GetStanceInfo(m_stance)->maxSpeed;

	//TheOtherSide use forceview at 49
	if (m_params.forceView < 50.0)
	{	// normal action
		// velocity/speed setting up
		m_desiredVelocity = m_input.movementVector;
		m_desiredVeloctyQuat = GetQuatFromMat33(m_viewMtx);

		//TheOtherSide
		//bool isGyro = g_pControlSystem->GetAbilitiesSystem()->scout.isGyroEnabled;
		//bool isSearch = g_pControlSystem->GetAbilitiesSystem()->scout.IsSearch;

		Vec3 column0(0, 0, 0);
		Vec3 column1(0, 0, 0);
		Vec3 column2(0, 0, 0);

		//if (!isGyro && !isSearch)
		//{
		//	column0 = GetEntity()->GetRotation().GetColumn0();
		//	column1 = GetEntity()->GetRotation().GetColumn1();
		//	column2 = GetEntity()->GetRotation().GetColumn2();
		//}
		//else
		{
			column0 = m_viewMtx.GetColumn(0);
			column1 = m_viewMtx.GetColumn(1);
			column2 = m_viewMtx.GetColumn(2);
		}

		m_desiredVelocity += column0 * m_input.deltaMovement.x * maxSpeed;
		m_desiredVelocity += column1 * m_input.deltaMovement.y * maxSpeed;
		m_desiredVelocity += column2 * m_input.deltaMovement.z * maxSpeed;

		//~TheOtherSide

		/*Vec3 deltarot = m_input.ConDeltaRotation

		m_desiredVeloctyQuat *= Quat(Matrix33::CreateFromVectors());*/

		m_velocity = m_stats.velocity;

		float desiredSpeed = m_desiredVelocity.GetLength();
		float fCurSpeed = m_stats.speed;

		float rate1 = min(maxSpeed, fabsf(desiredSpeed - fCurSpeed));
		float rateVelocity = (rate1 > 0.0f) ? min(frameTime * 2.5f, 5.0f / (rate1 * rate1)) : frameTime * 2.5f;
		float rateRotation = (rate1 > 0.0f) ? min(frameTime * 2.5f, 2.5f / (rate1 * rate1)) : frameTime * 2.5f;

		m_rateParams.mainRate = rate1;
		m_rateParams.velocityRate = rateVelocity;
		m_rateParams.rotationRate = rateRotation;

		// velocity/speed interpolation
		Interpolate(m_velocity, m_desiredVelocity, 1.0f, rateVelocity);

		// pitch/roll
		if (desiredSpeed > 0.0f && fCurSpeed > 0.0f)
		{
			Vec3 vUp(0.0f, 0.0f, 1.0f);
			Vec3 vFwd = m_velocity.GetNormalized();
			Vec3 vNew = m_desiredVelocity.GetNormalized();

			if (fabs(vFwd * vUp) < cosf(DEG2RAD(3.0f)))
			{
				vFwd.z = 0;
				vFwd.NormalizeSafe();
				Vec3 vWng = vFwd.Cross(vUp);
				vWng.NormalizeSafe();

				float cofRoll = 6.0f * (vWng * vNew) * fCurSpeed / maxSpeed;
				float cofPitch = (vNew * vFwd) * rate1 / maxSpeed;
				cofRoll = max(-1.0f, min(1.0f, cofRoll));
				cofPitch = max(-1.0f, min(1.0f, cofPitch));
				m_desiredVeloctyQuat *= Quat::CreateRotationY(DEG2RAD(60.0f) * cofRoll);
				m_desiredVeloctyQuat *= Quat::CreateRotationX(DEG2RAD(-60.0f) * cofPitch);
			}
		}

		// rotation interpolation
		Quat modelRot(GetEntity()->GetRotation());
		modelRot = Quat::CreateSlerp(modelRot, m_desiredVeloctyQuat, rateRotation);

		// commit the result interpolation
		assert(GetEntity()->GetRotation().IsValid());
		assert(GetEntity()->GetRotation().GetInverted().IsValid());
		assert(modelRot.IsValid());

		//TheOtherSide
		//now rotation give by Control System
		if (IsHaveOwner())
		{
			//if (isGyro || isSearch)
				m_moveRequest.rotation = GetEntity()->GetRotation().GetInverted() * modelRot;
			//else
			//	m_moveRequest.rotation = g_pControlSystem->GetLocalControlClient()->GetScoutParams().modelQuat;
		}
		else
			m_moveRequest.rotation = GetEntity()->GetRotation().GetInverted() * modelRot;

		//~TheOtherSide
		m_moveRequest.rotation.Normalize();

		assert(m_moveRequest.rotation.IsValid());

		m_moveRequest.velocity = m_velocity;
		m_moveRequest.type = eCMT_Fly;
	}
	else if (m_params.forceView < 101.0f)
	{	// for the emergency situation...set speed immidiately
		// velocity/speed setting up
		//TheOtherSide
		m_desiredVelocity = m_input.movementVector;
		m_desiredVelocity += m_viewMtx.GetColumn(0) * m_input.deltaMovement.x * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(1) * m_input.deltaMovement.y * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(2) * m_input.deltaMovement.z * maxSpeed;
		//~TheOtherSide

		m_velocity = m_stats.velocity;
		Interpolate(m_velocity, m_desiredVelocity, 1.0f, 0.25f);

		// just keep rotation angles
		Quat modelRot(GetEntity()->GetRotation());

		// commit the result interpolation & set velocity immidiately
		m_moveRequest.rotation = GetEntity()->GetRotation().GetInverted() * modelRot;
		m_moveRequest.rotation.Normalize();
		m_moveRequest.velocity = m_velocity;
		m_moveRequest.type = eCMT_Fly;
	}
	else if (m_params.forceView < 151.0f)
	{	// for the emergency situation...set speed immidiately
		// velocity/speed setting up
		//TheOtherSide
		m_desiredVelocity = m_input.movementVector;
		m_desiredVelocity += m_viewMtx.GetColumn(0) * m_input.deltaMovement.x * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(1) * m_input.deltaMovement.y * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(2) * m_input.deltaMovement.z * maxSpeed;
		//~TheOtherSide
		m_velocity = m_stats.velocity;
		Interpolate(m_velocity, m_desiredVelocity, 1.0f, 0.25f);

		m_desiredVeloctyQuat = GetQuatFromMat33(m_viewMtx);
		Quat modelRot(GetEntity()->GetRotation());
		modelRot = Quat::CreateSlerp(modelRot, m_desiredVeloctyQuat, 0.20f);

		// commit the result interpolation & set velocity immidiately
		m_moveRequest.rotation = GetEntity()->GetRotation().GetInverted() * modelRot;
		m_moveRequest.rotation.Normalize();
		m_moveRequest.velocity = m_velocity;
		m_moveRequest.type = eCMT_Fly;
	}
	else if (m_params.forceView < 201.0f)
	{	// for vsAir

		// velocity/speed setting up
		//TheOtherSide
		m_desiredVelocity = m_input.movementVector;
		m_desiredVelocity += m_viewMtx.GetColumn(0) * m_input.deltaMovement.x * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(1) * m_input.deltaMovement.y * maxSpeed;
		m_desiredVelocity += m_viewMtx.GetColumn(2) * m_input.deltaMovement.z * maxSpeed;
		//~TheOtherSide
		m_velocity = m_stats.velocity;
		Interpolate(m_velocity, m_desiredVelocity, 1.0f, 0.10f);

		m_desiredVeloctyQuat = GetQuatFromMat33(m_viewMtx);
		Quat modelRot(GetEntity()->GetRotation());
		modelRot = Quat::CreateSlerp(modelRot, m_desiredVeloctyQuat, 0.10f);

		// commit the result interpolation & set velocity immidiately
		m_moveRequest.rotation = GetEntity()->GetRotation().GetInverted() * modelRot;
		m_moveRequest.rotation.Normalize();
		m_moveRequest.velocity = m_velocity;
		m_moveRequest.type = eCMT_Fly;
	}

}

void CDrone::AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event)
{
	if (strcmp(event.m_EventName, "ObjectGrabbed") == 0) {
		m_pGrabHandler->StartGrab();
	}
	else if (strcmp(event.m_EventName, "ObjectThrown") == 0) {
		m_pGrabHandler->StartDrop();
	}
	else if (strcmp(event.m_EventName, "StartIK") == 0) {
		(static_cast <CAnimatedGrabHandler*> (m_pGrabHandler))->ActivateIK();
	}
}

void CDrone::UpdateGrab(float frameTime)
{
	CActor::UpdateGrab(frameTime);

	//bool showBeam(m_pGrabHandler && m_pGrabHandler->GetStats()->grabId > 0);

	//if (showBeam && !m_pBeamEffect)
	//	m_pBeamEffect = new CAlienBeam(this);

	//if (m_pBeamEffect)
	//{
	//	if (m_pBeamEffect->IsActive() != showBeam)
	//	{
	//		if (showBeam)
	//		{
	//			//FIXME:that cast is quite bad
	//			m_pBeamEffect->Start(
	//				"Alien_Weapons.Freeze_Beam.beam_firemode1",
	//				m_pGrabHandler->GetStats()->grabId,
	//				Ang3(gf_PI * -0.25f, 0, gf_PI),
	//				((CScoutBeam*)m_pGrabHandler)->m_beamBone
	//			);
	//		}
	//		else
	//		{
	//			m_pBeamEffect->Stop();
	//		}
	//	}

	//	m_pBeamEffect->Update(frameTime);
	//}
}

void CDrone::SetActorMovement(SMovementRequestParams& control)
{
	SMovementState state;
	GetMovementController()->GetMovementState(state);

	SetActorMovementCommon(control);

	if (control.vMoveDir.IsZero())
	{
		if (control.vLookTargetPos.IsZero())
		{
			SetDesiredDirection(GetEntity()->GetWorldRotation() * FORWARD_DIRECTION);
			SetDesiredSpeed(ZERO);
		}
		else
		{
			SetDesiredDirection((control.vLookTargetPos - state.eyePosition).GetNormalizedSafe());
			SetDesiredSpeed(ZERO);
		}
	}
	else
	{
		if (control.vLookTargetPos.IsZero())
		{
			SetDesiredDirection(control.vMoveDir.GetNormalizedSafe());
			SetDesiredSpeed(control.vMoveDir * control.fDesiredSpeed);
		}
		else
		{
			SetDesiredDirection((control.vLookTargetPos - state.eyePosition).GetNormalizedSafe());
			SetDesiredSpeed(control.vMoveDir * control.fDesiredSpeed);
		}
	}

	int actions;

	switch (control.bodystate)
	{
	case 1:
		actions = ACTION_CROUCH;
		break;
	case 2:
		actions = ACTION_PRONE;
		break;
	case 3:
		actions = ACTION_RELAXED;
		break;
	case 4:
		actions = ACTION_STEALTH;
		break;
	default:
		actions = 0;
		break;
	}

	// Override the stance based on special behavior.
	SetActorStance(control, actions);

	m_input.actions = actions;
}

void CDrone::FullSerialize(TSerialize ser)
{
	CAlien::FullSerialize(ser);
}

void CDrone::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	GetAlienMemoryStatistics(s);
}

void CDrone::GetActorInfo(SBodyInfo& bodyInfo)
{
	CAlien::GetActorInfo(bodyInfo);

	//int headBoneID = GetBoneID(BONE_HEAD);
	//if (headBoneID > -1 && GetEntity()->GetCharacter(0))
	//{
	//	Matrix33 HeadMat(Matrix33(GetEntity()->GetCharacter(0)->GetISkeletonPose()->GetAbsJointByID(headBoneID).q));
	//	bodyInfo.vEyeDirAnim = Matrix33(GetEntity()->GetSlotWorldTM(0) * HeadMat).GetColumn(0);
	//}
}

bool CDrone::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	return true;
}

//void CDrone::ProcessMovementNew(float frameTime)
//{
//
//}
//
//void CDrone::ProcessRotationNew(float frameTime)
//{
//
//}

