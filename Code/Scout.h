/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Implements the naked alien.

 -------------------------------------------------------------------------
  History:
  - 6:12:2004: Created by Filippo De Luca

*************************************************************************/
#ifndef __SCOUT_H__
#define __SCOUT_H__

#if _MSC_VER > 1000
# pragma once
#endif
#include "Alien.h"

class CAlienHealBomb :public CGameObjectExtensionHelper<CAlienHealBomb, IGameObjectExtension>
{
public:
	CAlienHealBomb();
	virtual ~CAlienHealBomb();

	// IGameObjectExtension
	virtual bool Init(IGameObject* pGameObject);
	virtual void InitClient(int channelId) {};
	virtual void PostInit(IGameObject* pGameObject);
	virtual void PostInitClient(int channelId) {};
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual void PostUpdate(float frameTime) {};
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent(const SGameObjectEvent&);
	virtual void ProcessEvent(SEntityEvent&);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryStatistics(ICrySizer* s);
	//~IGameObjectExtension

	bool Reset();
	//void DropBomb(float force = 5000.f, Vec3 target = Vec3(0, 0, 0));
	//void SetLandTarget(Vec3 Target);
	//void SetExplosionEffect();

	friend class CScout;

protected:
	static const int POSITION_ASPECT = eEA_GameServerStatic;

	IPhysicalEntity* m_pPhysicalEntity;

	// appearance of healing bomb
	IParticleEffect* m_pExplosionEffect;
	IParticleEmitter* m_pExplosionEmitter;
	bool	m_bPlanted;
	//bool	m_bDropped;
	float m_fMass;
	float m_fDensity;

	const char* m_sParticle;
	const char* m_sFileModel;
};

class CScout :
	public CAlien
{
public:

	CScout();
	~CScout();

	//TheOtherSide

	virtual void UpdateGlow(float energy);
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	//~TheOtherSide

	virtual IGrabHandler* CreateGrabHanlder();

	virtual bool CreateCodeEvent(SmartScriptTable& rTable);

	virtual void Revive(bool fromInit = false);

	virtual void ProcessRotation(float frameTime);
	virtual void ProcessSwimming(float frameTime);
	virtual void ProcessMovement(float frameTime);

	virtual void	SetActorStance(SMovementRequestParams& control, int& actions)
	{
		// Empty
	}
	virtual void AnimationEvent(ICharacterInstance*, const AnimEventInstance&);

	virtual void UpdateGrab(float frameTime);

	virtual void SetActorMovement(SMovementRequestParams& control);

	virtual void FullSerialize(TSerialize ser);
	//virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags );

	void GetMemoryStatistics(ICrySizer* s);

	//Player can't grab scouts
	virtual int	 GetActorSpecies() { return eGCT_UNKNOWN; }

	virtual void GetActorInfo(SBodyInfo& bodyInfo);
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	bool EnableSearchBeam(bool enable);
private:

	void ProcessMovementNew(float frameTime);
	void ProcessRotationNew(float frameTime);

protected:
};

#endif //__SCOUT_H__
