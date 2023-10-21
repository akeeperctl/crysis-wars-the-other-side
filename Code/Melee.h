/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Beam Fire Mode Implementation

-------------------------------------------------------------------------
History:
- 23:3:2006   13:02 : Created by Mбrcio Martins

*************************************************************************/
#ifndef __MELEE_H__
#define __MELEE_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "Weapon.h"
#include "Fists.h"
#include "ItemParamReader.h"
#include "IGameRulesSystem.h"

#define ResetValue(name, defaultValue)\
if (defaultInit)\
	name=defaultValue;\
reader.Read(#name, name)\

#define ResetValueEx(name, var, defaultValue) if (defaultInit) var=defaultValue; reader.Read(name, var)

class CMelee : public IFireMode
{
	struct StopAttackingAction;

	typedef struct SMeleeParams
	{
		SMeleeParams()
		{
			Reset();
		};

		void Reset(const IItemParamsNode* params = nullptr, bool defaultInit = true)
		{
			CItemParamReader reader(params);
			ResetValue(helper, "");
			ResetValue(range, 1.75f);
			ResetValue(damage, 32);
			ResetValue(damageAI, 10);
			ResetValue(crosshair, "default");
			ResetValue(hit_type, "melee");
			ResetValue(impulse, 50.0f);
			ResetValue(delay, 0.5f);
			ResetValue(duration, 0.5f);
		}

		void GetMemoryStatistics(ICrySizer* s)
		{
			s->Add(crosshair);
			s->Add(hit_type);
		}

		ItemString helper;
		float      range;

		short      damage;
		short      damageAI;
		ItemString crosshair;
		ItemString hit_type;

		float impulse;

		float delay;
		float duration;
	}         SMeleeParams;

	typedef struct SMeleeActions
	{
		SMeleeActions()
		{
			Reset();
		};

		void Reset(const IItemParamsNode* params = nullptr, bool defaultInit = true)
		{
			CItemParamReader reader(params);
			ResetValue(attack, "attack");
			ResetValue(hit, "hit");
		}

		void GetMemoryStatistics(ICrySizer* s)
		{
			s->Add(attack);
			s->Add(hit);
		}

		ItemString attack;
		ItemString hit;
	}              SMeleeActions;

public:
	CMelee();
	virtual ~CMelee();

	//IFireMode
	void Init(IWeapon* pWeapon, const struct IItemParamsNode* params) override;
	void Update(float frameTime, uint frameId) override;
	void PostUpdate(float frameTime) override {};
	void UpdateFPView(float frameTime) override {};
	void Release() override;
	void GetMemoryStatistics(ICrySizer* s) override;

	void ResetParams(const struct IItemParamsNode* params) override;
	void PatchParams(const struct IItemParamsNode* patch) override;

	void Activate(bool activate) override;

	int GetAmmoCount() const override
	{
		return 0;
	};

	int GetClipSize() const override
	{
		return 0;
	};

	bool OutOfAmmo() const override
	{
		return false;
	};

	bool CanReload() const override
	{
		return false;
	};
	void Reload(int zoomed) override {};

	bool IsReloading() override
	{
		return false;
	};
	void CancelReload() override {};

	bool CanCancelReload() override
	{
		return false;
	};

	bool AllowZoom() const override
	{
		return true;
	};
	void Cancel() override {};

	float GetRecoil() const override
	{
		return 0.0f;
	};

	float GetSpread() const override
	{
		return 0.0f;
	};

	float GetMinSpread() const override
	{
		return 0.0f;
	}

	float GetMaxSpread() const override
	{
		return 0.0f;
	}

	const char* GetCrosshair() const override
	{
		return "";
	};

	bool CanFire(bool considerAmmo = true) const override;
	void StartFire() override;
	void StopFire() override;

	bool IsFiring() const override
	{
		return m_attacking;
	};

	float GetHeat() const override
	{
		return 0.0f;
	};

	bool CanOverheat() const override
	{
		return false;
	};

	void StartSecondaryFire(EntityId shooterId) override {};

	void NetShoot(const Vec3& hit, int ph) override;
	void NetShootEx(const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, float extra, int ph) override;
	void NetEndReload() override {};

	void NetStartFire() override;
	void NetStopFire() override;
	void NetStartSecondaryFire() override {}

	EntityId GetProjectileId() const override
	{
		return 0;
	};

	EntityId RemoveProjectileId() override
	{
		return 0;
	};
	void SetProjectileId(EntityId id) override {};

	const char* GetType() const override;

	IEntityClass* GetAmmoType() const override
	{
		return nullptr;
	};
	int         GetDamage(float distance = 0.0f) const override;
	const char* GetDamageType() const override;

	float GetSpinUpTime() const override
	{
		return 0.0f;
	};

	float GetSpinDownTime() const override
	{
		return 0.0f;
	};

	float GetNextShotTime() const override
	{
		return 0.0f;
	};
	void SetNextShotTime(float time) override {};

	float GetFireRate() const override
	{
		return 0.0f;
	};

	void Enable(bool enable) override
	{
		m_enabled = enable;
	};

	bool IsEnabled() const override
	{
		return m_enabled;
	};

	Vec3 GetFiringPos(const Vec3& probableHit) const override
	{
		return ZERO;
	}

	Vec3 GetFiringDir(const Vec3& probableHit, const Vec3& firingPos) const override
	{
		return ZERO;
	}

	void SetName(const char* name) override
	{
		m_name = name;
	};

	const char* GetName() override
	{
		return m_name.empty() ? nullptr : m_name.c_str();
	};

	bool HasFireHelper() const override
	{
		return false;
	}

	Vec3 GetFireHelperPos() const override
	{
		return Vec3(ZERO);
	}

	Vec3 GetFireHelperDir() const override
	{
		return FORWARD_DIRECTION;
	}

	int GetCurrentBarrel() const override
	{
		return 0;
	}

	void Serialize(TSerialize ser) override {};
	void PostSerialize() override {};

	void SetRecoilMultiplier(float recoilMult) override { }

	float GetRecoilMultiplier() const override
	{
		return 1.0f;
	}

	void PatchSpreadMod(SSpreadModParams& sSMP) override {};
	void ResetSpreadMod() override {};

	void PatchRecoilMod(SRecoilModParams& sRMP) override {};
	void ResetRecoilMod() override {};

	void ResetLock() override {};
	void StartLocking(EntityId targetId, int partId) override {};
	void Lock(EntityId targetId, int partId) override {};
	void Unlock() override {};

	float GetRecoilAmount() const override
	{
		return 0;
	};
	//~IFireMode

	virtual bool PerformRayTest(const Vec3& pos, const Vec3& dir, float strength, bool remote);
	virtual bool PerformCylinderTest(const Vec3& pos, const Vec3& dir, float strength, bool remote);
	virtual void Hit(ray_hit* hit, const Vec3& dir, float damageScale, bool remote);
	virtual void Hit(geom_contact* contact, const Vec3& dir, float damageScale, bool remote);
	virtual void Hit(const Vec3& pt, const Vec3& dir, const Vec3& normal, IPhysicalEntity* pCollider, int partId, int ipart, int surfaceIdx, float damageScale, bool remote);
	virtual void Impulse(const Vec3& pt, const Vec3& dir, const Vec3& normal, IPhysicalEntity* pCollider, int partId, int ipart, int surfaceIdx, float impulseScale);

	virtual void ApplyCameraShake(bool hit);

	//Special case when performing melee with an object (offHand)
	//It must ignore also the held entity!
	virtual void IgnoreEntity(EntityId entityId)
	{
		m_ignoredEntity = entityId;
	}

	virtual void MeleeScale(float scale)
	{
		m_meleeScale = scale;
	}

	virtual float GetOwnerStrength() const;

protected:
	CWeapon* m_pWeapon;
	bool     m_enabled;

	bool m_attacking;
	Vec3 m_last_pos;

	float  m_delayTimer;
	float  m_durationTimer;
	string m_name;
	//Vec3		m_beginPos;
	bool m_attacked;

	EntityId m_ignoredEntity;
	float    m_meleeScale;

	SMeleeParams  m_meleeparams;
	SMeleeActions m_meleeactions;

	bool m_noImpulse;
};

#endif //__MELEE_H__
