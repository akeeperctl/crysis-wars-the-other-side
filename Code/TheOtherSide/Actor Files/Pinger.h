#ifndef __PINGER_H__
#define __PINGER_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "Alien.h"

#include <bitset>

class CPinger :
	public CAlien
{
public:

	CPinger();

	virtual IGrabHandler* CreateGrabHanlder();

	virtual void Revive(bool fromInit);
	bool	NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual bool CreateCodeEvent(SmartScriptTable& rTable);

	virtual void ProcessRotation(float frameTime);
	virtual void ProcessMovement(float frameTime);
	virtual void PostInit(IGameObject* pGameObject);
	virtual void ProcessAnimation(ICharacterInstance* pCharacter, float frameTime);
	virtual void PostPhysicalize();
	virtual void RagDollize(bool fallAndPlay);
	virtual void ProcessEvent(SEntityEvent& event);

	virtual void UpdateFiringDir(float frameTime);

	virtual void	SetActorStance(SMovementRequestParams& control, int& actions)
	{
		// Empty
	}

	virtual void GetActorInfo(SBodyInfo& bodyInfo);
	virtual void AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event);

	virtual void SetFiring(bool fire);

	virtual int GetBoneID(int ID, int slot = 0) const;

	virtual bool IsFlying() { return false; }

	virtual bool SetAnimationInput(const char* inputID, const char* value);

	virtual void FullSerialize(TSerialize ser);

	void GetMemoryStatistics(ICrySizer* s);

	//Player can't grab hunters
	virtual int	 GetActorSpecies() { return eGCT_UNKNOWN; }

	virtual void PlayAction(const char* action, const char* extension, bool looping);

	//TheOtherSide
	//bool IsShieldEnabled() { return m_energyParams.isHunterShieldEnabled; }
	//void ToggleShield();
	virtual void UpdateEnergyRecharge(float frametime) override;
	//tSoundID m_shieldSoundId;
	//~TheOtherSide

protected:

	void PlayFootstepEffects(int tentacle) const;
	void PlayFootliftEffects(int tentacle) const;

	bool TentacleInTheAir(int tentacle) const;
	/// These functions work from the animation point of view, unaltered by IK.
	/// See also m_footTouchesGround.
	bool TentacleGoingUp(int tentacle) const;
	bool TentacleGoingDown(int tentacle) const;

	bool Airborne() const;

	//Vec3 m_smoothMovementVec;
	//Vec3 m_balancePoint;
	//float m_zDelta;

	/// 'true' if the hunter is turning around.  We need to know because the decision
	/// whether to turn in this frame depends on whether the hunter's already turning.
	bool m_turning;
	/// Both given as the maximum dot product of (normalized) desired and actual
	/// directions for turning to happen.
	static const float s_turnThreshIdling;
	static const float s_turnThreshTurning;
	static const float s_turnThreshMoving;

	float m_lastTurnSpeed;

	//
	int m_IKLimbIndex[3];
	Vec3 m_footGroundPos[3];
	Vec3 m_footGroundPosLast[3];
	int  m_footGroundSurface[3];
	/// True if tentacle's final world position, after all adjustments, is on the ground.
	bool m_footTouchesGround[3];
	f32 m_footTouchesGroundSmooth[3];
	f32 m_footTouchesGroundSmoothRate[3];
	IAttachment* m_footAttachments[3];

	bool m_IKLook;

	//float m_nextStopCheck;

	//TEMPORARY
	float m_footSoundTime[3];

	//float m_smoothZ;
	//float m_zOffset;

	enum WalkEventIndex {
		LEFT_UP = 0,
		LEFT_DOWN,
		RIGHT_UP,
		RIGHT_DOWN,
		BACK_UP,
		BACK_DOWN,
		NUM_WALK_EVENTS
	};
	std::bitset<NUM_WALK_EVENTS> m_walkEventFlags;

	//IAnimationGraph::InputID m_inputAngleDeviation;
	//float m_angleDeviation;
};

inline bool CPinger::TentacleInTheAir(int tentacle) const
{
	return m_footGroundPos[tentacle] == Vec3(0.0f, 0.0f, 0.0f);
}

inline bool CPinger::TentacleGoingUp(int tentacle) const
{
	return m_walkEventFlags.test(2 * tentacle);
}

inline bool CPinger::TentacleGoingDown(int tentacle) const
{
	return m_walkEventFlags.test(2 * tentacle + 1);
}

inline bool CPinger::Airborne() const
{
	return false; //pretend the hunter is always on ground

	// m_stats.inAir indicates if the hunter as a whole is on the ground
	if (m_stats.inAir > 2.0f)
		return true;
	return false;
}

#endif //__PINGER_H__
