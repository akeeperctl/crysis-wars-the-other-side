/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Bullet

-------------------------------------------------------------------------
History:
- 12:10:2005   11:15 : Created by Márcio Martins

*************************************************************************/

# pragma once

#include "Projectile.h"

class CKvoltBullet : public CProjectile
{
public:
	CKvoltBullet();
	virtual ~CKvoltBullet();

	// CProjectile
	virtual void HandleEvent(const SGameObjectEvent&);
	// ~CProjectile

	//For underwater trails (Called only from WeaponSystem.cpp)
	static void SetWaterMaterialId();
	static int  GetWaterMaterialId() { return m_waterMaterialId; }

	static IEntityClass* EntityClass;

private:

	static int  m_waterMaterialId;
};