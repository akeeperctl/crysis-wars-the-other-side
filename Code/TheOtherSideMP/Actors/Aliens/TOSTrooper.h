#pragma once

#include "Trooper.h"



class CTOSTrooper  : public CTrooper  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSTrooper();
	~CTOSTrooper() ;

	//CTrooper
	void PostInit(IGameObject* pGameObject) ;
	void PostPhysicalize();
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;
	void ProcessMovement(float frameTime) ;
	void ProcessJumpFlyControl(const Vec3& move, float frameTime);
	void UpdateStats(float frameTime);
	//~CTrooper

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) ;
	//~ITOSMasterControllable

	bool ApplyActions(int actions); // нужна для поддержки m_actions не только в игроке

	//void ProcessJump(const CMovementRequest& request);


protected:
private:
};
