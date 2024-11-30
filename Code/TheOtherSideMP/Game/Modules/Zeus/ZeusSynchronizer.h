/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include <IGameObject.h>
#include "TheOtherSideMP/Game/Modules/GenericSynchronizer.h"

/**
 * \brief TOS Zeus Module сетевой синхронизатор
 * \note Обеспечивает вызов RMI'шек, необходимых для сетевой синхронизации работы модуля
 */
class CTOSZeusSynchronizer : public CTOSGenericSynchronizer  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	struct NetMakeParams
	{
		EntityId playerId;
		bool bMake;

		NetMakeParams()
			:
			playerId(0),
			bMake(false)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("playerId", playerId, 'eid');
			ser.Value("bMake", bMake, 'bool');
		}
	};
	const char* GetNameOfClass() { return "CTOSZeusSynchronizer"; }

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	DECLARE_SERVER_RMI_PREATTACH(SvRequestMakeZeus, NetMakeParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_PREATTACH(ClMakeZeus, NetMakeParams, eNRT_ReliableUnordered);

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterRemove, NetMasterAddingParams, eNRT_ReliableOrdered);
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestSetDesiredSlaveCls, NetDesiredSlaveClsParams, eNRT_ReliableOrdered);

	//DECLARE_CLIENT_RMI_NOATTACH(ClMasterClientStartControl, NetMasterStartControlParams, eNRT_ReliableOrdered);
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterClientStartControl, NetMasterStartControlParams, eNRT_ReliableOrdered);

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestDelegateAuthority, NetDelegateAuthorityParams, eNRT_ReliableOrdered);

	//DECLARE_CLIENT_RMI_NOATTACH(ClMasterClientStopControl, NetGenericNoParams, eNRT_ReliableOrdered);
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterClientStopControl, NetMasterStopControlParams, eNRT_ReliableOrdered);

	////TODO: 10/11/2023, 09:25 Создать модуль для ИИ, когда наберется достаточно функций.
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestSaveMCParams, NetMasterIdParams, eNRT_ReliableOrdered);
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestApplyMCSavedParams, NetMasterIdParams, eNRT_ReliableOrdered);


};