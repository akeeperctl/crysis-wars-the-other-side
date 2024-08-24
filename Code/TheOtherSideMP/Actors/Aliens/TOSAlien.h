#pragma once

#include "Alien.h"

class CTOSAlien: public CAlien  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterClient;

	CTOSAlien();
	~CTOSAlien() ;
	
	//CTOSActor
	void ProcessEvent(SEntityEvent& event) ;
	void PrePhysicsUpdate();
	void     SetHealth(int health) ;
	Matrix33 GetViewMtx() ;
	Matrix33 GetBaseMtx() ;
	Matrix33 GetEyeMtx() ;
	void	 Kill() ;
	void	 Revive(bool fromInit) ;
	bool	 ApplyActions(int actions);
	void SetParams(SmartScriptTable& rTable, bool resetFirst);
	//~CTOSActor

	//ITOSMasterControllable
	void ApplyMasterMovement(const Vec3& delta) ;
	//~ITOSMasterControllable

	void PostInit(IGameObject* pGameObject);
	void PostPhysicalize();
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;

protected:
private:
};