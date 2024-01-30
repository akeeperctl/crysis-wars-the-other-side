/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Implements the Trooper alien.

-------------------------------------------------------------------------
History:
- 21:7:2005: Created by Mikko Mononen

*************************************************************************/
#ifndef __TROOPER_H__
#define __TROOPER_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "Alien.h"

//TheOtherSide
#include "TheOtherSideMP/Actors/Aliens/TOSAlien.h"
//~TheOtherSide

constexpr int LOOKIK_BLEND_RATIOS = 5;

struct STrooperBeam
{
	bool active;
	int effectSlot;
	EntityId beamTargetId;

	STrooperBeam()
	{
		memset(this,0,sizeof(STrooperBeam));
	}

	void CreateBeam(IEntity *owner,const char *effect,EntityId targetId);
	void RemoveBeam(IEntity *owner);
	void Update(IEntity *owner,float frameTime);
};


class CTrooper :
	//TheOtherSide
	//public CAlien
	public CTOSAlien
	//~TheOtherSide
{
public:
	enum EJumpState {
		JS_None,
		JS_JumpStart,
		JS_ApplyImpulse,
		JS_Flying,
		JS_ApproachLanding,
		JS_Landing,
		JS_Landed,
		JS_Last = JS_Landed
	};

	struct SJumpParams 
	{
		Vec3 dest;
		Vec3 velocity;
		float duration;
		Vec3 addVelocity;
		EJumpState state;
		CTimeValue startTime;
		bool bUseLandAnim;
		bool bUseStartAnim;
		bool bFreeFall;
		bool bUseAnimEvent;
		bool bRelative;
		bool bUseSpecialAnim;
		bool bTrigger;
		EJumpAnimType specialAnimType;
		EAnimationMode specialAnimAGInput;
		string specialAnimAGInputValue;
		float landPreparationTime;
		float defaultLandPreparationTime;
		bool bPlayingSpecialAnim;
		float prevInAir;
		float landDepth;
		Vec3 curVelocity;
		Vec3 initLandVelocity;
		bool bUseLandEvent;

		SJumpParams():dest(ZERO),velocity(ZERO),addVelocity(ZERO), curVelocity(ZERO),initLandVelocity(ZERO)
		{
			Reset();
		}

		void Serialize (TSerialize ser )
		{
			ser.BeginGroup("JumpParams");
			ser.Value("dest",dest);
			ser.Value("velocity",velocity);
			ser.Value("curVelocity",curVelocity);
			ser.Value("addVelocity",addVelocity);
			ser.Value("bRelative",bRelative);
			ser.Value("duration",duration);
			ser.EnumValue("state",state, JS_None,JS_Last);
			ser.Value("bTrigger",bTrigger);
			ser.Value("startTime",startTime);
			ser.Value("bUseLandAnim",bUseLandAnim);
			ser.Value("bUseStartAnim",bUseStartAnim);
			ser.Value("bUseAnimEvent",bUseAnimEvent);
			ser.Value("bUseSpecialAnim",bUseSpecialAnim);
			ser.EnumValue("specialAnimType",specialAnimType,JUMP_ANIM_FLY,JUMP_ANIM_LAND);
			ser.EnumValue("specialAnimAGInput",specialAnimAGInput,AIANIM_SIGNAL,AIANIM_ACTION);
			ser.Value("specialAnimAGInputValue",specialAnimAGInputValue);
			ser.Value("defaultLandPreparationTime",defaultLandPreparationTime);
			ser.Value("landPreparationTime",landPreparationTime);
			ser.Value("bPlayingSpecialAnim",bPlayingSpecialAnim);
			ser.Value("prevInAir",prevInAir);
			ser.Value("landDepth",landDepth);
			ser.Value("initLandVelocity",initLandVelocity);
			ser.Value("bUseLandEvent",bUseLandEvent);
			ser.EndGroup();
		}


		void Reset()
		{
			dest = ZERO;
			velocity = ZERO;
			curVelocity = ZERO;
			addVelocity = ZERO;
			initLandVelocity = ZERO;
			duration = 0.f;
			state = JS_None;
			startTime = 0.f;
			bUseLandAnim = false;
			bFreeFall = false;
			bUseStartAnim = false;
			bUseAnimEvent = false;
			bRelative = false;
			bTrigger = false;
			bUseSpecialAnim = false;
			specialAnimType = JUMP_ANIM_FLY;
			specialAnimAGInput = AIANIM_ACTION;
			landPreparationTime = 0.1f;
			defaultLandPreparationTime = 0.1f;
			bPlayingSpecialAnim = false;
			prevInAir = 0;
			landDepth = 0;
			bUseLandEvent = false;
		}
	};
	//TheOtherSide
	//CTrooper() : CAlien(),
	CTrooper() : CTOSAlien(),
	//~TheOtherSide
		m_heightVariance(0),	
		m_heightVarianceLow(0),	
		m_heightVarianceHigh(0),
		m_heightVarianceFreq(0),
		m_heightVarianceRandomize(0)
	{
		m_modelQuat.SetIdentity();
	}

	void Revive(bool fromInit = false) override;

	void Update(SEntityUpdateContext&, int updateSlot) override;

	void SetActorMovement(SMovementRequestParams &control) override;

	void SetParams(SmartScriptTable &rTable,bool resetFirst) override;

	void ProcessRotation(float frameTime) override;
	void ProcessMovement(float frameTime) override;

	void ProcessAnimation(ICharacterInstance *pCharacter,float frameTime) override;

	void SetActorStance(SMovementRequestParams &control, int& actions) override;

	//virtual bool UpdateStance();

	void ResetAnimations() override;

	void UpdateStats(float frameTime) override;

	bool IsFlying() override
	{return false;}

	void BindInputs( IAnimationGraphState * pAGState ) override;

	//Player can grab troopers
	int	 GetActorSpecies() override
	{ return eGCT_TROOPER; }

	void GetMemoryStatistics(ICrySizer * s) override;

	void SetAnimTentacleParams(pe_params_rope& rope, float animBlend) override;

	void AnimationEvent(ICharacterInstance *pCharacter, const AnimEventInstance &event) override;

	void UpdateAnimGraph( IAnimationGraphState * pState ) override;

	void FullSerialize( TSerialize ser ) override;

private:
	void InitHeightVariance(SmartScriptTable &rTable);
	void Jump();
  void JumpEffect();
protected:
	typedef std::map<EStance,Vec3> TStanceMapCollSize;
	//Quat m_modelQuat;//the model rotation
	//QuatT m_modelAddQuat;//additional model rotation used for banking/tilting etc
	CTimeValue m_lastNotMovingTime;  
	f32		m_customLookIKBlends[LOOKIK_BLEND_RATIOS];
	float	m_oldSpeed;
	float m_heightVarianceLow;
	float m_heightVarianceHigh;
	float m_heightVariance;
	float m_heightVarianceFreq;
	float m_heightVarianceRandomize;

	float m_fDistanceToPathEnd;
	float m_Roll;
	float m_Rollx;
	float m_steerInertia;
	Vec3	m_landModelOffset;
	Vec3	m_steerModelOffset;
	Vec3  m_oldVelocity;
	//float m_Rollz;
	bool	m_bExactPositioning;
	CTimeValue m_lastExactPositioningTime;
	SJumpParams m_jumpParams;
	string m_overrideFlyAction;
	bool m_bOverrideFlyActionAnim;
	static const float CTentacle_maxTimeStep;
	static const float CMaxHeadFOR;
	CTimeValue m_lastTimeOnGround;
	Vec3 m_lastCheckEnvironmentPos;
	bool m_bNarrowEnvironment;
	//TStanceMapCollSize m_CollSize;
	float m_oldDirStrafe ;
	float m_oldDirFwd ;
	float m_fTtentacleBlendRotation;

	static const float ClandDuration;
	static const float ClandStiffnessMultiplier;


	IAnimationGraph::InputID m_idAngleXInput;
	IAnimationGraph::InputID m_idAngleZInput;
	IAnimationGraph::InputID m_idActionInput;
	IAnimationGraph::InputID m_idSignalInput;
	IAnimationGraph::InputID m_idMovementInput;
};


#endif //__TROOPER_H__
