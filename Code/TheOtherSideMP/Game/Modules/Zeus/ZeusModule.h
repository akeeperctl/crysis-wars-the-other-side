#pragma once

#include "TheOtherSideMP\Game\Modules\GenericModule.h"
#include <TheOtherSideMP\Actors\Player\TOSPlayer.h>

enum EZeusFlags
{
	eZF_CanRotateCamera = BIT(0),
	eZF_Possessing = BIT(1) // зевс вселился в кого-то
};

enum EZeusCommands
{
	eZC_KillSelected,
	eZC_RemoveSelected,
};

/**
 * \brief Модуль для обработки ситуаций режима игры Zeus
 */
class CTOSZeusModule : public CTOSGenericModule, IHardwareMouseEventListener
{
public:
	CTOSZeusModule();
	virtual ~CTOSZeusModule();

	//ITOSGameModule
	bool        OnInputEvent(const SInputEvent& event);
	bool        OnInputEventUI(const SInputEvent& event)
	{
		return false;
	};
	void        OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	void        GetMemoryStatistics(ICrySizer* s);
	const char* GetName()
	{
		return "CTOSGenericModule";
	};
	void        Init();
	void        Update(float frametime);
	void        Serialize(TSerialize ser);
	int GetDebugLog()
	{
		return m_debugLogMode;
	}
	void InitCVars(IConsole* pConsole);
	void InitCCommands(IConsole* pConsole);
	void ReleaseCVars();
	void ReleaseCCommands();
	//~ITOSGameModule

	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener

	void NetMakePlayerZeus(IActor* pPlayer);
	void ShowHUD(bool show);

	void SetZeusFlag(uint flag, bool value);
	bool GetZeusFlag(uint flag) const;

	void OnAction(const ActionId& action, int activationMode, float value);
protected:
	bool OnActionAttack2(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);

private:
	/// @brief Проекция координат мыши в мир от камеры
	/// @param ray - структура луча
	/// @param mouseWorldPos - мировые координаты мыши, которые будут спроецированы на некоторое расстояние от камеры
	/// @return 0 - попаданий не было, > 0 - есть попадания
	int	MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags);

	/// Можно ли выбрать сущности выделением нескольких сразу?
	bool CanSelectMultiplyWithBox() const;

	void GetSelectedEntities();
	EntityId GetMouseEntityId();
	void ShowMouse(bool show);
	void ApplyZeusProperties(IActor* pPlayer);

	//Выполнить команду всем выделенным сущностям
	bool ExecuteCommand(EZeusCommands command);

	CTOSPlayer* m_zeus;
	uint m_zeusFlags;
	ray_hit m_mouseRay;
	uint m_mouseRayEntityFlags;
	Vec2 m_anchoredMousePos;
	Vec3 m_worldMousePos;//TODO
	Vec3 m_worldProjectedMousePos; // проекция от камеры на позицию курсора
	Vec2i m_mouseIPos;
	Vec2 m_selectStartPos;
	Vec3 m_worldProjectedSelectStartPos;
	Vec2 m_selectStopPos;
	Vec3 m_draggingDelta;
	std::vector<EntityId> m_selectedEntities;
	std::map<EntityId, Vec3> m_selectStartEntitiesPositions;
	EntityId m_curClickedEntityId;
	EntityId m_lastClickedEntityId;

	float m_mouseDownDurationSec;
	bool m_select;
	bool m_dragging;
	bool m_ctrlModifier;
	bool m_altModifier;

	// Консольные значения
	float tos_sv_zeus_mass_selection_hold_sec;
};
