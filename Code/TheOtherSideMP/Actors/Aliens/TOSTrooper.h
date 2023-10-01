#pragma once

#include "Trooper.h"

class CTOSTrooper final : public CTrooper  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSTrooper();
	~CTOSTrooper() override;


	void PostInit(IGameObject* pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;



protected:
private:
};