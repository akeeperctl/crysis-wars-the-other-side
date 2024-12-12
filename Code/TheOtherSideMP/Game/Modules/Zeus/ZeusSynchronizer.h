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

	struct NetTransformParams
	{
		EntityId id;
		Vec3 pos;
		Vec3 dir;

		NetTransformParams()
			:
			id(ZERO),
			pos(ZERO),
			dir(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'eid');
			ser.Value("pos", pos, 'wrld');
			ser.Value("dir", dir, 'dir0');
		}
	};	
	
	struct NetRemoveParams
	{
		EntityId id;

		NetRemoveParams()
			:
			id(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'eid');
		}
	};		
	
	struct NetKillParams
	{
		EntityId targetId;
		EntityId shooterId;

		NetKillParams()
			:
			targetId(ZERO),
			shooterId(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("targetId", targetId, 'eid');
			ser.Value("shooterId", shooterId, 'eid');
		}
	};	
	
	struct NetExecuteOrderParams
	{
		EntityId id;
		EntityId targetId;
		Vec3 pos;
		int index;
		int maxCount;
		int goalPipeId;

		NetExecuteOrderParams()
			:
			id(ZERO),
			targetId(ZERO),
			pos(ZERO),
			index(ZERO),
			maxCount(ZERO),
			goalPipeId(ZERO)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'eid');
			ser.Value("targetId", targetId, 'eid');
			ser.Value("pos", pos, 'wrld');
			ser.Value("index", index, 'i8');
			ser.Value("maxCount", maxCount, 'i8');
			ser.Value("goalPipeId", goalPipeId, 'i8');
		}
	};	
	
	struct NetHideParams
	{
		EntityId id;
		bool bHide;

		NetHideParams()
			:
			id(ZERO),
			bHide(false)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'eid');
			ser.Value("bHide", bHide, 'bool');
		}
	};	
	
	struct NetMakeHostileParams
	{
		EntityId id;
		bool bHostile;

		NetMakeHostileParams()
			:
			id(ZERO),
			bHostile(false)
		{};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("id", id, 'eid');
			ser.Value("bHostile", bHostile, 'bool');
		}
	};
	const char* GetNameOfClass() { return "CTOSZeusSynchronizer"; }

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	DECLARE_SERVER_RMI_POSTATTACH(SvRequestAIMakeHostile, NetMakeHostileParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_POSTATTACH(SvRequestExecuteOrder, NetExecuteOrderParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_POSTATTACH(SvRequestKillEntity, NetKillParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_POSTATTACH(SvRequestRemoveEntity, NetRemoveParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_POSTATTACH(SvRequestHideEntity, NetHideParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_POSTATTACH(ClHideEntity, NetHideParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_POSTATTACH_FAST(SvRequestTransformEntity, NetTransformParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_POSTATTACH_FAST(ClTransformEntity, NetTransformParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_POSTATTACH(SvRequestCopyEntity, NetSpawnParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_POSTATTACH(SvRequestSpawnEntity, NetSpawnParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_POSTATTACH(ClSpawnEntity, NetSpawnedInfo, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_PREATTACH_FAST(SvRequestMakeZeus, NetMakeParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_PREATTACH_FAST(ClMakeZeus, NetMakeParams, eNRT_ReliableUnordered);
};