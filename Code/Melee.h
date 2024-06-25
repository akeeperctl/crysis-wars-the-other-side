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
	void Init(IWeapon* pWeapon, const struct IItemParamsNode* params) ;
	void Update(float frameTime, uint frameId) ;
	void PostUpdate(float frameTime)  {};
	void UpdateFPView(float frameTime)  {};
	void Release() ;
	void GetMemoryStatistics(ICrySizer* s) ;

	void ResetParams(const struct IItemParamsNode* params) ;
	void PatchParams(const struct IItemParamsNode* patch) ;

	void Activate(bool activate) ;

	int GetAmmoCount() const 
	{
		return 0;
	};

	int GetClipSize() const 
	{
		return 0;
	};

	bool OutOfAmmo() const 
	{
		return false;
	};

	bool CanReload() const 
	{
		return false;
	};
	void Reload(int zoomed)  {};

	bool IsReloading() 
	{
		return false;
	};
	void CancelReload()  {};

	bool CanCancelReload() 
	{
		return false;
	};

	bool AllowZoom() const 
	{
		return true;
	};
	void Cancel()  {};

	float GetRecoil() const 
	{
		return 0.0f;
	};

	float GetSpread() const 
	{
		return 0.0f;
	};

	float GetMinSpread() const 
	{
		return 0.0f;
	}

	float GetMaxSpread() const 
	{
		return 0.0f;
	}

	const char* GetCrosshair() const 
	{
		return "";
	};

	bool CanFire(bool considerAmmo = true) const ;
	void StartFire() ;
	void StopFire() ;

	bool IsFiring() const 
	{
		return m_attacking;
	};

	float GetHeat() const 
	{
		return 0.0f;
	};

	bool CanOverheat() const 
	{
		return false;
	};

	void StartSecondaryFire(EntityId shooterId)  {};

	void NetShoot(const Vec3& hit, int ph) ;
	void NetShootEx(const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, float extra, int ph) ;
	void NetEndReload()  {};

	void NetStartFire() ;
	void NetStopFire() ;
	void NetStartSecondaryFire()  {}

	EntityId GetProjectileId() const 
	{
		return 0;
	};

	EntityId RemoveProjectileId() 
	{
		return 0;
	};
	void SetProjectileId(EntityId id)  {};

	const char* GetType() const ;

	IEntityClass* GetAmmoType() const 
	{
		return nullptr;
	};
	int         GetDamage(float distance = 0.0f) const ;
	const char* GetDamageType() const ;

	float GetSpinUpTime() const 
	{
		return 0.0f;
	};

	float GetSpinDownTime() const 
	{
		return 0.0f;
	};

	float GetNextShotTime() const 
	{
		return 0.0f;
	};
	void SetNextShotTime(float time)  {};

	float GetFireRate() const 
	{
		return 0.0f;
	};

	void Enable(bool enable) 
	{
		m_enabled = enable;
	};

	bool IsEnabled() const 
	{
		return m_enabled;
	};

	Vec3 GetFiringPos(const Vec3& probableHit) const 
	{
		return ZERO;
	}

	Vec3 GetFiringDir(const Vec3& probableHit, const Vec3& firingPos) const 
	{
		return ZERO;
	}

	void SetName(const char* name) 
	{
		m_name = name;
	};

	const char* GetName() 
	{
		return m_name.empty() ? nullptr : m_name.c_str();
	};

	bool HasFireHelper() const 
	{
		return false;
	}

	Vec3 GetFireHelperPos() const 
	{
		return Vec3(ZERO);
	}

	Vec3 GetFireHelperDir() const 
	{
		return FORWARD_DIRECTION;
	}

	int GetCurrentBarrel() const 
	{
		return 0;
	}

	void Serialize(TSerialize ser)  {};
	void PostSerialize()  {};

	void SetRecoilMultiplier(float recoilMult)  { }

	float GetRecoilMultiplier() const 
	{
		return 1.0f;
	}

	void PatchSpreadMod(SSpreadModParams& sSMP)  {};
	void ResetSpreadMod()  {};

	void PatchRecoilMod(SRecoilModParams& sRMP)  {};
	void ResetRecoilMod()  {};

	void ResetLock()  {};
	void StartLocking(EntityId targetId, int partId)  {};
	void Lock(EntityId targetId, int partId)  {};
	void Unlock()  {};

	float GetRecoilAmount() const 
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
