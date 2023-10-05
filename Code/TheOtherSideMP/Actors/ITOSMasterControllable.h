#pragma once

struct ITOSMasterControllable
{
	virtual ~ITOSMasterControllable() = default;
	virtual void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) = 0;
};
