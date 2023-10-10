#ifndef _AI_AUTOBALANCE_
#define _AI_AUTOBALANCE_

#include <iaisystem.h>
#include <vector>

typedef std::vector<float> VectorOfFloats;

class CAIAutoBalance : public IAutoBalance
{
	int m_nAllowedDeaths;
	int m_nPlayerDeaths;

	float m_fLastPlayerDeathTime;
	float m_fTotalTimeStart;
	
	AIBalanceStats m_Stats;

	VectorOfFloats m_vEnemyLifetimes;
	float m_fAvgEnemyLifetime;

	float m_fAccuracyMult;
	float m_fAggressionMult;
	float m_fHealthMult;

	float m_fStartingAccuracy;
	float m_fStartingAggresion;
	float m_fStartingHealth;

	float m_fHitPercentage;

	float m_fMaxClampValue;

	int	m_nNumShotsFired;
	int	m_nNumShotsHit;

public:
	CAIAutoBalance(void);
	~CAIAutoBalance(void) override;

	void RegisterPlayerDeath() override;
	void RegisterEnemyLifetime(float fLifeInSeconds) override;
	void SetAllowedDeathCount(int nDeaths) override;
	void Checkpoint() override;
	void DebugDraw(IRenderer * pRenderer);
	void SetMultipliers(float fAccuracy, float fAggression, float fHealth) override;
	void GetMultipliers(float & fAccuracy, float & fAggression, float & fHealth) override;
	void AdjustDifficulty(bool bCalcDeath=true);
	void Clamp(float & fVal);
	void RegisterPlayerFire(int nShots) override;
	void RegisterPlayerHit() override;
	void CalcMinimum(void);
	void GetAutobalanceStats(AIBalanceStats & stats) override;
	void RegisterVehicleDestroyed(void) override;
};

#endif //#ifndef _AI_AUTOBALANCE_