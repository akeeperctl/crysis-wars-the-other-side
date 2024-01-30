/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: C++ Weapon Implementation

-------------------------------------------------------------------------
History:
- 22:8:2005   12:50 : Created by Mбrcio Martins

*************************************************************************/
#ifndef __WEAPON_H__
#define __WEAPON_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IAgent.h>
#include <IMusicSystem.h>
#include <IWeapon.h>
#include <VectorMap.h>

#include "Item.h"

constexpr float WEAPON_FADECROSSHAIR_SELECT	= 0.250f;
constexpr float WEAPON_FADECROSSHAIR_ZOOM	= 0.200f;
constexpr float WEAPON_SHOOT_TIMER			= 5000;

class CProjectile;

class CWeapon : public CItem, public IWeapon
{
	class ScheduleLayer_Leave;
	class ScheduleLayer_Enter;
	struct EndChangeFireModeAction;
	struct PlayLeverLayer;

protected:
	typedef std::map<string, int>        TFireModeIdMap;
	typedef std::vector<IFireMode*>      TFireModeVector;
	typedef std::map<string, int>        TZoomModeIdMap;
	typedef std::vector<IZoomMode*>      TZoomModeVector;
	typedef std::map<IEntityClass*, int> TAmmoMap;

	struct SListenerInfo
	{
		IWeaponEventListener* pListener;
#ifdef _DEBUG
		char who[64];
#endif
	};

	typedef std::vector<SListenerInfo> TEventListenerVector;

	typedef struct SWeaponCrosshairStats
	{
		SWeaponCrosshairStats()
			: fading(false),
			visible(true),
			fadefrom(1.0f),
			fadeto(1.0f),
			fadetime(0.0f),
			fadetimer(0.0f),
			opacity(1.0f) { }

		bool  fading;
		bool  visible;
		float fadefrom;
		float fadeto;
		float fadetime;
		float fadetimer;
		float opacity;
	};

public:
	CWeapon();
	~CWeapon() override;

	// IItem, IGameObjectExtension
	bool Init(IGameObject* pGameObject) override;

	void InitClient(int channelId) override
	{
		CItem::InitClient(channelId);
	};
	void Release() override;
	void FullSerialize(TSerialize ser) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void PostSerialize() override;
	void SerializeLTL(TSerialize ser) override;
	void Update(SEntityUpdateContext& ctx, int) override;
	void PostUpdate(float frameTime) override;
	void HandleEvent(const SGameObjectEvent&) override;
	void ProcessEvent(SEntityEvent& event) override;
	void SetChannelId(uint16 id) override {};
	void SetAuthority(bool auth) override;
	void GetMemoryStatistics(ICrySizer* s) override;

	void Reset() override;

	void OnAction(EntityId actorId, const ActionId& actionId, int activationMode, float value) override;
	void UpdateFPView(float frameTime) override;

	IWeapon* GetIWeapon() override
	{
		return this;
	};

	void MeleeAttack() override;
	bool CanMeleeAttack() const override;

	virtual IFireMode* GetMeleeFireMode() const
	{
		return m_melee;
	};

	void PerformThrow(float speedScale) override {};

	void Select(bool select) override;
	void Drop(float impulseScale, bool selectNext = true, bool byDeath = false) override;
	void Freeze(bool freeze) override;

	void OnPickedUp(EntityId actorId, bool destroyed) override;
	void OnDropped(EntityId actorId) override;

	void OnHit(float damage, const char* damageType) override;
	void OnDestroyed() override;
	void EnterWater(bool enter) override;

	//Needed for the mounted weapon
	void StartUse(EntityId userId) override;
	void StopUse(EntityId userId) override;

	bool         CheckAmmoRestrictions(EntityId pickerId) override;
	virtual bool CanUseAmmo(IEntityClass* pAmmoType);

	bool  SetAspectProfile(EEntityAspects aspect, uint8 profile) override;
	uint8 GetDefaultProfile(EEntityAspects aspect) override;

	bool FilterView(SViewParams& viewParams) override;
	void PostFilterView(struct SViewParams& viewParams) override;

	// ~IItem
	virtual bool HasAttachmentAtHelper(const char* helper);
	virtual void GetAttachmentsAtHelper(const char* helper, std::vector<string>& rAttachments);
	// Events
	virtual void OnShoot(EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel);
	virtual void OnStartFire(EntityId shooterId);
	virtual void OnStopFire(EntityId shooterId);
	virtual void OnStartReload(EntityId shooterId, IEntityClass* pAmmoType);
	virtual void OnEndReload(EntityId shooterId, IEntityClass* pAmmoType);
	virtual void OnOutOfAmmo(IEntityClass* pAmmoType);
	virtual void OnReadyToFire();
	virtual void OnMelee(EntityId shooterId);
	virtual void OnStartTargetting(IWeapon* pWeapon);
	virtual void OnStopTargetting(IWeapon* pWeapon);
	void         OnSelected(bool selected) override;

	// viewmodes
	virtual void IncrementViewmode();
	virtual void ExitViewmodes();

	// IWeapon
	void                  SetFiringLocator(IWeaponFiringLocator* pLocator) override;
	IWeaponFiringLocator* GetFiringLocator() const override;

	void AddEventListener(IWeaponEventListener* pListener, const char* who) override;
	void RemoveEventListener(IWeaponEventListener* pListener) override;

	void SetDestinationEntity(EntityId targetId) override;

	void SetDestination(const Vec3& pos) override
	{
		m_destination = pos;
	}

	const Vec3& GetDestination() override
	{
		return m_destination;
	}

	Vec3 GetFiringPos(const Vec3& probableHit) const override;
	Vec3 GetFiringDir(const Vec3& probableHit, const Vec3& firingPos) const override;

	void StartFire() override;
	void StopFire() override;
	bool CanFire() const override;

	void StartZoom(EntityId shooterId, int zoomed = 0) override;
	void StopZoom(EntityId shooterId) override;
	bool CanZoom() const override;
	void ExitZoom() override;
	bool IsZoomed() const override;
	bool IsZooming() const override;

	bool IsReloading() const override;

	void MountAt(const Vec3& pos) override;
	void MountAtEntity(EntityId entityId, const Vec3& pos, const Ang3& angles) override;

	void Reload(bool force = false) override;
	bool CanReload() const override;

	bool OutOfAmmo(bool allFireModes) const override;

	virtual float       GetDamage(int fmId, float distance) const;
	virtual const char* GetDamageType(int fmId) const;

	int  GetAmmoCount(IEntityClass* pAmmoType) const override;
	void SetAmmoCount(IEntityClass* pAmmoType, int count) override;

	virtual int  GetInventoryAmmoCount(IEntityClass* pAmmoType) const;
	virtual void SetInventoryAmmoCount(IEntityClass* pAmmoType, int count);

	int GetNumOfFireModes() const override
	{
		return m_firemodes.size();
	}

	IFireMode*   GetFireMode(int idx) const override;
	IFireMode*   GetFireMode(const char* name) const override;
	int          GetFireModeIdx(const char* name) const override;
	int          GetCurrentFireMode() const override;
	void         SetCurrentFireMode(int idx) override;
	void         SetCurrentFireMode(const char* name) override;
	void         ChangeFireMode() override;
	virtual int  GetNextFireMode(int currMode) const;
	virtual void EnableFireMode(int idx, bool enable);
	void         FixAccessories(SAccessoryParams* newParams, bool attach) override;

	IZoomMode*   GetZoomMode(int idx) const override;
	IZoomMode*   GetZoomMode(const char* name) const override;
	int          GetZoomModeIdx(const char* name) const override;
	int          GetCurrentZoomMode() const override;
	void         SetCurrentZoomMode(int idx) override;
	void         SetCurrentZoomMode(const char* name) override;
	void         ChangeZoomMode() override;
	virtual void EnableZoomMode(int idx, bool enable);
	virtual void RestartZoom(bool force = false);

	virtual void SetCrosshairVisibility(bool visible);
	bool         GetCrosshairVisibility() const override;
	virtual void SetCrosshairOpacity(float opacity);
	float        GetCrosshairOpacity() const override;
	virtual void FadeCrosshair(float from, float to, float time);
	virtual void UpdateCrosshair(float frameTime);

	void         AccessoriesChanged() override;
	virtual void PatchFireModeWithAccessory(IFireMode* pFireMode, const char* firemodeName);
	virtual void PatchZoomModeWithAccessory(IZoomMode* pZoomMode, const char* zoommodeName);

	float GetSpinUpTime() const override;
	float GetSpinDownTime() const override;

	void     SetHostId(EntityId hostId) override;
	EntityId GetHostId() const override;

	bool PredictProjectileHit(IPhysicalEntity* pShooter, const Vec3& pos, const Vec3& dir, const Vec3& velocity, float speed, Vec3& predictedPosOut, float& projectileSpeedOut, Vec3* pTrajectory = nullptr, unsigned int* trajectorySizeInOut = nullptr) const override;

	void ForceHitMaterial(int surfaceId) override
	{
		m_forcedHitMaterial = surfaceId;
	};

	int GetForcedHitMaterial() const override
	{
		return m_forcedHitMaterial;
	};

	const AIWeaponDescriptor& GetAIWeaponDescriptor() const override;

	//Activate/Deactivate Laser and Light for the AI (also player if neccessary)
	bool IsLamAttached() override;
	bool IsFlashlightAttached() override;
	void ActivateLamLaser(bool activate, bool aiRequest = true) override;
	void ActivateLamLight(bool activate, bool aiRequest = true) override;
	bool IsLamLaserActivated() override;
	bool IsLamLightActivated() override;
	void RaiseWeapon(bool raise, bool faster = false) override;

	float GetRecoilAmount() const override;

	// ~IWeapon

	int  GetMaxZoomSteps();
	void AssistAiming(float magnification = 1.0f, bool accurate = false);
	bool IsValidAssistTarget(IEntity* pEntity, IEntity* pSelf, bool includeVehicles = false);

	void AdvancedAssistAiming(float range, const Vec3& pos, Vec3& dir);

	bool IsSilencerAttached() const
	{
		return m_silencerAttached;
	}

	//	bool		IsLAMAttached() { return m_lamAttached; }

	//Activate/Deactivate Laser and Light for the AI (also player if neccessary)
	//	void    ActivateLamLaser(bool activate, bool aiRequest = true);
	//	void		ActivateLamLight(bool activate, bool aiRequest = true);

	void       StartChangeFireMode();
	void       EndChangeFireMode();
	ILINE bool IsSwitchingFireMode() const
	{
		return m_switchingFireMode;
	};
	ILINE bool IsFiring() const
	{
		return m_fm ? m_fm->IsFiring() : false;
	}

	//Targeting stuff
	bool IsTargetOn() const
	{
		return m_targetOn;
	}

	void ActivateTarget(bool activate)
	{
		m_targetOn = activate;
	}

	void SetAimLocation(const Vec3& location)
	{
		m_aimPosition = location;
	}

	void SetTargetLocation(const Vec3& location)
	{
		m_targetPosition = location;
	}

	Vec3& GetAimLocation()
	{
		return m_aimPosition;
	}

	Vec3& GetTargetLocation()
	{
		return m_targetPosition;
	}

	//Raise weapon
	ILINE virtual bool IsWeaponRaised() const
	{
		return (m_weaponRaised);
	}

	ILINE virtual float GetRaiseDistance()
	{
		return m_params.raise_distance;
	}

	ILINE virtual void SetWeaponRaised(bool raise)
	{
		m_weaponRaised = raise;
	}

	ILINE virtual uint8 GetRaisePose()
	{
		return m_raisePose;
	}

	ILINE virtual bool CanBeRaised()
	{
		return m_params.raiseable;
	}

	virtual void UpdateWeaponRaising(float frameTime);
	virtual void UpdateWeaponLowering(float frameTime);

	ILINE virtual void LowerWeapon(const bool lower)
	{
		m_weaponLowered = lower;
	}

	ILINE virtual bool IsWeaponLowered()
	{
		return m_weaponLowered;
	}

	ILINE virtual bool IsPendingFireRequest()
	{
		return m_requestedFire;
	}

	bool GetFireAlternation() const
	{
		return m_fire_alternation;
	}

	void SetFireAlternation(bool fireAlt)
	{
		m_fire_alternation = fireAlt;
	}

	//LAW special stuff
	void AutoDrop(bool doCount = true);
	void DoDrop();

	void AddFiredRocket()
	{
		m_firedRockets++;
	};

	//AI using dual wield socoms
	bool FireSlave(EntityId actorId, bool fire);
	void ReloadSlave();

	virtual EntityId GetHeldEntityId() const
	{
		return 0;
	}

	virtual void SendMusicLogicEvent(EMusicLogicEvents event);

	//Scopes
	void OnZoomIn();
	void OnZoomOut();
	bool GetScopePosition(Vec3& pos);

	// network
	struct WeaponRaiseParams
	{
		WeaponRaiseParams()
			: raise(false) {}

		WeaponRaiseParams(bool _raise)
			: raise(_raise) {}

		bool raise;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("raise", raise);
		}
	};

	struct SvRequestShootParams
	{
		SvRequestShootParams() {};

		SvRequestShootParams(const Vec3& pos_, const Vec3& dir_, const Vec3& at, int ph, uint16 seqn, uint8 seqnr)
			: pos(pos_),
			dir(dir_),
			hit(at),
			seq(seqn),
			seqr(seqnr),
			predictionHandle(ph) {};

		Vec3   pos;
		Vec3   dir;
		Vec3   hit;
		uint16 seq;
		uint8  seqr;
		int    predictionHandle;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir3');
			ser.Value("hit", hit, 'sHit');
			ser.Value("seq", seq, 'ui16');
			ser.Value("seqr", seqr, 'ui5');
			ser.Value("predictionHandle", predictionHandle, 'phdl');
		};
	};

	struct ClShootParams
	{
		ClShootParams() {};

		ClShootParams(const Vec3& at, int ph)
			: hit(at),
			predictionHandle(ph) {};

		Vec3 hit;
		int  predictionHandle;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("hit", hit, 'sHit');
			ser.Value("predictionHandle", predictionHandle, 'phdl');
		};
	};

	struct ClShootXParams
	{
		ClShootXParams() {};

		ClShootXParams(EntityId eid_, bool hit0_, const Vec3& hit_, int ph)
			: eid(eid_),
			hit0(hit0_),
			hit(hit_),
			predictionHandle(ph) {};

		EntityId eid;
		bool     hit0;
		Vec3     hit;
		int      predictionHandle;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("ref_eid", eid, 'eid');
			ser.Value("use_hit", hit0, 'bool');
			ser.Value("ref_hit", hit, hit0 ? 'hit0' : 'hit1');
			ser.Value("predictionHandle", predictionHandle, 'phdl');
		}
	};

	struct SvRequestShootExParams
	{
		SvRequestShootExParams() {};

		SvRequestShootExParams(const Vec3& _pos, const Vec3& _dir, const Vec3& _vel, const Vec3& _hit, float _extra, int ph, uint16 seqn, uint8 seqnr)
			: pos(_pos),
			dir(_dir),
			vel(_vel),
			hit(_hit),
			seq(seqn),
			seqr(seqnr),
			extra(_extra),
			predictionHandle(ph) {};

		Vec3   pos;
		Vec3   dir;
		Vec3   vel;
		Vec3   hit;
		uint16 seq;
		uint8  seqr;
		float  extra;
		int    predictionHandle;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir3');
			ser.Value("vel", vel, 'vel0');
			ser.Value("hit", hit, 'wrld');
			ser.Value("extra", extra, 'smal');
			ser.Value("seq", seq, 'ui16');
			ser.Value("seqr", seqr, 'ui5');
			ser.Value("predictionHandle", predictionHandle, 'phdl');
		};
	};

	struct SvRequestFireModeParams
	{
		SvRequestFireModeParams()
			: id(0) {};

		SvRequestFireModeParams(int fmId)
			: id(fmId) {};

		int id;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'fmod');
		};
	};

	struct RequestMeleeAttackParams
	{
		RequestMeleeAttackParams() {};

		RequestMeleeAttackParams(bool _wmelee, const Vec3& _pos, const Vec3& _dir, uint16 seqn)
			: wmelee(_wmelee),
			pos(_pos),
			dir(_dir),
			seq(seqn) {};

		bool   wmelee; // is this the special weapon melee mode?
		Vec3   pos;
		Vec3   dir;
		uint16 seq;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("wmelee", wmelee, 'bool');
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir3');
			ser.Value("seq", seq, 'ui16');
		}
	};

	struct ClMeleeAttackParams
	{
		ClMeleeAttackParams() {};

		ClMeleeAttackParams(bool _wmelee, const Vec3& _pos, const Vec3& _dir)
			: wmelee(_wmelee),
			pos(_pos),
			dir(_dir) {};

		bool wmelee; // is this the special weapon melee mode?
		Vec3 pos;
		Vec3 dir;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("wmelee", wmelee, 'bool');
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir3');
		}
	};

	struct RequestStartMeleeAttackParams
	{
		RequestStartMeleeAttackParams() {};

		RequestStartMeleeAttackParams(bool _wmelee)
			: wmelee(_wmelee) {};

		bool wmelee; // is this the special weapon melee mode?

		void SerializeWith(TSerialize ser)
		{
			ser.Value("wmelee", wmelee, 'bool');
		}
	};

	struct ClSetFireModeParams
	{
		ClSetFireModeParams()
			: id(0) {};

		ClSetFireModeParams(int fmId)
			: id(fmId) {};

		int id;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'fmod');
		};
	};

	struct EmptyParams
	{
		EmptyParams() {};
		void SerializeWith(TSerialize ser) {}
	};

	struct LockParams
	{
		LockParams()
			: entityId(0),
			partId(0) {};

		LockParams(EntityId id, int part)
			: entityId(id),
			partId(part) {};

		EntityId entityId;
		int      partId;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("entityId", entityId, 'eid');
			ser.Value("partId", partId);
		}
	};

	struct ZoomParams
	{
		ZoomParams()
			: fov(0) {};

		ZoomParams(float _fov)
			: fov(_fov) {};

		float fov;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("fov", fov, 'frad');
		}
	};

	static constexpr EEntityAspects ASPECT_FIREMODE = eEA_GameServerStatic;
	static constexpr EEntityAspects ASPECT_RAISED   = eEA_GameClientDynamic;
	static constexpr EEntityAspects ASPECT_AMMO     = eEA_GameServerDynamic;

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestShoot, SvRequestShootParams, eNRT_ReliableUnordered);
	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestShootEx, SvRequestShootExParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClShoot, ClShootParams, eNRT_UnreliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClShootX, ClShootXParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestStartFire, EmptyParams, eNRT_ReliableUnordered);
	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestStopFire, EmptyParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClStartFire, EmptyParams, eNRT_UnreliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClStopFire, EmptyParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestStartSecondaryFire, EmptyParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClStartSecondaryFire, EmptyParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestStartMeleeAttack, RequestStartMeleeAttackParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClStartMeleeAttack, RequestStartMeleeAttackParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestMeleeAttack, RequestMeleeAttackParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClMeleeAttack, ClMeleeAttackParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestZoom, ZoomParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClZoom, ZoomParams, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvRequestFireMode, SvRequestFireModeParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_POSTATTACH(ClSetFireMode, ClSetFireModeParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvRequestReload, EmptyParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH(ClReload, EmptyParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_POSTATTACH(ClEndReload, EmptyParams, eNRT_ReliableUnordered);

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestCancelReload, EmptyParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClCancelReload, EmptyParams, eNRT_ReliableOrdered);

	DECLARE_CLIENT_RMI_NOATTACH(ClLock, LockParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClUnlock, EmptyParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvRequestLock, LockParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvRequestUnlock, EmptyParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvRequestWeaponRaised, WeaponRaiseParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH(ClWeaponRaised, WeaponRaiseParams, eNRT_ReliableUnordered);

	virtual int  NetGetCurrentAmmoCount() const;
	virtual void NetSetCurrentAmmoCount(int count);

	virtual void NetShoot(const Vec3& hit, int predictionHandle);
	virtual void NetShootEx(const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, float extra, int predictionHandle);

	virtual void NetStartFire();
	virtual void NetStopFire();
	virtual void NetStartSecondaryFire();

	virtual void NetStartMeleeAttack(bool weaponMelee);
	virtual void NetMeleeAttack(bool weaponMelee, const Vec3& pos, const Vec3& dir);

	virtual void NetZoom(float fov);

	uint16 ILINE GenerateShootSeqN()
	{
		++m_shootSeqN;
		if (m_shootSeqN == 0)
			++m_shootSeqN;
		return m_shootSeqN;
	};

	uint16 ILINE GetShootSeqN() const
	{
		return m_shootSeqN;
	};

	void SendEndReload();
	void RequestShoot(IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, float extra, int predictionHandle, uint16 seq, uint8 seqr, bool forceExtended);
	void RequestStartFire();
	void RequestStopFire();
	void RequestReload();
	void RequestFireMode(int fmId);
	void RequestWeaponRaised(bool raise);
	void RequestStartSecondaryFire();

	void RequestStartMeleeAttack(bool weaponMelee);
	void RequestMeleeAttack(bool weaponMelee, const Vec3& pos, const Vec3& dir, uint16 seq);
	void RequestZoom(float fov);

	void RequestCancelReload();
	void RequestLock(EntityId id, int partId = 0);
	void RequestUnlock();

	bool         IsServerSpawn(IEntityClass* pAmmoType) const;
	CProjectile* SpawnAmmo(IEntityClass* pAmmoType, bool remote = false);

	bool AIUseEyeOffset() const
	{
		return m_AIUseEyeOffset;
	}

	bool AIUseOverrideOffset(EStance stance, float lean, Vec3& offset) const;

	virtual bool ApplyActorRecoil() const
	{
		return true;
	}

	void ForcePendingActions() override;

protected:
	bool                   ReadItemParams(const IItemParamsNode* params) override;
	const IItemParamsNode* GetFireModeParams(const char* name);
	const IItemParamsNode* GetZoomModeParams(const char* name);
	void                   InitFireModes(const IItemParamsNode* firemodes);
	void                   InitZoomModes(const IItemParamsNode* zoommodes);
	void                   InitAmmos(const IItemParamsNode* ammo);
	void                   InitAIData(const IItemParamsNode* aiDescriptor);
	void                   InitAIOffsets(const IItemParamsNode* aiOffsetData);

	EntityId GetLAMAttachment();
	EntityId GetFlashlightAttachment();

	void SetNextShotTime(bool activate);

	IFireMode* m_fm;

	IFireMode* m_melee;

	IZoomMode* m_zm;
	int        m_zmId;

	TFireModeIdMap  m_fmIds;
	TFireModeVector m_firemodes;

	TZoomModeIdMap  m_zmIds;
	TZoomModeVector m_zoommodes;

	TAmmoMap m_ammo;
	TAmmoMap m_bonusammo;
	TAmmoMap m_accessoryAmmo;
	TAmmoMap m_minDroppedAmmo; //SP only (AI drops always a minimum amount)

	bool m_fire_alternation;

	bool m_restartZoom; //this is a serialization helper
	int  m_restartZoomStep;
	int  m_zoomViewMode;

	const IItemParamsNode* m_fmDefaults;
	const IItemParamsNode* m_zmDefaults;
	const IItemParamsNode* m_xmlparams;

	TEventListenerVector         m_listeners;
	static TEventListenerVector* m_listenerCache;
	static bool                  m_listenerCacheInUse;
	IWeaponFiringLocator*        m_pFiringLocator;

	int m_forcedHitMaterial;

	SWeaponCrosshairStats m_crosshairstats;

	float m_dofValue;
	float m_dofSpeed;
	float m_focusValue;

	Vec3 m_destination;

	AIWeaponDescriptor               m_aiWeaponDescriptor;
	bool                             m_AIUseEyeOffset;
	typedef VectorMap<EStance, Vec3> TStanceWeaponOffset;
	TStanceWeaponOffset              m_StanceWeponOffset;
	TStanceWeaponOffset              m_StanceWeponOffsetLeanLeft;
	TStanceWeaponOffset              m_StanceWeponOffsetLeanRight;

	std::vector<ItemString> m_viewModeList;
	int                     m_currentViewMode;
	int                     m_defaultViewMode;
	bool                    m_useViewMode;

	bool     m_silencerAttached;
	bool     m_lamAttached;
	EntityId m_lamID;

	bool m_targetOn;
	Vec3 m_aimPosition;
	Vec3 m_targetPosition;

	bool  m_weaponRaised;
	bool  m_weaponLowered;
	float m_raiseProbability;

	bool m_switchingFireMode;
	bool m_switchLeverLayers;

	int m_firedRockets; //LAW stuff (how many rockets are still flying)

	float m_nextShotTime;

	uint8  m_raisePose;
	uint16 m_shootSeqN;

private:
	static TActionHandler<CWeapon> s_actionHandler;

	static void RegisterActions();

	bool OnActionAttack(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionAttackSecondary(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionReload(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionSpecial(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionModify(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionFiremode(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionZoomIn(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionZoomOut(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionZoom(EntityId actorId, const ActionId& actionId, int activationMode, float value);
	bool OnActionZoomXI(EntityId actorId, const ActionId& actionId, int activationMode, float value);

	bool PreActionAttack(bool startFire) const;
	void RestorePlayerSprintingStats();

	//Flags for force input states (make weapon more responsive)
	bool m_requestedFire;

	ILINE void ClearInputFlags()
	{
		m_requestedFire = false;
	}

	void CacheRaisePose();
};

#endif //__WEAPON_H__
