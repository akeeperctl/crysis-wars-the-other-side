#pragma once

#include "Alien.h"

class CTOSAlien: public CAlien  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterClient;

	CTOSAlien();
	~CTOSAlien() override;

	static constexpr int SERVER_ASPECT_HEALTH = eEA_GameServerStatic;
	static constexpr int CLIENT_ASPECT_INPUT = eEA_GameClientDynamic;
;
	//CTOSActor
	void ProcessEvent(SEntityEvent& event) override;

	void     SetHealth(int health) override;
	Matrix33 GetViewMtx() override;
	Matrix33 GetBaseMtx() override;
	Matrix33 GetEyeMtx() override;
	//~CTOSActor

	//ITOSMasterControllable
	void ApplyMasterMovement(const Vec3& delta) override;
	//~ITOSMasterControllable

	void PostInit(IGameObject* pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;

protected:
private:
};