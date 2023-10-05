#pragma once

#include "Trooper.h"

#include "../ITOSMasterControllable.h"

class CTOSTrooper final : public CTrooper  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSTrooper();
	~CTOSTrooper() override;

	//CTrooper
	void PostInit(IGameObject* pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	//~CTrooper

	//ITOSMasterControllable
	void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) override;
	//~ITOSMasterControllable


protected:
private:
};
