/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once



struct ITOSMasterControllable
{
	virtual ~ITOSMasterControllable() {};
	virtual void UpdateMasterView(SViewParams& viewParams, Vec3& offsetX, Vec3& offsetY, Vec3& offsetZ, Vec3& target, Vec3& current, float& currentFov) = 0;
	virtual void ApplyMasterMovement(const Vec3& delta) = 0;
};
