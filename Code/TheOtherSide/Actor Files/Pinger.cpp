#include "StdAfx.h"
#include "Game.h"
#include "GameCVars.h"
#include "Pinger.h"
#include "GameUtils.h"

#include <IViewSystem.h>
#include <IItemSystem.h>
#include <IPhysics.h>
#include <ICryAnimation.h>
#include <ISerialize.h>
#include <IRenderAuxGeom.h>
#include <IMaterialEffects.h>

const float CPinger::s_turnThreshIdling = cry_cosf(DEG2RAD(60.0f));
const float CPinger::s_turnThreshTurning = cry_cosf(DEG2RAD(15.0f));
const float CPinger::s_turnThreshMoving = cry_cosf(DEG2RAD(15.0f));

//Just for getting water surface id
#include "Bullet.h"

CPinger::CPinger()
{
	//isShieldEnabled = false;
	//m_shieldSoundId = 0;
	//m_angleDeviation = 0.0f;
	m_lastTurnSpeed = 0.5f;
	//m_smoothMovementVec = Vec3(0, 0, 0);
}

IGrabHandler* CPinger::CreateGrabHanlder()
{
	m_pGrabHandler = new CAnimatedGrabHandler(this);
	return m_pGrabHandler;
}

//////////////////////////////////////////////////////////////////////////
void CPinger::RagDollize(bool fallAndPlay)
{
	// Hunter do not ragdollize.
}

//////////////////////////////////////////////////////////////////////////
void CPinger::PostPhysicalize()
{
	CAlien::PostPhysicalize();

	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	IPhysicalEntity* pPhysEnt = pCharacter ? pCharacter->GetISkeletonPose()->GetCharacterPhysics(-1) : NULL;

	if (pPhysEnt)
	{
		pe_params_pos pp;
		pp.iSimClass = 2;
		pPhysEnt->SetParams(&pp);

		pe_simulation_params ps;
		ps.mass = 0;
		pPhysEnt->SetParams(&ps);
	}

	/*
		// GC 2007 : this is an unfortunate solution for a hunter physics collision bug [JAN]
		if (IPhysicalEntity *pPE = GetEntity()->GetPhysics())
		{
			pe_params_part params;
			params.ipart = 0;
			params.scale = 0;
			params.flagsAND = ~geom_colltype_player;
			pPE->SetParams(&params);
		}
	*/
}

bool CPinger::CreateCodeEvent(SmartScriptTable& rTable)
{
	const char* event = NULL;
	rTable->GetValue("event", event);

	if (event && !strcmp(event, "IKLook"))
	{
		bool lastIKLook(m_IKLook);
		rTable->GetValue("activate", m_IKLook);

		return true;
	}
	else if (event && !strcmp(event, "dropObject"))
	{
		if (m_pGrabHandler)
			m_pGrabHandler->SetDrop(rTable);

		return true;
	}
	else
		return CActor::CreateCodeEvent(rTable);
}

void CPinger::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_UNHIDE:
	{
		ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
		IPhysicalEntity* pPhysEnt = pCharacter ? pCharacter->GetISkeletonPose()->GetCharacterPhysics(-1) : NULL;

		if (pPhysEnt)
		{
			pe_params_pos pp;
			pp.iSimClass = 2;
			pPhysEnt->SetParams(&pp);

			pe_simulation_params ps;
			ps.mass = 0;
			pPhysEnt->SetParams(&ps);
		}
		break;
	}
	}
	CAlien::ProcessEvent(event);
}

void CPinger::Revive(bool fromInit)
{
	CAlien::Revive(fromInit);

	memset(m_footGroundPos, 0, sizeof(m_footGroundPos));
	memset(m_footGroundPosLast, 0, sizeof(m_footGroundPosLast));
	memset(m_IKLimbIndex, -1, sizeof(m_IKLimbIndex));
	memset(m_footSoundTime, 0, sizeof(m_footSoundTime));
	memset(m_footGroundSurface, 0, sizeof(m_footGroundSurface));
	memset(m_footAttachments, 0, sizeof(m_footAttachments));
	for (int i = 0; i < 3; ++i)
	{
		m_footTouchesGround[i] = false;
		m_footTouchesGroundSmooth[i] = 0.0f;
		m_footTouchesGroundSmoothRate[i] = 0.0f;
	}
	m_IKLook = false;

	//m_smoothMovementVec.Set(0, 0, 0);
	//m_balancePoint = GetEntity()->GetWorldPos();

	//m_nextStopCheck = 0.0f;

//	m_zDelta = 0.0f;

	m_turning = false;
	m_lastTurnSpeed = 0.5f;

	//m_smoothZ = GetEntity()->GetWorldPos().z;

	//m_zOffset = 0.0f;

	m_walkEventFlags.reset();

	//FIXME:
	if (m_pAnimatedCharacter)
	{
		SAnimatedCharacterParams params = m_pAnimatedCharacter->GetParams();

		params.flags &= ~(eACF_NoTransRot2k | eACF_ConstrainDesiredSpeedToXY/* | eACF_NoLMErrorCorrection*/);
		params.flags |= eACF_ImmediateStance | eACF_ConstrainDesiredSpeedToXY | eACF_ZCoordinateFromPhysics /*| eACF_NoLMErrorCorrection*/;

		m_pAnimatedCharacter->SetParams(params);
	}

	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	if (pCharacter)
	{
		pCharacter->SetFlags(pCharacter->GetFlags() | CS_FLAG_UPDATE_ALWAYS);

		//IK Limbs are init from Lua
		//TODO:this is temporary: remove it once the anim sys supports keyframe events
		if (m_IKLimbIndex[0] < 0)	m_IKLimbIndex[0] = GetIKLimbIndex("LeftTentacle");
		if (m_IKLimbIndex[1] < 0)	m_IKLimbIndex[1] = GetIKLimbIndex("RightTentacle");
		if (m_IKLimbIndex[2] < 0)	m_IKLimbIndex[2] = GetIKLimbIndex("BackTentacle");
		//if (m_IKLimbIndex[3] < 0)	m_IKLimbIndex[3] = GetIKLimbIndex("backRightTentacle");

		// create attachments for footstep fx
		IAttachmentManager* pAttMan = pCharacter->GetIAttachmentManager();
		for (int i = 0; i < 3; ++i)
		{
			if (m_IKLimbIndex[i] == -1)
				continue;

			SIKLimb* pLimb = &m_IKLimbs[m_IKLimbIndex[i]];
			const char* boneName = pCharacter->GetISkeletonPose()->GetJointNameByID(pLimb->endBoneID);
			if (boneName && boneName[0])
			{
				char attName[128];
				_snprintf(attName, sizeof(attName), "%s_effect_attach", boneName);
				attName[sizeof(attName) - 1] = 0;

				IAttachment* pAttachment = pAttMan->GetInterfaceByName(attName);
				if (!pAttachment)
					pAttachment = pAttMan->CreateAttachment(attName, CA_BONE, boneName);
				else
					pAttachment->ClearBinding();

				m_footAttachments[i] = pAttachment;

				//before we use CCD-IK the first time, we have to initialize the limp-position to the current FK-position
				pLimb->Update(GetEntity(), 0.000001f);
				Vec3 lPos(pLimb->lAnimPos.x, pLimb->lAnimPos.y, 0);
				m_footGroundPos[i] = GetEntity()->GetSlotWorldTM(pLimb->characterSlot) * lPos;
			}
		}
	}

	//TheOtherSide
	m_energyParams.isHunterShieldEnabled = false;
	m_energyParams.cannotRegen = false;
	//~TheOtherSide
}

bool CPinger::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (!CAlien::NetSerialize(ser, aspect, profile, flags))
		return false;
	return true;
}

void CPinger::UpdateFiringDir(float frameTime)
{
	m_stats.fireDir = m_viewMtx.GetColumn(1);//Vec3::CreateSlerp(m_stats.fireDir,m_viewMtx.GetColumn(1),1.9f*frameTime);
}

void CPinger::ProcessRotation(float frameTime)
{
	IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();

	if (!pPhysEnt)
		return;

	if (frameTime > 0.1f)
		frameTime = 0.1f;

	float rotSpeed(0.3f);

	if (m_input.viewVector.len2() > 0.0f)
	{
		// pvl Probably obsolete.
		m_eyeMtx.SetRotationVDir(m_input.viewVector.GetNormalizedSafe());
	}

	if (m_input.viewVector.len2() > 0.0f)
	{
		// pvl The same as m_eyeMtx above
		m_viewMtx.SetRotationVDir(m_input.viewVector.GetNormalizedSafe());
	}

	// compute the turn speed
	// NOTE Sep 13, 2007: <pvl> I suspect the turn speed has no influence on the hunter
	Vec3 forward(m_viewMtx.GetColumn(1));

	// pvl Check if the hunter is looking directly up or down.
	if (Vec3(forward.x, forward.y, 0).len2() > 0.001f)
	{
		//force the up vector to be 0,0,1 for now
		Vec3 up(0, 0, 1);
		Vec3 right = (up % forward).GetNormalized();

		Quat goalQuat(Matrix33::CreateFromVectors(right % up, right, up) * Matrix33::CreateRotationZ(gf_PI * -0.5f));
		Quat currQuat(m_baseMtx);

		float rotSpeed = m_params.rotSpeed_min + (1.0f - (max(GetStanceInfo(m_stance)->maxSpeed - max(m_stats.speed - m_params.speed_min, 0.0f), 0.0f) / GetStanceInfo(m_stance)->maxSpeed)) * (m_params.rotSpeed_max - m_params.rotSpeed_min);
		Interpolate(m_turnSpeed, rotSpeed, 3.0f / 100.0f, frameTime);
	}
}

void CPinger::ProcessMovement(float frameTime)
{
	IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();

	if (!pPhysEnt)
		return;

	auto pCharacter = GetEntity()->GetCharacter(0);
	if (!pCharacter)
		return;

	if (!m_pAnimatedCharacter)
		return;

	if (frameTime > 0.1f)
		frameTime = 0.1f;

	//movement
	Vec3 requestedMove;
	float	reqSpeed, maxSpeed;
	GetMovementVector(requestedMove, reqSpeed, maxSpeed);

	Vec3 move(requestedMove);
	move -= move * (m_baseMtx * Matrix33::CreateScale(Vec3(0, 0, 1)));//make it flat

	if (m_stats.sprintLeft)
		move *= m_params.sprintMultiplier;

	Vec3 viewFwd(m_viewMtx.GetColumn1() * 10);
	viewFwd.z = 0.0f;
	viewFwd.Normalize();

	Vec3 entFwd(GetEntity()->GetRotation().GetColumn1());
	entFwd.z = 0.0f;
	entFwd.Normalize();

	// how much deviate?
	const float dot = entFwd | viewFwd;

	// in which direction did pinger deviate?
	const float cross = entFwd.Cross(viewFwd).z;

	// 1.0 - left, 0.5 - idle, 0.0 - right
	float turnSpeed = 0.5f;
	float stopSpeed = 0;

	//Idle Turning
	if (move == Vec3(0,0,0))
	{
		m_moveRequest.velocity.zero();

		//FIX spin around
		m_stats.speed = 0;
		m_stats.desiredSpeed = 0;

		m_pAnimatedCharacter->SetMovementControlMethods(eMCM_Animation, eMCM_Entity);

		if ((!m_turning && (dot < s_turnThreshIdling)) || (m_turning && (dot < s_turnThreshTurning)))
		{
			if (cross > 0.0f)
			{
				//turn left
				turnSpeed = 1.0f;
			}
			else
			{
				//turn right
				turnSpeed = 0.0f;
			}
			m_turning = true;
		}
		else
		{

			//idle
			turnSpeed = 0.5f;
			stopSpeed = 3;
			m_turning = false;
		}

		Interpolate(m_lastTurnSpeed, turnSpeed, 5.5f - stopSpeed, frameTime);
		pCharacter->GetISkeletonAnim()->SetBlendSpaceOverride(eMotionParamID_TurnAngle, m_lastTurnSpeed, true);
	}
	else
	{
		//Move Turning
		m_pAnimatedCharacter->SetMovementControlMethods(eMCM_Entity, eMCM_Entity);

		const float rotationSpeed = 15.f;

		if (dot < s_turnThreshMoving)
		{
			if (cross > 0.0f)
			{
				//turn left
				m_moveRequest.rotation.SetRotationZ(DEG2RAD(15.0f) * frameTime * rotationSpeed);
			}
			else
			{
				//turn right
				m_moveRequest.rotation.SetRotationZ(-DEG2RAD(15.0f) * frameTime * rotationSpeed);
			}
		}
		else
		{
			//no turning
			m_moveRequest.rotation.SetIdentity();
		}
	}

	m_moveRequest.velocity = move;
	m_moveRequest.type = eCMT_Normal;
	m_moveRequest.prediction.nStates = 0;

	//FIXME:sometime
	m_stats.desiredSpeed = m_stats.speed;

	const string name = GetEntity()->GetName();
	if (name == "pingerX")
	{
		float color[] = { 1,1,1,1 };

		gEnv->pRenderer->Draw2dLabel(100,100,1.3f,color,false,"Cross %f", cross);
		gEnv->pRenderer->Draw2dLabel(100, 120, 1.3f, color, false, "Dot %f", dot);
		gEnv->pRenderer->Draw2dLabel(100, 140, 1.3f, color, false, "m_stats.speed %f", 
			m_stats.speed);		
		gEnv->pRenderer->Draw2dLabel(100, 160, 1.3f, color, false, "m_moveRequest.velocity (%f, %f, %f)", 
			m_moveRequest.velocity.x, m_moveRequest.velocity.y, m_moveRequest.velocity.z);		
		gEnv->pRenderer->Draw2dLabel(100, 180, 1.3f, color, false, "m_moveRequest.rotation.GetRotZ (%f)",
			m_moveRequest.rotation.GetRotZ());
		gEnv->pRenderer->Draw2dLabel(100, 200, 1.3f, color, false, "reqSpeed %f",
			reqSpeed);
		gEnv->pRenderer->Draw2dLabel(100, 220, 1.3f, color, false, "s_turnThreshIdling dot = %f",
			s_turnThreshIdling);
		gEnv->pRenderer->Draw2dLabel(100, 240, 1.3f, color, false, "s_turnThreshTurning dot = %f",
			s_turnThreshTurning);
		gEnv->pRenderer->Draw2dLabel(100, 260, 1.3f, color, false, "m_turning %i",
			m_turning);		
		gEnv->pRenderer->Draw2dLabel(100, 320, 1.3f, color, false, "m_lastTurnSpeed %f",
			m_lastTurnSpeed);
		gEnv->pRenderer->Draw2dLabel(100, 400, 1.3f, color, false, "turnSpeed %f",
			turnSpeed);
	}
}

void CPinger::ProcessAnimation(ICharacterInstance* pCharacter, float frameTime)
{
	GetISystem()->GetIRenderer()->GetIRenderAuxGeom()->SetRenderFlags(e_Def3DPublicRenderflags);

	if (!pCharacter)
		return;

	//float lookIKBlends[5];
	//lookIKBlends[0] = 0.00f;		// spine 1
	//lookIKBlends[1] = 0.00f;		// spine 2
	//lookIKBlends[2] = 0.00f;		// spine 3
	//lookIKBlends[3] = 0.00f;		// neck
	//lookIKBlends[4] = 1.00f;		// head
	pCharacter->GetISkeletonPose()->SetLookIK(1, DEG2RAD(120), m_stats.lookTargetSmooth);
}

void CPinger::PlayFootstepEffects(int tentacle) const
{
	IMaterialEffects* pMaterialEffects = g_pGame->GetIGameFramework()->GetIMaterialEffects();

	float footWaterLevel = gEnv->p3DEngine->GetWaterLevel(&m_footGroundPos[tentacle]);

	TMFXEffectId effectId = InvalidEffectId;
	if (footWaterLevel > m_footGroundPos[tentacle].z)
		effectId = pMaterialEffects->GetEffectId("hunter_footstep", CBullet::GetWaterMaterialId());
	else
		effectId = pMaterialEffects->GetEffectId("hunter_footstep", m_footGroundSurface[tentacle]);
	if (effectId != InvalidEffectId)
	{
		SMFXRunTimeEffectParams fxparams;
		fxparams.pos = m_footGroundPos[tentacle];
		fxparams.soundSemantic = eSoundSemantic_Physics_Footstep;
		pMaterialEffects->ExecuteEffect(effectId, fxparams);
	}
}

void CPinger::PlayFootliftEffects(int tentacle) const
{
	IMaterialEffects* pMaterialEffects = g_pGame->GetIGameFramework()->GetIMaterialEffects();

	float footWaterLevel = gEnv->p3DEngine->GetWaterLevel(&m_footGroundPos[tentacle]);

	TMFXEffectId effectId = InvalidEffectId;
	if (footWaterLevel > m_footGroundPos[tentacle].z)
		effectId = pMaterialEffects->GetEffectId("hunter_footlift", CBullet::GetWaterMaterialId());
	else
		effectId = pMaterialEffects->GetEffectId("hunter_footlift", m_footGroundSurface[tentacle]);
	if (effectId != InvalidEffectId)
	{
		SMFXResourceListPtr pList = pMaterialEffects->GetResources(effectId);
		if (pList && pList->m_particleList)
		{
			const char* effect = pList->m_particleList->m_particleParams.name;
			CEffectAttachment* pEffectAttachment = new CEffectAttachment(effect, Vec3Constants<float>::fVec3_Zero, Vec3Constants<float>::fVec3_OneY, 1.f);
			pEffectAttachment->CreateEffect();
			m_footAttachments[tentacle]->AddBinding(pEffectAttachment);
		}
	}
}

void CPinger::SetFiring(bool fire)
{
	if (fire != m_stats.isFiring)
	{
		if (!m_stats.isFiring)
			CreateScriptEvent("fireWeapon", 0);

		m_stats.isFiring = fire;
	}
}

int CPinger::GetBoneID(int ID, int slot) const
{
	if (m_boneIDs[ID] < 0)
	{
		ICharacterInstance* pCharacter = GetEntity()->GetCharacter(slot);
		if (!pCharacter)
			return -1;

		char boneStr[64];
		switch (ID)
		{
		case BONE_HEAD:		strcpy(boneStr, "turret_locker"); break;
		case BONE_WEAPON:	strcpy(boneStr, "nose_turret"); break;
		case BONE_EYE_R:	strcpy(boneStr, "nose_turret"); break;
		case BONE_EYE_L:	strcpy(boneStr, "nose_turret"); break;
		}

		m_boneIDs[ID] = pCharacter->GetISkeletonPose()->GetJointIDByName(boneStr);
	}

	return CActor::GetBoneID(ID, slot);
}

void CPinger::GetActorInfo(SBodyInfo& bodyInfo)
{
	CAlien::GetActorInfo(bodyInfo);

	int headBoneID = GetBoneID(BONE_HEAD);
	if (headBoneID > -1 && GetEntity()->GetCharacter(0))
	{
		//	Matrix33 HeadMat(GetEntity()->GetCharacter(0)->GetISkeleton()->GetAbsJMatrixByID(headBoneID));
		Matrix34 HeadMat(Matrix34(GetEntity()->GetCharacter(0)->GetISkeletonPose()->GetAbsJointByID(headBoneID)));
		HeadMat = GetEntity()->GetSlotWorldTM(0) * HeadMat;
		bodyInfo.vFirePos = HeadMat.GetColumn3();
		bodyInfo.vFireDir = bodyInfo.vEyeDir = HeadMat.GetColumn(1);
	}

	bodyInfo.vFireDir = bodyInfo.vEyeDir = m_viewMtx.GetColumn(1);

	//gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(bodyInfo.vEyePos, ColorB(0,255,0,100), bodyInfo.vEyePos + bodyInfo.vEyeDir * 10.0f, ColorB(255,255,0,100));
}

bool CPinger::SetAnimationInput(const char* inputID, const char* value)
{
	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	if (pCharacter && pCharacter->GetISkeletonAnim()->GetTrackViewStatus())
		return false;

	return CActor::SetAnimationInput(inputID, value);
}

void CPinger::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	GetAlienMemoryStatistics(s);
}

void CPinger::AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event)
{
	// NOTE Dez 19, 2006: <pvl> grabbing events
	if (strcmp(event.m_EventName, "ObjectGrabbed") == 0) {
		m_pGrabHandler->StartGrab();
	}
	else if (strcmp(event.m_EventName, "ObjectThrown") == 0) {
		m_pGrabHandler->StartDrop();
	}
	else if (strcmp(event.m_EventName, "StartIK") == 0) {
		(static_cast <CAnimatedGrabHandler*> (m_pGrabHandler))->ActivateIK();

		// NOTE Dez 19, 2006: <pvl> walking events
		//
		// NOTE Jul 31, 2007: <pvl> doesn't feel right but works: we accept all "down"
		// events but ignore "up" events if they aren't sent by the "most recent"
		// animation in the anim queue.  That's either the only animation in the queue or
		// the animation that's bleding in (while all others are blending out).
		//
		// This, with addition of "down" events to the beginning of non-walking animations
		// for every tentacle that's down at the beginning of that animation, helps
		// to prevent situation where an anim being blended out issues an "up" event
		// but doesn't manage to send the matching "down".  This can leave the tentacle
		// in the air for an indefinite amount of time, causing sliding.
	}
	else if (strcmp(event.m_EventName, "RightUp") == 0) {
		if (event.m_nAnimNumberInQueue == 0)
			m_walkEventFlags.set(RIGHT_UP);
	}
	else if (strcmp(event.m_EventName, "RightDown") == 0) {
		m_walkEventFlags.set(RIGHT_DOWN);
	}
	else if (strcmp(event.m_EventName, "LeftUp") == 0) {
		if (event.m_nAnimNumberInQueue == 0)
			m_walkEventFlags.set(LEFT_UP);
	}
	else if (strcmp(event.m_EventName, "LeftDown") == 0) {
		m_walkEventFlags.set(LEFT_DOWN);
	}
	else if (strcmp(event.m_EventName, "BackUp") == 0) {
		if (event.m_nAnimNumberInQueue == 0)
			m_walkEventFlags.set(BACK_UP);
	}
	else if (strcmp(event.m_EventName, "BackDown") == 0) {
		m_walkEventFlags.set(BACK_DOWN);
	}
	//else if (strcmp(event.m_EventName, "BackLeftUp") == 0) {
	//	if (event.m_nAnimNumberInQueue == 0)
	//		m_walkEventFlags.set(BACK_LEFT_UP);
	//}
	//else if (strcmp(event.m_EventName, "BackLeftDown") == 0) {
	//	m_walkEventFlags.set(BACK_LEFT_DOWN);
	//}
	//else if (strcmp(event.m_EventName, "BackRightUp") == 0) {
	//	if (event.m_nAnimNumberInQueue == 0)
	//		m_walkEventFlags.set(BACK_RIGHT_UP);
	//}
	//else if (strcmp(event.m_EventName, "BackRightDown") == 0) {
	//	m_walkEventFlags.set(BACK_RIGHT_DOWN);
	//}
	else
		CActor::AnimationEvent(pCharacter, event);
}

void CPinger::PlayAction(const char* action, const char* extension, bool looping)
{
	if (!m_pAnimatedCharacter)
		return;

	if (looping)
		m_pAnimatedCharacter->GetAnimationGraphState()->SetInput("Action", action);
	else
		m_pAnimatedCharacter->GetAnimationGraphState()->SetInput("Signal", action);
}

//void CPinger::ToggleShield()
//{
//	m_energyParams.isHunterShieldEnabled = !m_energyParams.isHunterShieldEnabled;
//}

void CPinger::UpdateEnergyRecharge(float frametime)
{
	bool  isServer = gEnv->bServer;
	float rechargeTime = m_energyParams.alienEnergyRechargeTime;
	float recharge = ALIEN_MAX_ENERGY / max(0.01f, rechargeTime);
	m_energyParams.alienEnergyRechargeRate = recharge;

	if (gEnv->bServer)
	{
		if (GetHealth() != 0 && !m_energyParams.cannotRegen && !m_energyParams.isHunterShieldEnabled)
		{
			if (GetAlienEnergy() <= 0.1f)
				m_energyParams.isHunterShieldEnabled = false;

			if (recharge < 0.0f || m_energyParams.alienEnergyRechargeDelay <= 0.0f)
			{
				SetAlienEnergy(clamp(m_energyParams.energy + recharge * frametime, 0.0f, ALIEN_MAX_ENERGY));
			}
		}
	}

	//recharge alien energy
	if (m_energyParams.alienEnergyRechargeDelay > 0.0f)
		m_energyParams.alienEnergyRechargeDelay = max(0.0f, m_energyParams.alienEnergyRechargeDelay - frametime);
}

template <typename T>
static void SerializeArray(TSerialize ser, T* arr, int elemCount, const string& label)
{
	const int labelBufSize = label.length() + 32;
	char* labelBuf = new char[labelBufSize];

	string labelFmt(label);
	labelFmt += "%d";

	for (int i = 0; i < elemCount; ++i)
	{
		_snprintf(labelBuf, labelBufSize, labelFmt.c_str(), i);
		ser.Value(labelBuf, arr[i]);
	}

	delete[] labelBuf;
}

void CPinger::FullSerialize(TSerialize ser)
{
	CAlien::FullSerialize(ser);

	ser.BeginGroup("CPinger");
	//ser.Value("m_smoothMovementVec", m_smoothMovementVec);
	//ser.Value("m_balancePoint", m_balancePoint);
	//ser.Value("m_zDelta", m_zDelta);
	ser.Value("m_turning", m_turning);

	SerializeArray(ser, m_IKLimbIndex, 4, "IKLimbIndex");
	SerializeArray(ser, m_footGroundPos, 4, "footGroundPos");
	SerializeArray(ser, m_footGroundPosLast, 4, "footGroundPosLast");
	SerializeArray(ser, m_footGroundSurface, 4, "footGroundSurface");
	SerializeArray(ser, m_footTouchesGround, 4, "footTouchesGround");
	SerializeArray(ser, m_footTouchesGroundSmooth, 4, "footTouchesGroundSmooth");
	SerializeArray(ser, m_footTouchesGroundSmoothRate, 4, "footTouchesGroundSmoothRate");
	SerializeArray(ser, m_footSoundTime, 4, "footSoundTime");

	ser.Value("m_IKLook", m_IKLook);
	//ser.Value("m_nextStopCheck", m_nextStopCheck);
	//ser.Value("m_smoothZ", m_smoothZ);
	//ser.Value("m_zOffset", m_zOffset);

	// NOTE Okt 6, 2007: <pvl> the explicit std::string/string casts in bitset
	// serialization are necessary because the engine plays some dirty tricks
	// with std::string (replacing it silently with something else) and
	// std::bitset requires std::string.
	if (ser.IsReading())
	{
		string walkEventFlagsStr;
		ser.Value("walkEventFlags", walkEventFlagsStr);
		m_walkEventFlags = std::bitset <NUM_WALK_EVENTS>(std::string(walkEventFlagsStr.c_str()));
	}
	else
	{
		// NOTE Okt 6, 2007: <pvl> STLP implementation of bitset::to_string() seems
		// to be a bit lacking.  It seems necessary to explicitly list std::string's
		// template arguments at the call site (I think MS STL doesn't have the problem).
		ser.Value("walkEventFlags", string(m_walkEventFlags.to_string<char, std::char_traits<char>, std::allocator<char> >().c_str()));
	}

	ser.EndGroup();
}

void CPinger::PostInit(IGameObject* pGameObject)
{
	CAlien::PostInit(pGameObject);
}