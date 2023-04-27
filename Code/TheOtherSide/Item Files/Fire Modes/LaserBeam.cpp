#include "StdAfx.h"
#include "LaserBeam.h"
#include "Game.h"
#include "GameRules.h"
#include "WeaponSystem.h"
#include "IMaterialEffects.h"

//------------------------------------------------------------------------
CLaserBeam::CLaserBeam()
{
}

//------------------------------------------------------------------------
CLaserBeam::~CLaserBeam()
{
}

//------------------------------------------------------------------------
void CLaserBeam::ResetParams(const struct IItemParamsNode* params)
{
	CBeam::ResetParams(params);
}

//------------------------------------------------------------------------
void CLaserBeam::PatchParams(const struct IItemParamsNode* patch)
{
	CBeam::PatchParams(patch);
}

//------------------------------------------------------------------------
void CLaserBeam::Hit(ray_hit& hit, const Vec3& dir)
{
}

//------------------------------------------------------------------------
void CLaserBeam::Tick(ray_hit& hit, const Vec3& dir)
{
}

void CLaserBeam::Decal(const ray_hit& rayhit, const Vec3& dir)
{
	//Define decal info here
	CryEngineDecalInfo decal;
	strcpy(decal.szMaterialName, m_beamparams.hit_decal.c_str());

	//Get proper decal material here
	const auto mfx = gEnv->pGame->GetIGameFramework()->GetIMaterialEffects();
	const int matID = rayhit.surface_idx;

	const string bulletImpactName = this->m_fireparams.hit_type;
	const TMFXEffectId effectId = mfx->GetEffectId(bulletImpactName.c_str(), matID);

	const SMFXResourceListPtr pList = mfx->GetResources(effectId);
	if (pList && pList->m_decalList)
	{
		const char* properMaterial = pList->m_decalList->m_decalParams.material;
		strcpy(decal.szMaterialName, properMaterial);
	}

	//Create the decal here
	decal.vPos = rayhit.pt;
	decal.vNormal = rayhit.n;
	decal.fSize = m_beamparams.hit_decal_size;
	if (m_beamparams.hit_decal_size_min != 0.f)
		decal.fSize -= Random() * max(0.f, m_beamparams.hit_decal_size - m_beamparams.hit_decal_size_min);
	decal.fLifeTime = m_beamparams.hit_decal_lifetime;
	decal.bAdjustPos = true;

	decal.fAngle = RAD2DEG(acos_tpl(rayhit.n.Dot(dir)));
	decal.vHitDirection = dir;

	if (rayhit.pCollider)
	{
		if (IRenderNode* pRenderNode = (IRenderNode*)rayhit.pCollider->GetForeignData(PHYS_FOREIGN_ID_STATIC))
			decal.ownerInfo.pRenderNode = pRenderNode;
		else if (const IEntity* pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(rayhit.pCollider))
		{
			IEntityRenderProxy* pRenderProxy = (IEntityRenderProxy*)pEntity->GetProxy(ENTITY_PROXY_RENDER);;
			if (pRenderProxy)
				decal.ownerInfo.pRenderNode = pRenderProxy->GetRenderNode();
		}
	}

	gEnv->p3DEngine->CreateDecal(decal);
}

//------------------------------------------------------------------------
void CLaserBeam::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	CBeam::GetMemoryStatistics(s);
}