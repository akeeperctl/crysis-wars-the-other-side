// ReSharper disable CppInconsistentNaming
#pragma once

namespace TOS_NET
{
	static constexpr EEntityAspects CLIENT_ASPECT_STATIC = eEA_GameClientStatic;
	static constexpr EEntityAspects CLIENT_ASPECT_DYNAMIC = eEA_GameClientDynamic;

	static constexpr EEntityAspects SERVER_ASPECT_DYNAMIC = eEA_GameServerDynamic; // AI
	static constexpr EEntityAspects SERVER_ASPECT_STATIC = eEA_GameServerStatic;

	/**
	 * \brief Проверка принадлежат ли права pGO на изменение сущности с id checkedId
	 * \n Только на сервере!!!
	 * \param pGO - указатель на игровой объект (игрок или что-то подобное)
	 * \param checkedId - ид проверяемой сущности (транспорт, раб или т.п.)
	 * \return true, если права на изменение есть
	 */
	inline bool IsHaveAuthority(const IGameObject* pGO, const EntityId checkedId)
	{
		CRY_ASSERT_MESSAGE(gEnv->bServer, "You cannot check permissions to change an entity on the client side");
		if (!gEnv->bServer)
			return false;

		const auto pNetContext = g_pGame->GetIGameFramework()->GetNetContext();
		assert(pNetContext);
		if (!pNetContext)
			return false;

		assert(pGO);
		if (!pGO)
			return false;

		const auto pCheckedEntity = gEnv->pEntitySystem->GetEntity(checkedId);
		assert(pCheckedEntity);
		if (!pCheckedEntity)
			return false;

		const int gOChannelId = pGO->GetChannelId();
		const auto pGONetChannel = g_pGame->GetIGameFramework()->GetNetChannel(gOChannelId);

		assert(pGONetChannel);
		if (!pGONetChannel)
			return false;

		return 	pNetContext->RemoteContextHasAuthority(pGONetChannel, gOChannelId);
	}

	inline bool DelegateAuthority(const IGameObject* pGO, const EntityId slaveId)
	{
		CRY_ASSERT_MESSAGE(gEnv->bServer, "You cannot delegate permissions to change an entity on the client side");
		if (!gEnv->bServer)
			return false;

		const auto pNetContext = g_pGame->GetIGameFramework()->GetNetContext();
		assert(pNetContext);
		if (!pNetContext)
			return false;

		assert(pGO);
		if (!pGO)
			return false;

		const auto pCheckedEntity = gEnv->pEntitySystem->GetEntity(slaveId);
		assert(pCheckedEntity);
		if (!pCheckedEntity)
			return false;

		const int gOChannelId = pGO->GetChannelId();
		const auto pGONetChannel = g_pGame->GetIGameFramework()->GetNetChannel(gOChannelId);

		assert(pGONetChannel);
		if (!pGONetChannel)
			return false;

		const auto isAuth = pNetContext->RemoteContextHasAuthority(pGONetChannel, slaveId);
		if (!isAuth)
			pNetContext->DelegateAuthority(slaveId, pGONetChannel);

		return true;
	}
}