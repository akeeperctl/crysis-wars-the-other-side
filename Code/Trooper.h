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

#define LOOKIK_BLEND_RATIOS 5

struct STrooperBeam
{
	bool active;
	int effectSlot;
	EntityId beamTargetId;

	STrooperBeam()
	{
		memset(this, 0, sizeof(STrooperBeam));
	}

	void CreateBeam(IEntity* owner, const char* effect, EntityId targetId);
	void RemoveBeam(IEntity* owner);
	void Update(IEntity* owner, float frameTime);
};

class CTrooper :
	public CAlien
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

		SJumpParams() :dest(ZERO), velocity(ZERO), addVelocity(ZERO), curVelocity(ZERO), initLandVelocity(ZERO)
		{
			Reset();
		}

		void Serialize(TSerialize ser)
		{
			ser.BeginGroup("JumpParams");
			ser.Value("dest", dest);
			ser.Value("velocity", velocity);
			ser.Value("curVelocity", curVelocity);
			ser.Value("addVelocity", addVelocity);
			ser.Value("bRelative", bRelative);
			ser.Value("duration", duration);
			ser.EnumValue("state", state, JS_None, JS_Last);
			ser.Value("bTrigger", bTrigger);
			ser.Value("startTime", startTime);
			ser.Value("bUseLandAnim", bUseLandAnim);
			ser.Value("bUseStartAnim", bUseStartAnim);
			ser.Value("bUseAnimEvent", bUseAnimEvent);
			ser.Value("bUseSpecialAnim", bUseSpecialAnim);
			ser.EnumValue("specialAnimType", specialAnimType, JUMP_ANIM_FLY, JUMP_ANIM_LAND);
			ser.EnumValue("specialAnimAGInput", specialAnimAGInput, AIANIM_SIGNAL, AIANIM_ACTION);
			ser.Value("specialAnimAGInputValue", specialAnimAGInputValue);
			ser.Value("defaultLandPreparationTime", defaultLandPreparationTime);
			ser.Value("landPreparationTime", landPreparationTime);
			ser.Value("bPlayingSpecialAnim", bPlayingSpecialAnim);
			ser.Value("prevInAir", prevInAir);
			ser.Value("landDepth", landDepth);
			ser.Value("initLandVelocity", initLandVelocity);
			ser.Value("bUseLandEvent", bUseLandEvent);
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

	CTrooper() : CAlien(),
		m_heightVariance(0),
		m_heightVarianceLow(0),
		m_heightVarianceHigh(0),
		m_heightVarianceFreq(0),
		m_heightVarianceRandomize(0)
	{
		m_modelQuat.SetIdentity();
		m_rageMode.pTrooper = this;
	}

	//TheOtherSide

	enum EShieldTypes
	{
		eShieldType_Owner,//Trooper is common trooper
		eShieldType_Guardian,//Trooper is the shields source guardian trooper
		eShieldType_Leader,//Trooper is heavy tank
		eShieldType_Last//Need for serialization
	};

	struct STrooperShieldParams
	{
		STrooperShieldParams()
		{
			//Shield owner energy
			energyRechargeDelay = 0;
			energyRechargeRate = 0;
			energy = 0;

			//Projecting shield
			isProjecting = 0;
			isInRange = 0;

			//By default all troopers have shields, but only guardians can spread shields to other troopers
			shieldType = eShieldType_Owner;

			modelPath = 0;
			modelSlot = 0;

			guardianId = 0;
			canGuardianShieldProj = 0;

			//Store the shield owners in guardian's storage
			shieldOwners.clear();
			beamEntities.clear();
			beamPointers.clear();
		}

		float GetRange() { return 15.0f; };

		void Serialize(TSerialize ser)
		{
			ser.BeginGroup("TrooperShieldParams");
			SER_VALUE(isProjecting);
			SER_VALUE(isInRange);
			SER_VALUE(guardianId);
			SER_VALUE(canGuardianShieldProj);
			SER_VALUE(modelSlot);
			//SER_VALUE(modelPath);
			SER_VALUE(energy);
			SER_VALUE(energyRechargeDelay);
			SER_VALUE(energyRechargeRate);
			SER_VALUE_ENUM(shieldType, eShieldType_Owner, eShieldType_Last);

			int iSize = shieldOwners.size();
			ser.Value("shieldOwnersLength", iSize);

			//only for vector serialize
			char szID[64];
			if (ser.IsReading())
			{
				for (int i = 0; i < iSize; i++)
				{
					sprintf(szID, "shieldOwnersID%d", i);
					EntityId id = 0;
					ser.Value(szID, id);
					shieldOwners.push_back(id);
				}
			}
			else
			{
				for (int i = 0; i < iSize; i++)
				{
					sprintf(szID, "shieldOwnersID%d", i);
					ser.Value(szID, shieldOwners[i]);
				}
			}
			ser.EndGroup();
		}

		//Shield owner energy
		float energyRechargeDelay;
		float energyRechargeRate;
		float energy;

		//Projecting shield
		bool isProjecting;

		//Trooper can project shield only when dist and energy are have normal values
		bool isInRange;

		//Trooper shield type, shield owner or shield source
		EShieldTypes shieldType;

		//Shield model path and trooper slot, where shield can exist
		const char* modelPath;
		float		modelSlot;

		//Guardian Id need for sort guarded/non guarded troopers
		EntityId guardianId;

		//When guardian have the player owner then player must press action to enable shield projection
		bool canGuardianShieldProj;

		//Store the shield owners in guardian's storage
		std::vector<EntityId> shieldOwners;

		//Key is shield owner's entity id, value is pointer to its particle emitter
		std::vector<IEntity*> beamEntities;

		//Key is shield owner's entity id, value is entityId to its particle emitter entity
		std::map<EntityId, EntityId> beamPointers;

		const int maxShieldOwners = 8;

		
	};

	struct STrooperLamStats
	{
		IEntity* pLamOneEntity;
		IEntity* pLamTwoEntity;
		IEntity* pLamThreeEntity;

		EntityId lamOneId;
		EntityId lamTwoId;
		EntityId lamThreeId;

		//bool isChildAttached;
		bool isActive;

		STrooperLamStats()
		{
			pLamOneEntity = 0;
			pLamTwoEntity = 0;
			pLamThreeEntity = 0;

			lamOneId = 0;
			lamTwoId = 0;
			lamThreeId = 0;

			isActive = false;
			//isChildAttached = false;
		}

		void Serialize(TSerialize ser)
		{
			ser.BeginGroup("TrooperLamParams");
			//SER_VALUE(lamOneId);
			//SER_VALUE(lamTwoId);
			//SER_VALUE(lamThreeId);
			SER_VALUE(isActive);
			//SER_VALUE(isChildAttached);
			ser.EndGroup();
		}
	};

	struct STrooperRageMode
	{
		STrooperRageMode()
		{
			Reset();
			pTrooper = 0;
		}

		void Update(const float frametime);
		void ToggleMode(const bool toggle, const float rageDuration = 0);

		float GetReloadDuration() const { return 60.0f; };
		float GetPostFXValue() const { return 0.15f; };

		void Reset()
		{
			reloadDuration = 0;
			rageMaxDuration = 0;
			isActive = false;
			isReloading = false;
		}

		void Serialize(TSerialize ser)
		{
			ser.BeginGroup("TrooperRageStats");
			SER_VALUE(reloadDuration);
			SER_VALUE(rageMaxDuration);
			SER_VALUE(isActive);
			SER_VALUE(isReloading);
			ser.EndGroup();
		}

		float reloadDuration;
		float rageMaxDuration;
		bool isActive;
		bool isReloading;
		CTrooper* pTrooper;
	};

	//virtual void DoSetState(const SSerializedAlienInput& input);

	virtual void UpdateGlow(float energy);

	virtual void Revive(bool fromInit = false);
	virtual void Reset(bool toGame);

	virtual void Update(SEntityUpdateContext&, int updateSlot);

	virtual void SetActorMovement(SMovementRequestParams& control);

	virtual void SetParams(SmartScriptTable& rTable, bool resetFirst);

	virtual void ProcessRotation(float frameTime);
	virtual void ProcessMovement(float frameTime);

	virtual void ProcessAnimation(ICharacterInstance* pCharacter, float frameTime);

	virtual void SetActorStance(SMovementRequestParams& control, int& actions);

	//virtual bool UpdateStance();

	virtual void ResetAnimations();

	virtual void UpdateStats(float frameTime);

	virtual bool IsFlying() { return false; }

	virtual void BindInputs(IAnimationGraphState* pAGState);

	//Player can grab troopers
	virtual int	 GetActorSpecies() { return eGCT_TROOPER; }

	void GetMemoryStatistics(ICrySizer* s);

	virtual void SetAnimTentacleParams(pe_params_rope& rope, float animBlend);

	virtual void AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event);

	virtual void UpdateAnimGraph(IAnimationGraphState* pState);

	//TheOtherSide
	void ProcessFlyControl(Vec3& move, float frameTime);

	void GetTrooperMovementVector(Vec3& move, float& speed, float& maxSpeed, float& sprintMult);
	virtual void Kill() override;

	virtual bool CreateCodeEvent(SmartScriptTable& rTable);
	virtual bool EnableLamLights(bool enable) override;
	void	ForceDisableLasers();

	void UpdateLamLights(float frametime, ICharacterInstance* character);
	void SetupLamLights();

	void SetLamLightsPos(Vec3 pos1);//Set laser pos in world space
	void SetLamLightsDir(Vec3 dir1);//Set laser dir from normalized vector

	virtual void OnAction(const ActionId& actionId, int activationMode, float value);
	//~TheOtherSide

	virtual SJumpParams GetJumpParams() { return m_jumpParams; };
	SJumpParams m_jumpParams;
	void FullSerialize(TSerialize ser);
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	void Jump();
	void JumpEffect();
protected:
	void InitHeightVariance(SmartScriptTable& rTable);

public:
	//TheOtherSide
	//Shield stuff;
	void SetupShieldStuff();//Called first when trooper's entity is creating
	void ProjectShield(bool project);//Called when guardian is be near around this shield owner and owner have energy for shield projecting

	//Shield energy manipulations
	void  SetShieldEnergy(float energy);
	float GetShieldEnergy();
	float GetMaxShieldEnergy();
	void  ResetShieldEnergy();

	//Update Shield stuff
	void UpdateShields(float frametime);

	//Shield Owner stuff
	void SetGuardianId(EntityId id);

	//Guardian stuff
	bool ApplyGuardianType();
	bool OnShockwaveCreated();

	//Leader Stuff
	bool ApplyLeaderType();

	bool ApplyCloakType();

public:
	STrooperRageMode m_rageMode;
	STrooperLamStats m_lamStats;
	STrooperShieldParams m_shieldParams;
	//~TheOtherSide
protected:
	typedef std::map<EStance, Vec3> TStanceMapCollSize;
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

	string m_overrideFlyAction;
	bool m_bOverrideFlyActionAnim;
	static const float CTentacle_maxTimeStep;
	static const float CMaxHeadFOR;
	CTimeValue m_lastTimeOnGround;
	Vec3 m_lastCheckEnvironmentPos;
	bool m_bNarrowEnvironment;
	//TStanceMapCollSize m_CollSize;
	float m_oldDirStrafe;
	float m_oldDirFwd;
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
