/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
#include "HUD/GameFlashAnimation.h"
#include "HUD/HUD.h"
#include "TheOtherSideMP\Game\Modules\GenericModule.h"
#include <TheOtherSideMP\Actors\Player\TOSPlayer.h>
#include <TheOtherSideMP\TOSSmartStruct.h>

constexpr uint ZEUS_PP_AMOUNT_KEY = 300;
constexpr auto ZEUS_DEFAULT_MOUSE_ENT_FLAGS = ent_terrain; //ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;
constexpr auto ZEUS_DRAGGING_MOUSE_ENT_FLAGS = ent_terrain;

/**
 * \brief Модуль для обработки ситуаций режима игры Zeus
 */
class CTOSZeusModule : public CTOSGenericModule, IHardwareMouseEventListener, IFSCommandHandler
{
public:
	friend class CHUD;
	friend class CScriptBind_Zeus;

	enum class ECommand : int
	{
		KillSelected,
		RemoveSelected,
		CopySelected,
		OrderSelected,
	};

	enum class EFlag : int
	{
		CanRotateCamera = BIT(0),
		Possessing = BIT(1), // зевс вселился в кого-то
		CanUseMouse = BIT(2),
		Zeusing = BIT(3), // зевс активирован
	};

	enum class EIconColor
	{
		Grey = 1,
		Blue,
		Red,
		Yellow,
	};

	enum class EIcon
	{
		Base = 1,
		Car,
		Helicopter,
		Tank,
		Boat,
		Flag,
		Flash,
		Unit,
		Star,
		Circle,
		AlienTrooper,
		AlienScout,
		Ammo,
		Rifle,
		Antenn,
		DOT,
		Turret,
		BrokenWall,
	};


	struct SUnitIcon
	{
		SUnitIcon(EntityId _id, int _x, int _y, int _icontype, int _friendly, float _distance, float _size, float _rotation, int _healthValue, int _selected)
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

		bool operator ==(const SUnitIcon& compare) const
		{
			return id == compare.id;
		}

	};

	struct SOrderIcon
	{
		SOrderIcon(int _x, int _y, float _size)
			: x(_x),
			y(_y),
			size(_size)
		{

		}

		int x;
		int y;
		float size;
	};

	struct SOBBWorldPos : public STOSSmartStruct
	{
		SOBBWorldPos()
		{
			wPos = Vec3(ZERO);
		}
		SOBBWorldPos(const OBB& _obb, const Vec3& _worldPos)
		{
			obb.c = _obb.c;
			obb.h = _obb.h;
			obb.m33 = _obb.m33;

			wPos = _worldPos;
		}

		OBB obb;
		Vec3 wPos;
	};

	struct SItem
	{
		string strName;
		string strDesc;
		string strClass;
		string strCategory;
		int uniqueLoadoutGroup;
		int uniqueLoadoutCount;
		int iPrice;
		int isUnique;
		int iCount;
		int iMaxCount;
		int iInventoryID;
		float level;
		bool isWeapon;
		bool bAmmoType;
		bool bVehicleType;
		bool loadout;
		bool special;
	};

	struct STab
	{
		STab()
			: iIndex(0),
			strName("")
		{

		}		
		
		STab(int _index)
			: iIndex(_index),
			strName("")
		{

		}

		bool operator > (const STab& other) const
		{
			return this->iIndex > other.iIndex;
		}

		bool operator >= (const STab& other) const
		{
			return this->iIndex >= other.iIndex;
		}

		bool operator < (const STab& other) const
		{
			return this->iIndex < other.iIndex;
		}

		bool operator <= (const STab& other) const
		{
			return this->iIndex <= other.iIndex;
		}

		bool operator == (const STab& other) const
		{
			return this->iIndex == other.iIndex;
		}

		bool operator != (const STab& other) const
		{
			return this->iIndex != other.iIndex;
		}

		string strName;
		int iIndex;
	};

	//enum EOrderType
	//{
	//	E_ORDER_ENTER_VEHICLE,
	//	E_ORDER_PICKUP_ITEM,
	//	E_ORDER_USE_OBJECT,
	//	E_ORDER_PURSUIT_AND_KILL_TARGET,
	//	E_ORDER_GOTO,
	//};

	struct SOrder
	{
		Vec3 pos;
		EntityId targetId;
		//EOrderType type;
	};

	struct Network
	{
		friend class CTOSZeusSynchronizer;
		friend class CTOSZeusModule;

		Network::Network()
			:
			pParent(nullptr)
		{}		
		
		Network::Network(CTOSZeusModule* _pParent)
			:
			pParent(_pParent)
		{}

		void MakeZeus(IActor* pPlayer, bool bMake);
		void SetPP(int amount);
		int  GetPP();

	private:
		void ServerEntitySpawned(EntityId id, const Vec3& pos, int clientChannelId);
		void ServerEntityCopied(EntityId id, const Vec3& pos, int clientChannelId);

	public:
		CTOSZeusModule* pParent;
	};

	struct Local
	{
		friend class CTOSZeusSynchronizer;
		friend class CTOSZeusModule;
		friend class CScriptBind_Zeus;

		Local::Local(CTOSZeusModule* _pParent);
		bool GetFlag(EFlag flag) const;
		CTOSPlayer* GetPlayer() const;

	private:
		void ShowMouse(bool show);
		void SetFlag(EFlag flag, bool value);
		void Reset();

		// Фильтр на сущности. True - сущность можно выбрать, false - нельзя
		void SaveEntitiesStartPositions();
		bool SelectionFilter(EntityId id) const;
		void UpdateUnitIcons(IActor* pClientActor);
		void UpdateOrderIcons();
		void CreateOrder(EntityId executorId, const SOrder& info);
		void StopOrder(EntityId executorId);
		void RemoveOrder(EntityId executorId);

		/// @brief Проекция координат мыши в мир от камеры
		/// @param ray - структура луча
		/// @param mouseWorldPos - мировые координаты мыши, которые будут спроецированы на некоторое расстояние от камеры
		/// @param boxDistanceAdjustment true - максимальное расстояние будет равно расстоянию до кликнутой сущности, false - максимально далеко
		/// @return 0 - попаданий не было, > 0 - есть попадания
		int	MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags, bool boxDistanceAdjustment);

		/// @brief Обновляет позицию перетаскиваемого объекта.
		/// @param id Идентификатор перетаскиваемого объекта.
		/// @param pClickedEntity Указатель на объект, на который щелкнули.
		/// @param pZeusPhys Указатель на физический объект Зевса.
		/// @param container Карта, содержащая позиции всех перетаскиваемых объектов.
		/// @param heightAutoCalc Флаг, указывающий, нужно ли автоматически вычислять высоту перетаскиваемого объекта.
		/// @return true, если обновление прошло успешно, иначе false.
		bool UpdateDraggedEntity(EntityId id, const IEntity* pClickedEntity, IPhysicalEntity* pZeusPhys, std::map<EntityId, _smart_ptr<SOBBWorldPos>>& container, bool heightAutoCalc);
		void UpdateDebug(bool zeusMoving, const Vec3& zeusDynVec);

		void OnEntityIconPressed(IEntity* pEntity);
		EntityId GetMouseEntityId() const;

		/// @brief Получить сущности, находящиеся в границах выделенной через мышь области 
		void GetSelectedEntities();

		/// Можно ли выбрать сущности выделением нескольких сразу?
		bool CanSelectMultiplyWithBox() const;

		void DeselectEntities();
		std::set<EntityId>::iterator DeselectEntity(EntityId id);
		void SelectEntity(EntityId id);
		void ClickEntity(EntityId id, const Vec3& clickPos);
		bool IsSelectedEntity(EntityId id);

		/// @brief Обрабатывает однократный выбор сущности.
		/// @param id Идентификатор выбранной сущности.
		void HandleOnceSelection(EntityId id);

		//Выполнить команду всем выделенным сущностям
		bool ExecuteCommand(ECommand command);

	private:
		float m_draggingMoveStartTimer; /// Таймер начала перемещения сущностей после включения перетаскивания
		float m_mouseDownDurationSec; /// используется для включения режима выделения нескольких объектов одновременно

		bool m_mouseDisplayed;
		bool m_copying;
		bool m_select;
		bool m_dragging;
		bool m_doubleClick;
		bool m_ctrlModifier;
		bool m_shiftModifier;
		bool m_altModifier;
		bool m_debugZModifier;
		bool m_spaceFreeCam;

		uint m_mouseRayEntityFlags;
		uint m_zeusFlags;

		Vec3 m_worldProjectedMousePos; // проекция от камеры до курсора умноженное на некоторое расстояние
		Vec3 m_clickedSelectStartPos; // позиция кликнутой сущности во время её выделения
		Vec3 m_worldMousePos;
		Vec3 m_draggingDelta;
		Vec3 m_orderPos;
		Vec2 m_selectStartPos;
		Vec2 m_selectStopPos;
		Vec2 m_anchoredMousePos; // используется при остановке движения мыши, когда вертится камера
		Vec2i m_mouseIPos;

		SmartScriptTable m_orderInfo;
		SmartScriptTable m_executorInfo;

		std::set<EntityId> m_doubleClickLastSelectedEntities;
		std::set<EntityId> m_selectedEntities; /// выделенные сущности

		std::map<EntityId, Vec3> m_selectStartEntitiesPositions;
		std::map<EntityId, Vec3> m_storedEntitiesPositions;
		std::map<EntityId, _smart_ptr<SOBBWorldPos>> m_boxes; /// боксы выделенных сущностей
		std::map<EntityId, SOrder> m_orders;

		EntityId m_mouseOveredEntityId;
		EntityId m_curClickedEntityId;
		EntityId m_lastClickedEntityId;
		EntityId m_dragTargetId; // Сущность на которую перетаскивают
		EntityId m_orderTargetId;

		ray_hit m_mouseRay;
	public:
		CTOSZeusModule* pParent;
	};

	struct HUD
	{
		friend class CTOSZeusSynchronizer;
		friend class CTOSZeusModule;
		friend class CHUD;

		HUD::HUD(CTOSZeusModule* _pParent);

	private:
		void Reset();

		/// @brief Обновляет иконку бойца на экране.
		/// @param objective идентификатор бойца
		/// @param friendly 0 - серый, 1 - синий, 2 - красный
		/// @param iconType - номер иконки
		/// @param localOffset - локальное смещение иконки
		void CreateUnitIcon(EntityId objective, int friendly, int iconType, const Vec3 localOffset);
		void CreateOrderIcon(const Vec3& worldPos);
		void CreateOrderLine(EntityId executor, const Vec3& orderWorldPos);
		void FlashUpdateUnitIcons();
		void FlashUpdateOrderIcons();
		void Init();
		void InGamePostUpdate(float frametime);
		bool IsShowZeusMenu() const;
		void UnloadSimpleAssets(bool unload);
		void ShowPlayerHUD(bool show);
		void UpdateZeusMenuItemList(const char* szPageIdx);
		bool ShowZeusMenu(bool show);
		/// @brief Считать xml и записать все item'ы в массив
		bool MenuLoadItems();

	public:
		CTOSZeusModule* pParent;
	private:
		CGameFlashAnimation m_animZeusUnitIcons;
		CGameFlashAnimation m_animZeusOrderIcons;
		CGameFlashAnimation m_animZeusMenu;

		std::vector<SUnitIcon> m_unitIcons;
		std::vector<SOrderIcon> m_orderIcons;
		std::map<STab, std::vector<SItem>> m_menuItems;

		bool m_menuShow;
		bool m_menuSpawnHandling; /// true - сущность из меню заспавнена, но не поставлена на карту
		string m_menuFilename;
		uint m_menuCurrentPage;
	};

	CTOSZeusModule();
	virtual ~CTOSZeusModule();
	void Reset();

	//ITOSGameModule
	bool        OnInputEvent(const SInputEvent& event);
	bool        OnInputEventUI(const SInputEvent& event);
	void        OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event);
	void        GetMemoryStatistics(ICrySizer* s);
	const char* GetName();
	void        Init();
	void        Update(float frametime);
	void        Serialize(TSerialize ser);
	int			GetDebugLog();
	void		InitCVars(IConsole* pConsole);
	void		InitCCommands(IConsole* pConsole);
	void		InitScriptBinds();
	void		ReleaseCVars();
	void		ReleaseCCommands();
	void		ReleaseScriptBinds();
	//~ITOSGameModule

	//IHardwareMouseEventListener
	void OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent);
	//~IHardwareMouseEventListener
	
	//IFSCommandHandler
	void HandleFSCommand(const char* pCommand, const char* pArgs);
	//~IFSCommandHandler

	//Console Commands
	static void CmdReloadMenuItems(IConsoleCmdArgs* pArgs);
	static void CmdBecomeZeus(IConsoleCmdArgs* pArgs);
	//~Console Commands

	bool IsPhysicsAllowed(const IEntity* pEntity);
	// Получить локального игрока
	CTOSPlayer* GetPlayer() const; 
	Network& GetNetwork();
	Local& GetLocal();
	HUD& GetHUD();

private:
	Local m_local;
	Network m_network;
	HUD m_hud;

	CScriptBind_Zeus* m_pZeusScriptBind;
	IPersistantDebug* m_pPersistantDebug;

	// std::set<EntityId> m_copiedEntities; /// скопированные сущности
	// std::map<EntityId, SOBBWorldPos*> m_copiedBoxes; /// боксы скопированных сущностей

	// Консольные значения
	float tos_sv_zeus_mass_selection_hold_sec;
	int tos_sv_zeus_selection_always_select_parent;
	int tos_sv_zeus_dragging_ignore_dead_bodies;
	float tos_sv_zeus_dragging_move_start_delay;
	int tos_sv_zeus_dragging_entities_auto_height;
	int tos_sv_zeus_dragging_entities_height_type;

	int tos_sv_zeus_on_screen_force_show;
	float tos_sv_zeus_on_screen_near_size;
	float tos_sv_zeus_on_screen_far_size;
	int tos_sv_zeus_on_screen_near_distance;
	int tos_sv_zeus_on_screen_far_distance;
	int tos_sv_zeus_on_screen_offsetX;
	int tos_sv_zeus_on_screen_offsetY;

	int tos_sv_zeus_selection_ignore_default;
	int tos_sv_zeus_selection_ignore_basic_entity;
	int tos_sv_zeus_selection_ignore_rigid_body;
	int tos_sv_zeus_selection_ignore_destroyable_object;
	int tos_sv_zeus_selection_ignore_breakable_object;
	int tos_sv_zeus_selection_ignore_anim_object;
	int tos_sv_zeus_selection_ignore_pressurized_object;
	int tos_sv_zeus_selection_ignore_switch;
	int tos_sv_zeus_selection_ignore_spawn_group;
	int tos_sv_zeus_selection_ignore_interactive_entity;
	int tos_sv_zeus_selection_ignore_vehicle_part_detached;

	int tos_sv_zeus_update;
	int tos_cl_zeus_dragging_draw_debug;

public:
	static std::map<string, string> s_classToConsoleVar;
};
