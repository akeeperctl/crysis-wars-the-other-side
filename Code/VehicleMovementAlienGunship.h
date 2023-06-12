/*************************************************************************
CryTechLab source file.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Implements a Crysis 2 Alien Gunship movement

-------------------------------------------------------------------------
History:
- 21:02:2022: Created AlienKeeper

*************************************************************************/
#include "VehicleMovementBase.h"
#include "VehicleMovementHelicopter.h"

class CVehicleMovementAlienGunship
	: public CVehicleMovementHelicopter
{
public:

	CVehicleMovementAlienGunship();
	virtual ~CVehicleMovementAlienGunship() {}

	// IVehicleMovement
	virtual bool Init(IVehicle* pVehicle, const SmartScriptTable& table);
	virtual void PostInit();
	virtual void Reset();

	virtual void OnEvent(EVehicleMovementEvent event, const SVehicleMovementEventParams& params);
	virtual void ProcessActions(const float deltaTime);
	virtual void ProcessAI(const float deltaTime);
	virtual void Update(const float deltaTime);

	void Serialize(TSerialize ser, unsigned aspects);
	// ~IVehicleMovement

	// CVehicleMovementHelicopter
	virtual void ResetActions();
	virtual bool StartEngine(EntityId driverId);
	virtual void StopEngine();
	virtual void PreProcessMovement(const float deltaTime);
	virtual void UpdateEngine(float deltaTime);
	virtual void GetMemoryStatistics(ICrySizer* s);
	// ~CVehicleMovementHelicopter

	//TheOtherSide
	//CVehicleMovementBase
	virtual void InitSoundNames(const char* eventGroupName) override;
	//~CVehicleMovementBase
	//~TheOtherSide

protected:

	//CVehicleMovementVTOL
	void SetHorizontalMode(float horizontal);
	float GetDamageMult();

	bool m_isOveridingImpulse;

	IVehicleAnimation* m_pEngineForwardAnimation;
	TVehicleAnimStateId m_wingHorizontalStateId;
	TVehicleAnimStateId m_wingVerticalStateId;

	IVehicleComponent* pWingComponentLeft;
	IVehicleComponent* pWingComponentRight;

	ICVar* m_pStabilizeVTOL;

	// settings (only changed during init)
	float m_horizontal;
	float m_maxFwdSpeedHorizMode;
	float m_maxUpSpeedHorizMode;
	float m_strafeForce;

	float m_angleLift;

	float m_relaxForce;
	float m_relaxRollTime;
	float m_relaxPitchTime;

	float m_timeUntilWingsRotate;

	float m_horizFwdForce;
	float m_horizLeftForce;
	float m_boostForce;
	float m_forwardInverseMult;

	float m_wingsSpeed;

	SPID m_fwdPID;

	// player variables

	float m_strafeAction;
	float m_relaxTimer;
	float m_soundParamTurn;
	float m_liftPitchAngle;

	// variables updated in ProcessMovement/Update
	float m_wingsTimer;
	bool m_isVTOLMovement; // used sometime by ai to force the vtol to behave like an helicopter

	float m_engineForwardAnimTime;
	float m_engineForwardAnimTimeInterp;
	//~CVehicleMovementVTOL

	IVehicleAnimation* m_pEnginesRotAnimation;
	float m_enginesRotAnimTime;
};