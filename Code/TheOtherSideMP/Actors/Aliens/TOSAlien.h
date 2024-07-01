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
	void PrePhysicsUpdate() ;


	void     SetHealth(int health) ;
	Matrix33 GetViewMtx() ;
	Matrix33 GetBaseMtx() ;
	Matrix33 GetEyeMtx() ;
	void	 Kill() ;
	bool	 ApplyActions(int actions);
	//~CTOSActor

	//ITOSMasterControllable
	void ApplyMasterMovement(const Vec3& delta) ;
	//~ITOSMasterControllable

	void PostInit(IGameObject* pGameObject) ;
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;

protected:
private:
};