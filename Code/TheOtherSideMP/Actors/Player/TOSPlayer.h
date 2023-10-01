#pragma once

#include "Player.h"
class CTOSMasterClient;

class CTOSPlayer final : public CPlayer  // NOLINT(cppcoreguidelines-special-member-functions)
{
	friend class CPlayerMovement;
	friend class CPlayerRotation;
	friend class CPlayerInput;
	friend class CPlayerView;
	friend class CNetPlayerInput;

public:
	CTOSPlayer();
	~CTOSPlayer() override;


public:


	/**
	* Init Client
	* ���������� �� ������� ��� �������� ������� �� �������.
	* �������������� ������� GameObjectExtension �� ������� �� ������������� ������.
	*/
	void InitClient(int channelId ) override;


	/**
	* Init Local Player
	* ���������� �� ��������� ������ ������ ��� ����������� ��� � �������
	*/
	void InitLocalPlayer() override;


	void PostInit( IGameObject * pGameObject ) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	void Release() override;

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