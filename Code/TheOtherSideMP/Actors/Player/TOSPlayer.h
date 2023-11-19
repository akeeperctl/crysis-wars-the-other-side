#pragma once

#include "Player.h"
class CTOSEnergyConsumer;
class CTOSMasterClient;

class CTOSPlayer final : public CPlayer  // NOLINT(cppcoreguidelines-special-member-functions)
{
	friend class CPlayerMovement;
	friend class CPlayerRotation;
	friend class CPlayerInput;
	friend class CPlayerView;
	friend class CNetPlayerInput;
	friend class CTOSMasterClient;

public:
	CTOSPlayer();
	~CTOSPlayer() override;

	//CPlayer
	/**
	* Init Client
	* Вызывается на сервере при создании объекта на сервере.
	* Инициализирует текущий GameObjectExtension на клиенте по определенному каналу.
	*/
	bool Init(IGameObject* pGameObject) override;
	void InitClient(int channelId ) override;


	/**
	* Init Local Player
	* Вызывается на локальной машине игрока при подключении его к серверу
	*/
	void InitLocalPlayer() override;
	void SetSpectatorMode(uint8 mode, EntityId targetId) override;

	void PostInit( IGameObject * pGameObject ) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Release() override;
	void UpdateView(SViewParams& viewParams) override;
	void PostUpdateView(SViewParams& viewParams) override;
	//~CPlayer

	//CTOSActor
	Matrix33 GetViewMtx() override;
	Matrix33 GetBaseMtx() override;
	Matrix33 GetEyeMtx() override;
	//~CTOSActor

	CTOSMasterClient* GetMasterClient() const;
	CTOSEnergyConsumer* GetEnergyConsumer() const;


protected:
	void ClearInterference();

private:

 /**
 * Master Client
 * Является Мастер-Клиентом для каждого игрока.
 * От сервера к клиенту Мастер-Клиент будет не доступен.
 * Будет доступен только внутри клиента.
 */
	CTOSMasterClient* m_pMasterClient;
	CTOSEnergyConsumer* m_pEnergyConsumer;
};