#include "StdAfx.h"
#include "EnergyСonsumer.h"
#include "Game.h"
#include "GameCVars.h"

#include "TheOtherSideMP/Helpers/TOS_NET.h"

#define DEFAULT_ENERGY 200

string CTOSEnergyConsumer::s_debugEntityName = "";

CTOSEnergyConsumer::CTOSEnergyConsumer()
	: m_energy(0),
	m_maxEnergy(0),
	m_regenStartDelay(0),
	m_drainValue(0),
	m_enableUpdate(true)
{
	
}

CTOSEnergyConsumer::~CTOSEnergyConsumer()
{
	
}

bool CTOSEnergyConsumer::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetGameObject()->EnablePostUpdates(this);

	return true;
}

void CTOSEnergyConsumer::PostInit(IGameObject* pGameObject)
{
	Reset();

	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_Always);
	pGameObject->EnablePostUpdates(this);

	CryLogAlways("[%s][%s][%s] for GameObject %s with energy %1.f", 
		TOS_Debug::GetEnv(), 
		TOS_Debug::GetAct(1), 
		__FUNCTION__, 
		pGameObject->GetEntity()->GetName(),
		m_maxEnergy);
}

void CTOSEnergyConsumer::InitClient(int channelId)
{
	
}

void CTOSEnergyConsumer::PostInitClient(int channelId)
{
	
}

void CTOSEnergyConsumer::Release()
{
	delete this;
}

void CTOSEnergyConsumer::FullSerialize(TSerialize ser)
{
	
}

bool CTOSEnergyConsumer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (aspect == TOS_NET::SERVER_ASPECT_STATIC)
	{
		ser.Value("energy", m_energy);
		ser.Value("maxEnergy", m_maxEnergy);
		ser.Value("drain", m_drainValue);
	}

	return true;
}

void CTOSEnergyConsumer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (!m_enableUpdate)
		return;

	constexpr float rechargeTime = 20.0f;
	const float frameTime = ctx.fFrameTime;
	float regenRate = m_maxEnergy / max(0.01f, rechargeTime);

	if (m_drainValue > 0.0f)
		regenRate = min(regenRate - max(1.0f, m_drainValue), -max(1.0f, m_drainValue));

	if (gEnv->bServer)
	{
		if (regenRate < 0.0f || m_regenStartDelay <= 0.0f)
		{
			SetEnergy(clamp(m_energy + regenRate * ctx.fFrameTime, 0.0f, m_maxEnergy));
		}
	}

	if (m_regenStartDelay > 0.0f)
		m_regenStartDelay = max(0.0f, m_regenStartDelay - frameTime);
}

void CTOSEnergyConsumer::HandleEvent(const SGameObjectEvent&)
{
	
}

void CTOSEnergyConsumer::ProcessEvent(SEntityEvent&)
{
	
}

void CTOSEnergyConsumer::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

bool CTOSEnergyConsumer::SetEnergy(float value, const bool initiated)
{
	value = clamp(value, 0.0f, m_maxEnergy);
	//if (value != m_energy && gEnv->bServer)
		//GetGameObject()->ChangedNetworkState(CPlayer::ASPECT_NANO_SUIT_ENERGY);

	if (value != m_energy)
	{
		// call listeners on nano energy change
		//if (m_listeners.empty() == false)
		//{
		//	std::vector<INanoSuitListener*>::iterator iter = m_listeners.begin();
		//	while (iter != m_listeners.end())
		//	{
		//		(*iter)->EnergyChanged(value);
		//		++iter;
		//	}
		//}
	}

	if (value < m_energy)
	{
		//if (!playerInitiated)
		//{
			//armor mode hit fx (in armor mode energy is decreased by damage
			/*if (m_energy-value>=NANOSUIT_ENERGY * 0.2f) //now always happening on hit
			{
				if(m_pOwner && !m_pOwner->IsGod() && !m_pOwner->IsThirdPerson() && (m_currentMode == NANOMODE_DEFENSE))
				{
					IMaterialEffects* pMaterialEffects = gEnv->pGame->GetIGameFramework()->GetIMaterialEffects();
					SMFXRunTimeEffectParams params;
					params.pos = m_pOwner->GetEntity()->GetWorldPos();
					params.soundSemantic = eSoundSemantic_NanoSuit;
					TMFXEffectId id = pMaterialEffects->GetEffectIdByName("player_fx", "player_damage_armormode");
					pMaterialEffects->ExecuteEffect(id, params);
				}
			}*/

			// if we cross the 20% boundary we don't regenerate for 3secs
			/*
			if (gEnv->bMultiplayer && 
				value/m_maxEnergy <= 0.2f && 
				m_energy>value && 
				g_pGameCVars->g_mpSpeedRechargeDelay)
				m_regenStartDelay=3.0f;
			*/
		//}

		if (!gEnv->bMultiplayer)
		{
			//m_regenStartDelay = g_pGameCVars->g_playerSuitEnergyRechargeDelay; == 1.0
			m_regenStartDelay = 1.0f;
		}

		if (!initiated)
		{
			// if we cross the 20% boundary we don't regenerate for 3secs
			if (gEnv->bMultiplayer &&
				value / m_maxEnergy <= 0.2f &&
				m_energy > value &&
				g_pGameCVars->g_mpSpeedRechargeDelay)
			{
				m_regenStartDelay = 3.0f;
			}
		}

		// spending energy cancels invulnerability
		//if (m_invulnerable && gEnv->bServer)
			//SetInvulnerability(false);
	}

	m_energy = value;

	if (gEnv->bServer)
	{
		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}

	return true;
}

float CTOSEnergyConsumer::GetEnergy() const
{
	return m_energy;
}

bool CTOSEnergyConsumer::SetMaxEnergy(const float value)
{
	m_maxEnergy = value;
	return true;
}

float CTOSEnergyConsumer::GetMaxEnergy() const
{
	return m_maxEnergy;
}

bool CTOSEnergyConsumer::SetDrainValue(const float value)
{
	m_drainValue = value;
	return true;
}

float CTOSEnergyConsumer::GetDrainValue() const
{
	return m_drainValue;
}

void CTOSEnergyConsumer::EnableUpdate(const bool enable)
{
	m_enableUpdate = enable;
}

bool CTOSEnergyConsumer::IsUpdating() const
{
	return m_enableUpdate;
}

void CTOSEnergyConsumer::Reset()
{
	if (gEnv->bServer)
	{
		m_energy = m_maxEnergy = DEFAULT_ENERGY;
		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}
}

bool CTOSEnergyConsumer::SetDebugEntityName(const char* name)
{
	s_debugEntityName = name;

	return true;
}


