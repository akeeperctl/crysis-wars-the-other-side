#pragma once

#include "Alien.h"

class CTOSAlien: public CAlien  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSAlien();
	~CTOSAlien() override;

	static constexpr int ASPECT_HEALTH = eEA_GameServerStatic;

	void PostInit(IGameObject* pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;

	//CTOSActor
	void     SetHealth(int health) override;
	Matrix33 GetViewMtx() override;
	Matrix33 GetBaseMtx() override;
	Matrix33 GetEyeMtx() override;
	//~CTOSActor


protected:
private:
};