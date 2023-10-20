#pragma once

#include <IGameObject.h>

class CTOSGenericModule;
struct NetGenericPintestParams;

// Send rmi to the server from client
// Example: INVOKETEST_RMI_TO_SERVER("[CTOSGame::OnExtraGameplayEvent]");
#define INVOKETEST_RMI_TO_SERVER(pGameObject, comment)\
	NetGenericPintestParams params;\
	params.commentary = (comment);\
	(pGameObject)->InvokeRMI(CTOSGenericSynchronizer::SvRequestPintest(), params, eRMI_ToServer)\

// Send rmi to the client from server
// Example: RMI_TO_CLIENT_PINTEST("[CTOSGame::OnExtraGameplayEvent]", pGO->GetChannelId());
#define INVOKETEST_RMI_TO_CLIENT(pGameObject, comment, clientChannelId)\
	NetGenericPintestParams params;\
	params.commentary = (comment);\
	(pGameObject)->InvokeRMI(CTOSGenericSynchronizer::ClPintest(), params, eRMI_ToClientChannel, (clientChannelId))\

// Send rmi to the clients from server
#define INVOKETEST_RMI_TO_REMOTE_CLIENTS(pGameObject, comment)\
	NetGenericPintestParams params;\
	params.commentary = (comment);\
	(pGameObject)->InvokeRMI(CTOSGenericSynchronizer::ClPintest(), params, eRMI_ToRemoteClients)\

struct NetGenericNoParams
{
	NetGenericNoParams() {};

	void SerializeWith(TSerialize ser) const {};
};

struct NetGenericSetSpeciesParams
{
	EntityId aiEntId;
	int species;
	NetGenericSetSpeciesParams()
		: aiEntId(0),
		species(0) {} ;

	explicit NetGenericSetSpeciesParams(int _species)
		: aiEntId(0),
		species(_species) { }

	void SerializeWith(TSerialize ser)
	{
		//ui5 это от 0 до 31
		ser.Value("species", species, 'ui5');
		ser.Value("aiEntId", aiEntId, 'eid');
	}
};

struct NetGenericPintestParams
{
	string commentary;
	NetGenericPintestParams() {};

	explicit NetGenericPintestParams(const char* _commentary)
		: commentary(_commentary)
	{
	}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("commentary", commentary, 'stab');
	}
};

typedef std::map<const char*, EntityId> TSynches;

/**
 * \brief TOS Generic Module сетевой синхронизатор
 * \note Обеспечивает вызов RMI'шек, необходимых для сетевой синхронизации работы модуля.
 * \note Используется как родительский синхронизатор для остальных модулей.
 * \note Сам по себе синхронизатор с этим классом не создаётся.
 */
class CTOSGenericSynchronizer : public CGameObjectExtensionHelper<CTOSGenericSynchronizer, IGameObjectExtension, 64>
{
	friend class CTOSMasterSynchronizer;
	friend class CTOSGenericModule;

public:
	CTOSGenericSynchronizer();
	virtual ~CTOSGenericSynchronizer();

	// IGameObjectExtension
	bool Init(IGameObject* pGameObject) override;
	void InitClient(int channelId) override;
	void PostInit(IGameObject* pGameObject) override;
	void PostInitClient(int channelId) override;
	void Release() override;
	void FullSerialize(TSerialize ser) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void PostSerialize() override {};
	void SerializeSpawnInfo(TSerialize ser) override {}
	ISerializableInfoPtr GetSpawnInfo() override { return 0; }
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void PostUpdate(float frameTime) override {};
	void PostRemoteSpawn() override {};
	void HandleEvent(const SGameObjectEvent&) override;
	void ProcessEvent(SEntityEvent&) override;
	void SetChannelId(uint16 id) override {};
	void SetAuthority(bool auth) override;
	void GetMemoryStatistics(ICrySizer* s) override;
	//~IGameObjectExtension

	//CTOSGenericModule* GetModule();

	// Пример: RMISend(CTOSMasterSynchronizer::SvRequestMasterAdd(), params, eRMI_ToServer);
	template <class MI, class T>
	void RMISend(const MI method, const T& params, unsigned where, int channel = -1)
	{
		auto pGP = GetGameObject();
		assert(pGP);

		if (!pGP)
			return;

		pGP->InvokeRMI(method, params, where, channel);
	}

	// Пример: RMISendWithDependentObject(CTOSMasterSynchronizer::SvRequestMasterAdd(), params, eRMI_ToServer, entityId);
	template <class MI, class T>
	void RMISendWithDependentObject(const MI method, const T& params, unsigned where, EntityId ent, int channel = -1)
	{
		auto pGP = GetGameObject();
		assert(pGP);

		if (!pGP)
			return;

		pGP->InvokeRMIWithDependentObject(method, params, where, ent, channel);
	}

	virtual const char* GetClassName()
	{
		CRY_ASSERT_MESSAGE(0, "GetClassName called from CTOSGenericSynchronizer");
		return "CTOSGenericSynchronizer";
	}

	static void GetSynchonizers(TSynches& synches);

	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета

	DECLARE_SERVER_RMI_NOATTACH(SvRequestPintest, NetGenericPintestParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClPintest, NetGenericPintestParams, eNRT_ReliableOrdered);

protected:
	//void SetModule(CTOSGenericModule* pModule);
	//CTOSGenericModule* m_pModule;
private:
	static TSynches s_synchronizers;
};