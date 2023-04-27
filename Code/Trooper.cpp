/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 21:7:2005: Created by Mikko Mononen

*************************************************************************/
#include "StdAfx.h"
#include "Game.h"
#include "GameCVars.h"

#include "Trooper.h"
#include "GameUtils.h"

#include <IViewSystem.h>
#include <IItemSystem.h>
#include <IPhysics.h>
#include <ICryAnimation.h>
#include <ISerialize.h>
#include <IRenderAuxGeom.h>
#include <IMaterialEffects.h>
#include <IEffectSystem.h>

//TheOtherSide

#include "Lam.h"
#include "Weapon.h"
#include "Player.h"
#include "PlayerInput.h"
#include "Single.h"
#include "GameActions.h"
#include "NetInputChainDebug.h"

#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Conqueror/ConquerorSystem.h"
//~TheOtherSide

#define SHIELD_MAX_ENERGY 500.f

const float CTrooper::CTentacle_maxTimeStep = 0.02f;
const float CTrooper::CMaxHeadFOR = 0.5 * 3.14159f;
const float CTrooper::ClandDuration = 1.f;
const float CTrooper::ClandStiffnessMultiplier = 10;

void CTrooper::Revive(bool fromInit)
{
	m_rageMode.Reset();

	ResetShieldEnergy();
	SetupShieldStuff();
	ProjectShield(false);
	if (m_shieldParams.shieldType == eShieldType_Guardian)
	{
		m_shieldParams.canGuardianShieldProj = 0;

		auto it = m_shieldParams.shieldOwners.begin();
		auto end = m_shieldParams.shieldOwners.end();

		for (; it != end; it++)
		{
			CTrooper* pTrooper = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it));
			if (!pTrooper)
				continue;

			//Remove id of the guardian from its shield owners --> turn off shield projection --> if projection off --> regen energy
			pTrooper->SetGuardianId(0);
		}

		//Создать всем ии по отряду. Лидер определяется из поведения, а члены отряда появляются из номера группы.

		//std::vector<IEntity*>::iterator it2 = m_shieldParams.beamEntities.begin();
		//std::vector<IEntity*>::iterator end2 = m_shieldParams.beamEntities.end();
		//for (; it2 != end2; it2++)
		//{
		//	IEntity* pEmitter = *it2;
		//	if (pEmitter)
		//	{
		//		//вылетает null если удалить
		//		gEnv->pEntitySystem->RemoveEntity(pEmitter->GetId());
		//		pEmitter->Activate(false);
		//	}
		//}

		m_shieldParams.shieldOwners.clear();
		m_shieldParams.beamEntities.clear();
		//m_shieldParams.beamSlots.clear();
	}

	m_customLookIKBlends[0] = 0;
	m_customLookIKBlends[1] = 0;
	m_customLookIKBlends[2] = 0;
	m_customLookIKBlends[3] = 0;
	m_customLookIKBlends[4] = 1;

	m_steerInertia = 0;

	CAlien::Revive(fromInit);

	m_modelQuat.SetIdentity();
	//m_modelAddQuat.SetIdentity();

	if (m_pAnimatedCharacter)
	{
		SAnimatedCharacterParams params = m_pAnimatedCharacter->GetParams();
		params.flags &= ~eACF_NoTransRot2k;
		m_pAnimatedCharacter->SetParams(params);
	}

	//TheOtherSide
	const auto isConquest = g_pControlSystem->GetConquerorSystem()->IsGamemode();

	if (GetHealth() > 0 && 
		!gEnv->pSystem->IsEditorMode() && 
		!m_isUseCloak &&
		!isConquest)
	{
		SetupLamLights();
		EnableLamLights(false);
	}
	//~TheOtherSide

	m_lastNotMovingTime = gEnv->pSystem->GetITimer()->GetFrameStartTime();
	m_oldSpeed = 0;
	m_Roll = 0;
	m_Rollx = 0;
	m_oldDirFwd = 0;
	m_oldDirStrafe = 0;
	m_oldVelocity = ZERO;

	m_fDistanceToPathEnd = 0;
	m_bExactPositioning = false;
	m_lastExactPositioningTime = 0.f;

	m_landModelOffset = ZERO;
	m_steerModelOffset = ZERO;

	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	int tNum(0);
	IPhysicalEntity* pTentacle = pCharacter ? pCharacter->GetISkeletonPose()->GetCharacterPhysics(tNum) : NULL;

	pe_simulation_params ps;
	ps.maxTimeStep = CTentacle_maxTimeStep;

	while (pTentacle)
	{
		pTentacle->SetParams(&ps);
		pTentacle = pCharacter->GetISkeletonPose()->GetCharacterPhysics(++tNum);
	}

	if (m_stance == m_desiredStance)
		m_stance = STANCE_NULL;
	UpdateStance();//not this

	m_lastTimeOnGround = gEnv->pSystem->GetITimer()->GetFrameStartTime();
	m_bOverrideFlyActionAnim = false;
	m_overrideFlyAction = "idle";

	m_bNarrowEnvironment = false;
	m_lastCheckEnvironmentPos = ZERO;
	m_fTtentacleBlendRotation = 0.f;

	IScriptTable* scriptTable = GetEntity()->GetScriptTable();
	if (scriptTable)
	{
		scriptTable->GetValue("landPreparationTime", m_jumpParams.defaultLandPreparationTime);

		//TheOtherSide
		/*if (scriptTable->GetValue("IsHaveCloak", m_isUseCloak))
		{
			CryLogAlways("m_isUseCloak %i", (int)m_isUseCloak);

		}*/
		//~TheOtherSide
	}

	m_jumpParams.Reset();// not this

	/*
		m_CollSize.clear();
		//m_CollSize.insert(std::make_pair(STANCE_NULL,GetStanceInfo(STANCE_NULL)->size));
		m_CollSize.insert(std::make_pair(STANCE_STAND,GetStanceInfo(STANCE_STAND)->size));
		m_CollSize.insert(std::make_pair(STANCE_STEALTH,GetStanceInfo(STANCE_STEALTH)->size));
		m_CollSize.insert(std::make_pair(STANCE_CROUCH,GetStanceInfo(STANCE_CROUCH)->size));
		m_CollSize.insert(std::make_pair(STANCE_RELAXED,GetStanceInfo(STANCE_RELAXED)->size));
		//m_CollSize.insert(std::make_pair(STANCE_PRONE,GetStanceInfo(STANCE_PRONE)->size));
		*/
}

void CTrooper::Reset(bool toGame)
{
	//ForceDisableLasers();
	//SetupLamLights();
	//m_lamStats = STrooperLamStats();
}

void CTrooper::SetParams(SmartScriptTable& rTable, bool resetFirst)
{
	if (rTable->GetValue("useLandEvent", m_jumpParams.bUseLandEvent))
		return;
	if (rTable->GetValue("specialAnimType", (int&)m_jumpParams.specialAnimType))
	{
		if (m_jumpParams.specialAnimType == JUMP_ANIM_LAND)
			m_jumpParams.bUseLandAnim = true;
		rTable->GetValue("specialAnimAGInput", (int&)m_jumpParams.specialAnimAGInput);
		char* szValue = NULL;
		if (rTable->HaveValue("specialAnimAGInputValue"))
		{
			rTable->GetValue("specialAnimAGInputValue", szValue);
			m_jumpParams.specialAnimAGInputValue = szValue;
		}
		return;
	}

	char* szValue2 = NULL;
	if (rTable->GetValue("overrideFlyAction", szValue2))
	{
		m_overrideFlyAction = szValue2;
		return;
	}

	m_jumpParams.addVelocity = ZERO;
	if (rTable->GetValue("jumpTo", m_jumpParams.dest) && !m_jumpParams.dest.IsZero() || rTable->GetValue("addVelocity", m_jumpParams.addVelocity))
	{
		rTable->GetValue("jumpVelocity", m_jumpParams.velocity);
		rTable->GetValue("jumpTime", m_jumpParams.duration);
		m_jumpParams.bUseStartAnim = false;
		rTable->GetValue("jumpStart", m_jumpParams.bUseStartAnim);
		m_jumpParams.bUseSpecialAnim = false;
		rTable->GetValue("useSpecialAnim", m_jumpParams.bUseSpecialAnim);
		m_jumpParams.bUseAnimEvent = false;
		rTable->GetValue("useAnimEvent", m_jumpParams.bUseAnimEvent);
		m_jumpParams.bUseLandAnim = false;
		rTable->GetValue("jumpLand", m_jumpParams.bUseLandAnim);
		m_jumpParams.bRelative = false;
		rTable->GetValue("relative", m_jumpParams.bRelative);

		m_jumpParams.landPreparationTime = m_jumpParams.defaultLandPreparationTime;

		//m_input.actions |= ACTION_JUMP;
		m_jumpParams.bTrigger = true;
		//m_jumpParams.state = JS_None;
		m_jumpParams.bFreeFall = false;
		m_jumpParams.bPlayingSpecialAnim = false;
	}
	else
	{
		m_jumpParams.state = JS_None;
		CAlien::SetParams(rTable, resetFirst);
		InitHeightVariance(rTable);
	}

	rTable->GetValue("landPreparationTime", m_jumpParams.landPreparationTime);
	rTable->GetValue("steerInertia", m_steerInertia);
}

void CTrooper::InitHeightVariance(SmartScriptTable& rTable)
{
	float freqMin = 0, freqMax = 0;

	if (rTable->GetValue("heightVarianceLow", m_heightVarianceLow) && rTable->GetValue("heightVarianceHigh", m_heightVarianceHigh))
	{
		m_heightVarianceRandomize = cry_rand() / (float)RAND_MAX;
		/*		float range = m_heightVarianceHigh - m_heightVarianceLow;
				m_heightVarianceLow = m_heightVarianceLow-range/2;
				m_heightVarianceHigh = m_heightVarianceLow+range/2;*/
	}
	if (rTable->GetValue("heightVarianceOscMin", freqMin) && rTable->GetValue("heightVarianceOscMax", freqMax))
		m_heightVarianceFreq = freqMin + (cry_rand() / (float)RAND_MAX) * (freqMax - freqMin);
}

void CTrooper::SetupShieldStuff()
{
	const char* modelPath = "Objects/effects/energy_sphere_b.cgf";
	m_shieldParams.modelPath = modelPath;

	SEntitySlotInfo info;
	GetEntity()->GetSlotInfo(m_shieldParams.modelSlot, info);

	//Shield physics model setup
	{
		if (m_shieldParams.modelSlot == -1 || !info.pStatObj)
		{
			m_shieldParams.modelSlot = GetEntity()->LoadGeometry(-1, modelPath);
		}

		if (info.pStatObj)
		{
			Matrix34 shieldMat34 = GetEntity()->GetSlotLocalTM(m_shieldParams.modelSlot, false);
			shieldMat34.SetScale(Vec3(0.5f, 0.5f, 0.5f));
			shieldMat34.SetTranslation(Vec3(0, -0.24f, 1.26f));

			GetEntity()->SetSlotLocalTM(m_shieldParams.modelSlot, shieldMat34);

			uint flags = GetEntity()->GetSlotFlags(m_shieldParams.modelSlot);
			flags &= ~ENTITY_SLOT_RENDER;
			GetEntity()->SetSlotFlags(m_shieldParams.modelSlot, flags);
		}
	}

	//Shield beam from guardian to shield owner setup
	{
		/*IParticleEffect* pBeamEffect = gEnv->p3DEngine->FindParticleEffect("expansion_alien_fx.alien_beam_intense.shield_beam");
		if (pBeamEffect)
		{
			IParticleEmitter* pBeamEmitter = GetEntity()->GetParticleEmitter(m_shieldParams.beamEmitterSlot);
			if (!pBeamEmitter)
			{
				m_shieldParams.beamEmitterSlot = GetEntity()->LoadParticleEmitter(-1, pBeamEffect);

				IParticleEmitter* pBeamEmitter = GetEntity()->GetParticleEmitter(m_shieldParams.beamEmitterSlot);
				if (!pBeamEmitter)
					return;

				pBeamEmitter->Activate(false);
				m_shieldParams.isBeamActive = false;
			}
		}*/
	}
}

void CTrooper::ProjectShield(bool project)
{
	if (project)
	{
		if (m_shieldParams.energy < 0.1f || m_shieldParams.isProjecting)
			return;

		SEntityPhysicalizeParams params;
		params.type = PE_STATIC;
		GetEntity()->PhysicalizeSlot(m_shieldParams.modelSlot, params);

		uint flags = GetEntity()->GetSlotFlags(m_shieldParams.modelSlot);
		flags |= ENTITY_SLOT_RENDER;
		GetEntity()->SetSlotFlags(m_shieldParams.modelSlot, flags);

		m_shieldParams.isProjecting = true;
	}
	else
	{
		GetEntity()->UnphysicalizeSlot(m_shieldParams.modelSlot);

		uint flags = GetEntity()->GetSlotFlags(m_shieldParams.modelSlot);
		flags &= ~ENTITY_SLOT_RENDER;
		GetEntity()->SetSlotFlags(m_shieldParams.modelSlot, flags);

		m_shieldParams.isProjecting = false;
	}
}

void CTrooper::SetShieldEnergy(float energy)
{
	energy = clamp(energy, 0.0f, GetMaxShieldEnergy());

	if (energy < m_shieldParams.energy)
	{
		m_shieldParams.energyRechargeDelay = 3.f/*g_pGameCVars->ctrl_alienShieldRechargeDelay*/;
	}

	m_shieldParams.energy = energy;

	//GetGameObject()->ChangedNetworkState(CPlayer::ASPECT_NANO_SUIT_ENERGY);
}

float CTrooper::GetShieldEnergy()
{
	return m_shieldParams.energy;
}

float CTrooper::GetMaxShieldEnergy()
{
	return SHIELD_MAX_ENERGY;
}

void CTrooper::ResetShieldEnergy()
{
	SetShieldEnergy(GetMaxShieldEnergy());
}

void CTrooper::UpdateShields(float frametime)
{
	if (GetHealth() < 0.1f)
		return;

	if (m_isUseCloak)
		return;

	const CTrooper::EShieldTypes type = m_shieldParams.shieldType;
	switch (type)
	{
	case CTrooper::eShieldType_Owner:
	{
		//120 sec to recharge the shield energy
		//float rechargeRate = 12.0f;
		const float maxEnergy = GetMaxShieldEnergy();
		const float rechargeRate = m_shieldParams.energyRechargeRate = 60;
		const float recharge = maxEnergy / max(0.01f, rechargeRate);

		if (m_shieldParams.energyRechargeDelay > 0.0f)
			m_shieldParams.energyRechargeDelay = max(0.0f, m_shieldParams.energyRechargeDelay - frametime);

		const bool isServer = gEnv->bServer;
		if (isServer)
		{
			if (!GetOwnerActor())
			{
				if (GetHealth() != 0 && !m_empInfo.isEmpState /*&& !m_shieldParams.isProjecting*/)
				{
					if (recharge < 0.0f || m_shieldParams.energyRechargeDelay <= 0.0f)
					{
						float desiredEnergy = m_shieldParams.energy + recharge * frametime;
						SetShieldEnergy(clamp(desiredEnergy, 0.0f, maxEnergy));
					}
				}
			}
			else
			{
				//Regenerate the shield energy the alien can only when the shield not projected
				if (GetHealth() != 0 && !m_empInfo.isEmpState /*&& !m_shieldParams.isProjecting*/)
				{
					if (recharge < 0.0f || m_shieldParams.energyRechargeDelay <= 0.0f)
					{
						float desiredEnergy = m_shieldParams.energy + recharge * frametime;
						SetShieldEnergy(clamp(desiredEnergy, 0.0f, maxEnergy));
					}
				}
			}
		}

		const float energy = GetShieldEnergy();
		const bool isProjecting = m_shieldParams.isProjecting;

		bool isGuardianInEmp = false;
		bool canGuardianProject = false;
		bool isGuardianHaveOwner = false;

		auto pGuardian = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_shieldParams.guardianId));
		if (pGuardian)
		{
			canGuardianProject = pGuardian->m_shieldParams.canGuardianShieldProj;
			isGuardianInEmp = pGuardian->m_empInfo.isEmpState;
			isGuardianHaveOwner = pGuardian->IsHaveOwner();
		}

		if (!GetOwnerActor())
		{
			//If the shield owner not have the player-owner
			if (!isGuardianHaveOwner)
			{
				//If the guardian not have the player-owner
				if (energy < 0.1f || m_shieldParams.guardianId == 0 || !m_shieldParams.isInRange || isGuardianInEmp)
				{
					if (isProjecting)
					{
						ProjectShield(false);
					}
				}
				else if (m_shieldParams.isInRange)
				{
					if (m_shieldParams.guardianId != 0 && !isProjecting)
					{
						ProjectShield(true);
					}
				}
			}
			else
			{
				//If the guardian have the player-owner

				if (!canGuardianProject || energy < 0.1f || m_shieldParams.guardianId == 0 || !m_shieldParams.isInRange || isGuardianInEmp)
				{
					if (isProjecting)
					{
						ProjectShield(false);
					}
				}
				else if (canGuardianProject && m_shieldParams.isInRange)
				{
					if (m_shieldParams.guardianId != 0 && !isProjecting)
					{
						ProjectShield(true);
					}
				}
			}
		}
		else
		{
			//If the shield owner have the player-owner

			if (!isGuardianHaveOwner)
			{
				if (energy < 0.1f || m_shieldParams.guardianId == 0 || !m_shieldParams.isInRange || isGuardianInEmp)
				{
					if (isProjecting)
					{
						ProjectShield(false);
					}
				}
				else if (energy == maxEnergy)
				{
					if (m_shieldParams.guardianId != 0 && m_shieldParams.isInRange && !isProjecting)
					{
						ProjectShield(true);
					}
				}
			}
			else
			{
				//If the guardian and shield owner have player-owner
				if (!canGuardianProject || energy < 0.1f || m_shieldParams.guardianId == 0 || !m_shieldParams.isInRange || isGuardianInEmp)
				{
					if (isProjecting)
					{
						ProjectShield(false);
					}
				}
				else if (canGuardianProject && m_shieldParams.isInRange)
				{
					if (m_shieldParams.guardianId != 0 && !isProjecting)
					{
						ProjectShield(true);
					}
				}
			}
		}

		//static float color[] = { 1,1,1,1 };
		//gEnv->pRenderer->Draw2dLabel(10, 10, 1.2f, color, false, "Type: Owner");
		//gEnv->pRenderer->Draw2dLabel(10, 30, 1.2f, color, false, "Shield isProjecting %d", isProjecting);
		//gEnv->pRenderer->Draw2dLabel(10, 50, 1.2f, color, false, "Shield isInRange %d", m_shieldParams.isInRange);
		//gEnv->pRenderer->Draw2dLabel(10, 70, 1.2f, color, false, "Shield energy %f", energy);
		//gEnv->pRenderer->Draw2dLabel(10, 90, 1.2f, color, false, "Shield guardianId %d",m_shieldParams.guardianId);
		//gEnv->pRenderer->Draw2dLabel(10, 110, 1.2f, color, false, "Shield beamEmitterSlot %d", m_shieldParams.beamEmitterSlot);
		break;
	}
	case CTrooper::eShieldType_Guardian:
	{
		static const int maxShieldOwners = m_shieldParams.maxShieldOwners;

		IAISystem* pAI = gEnv->pAISystem;
		if (!pAI)
			break;

		IAIObject* pTrooperAi = GetEntity()->GetAI();
		if (!pTrooperAi)
			break;

		if (!IsHaveOwner())
		{
			auto pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(this, 1);
			if (pSquad && pSquad->GetLeader() != 0)
			{
				//Get the shield owners from the player squad members

				for (auto& member : pSquad->GetAllMembers())
				{
					if (pSquad->GetActor(member.GetId()))
					{
						if (IEntity* pEntity = pSquad->GetActor(member.GetId())->GetEntity())
						{
							string className = pEntity->GetClass()->GetName();
							bool isTrooper = className == "Trooper";

							if (isTrooper && pEntity->GetId() != GetEntity()->GetId())
							{
								IAIObject* pEntityAi = pEntity->GetAI();
								if (!pEntityAi)
									continue;

								CTrooper* pShieldOwner = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
								if (!pShieldOwner)
									continue;

								if (pShieldOwner->GetHealth() < 0.1f || pShieldOwner->m_shieldParams.shieldType == eShieldType_Guardian || pShieldOwner->m_isUseCloak)
									continue;

								if (m_shieldParams.shieldOwners.size() < maxShieldOwners)
								{
									const bool isFinded = stl::find(m_shieldParams.shieldOwners, pEntity->GetId());
									if (isFinded)
										continue;

									if (m_shieldParams.beamEntities.size() > 0)
									{
										const auto index = m_shieldParams.shieldOwners.size();

										auto pBeamEntity = m_shieldParams.beamEntities[index];
										if (pBeamEntity)
										{
											int slot = -1;

											SmartScriptTable props;
											IScriptTable* pTable = pBeamEntity->GetScriptTable();
											if (pTable && pTable->GetValue("Properties", props))
											{
												pTable->GetValue("nParticleSlot", slot);
											}

											if (IParticleEmitter* pEmitter = pBeamEntity->GetParticleEmitter(slot))
											{
												m_shieldParams.beamPointers[pShieldOwner->GetEntityId()] = pBeamEntity->GetId();
											}
										}

										pShieldOwner->SetGuardianId(GetEntityId());
										m_shieldParams.shieldOwners.push_back(pShieldOwner->GetEntityId());
									}

									//CryLogAlways("Add shield owner from squad");
								}
							}
						}
					}
				}
			}
			else
			{
				//Get the shield owners from the guardian group Id
				IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
				while (!pIt->IsEnd())
				{
					if (IEntity* pEntity = pIt->Next())
					{
						const string className = pEntity->GetClass()->GetName();
						const bool isTrooper = className == "Trooper";

						if (isTrooper && pEntity->GetId() != GetEntity()->GetId())
						{
							IAIObject* pEntityAi = pEntity->GetAI();
							if (!pEntityAi)
								continue;

							const int guardianGroupId = pTrooperAi->GetGroupId();

							if (pEntityAi->GetGroupId() == guardianGroupId)
							{
								CTrooper* pShieldOwner = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
								if (!pShieldOwner)
									continue;

								if (pShieldOwner->GetHealth() < 0.1f || pShieldOwner->m_shieldParams.shieldType == eShieldType_Guardian || pShieldOwner->m_isUseCloak)
									continue;

								if (m_shieldParams.shieldOwners.size() < maxShieldOwners)
								{
									const bool isFinded = stl::find(m_shieldParams.shieldOwners, pEntity->GetId());
									if (isFinded)
										continue;

									const int index = m_shieldParams.shieldOwners.size();
									if (IEntity* pBeamEntity = m_shieldParams.beamEntities[index])
									{
										int slot = -1;

										SmartScriptTable props;
										IScriptTable* pTable = pBeamEntity->GetScriptTable();
										if (pTable && pTable->GetValue("Properties", props))
										{
											pTable->GetValue("nParticleSlot", slot);
										}

										if (IParticleEmitter* pEmitter = pBeamEntity->GetParticleEmitter(slot))
										{
											m_shieldParams.beamPointers[pShieldOwner->GetEntityId()] = pBeamEntity->GetId();
										}
									}
									pShieldOwner->SetGuardianId(GetEntityId());
									m_shieldParams.shieldOwners.push_back(pShieldOwner->GetEntityId());
								}
							}
						}
					}
				}
			}

			//static float color[] = { 1,1,1,1 };
			//gEnv->pRenderer->Draw2dLabel(10, 190, 1.2f, color, false, "Type: Guardian");
			//gEnv->pRenderer->Draw2dLabel(10, 210, 1.2f, color, false, "Guardian squad size %d",int(m_shieldParams.shieldOwners.size()));
		}
		else
		{
			//Get the shield owners from the player squad members
			auto* pSquad = g_pControlSystem->GetSquadSystem()->GetSquadFromMember(this, 1);
			if (pSquad && pSquad->GetLeader() != 0)
			{
				for (auto& member : pSquad->GetAllMembers())
				{
					if (pSquad->GetActor(member.GetId()))
					{
						if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(member.GetId()))
						{
							const string className = pEntity->GetClass()->GetName();
							const bool isTrooper = className == "Trooper";

							if (isTrooper && pEntity->GetId() != GetEntity()->GetId())
							{
								IAIObject* pEntityAi = pEntity->GetAI();
								if (!pEntityAi)
									continue;

								const int guardianGroupId = pTrooperAi->GetGroupId();

								if (pEntityAi->GetGroupId() == guardianGroupId)
								{
									CTrooper* pShieldOwner = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId()));
									if (!pShieldOwner)
										continue;

									if (pShieldOwner->GetHealth() < 0.1f || pShieldOwner->m_shieldParams.shieldType == eShieldType_Guardian)
										continue;

									if (m_shieldParams.shieldOwners.size() < maxShieldOwners)
									{
										const bool isFinded = stl::find(m_shieldParams.shieldOwners, pEntity->GetId());
										if (isFinded)
											continue;

										const int index = m_shieldParams.shieldOwners.size();
										if (IEntity* pBeamEntity = m_shieldParams.beamEntities[index])
										{
											int slot = -1;

											SmartScriptTable props;
											IScriptTable* pTable = pBeamEntity->GetScriptTable();
											if (pTable && pTable->GetValue("Properties", props))
											{
												pTable->GetValue("nParticleSlot", slot);
											}

											if (IParticleEmitter* pEmitter = pBeamEntity->GetParticleEmitter(slot))
											{
												m_shieldParams.beamPointers[pShieldOwner->GetEntityId()] = pBeamEntity->GetId();
											}
										}
										pShieldOwner->SetGuardianId(GetEntityId());
										m_shieldParams.shieldOwners.push_back(pShieldOwner->GetEntityId());
									}
								}
							}
						}
					}
				}
			}
		}

		//Project beams to the shield owners
		std::vector<EntityId>::iterator it = m_shieldParams.shieldOwners.begin();
		std::vector<EntityId>::iterator end = m_shieldParams.shieldOwners.end();
		for (; it != end; it++)
		{
			CTrooper* pShieldOwner = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it));
			if (!pShieldOwner)
				continue;

			Vec3 guardianPos = GetEntity()->GetWorldPos();
			Vec3 shieldOwnerPos = pShieldOwner->GetEntity()->GetWorldPos();

			const float range = m_shieldParams.GetRange();
			const float distance = (guardianPos - shieldOwnerPos).GetLength();

			if (distance <= range)
				pShieldOwner->m_shieldParams.isInRange = 1;
			else
				pShieldOwner->m_shieldParams.isInRange = 0;

			//int index = it - m_shieldParams.shieldOwners.begin();

			IEntity* pBeamEmitEntity = gEnv->pEntitySystem->GetEntity(m_shieldParams.beamPointers[pShieldOwner->GetEntityId()]);
			if (pBeamEmitEntity)
			{
				int slot = -1;

				SmartScriptTable props;
				IScriptTable* pTable = pBeamEmitEntity->GetScriptTable();
				if (pTable && pTable->GetValue("Properties", props))
				{
					pTable->GetValue("nParticleSlot", slot);
				}

				Matrix34 newBeamMat = GetEntity()->GetWorldTM();
				const Vec3 pos = newBeamMat.GetTranslation() + Vec3(0, 0, 1.5f);

				Vec3 newShieldOwnerPos(shieldOwnerPos);
				newShieldOwnerPos.z += 1.5f;
				Vec3 dir = (newShieldOwnerPos - guardianPos).GetNormalized();

				newBeamMat.SetTranslationMat(pos);

				pBeamEmitEntity->SetWorldTM(newBeamMat);
				pBeamEmitEntity->SetRotation(Quat::CreateRotationVDir(dir));

				IParticleEmitter* pEmitter = pBeamEmitEntity->GetParticleEmitter(slot);
				if (pEmitter)
				{
					//m_shieldParams.beamPointers[pShieldOwner->GetEntityId()] = pEmitter;
					ParticleTarget targetOptions;
					targetOptions.bTarget = true;
					targetOptions.bExtendCount = true;
					targetOptions.bExtendLife = false;
					targetOptions.bExtendSpeed = true;
					targetOptions.bPriority = true;
					//targetOptions.bStretch = true;

					if (pShieldOwner->m_shieldParams.isProjecting)
					{
						if (!pEmitter->IsAlive())
							pEmitter->Activate(true);
					}
					else
					{
						if (pEmitter->IsAlive())
							pEmitter->Activate(false);
					}

					const Vec3 pos = pShieldOwner->GetEntity()->GetCharacter(0)->GetISkeletonPose()->GetAbsJointByID(GetBoneID(BONE_WEAPON)).t;
					targetOptions.vTarget = pShieldOwner->GetEntity()->GetWorldTM() * pos;
					pEmitter->SetTarget(targetOptions);
				}
			}
		}

		break;
	}
	case CTrooper::eShieldType_Last:
		break;
	default:
		break;
	}
}

void CTrooper::SetGuardianId(EntityId id)
{
	if (m_shieldParams.shieldType == eShieldType_Owner)
		m_shieldParams.guardianId = id;
}

bool CTrooper::ApplyGuardianType()
{
	m_shieldParams.shieldType = eShieldType_Guardian;

	if (m_shieldParams.beamEntities.empty())
	{
		IParticleEffect* pBeamEffect = gEnv->p3DEngine->FindParticleEffect("expansion_alien_fx.alien_beam_intense.shield_beam");
		if (pBeamEffect)
		{
			for (int i = 0; i < m_shieldParams.maxShieldOwners; i++)
			{
				SEntitySpawnParams params;
				params.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect");
				params.vPosition = GetEntity()->GetWorldPos();

				IEntity* pParticle = gEnv->pEntitySystem->SpawnEntity(params);
				if (pParticle)
				{
					SmartScriptTable props;
					IScriptTable* pTable = pParticle->GetScriptTable();
					if (pTable && pTable->GetValue("Properties", props))
					{
						props->SetValue("ParticleEffect", "expansion_alien_fx.alien_beam_intense.shield_beam");

						Script::CallMethod(pTable, "Enable");

						int slot = -1;
						pTable->GetValue("nParticleSlot", slot);

						IParticleEmitter* pEmitter = pParticle->GetParticleEmitter(slot);
						if (pEmitter)
						{
							pEmitter->Activate(false);
							//CryLogAlways("Particle entity created and emitter deactivated");
						}

						m_shieldParams.beamEntities.push_back(pParticle);
					}
				}
			}
		}
	}

	//CryLogAlways("%s ApplyGuardianType", GetEntity()->GetName());
	return true;
}

bool CTrooper::OnShockwaveCreated()
{
	return false;
}

bool CTrooper::ApplyLeaderType()
{
	m_shieldParams.shieldType = eShieldType_Leader;
	return true;
}

bool CTrooper::ApplyCloakType()
{
	m_isUseCloak = true;

	return true;
}

void CTrooper::ProcessFlyControl(Vec3 &move, float frameTime)
{
	//Adds controlled flight when play as a trooper
	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		if (move.x || move.y)
		{
			Vec3 desiredVelocityClamped = m_input.deltaMovement;
			float desiredVelocityMag = desiredVelocityClamped.GetLength();
			if (desiredVelocityMag > 1.0f)
				desiredVelocityClamped /= desiredVelocityMag;

			float backwardMul = 1.0f;

			if (desiredVelocityClamped.y < 0.0f)
				backwardMul = LERP(backwardMul, 1.0f, -desiredVelocityClamped.y);

			pe_action_move actionMove;
			actionMove.iJump = 2;

			//углы поворота добываются в функциях GetAngles в виде Radian. 
			//Для удобства работы с углами, специально для юзера они преобразуются в градусы/Degrees (0-360)  

			Ang3 angles(0, 0, GetAngles().z);
			Matrix33 baseMtxZ; baseMtxZ.SetRotationXYZ(angles);

			Vec3 actual = baseMtxZ * Vec3(0, 0, 0);
			Vec3 goal = baseMtxZ * desiredVelocityClamped;

			Interpolate(actual, goal, 5.0f, frameTime);

			actionMove.dir = actual;

			if (m_stats.speed <= 12.0f)
				GetEntity()->GetPhysics()->Action(&actionMove);

			//Matrix rotation debug
			bool isDebugEnabled = false;
			if(isDebugEnabled)
			{
				static float c[] = { 1,1,1,1 };
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 0, 255, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn0(), ColorB(0, 0, 255, 255));
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(255, 0, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn1(), ColorB(255, 0, 0, 255));
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(GetEntity()->GetWorldPos(), ColorB(0, 255, 0, 255), GetEntity()->GetWorldPos() + baseMtxZ.GetColumn2(), ColorB(0, 255, 0, 255));
			}
		}
	}
}

void CTrooper::GetTrooperMovementVector(Vec3& move, float& speed, float& maxSpeed, float& sprintMult)
{
	//TheOtherSide
	maxSpeed = 6.0f;/*GetStanceInfo(m_stance)->maxSpeed;*/
	sprintMult = 1.0f;

	if (IsLocalOwner() && g_pControlSystem->GetLocalControlClient())
		sprintMult = g_pControlSystem->GetLocalControlClient()->GetTrooperParams().sprintMult;

	// AI Movement
	move = m_input.movementVector;
	// Player movement
	// For controlling an alien as if it was a player (dbg stuff)
	move += m_viewMtx.GetColumn(0) * m_input.deltaMovement.x * maxSpeed;
	move += m_viewMtx.GetColumn(1) * m_input.deltaMovement.y * maxSpeed;
	move += m_viewMtx.GetColumn(2) * m_input.deltaMovement.z * maxSpeed;

	//~TheOtherSide

	// probably obsolete
	//move += m_viewMtx.GetColumn(1) * m_params.approachLookat * maxSpeed;

	// Cap the speed to stance max stance speed.
	speed = move.len();
	if (speed > maxSpeed)
	{
		move *= maxSpeed / speed;
		speed = maxSpeed;
	}
}

void CTrooper::Kill()
{
	//TheOtherSide
	m_rageMode.Reset();

	SetShieldEnergy(0);
	ProjectShield(false);
	ForceDisableLasers();

	if (m_shieldParams.shieldType == eShieldType_Guardian)
	{
		//m_shieldParams.shieldOwners.clear();

		std::vector<EntityId>::iterator it = m_shieldParams.shieldOwners.begin();
		std::vector<EntityId>::iterator end = m_shieldParams.shieldOwners.end();
		for (; it != end; it++)
		{
			CTrooper* pTrooper = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it));
			if (!pTrooper)
				continue;

			//Remove id of the guardian from its shield owners --> turn off shield projection --> if projection off --> regen energy
			pTrooper->SetGuardianId(0);
		}

		std::vector<IEntity*>::iterator it2 = m_shieldParams.beamEntities.begin();
		std::vector<IEntity*>::iterator end2 = m_shieldParams.beamEntities.end();
		for (; it2 != end2; it2++)
		{
			IEntity* pBeam = *it2;
			if (pBeam)
			{
				//CryLogAlways("RemoveEntity %s", pBeam->GetName());
				gEnv->pEntitySystem->RemoveEntity(pBeam->GetId());
			}
		}

		m_shieldParams.shieldOwners.clear();
		m_shieldParams.beamEntities.clear();
		//m_shieldParams.beamSlots.clear();
	}
	else if (m_shieldParams.shieldType == eShieldType_Owner)
	{
		CTrooper* pGuardian = static_cast<CTrooper*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_shieldParams.guardianId));
		if (pGuardian)
		{
			IEntity* pBeamEntity = gEnv->pEntitySystem->GetEntity(pGuardian->m_shieldParams.beamPointers[GetEntityId()]);
			if (pBeamEntity)
			{
				int slot = -1;

				SmartScriptTable props;
				IScriptTable* pTable = pBeamEntity->GetScriptTable();
				if (pTable && pTable->GetValue("Properties", props))
					pTable->GetValue("nParticleSlot", slot);

				if (IParticleEmitter* pEmitter = pBeamEntity->GetParticleEmitter(slot))
					pEmitter->Activate(false);
			}

			stl::find_and_erase(pGuardian->m_shieldParams.shieldOwners, GetEntityId());
			//CryLogAlways("Deactivate emitter");
		}

		SetGuardianId(0);
	}
	//~TheOtherSide

	CAlien::Kill();
}

bool CTrooper::CreateCodeEvent(SmartScriptTable& rTable)
{
	const char* event = 0;
	if (!rTable->GetValue("event", event))
		return false;

	//CryLogAlways("CTrooper::CreateCodeEvent (%s)", event);

	if (!strcmp(event, "lamLights"))
	{
		bool bEnable = false;
		if (!(rTable->GetValue("enable", bEnable)))
			return false;

		return EnableLamLights(bEnable);
	}
	else if (!strcmp(event, "applyGuardianStuff"))
	{
		//CryLogAlways("applyGuardianStuff");
		return ApplyGuardianType();
	}
	else if (!strcmp(event, "applyLeaderStuff"))
	{
		//CryLogAlways("applyGuardianStuff");
		return ApplyLeaderType();
	}
	else if (!strcmp(event, "applyCloakStuff"))
	{
		//CryLogAlways("applyGuardianStuff");
		return ApplyCloakType();
	}
	else if (!strcmp(event, "AICreateShockwave"))
	{
		//CryLogAlways("CreateShockwave");
		//float reloadTimer = 0;
		//if (!(rTable->GetValue("reloadTimer", reloadTimer)))
		//	return false;
		//CryLogAlways("CreateShockwave timer %1.f", reloadTimer);

		return OnShockwaveCreated();
	}
	else
		return CAlien::CreateCodeEvent(rTable);
}

void CTrooper::UpdateGlow(float energy)
{
	float fGlowPower = (energy / 200) * 10;

	const int nBodySlot = 0;
	const int nHeadSlot = 1;

	// Update glow from lua
	IScriptTable* pTable = this->GetEntity()->GetScriptTable();
	if (pTable)
	{
		Script::CallMethod(pTable, "SetGlowing", nBodySlot, fGlowPower);
		Script::CallMethod(pTable, "SetGlowing", nHeadSlot, fGlowPower);
	}
}

void CTrooper::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	IEntity* pEnt = GetEntity();
	if (pEnt->IsHidden())
		return;

	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	CActor::Update(ctx, updateSlot);

	const float frameTime = ctx.fFrameTime;

	//float color[] = { 1,1,1,1 };
	//if (gEnv->bServer)
	//{
	//	gEnv->pRenderer->Draw2dLabel(20, 100, 1.3f, color, false, "SERVER: %s Health: %1.f", GetEntity()->GetName(), m_health);
	//	gEnv->pRenderer->Draw2dLabel(20, 120, 1.3f, color, false, "SERVER: %s Energy: %1.f", GetEntity()->GetName(), m_energyParams.energy);
	//}
	//if (gEnv->bClient)
	//{
	//	gEnv->pRenderer->Draw2dLabel(20, 140, 1.3f, color, false, "CLIENT: %s Health: %1.f", GetEntity()->GetName(), m_health);
	//	gEnv->pRenderer->Draw2dLabel(20, 160, 1.3f, color, false, "CLIENT: %s Energy: %1.f", GetEntity()->GetName(), m_energyParams.energy);
	//}

	if (!m_stats.isRagDoll && GetHealth() > 0)
	{
		UpdateGlow(GetAlienEnergy());

		//animation processing
		ProcessAnimation(pEnt->GetCharacter(0), frameTime);

		//reset the input for the next frame
		//TheOtherSide
		if (IsClient())
			m_input.ResetDeltas();
		//~TheOtherSide
		//update tentacles blending
		Vec3 refVec(-m_viewMtx.GetColumn(1) * max(0.1f, m_params.forceView) + -m_desiredVelocity);
		refVec.NormalizeSafe();

		float directionDot = min(1.0f, fabsf(refVec * m_baseMtx.GetColumn(0)) * 3.0f);
		//float animStiff = 0.0f;

		if (m_params.blendingRatio > 0.001f)
		{
			float ratio((GetStanceInfo(m_stance)->maxSpeed - m_stats.speed * directionDot) / GetStanceInfo(m_stance)->maxSpeed);
			Interpolate(m_tentacleBlendRatio, m_params.blendingRatio, 20.0f, frameTime);
			//animStiff = 1.0f + (ratio)*m_tentacleBlendRatio;
		}

		//SetTentacles(pCharacter,animStiff);
		//CryLogAlways("%.1f",animStiff);
		if (gEnv->bClient)
		{
			float dist2 = (gEnv->pRenderer->GetCamera().GetPosition() - GetEntity()->GetWorldPos()).GetLengthSquared();

			//update ground effects, if any
			if (m_pGroundEffect)
			{
				float cloakMult = (m_stats.cloaked) ? 0.5f : 1.f;
				float sizeScale = m_params.groundEffectBaseScale * cloakMult;
				float countScale = /*1.f * */ cloakMult;
				float speedScale = /*1.f * */ cloakMult;

				if (m_params.groundEffectMaxSpeed != 0.f)
				{
					const static float minspeed = 1.f;
					float speed = max(0.f, m_stats.speed + m_stats.angVelocity.len() - minspeed);
					float speedScale = min(1.f, speed / m_params.groundEffectMaxSpeed);
					sizeScale *= speedScale;
					countScale *= speedScale;
				}

				m_pGroundEffect->SetBaseScale(sizeScale, countScale, speedScale);
				m_pGroundEffect->Update();
			}

			if (m_pTrailAttachment)
			{
				CEffectAttachment* pEffectAttachment = (CEffectAttachment*)m_pTrailAttachment->GetIAttachmentObject();
				if (pEffectAttachment)
				{
					float goalspeed = max(1.f, m_stats.speed - m_params.trailEffectMinSpeed);
					Interpolate(m_trailSpeedScale, goalspeed, 3.f, frameTime);

					SpawnParams sp;
					//if (m_params.trailEffectMaxSpeedSize != 0.f)
					sp.fSizeScale = max(0.01f, div_min(m_trailSpeedScale, m_params.trailEffectMaxSpeedSize, 1.f));
					//sp.fSizeScale = min(1.f, max(0.01f, m_trailSpeedScale/m_params.trailEffectMaxSpeedSize));

				//if (m_params.trailEffectMaxSpeedCount != 0.f)
					sp.fCountScale = div_min(m_trailSpeedScale, m_params.trailEffectMaxSpeedCount, 1.f);
					//sp.fCountScale = min(1.f, m_trailSpeedScale / m_params.trailEffectMaxSpeedCount);

					pEffectAttachment->SetSpawnParams(sp);
				}
			}

			if (m_pHealthTrailAttachment)
			{
				CEffectAttachment* pEffectAttachment = (CEffectAttachment*)m_pHealthTrailAttachment->GetIAttachmentObject();
				if (pEffectAttachment)
				{
					float goal = 1.0f - ((float)GetHealth() / (float)max(1, GetMaxHealth()));
					Interpolate(m_healthTrailScale, goal, 2.f, frameTime);

					SpawnParams sp;
					if (m_params.healthTrailEffectMaxSize != 0.f)
						sp.fSizeScale = min(1.f, max(0.01f, m_healthTrailScale / m_params.healthTrailEffectMaxSize));

					if (m_params.healthTrailEffectMaxCount != 0.f)
						sp.fCountScale = 1.0f; // min(1.f, m_healthTrailScale / m_params.healthTrailEffectMaxCount);

					pEffectAttachment->SetSpawnParams(sp);
				}
			}

			if (m_searchbeam.isActive)
				UpdateSearchBeam(frameTime);

			if (m_pTurnSound && m_params.turnSoundMaxVel != 0.f && m_params.turnSoundBoneId != -1 && !m_pTurnSound->IsPlaying() && dist2 < sqr(60.f))
			{
				if (IPhysicalEntity* pPhysics = GetEntity()->GetPhysics())
				{
					pe_status_dynamics dyn;
					dyn.partid = m_params.turnSoundBoneId;
					if (pPhysics->GetStatus(&dyn) && dyn.v.len2() > sqr(0.01f) && dyn.w.len2() > sqr(0.5f * m_params.turnSoundMaxVel))
					{
						float speedRel = min(1.f, dyn.w.len() / m_params.turnSoundMaxVel);

						IEntitySoundProxy* pSoundProxy = (IEntitySoundProxy*)GetEntity()->CreateProxy(ENTITY_PROXY_SOUND);
						int nIndex = m_pTurnSound->SetParam("acceleration", speedRel);
						pSoundProxy->PlaySound(m_pTurnSound);
						pSoundProxy->SetStaticSound(m_pTurnSound->GetId(), true);
						//CryLog("angSpeed %.2f (rel %.2f)", dyn.w.len(), speedRel);
					}
				}
			}
		}
	}

	m_oldSpeed = m_stats.speed;

	//TheOtherSide alien energy

	UpdateEnergyRecharge(frameTime);


	/*bool isServer = gEnv->bServer;
	float rechargeTime = 12.0f;
	float recharge = ALIEN_ENERGY / max(0.01f, rechargeTime);
	m_energyParams.alienEnergyRechargeRate = recharge;

	NETINPUT_TRACE(GetEntityId(), m_energyParams.energy);
	NETINPUT_TRACE(GetEntityId(), recharge);

	if (isServer)
	{
		if (GetHealth() != 0)
		{
			if (recharge < 0.0f || m_energyParams.alienEnergyRechargeDelay <= 0.0f)
			{
				SetAlienEnergy(clamp(m_energyParams.energy + recharge * frameTime, 0.0f, ALIEN_ENERGY));
			}
		}
	}

	if (m_energyParams.alienEnergyRechargeDelay > 0.0f)
		m_energyParams.alienEnergyRechargeDelay = max(0.0f, m_energyParams.alienEnergyRechargeDelay - frameTime);*/

	//TheOtherSide laser update pos and dir
	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	if (pCharacter && GetHealth() > 0)
		UpdateLamLights(frameTime, pCharacter);

	//alien in emp update
	if (m_empInfo.isEmpState)
	{
		if (m_empInfo.empTimer >= 0.01f)
		{
			Interpolate(m_empInfo.empTimer, 0, 1.f, frameTime);

			if (m_empInfo.empTimer <= 0.01f)
				EnableEmpState(false, 0, this);
		}
	}

	if (gEnv->bEditor)
	{
		if (gEnv->bEditorGameMode)
			UpdateShields(frameTime);
	}
	else
	{
		UpdateShields(frameTime);
	}

	m_rageMode.Update(frameTime);
}

void CTrooper::UpdateStats(float frameTime)
{
	CAlien::UpdateStats(frameTime);

	IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();
	if (!pPhysEnt)
		return;

	if (InZeroG())
	{
		ray_hit hit;
		int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);

		if (gEnv->pPhysicalWorld->RayWorldIntersection(GetEntity()->GetWorldPos(), m_baseMtx.GetColumn(2) * -5.0f, ent_terrain | ent_static | ent_rigid, rayFlags, &hit, 1, &pPhysEnt, 1))
		{
			pe_player_dynamics newGravity;
			newGravity.gravity = m_baseMtx.GetColumn(2) * -9.81f;
			pPhysEnt->SetParams(&newGravity);
		}
		else
			m_stats.isFloating = true;
	}
	else
	{
		pe_player_dynamics newGravity;
		m_stats.gravity.Set(0, 0, -9.81f);
		newGravity.gravity = m_stats.gravity;
		pPhysEnt->SetParams(&newGravity);
	}
}

void CTrooper::ProcessRotation(float frameTime)
{
	IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();

	if (!pPhysEnt)
		return;

	if (frameTime > 0.2f)
		frameTime = 0.2f;

	float rotSpeed(10.5f);
	/*
		if (m_input.viewVector.len2()>0.0f)
		{
			m_eyeMtx.SetRotationVDir(m_input.viewVector.GetNormalizedSafe());
		}
	*/
	if (m_input.viewVector.len2() > 0.0f)
	{
		/*const SAnimationTarget * pAnimTarget = GetAnimationGraphState()->GetAnimationTarget();
		if (!(pAnimTarget != NULL ))
			m_viewMtx.SetRotationVDir(m_input.viewVector.GetNormalizedSafe());*/
	}
	else
	{
		/* TODO: check, rotation is done in SetDesiredDirection
		Ang3 desiredAngVel(m_input.deltaRotation.x * rotSpeed,0,m_input.deltaRotation.z * rotSpeed);

		//rollage
		if (m_input.actions & ACTION_LEANLEFT)
			desiredAngVel.y -= 10.0f * rotSpeed;
		if (m_input.actions & ACTION_LEANRIGHT)
			desiredAngVel.y += 10.0f * rotSpeed;

		Interpolate(m_angularVel,desiredAngVel,3.5f,frameTime);

		Matrix33 yawMtx;
		Matrix33 pitchMtx;
		Matrix33 rollMtx;

		//yaw
		yawMtx.SetRotationZ(m_angularVel.z * gf_PI/180.0f);
		//pitch
		pitchMtx.SetRotationX(m_angularVel.x * gf_PI/180.0f);
		//roll
		if (fabs(m_angularVel.y) > 0.001f)
			rollMtx.SetRotationY(m_angularVel.y * gf_PI/180.0f);
		else
			rollMtx.SetIdentity();
		//

		m_viewMtx = m_viewMtx * yawMtx * pitchMtx * rollMtx;
		m_viewMtx.OrthonormalizeFast();
		*/
	}

	//now build the base matrix
	Vec3 forward(m_viewMtx.GetColumn(1));

	if (forward.z > -0.9f && forward.z < 0.9f) //apply rotation only if trooper is not looking at too vertical direction
	{
		Vec3 forwardMove(m_stats.velocity.GetNormalizedSafe());
		if (forwardMove.IsZero())
			forwardMove = forward;
		Vec3 forwardMoveXY(forwardMove.x, forwardMove.y, 0);
		forwardMoveXY.NormalizeSafe();

		Quat currRotation(GetEntity()->GetRotation());
		currRotation.Normalize();

		float roll = 0;
		float rollx = 0;

		Vec3 up(m_viewMtx.GetColumn2());

		int oldDir = m_stats.movementDir;

		if ((forward - up).len2() > 0.001f)
		{
			float dotY(forwardMoveXY * m_viewMtx.GetColumn(1));
			//float dotX(forwardMoveXY * m_viewMtx.GetColumn(0));
			if (dotY < -0.6f)
			{
				m_stats.movementDir = 1;	//moving backwards
			}
			else
			{
				m_stats.movementDir = 0;
			}

			IEntity* pEntity = GetEntity();
			IAIObject* pAIObject;
			IUnknownProxy* pProxy;
			if (pEntity && (pAIObject = pEntity->GetAI()) && (pProxy = pAIObject->GetProxy()))
			{
				IAnimationGraphState* pAGState = GetAnimationGraphState();
				if (pAGState)
				{
					pAGState->SetInput(m_idMovementInput, m_stats.movementDir);
				}
			}
			//if (m_stats.inAir<0.2f)
//			forward = m_viewMtx.GetColumn(1);

			IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();
			if (m_bExactPositioning)
			{
				roll = 0;
				rollx = 0;
				m_oldDirStrafe = 0;
				m_oldDirFwd = 0;
				if (pPhysEnt)
				{
					pe_player_dimensions params;
					if (pPhysEnt->GetParams(&params))
					{
						if (params.heightPivot != 0)
						{
							Interpolate(params.heightPivot, 0, 10.f, frameTime);
							pPhysEnt->SetParams(&params);
						}
					}
				}
				/*
					Matrix33 mrot(m_baseMtx);
					Matrix33 mrot2(m_modelQuat);
					Vec3 basepos = GetEntity()->GetWorldPos()+Vec3(0,0,0.7f);
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(28,28,0,255), basepos + mrot.GetColumn(0) * 2.0f, ColorB(128,128,0,255));
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(192,192,0,255), basepos + mrot.GetColumn(1)* 2.0f, ColorB(192,192,0,255));
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(40,40,0,255), basepos + mrot.GetColumn(2) * 2.0f, ColorB(140,140,0,255));
					basepos.z +=0.1f;
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(38,0,0,255), basepos + mrot2.GetColumn(0) * 2.0f, ColorB(138,0,0,255));
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(255,0,0,255), basepos + mrot2.GetColumn(1)* 2.0f, ColorB(255,0,0,255));
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(70,0,0,255), basepos + mrot2.GetColumn(2) * 2.0f, ColorB(170,0,0,255));

					basepos.z +=0.4f;
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(255,255,255,255), basepos + forward * 3.0f, ColorB(255,255,255,255));

				*/
			}
			else if (m_stats.inAir > 0)
			{
				roll = 0;
				rollx = 0;
				m_oldDirStrafe = 0;
				m_oldDirFwd = 0;
			}
			else
			{
				// Luciano : add banking factor when moving

				Vec3 forwardXY(forward.x, forward.y, 0);
				forwardXY.NormalizeSafe();

				Matrix33 invViewMtx(m_viewMtx.GetInverted());
				Vec3 localVel = invViewMtx * m_stats.velocity;

				float vX = localVel.x;
				float l = localVel.GetLength2D();
				float vXn = (l > 0 ? vX / l : 0);
				float accelStrafe = iszero(frameTime) ? 0.0f : (vX - m_oldDirStrafe) / frameTime;

				m_oldDirStrafe = vX;
				vXn *= vXn;

				float bankMultiplier = g_pGameCVars->g_trooperBankingMultiplier;

				roll = vXn * vXn * (accelStrafe / 3 + vX / 6) * bankMultiplier;
				roll = min(max(-DEG2RAD(15.0f), roll), DEG2RAD(15.0f));

				/*	disabled banking around X (when moving forward/backward)
				float vY = localVel.y;
				float vYn = (l>0 ? vY/l : 0);
				float accelFwd =  (vY - m_oldDirFwd)/frameTime;
				vYn *= vYn;
				m_oldDirFwd = vY;
				rollx =  -vYn*vYn *(accelFwd/6 +  vY /6)*bankMultiplier;
				if(rollx > 0)
					rollx/=2;
				rollx = min(max(-DEG2RAD(15.0f),rollx),DEG2RAD(7.5f));
				*/

				// tilt the trooper more like the ground
				pe_status_living livStat;
				if (pPhysEnt)
				{
					pPhysEnt->GetStatus(&livStat);
					Vec3 groundNormal((up + livStat.groundSlope).GetNormalizedSafe());
					Vec3 localUp(invViewMtx * (Vec3Constants<float>::fVec3_OneZ * Matrix33::CreateRotationXYZ(Ang3(rollx, roll, 0))));
					Vec3 localGroundN(invViewMtx * groundNormal);
					Vec3 localUpx(localUp.x, 0, localUp.z);
					Vec3 localUpy(0, localUp.y, localUp.z);
					Vec3 localGroundNx(localGroundN.x, 0, localGroundN.z);
					Vec3 localGroundNy(0, localGroundN.y, localGroundN.z);
					localUpx.NormalizeSafe();
					localUpy.NormalizeSafe();
					localGroundNx.NormalizeSafe();
					localGroundNy.NormalizeSafe();

					float dotNy = localUpy.Dot(localGroundNy);
					float dotNx = localUpx.Dot(localGroundNx);
					if (dotNy > 1)
						dotNy = 1;
					if (dotNx > 1)
						dotNx = 1;

					float angley = cry_acosf(dotNx) * sgn(localGroundN.x - localUp.x);
					float anglex = cry_acosf(dotNy) * sgn(localUp.y - localGroundN.y);

					static const float BANK_PRECISION = 100.f;
					anglex = floor(anglex * BANK_PRECISION) / BANK_PRECISION;
					angley = floor(angley * BANK_PRECISION) / BANK_PRECISION;
					//y =angley;
					rollx += anglex;
					roll += angley;
				}
			}
			Interpolate(m_Roll, roll, 2.0f, frameTime, 3.0f);
			Interpolate(m_Rollx, rollx, 2.0f, frameTime, 3.0f);

			up.Set(0, 0, 1);
			forward.z = 0;
			forward.NormalizeSafe();
			Vec3 right = -(up % forward).GetNormalized();

			Quat currQuat(Matrix33::CreateFromVectors(right, up % right, up));
			currQuat.Normalize();

			float maxSpeed = GetStanceInfo(m_stance)->maxSpeed;
			float rotSpeed;

			SActorStats* pStats = GetActorStats();

			if (pStats && pStats->inFiring > 8.5f)
				rotSpeed = m_params.rotSpeed_max;
			else if (m_bExactPositioning)
				rotSpeed = m_params.rotSpeed_max * 3;
			else
				rotSpeed = m_params.rotSpeed_min + (max(maxSpeed - max(m_stats.speed - m_params.speed_min, 0.0f), 0.0f) / maxSpeed) * (m_params.rotSpeed_max - m_params.rotSpeed_min);

			Interpolate(m_turnSpeed, rotSpeed, 2.5f, frameTime);
			//m_turnSpeed = rotSpeed;

			m_modelQuat = Quat::CreateSlerp(currRotation, currQuat, min(1.0f, frameTime * m_turnSpeed));
			m_modelQuat.Normalize();

			m_moveRequest.rotation = currRotation.GetInverted() * m_modelQuat;
			m_moveRequest.rotation.Normalize();

			m_baseMtx = Matrix33(Quat::CreateSlerp(currQuat, m_modelQuat, frameTime * m_turnSpeed));
			m_baseMtx.OrthonormalizeFast();

			//update the character offset
			Vec3 goal = (m_stats.isRagDoll ? Vec3(0, 0, 0) : GetStanceInfo(m_stance)->modelOffset);
			Interpolate(m_modelOffset, goal, 5.0f, frameTime);

			//			m_modelAddQuat.SetIdentity();
			m_charLocalMtx.SetIdentity();
			pe_player_dimensions params;
			if (pPhysEnt->GetParams(&params))
			{
				// rotate the character around the collider center
				m_charLocalMtx.SetRotationXYZ(Ang3(m_Rollx, m_Roll, 0));
				Vec3 pivot(0, 0, params.heightCollider);
				Vec3 trans(m_charLocalMtx.TransformVector(pivot));
				trans.z -= pivot.z;
				m_charLocalMtx.SetTranslation(-trans + m_modelOffset + m_modelOffsetAdd);
				//				float transx = heightPivot * tan(m_Rollx);
				//				float transy = heightPivot * tan(m_Roll);
				//				m_modelAddQuat.SetTranslation(Vec3(-transy, -transx,0) + m_modelOffset+m_modelOffsetAdd);
			}
			//m_modelAddQuat = QuatT(Matrix33::CreateIdentity() * Matrix33::CreateRotationXYZ(Ang3(m_Rollx,m_Roll,0)));//goalQuat;

		//m_charLocalMtx = Matrix34(m_modelAddQuat);
			GetAnimatedCharacter()->SetExtraAnimationOffset(m_charLocalMtx);
			//GetAnimatedCharacter()->SetExtraAnimationOffset(m_modelAddQuat);
/*
			m_modelQuat = Quat::CreateSlerp( GetEntity()->GetRotation().GetNormalized(), goalQuat, min(0.5f,frameTime * m_turnSpeed)  );
			m_modelQuat.Normalize();
			m_moveRequest.rotation = currRotation.GetInverted() * m_modelQuat;
			m_moveRequest.rotation.Normalize();
			Quat currQuat(m_baseMtx);
			m_baseMtx = Matrix33(Quat::CreateSlerp( currQuat.GetNormalized(), m_modelQuat, frameTime * m_turnSpeed ));
			m_baseMtx.OrthonormalizeFast();
*/

/*
Matrix33 mrot(m_charLocalMtx);
Matrix33 mrot2(m_modelQuat);
Vec3 basepos = GetEntity()->GetWorldPos()+Vec3(0,0,0.7f);
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(28,28,0,255), basepos + mrot.GetColumn(0) * 2.0f, ColorB(128,128,0,255));
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(192,192,0,255), basepos + mrot.GetColumn(1)* 2.0f, ColorB(192,192,0,255));
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(40,40,0,255), basepos + mrot.GetColumn(2) * 2.0f, ColorB(140,140,0,255));

basepos.z +=0.1f;
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(38,0,0,255), basepos + mrot2.GetColumn(0) * 2.0f, ColorB(138,0,0,255));
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(255,0,0,255), basepos + mrot2.GetColumn(1)* 2.0f, ColorB(255,0,0,255));
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(70,0,0,255), basepos + mrot2.GetColumn(2) * 2.0f, ColorB(170,0,0,255));

basepos.z +=0.4f;
gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(basepos, ColorB(255,255,255,255), basepos + forward * 3.0f, ColorB(255,255,255,255));
*/
		}
		else
			m_moveRequest.rotation.SetIdentity();
	}
}

void CTrooper::UpdateAnimGraph(IAnimationGraphState* pState)
{
	CActor::UpdateAnimGraph(pState);

	if (pState)
	{
		pState->SetInput(m_inputSpeed, m_stats.inAir <= 0 && !m_bExactPositioning ? m_stats.speed : 0);
		pState->SetInput(m_inputDesiredSpeed, m_stats.desiredSpeed);
	}
}

void CTrooper::SetAnimTentacleParams(pe_params_rope& pRope, float physicBlend)
{
	float stiffnessOverride = g_pGameCVars->g_trooperTentacleAnimBlend;
	if (/*m_bExactPositioning ||*/ stiffnessOverride < 0 || physicBlend < 0.001f)
	{
		pRope.stiffnessAnim = 0;	// Special case, use full animation.
		pRope.dampingAnim = 1.0f;	// When stiffness is zero, this value does not really matter, set it to sane value anyway.
	}
	else
	{
		/*
		IPhysicalEntity *phys = GetEntity()->GetPhysics();
		pe_status_dynamics	dyn;
		if(phys)
			phys->GetStatus(&dyn);
		*/
		float coeff = 1 - sqrt(physicBlend < 0 ? 0 : physicBlend);

		float frameTime = gEnv->pSystem->GetITimer()->GetFrameTime();
		if (frameTime == 0)
			frameTime = 0.05f;

		// check big rotations
		float rot = m_viewMtx.GetColumn1().GetNormalizedSafe().Dot(m_baseMtx.GetColumn1().GetNormalizedSafe());
		if (m_fTtentacleBlendRotation == 0)
		{
			if (rot < 0)
				m_fTtentacleBlendRotation = 0.5f - rot;
		}
		if (m_fTtentacleBlendRotation > 0)
		{
			coeff -= 0.4f * (m_fTtentacleBlendRotation / 1.5);
			if (coeff < 0.01f)
				coeff = 0.01f;
			m_fTtentacleBlendRotation -= frameTime;
			if (m_fTtentacleBlendRotation < 0)
				m_fTtentacleBlendRotation = 0;
		}
		//Vec3 velocity = (phys ? dyn.v : m_moveRequest.velocity);
		//float speed = velocity.GetLength();
		float vertSpeed = fabs(m_stats.velocity.z);
		coeff *= (vertSpeed / 10 + 1);
		if (m_stats.speed > 0.03f) //moving either backward or forward, increase stiffness
		{
			Vec3 forward = m_stats.velocity.GetNormalizedSafe();
			float dotX = forward * m_viewMtx.GetColumn(1);
			//coeff *= (2 - fabs(dotX));
			if (dotX > 0.5f)
				coeff *= (1 + 2 * (dotX - 0.5f));
		}
		// make tentacles stiffer when accelerating/decelerating

		float accelCoeff = 1 + fabs(m_stats.speed - m_oldSpeed) / frameTime / 30;
		if (accelCoeff > 2)
			accelCoeff = 2;

		coeff *= accelCoeff;
		if (coeff >= 1)
		{
			//coeff=1;
			pRope.stiffnessAnim = 0;
			pRope.dampingAnim = 1;
		}
		else
		{
			pRope.stiffnessAnim = 0.9 * coeff / CTentacle_maxTimeStep;
			pRope.dampingAnim = 0.4f * coeff / CTentacle_maxTimeStep;
		}
	}
}

void CTrooper::ProcessMovement(float frameTime)
{
	//frameTime = min(1.0f, frameTime);

	if (frameTime > 0.1f)
		frameTime = 0.1f;

	//movement
	Vec3 move;
	float	reqSpeed, maxSpeed, sprintMult;
	GetTrooperMovementVector(move, reqSpeed, maxSpeed, sprintMult);

	move *= sprintMult;

	//move += m_stats.velocity;

	//move += m_viewMtx.GetColumn(0) * m_pInput->deltaMovement.x * 10.0f;
	//move += m_viewMtx.GetColumn(1) * m_pInput->deltaMovement.y * 10.0f;
	//move += m_viewMtx.GetColumn(2) * m_pInput->deltaMovement.z * 10.0f;

	// NOTE Jan 18, 2007: <pvl> preserve unmodified AI request for later use
	Vec3 ai_requested_movement = move;

	CTimeValue currTime = gEnv->pSystem->GetITimer()->GetFrameStartTime();

	if (!m_stats.isFloating)
		move -= move * (m_baseMtx * Matrix33::CreateScale(Vec3Constants<float>::fVec3_OneZ));//make it flat

	//if (m_stats.sprintLeft)
	//	move *= m_params.sprintMultiplier;

	//TheOtherSide
	if (m_shieldParams.shieldType == eShieldType_Leader)
		move *= 0.8;
	//~TheOtherSide

	m_moveRequest.type = eCMT_Normal;

	if (m_bExactPositioning)
	{
		m_lastExactPositioningTime = currTime;
	}


	//Trooper sprint debug
	bool isDebugEnabled = false;
	if (isDebugEnabled)
	{
		static float c[] = { 1,1,1,1 };
		gEnv->pRenderer->Draw2dLabel(20, 20, 2, c, false, "m_params.sprintMultiplier: %f", m_params.sprintMultiplier);
		gEnv->pRenderer->Draw2dLabel(20, 40, 2, c, false, "m_params.sprintDuration: %f", m_params.sprintDuration);
		gEnv->pRenderer->Draw2dLabel(20, 60, 2, c, false, "m_stats.sprintLeft: %f", m_stats.sprintLeft);
		gEnv->pRenderer->Draw2dLabel(20, 80, 2, c, false, "m_stats.sprintMaxSpeed: %f", m_stats.sprintMaxSpeed);
		gEnv->pRenderer->Draw2dLabel(20, 100, 2, c, false, "m_stats.sprintTreshold: %f", m_stats.sprintTreshold);
		gEnv->pRenderer->Draw2dLabel(20, 120, 2, c, false, "m_stats.speed: %f", m_stats.speed);
	}

	IAnimationGraphState* pAGState = GetAnimationGraphState();
	IPhysicalEntity* phys = GetEntity()->GetPhysics();

	if (!m_bExactPositioning)// &&
		//(gEnv->pSystem->GetITimer()->GetFrameStartTime() - m_lastExactPositioningTime).GetSeconds() > 0.5f)
	{
		if (m_jumpParams.bTrigger)
		{
			m_jumpParams.bTrigger = false;
			m_jumpParams.state = JS_JumpStart;
			m_jumpParams.startTime = currTime;
			//m_moveRequest.type = eCMT_JumpAccumulate;
			IAnimationGraphState* pAGState = GetAnimationGraphState();
			if (pAGState && m_jumpParams.bUseStartAnim)
			{
				Vec3 vN(m_jumpParams.velocity);
				Vec3 vNx(vN);
				vN.NormalizeSafe();
				vNx.z = 0;
				vNx.NormalizeSafe();
				float dot = vN.Dot(vNx);
				float anglex = RAD2DEG(cry_acosf(CLAMP(dot, -1.f, 1.f)));
				if (m_viewMtx.GetColumn1().Dot(m_jumpParams.velocity) < -0.001) // jump backwards
					anglex = 180 - anglex;

				pAGState->SetInput(m_idAngleXInput, anglex);

				Vec3 viewDir(m_viewMtx.GetColumn(1));
				dot = vNx.Dot(viewDir);
				float anglez = RAD2DEG(cry_acosf(CLAMP(dot, -1.f, 1.f)) * sgn(vNx.Cross(viewDir).z));
				pAGState->SetInput(m_idAngleZInput, anglez);

				if (m_jumpParams.bUseSpecialAnim && m_jumpParams.specialAnimType == JUMP_ANIM_FLY)
					pAGState->SetInput(m_idActionInput, m_jumpParams.specialAnimAGInputValue);
				else
					pAGState->SetInput(m_idActionInput, "fly");
			}
		}

		if (m_jumpParams.state == JS_JumpStart)
		{
			if ((!m_jumpParams.bUseStartAnim && !m_jumpParams.bUseAnimEvent) ||
				(currTime - m_jumpParams.startTime).GetSeconds() > 0.9f)
			{
				Jump();
			}
		}
		else if (m_jumpParams.state == JS_ApplyImpulse)
		{
			// actual impulse will be applied now
			if (phys)
			{
				pe_player_dynamics simParSet;
				simParSet.bSwimming = true;
				phys->SetParams(&simParSet);
			}
			m_moveRequest.type = eCMT_JumpInstant;

			pe_status_dynamics	dyn;
			if (!m_jumpParams.velocity.IsZero() && phys && phys->GetStatus(&dyn))
			{
				Vec3 vel(m_jumpParams.bRelative ? m_jumpParams.velocity : m_jumpParams.velocity - dyn.v);
				move = (vel + m_jumpParams.addVelocity);
				m_jumpParams.state = JS_Flying;//JS_JumpStart;
				JumpEffect();
			}
			else
				m_jumpParams.state = JS_None;

			m_jumpParams.velocity = ZERO;
			m_jumpParams.startTime = currTime;
		}

		if (m_stats.inAir <= 0)
			m_lastTimeOnGround = currTime;

		if (m_stats.inAir > 0.3f && phys)
		{
			pe_player_dynamics simParSet;
			simParSet.bSwimming = false;
			phys->SetParams(&simParSet);
		}

		if (m_stats.inAir > 0.0f && !InZeroG() && !GetEntity()->GetParent() && m_jumpParams.state != JS_ApproachLanding && m_jumpParams.state != JS_Landing)
		{
			m_jumpParams.curVelocity = m_stats.velocity;
			//check free fall
			if ((currTime - m_lastTimeOnGround).GetSeconds() > 0.5f) //TheOtherSide set to 0.5 sec
			{
				if (m_jumpParams.state == JS_None || /*m_jumpParams.state ==JS_Landing ||*/ m_jumpParams.state == JS_Landed)
				{
					//IAnimationGraphState* pAGState = GetAnimationGraphState();
					bool bUseSpecialFlyAnim = (m_bOverrideFlyActionAnim || m_jumpParams.bUseSpecialAnim && m_jumpParams.specialAnimType == JUMP_ANIM_FLY);
					if (pAGState && !bUseSpecialFlyAnim)
					{
						pAGState->SetInput(m_idActionInput, m_bOverrideFlyActionAnim ? m_overrideFlyAction : "flyNoStart");
					}
					m_jumpParams.state = JS_Flying;
					if (!bUseSpecialFlyAnim)
						m_jumpParams.bUseLandAnim = true;
					m_jumpParams.bFreeFall = true;
				}

				//IPhysicalEntity *phys = GetEntity()->GetPhysics();
				if (phys)
				{
					if (m_jumpParams.bUseLandAnim)
					{
						// computing remaining time to land
						float t = -1;
						Vec3 vN(m_stats.velocity / (m_stats.speed > 0 ? m_stats.speed : 1));
						if (!m_jumpParams.bFreeFall)//&& t > 0.4f) // avoid raycast when expected left fly time is high enough
							t = m_jumpParams.duration - (currTime - m_jumpParams.startTime).GetSeconds();
						else
						{
							if (vN.z < -0.05f) // going down
							{
								ray_hit hit;
								int rayFlags = rwi_stop_at_pierceable | (geom_colltype_player << rwi_colltype_bit);
								Vec3 pos(GetEntity()->GetWorldPos());
								if (gEnv->pPhysicalWorld->RayWorldIntersection(pos, (vN.z < 0 ? vN : -Vec3Constants<float>::fVec3_OneZ) * 20, ent_terrain | ent_static | ent_rigid, rayFlags, &hit, 1, &phys, 1))
								{
									// find approximate time of landing with given velocity
									Vec3 dist(pos - hit.pt); //Distance::Point_Point(pos,hit.pt);
									// use current actual gravity	of the object
									pe_player_dynamics	dyn;
									if (phys->GetParams(&dyn))
									{
										float a = dyn.gravity.z / 2;
										float b = m_stats.velocity.z;
										float c = dist.z;
										float d = b * b - 4 * a * c;
										if (d >= 0)
										{
											float sqrtd = sqrt(d);
											t = (-b + sqrtd) / (2 * a);
											float t1 = (-b - sqrtd) / (2 * a);
											if (t < 0 || t1 >= 0 && t1 < t)
												t = t1;
										}
									}
								}
							}
						}
						if (t >= 0 && t < 2 * frameTime)
						{
							m_jumpParams.state = JS_Landing;
							m_jumpParams.startTime = currTime;
							m_jumpParams.initLandVelocity = m_jumpParams.curVelocity;
							m_jumpParams.landDepth = m_jumpParams.curVelocity.GetNormalizedSafe().z;
						}
						if (t >= 0 && t < m_jumpParams.landPreparationTime && !m_bOverrideFlyActionAnim)
						{
							//IAnimationGraphState* pAGState = GetAnimationGraphState();
							if (pAGState)
							{
								Vec3 vNx(vN);
								vNx.z = 0;
								vNx.NormalizeSafe();
								float dot = vN.Dot(vNx);
								float anglex = RAD2DEG(cry_acosf(CLAMP(dot, -1.f, 1.f)));
								if (m_viewMtx.GetColumn1().Dot(vN) < -0.001) // land backwards
									anglex = 180 - anglex;

								pAGState->SetInput(m_idAngleXInput, anglex);

								Vec3 viewDir(m_viewMtx.GetColumn(1));
								dot = vNx.Dot(viewDir);
								float anglez = RAD2DEG(cry_acosf(CLAMP(dot, -1.f, 1.f)) * sgn(vNx.Cross(viewDir).z));
								pAGState->SetInput(m_idAngleZInput, anglez);

								if (m_jumpParams.bUseSpecialAnim && m_jumpParams.specialAnimType == JUMP_ANIM_LAND)
								{
									if (m_jumpParams.specialAnimAGInput == AIANIM_ACTION)
										pAGState->SetInput(m_idActionInput, m_jumpParams.specialAnimAGInputValue);
									else
									{
										pAGState->SetInput(m_idActionInput, "idle");
										pAGState->SetInput(m_idSignalInput, m_jumpParams.specialAnimAGInputValue);
									}
									m_jumpParams.bPlayingSpecialAnim = true;
								}
								else
									pAGState->SetInput(m_idActionInput, "idle");
							}

							//m_bOverrideFlyActionAnim = false;
							m_jumpParams.bFreeFall = false;
							m_jumpParams.bUseLandAnim = false;
							//if(m_jumpParams.state !=JS_Landing)
							m_jumpParams.state = JS_ApproachLanding;
						}
					}
				}
			}
		}

		//if (m_stats.inAir == 0 && !InZeroG() && (m_jumpParams.state == JS_Flying || m_jumpParams.state==JS_ApproachLanding ))
		if (m_stats.inAir == 0 && m_jumpParams.prevInAir > 0.3f)
		{
			m_jumpParams.bFreeFall = false;
			//m_bOverrideFlyActionAnim = false;
			IAISignalExtraData* pData = NULL;
			if (m_jumpParams.bUseSpecialAnim && m_jumpParams.specialAnimType == JUMP_ANIM_LAND
				&& !m_jumpParams.bPlayingSpecialAnim)
			{
				// something went wrong, trooper landed before playing the special land animation he was supposed to do

				if (GetEntity()->GetAI())
					pData = gEnv->pAISystem->CreateSignalExtraData();

				pData->iValue = 1;
			}
			// send land event/signal

			if (GetEntity()->GetAI())
				gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 1, "OnLand", GetEntity()->GetAI(), pData);

			if (m_jumpParams.bUseLandEvent)
			{
				SEntityEvent event(ENTITY_EVENT_SCRIPT_EVENT);
				event.nParam[0] = (INT_PTR)"Land";
				event.nParam[1] = IEntityClass::EVT_BOOL;
				bool bValue = true;
				event.nParam[2] = (INT_PTR)&bValue;
				GetEntity()->SendEvent(event);
			}
			m_jumpParams.bUseLandEvent = false;

			if (!(m_jumpParams.bUseSpecialAnim && m_jumpParams.specialAnimAGInput == AIANIM_ACTION && m_jumpParams.specialAnimType == JUMP_ANIM_LAND))
			{
				IAnimationGraphState* pAGState = GetAnimationGraphState();
				if (pAGState && !m_bOverrideFlyActionAnim)
				{
					pAGState->SetInput(m_idActionInput, "idle");
				}
				m_jumpParams.state = JS_Landing;
				m_jumpParams.startTime = currTime;
				m_jumpParams.initLandVelocity = m_jumpParams.curVelocity;
				m_jumpParams.landDepth = m_jumpParams.curVelocity.GetNormalizedSafe().z;
			}
			else
				m_jumpParams.state = JS_None;

			m_jumpParams.bUseSpecialAnim = false;

			if (phys)
			{
				pe_player_dynamics simParSet;
				simParSet.bSwimming = false;
				phys->SetParams(&simParSet);
			}
			JumpEffect();
		}
	}
	else // exact positioning
	{
		//CryLogAlways("Exact Positioning");
		m_jumpParams.state = JS_None;
		m_jumpParams.bFreeFall = false;
		m_jumpParams.bUseLandAnim = false;
		m_bOverrideFlyActionAnim = false;
		m_overrideFlyAction = "idle";

		if (phys)
		{
			pe_player_dynamics simParSet;
			simParSet.bSwimming = false;
			phys->SetParams(&simParSet);
		}
	}

	//TheOtherSide
	if (g_pControlSystem->GetLocalEnabled())
	{
		IEntity* pControlledEntity = g_pControlSystem->GetLocalControlClient()->GetControlledEntity();
		if (GetEntity() == pControlledEntity && m_stats.inAir > 0.05f)
			ProcessFlyControl(move, frameTime);
	}
	//~TheOtherSide

	m_stats.desiredSpeed = m_stats.speed;
	m_moveRequest.velocity = move;
	m_velocity.zero();
	m_jumpParams.prevInAir = m_stats.inAir;

	//m_pInput->deltaMovement = Vec3(0, 0, 0);
}

void CTrooper::ProcessAnimation(ICharacterInstance* pCharacter, float frameTime)
{
	IPhysicalEntity* pPhysEnt = GetEntity()->GetPhysics();
	if (pPhysEnt)
	{
		if (m_bExactPositioning || m_stats.inAir > 0)
			m_heightVariance = 0;
		else
		{
			float curTime = gEnv->pSystem->GetITimer()->GetFrameStartTime().GetSeconds();
			float speedCoeff = m_stats.speed / 10;
			if (speedCoeff > 1)
				speedCoeff = 1;
			else if (speedCoeff < 0.1f)
				speedCoeff = 0;
			float range = (m_heightVarianceHigh - m_heightVarianceLow);
			m_heightVariance = speedCoeff * (m_heightVarianceLow + range / 2 + sin((curTime + m_heightVarianceRandomize) * m_heightVarianceFreq * 2 * gf_PI) * range);
		}

		pe_player_dimensions params;
		if (pPhysEnt->GetParams(&params))
		{
			Interpolate(params.heightPivot, m_heightVariance, 3.f, frameTime);
			pPhysEnt->SetParams(&params);
		}

		if (!m_bExactPositioning)
		{
			IAnimationGraphState* pAGState = GetAnimationGraphState();
			if (pAGState)
			{
				if (!m_bOverrideFlyActionAnim && m_overrideFlyAction != "idle")
				{
					pAGState->SetInput(m_idActionInput, m_overrideFlyAction);
					m_bOverrideFlyActionAnim = true;
				}
				else if (m_bOverrideFlyActionAnim && m_overrideFlyAction == "idle")
				{
					pAGState->SetInput(m_idActionInput, "idle");
					m_bOverrideFlyActionAnim = false;
				}
			}
		}

		if (m_jumpParams.state == JS_Landing)
		{
			float landTime = (gEnv->pSystem->GetITimer()->GetFrameStartTime() - m_jumpParams.startTime).GetSeconds();
			if (landTime >= ClandDuration)
			{
				m_landModelOffset = ZERO;
				//m_stats.dynModelOffset = ZERO;
				m_jumpParams.state = JS_None;
			}
			else
			{
				float timeToZero = ClandDuration / 2;
				float frameTime = gEnv->pSystem->GetITimer()->GetFrameTime();
				if (m_jumpParams.curVelocity.z < -0.01f) // going down
				{
					m_jumpParams.curVelocity -= m_jumpParams.initLandVelocity * frameTime * (ClandDuration - timeToZero) * ClandStiffnessMultiplier;
					//Interpolate(m_jumpParams.curVelocity,ZERO,1/(m_landDuration - timeToZero),gEnv->pSystem->GetITimer()->GetFrameTime());
/*					char b[1000];
					sprintf(b,"%f\n",m_jumpParams.curVelocity.z);
					OutputDebugString(b);
*/
					m_landModelOffset += m_baseMtx.GetInverted() * m_jumpParams.curVelocity * frameTime;
				}
				else
				{
					Interpolate(m_landModelOffset, ZERO, 2 / timeToZero, frameTime);
				}
			}
		}
	}

	// simulating inertia when changing speed/direction
	//TheOtherSide
	if (m_steerInertia > 0 && !IsHaveOwner())
	{
		//~TheOtherSide
		Vec3 goalSteelModelOffset(ZERO);
		float dot = 0;
		float interpolateSpeed;
		if (m_stats.inAir <= 0 && !m_bExactPositioning && !m_stats.isGrabbed)
		{
			Vec3 desiredMovement(m_input.movementVector);
			float deslength = desiredMovement.GetLength();
			float maxSpeed = GetStanceInfo(m_stance)->maxSpeed;
			if (deslength > 0.2f && maxSpeed > 0)
			{
				desiredMovement /= deslength;
				Vec3 curMoveDir(m_stats.velocity / maxSpeed);
				float curSpeed = curMoveDir.GetLength();
				if (curSpeed > 0)
					curMoveDir /= curSpeed;
				dot = desiredMovement.Dot(curMoveDir);
				if (dot < 0.9f)
				{
					float acc = m_stats.speed * (1 - dot) / 2;
					goalSteelModelOffset = acc * m_steerInertia * m_stats.velocity.GetNormalizedSafe();
					goalSteelModelOffset = m_baseMtx.GetInverted() * goalSteelModelOffset;
				}
			}
			Interpolate(m_oldVelocity, m_stats.velocity, 2.0f + max(dot, 0.f), frameTime);
			goalSteelModelOffset = m_baseMtx.GetInverted() * (m_oldVelocity - m_stats.velocity) * m_steerInertia;
			/* debug
			Vec3 pos(GetEntity()->GetWorldPos());
			pos.z+=1;
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(pos, ColorB(128,255,128,255), pos + desiredMovement, ColorB(128,255,128,255), 1.0f);
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(pos, ColorB(255,128,128,255), pos + m_stats.velocity, ColorB(255,128,128,255), 1.0f);
			*/
			interpolateSpeed = 2.0f + max(dot, 0.f);
		}
		else if (m_bExactPositioning)
			interpolateSpeed = 6.0f;
		else if (m_stats.isGrabbed)
			interpolateSpeed = 6.0f;
		else
			interpolateSpeed = 4.0f;

		Interpolate(m_steerModelOffset, goalSteelModelOffset, interpolateSpeed, frameTime);
	}

	m_modelOffsetAdd = m_landModelOffset + m_steerModelOffset;

	//Beni - Disable look IK while the trooper is grabbed (special "state")
	if (pCharacter && !IsHaveOwner())
	{
		if (m_stats.isGrabbed)
			pCharacter->GetISkeletonPose()->SetLookIK(false, 0, Vec3(0, 0, 0));
		else
			pCharacter->GetISkeletonPose()->SetLookIK(true, gf_PI * 0.9f, m_stats.lookTargetSmooth);//,m_customLookIKBlends);
	}

	//m_oldSpeed = speed;
}

//---------------------------------
//AI Specific
void CTrooper::SetActorMovement(SMovementRequestParams& control)
{
	//if (IsClient())
	//	return;

	SMovementState state;
	GetMovementController()->GetMovementState(state);

	CAlien::SetActorMovementCommon(control);
	Vec3 mypos(GetEntity()->GetWorldPos());

	const SAnimationTarget* pAnimTarget = GetAnimationGraphState()->GetAnimationTarget();
	if ((pAnimTarget != NULL) && pAnimTarget->preparing)
	{
		//CryLogAlways("%s pAnimTarget->preparing",GetEntity()->GetName());
		float offset = 3.0f;
		Vec3 bodyTarget = pAnimTarget->position + offset * (pAnimTarget->orientation * FORWARD_DIRECTION);// + Vec3(0, 0, 1.5);
		Vec3 bodyDir(bodyTarget - mypos);
		bodyDir.z = 0;
		bodyDir = bodyDir.GetNormalizedSafe(pAnimTarget->orientation * FORWARD_DIRECTION);
		//gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(mypos, ColorB(255,128,128,255), mypos + bodyDir * 10.0f, ColorB(255,255,255,255), 5.0f);
		//gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(bodyTarget, 0.2f, ColorB(255, 255, 255, 255), true);
		SetDesiredDirection(bodyDir);
		/*
		float dist = Distance::Point_Point(state.weaponPosition,pAnimTarget->position);
		float coeff = pAnimTarget->maxRadius - dist;
		if(coeff<0)
			coeff = 0;
		coeff *= 5*coeff;
		Vec3 targetPos = pAnimTarget->position + coeff * (pAnimTarget->orientation * FORWARD_DIRECTION) + Vec3(0, 0, 1.5);
		SetDesiredDirection((targetPos - state.weaponPosition).GetNormalizedSafe());
		*/

		pAnimTarget->notAiControlledAnymore = true;
	}
	else if (!control.vAimTargetPos.IsZero())
		SetDesiredDirection((control.vAimTargetPos - state.weaponPosition).GetNormalizedSafe());
	else if (!control.vLookTargetPos.IsZero())
		SetDesiredDirection((control.vLookTargetPos - state.eyePosition).GetNormalizedSafe());
	else if (!control.vMoveDir.IsZero() && control.fDesiredSpeed > 0 && (!pAnimTarget || !pAnimTarget->notAiControlledAnymore))
		SetDesiredDirection(control.vMoveDir);
	else
		SetDesiredDirection(GetEntity()->GetWorldRotation() * FORWARD_DIRECTION);

	SetDesiredSpeed(control.vMoveDir * control.fDesiredSpeed);

	//	m_input.actions = control.m_desiredActions;
	int actions;
	switch (control.bodystate)
	{
	case 1:
		actions = ACTION_CROUCH;
		break;
	case 2:
		actions = ACTION_PRONE;
		break;
	case 3:
		actions = ACTION_RELAXED;
		break;
	case 4:
		actions = ACTION_STEALTH;
		break;
	default:
		actions = 0;
		break;
	}

	// Override the stance based on special behavior.
	//SetActorStance(control, actions);

	//	SetTilt(control,actions);

	m_input.actions = actions;

	Vec3	fireDir = GetEntity()->GetWorldRotation() * FORWARD_DIRECTION;
	if (!control.vAimTargetPos.IsZero())
	{
		fireDir = control.vAimTargetPos - state.weaponPosition;
		fireDir.NormalizeSafe();
	}
	if (IScriptTable* pScriptTable = GetEntity()->GetScriptTable())
		pScriptTable->SetValue("fireDir", fireDir);

	m_fDistanceToPathEnd = control.fDistanceToPathEnd;
	m_bExactPositioning = pAnimTarget && pAnimTarget->notAiControlledAnymore;
}

void CTrooper::SetActorStance(SMovementRequestParams& control, int& actions)
{
	IPuppet* pPuppet;
	if (GetEntity() && GetEntity()->GetAI() && (pPuppet = GetEntity()->GetAI()->CastToIPuppet()))
	{
		float distance = control.fDistanceToPathEnd;
		if (m_stance == STANCE_PRONE)
		{
			IAIActor* pAIActor = CastToIAIActorSafe(GetEntity()->GetAI());
			if (pAIActor)
			{
				SOBJECTSTATE* pAIState(pAIActor->GetState());
				if (pAIState && (pAIState->allowStrafing || distance < g_pGame->GetCVars()->g_trooperProneMinDistance))
					pAIState->bodystate = BODYPOS_RELAX;
			}
			actions = ACTION_RELAXED;
		}
	}
}

//bool CTrooper::UpdateStance()
//{
//	if (m_stance != GetStance())
//	{
//		EStance oldStance = m_stance;
//		m_stance = GetStance();
//		StanceChanged( oldStance );
//
//		IPhysicalEntity *pPhysEnt = GetEntity()->GetPhysics();
//		if (pPhysEnt)
//		{
//			pe_player_dimensions playerDim;
//			const SStanceInfo *sInfo = GetStanceInfo(m_stance);
//
//			playerDim.heightEye = 0.0f;
//			playerDim.heightCollider = sInfo->heightCollider;
//			playerDim.sizeCollider = sInfo->size;
////			playerDim.heightPivot = m_bExactPositioning ? 0 : m_heightVariance;
//			playerDim.maxUnproj = max(0.0f,sInfo->heightPivot);
//			playerDim.bUseCapsule = sInfo->useCapsule;
//
//			int result(pPhysEnt->SetParams(&playerDim));
//
//			pe_action_awake aa;
//			aa.bAwake = 1;
//			pPhysEnt->Action(&aa);
//		}
//	}
//	return true;
//}

void CTrooper::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	GetAlienMemoryStatistics(s);
}

void CTrooper::Jump()
{
	if (m_jumpParams.velocity.IsZero())
		return;

	IPhysicalEntity* phys = GetEntity()->GetPhysics();
	m_jumpParams.state = phys ? JS_ApplyImpulse : JS_None;
}

void CTrooper::JumpEffect()
{
	IMaterialEffects* pMaterialEffects = g_pGame->GetIGameFramework()->GetIMaterialEffects();
	TMFXEffectId effectId = pMaterialEffects->GetEffectId("trooper_jump", m_stats.groundMaterialIdx);
	if (effectId != InvalidEffectId)
	{
		SMFXRunTimeEffectParams fxparams;
		fxparams.pos = GetEntity()->GetWorldPos();
		fxparams.soundSemantic = eSoundSemantic_Physics_Footstep;
		pMaterialEffects->ExecuteEffect(effectId, fxparams);
	}
}

void CTrooper::AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event)
{
	if (stricmp(event.m_EventName, "jump") == 0 && m_jumpParams.state != JS_Flying)
		Jump();
	//else
	CAlien::AnimationEvent(pCharacter, event);
}

void CTrooper::ResetAnimations()
{
	ICharacterInstance* character = GetEntity()->GetCharacter(0);

	if (character)
	{
		if (m_pAnimatedCharacter)
		{
			m_pAnimatedCharacter->ClearForcedStates();
			//m_pAnimatedCharacter->GetAnimationGraphState()->Pause(true, eAGP_StartGame);
		}

		character->GetISkeletonAnim()->StopAnimationsAllLayers();
		character->GetISkeletonPose()->SetLookIK(false, gf_PI * 0.9f, m_stats.lookTargetSmooth);//,m_customLookIKBlends);
	}
}

void CTrooper::BindInputs(IAnimationGraphState* pAGState)
{
	CAlien::BindInputs(pAGState);
	if (pAGState)
	{
		m_idAngleXInput = pAGState->GetInputId("AngleX");
		m_idAngleZInput = pAGState->GetInputId("AngleZ");
		m_idActionInput = pAGState->GetInputId("Action");
		m_idSignalInput = pAGState->GetInputId("Signal");
		m_idMovementInput = pAGState->GetInputId("MovementDir");
	}
}

void CTrooper::FullSerialize(TSerialize ser)
{
	CAlien::FullSerialize(ser);
	ser.Value("m_modelQuat", m_modelQuat);
	ser.Value("m_lastNotMovingTime", m_lastNotMovingTime);
	ser.Value("m_oldSpeed", m_oldSpeed);
	ser.Value("m_heightVariance", m_heightVariance);
	ser.Value("m_fDistanceToPathEnd", m_fDistanceToPathEnd);
	ser.Value("m_Roll", m_Roll);
	ser.Value("m_Rollx", m_Rollx);
	ser.Value("m_bExactPositioning", m_bExactPositioning);
	ser.Value("m_lastExactPositioningTime", m_lastExactPositioningTime);
	ser.Value("m_lastTimeOnGround", m_lastTimeOnGround);
	ser.Value("m_overrideFlyAction", m_overrideFlyAction);
	ser.Value("m_bOverrideFlyActionAnim", m_bOverrideFlyActionAnim);
	ser.Value("m_heightVarianceLow", m_heightVarianceLow);
	ser.Value("m_heightVarianceHigh", m_heightVarianceHigh);
	ser.Value("m_heightVarianceFreq", m_heightVarianceFreq);
	ser.Value("m_heightVarianceRandomize", m_heightVarianceRandomize);
	ser.Value("m_oldDirStrafe", m_oldDirStrafe);
	ser.Value("m_oldDirFwd", m_oldDirFwd);
	ser.Value("m_steerInertia", m_steerInertia);
	ser.Value("m_landModelOffset", m_landModelOffset);
	ser.Value("m_steerModelOffset", m_steerModelOffset);
	ser.Value("m_oldVelocity", m_oldVelocity);
	ser.Value("m_fTtentacleBlendRotation", m_fTtentacleBlendRotation);
	m_jumpParams.Serialize(ser);

	//TheOtherSide
	m_lamStats.Serialize(ser);
	m_shieldParams.Serialize(ser);
	//~TheOtherSide
}

bool CTrooper::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	int syncMode = 2;
	if (syncMode == 1)
	{
		//if (!CAlien::NetSerialize(ser, aspect, profile, flags))
	//	return false;

		float color[] = { 1,1,1,1 };
		gEnv->pRenderer->Draw2dLabel(20, 20, 1.15f, color, false, "[Trooper][NetSerialize Mode 1]");

		//if (gEnv->bClient)
		//	gEnv->pRenderer->Draw2dLabel(20, 40, 1.3f, color, false, "%s Writing OnClient", (string)GetEntity()->GetName());
		//if (gEnv->bServer)
		//	gEnv->pRenderer->Draw2dLabel(20, 60, 1.3f, color, false, "%s Writing OnServer", (string)GetEntity()->GetName());
		//if (!gEnv->bServer && gEnv->bClient)
		//	gEnv->pRenderer->Draw2dLabel(20, 80, 1.3f, color, false, "%s Writing OnRemoteClient", (string)GetEntity()->GetName());

		if (aspect == IPlayerInput::INPUT_ASPECT)
		{
			if (gEnv->bClient)
				gEnv->pRenderer->Draw2dLabel(20, 40, 1.15f, color, false, "[Trooper][Aspect][INPUT_ASPECT][CLIENT]");
			if (gEnv->bServer)
				gEnv->pRenderer->Draw2dLabel(20, 60, 1.15f, color, false, "[Trooper][Aspect][INPUT_ASPECT][SERVER]");
			if (!gEnv->bServer && gEnv->bClient)
				gEnv->pRenderer->Draw2dLabel(20, 80, 1.15f, color, false, "[Trooper][Aspect][INPUT_ASPECT][DEDICATED]");

			Vec3 bodyDir(FORWARD_DIRECTION);
			Vec3 lookDir(FORWARD_DIRECTION);
			Vec3 deltaMov(ZERO);

			if (/*m_pPlayerInput.get() &&*/ ser.IsWriting())
			{
				//m_pPlayerInput->GetState(serializedInput);

				lookDir = bodyDir = GetAlienViewMtx().GetColumn1().GetNormalized();
				deltaMov = m_input.deltaMovement;

				/*float color[] = { 1,1,1,1 };

				if (gEnv->bClient)
					gEnv->pRenderer->Draw2dLabel(20, 40, 1.3f, color, false, "%s Writing OnClient", (string)GetEntity()->GetName());
				if (gEnv->bServer)
					gEnv->pRenderer->Draw2dLabel(20, 60, 1.3f, color, false, "%s Writing OnServer", (string)GetEntity()->GetName());
				if (!gEnv->bServer && gEnv->bClient)
					gEnv->pRenderer->Draw2dLabel(20, 80, 1.3f, color, false, "%s Writing OnRemoteClient", (string)GetEntity()->GetName());*/
			}

			//ser.Value("stance", stance, 'stnc');
			// note: i'm not sure what some of these parameters mean, but i copied them from the defaults in serpolicy.h
			// however, the rounding mode for this value must ensure that zero gets sent as a zero, not anything else, or things break rather badly
			ser.Value("deltaMovement", deltaMov, 'pMov');
			ser.Value("lookDirection", lookDir, 'dir0');
			//ser.Value("sprint", sprint, 'bool');
			//ser.Value("leanl", leanl, 'bool');
			//ser.Value("leanr", leanr, 'bool');


			if (/*m_pPlayerInput.get() &&*/ ser.IsReading())
			{
				//m_pPlayerInput->SetState(serializedInput);

				SetAlienMove(deltaMov);
				SetDesiredDirection(lookDir);

				//float color[] = { 1,1,1,1 };

				//if (gEnv->bClient)
				//	gEnv->pRenderer->Draw2dLabel(20, 100, 1.3f, color, false, "%s Reading OnClient", (string)GetEntity()->GetName());
				//if (gEnv->bServer)
				//	gEnv->pRenderer->Draw2dLabel(20, 120, 1.3f, color, false, "%s Reading OnServer", (string)GetEntity()->GetName());
				//if (!gEnv->bServer && gEnv->bClient)
				//	gEnv->pRenderer->Draw2dLabel(20, 140, 1.3f, color, false, "%s Reading OnRemoteClient", (string)GetEntity()->GetName());
			}
		}
	}
	else if (syncMode == 2)
	{
		//Need for physics serialization (position, impulses and other)
		if (!CActor::NetSerialize(ser, aspect, profile, flags))
			return false;

		float color[] = { 1,1,1,1 };
		gEnv->pRenderer->Draw2dLabel(20, 20, 1.15f, color, false, "[Trooper][NetSerialize Mode 2]");

		if (aspect == ASPECT_HEALTH)
		{

			ser.Value("alienHealth", m_health);

			bool isFrozen = m_stats.isFrozen;
			ser.Value("alienIsFrozen", isFrozen, 'bool');
			ser.Value("alienFrozenAmount", m_frozenAmount, 'frzn');
		}

		if (aspect == ASPECT_CURRENT_ITEM)
		{
			bool reading = ser.IsReading();
			bool hasWeapon = false;
			if (!reading)
				hasWeapon = NetGetCurrentItem() != 0;

			ser.Value("alienHasWeapon", hasWeapon, 'bool');
			ser.Value("alienCurrentItemId", static_cast<CActor*>(this), &CActor::NetGetCurrentItem, &CActor::NetSetCurrentItem, 'eid');

			if (reading && hasWeapon && NetGetCurrentItem() == 0) // fix the case where this guy's weapon might not have been bound on this client yet
				ser.FlagPartialRead();
		}

		m_energyParams.Serialize(ser, aspect);

		if (aspect == ASPECT_INPUT)
		{
			SSerializedAlienInput serializedInput;

			if (ser.IsWriting())
				GetInputState(serializedInput);

			serializedInput.Serialize(ser);

			if (ser.IsReading())
				SetInputState(serializedInput);
		}

	}
	return true;
}

void CTrooper::SetupLamLights()
{
	//Akeeper: Lasers very unstable and cause crashes
	if (m_isUseCloak || true)
		return;

	//CryLogAlways("Setup trooper lasers...");

	// setup lasers
	static IEntityClass* pLamOneClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TrooperSeekBeam_0");
	static IEntityClass* pLamTwoClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TrooperSeekBeam_1");
	static IEntityClass* pLamThreeClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TrooperSeekBeam_2");

	m_lamStats.lamOneId = GetInventory()->GetItemByClass(pLamOneClass);
	m_lamStats.lamTwoId = GetInventory()->GetItemByClass(pLamTwoClass);
	m_lamStats.lamThreeId = GetInventory()->GetItemByClass(pLamThreeClass);

	if (m_lamStats.lamOneId != 0)
		m_lamStats.pLamOneEntity = gEnv->pEntitySystem->GetEntity(m_lamStats.lamOneId);

	if (m_lamStats.lamTwoId)
		m_lamStats.pLamTwoEntity = gEnv->pEntitySystem->GetEntity(m_lamStats.lamTwoId);

	if (m_lamStats.lamThreeId)
		m_lamStats.pLamThreeEntity = gEnv->pEntitySystem->GetEntity(m_lamStats.lamThreeId);

	if (m_lamStats.pLamOneEntity && m_lamStats.pLamTwoEntity && m_lamStats.pLamThreeEntity)
	{
		m_lamStats.pLamOneEntity->DetachAll(/*IEntity::ATTACHMENT_KEEP_TRANSFORMATION*/);

		Vec3 vLaserOnePos(m_lamStats.pLamOneEntity->GetWorldPos());

		Vec3 vLaserTwoPos(vLaserOnePos.x + 0.06f, vLaserOnePos.y, vLaserOnePos.z - 0.06f);
		Vec3 vLaserThreePos(vLaserOnePos.x - 0.06f, vLaserOnePos.y, vLaserOnePos.z - 0.06f);

		m_lamStats.pLamTwoEntity->SetWorldTM(Matrix34::CreateTranslationMat(vLaserTwoPos));
		m_lamStats.pLamThreeEntity->SetWorldTM(Matrix34::CreateTranslationMat(vLaserThreePos));

		m_lamStats.pLamTwoEntity->SetRotation(m_lamStats.pLamOneEntity->GetRotation());
		m_lamStats.pLamThreeEntity->SetRotation(m_lamStats.pLamOneEntity->GetRotation());

		m_lamStats.pLamOneEntity->AttachChild(m_lamStats.pLamTwoEntity, IEntity::ATTACHMENT_KEEP_TRANSFORMATION);
		m_lamStats.pLamOneEntity->AttachChild(m_lamStats.pLamThreeEntity, IEntity::ATTACHMENT_KEEP_TRANSFORMATION);
	}
}

bool CTrooper::EnableLamLights(bool enable)
{
	if (m_isUseCloak)
		return false;

	//CryLogAlways("EnableLamLights %d", enable);
	if (!(m_lamStats.lamOneId && m_lamStats.pLamOneEntity))
		return false;
	if (!(m_lamStats.lamTwoId && m_lamStats.pLamTwoEntity))
		return false;
	if (!(m_lamStats.lamThreeId && m_lamStats.pLamThreeEntity))
		return false;

	if (enable && m_empInfo.isEmpState)
		return false;

	m_lamStats.isActive = enable;

	CLam* pLamOne = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamOneId);
	CLam* pLamTwo = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamTwoId);
	CLam* pLamThree = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamThreeId);
	if (pLamOne && pLamTwo && pLamThree)
	{
		pLamOne->ActivateLaser(enable, true);
		pLamTwo->ActivateLaser(enable, true);
		pLamThree->ActivateLaser(enable, true);
		return true;
	}

	return false;
}

void CTrooper::ForceDisableLasers()
{
	if (!(m_lamStats.lamOneId && m_lamStats.pLamOneEntity))
		return;
	if (!(m_lamStats.lamTwoId && m_lamStats.pLamTwoEntity))
		return;
	if (!(m_lamStats.lamThreeId && m_lamStats.pLamThreeEntity))
		return;

	m_lamStats.isActive = false;

	CLam* pLamOne = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamOneId);
	CLam* pLamTwo = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamTwoId);
	CLam* pLamThree = (CLam*)g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(m_lamStats.lamThreeId);

	if (pLamOne && pLamTwo && pLamThree)
	{
		pLamOne->ActivateLaser(false, true);
		pLamTwo->ActivateLaser(false, true);
		pLamThree->ActivateLaser(false, true);
	}

	m_lamStats = STrooperLamStats();
}

void CTrooper::UpdateLamLights(float frametime, ICharacterInstance* pCharacter)
{
	return;
	
	SMovementState state;
	GetMovementController()->GetMovementState(state);

	if (GetHealth() <= 0 || m_isUseCloak)
	{
		if (m_lamStats.isActive)
			ForceDisableLasers();

		return;
	}
		

	if (m_empInfo.isEmpState && m_lamStats.isActive)
	{
		EnableLamLights(false);
		return;
	}

	CPlayer* pOwner = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetOwnerId()));

	if (GetCurrentItem())
	{
		CWeapon* pWeapon = static_cast<CWeapon*>(GetCurrentItem()->GetIWeapon());
		if (pWeapon)
		{
			if (pOwner)
			{
				if (m_lamStats.isActive && !pWeapon->IsFiring())
				{
					static float timerStart = 5;
					Interpolate(timerStart, 0, 1.f, frametime);

					//float color[] = { 1,1,1,1 };
					//gEnv->pRenderer->Draw2dLabel(20, 20, 1.3f, color, false, "timerStart %f", timerStart);

					if (timerStart <= 0.1f)
					{
						EnableLamLights(false);
						timerStart = 5;
					}
				}
				else if (!m_lamStats.isActive && pWeapon->IsFiring() && GetHealth() > 0.1f)
					EnableLamLights(true);
			}
		}
	}

	if (m_lamStats.pLamOneEntity && m_lamStats.pLamTwoEntity && m_lamStats.pLamThreeEntity)
	{
		Matrix34 boneWorldMatrix = GetEntity()->GetSlotWorldTM(0) * Matrix34(pCharacter->GetISkeletonPose()->GetAbsJointByID(GetBoneID(BONE_WEAPON)));

		Vec3 laserOnePos = boneWorldMatrix.GetTranslation();
		Vec3 laserOneDir(0, 0, 0);

		bool ok = false;

		if (pOwner)
			laserOneDir = state.fireDirection;
		else
		{
			//true - the laser dir defined from the trooper ai
			IAIObject* pTrooperAI = GetEntity()->GetAI();
			if (pTrooperAI)
			{
				IPipeUser* pPipeUser = pTrooperAI->CastToIPipeUser();
				if (pPipeUser)
				{
					IAIObject* pAttTarget = pPipeUser->GetAttentionTarget();
					if (pAttTarget)
					{
						IEntity* pTarget = pAttTarget->GetEntity();
						if (pTarget)
						{
							AABB tgbounds;
							pTarget->GetWorldBounds(tgbounds);

							Vec3 laserTargetPos = tgbounds.GetCenter();
							laserOneDir = (laserTargetPos - laserOnePos).GetNormalized();

							//Auto Enable lasers when AI see target
							if (!m_lamStats.isActive && GetHealth() > 0.1f)
							{
								EnableLamLights(true);
							}

							ok = true;
						}
					}
				}
			}

			//if laser dir not defined from the trooper ai
			if (!ok)
			{
				//Auto disable lasers when ai not see targets
				if (m_lamStats.isActive)
				{
					EnableLamLights(false);
				}

				//vLaserOneDir = GetEntity()->GetWorldTM().GetColumn1().GetNormalized();
			}
		}

		//Prevent crash in editor when press the ESC key
		if (!g_pGame->GetIGameFramework()->IsEditing())
		{
			SetLamLightsPos(laserOnePos);
			SetLamLightsDir(laserOneDir);
		}



		//float color[] = { 1,1,1,1 };
		//gEnv->pRenderer->Draw2dLabel(20, 50, 1.3f, color, false, "owner %d ok %d laser one world pos (%f,%f,%f)", int(pOwner!=0), int(ok), vLaserOnePos.x, vLaserOnePos.y, vLaserOnePos.z);
		//gEnv->pRenderer->Draw2dLabel(20, 70, 1.3f, color, false, "laser two world pos (%f,%f,%f)", vLaserTwoPos.x, vLaserTwoPos.y, vLaserTwoPos.z);
		//gEnv->pRenderer->Draw2dLabel(20, 90, 1.3f, color, false, "laser three world pos (%f,%f,%f)", vLaserThreePos.x, vLaserThreePos.y, vLaserThreePos.z);
	}
}

void CTrooper::SetLamLightsPos(Vec3 pos1)
{
	auto mat = Matrix34::CreateTranslationMat(pos1);

	//In Conquest call crash some times
	if (m_lamStats.pLamOneEntity)
		m_lamStats.pLamOneEntity->SetWorldTM(mat);
}

void CTrooper::SetLamLightsDir(Vec3 dir1)
{
	if (m_lamStats.pLamOneEntity)
		m_lamStats.pLamOneEntity->SetRotation(Quat::CreateRotationVDir(dir1));
}

void CTrooper::OnAction(const ActionId& actionId, int activationMode, float value)
{
}

void CTrooper::STrooperRageMode::ToggleMode(const bool toggle, const float rageDuration)
{
	if (!pTrooper)
		return;

	if (pTrooper->m_shieldParams.shieldType == eShieldType_Owner)
	{
		CSingle* pSingleFM = 0;
		CWeapon* pWeapon = g_pControlSystem->GetLocalControlClient()->GetCurrentWeapon(pTrooper);
		if (pWeapon)
		{
			const int fireMode = pWeapon->GetCurrentFireMode();
			pSingleFM = static_cast<CSingle*>(pWeapon->GetFireMode(fireMode));
		}

		if (toggle)
		{
			if (!isActive && !isReloading)
			{
				rageMaxDuration = rageDuration;
				isActive = true;

				if (pTrooper->IsLocalOwner())
					gEnv->p3DEngine->SetPostEffectParam("FilterChromaShift_User_Amount", GetPostFXValue());

				//Disable overheating while in the rage mode
				if (pSingleFM && pSingleFM->CanOverheat())
					pSingleFM->ToggleHeating(false);
			}
		}
		else
		{
			if (isActive)
			{
				isActive = false;
				rageMaxDuration = 0.0f;

				if (!isReloading)
				{
					reloadDuration = GetReloadDuration();
					isReloading = true;
				}

				if (pTrooper->IsLocalOwner())
					gEnv->p3DEngine->SetPostEffectParam("FilterChromaShift_User_Amount", 0.0f);

				//Enable overheating while in the normal mode
				if (pSingleFM && pSingleFM->CanOverheat() && !pSingleFM->m_IsAllowHeating)
					pSingleFM->ToggleHeating(true);
			}
		}
	}
}

void CTrooper::STrooperRageMode::Update(const float frametime)
{
	if (!pTrooper)
		return;

	if (pTrooper->m_shieldParams.shieldType == eShieldType_Owner)
	{
		CSingle* pFiremode = 0;
		CWeapon* pWeapon = g_pControlSystem->GetLocalControlClient()->GetCurrentWeapon(pTrooper);
		if (pWeapon)
		{
			const int fireMode = pWeapon->GetCurrentFireMode();
			pFiremode = static_cast<CSingle*>(pWeapon->GetFireMode(fireMode));
		}

		if (isActive)
		{
			if (rageMaxDuration > 0)
				rageMaxDuration -= frametime;
			else
			{
				isActive = false;

				reloadDuration = GetReloadDuration();
				isReloading = true;
			}

			if (rageMaxDuration < 0)
				rageMaxDuration = 0;
		}

		if (isReloading)
		{
			if (pTrooper->IsLocalOwner())
			{
				static float value = 0.f;
				gEnv->p3DEngine->GetPostEffectParam("FilterChromaShift_User_Amount", value);

				if (value > 0.0f)
					gEnv->p3DEngine->SetPostEffectParam("FilterChromaShift_User_Amount", 0.0f);
			}

			//Enable overheating while in the normal mode
			if (pFiremode && pFiremode->CanOverheat() && !pFiremode->m_IsAllowHeating)
				pFiremode->ToggleHeating(true);

			if (reloadDuration > 0)
				reloadDuration -= frametime;

			if (reloadDuration < 0)
				reloadDuration = 0;

			if (reloadDuration == 0)
				isReloading = false;
		}

		//draw debug
		if (g_pGameCVars->ctrl_debug_draw == 1)
		{
			static float c[] = { 1,1,1,1 };
			gEnv->pRenderer->Draw2dLabel(20, 320, 1.15f, c, false, "Trooper Rage: Reload duration %1.f", reloadDuration);
			gEnv->pRenderer->Draw2dLabel(20, 340, 1.15f, c, false, "Trooper Rage: Is reloading %d", isReloading);
			gEnv->pRenderer->Draw2dLabel(20, 360, 1.15f, c, false, "Trooper Rage: Rage duration %1.f", rageMaxDuration);
			gEnv->pRenderer->Draw2dLabel(20, 380, 1.15f, c, false, "Trooper Rage: Is active %d", isActive);
		}
	}
}