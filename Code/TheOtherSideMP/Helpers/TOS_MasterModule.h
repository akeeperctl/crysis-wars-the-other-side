#pragma once

#include "Player.h"
#include "TheOtherSideMP/Actors/Player/TOSPlayer.h"
#include "TheOtherSideMP/Game/Modules/Master/MasterClient.h"

namespace TOS_MasterModule
{
	static bool ClientPlayerHaveSlave(CPlayer* pPlayer)
	{
		bool haveSlave = false;

		const auto pTOSPlayer = dynamic_cast<CTOSPlayer*>(pPlayer);
		assert(pTOSPlayer);

		if (pTOSPlayer)
		{
			const auto pMC = pTOSPlayer->GetMasterClient();
			assert(pMC);

			if (pMC && pMC->GetSlaveEntity())
				haveSlave = true;
		}

		return haveSlave;
	}
}