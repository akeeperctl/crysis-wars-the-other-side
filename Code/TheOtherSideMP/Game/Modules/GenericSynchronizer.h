#pragma once

#include <IGameObject.h>

class CTOSGenericModule;
struct PintestParams;

// Send pintest rmi to the server
//Example: RMISENDER_SERVER_PINTEST("[CTOSGame::OnExtraGameplayEvent]");
#define SYNCHRONIZER_SERVER_PINTEST(comment)\
	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();\
	assert(pSender);\
	PintestParams params;\
	params.commentary = comment;\
	pSender->RMISend(CTOSModuleSynchronizer::SvRequestPintest(), params, eRMI_ToServer)\

// Send pintest rmi to the client
//Example: RMISENDER_CLIENT_PINTEST("[CTOSGame::OnExtraGameplayEvent]", pGO->GetChannelId());
#define SYNCHRONIZER_CLIENT_PINTEST(comment, clientChannelId)\
	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();\
	assert(pSender);\
	PintestParams params;\
	params.commentary = comment;\
	pSender->RMISend(CTOSModuleSynchronizer::ClPintest(), params, eRMI_ToClientChannel, clientChannelId)\

struct NoParams
{
	NoParams() {};

	void SerializeWith(TSerialize ser) const {};
};

struct PintestParams
{
	string commentary;
	PintestParams() {};

	explicit PintestParams(const char* _commentary)
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

	DECLARE_SERVER_RMI_NOATTACH(SvRequestPintest, PintestParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClPintest, PintestParams, eNRT_ReliableOrdered);

protected:
	//void SetModule(CTOSGenericModule* pModule);
	//CTOSGenericModule* m_pModule;
private:
	static TSynches s_synchronizers;
};