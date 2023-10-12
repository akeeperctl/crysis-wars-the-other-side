#pragma once

#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IVehicleSystem.h>

#include <TheOtherSideMP/Actors/player/TOSPlayer.h>
#include <TheOtherSideMP/Game/Modules/ITOSGameModule.h>

#include "MasterModule.h"

class CTOSHUDCrosshair;
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
 * \n В частности перемещение, управление, угол обзора и т.д.
 * \n Dude - это локальный персонаж.
 * \n Автоудаление класса: отсутствует.
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
		return m_pSlaveEntity;
	};

	/**
	 * \brief Изменяет указатель на контролируемую сущность раба
	 * \param pEntity - указатель на новую сущность. Это должен быть актёр, наследованный от IActor! Не передавать сюда пустой указатель!
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
	/**
	 * \brief Подготовить локального персонажа перед началом/прекращением управления рабом
	 * \param toStartControl - Если true, то подготовка персонажа будет проходить как подготовка перед началом управления рабом
	 */
	void PrepareDude(bool toStartControl) const;

	CTOSPlayer* m_pLocalDude; ///< Указатель на локального персонажа с именем \a Dude. \n Появляется в одиночной игре.
	IEntity* m_pSlaveEntity; ///< Указатель на сущность раба, которую контролирует локальный персонаж.
	//MCSaved m_dudeSavedParams; ///< Хранит сохраненные параметры лок. персонажа \n для их применения перед/после начала управления рабом
	CTOSHUDCrosshair* m_pHUDCrosshair;

public:
	//static constexpr int INPUT_ASPECT = eEA_GameClientDynamic;
	//static constexpr int ALIVE_ASPECT = eEA_GameServerDynamic;
	//static constexpr int OWNER_ASPECT = eEA_GameServerStatic;
};