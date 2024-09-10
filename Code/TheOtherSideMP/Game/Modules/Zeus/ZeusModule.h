#pragma once
#include "HUD/GameFlashAnimation.h"
#include "HUD/HUD.h"
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

enum EZeusOnScreenIcon
{
	eZSI_Base = 0,
	eZSI_Car,
	eZSI_Helicopter,
	eZSI_Tank,
	eZSI_Boat,
	eZSI_Flag,
	eZSI_Flash,
	eZSI_Unit,
	eZSI_Star,
	eZSI_Circle,
	eZSI_AlienTrooper,
	eZSI_AlienScout,
};

/**
 * \brief Модуль для обработки ситуаций режима игры Zeus
 */
class CTOSZeusModule : public CTOSGenericModule, IHardwareMouseEventListener, IFSCommandHandler
{
public:
	struct SOnScreenIcon
	{
		SOnScreenIcon(EntityId _id, int _x, int _y, int _icontype, int _friendly, float _distance, float _size, float _rotation, int _healthValue, int _selected)
			: id(_id),
			x(_x),
			y(_y),
			icontype(_icontype),
			friendly(_friendly),
			distance(_distance),
			size(_size),
			rotation(_rotation),
			healthValue(_healthValue),
			selected(_selected)
		{}
		EntityId id;
		int x;
		int y;
		int icontype;
		int friendly;
		float distance;
		CryFixedWStringT<64> text;
		float size;
		float rotation;
		int healthValue;
		int selected;

		bool operator ==(const CHUD::SOnScreenIcon& compare) const
		{
			return id == compare.id;
		}

	};

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

	void SaveEntitiesStartPositions();

	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener

	//IFSCommandHandler
	void HandleFSCommand(const char* pCommand, const char* pArgs);
	//~IFSCommandHandler

	void NetMakePlayerZeus(IActor* pPlayer);
	void ShowHUD(bool show);

	void SetZeusFlag(uint flag, bool value);
	bool GetZeusFlag(uint flag) const;
	void Reset();

	void OnAction(const ActionId& action, int activationMode, float value);
protected:
	bool OnActionAttack2(const CTOSActor* pActor, const ActionId& actionId, int activationMode, float value, float pressedDur = 0);

private:
	void UpdateDebug(bool zeusMoving, const Vec3& zeusDynVec);

	/// @brief Проекция координат мыши в мир от камеры
	/// @param ray - структура луча
	/// @param mouseWorldPos - мировые координаты мыши, которые будут спроецированы на некоторое расстояние от камеры
	/// @return 0 - попаданий не было, > 0 - есть попадания
	int	MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags);

	/// Можно ли выбрать сущности выделением нескольких сразу?
	bool CanSelectMultiplyWithBox() const;

	void OnEntityIconPressed(IEntity* pEntity);

	/// @brief Получить сущности, находящиеся в границах выделенной через мышь области 
	void GetSelectedEntities();
	EntityId GetMouseEntityId();

	void DeselectEntities();

	/// @brief Обрабатывает однократный выбор сущности.
	/// @param id Идентификатор выбранной сущности.
	void HandleOnceSelection(EntityId id);
	void ShowMouse(bool show);

	void ApplyZeusProperties(IActor* pPlayer);

	//Выполнить команду всем выделенным сущностям
	bool ExecuteCommand(EZeusCommands command);

	/// @brief Обновляет иконку бойца на экране.
	/// 
	/// @param objective идентификатор бойца
	/// @param friendly 0 - серый, 1 - синий, 2 - красный
	/// @param iconType - номер иконки
	/// @param localOffset - локальное смещение иконки
	/// @param distance 
	/// @param size 
	void HUDUpdateZeusUnitIcon(EntityId objective, int friendly, int iconType, const Vec3 localOffset);
	void HUDUpdateAllZeusUnitIcons();
	void HUDInit();
	void HUDInGamePostUpdate(float frametime);
	void HUDUnloadSimpleAssets(bool unload);

	CTOSPlayer* m_zeus;
	uint m_zeusFlags;
	ray_hit m_mouseRay;
	uint m_mouseRayEntityFlags;
	Vec2 m_anchoredMousePos; // используется при остановке движения мыши, когда вертится камера
	Vec3 m_worldMousePos;
	Vec3 m_worldProjectedMousePos; // проекция от камеры до курсора умноженное на некоторое расстояние
	Vec2i m_mouseIPos;
	Vec2 m_selectStartPos;
	Vec3 m_clickedSelectStartPos; // позиция кликнутой сущности во время её выделения
	Vec2 m_selectStopPos;
	Vec3 m_draggingDelta;

	std::set<EntityId> m_selectedEntities;
	std::map<EntityId, Vec3> m_selectStartEntitiesPositions;
	std::map<EntityId, Vec3> m_storedEntitiesPositions;
	std::vector<SOnScreenIcon> m_onScreenIcons;
	EntityId m_mouseOveredEntityId;
	EntityId m_curClickedEntityId;
	EntityId m_lastClickedEntityId;

	float m_updateIconOnScreenTimer;
	float m_draggingMoveStartTimer; /// Таймер начала перемещения сущностей после включения перетаскивания
	float m_mouseDownDurationSec; /// используется для включения режима выделения нескольких объектов одновременно
	bool m_select;
	bool m_dragging;
	bool m_ctrlModifier;
	bool m_shiftModifier;
	bool m_altModifier;
	bool m_debugZModifier;

	//HUD
	CGameFlashAnimation m_animZeusScreenIcons;

	// Консольные значения
	float tos_sv_zeus_mass_selection_hold_sec;
	int tos_sv_zeus_always_select_parent;
	int tos_sv_zeus_can_drag_dead_bodies;
	float tos_sv_zeus_dragging_move_start_delay;
	int tos_sv_zeus_dragging_entities_auto_height;
	int tos_sv_zeus_dragging_entities_height_type;

	int tos_sv_zeus_force_on_screen_icons;
	float tos_sv_zeus_on_screen_near_size;
	float tos_sv_zeus_on_screen_far_size;
	int tos_sv_zeus_on_screen_near_distance;
	int tos_sv_zeus_on_screen_far_distance;
	int tos_sv_zeus_on_screen_offsetX;
	int tos_sv_zeus_on_screen_offsetY;

	float tos_sv_zeus_on_screen_update_delay;

	int tos_cl_zeus_dragging_draw_debug;
};
