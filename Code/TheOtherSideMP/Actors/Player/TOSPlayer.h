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
	* ���������� �� ������� ��� �������� ������� �� �������.
	* �������������� ������� GameObjectExtension �� ������� �� ������������� ������.
	*/
	virtual void InitClient(int channelId );


	/**
	* Init Local Player
	* ���������� �� ��������� ������ ������ ��� ����������� ��� � �������
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
 * �������� ������-�������� ��� ������� ������.
 * �� ������� � ������� ������-������ ����� �� ��������.
 * ����� �������� ������ ������ �������.
 */
	CTOSMasterClient* m_pMasterClient;
};