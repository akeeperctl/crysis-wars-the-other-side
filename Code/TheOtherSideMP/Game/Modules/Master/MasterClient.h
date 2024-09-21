/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

// ReSharper disable CppInconsistentNaming
// ReSharper disable CppPolymorphicClassWithNonVirtualPublicDestructor
#pragma once

#include <IGameFramework.h>
#include <IGameRulesSystem.h>
#include <IVehicleSystem.h>

#include <TheOtherSideMP/Actors/player/TOSPlayer.h>
#include <TheOtherSideMP/Game/Modules/ITOSGameModule.h>

#include "MasterModule.h"
#include <PlayerInput.h>

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

enum EFactionPriority
{
	eFP_Master, // команда и фракция берутся с мастера и применяются на раба
	eFP_Slave, // команда и фракция берутся с раба и применяются на мастера
	eFP_Last
};

 /**
 * TOS MasterClient
 * Класс описывает то, как будет происходить взаимодействие между Мастером и Рабом.
 * \n В частности перемещение, управление, угол обзора, работа с худом и т.д
 * \n Dude - это локальный персонаж.
 * \n Автоудаление класса: отсутствует.
 */
class CTOSMasterClient  : public IActionListener
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

	enum ETOSDudePrepareFlags
	{
		//TOS_DUDE_FLAG_HIDE_MODEL           = BIT(0),
		TOS_DUDE_FLAG_BEAM_MODEL = BIT(1),
		//TOS_DUDE_FLAG_CLEAR_INVENTORY      = BIT(2),
		TOS_DUDE_FLAG_DISABLE_SUIT = BIT(2),
		TOS_DUDE_FLAG_ENABLE_ACTION_FILTER = BIT(3),
		TOS_DUDE_FLAG_SAVELOAD_PARAMS = BIT(4),
	};

	enum ETOSActionFlags
	{
		TOS_PRESSED = BIT(0),
		TOS_HOLD = BIT(1),
	};

	struct SCrosshairInfo
	{
		SCrosshairInfo()
			: lastTargetId(0),
			targetId(0),
			worldPos(ZERO),
			rayHit()
		{}

		EntityId lastTargetId;
		EntityId targetId;
		Vec3 worldPos;
		ray_hit rayHit;
	};

	struct SCameraInfo
	{
		SCameraInfo() :
			worldPos(ZERO),
			viewDir(ZERO)
		{}

		Vec3 worldPos;///< мировая позиция камеры
		Vec3 viewDir;///< направление, куда смотрит камера
		Vec3 lookPointPos;///< точка, куда смотрит камера (вычисляется так: pointPos = worldPos + viewDir)
	};

	struct SMeleeInfo
	{
		SMeleeInfo()
			: targetId(0),
			targetPos(ZERO),
			rayHit(),
			hitCount(0) {}

		EntityId targetId;
		Vec3 targetPos;
		ray_hit rayHit;
		int hitCount;
	};

	struct SLookFireInfo
	{
		SLookFireInfo()
			: fireTargetPos(ZERO),
			fireTargetId(ZERO),
			lookTargetPos(ZERO),
			rayHit() {}

		Vec3 fireTargetPos;
		EntityId fireTargetId;
		Vec3 lookTargetPos;
		ray_hit rayHit;
	};


	// IActionListener интерфейс используется только для декларации функции.
	// Этот класс не слушает actions сам по себе, а работает через функцию в PlayerInput 
	void OnAction(const ActionId& action, int activationMode, float value) ;
	// ~IActionListener

private:
	bool OnActionAttack1(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionAttack2(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionSpecial(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionMoveForward(CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionMoveBack(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionMoveLeft(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionMoveRight(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionJump(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionHoldTest(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);
	bool OnActionSprint(CTOSActor* pActor,const ActionId& actionId, int activationMode, float value, float pressedDur = 0);

public:

	void OnEntityEvent(IEntity* pEntity, const SEntityEvent& event);

	void StartControl(IEntity* pEntity, uint dudeFlags = 0, bool fromFG = false, EFactionPriority priority = eFP_Master);
	void StopControl(bool callFromFG = false);

	/**
	 * \brief Получить указатель на актёра раба
	 * \return Указатель на управляемого актёра раба
	 */
	CTOSActor* GetSlaveActor() const
	{
		if (!m_pSlaveEntity)
			return nullptr;

		const auto pActor = dynamic_cast<CTOSActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_pSlaveEntity->GetId()));
		assert(pActor);

		return pActor;
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
	 * \brief Получить EntityId сущности раба
	 * \return EntityId управляемой сущности раба
	 */
	EntityId GetSlaveEntityId() const
	{
		//assert(m_pControlledEntity);
		return m_pSlaveEntity ? m_pSlaveEntity->GetId() : 0;
	};

	void UpdateView(SViewParams& viewParams) const;
	void Update(float frametime);
	void AnimationEvent(IEntity* pEntity, ICharacterInstance* pCharacter, const AnimEventInstance& event);


private:
	/**
	 * \brief Изменяет указатель на контролируемую сущность раба
	 * \param pEntity - указатель на новую сущность. Это должен быть актёр, наследованный от IActor! Не передавать сюда пустой указатель!
	 * \param cls - класс новой сущности
	 * \return True, если сущность успешно изменена
	 */
	bool SetSlaveEntity(IEntity* pEntity, const char* cls);

	void PrePhysicsUpdate();

	/**
	 * \brief Зануляет указатель на сущность раба, который сохранен в мастер-клиенте
	 */
	void ClearSlaveEntity();

	void OnActionDelayReleased(const ActionId action, float pressedTimeLen);

	void SendMovementRequest(IMovementController* pController, CMovementRequest& request);

	//void ProcessMeleeDamage() const;
	void UpdateMeleeTarget(const IEntity* pSlaveEntity, const int rayFlags, const unsigned entityFlags, const SMovementState& state);
	void UpdateCrosshair(const IEntity* pSlaveEntity, const IActor* pLocalDudeActor, int rayFlags, unsigned entityFlags);
	void UpdateLookFire(const IEntity* pSlaveEntity, const int rayFlags, const unsigned entityFlags, const SMovementState& state);

	CWeapon* GetCurrentWeapon(const IActor* pActor) const;
	bool     IsFriendlyEntity(IEntity* pEntity, IEntity* pTarget) const;

	/**
	 * \brief Подготовить локального персонажа перед началом/прекращением управления рабом
	 * \param toStartControl - Если true, то подготовка персонажа будет проходить как подготовка перед началом управления рабом
	 * \param dudeFlags - Битовые флаги, применяемые к персонажу локального игрока (Dude)
	 */
	void PrepareDude(bool toStartControl, uint dudeFlags) const;


	bool PrepareNextSlave(CTOSActor* pNextActor) const;
	bool PreparePrevSlave(CTOSActor* pPrevActor) const;


	CTOSPlayer* m_pLocalDude; ///< Указатель на локального персонажа с именем \a Dude. \n Появляется в одиночной игре. \n Задаётся при вызове InitLocalPlayer у CTOSPlayer
	IEntity*    m_pSlaveEntity; ///< Указатель на сущность раба, которую контролирует локальный персонаж.

	uint m_actions;
	uint m_dudeFlags;
	CMovementRequest m_movementRequest;

	Vec3 m_deltaMovement;///< направление движения. От -1 до 1. Не сбрасывается до 0 когда действие не выполняется.
	//CPlayerInput* m_pPlayerGruntInput;

	//CCamera* m_pWorldCamera;
	SMeleeInfo m_meleeInfo;
	SCameraInfo m_cameraInfo;
	SCrosshairInfo m_crosshairInfo;
	SLookFireInfo m_lookfireInfo;
	
	std::map<ActionId, float> m_actionPressedDuration;
	std::map<ActionId, uint> m_actionFlags;
};