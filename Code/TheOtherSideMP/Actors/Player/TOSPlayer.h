#pragma once

#include "Player.h"
class CTOSMasterClient;

class CTOSPlayer : public CPlayer
{
	friend class CPlayerMovement;
	friend class CPlayerRotation;
	friend class CPlayerInput;
	friend class CPlayerView;
	friend class CNetPlayerInput;

public:
	CTOSPlayer();
	~CTOSPlayer();


public:


	/**
	* Init Client
	* Вызывается на сервере при создании объекта на сервере.
	* Инициализирует текущий GameObjectExtension на клиенте по определенному каналу.
	*/
	virtual void InitClient(int channelId );


	/**
	* Init Local Player
	* Вызывается на локальной машине игрока при подключении его к серверу
	*/
	virtual void InitLocalPlayer();


	virtual void PostInit( IGameObject * pGameObject );
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void Release();

	CTOSMasterClient* GetMasterClient();

protected:
private:

 /**
 * Master Client
 * Является Мастер-Клиентом для каждого игрока.
 * От сервера к клиенту Мастер-Клиент будет не доступен.
 * Будет доступен только внутри клиента.
 */
	CTOSMasterClient* m_pMasterClient;
};