#pragma once

#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IVehicleSystem.h>

#include <TheOtherSideMP/Actors/player/TOSPlayer.h>
#include <TheOtherSideMP/Game/Modules/ITOSGameModule.h>


class CTOSAIActionTracker;
class CTOSMasterClient;
class CSquadSystem;
class CGameFlashAnimation;
class CConquerorSystem;
class CTOSAbilitiesSystem;
class CVehicleMovementBase;

struct IHardwareMouseEventListener;
struct IHitListener;


 /**
 * TOS MasterClient
 * Класс описывает то, как будет происходить взаимодействие между Мастером и Рабом.
 * В частности перемещение, управление, угол обзора и т.д
 * Автоудаление: отсутствует.
 */
class CTOSMasterClient final  // NOLINT(cppcoreguidelines-special-member-functions)
{
	//friend class CGameRules;
	//friend class CControlSystem;
	//friend class CNetControlClient;
	//friend class CSquadSystem;
	//friend class CConquerorSystem;
	//friend class CPlayerInput;
	//friend class CTOSAbilitiesSystem;
	//friend class CAbilityOwner;
	//friend class CSquad;
	//friend class CMember;
public:
	explicit CTOSMasterClient(CTOSPlayer* pPlayer);
	~CTOSMasterClient();

	void StartControl(IEntity* pEntity);
	void StopControl();

	/**
	 * \brief Получить указатель на сущность раба
	 * \return Указатель на управляемую сущность раба
	 */
	IEntity* GetSlaveEntity() const
	{
		//assert(m_pControlledEntity);
		return m_pControlledEntity;
	};

	/**
	 * \brief Изменяет указатель на контролируемую сущность раба
	 * \param pEntity - указатель на новую сущность. Не передавать сюда пустой указатель!
	 * \param cls - класс новой сущности
	 * \return True, если сущность успешно изменена
	 */
	bool SetSlaveEntity(IEntity* pEntity, const char* cls);

	/**
	 * \brief Зануляет указатель на сущность раба, который сохранен в мастер-клиенте
	 */
	void ClearSlaveEntity();

	void UpdateView(SViewParams& viewParams) const;

private:
	CTOSPlayer* m_pLocalDude;
	IEntity* m_pControlledEntity;

public:
	//static constexpr int INPUT_ASPECT = eEA_GameClientDynamic;
	//static constexpr int ALIVE_ASPECT = eEA_GameServerDynamic;
	//static constexpr int OWNER_ASPECT = eEA_GameServerStatic;
};