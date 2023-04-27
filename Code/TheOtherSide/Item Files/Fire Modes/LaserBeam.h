#ifndef __LASERBEAM_H__
#define __LASERBEAM_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "Beam.h"

class CLaserBeam :
	public CBeam
{
public:

	CLaserBeam();
	virtual ~CLaserBeam();

	virtual void ResetParams(const struct IItemParamsNode* params);
	virtual void PatchParams(const struct IItemParamsNode* patch);

	virtual void Hit(ray_hit& hit, const Vec3& dir);
	virtual void Tick(ray_hit& hit, const Vec3& dir);
	virtual void Decal(const ray_hit& rayhit, const Vec3& dir) override;

	virtual void GetMemoryStatistics(ICrySizer* s);
};

#endif //__LASERBEAM_H__