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
	struct NetSpawnParams
	{
		int playerChannelId;
		string spawnedName;
		string className;
		Vec3 pos;
		Vec3 dir;

		NetSpawnParams()
			:
			playerChannelId(0),
			spawnedName(""),
			className(""),
			pos(ZERO),
			dir(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("playerChannelId", playerChannelId, 'i8');
			ser.Value("spawnedName", spawnedName, 'stab');
			ser.Value("className", className, 'stab');
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir0');
		}
	};	
	
	struct NetSpawnedInfo
	{
		EntityId spawnedId;
		Vec3 spawnedPos;

		NetSpawnedInfo()
			:
			spawnedId(0),
			spawnedPos(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("spawnedId", spawnedId, 'eid');
			ser.Value("spawnedPos", spawnedPos, 'wrld');
		}
	};

	struct NetMakeParams
	{
		int playerChannelId;
		bool bMake;

		NetMakeParams()
			:
			playerChannelId(0),
			bMake(false)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("playerChannelId", playerChannelId, 'i8');
			ser.Value("bMake", bMake, 'bool');
		}
	};
	const char* GetNameOfClass() { return "CTOSZeusSynchronizer"; }

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	DECLARE_SERVER_RMI_NOATTACH_FAST(SvRequestSpawnEntity, NetSpawnParams, eNRT_UnreliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH_FAST(ClSpawnEntity, NetSpawnedInfo, eNRT_UnreliableOrdered);

	DECLARE_SERVER_RMI_PREATTACH_FAST(SvRequestMakeZeus, NetMakeParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_PREATTACH_FAST(ClMakeZeus, NetMakeParams, eNRT_ReliableUnordered);

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