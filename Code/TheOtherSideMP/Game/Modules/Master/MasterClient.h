// ReSharper disable CppInconsistentNaming
// ReSharper disable CppPolymorphicClassWithNonVirtualPublicDestructor
#pragma once

#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IHardwareMouse.h>
#include <ILevelSystem.h>
#include <IVehicleSystem.h>

#include <TheOtherSideMP/Actors/player/TOSPlayer.h>
#include <TheOtherSideMP/Game/Modules/ITOSGameModule.h>

#include "MasterModule.h"

enum ETOSDudePrepareFlags
{
	TOS_DUDE_FLAG_HIDE_MODEL           = BIT(0),
	TOS_DUDE_FLAG_BEAM_MODEL           = BIT(1),
	//TOS_DUDE_FLAG_CLEAR_INVENTORY      = BIT(2),
	TOS_DUDE_FLAG_DISABLE_SUIT         = BIT(2),
	TOS_DUDE_FLAG_ENABLE_ACTION_FILTER = BIT(3)
};

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
class CTOSMasterClient final : public IActionListener
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

	// IActionListener интерфейс используется только для декларации функции.
	// Этот класс не слушает actions сам по себе, а работает через функцию в PlayerInput 
	void OnAction(const ActionId& action, int activationMode, float value) override;
	// ~IActionListener

	bool OnActionMoveForward(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveBack(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveLeft(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveRight(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value);
	bool OnActionJump(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value);

	void OnEntityEvent(IEntity* pEntity, const SEntityEvent& event);

	void StartControl(IEntity* pEntity, uint dudeFlags = 0);
	void StopControl();

	/**
	 * \brief Получить указатель на актёра раба
	 * \return Указатель на управляемого актёра раба
	 */
	CTOSActor* GetSlaveActor() const
	{
		if (!m_pSlaveEntity)
			return nullptr;

		return dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_pSlaveEntity->GetId()));
	};

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
	void PrePhysicsUpdate();
	void Update(float frametime);
	//void Update(IEntity* pEntity);

private:
	/**
	 * \brief Подготовить локального персонажа перед началом/прекращением управления рабом
	 * \param toStartControl - Если true, то подготовка персонажа будет проходить как подготовка перед началом управления рабом
	 * \param dudeFlags - Битовые флаги, применяемые к персонажу локального игрока (Dude)
	 */
	void PrepareDude(bool toStartControl, uint dudeFlags) const;

public:

private:
	CTOSPlayer* m_pLocalDude; ///< Указатель на локального персонажа с именем \a Dude. \n Появляется в одиночной игре.
	IEntity*    m_pSlaveEntity; ///< Указатель на сущность раба, которую контролирует локальный персонаж.
	//MCSaved m_dudeSavedParams; ///< Хранит сохраненные параметры лок. персонажа \n для их применения перед/после начала управления рабом
	CTOSHUDCrosshair* m_pHUDCrosshair;

	uint m_dudeFlags;
	CMovementRequest m_movementRequest;

	Vec3 m_movementDir;///< направление движения. От -1 до 1. Не сбрасывается до 0 когда действие не выполняется.

	CCamera* m_pWorldCamera;

public:
	//static constexpr int INPUT_ASPECT = eEA_GameClientDynamic;
	//static constexpr int ALIVE_ASPECT = eEA_GameServerDynamic;
	//static constexpr int OWNER_ASPECT = eEA_GameServerStatic;
};