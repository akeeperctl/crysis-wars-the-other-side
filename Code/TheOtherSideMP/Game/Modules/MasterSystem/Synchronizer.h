#pragma once

#include <IGameObject.h>
#include "TheOtherSideMP/Game/Modules/MasterSystem/MasterModule.h"

struct PintestParams;

// Send pintest rmi to the server
//Example: RMISENDER_SERVER_PINTEST("[CTOSGame::OnExtraGameplayEvent]");
#define SYNCHRONIZER_SERVER_PINTEST(comment)\
	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();\
	assert(pSender);\
	PintestParams params;\
	params.commentary = comment;\
	pSender->RMISend(CTOSMasterSynchronizer::SvRequestPintest(), params, eRMI_ToServer)\

// Send pintest rmi to the client
//Example: RMISENDER_CLIENT_PINTEST("[CTOSGame::OnExtraGameplayEvent]", pGO->GetChannelId());
#define SYNCHRONIZER_CLIENT_PINTEST(comment, clientChannelId)\
	auto pSender = g_pTOSGame->GetMasterModule()->GetRMISender();\
	assert(pSender);\
	PintestParams params;\
	params.commentary = comment;\
	pSender->RMISend(CTOSMasterSynchronizer::ClPintest(), params, eRMI_ToClientChannel, clientChannelId)\

struct NoParams
{
	NoParams() {};

	void SerializeWith(TSerialize ser) {};
};

struct PintestParams
{
	string commentary;
	PintestParams() {};
	PintestParams(const char* _commentary)
		: commentary(_commentary)
	{
	}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("commentary", commentary, 'stab');
	}
};

struct MasterAddingParams
{
	EntityId entityId;
	MasterAddingParams() {};
	MasterAddingParams(EntityId entId)
		: entityId(entId)
	{
	}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("entityId", entityId, 'eid');
	}
};

class CTOSMasterSynchronizer : public CGameObjectExtensionHelper<CTOSMasterSynchronizer, IGameObjectExtension, 64>
{
public:
	CTOSMasterSynchronizer();
	~CTOSMasterSynchronizer();

	// IGameObjectExtension
	virtual bool Init(IGameObject* pGameObject);
	virtual void InitClient(int channelId);
	virtual void PostInit(IGameObject* pGameObject);
	virtual void PostInitClient(int channelId);
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual void PostUpdate(float frameTime) {};
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent(const SGameObjectEvent&);
	virtual void ProcessEvent(SEntityEvent&);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryStatistics(ICrySizer* s);
	//~IGameObjectExtension

	template <class MI, class T>
	void RMISend(const MI method, const T& params, unsigned where, int channel = -1)
	{
		auto pGP = GetGameObject();
		assert(pGP);

		if (!pGP)
			return;

		pGP->InvokeRMI(method, params, where, channel);
	}

	template <class MI, class T>
	void RMISendWithDependentObject(const MI method, const T& params, unsigned where, EntityId ent, int channel = -1)
	{
		auto pGP = GetGameObject();
		assert(pGP);

		if (!pGP)
			return;

		pGP->InvokeRMI_Primitive(method, params, where, channel, ent);
	}


	//CLIENT - Направленные на клиент
	//SERVER - Направленные на сервер с клиента
	//NOATTACH - Без привязки к данным сериализации
	//Reliable - надёжная доставка пакета
	
	DECLARE_SERVER_RMI_NOATTACH(SvRequestPintest, PintestParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClPintest, PintestParams, eNRT_ReliableOrdered);

	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterAdd, MasterAddingParams, eNRT_ReliableOrdered);
	//DECLARE_SERVER_RMI_NOATTACH(SvRequestMasterRemove, MasterAddingParams, eNRT_ReliableOrdered);

protected:

private:
};