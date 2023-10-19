/*************************************************************************
	Crytek Source File.
	Copyright (C), Crytek Studios, 2001-2004.
	-------------------------------------------------------------------------
	$Id$
	$DateTime$

	-------------------------------------------------------------------------
	History:
	- 15:2:2006   12:50 : Created by Mбrcio Martins

*************************************************************************/
// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
// ReSharper disable CppClangTidyPerformanceNoIntToPtr
// ReSharper disable CppClangTidyModernizeAvoidCArrays
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntConversion
#include "StdAfx.h"

#include "Actor.h"
#include "Game.h"
#include "GameRules.h"
#include "Single.h"
#include "Weapon.h"

/*
#define CHECK_OWNER_REQUEST()	\
	{ \
		uint16 channelId=m_pGameFramework->GetGameChannelId(pNetChannel);	\
		IActor *pOwnerActor=GetOwnerActor(); \
		if (!pOwnerActor || pOwnerActor->GetChannelId()!=channelId) \
		{ \
			CryLogAlways("[gamenet] Disconnecting %s. Bogus weapon action '%s' request! %s %d!=%d (%s!=%s)", \
			pNetChannel->GetName(), __FUNCTION__, pOwnerActor?pOwnerActor->GetEntity()->GetName():"null", \
			pOwnerActor?pOwnerActor->GetChannelId():0, channelId,\
			pOwnerActor?pOwnerActor->GetEntity()->GetName():"null", \
			m_pGameFramework->GetIActorSystem()->GetActorByChannelId(channelId)?m_pGameFramework->GetIActorSystem()->GetActorByChannelId(channelId)->GetEntity()->GetName():"null"); \

			return false; \
		} \
	} \
*/

#define CHECK_OWNER_REQUEST()	\
	{ \
		uint16 channelId=m_pGameFramework->GetGameChannelId(pNetChannel);	\
		IActor *pOwnerActor=GetOwnerActor(); \
		if (pOwnerActor && pOwnerActor->GetChannelId()!=channelId && !IsDemoPlayback()) \
			return true; \
	}

//------------------------------------------------------------------------
int CWeapon::NetGetCurrentAmmoCount() const
{
	if (!m_fm)
		return 0;

	return GetAmmoCount(m_fm->GetAmmoType());
}

//------------------------------------------------------------------------
void CWeapon::NetSetCurrentAmmoCount(const int count)
{
	if (!m_fm)
		return;

	SetAmmoCount(m_fm->GetAmmoType(), count);
}

//------------------------------------------------------------------------
void CWeapon::NetShoot(const Vec3& hit, const int predictionHandle)
{
	if (m_fm)
		m_fm->NetShoot(hit, predictionHandle);
}

//------------------------------------------------------------------------
void CWeapon::NetShootEx(const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, const float extra, const int predictionHandle)
{
	if (m_fm)
		m_fm->NetShootEx(pos, dir, vel, hit, extra, predictionHandle);
}

//------------------------------------------------------------------------
void CWeapon::NetStartFire()
{
	if (m_fm)
		m_fm->NetStartFire();
}

//------------------------------------------------------------------------
void CWeapon::NetStopFire()
{
	if (m_fm)
		m_fm->NetStopFire();
}

//------------------------------------------------------------------------
void CWeapon::NetStartSecondaryFire()
{
	if (m_fm)
		m_fm->NetStartSecondaryFire();

	//gEnv->pLog->Log("<<< NetStartSecondaryFire!!! >>>");
}

//------------------------------------------------------------------------
void CWeapon::NetStartMeleeAttack(const bool weaponMelee)
{
	if (weaponMelee && m_melee)
		m_melee->NetStartFire();
	else if (m_fm)
		m_fm->NetStartFire();
}

//------------------------------------------------------------------------
void CWeapon::NetMeleeAttack(const bool weaponMelee, const Vec3& pos, const Vec3& dir)
{
	if (weaponMelee && m_melee)
	{
		m_melee->NetShootEx(pos, dir, ZERO, ZERO, 1.0f, 0);
		if (IsServer())
			m_pGameplayRecorder->Event(GetOwner(), GameplayEvent(eGE_WeaponMelee, nullptr, 0, reinterpret_cast<void*>(GetEntityId())));
	}
	else if (m_fm)
	{
		m_fm->NetShootEx(pos, dir, ZERO, ZERO, 1.0f, 0);
		if (IsServer())
			m_pGameplayRecorder->Event(GetOwner(), GameplayEvent(eGE_WeaponMelee, nullptr, 0, reinterpret_cast<void*>(GetEntityId())));
	}
}

//------------------------------------------------------------------------
void CWeapon::NetZoom(const float fov)
{
	if (CActor* pOwner = GetOwnerActor())
	{
		if (pOwner->IsClient())
			return;

		SActorParams* pActorParams = pOwner->GetActorParams();
		if (!pActorParams)
			return;

		pActorParams->viewFoVScale = fov;
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestShoot(IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel, const Vec3& hit, const float extra, const int predictionHandle, const uint16 seq, const uint8 seqr, const bool forceExtended)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();

	if ((!pActor || pActor->IsClient()) && IsClient())
	{
		if (pActor)
			pActor->GetGameObject()->Pulse('bang');
		GetGameObject()->Pulse('bang');

		if (IsServerSpawn(pAmmoType) || forceExtended)
			GetGameObject()->InvokeRMI(SvRequestShootEx(), SvRequestShootExParams(pos, dir, vel, hit, extra, predictionHandle, seq, seqr), eRMI_ToServer);
		else
			GetGameObject()->InvokeRMI(SvRequestShoot(), SvRequestShootParams(pos, dir, hit, predictionHandle, seq, seqr), eRMI_ToServer);
	}
	else if (!IsClient() && IsServer())
	{
		if (IsServerSpawn(pAmmoType) || forceExtended)
		{
			GetGameObject()->InvokeRMI(ClShoot(), ClShootParams(pos + dir * 5.0f, predictionHandle), eRMI_ToAllClients);
			NetShootEx(pos, dir, vel, hit, extra, predictionHandle);
		}
		else
		{
			GetGameObject()->InvokeRMI(ClShoot(), ClShootParams(hit, predictionHandle), eRMI_ToAllClients);
			NetShoot(hit, predictionHandle);
		}
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestMeleeAttack(const bool weaponMelee, const Vec3& pos, const Vec3& dir, const uint16 seq)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
	{
		GetGameObject()->InvokeRMI(SvRequestMeleeAttack(), RequestMeleeAttackParams(weaponMelee, pos, dir, seq), eRMI_ToServer);
	}
	else if (!IsClient() && IsServer())
	{
		GetGameObject()->InvokeRMI(ClMeleeAttack(), ClMeleeAttackParams(weaponMelee, pos, dir), eRMI_ToAllClients);
		NetMeleeAttack(weaponMelee, pos, dir);
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestStartFire()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
		GetGameObject()->InvokeRMI(SvRequestStartFire(), EmptyParams(), eRMI_ToServer);
	else if (!IsClient() && IsServer())
		GetGameObject()->InvokeRMI(ClStartFire(), EmptyParams(), eRMI_ToAllClients);
}

//------------------------------------------------------------------------
void CWeapon::RequestStartMeleeAttack(const bool weaponMelee)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
	{
		GetGameObject()->InvokeRMI(SvRequestStartMeleeAttack(), RequestStartMeleeAttackParams(weaponMelee), eRMI_ToServer);
	}
	else if (!IsClient() && IsServer())
	{
		GetGameObject()->InvokeRMI(ClStartMeleeAttack(), RequestStartMeleeAttackParams(weaponMelee), eRMI_ToAllClients);
		NetStartMeleeAttack(weaponMelee);
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestZoom(const float fov)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
	{
		GetGameObject()->InvokeRMI(SvRequestZoom(), ZoomParams(fov), eRMI_ToServer);
	}
	else if (!IsClient() && IsServer())
	{
		GetGameObject()->InvokeRMI(ClZoom(), ZoomParams(fov), eRMI_ToAllClients);
		NetZoom(fov);
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestStopFire()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
		GetGameObject()->InvokeRMI(SvRequestStopFire(), EmptyParams(), eRMI_ToServer);
	else if (!IsClient() && IsServer())
		GetGameObject()->InvokeRMI(ClStopFire(), EmptyParams(), eRMI_ToAllClients);
}

//------------------------------------------------------------------------
void CWeapon::RequestReload()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
		GetGameObject()->InvokeRMI(SvRequestReload(), EmptyParams(), eRMI_ToServer);
	else if (!IsClient() && IsServer())
		GetGameObject()->InvokeRMI(ClReload(), EmptyParams(), eRMI_ToAllClients);
}

//-----------------------------------------------------------------------
void CWeapon::RequestCancelReload()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
		GetGameObject()->InvokeRMI(SvRequestCancelReload(), EmptyParams(), eRMI_ToServer);
	else if (!IsClient() && IsServer())
		GetGameObject()->InvokeRMI(ClCancelReload(), EmptyParams(), eRMI_ToAllClients);
}

//------------------------------------------------------------------------
void CWeapon::RequestFireMode(const int fmId)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if (!pActor || pActor->IsClient())
	{
		if (gEnv->bServer)
			SetCurrentFireMode(fmId); // serialization will fix the rest.
		else
			GetGameObject()->InvokeRMI(SvRequestFireMode(), SvRequestFireModeParams(fmId), eRMI_ToServer);
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestLock(const EntityId id, const int partId)
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if (!pActor || pActor->IsClient())
	{
		if (gEnv->bServer)
		{
			if (m_fm)
				m_fm->Lock(id, partId);

			GetGameObject()->InvokeRMI(ClLock(), LockParams(id, partId), eRMI_ToRemoteClients);
		}
		else
		{
			GetGameObject()->InvokeRMI(SvRequestLock(), LockParams(id, partId), eRMI_ToServer);
		}
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestUnlock()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if (!pActor || pActor->IsClient())
		GetGameObject()->InvokeRMI(SvRequestUnlock(), EmptyParams(), eRMI_ToServer);
}

//------------------------------------------------------------------------
void CWeapon::RequestWeaponRaised(const bool raise)
{
	if (gEnv->bMultiplayer)
	{
		const CActor* pActor = GetOwnerActor();
		if (pActor && pActor->IsClient())
		{
			if (gEnv->bServer)
				GetGameObject()->InvokeRMI(ClWeaponRaised(), WeaponRaiseParams(raise), eRMI_ToRemoteClients | eRMI_NoLocalCalls);
			else
				GetGameObject()->InvokeRMI(SvRequestWeaponRaised(), WeaponRaiseParams(raise), eRMI_ToServer);
		}
	}
}

//------------------------------------------------------------------------
void CWeapon::RequestStartSecondaryFire()
{
	const IActor* pActor = m_pGameFramework->GetClientActor();
	if ((!pActor || pActor->IsClient()) && IsClient())
		GetGameObject()->InvokeRMI(SvRequestStartSecondaryFire(), EmptyParams(), eRMI_ToServer);
	else if (!IsClient() && IsServer())
		GetGameObject()->InvokeRMI(ClStartSecondaryFire(), EmptyParams(), eRMI_ToAllClients);
}

//------------------------------------------------------------------------
void CWeapon::SendEndReload()
{
	int channelId = 0;
	if (const CActor* pActor = GetOwnerActor())
		channelId = pActor->GetChannelId();

	GetGameObject()->InvokeRMI(ClEndReload(), EmptyParams(), eRMI_ToClientChannel | eRMI_NoLocalCalls, channelId);
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestStartFire)
{
	CHECK_OWNER_REQUEST();

	GetGameObject()->InvokeRMI(ClStartFire(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	const IActor* pLocalActor = m_pGameFramework->GetClientActor();
	const bool    isLocal = pLocalActor && pActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

	if (!isLocal)
		NetStartFire();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestStopFire)
{
	CHECK_OWNER_REQUEST();

	GetGameObject()->InvokeRMI(ClStopFire(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	const IActor* pLocalActor = m_pGameFramework->GetClientActor();
	const bool    isLocal = pLocalActor && pActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

	if (!isLocal)
		NetStopFire();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClStartFire)
{
	NetStartFire();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClStopFire)
{
	NetStopFire();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestShoot)
{
	CHECK_OWNER_REQUEST();

	bool          ok = true;
	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (!pActor || pActor->GetHealth() <= 0)
		ok = false;

	ok &= !OutOfAmmo(false);

	if (ok)
	{
		if (pActor)
			pActor->GetGameObject()->Pulse('bang');
		GetGameObject()->Pulse('bang');

		static ray_hit rh;

		IEntity*         pEntity = nullptr;
		IPhysicalEntity* pSkipEnts[10];
		const int        nSkipEnts = CSingle::GetSkipEntities(this, pSkipEnts, 10);
		if (gEnv->pPhysicalWorld->RayWorldIntersection(params.pos, params.dir * 4096.0f, ent_all & ~ent_terrain, rwi_stop_at_pierceable | rwi_ignore_back_faces, &rh, 1, pSkipEnts, nSkipEnts))
			pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(rh.pCollider);
		if (pEntity)
		{
			if (INetContext* pNC = gEnv->pGame->GetIGameFramework()->GetNetContext())
				if (pNC->IsBound(pEntity->GetId()))
				{
					AABB bbox;
					pEntity->GetWorldBounds(bbox);
					const bool hit0 = bbox.GetRadius() < 1.0f; // this (radius*2) must match the value in CompressionPolicy.xml ("hit0")
					const Vec3 hitLocal = pEntity->GetWorldTM().GetInvertedFast() * rh.pt;
					//GetGameObject()->InvokeRMI(CWeapon::ClShootX(), ClShootXParams(pEntity->GetId(), hit0, hitLocal, params.predictionHandle),
					//	eRMI_ToOtherClients|eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));
					GetGameObject()->InvokeRMIWithDependentObject(ClShootX(), ClShootXParams(pEntity->GetId(), hit0, hitLocal, params.predictionHandle), eRMI_ToOtherClients | eRMI_NoLocalCalls, pEntity->GetId(), m_pGameFramework->GetGameChannelId(pNetChannel));
				}
		}
		else
		{
			GetGameObject()->InvokeRMI(ClShoot(), ClShootParams(params.hit, params.predictionHandle), eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));
		}

		const IActor*    pLocalActor = m_pGameFramework->GetClientActor();
		const bool isLocal = pLocalActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

		if (!isLocal)
			NetShoot(params.hit, params.predictionHandle);

		if (pActor && !isLocal && params.seq)
			if (CGameRules* pGameRules = g_pGame->GetGameRules())
				pGameRules->ValidateShot(pActor->GetEntityId(), GetEntityId(), params.seq, params.seqr);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestShootEx)
{
	CHECK_OWNER_REQUEST();

	bool          ok = true;
	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (!pActor || pActor->GetHealth() <= 0)
		ok = false;

	ok &= !OutOfAmmo(false);

	if (ok)
	{
		if (pActor)
			pActor->GetGameObject()->Pulse('bang');
		GetGameObject()->Pulse('bang');

		GetGameObject()->InvokeRMI(ClShoot(), ClShootParams(params.pos + params.dir * 5.0f, params.predictionHandle), eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

		const IActor*    pLocalActor = m_pGameFramework->GetClientActor();
		const bool isLocal = pLocalActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

		if (!isLocal)
			NetShootEx(params.pos, params.dir, params.vel, params.hit, params.extra, params.predictionHandle);

		if (pActor && !isLocal && params.seq)
			if (CGameRules* pGameRules = g_pGame->GetGameRules())
				pGameRules->ValidateShot(pActor->GetEntityId(), GetEntityId(), params.seq, params.seqr);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClShoot)
{
	NetShoot(params.hit, params.predictionHandle);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClShootX)
{
	if (const IEntity* pEntity = gEnv->pEntitySystem->GetEntity(params.eid))
	{
		const Vec3 hit = pEntity->GetWorldTM() * params.hit;
		NetShoot(hit, params.predictionHandle);
	}
	else
	{
		GameWarning("ClShootX: invalid entity id %.8x", params.eid);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestStartMeleeAttack)
{
	CHECK_OWNER_REQUEST();

	GetGameObject()->InvokeRMI(ClStartMeleeAttack(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	const IActor* pLocalActor = m_pGameFramework->GetClientActor();
	const bool    isLocal = pLocalActor && pActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

	if (!isLocal)
		NetStartMeleeAttack(params.wmelee);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClStartMeleeAttack)
{
	NetStartMeleeAttack(params.wmelee);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestMeleeAttack)
{
	CHECK_OWNER_REQUEST();

	bool          ok = true;
	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (pActor && pActor->GetHealth() <= 0)
		ok = false;

	if (ok)
	{
		GetGameObject()->InvokeRMI(ClMeleeAttack(), ClMeleeAttackParams(params.wmelee, params.pos, params.dir), eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

		const IActor*    pLocalActor = m_pGameFramework->GetClientActor();
		const bool isLocal = pLocalActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

		if (!isLocal)
			NetMeleeAttack(params.wmelee, params.pos, params.dir);

		if (pActor && !isLocal && params.seq)
			if (CGameRules* pGameRules = g_pGame->GetGameRules())
				pGameRules->ValidateShot(pActor->GetEntityId(), GetEntityId(), params.seq, 0);

		m_pGameplayRecorder->Event(GetOwner(), GameplayEvent(eGE_WeaponMelee, nullptr, 0, reinterpret_cast<void*>(GetEntityId())));
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClMeleeAttack)
{
	NetMeleeAttack(params.wmelee, params.pos, params.dir);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestZoom)
{
	CHECK_OWNER_REQUEST();

	bool          ok = true;
	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (!pActor || pActor->GetHealth() <= 0)
		ok = false;

	if (ok)
	{
		GetGameObject()->InvokeRMI(ClZoom(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

		const IActor*    pLocalActor = m_pGameFramework->GetClientActor();
		const bool isLocal = pLocalActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

		if (!isLocal)
			NetZoom(params.fov);

		int event = eGE_ZoomedOut;
		if (params.fov < 0.99f)
			event = eGE_ZoomedIn;
		m_pGameplayRecorder->Event(GetOwner(), GameplayEvent(event, nullptr, 0, reinterpret_cast<void*>(GetEntityId())));
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClZoom)
{
	NetZoom(params.fov);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestFireMode)
{
	CHECK_OWNER_REQUEST();

	SetCurrentFireMode(params.id);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClSetFireMode)
{
	SetCurrentFireMode(params.id);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestReload)
{
	CHECK_OWNER_REQUEST();

	bool          ok = true;
	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (!pActor || pActor->GetHealth() <= 0)
		ok = false;

	if (ok)
	{
		GetGameObject()->InvokeRMI(ClReload(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, m_pGameFramework->GetGameChannelId(pNetChannel));

		const IActor*    pLocalActor = m_pGameFramework->GetClientActor();
		const bool isLocal = pLocalActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

		if (!isLocal && m_fm)
			m_fm->Reload(0);

		m_pGameplayRecorder->Event(GetOwner(), GameplayEvent(eGE_WeaponReload, nullptr, 0, reinterpret_cast<void*>(GetEntityId())));
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClReload)
{
	if (m_fm)
	{
		if (m_zm)
			m_fm->Reload(m_zm->GetCurrentStep());
		else
			m_fm->Reload(false);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClEndReload)
{
	if (m_fm)
		m_fm->NetEndReload();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestCancelReload)
{
	CHECK_OWNER_REQUEST();

	if (m_fm)
	{
		m_fm->CancelReload();
		GetGameObject()->InvokeRMI(ClCancelReload(), params, eRMI_ToRemoteClients);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClCancelReload)
{
	if (m_fm)
		m_fm->CancelReload();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClLock)
{
	if (m_fm)
		m_fm->Lock(params.entityId, params.partId);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClUnlock)
{
	if (m_fm)
		m_fm->Unlock();

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestLock)
{
	CHECK_OWNER_REQUEST();

	if (m_fm)
		m_fm->Lock(params.entityId, params.partId);

	GetGameObject()->InvokeRMI(ClLock(), params, eRMI_ToRemoteClients);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestUnlock)
{
	CHECK_OWNER_REQUEST();

	if (m_fm)
		m_fm->Unlock();

	GetGameObject()->InvokeRMI(ClUnlock(), params, eRMI_ToRemoteClients);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestWeaponRaised)
{
	CHECK_OWNER_REQUEST();

	GetGameObject()->InvokeRMI(ClWeaponRaised(), params, eRMI_ToAllClients);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClWeaponRaised)
{
	const CActor* pActor = GetOwnerActor();
	if (pActor && !pActor->IsClient())
		RaiseWeapon(params.raise);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, SvRequestStartSecondaryFire)
{
	CHECK_OWNER_REQUEST();

	const CActor* pActor = GetActorByNetChannel(pNetChannel);
	if (!pActor || pActor->GetHealth() <= 0)
		return true;

	GetGameObject()->InvokeRMI(ClStartSecondaryFire(), params, eRMI_ToAllClients, m_pGameFramework->GetGameChannelId(pNetChannel));

	const IActor* pLocalActor = m_pGameFramework->GetClientActor();
	// NOTE: only recall for dedicated server (!IsClient()), otherwise one will receive a double call on server and client setup
	const bool isLocal = pLocalActor && pActor && pLocalActor->GetChannelId() == pActor->GetChannelId();

	if (!isLocal && !IsClient())
		NetStartSecondaryFire();

	//GetGameObject()->InvokeRMI(CWeapon::ClStartSecondaryFire(), params, eRMI_ToAllClients);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CWeapon, ClStartSecondaryFire)
{
	NetStartSecondaryFire();

	return true;
}
