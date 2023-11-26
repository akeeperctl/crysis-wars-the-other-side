#pragma once

#include "Trooper.h"



class CTOSTrooper final : public CTrooper  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSTrooper();
	~CTOSTrooper() override;

	//CTrooper
	void PostInit(IGameObject* pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void ProcessMovement(float frameTime) override;
	void ProcessJumpFlyControl(const Vec3& move, float frameTime);
	//~CTrooper

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) override;
	//~ITOSMasterControllable

	void ProcessJump(const CMovementRequest& request);

protected:
private:
};
