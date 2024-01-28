#include "StdAfx.h"
#include "EnergyСonsumer.h"
#include "Game.h"
#include "GameCVars.h"

#include "TheOtherSideMP/Helpers/TOS_NET.h"

string CTOSEnergyConsumer::s_debugEntityName = "";

CTOSEnergyConsumer::CTOSEnergyConsumer()
	: m_energy(0),
	m_maxEnergy(0),
	m_regenStartDelay(0),
	m_regenStartDelaySP(0),
	m_regenStartDelayMP(0),
	m_regenStartDelay20Boundary(0),
	m_rechargeTimeSP(0),
	m_rechargeTimeMP(0),
	m_drainValue(0),
	m_enableUpdate(true) { }

CTOSEnergyConsumer::~CTOSEnergyConsumer() { }

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

	CryLogAlwaysDev("[%s][%s][%s] for GameObject %s with energy %1.f", TOS_Debug::GetEnv(), TOS_Debug::GetAct(1), __FUNCTION__, pGameObject->GetEntity()->GetName(), m_maxEnergy);
}

void CTOSEnergyConsumer::InitClient(int channelId) { }

void CTOSEnergyConsumer::PostInitClient(int channelId) { }

void CTOSEnergyConsumer::Release()
{
	delete this;
}

void CTOSEnergyConsumer::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("TOSEnergyConsumer");
	ser.Value("energy", m_energy);
	ser.Value("maxEnergy", m_maxEnergy);
	ser.Value("drain", m_drainValue);
	ser.Value("regenStartDelay", m_regenStartDelay);
	ser.Value("regenStartDelayMP", m_regenStartDelayMP);
	ser.Value("regenStartDelaySP", m_regenStartDelaySP);
	ser.Value("enableUpdate", m_enableUpdate);
	ser.EndGroup();
}

bool CTOSEnergyConsumer::NetSerialize(TSerialize ser, const EEntityAspects aspect, uint8 profile, int flags)
{
	if (aspect == TOS_NET::SERVER_ASPECT_STATIC)
	{
		ser.Value("energy", m_energy);
		ser.Value("maxEnergy", m_maxEnergy);
		ser.Value("drain", m_drainValue);
		ser.Value("regenStartDelay", m_regenStartDelay);
	}

	return true;
}

void CTOSEnergyConsumer::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (!m_enableUpdate)
		return;

	const float		rechargeTime = gEnv->bMultiplayer ? m_rechargeTimeMP : m_rechargeTimeSP;
	const float     frameTime    = ctx.fFrameTime;
	float           regenRate    = m_maxEnergy / max(0.01f, rechargeTime);

	if (m_drainValue > 0.0f)
		regenRate = min(regenRate - max(1.0f, m_drainValue), -max(1.0f, m_drainValue));

	if (gEnv->bServer)
	{
		if (regenRate < 0.0f || m_regenStartDelay <= 0.0f)
			SetEnergy(clamp(m_energy + regenRate * ctx.fFrameTime, 0.0f, m_maxEnergy));
	}

	if (m_regenStartDelay > 0.0f)
		m_regenStartDelay = max(0.0f, m_regenStartDelay - frameTime);
}

void CTOSEnergyConsumer::HandleEvent(const SGameObjectEvent&) { }

void CTOSEnergyConsumer::ProcessEvent(SEntityEvent&) { }

void CTOSEnergyConsumer::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
}

bool CTOSEnergyConsumer::AddEnergy(const float value)
{
	if (gEnv->bServer)
	{
		SetEnergy(m_energy + value);
		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}
	else if (gEnv->bClient)
	{
		NetEnergyParams params;
		params.energy = m_energy + value;

		GetGameObject()->InvokeRMI(SvRequestSetEnergy(), params, eRMI_ToServer);
	}


	return true;
}

bool CTOSEnergyConsumer::SetEnergy(float value, const bool initiated)
{
	value = clamp(value, 0.0f, m_maxEnergy);

	if (gEnv->bServer)
	{
		if (value < m_energy)
		{
			m_regenStartDelay = gEnv->bMultiplayer ? m_regenStartDelayMP : m_regenStartDelaySP;

			if (!initiated)
			{
				// if we cross the 20% boundary we don't regenerate for 3secs
				if (gEnv->bMultiplayer && value / m_maxEnergy <= 0.2f && value < m_energy && g_pGameCVars->g_mpSpeedRechargeDelay)
					m_regenStartDelay = m_regenStartDelay20Boundary;
			}
		}

		m_energy = value;

		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}
	else if (gEnv->bClient)
	{
		NetEnergyParams params;
		params.energy = value;
		params.initiated = initiated;
		params.forced = false;

		GetGameObject()->InvokeRMI(SvRequestSetEnergy(), params, eRMI_ToServer);
	}

	return true;
}

bool CTOSEnergyConsumer::SetEnergyForced(const float value)
{
	if (gEnv->bServer)
	{
		m_energy = value;
		GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}
	else if (gEnv->bClient)
	{
		NetEnergyParams params;
		params.energy = value;
		params.initiated = false;
		params.forced = true;

		GetGameObject()->InvokeRMI(SvRequestSetEnergy(), params, eRMI_ToServer);
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
	SetEnergy(DEFAULT_ENERGY);
	SetMaxEnergy(DEFAULT_ENERGY);

	m_regenStartDelayMP = 0.0f;
	m_regenStartDelaySP = 1.0f;
}

bool CTOSEnergyConsumer::SetRegenStartDelaySP(const float val)
{
	m_regenStartDelaySP = val;
	return true;
}

bool CTOSEnergyConsumer::SetRegenStartDelayMP(const float val)
{
	m_regenStartDelayMP = val;
	return true;
}

bool CTOSEnergyConsumer::SetRegenStartDelay20Boundary(const float val)
{
	m_regenStartDelay20Boundary = val;
	return true;
}

float CTOSEnergyConsumer::GetRegenStartDelay() const
{
	return m_regenStartDelay;
}

void CTOSEnergyConsumer::SetRechargeTimeSP(const float time)
{
	m_rechargeTimeSP = time;
}

void CTOSEnergyConsumer::SetRechargeTimeMP(const float time)
{
	m_rechargeTimeMP = time;
}

bool CTOSEnergyConsumer::SetDebugEntityName(const char* name)
{
	s_debugEntityName = name;

	return true;
}

IMPLEMENT_RMI(CTOSEnergyConsumer, SvRequestSetEnergy)
{
	if (gEnv->bServer)
	{
		const float energy = params.energy;

		if (params.forced)
			SetEnergyForced(energy);
		else
			SetEnergy(energy, params.initiated);
	}

	return true;
}