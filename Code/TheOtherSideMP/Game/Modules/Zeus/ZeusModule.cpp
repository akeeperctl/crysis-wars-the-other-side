#include "StdAfx.h"
#include "HUD/HUD.h"
#include "HUD/HUDSilhouettes.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include <TheOtherSideMP\HUD\TOSCrosshair.h>
#include <TheOtherSideMP\Helpers\TOS_AI.h>
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_NET.h>
#include <TheOtherSideMP\Helpers\TOS_Inventory.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include <TheOtherSideMP\Helpers\TOS_Vehicle.h>
#include <TheOtherSideMP\Helpers\TOS_STL.h>
#include <Cry_Camera.h>

const uint DEFAULT_MOUSE_ENT_FLAGS = ent_terrain; //ent_living | ent_rigid | ent_static | ent_terrain | ent_sleeping_rigid | ent_independent;
const uint DRAGGING_MOUSE_ENT_FLAGS = ent_terrain;

static void InitSelectionFilterClasses();
std::map<string, string> CTOSZeusModule::s_classToConsoleVar;

enum eScreenIconType
{
	eFTI_Grey = 0,
	eFTI_Blue,
	eFTI_Red,
	eFTI_Yellow,
};

CTOSZeusModule::CTOSZeusModule()
	: m_zeus(nullptr),
	m_zeusFlags(0),
	m_anchoredMousePos(ZERO),
	m_worldMousePos(ZERO),
	m_worldProjectedMousePos(ZERO),
	m_selectStartPos(ZERO),
	m_clickedSelectStartPos(ZERO),
	m_selectStopPos(ZERO),
	m_mouseIPos(ZERO),
	m_draggingDelta(ZERO),
	m_lastClickedEntityId(0),
	m_curClickedEntityId(0),
	m_mouseOveredEntityId(0),
	m_mouseDownDurationSec(0),
	m_draggingMoveStartTimer(0),
	m_updateIconOnScreenTimer(0),
	m_mouseRayEntityFlags(DEFAULT_MOUSE_ENT_FLAGS),
	m_shiftModifier(false),
	m_ctrlModifier(false),
	m_doubleClick(false),
	m_copying(false),
	m_altModifier(false),
	m_debugZModifier(false),
	m_select(false),
	m_dragging(false)
{}

CTOSZeusModule::~CTOSZeusModule()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->RemoveListener(this);
}

void CTOSZeusModule::Init()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->AddListener(this);

	InitSelectionFilterClasses();
	Reset();
}

void CTOSZeusModule::Reset()
{
	if (m_zeus)
	{
		m_zeus->m_isZeus = false;
		m_zeus = nullptr;
	}
	m_zeusFlags = 0;
	m_select = false;
	m_dragging = false;
	m_doubleClick = false;
	m_copying = false;
	m_ctrlModifier = false;
	m_altModifier = false;
	m_shiftModifier = false;
	m_debugZModifier = false;


	m_selectedEntities.clear();
	m_selectStartEntitiesPositions.clear();
	m_storedEntitiesPositions.clear();
	m_boxes.clear();
	m_lastClickedEntityId = m_curClickedEntityId = 0;
	m_mouseOveredEntityId = 0;
	m_updateIconOnScreenTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_update_delay", 0);
}

bool CTOSZeusModule::OnInputEvent(const SInputEvent& event)
{
	if (event.deviceId == EDeviceId::eDI_Keyboard)
	{
		if (event.keyId == EKeyId::eKI_LCtrl)
		{
			if (event.state == eIS_Pressed)
				m_ctrlModifier = true;
			else if (event.state == eIS_Released)
				m_ctrlModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_C)
		{
			if (event.state == eIS_Pressed)
			{
				if (m_ctrlModifier && !m_copying)
				{
					m_copying = true;
					ExecuteCommand(eZC_CopySelected);
				}
			}
		}
		else if (event.keyId == EKeyId::eKI_LAlt)
		{
			if (event.state == eIS_Pressed)
			{
				m_altModifier = true;
				m_select = false;
			}
			else if (event.state == eIS_Released)
			{
				m_altModifier = false;
			}
		}
		else if (event.keyId == EKeyId::eKI_Z)
		{
			if (event.state == eIS_Pressed)
				m_debugZModifier = true;
			else if (event.state == eIS_Released)
			{
				if (m_debugZModifier)
				{
					m_debugZModifier = false;

					for (auto it = m_selectedEntities.cbegin(); it != m_selectedEntities.cend(); it++)
					{
						if (!SelectionFilter(*it))
							it = DeselectEntity(*it);
					}
				}
			}
		}
		else if (event.keyId == EKeyId::eKI_LShift)
		{
			if (event.state == eIS_Pressed)
				m_shiftModifier = true;
			else if (event.state == eIS_Released)
				m_shiftModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_End)
		{
			ExecuteCommand(eZC_KillSelected);
		}
		else if (event.keyId == EKeyId::eKI_Delete)
		{
			ExecuteCommand(eZC_RemoveSelected);
		}
	}

	return true;
}

bool CTOSZeusModule::CanSelectMultiplyWithBox() const
{
	return m_mouseDownDurationSec > TOS_Console::GetSafeFloatVar("tos_sv_zeus_mass_selection_hold_sec", 0.2f);
}

EntityId CTOSZeusModule::GetMouseEntityId()
{
	if (!m_zeus)
		return 0;

	const auto vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
	const auto vDir = (m_worldProjectedMousePos - vCamPos).GetNormalizedSafe();
	auto pPhys = m_zeus->GetEntity()->GetPhysics();

	ray_hit hit;
	const auto queryFlags = ent_all;
	const unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
	const float fRange = gEnv->p3DEngine->GetMaxViewDistance();

	if (gEnv->pPhysicalWorld && gEnv->pPhysicalWorld->RayWorldIntersection(vCamPos, vDir * fRange, queryFlags, flags, &hit, 1, pPhys))
	{
		if (gEnv->p3DEngine->RefineRayHit(&hit, vDir * fRange))
		{
			if (const auto pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider))
				return pEntity->GetId();
		}
	}

	return 0;
}

void CTOSZeusModule::DeselectEntities()
{
	m_doubleClickLastSelectedEntities.clear();
	m_selectedEntities.clear();
	m_boxes.clear();
}

std::set<EntityId>::iterator CTOSZeusModule::DeselectEntity(EntityId id)
{
	auto it = std::lower_bound(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (it != m_selectedEntities.end() && *it == id)
	{
		it = m_selectedEntities.erase(it);
		m_boxes.erase(id);
		return it;
	}
}

static CTOSZeusModule::SOBBWorldPos* CreateBoxForEntity(EntityId id)
{
	OBB box;
	AABB entityBox;
	Vec3 worldPos(ZERO);

	auto pEntity = TOS_GET_ENTITY(id);
	if (pEntity)
	{
		pEntity->GetLocalBounds(entityBox);
		box.SetOBBfromAABB(pEntity->GetRotation(), entityBox);

		worldPos = pEntity->GetWorldPos();
	}

	auto wbox = new CTOSZeusModule::SOBBWorldPos();

	if (box.c.IsZero())
	{
		pEntity->GetWorldBounds(entityBox);
		box = OBB::CreateOBBfromAABB(pEntity->GetRotation(), entityBox);
	}

	wbox->obb = box;
	wbox->wPos = worldPos;

	return wbox;
}

void CTOSZeusModule::SelectEntity(EntityId id)
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (pEntity)
	{
		m_selectedEntities.insert(id);
		m_boxes[id] = CreateBoxForEntity(id);
	}
}

void CTOSZeusModule::HandleOnceSelection(EntityId id)
{
	// при копировании нельзя выделять новые сущности
	// т.к. сначала нужно переместить скопированные на места
	if (!m_copying)
	{
		m_lastClickedEntityId = m_curClickedEntityId;
		m_curClickedEntityId = id;

		// Одиночное выделение
		if (!m_ctrlModifier)
		{
			//const auto clickedIter = stl::binary_find(m_selectedEntities.begin(), m_selectedEntities.end(), m_curClickedEntityId);
			//const bool clickedSelected = clickedIter != m_selectedEntities.end();

			// При каждом клике без CTRL снимаем выделение если кликнули не на последнюю кликнутую сущность
			//if (m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0)
			if ((m_curClickedEntityId != m_lastClickedEntityId || m_curClickedEntityId == 0))
				DeselectEntities();

			// После чистки всех выделенных сущностей, выделяем кликнутую сущность
			if (m_curClickedEntityId != 0 && SelectionFilter(m_curClickedEntityId))
				SelectEntity(m_curClickedEntityId);
		}
	}

	auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	if (pEntity)
		m_clickedSelectStartPos = pEntity->GetWorldPos();
}

void CTOSZeusModule::OnEntityIconPressed(IEntity* pEntity)
{
	if (!pEntity)
		return;

	HandleOnceSelection(pEntity->GetId());
}

void CTOSZeusModule::SaveEntitiesStartPositions()
{
	m_selectStartEntitiesPositions.clear();

	for (auto it = m_selectedEntities.begin(); it != m_selectedEntities.end(); it++)
	{
		EntityId id = *it;

		auto pos = stl::find_in_map(m_selectStartEntitiesPositions, id, Vec3(ZERO));
		if (pos.IsZero())
		{
			auto pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				m_selectStartEntitiesPositions[id] = pEntity->GetWorldPos();
				m_storedEntitiesPositions[id] = pEntity->GetWorldPos();
			}
		}
	}
}

static bool IsPhysicsAllowed(const IEntity* pEntity)
{
	if (!pEntity)
		return false;

	IPhysicalEntity* physEnt = pEntity->GetPhysics();
	if (!physEnt)
		return false;

	// допустимые сущности
	const auto type = physEnt->GetType();
	if (!(type == PE_LIVING || type == PE_RIGID || type == PE_STATIC || type == PE_WHEELEDVEHICLE || PE_ARTICULATED))
		return false;
}

static bool EntityIsSimilarToEntity(IEntity* pFirstEntity, IEntity* pSecondEntity)
{
	if (!pFirstEntity || !pSecondEntity)
		return false;

	if (pFirstEntity->GetClass() != pSecondEntity->GetClass())
		return false;

	const auto pFirstArchetype = pFirstEntity->GetArchetype();
	const auto pSecondArchetype = pSecondEntity->GetArchetype();
	if (pFirstArchetype && pSecondArchetype)
	{
		if (string(pFirstArchetype->GetName()) != pSecondArchetype->GetName())
			return false;
	}

	int firstSpecies = -1;
	int secondSpecies = -1;

	SmartScriptTable props;
	const auto pFirstTable = pFirstEntity->GetScriptTable();
	if (pFirstTable)
	{
		pFirstTable->GetValue("Properties", props);
		props->GetValue("species", firstSpecies);
	}

	const auto pSecondTable = pSecondEntity->GetScriptTable();
	if (pSecondTable)
	{
		pSecondEntity->GetScriptTable()->GetValue("Properties", props);
		props->GetValue("species", secondSpecies);
	}

	if (firstSpecies != secondSpecies)
		return false;

	return true;
}

// Карта для сопоставления названий классов с консольными переменными
static void InitSelectionFilterClasses()
{
	CTOSZeusModule::s_classToConsoleVar["BasicEntity"] = "tos_sv_zeus_selection_ignore_basic_entity";
	CTOSZeusModule::s_classToConsoleVar["RigidBody"] = "tos_sv_zeus_selection_ignore_rigid_body";
	CTOSZeusModule::s_classToConsoleVar["RigidBodyEx"] = "tos_sv_zeus_selection_ignore_rigid_body";
	CTOSZeusModule::s_classToConsoleVar["DestroyableObject"] = "tos_sv_zeus_selection_ignore_destroyable_object";
	CTOSZeusModule::s_classToConsoleVar["BreakableObject"] = "tos_sv_zeus_selection_ignore_breakable_object";
	CTOSZeusModule::s_classToConsoleVar["AnimObject"] = "tos_sv_zeus_selection_ignore_anim_object";
	CTOSZeusModule::s_classToConsoleVar["PressurizedObject"] = "tos_sv_zeus_selection_ignore_pressurized_object";
	CTOSZeusModule::s_classToConsoleVar["Switch"] = "tos_sv_zeus_selection_ignore_switch";
	CTOSZeusModule::s_classToConsoleVar["SpawnGroup"] = "tos_sv_zeus_selection_ignore_spawn_group";
	CTOSZeusModule::s_classToConsoleVar["InteractiveEntity"] = "tos_sv_zeus_selection_ignore_interactive_entity";
	CTOSZeusModule::s_classToConsoleVar["VehiclePartDetached"] = "tos_sv_zeus_selection_ignore_vehicle_part_detached";
}

bool CTOSZeusModule::SelectionFilter(EntityId id)
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (!pEntity)
	{
		return false;
	}

	if (m_debugZModifier)
		return true;

	const string className = pEntity->GetClass()->GetName();
	auto it = s_classToConsoleVar.find(className);

	// Проверяем, есть ли сопоставление для текущего класса
	if (it != s_classToConsoleVar.end())
	{
		// Получаем консольную переменную для текущего класса
		const string consoleVarName = it->second;

		// Проверяем значение консольной переменной
		if (TOS_Console::GetSafeIntVar(consoleVarName.c_str(), 1) == 1)
		{
			return false;
		}
	}

	// Проверяем класс по умолчанию
	if (pEntity->GetClass() == gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass())
	{
		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_selection_ignore_default", 1) == 1)
		{
			return false;
		}
	}

	return true;
}

void CTOSZeusModule::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_mouseIPos.x = iX;
	m_mouseIPos.y = iY;

	auto mod_iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, mod_iY, 0.0f, &m_worldMousePos.x, &m_worldMousePos.y, &m_worldMousePos.z);

	auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		SFlashCursorEvent::ECursorState eCursorState = SFlashCursorEvent::eCursorMoved;
		if (HARDWAREMOUSEEVENT_LBUTTONDOWN == eHardwareMouseEvent)
		{
			eCursorState = SFlashCursorEvent::eCursorPressed;
		}
		else if (HARDWAREMOUSEEVENT_LBUTTONUP == eHardwareMouseEvent)
		{
			eCursorState = SFlashCursorEvent::eCursorReleased;
		}

		if (m_animZeusScreenIcons.IsLoaded())
		{
			int x(iX), y(iY);
			m_animZeusScreenIcons.GetFlashPlayer()->ScreenToClient(x, y);
			m_animZeusScreenIcons.GetFlashPlayer()->SendCursorEvent(SFlashCursorEvent(eCursorState, x, y));
		}

		if (!pHUD->IsHaveModalHUD())
		{
			if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOUBLECLICK)
			{
				m_doubleClick = true;
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOWN)
			{
				m_select = true;
				m_selectStartPos = Vec2i(iX, iY);

				HandleOnceSelection(GetMouseEntityId());
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONUP)
			{
				m_select = false;
				m_selectStopPos = Vec2i(iX, iY);

				if (!m_dragging && CanSelectMultiplyWithBox())
					GetSelectedEntities();
				else
				{
					// Множественное выделение c зажатым модификатором
					if (m_ctrlModifier)
					{
						if (!m_dragging && !m_doubleClick && m_curClickedEntityId != 0)
						{
							if (m_selectedEntities.count(m_curClickedEntityId) > 0)
							{
								//if (delta <= 0.15f)
								DeselectEntity(m_curClickedEntityId);
							}
							else
							{
								if (SelectionFilter(m_curClickedEntityId))
									SelectEntity(m_curClickedEntityId);
							}
						}
					}
				}

				if (m_doubleClick)
				{
					const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
					if (pClickedEntity)
					{
						SmartScriptTable props;
						int clickedSpecies = -1;
						auto pTable = pClickedEntity->GetScriptTable();
						if (pTable)
						{
							pTable->GetValue("Properties", props);
							props->GetValue("species", clickedSpecies);
						}

						const IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
						if (!pClientActor)
							return;

						const auto clickedIter = stl::binary_find(m_doubleClickLastSelectedEntities.cbegin(), m_doubleClickLastSelectedEntities.cend(), m_curClickedEntityId);
						const bool clickedSelected = clickedIter != m_doubleClickLastSelectedEntities.cend();
						if (clickedSelected)
						{
							// Снимаем выделение последних выделенных подобных сущностей
							for (auto it = m_doubleClickLastSelectedEntities.begin(); it != m_doubleClickLastSelectedEntities.end();)
							{
								auto pEntity = TOS_GET_ENTITY(*it);

								if (EntityIsSimilarToEntity(pEntity, pClickedEntity) || pEntity == pClickedEntity)
								{
									DeselectEntity(*it);
									it = m_doubleClickLastSelectedEntities.erase(it);
								}
								else
								{
									it++;
								}
							}
						}
						else
						{
							// Выбираем подобные сущности в поле зрения

							IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
							while (!pIt->IsEnd())
							{
								if (IEntity* pEntity = pIt->Next())
								{
									const auto id = pEntity->GetId();

									if (!SelectionFilter(id))
										continue;

									if (pEntity->IsGarbage())
										continue;

									if (pEntity->IsHidden())
										continue;

									if (!IsPhysicsAllowed(pEntity))
										continue;

									// пропускаем сущность актера клиента
									if (id == pClientActor->GetEntityId())
										continue;

									AABB worldBounds;
									pEntity->GetWorldBounds(worldBounds);

									//skip further calculations if the entity is not visible at all...
									if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
										continue;

									if (!EntityIsSimilarToEntity(pEntity, pClickedEntity))
										continue;

									SelectEntity(id);
									m_doubleClickLastSelectedEntities.insert(id);
								}
							}
						}
					}



					m_doubleClick = false;
				}

				// Перед тем как закончить копирование..
				// Показываем все скопированные сущности
				if (m_copying)
				{
					for (auto it = m_selectedEntities.cbegin(); it != m_selectedEntities.cend(); it++)
					{
						auto pEntity = TOS_GET_ENTITY(*it);
						if (pEntity)
							pEntity->Hide(false);
					}
				}

				m_copying = false;

				if (m_dragging)
				{
					for (auto it = m_selectedEntities.begin(); it != m_selectedEntities.end(); it++)
					{
						auto pEntity = TOS_GET_ENTITY(*it);
						if (!pEntity)
							continue;

						// Применяем сдвинутые позиции боксов на сущности
						const auto pBox = m_boxes[pEntity->GetId()];
						pEntity->SetWorldTM(Matrix34::CreateTranslationMat(pBox->wPos));
						pEntity->SetRotation(Quat(pBox->obb.m33));

						// Пинаем физику выделенных сущностей после того как закончили их перетаскивать
						auto pPhys = pEntity->GetPhysics();
						if (!pPhys)
							continue;

						pe_action_awake awake;
						awake.bAwake = 1;

						pPhys->Action(&awake);
					}
				}

				m_dragging = false;
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_MOVE)
			{
				// При перетаскивании кликнутая сущность должна быть выделена
				const auto clickedIter = stl::binary_find(m_selectedEntities.begin(), m_selectedEntities.end(), m_curClickedEntityId);
				const bool clickedSelected = clickedIter != m_selectedEntities.end();

				// Мышь находится в диапазоне иконки кликнутой сущности. True - да
				const bool clickedOveredByMouse = m_mouseOveredEntityId == m_curClickedEntityId;

				if ((m_select) && m_curClickedEntityId != 0 && clickedSelected && clickedOveredByMouse)
				{
					// Перед тем как начать перемещение сущностей...
					if (m_dragging == false)
					{
						// Сохраняем начальное положение каждой выделенной сущности
						SaveEntitiesStartPositions();

						// Запуск таймера 
						m_draggingMoveStartTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_dragging_move_start_delay", 0.05f);
					}

					m_dragging = true;
				}
			}

		}
	}
}

void CTOSZeusModule::GetSelectedEntities()
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (!pIt->IsEnd())
	{
		if (IEntity* pEntity = pIt->Next())
		{
			// выделяем только родительскую сущность
			if (TOS_Console::GetSafeIntVar("tos_sv_zeus_selection_always_select_parent", 1) == 1 && pEntity->GetParent())
				pEntity = pEntity->GetParent();

			if (!SelectionFilter(pEntity->GetId()))
				continue;

			if (m_debugZModifier == false)
			{
				// пропускаем мусор (при удалении сущности, в редакторе остается мусор от сущности, который можно выделить повторно)
				if (pEntity->IsGarbage())
					continue;

				if (!IsPhysicsAllowed(pEntity))
					continue;
			}

			IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
			if (!pClientActor)
				return;

			// пропускаем сущность актера клиента
			if (pEntity->GetId() == pClientActor->GetEntityId())
				continue;

			AABB worldBounds;
			pEntity->GetWorldBounds(worldBounds);

			//skip further calculations if the entity is not visible at all...
			if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
				continue;

			const Vec3 wpos = pEntity->GetWorldPos();
			const Quat rot = pEntity->GetWorldRotation();
			AABB localBounds;

			pEntity->GetLocalBounds(localBounds);

			//get min and max values of the entity bounding box (local positions)
			static Vec3 points[2];
			points[0] = wpos + rot * localBounds.min;
			points[1] = wpos + rot * localBounds.max;

			static Vec3 pointsProjected[2];

			//project the bounding box min max values to screen positions
			for (int i = 0; i < 2; ++i)
			{
				gEnv->pRenderer->ProjectToScreen(points[i].x, points[i].y, points[i].z, &pointsProjected[i].x, &pointsProjected[i].y, &pointsProjected[i].z);
				const float fWidth = gEnv->pRenderer->GetWidth();
				const float fHeight = gEnv->pRenderer->GetHeight();

				//scale projected values to the actual screen resolution
				pointsProjected[i].x *= 0.01f * fWidth;
				pointsProjected[i].y *= 0.01f * fHeight;
			}

			// Check if the projected bounding box min max values are fully or partly inside the screen selection 
			bool isXOverlap =
				// Полное пересечение по X
				(m_selectStartPos.x <= pointsProjected[0].x && pointsProjected[1].x <= m_selectStopPos.x) ||
				// Полное пересечение по X (справа налево)
				(m_selectStartPos.x >= pointsProjected[0].x && pointsProjected[1].x >= m_selectStopPos.x) ||
				// Частичное пересечение по X
				(m_selectStartPos.x <= pointsProjected[0].x && pointsProjected[0].x <= m_selectStopPos.x) ||
				(m_selectStartPos.x >= pointsProjected[0].x && m_selectStopPos.x <= pointsProjected[1].x) ||
				(m_selectStartPos.x <= pointsProjected[1].x && m_selectStopPos.x >= pointsProjected[1].x);

			bool isYOverlap =
				// Полное пересечение по Y
				(m_selectStartPos.y <= pointsProjected[0].y && pointsProjected[1].y <= m_selectStopPos.y) ||
				// Полное пересечение по Y (снизу вверх)
				(m_selectStartPos.y >= pointsProjected[0].y && pointsProjected[1].y >= m_selectStopPos.y) ||
				// Частичное пересечение по Y
				(m_selectStartPos.y <= pointsProjected[0].y && m_selectStopPos.y >= pointsProjected[0].y) ||
				(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[0].y) ||
				(m_selectStartPos.y <= pointsProjected[1].y && m_selectStopPos.y >= pointsProjected[1].y);

			if (isXOverlap && isYOverlap)
			{
				// finally we have an entity id
				// if left or right CTRL is not pressed we can directly add every entity id, old entity id's are already deleted
				//m_selectedEntities.insert(pEntity->GetId());
				SelectEntity(pEntity->GetId());
			}
		}
	}
}

void CTOSZeusModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	auto pHUD = g_pGame->GetHUD();
	bool noModalOrNoHUD = !pHUD || (pHUD && !pHUD->IsHaveModalHUD());

	switch (event.event)
	{
		case eEGE_ActorRevived:
		{
			if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
			{
				Reset();
				if (noModalOrNoHUD)
					ShowMouse(false);
			}
			break;
		}
		case eEGE_MasterClientOnStartControl:
		{
			if (m_zeus)
			{
				ShowHUD(true);
				if (noModalOrNoHUD)
					ShowMouse(false);

				SetZeusFlag(eZF_Possessing, true);
			}
			break;
		}
		case eEGE_MasterClientOnStopControl:
		{
			if (m_zeus)
			{
				ApplyZeusProperties(m_zeus);
				SetZeusFlag(eZF_Possessing, false);
			}
			break;
		}
		case eEGE_HUDInit:
		{
			HUDInit();
			break;
		}
		case eEGE_HUDInGamePostUpdate:
		{
			HUDInGamePostUpdate(event.value);
			break;
		}
		case eEGE_HUDUnloadSimpleAssets:
		{
			HUDUnloadSimpleAssets(event.int_value);
			break;
		}
		//case eEGE_ActorEnterVehicle:
		//{
		//	if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
		//	{
		//		if (noModalOrNoHUD)
		//			ShowMouse(false);
		//	}

		//	break;
		//}
		//case eEGE_ActorExitVehicle:
		//{
		//	if (m_zeus && m_zeus->GetEntityId() == pEntity->GetId())
		//	{				
		//		//ApplyZeusProperties(m_zeus);
		//	}
		//	break;
		//}
		default:
			break;
	}
}


void CTOSZeusModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	s->AddContainer(m_selectedEntities);
	s->AddContainer(m_doubleClickLastSelectedEntities);
	s->AddContainer(m_selectStartEntitiesPositions);
	s->AddContainer(m_storedEntitiesPositions);
	s->AddContainer(m_onScreenIcons);
	s->AddContainer(m_boxes);
}

int CTOSZeusModule::MouseProjectToWorld(ray_hit& ray, const Vec3& mouseWorldPos, uint entityFlags)
{
	if (!m_zeus || !m_zeus->GetEntity() || !m_zeus->GetEntity()->GetPhysics())
		return 0;

	const Vec3 camWorldPos = gEnv->pSystem->GetViewCamera().GetPosition();
	Vec3 camToMouseDir = (mouseWorldPos - camWorldPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();

	float clickedBoxDistance = 10.0f; // дистанция до кликнутой сущности

	IPhysicalEntity* pSkipEnts[2] = {m_zeus->GetEntity()->GetPhysics(), nullptr};
	const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	if (pClickedEntity)
		pSkipEnts[1] = pClickedEntity->GetPhysics();

	//auto obbWP = stl::find_in_map(m_boxes, m_curClickedEntityId, SOBBWorldPos());
	auto it = m_boxes.find(m_curClickedEntityId);
	if (it != m_boxes.end())
	{
		//clickedBoxDistance = pClickedEntity->GetWorldPos().GetDistance(mouseWorldPos);
		clickedBoxDistance = it->second->wPos.GetDistance(mouseWorldPos);
		camToMouseDir = camToMouseDir.GetNormalizedSafe() * clickedBoxDistance;
	}

	const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
	const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_stop_at_pierceable);

	return gEnv->pPhysicalWorld->RayWorldIntersection(mouseWorldPos, camToMouseDir, entityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
}

bool CTOSZeusModule::UpdateDraggedEntity(EntityId id, const IEntity* pClickedEntity, IPhysicalEntity* pZeusPhys, std::map<EntityId, SOBBWorldPos*>& container, bool heightAutoCalc)
{
	auto pEntity = TOS_GET_ENTITY(id);
	if (!pEntity)
		return false;

	const string className = pEntity->GetClass()->GetName();

	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
	if ((pActor && pActor->GetHealth() < 0.0f) || className == "DeadBody")
	{
		// Пропускаем убитых актеров при условии
		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_ignore_dead_bodies", 0) < 1)
			return false;
	}

	Matrix34 mat34 = pEntity->GetWorldTM();
	//const Vec3 curPos = mat34.GetTranslation();
	Vec3 curPos = container[id]->wPos;
	if (curPos.IsZero())
		curPos = mat34.GetTranslation();

	const Vec3 startPos = m_selectStartEntitiesPositions[id];

	if (!m_shiftModifier)
	{
		Vec3 newPos(ZERO);
		if (m_altModifier)
		{
			// Перемещение по Z
			float height = startPos.z + m_draggingDelta.z;
			m_storedEntitiesPositions[id].z = height;

			newPos = Vec3(curPos.x, curPos.y, height);
		}
		else
		{
			// Перемещение по X, Y с автоприлипанием к поверхности
			newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, m_storedEntitiesPositions[id].z);

			if (heightAutoCalc)
			{
				const Vec3 entToGroundDir = (Vec3(curPos.x, curPos.y, curPos.z - 2) - curPos).GetNormalizedSafe() * gEnv->p3DEngine->GetMaxViewDistance();
				const int rayFlags = (COLLISION_RAY_PIERCABILITY & rwi_pierceability_mask);
				ray_hit ray;

				IPhysicalEntity* pSkipEnts[3];
				pSkipEnts[0] = pZeusPhys;
				pSkipEnts[1] = pEntity->GetPhysics();

				if (pClickedEntity)
					pSkipEnts[2] = pClickedEntity->GetPhysics();

				const int nSkip = sizeof(pSkipEnts) / sizeof(pSkipEnts[0]);
				const int hitNum = gEnv->pPhysicalWorld->RayWorldIntersection(Vec3(curPos.x, curPos.y, curPos.z + 1), entToGroundDir, m_mouseRayEntityFlags, rayFlags, &ray, 1, pSkipEnts, nSkip);
				if (hitNum)
					newPos.z = ray.pt.z + 0.01f;
			}
		}

		container[id]->wPos = newPos;

		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_move_boxes_separately", 0) == 0)
		{
			mat34.SetTranslation(container[id]->wPos);
			pEntity->SetWorldTM(mat34);
		}
	}
	else
	{
		const Vec3 toMouseDir = (m_worldProjectedMousePos - curPos).GetNormalizedSafe();
		const Quat rot = Quat::CreateRotationVDir(toMouseDir);
		container[id]->wPos = curPos;
		container[id]->obb.m33 = Matrix33(rot);

		if (TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_move_boxes_separately", 0) == 0)
		{
			// Чтобы не падали сущности на высоте
			mat34.SetTranslation(Vec3(container[id]->wPos.x, container[id]->wPos.y, m_storedEntitiesPositions[id].z));
			pEntity->SetWorldTM(mat34);
			pEntity->SetRotation(rot);
		}
	}

	return true;
}

void CTOSZeusModule::Update(float frametime)
{
	if (!m_zeus || !m_zeus->IsClient())
		return;

	//int frameID = gEnv->pRenderer->GetFrameID();

	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		// Привязка мыши к позиции, когда крутится камера
		///////////////////////////////////////////////////////////////////////
		if (m_zeusFlags & eZF_CanRotateCamera && !(m_zeusFlags & eZF_Possessing))
		{
			if (m_anchoredMousePos == Vec2(0, 0))
			{
				Vec2 mousePos;
				pMouse->GetHardwareMousePosition(&mousePos.x, &mousePos.y);

				m_anchoredMousePos = mousePos;
			}

			pMouse->SetHardwareMousePosition(m_anchoredMousePos.x, m_anchoredMousePos.y);
		}
		else
		{
			if (m_anchoredMousePos != Vec2(0, 0))
				m_anchoredMousePos.zero();
		}
	}

	if (m_select)
		m_mouseDownDurationSec += frametime;
	else
		m_mouseDownDurationSec = 0.0f;

	MouseProjectToWorld(m_mouseRay, m_worldMousePos, m_mouseRayEntityFlags);
	m_worldProjectedMousePos = m_mouseRay.pt;

	// Обработка таймера задержки перемещения выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_dragging && m_draggingMoveStartTimer > 0.0f)
		m_draggingMoveStartTimer -= frametime;

	if (m_draggingMoveStartTimer <= 0.0f)
		m_draggingMoveStartTimer = 0.0f;

	pe_status_dynamics zeus_dyn;
	const auto pZeusPhys = m_zeus->GetEntity()->GetPhysics();
	if (pZeusPhys)
		pZeusPhys->GetStatus(&zeus_dyn);

	const bool zeusMoving = zeus_dyn.v.len() > 0.1f;

	//Перемещение боксов выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_dragging && !zeusMoving && m_draggingMoveStartTimer == 0.0f)
	{
		const bool autoEntitiesHeight = TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_entities_auto_height", 0) == 1; // Расчет высоты для каждой сущности отдельно
		const auto pClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);

		m_mouseRayEntityFlags = DRAGGING_MOUSE_ENT_FLAGS;
		m_draggingDelta = m_worldProjectedMousePos - m_clickedSelectStartPos;

		for (auto it = m_selectedEntities.cbegin(); it != m_selectedEntities.cend(); it++)
		{
			const auto id = *it;
			if (!UpdateDraggedEntity(id, pClickedEntity, pZeusPhys, m_boxes, autoEntitiesHeight))
				continue;
		}
	}
	else if (!m_dragging)
	{
		for (auto it = m_selectedEntities.cbegin(); it != m_selectedEntities.cend(); it++)
		{
			const EntityId id = *it;
			const IEntity* pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				m_boxes[id]->wPos = pEntity->GetWorldPos();
				m_boxes[id]->obb.m33 = Matrix33(pEntity->GetRotation());
			}

		}
	}
	else
	{
		m_mouseRayEntityFlags = DEFAULT_MOUSE_ENT_FLAGS;
	}

	// Отрисовка границ выделения
	///////////////////////////////////////////////////////////////////////
	if (m_select && CanSelectMultiplyWithBox() && !m_dragging)
	{
		if (IRenderAuxGeom* pGeom = gEnv->pRenderer->GetIRenderAuxGeom())
		{
			//calculate the four selection boundary points
			Vec3 vTopLeft(m_selectStartPos.x, m_selectStartPos.y, 0.0f);
			Vec3 vTopRight(m_mouseIPos.x, m_selectStartPos.y, 0.0f);
			Vec3 vBottomLeft(m_selectStartPos.x, m_mouseIPos.y, 0.0f);
			Vec3 vBottomRight(m_mouseIPos.x, m_mouseIPos.y, 0.0f);

			gEnv->pRenderer->Set2DMode(true, gEnv->pRenderer->GetWidth(), gEnv->pRenderer->GetHeight());

			//set boundary color: white
			ColorB col(255, 255, 255, 255);

			pGeom->DrawLine(vTopLeft, col, vTopRight, col);
			pGeom->DrawLine(vTopRight, col, vBottomRight, col);
			pGeom->DrawLine(vTopLeft, col, vBottomLeft, col);
			pGeom->DrawLine(vBottomLeft, col, vBottomRight, col);

			gEnv->pRenderer->Set2DMode(false, 0, 0);
		}
	}

	// Отрисовка флэш иконок под сущностями
	///////////////////////////////////////////////////////////////////////
	if (m_updateIconOnScreenTimer > 0.0f)
		m_updateIconOnScreenTimer -= frametime;

	if (m_updateIconOnScreenTimer <= 0.0f)
	{
		m_updateIconOnScreenTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_on_screen_update_delay", 0);

		IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
		if (!pClientActor)
			return;

		IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
		while (!pIt->IsEnd())
		{
			int color = eFTI_Grey;
			int icon = eZSI_Base;

			if (IEntity* pEntity = pIt->Next())
			{
				const auto id = pEntity->GetId();

				if (!SelectionFilter(id))
					continue;

				if (pEntity->IsGarbage())
					continue;

				if (pEntity->IsHidden())
				{
					const auto selectedIter = stl::binary_find(m_selectedEntities.cbegin(), m_selectedEntities.cend(), id);
					const bool selected = selectedIter != m_selectedEntities.cend();
					if (!(m_copying && selected))
					{
						continue;
					}
				}

				if (!IsPhysicsAllowed(pEntity))
					continue;

				// пропускаем сущность актера клиента
				if (id == pClientActor->GetEntityId())
					continue;

				AABB worldBounds;
				pEntity->GetWorldBounds(worldBounds);

				//skip further calculations if the entity is not visible at all...
				if (gEnv->pSystem->GetViewCamera().IsAABBVisible_F(worldBounds) == CULL_EXCLUSION)
					continue;

				IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
				if (pVehicle)
				{
					//TODO Танк, VTOL!
					const auto movType = pVehicle->GetMovement()->GetMovementType();
					if (movType == IVehicleMovement::eVMT_Land)
						icon = eZSI_Car;
					else if (movType == IVehicleMovement::eVMT_Air)
						icon = eZSI_Helicopter;
					else if (movType == IVehicleMovement::eVMT_Sea || IVehicleMovement::eVMT_Amphibious)
						icon = eZSI_Boat;

					// не пустые тс должны быть желтые
					if (pVehicle->GetStatus().passengerCount > 0)
						color = eFTI_Yellow;

					if (pVehicle->IsDestroyed())
						color = eFTI_Grey;
				}

				const IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
				if (pActor)
				{
					icon = eZSI_Unit;

					if (pActor->GetHealth() > 0)
						color = eFTI_Yellow;
				}

				const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(id);
				if (pItem)
				{
					icon = eZSI_Circle;
				}

				HUDUpdateZeusUnitIcon(id, color, icon, Vec3(0, 0, 0));
			}
		}
	}

	// Отрисовка квадрата выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	const auto& color = ColorB(255, 255, 255, 255);
	const auto mode = eBBD_Faceted;
	const auto solid = false;

	for (auto it = m_boxes.cbegin(); it != m_boxes.cend(); it++)
	{
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawOBB(it->second->obb, it->second->wPos, solid, color, mode);
	}

	// Отрисовка отладки
	///////////////////////////////////////////////////////////////////////
	UpdateDebug(zeusMoving, zeus_dyn.v);
}

void CTOSZeusModule::UpdateDebug(bool zeusMoving, const Vec3& zeusDynVec)
{
	IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
	pPD->Begin("ZeusModule", true);
	const auto blue = ColorF(0, 0, 1, 1);
	const auto green = ColorF(0, 1, 0, 1);
	const auto red = ColorF(1, 0, 0, 1);

	// Отрисовка отладки позиции мыши в реальном мире
	///////////////////////////////////////////////////////////////////////
	pPD->AddSphere(m_worldProjectedMousePos, 0.20f, blue, 1.0f);

	if (TOS_Console::GetSafeIntVar("tos_cl_zeus_dragging_draw_debug", 0) > 0 && m_dragging)
	{
		// Отрисовка фактических координат сущности
		const auto pEntity = TOS_GET_ENTITY(m_curClickedEntityId);
		if (pEntity)
		{
			const Vec3 startPos = m_selectStartEntitiesPositions[pEntity->GetId()];
			Vec3 newPos = Vec3(startPos.x + m_draggingDelta.x, startPos.y + m_draggingDelta.y, startPos.z);
			if (m_altModifier)
			{
				newPos.z += m_draggingDelta.z;
			}

			pPD->AddSphere(newPos, 0.20f, red, 1.0f);
			pPD->AddLine(startPos, newPos, red, 1.0f);
		}

		// Отрисовка точки начала выделения сущностей
		///////////////////////////////////////////////////////////////////////
		pPD->AddText(m_mouseIPos.x, m_mouseIPos.y, 1.4f, green, 1.0f, "m_draggingDelta (%1.f, %1.f, %1.f)",
					 m_draggingDelta.x, m_draggingDelta.y, m_draggingDelta.z);
		pPD->AddSphere(m_clickedSelectStartPos, 0.20f, green, 1.0f);
		pPD->AddLine(m_clickedSelectStartPos, m_worldProjectedMousePos, green, 1.0f);
	}

	// Вывод текстовой отладки
	///////////////////////////////////////////////////////////////////////
	float color[] = {1,1,1,1};
	const int startY = 100;
	const int deltaY = 20;

	const auto pLastClickedEntity = TOS_GET_ENTITY(m_lastClickedEntityId);
	const auto pCurrClickedEntity = TOS_GET_ENTITY(m_curClickedEntityId);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 0, 1.3f, color, false, "lastClickedEntity = %s", pLastClickedEntity ? pLastClickedEntity->GetName() : "");
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 1, 1.3f, color, false, "currClickedEntity  = %s", pCurrClickedEntity ? pCurrClickedEntity->GetName() : "");

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 2, 1.3f, color, false, "m_draggingMoveStartTimer = %f", m_draggingMoveStartTimer);

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 3, 1.3f, color, false, "m_mouseIPos = (%i, %i)", m_mouseIPos.x, m_mouseIPos.y);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 4, 1.3f, color, false, "m_mouseDownDurationSec = %1.f", m_mouseDownDurationSec);

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 5, 1.3f, color, false, "m_doubleClick = %i", int(m_doubleClick));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 6, 1.3f, color, false, "m_select = %i", int(m_select));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 7, 1.3f, color, false, "m_dragging = %i", int(m_dragging));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 8, 1.3f, color, false, "m_copying = %i", int(m_copying));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 9, 1.3f, color, false, "m_ctrlModifier = %i", int(m_ctrlModifier));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 10, 1.3f, color, false, "m_altModifier = %i", int(m_altModifier));
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 11, 1.3f, color, false, "m_debugZModifier = %i", int(m_debugZModifier));

	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 12, 1.3f, color, false, "zeusDynVec = (%1.f,%1.f,%1.f)", zeusDynVec.x, zeusDynVec.y, zeusDynVec.z);
	gEnv->pRenderer->Draw2dLabel(100, startY + deltaY * 13, 1.3f, color, false, "zeusMoving = %i", zeusMoving);

	if (!m_selectedEntities.empty())
	{
		TOS_Debug::DrawEntitiesName2DLabel(m_selectedEntities, "Selected Entities: ", 100, startY + deltaY * 14, deltaY);
	}
	if (!m_doubleClickLastSelectedEntities.empty())
	{
		TOS_Debug::DrawEntitiesName2DLabel(m_doubleClickLastSelectedEntities, "DC Selected Entities: ", 300, startY + deltaY * 14, deltaY);
	}

}

void CTOSZeusModule::Serialize(TSerialize ser)
{

}

void CTOSZeusModule::NetMakePlayerZeus(IActor* pPlayer)
{
	if (!pPlayer)
		return;

	if (m_zeus)
		return;

	m_zeus = static_cast<CTOSPlayer*>(pPlayer);
	ApplyZeusProperties(m_zeus);
}

void CTOSZeusModule::ShowHUD(bool show)
{
	const auto pHUD = g_pGame->GetHUD();
	if (pHUD)
	{
		// HP
		pHUD->ShowPlayerStats(show);

		pHUD->ShowRadar(show);
		pHUD->ShowCrosshair(show);
	}
}

void CTOSZeusModule::SetZeusFlag(uint flag, bool value)
{
	m_zeusFlags = value ? (m_zeusFlags | flag) : (m_zeusFlags & ~flag);
}

bool CTOSZeusModule::GetZeusFlag(uint flag) const
{
	return (m_zeusFlags & flag) != 0;
}

void CTOSZeusModule::ShowMouse(bool show)
{
	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		show ? pMouse->IncrementCounter() : pMouse->DecrementCounter();
		pMouse->ConfineCursor(show);
	}

	//auto pHUD = g_pGame->GetHUD();
	//if (pHUD)
	//{
	//	show ? pHUD->CursorIncrementCounter() : pHUD->CursorDecrementCounter();
	//}
}

void CTOSZeusModule::ApplyZeusProperties(IActor* pPlayer)
{
	auto pTOSPlayer = static_cast<CTOSPlayer*>(pPlayer);

	// Сбрасываем статы
	pTOSPlayer->GetActorStats()->inAir = 0.0f;
	pTOSPlayer->GetActorStats()->onGround = 0.0f;

	if (gEnv->bServer)
	{
		// Становимся неуязвимым к урону
		pTOSPlayer->m_isZeus = true;
		pTOSPlayer->GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
	}

	// Отбираем оружие
	IInventory* pInventory = pTOSPlayer->GetInventory();
	if (pInventory)
	{
		pInventory->HolsterItem(true);
		pInventory->RemoveAllItems();
		TOS_Inventory::GiveItem(pTOSPlayer, "NightVision", false, false, false);
	}

	// Скрываем игрока
	if (gEnv->bClient)
		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::SvRequestHideMe(), NetHideMeParams(true), eRMI_ToServer);
	else if (gEnv->bServer)
		pTOSPlayer->GetGameObject()->InvokeRMI(CTOSActor::ClMarkHideMe(), NetHideMeParams(true), eRMI_ToAllClients | eRMI_NoLocalCalls);

	pTOSPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Spectator);

	// Режим полета со столкновениями
	pTOSPlayer->SetFlyMode(1);

	// убираем нанокостюм
	CNanoSuit* pSuit = pTOSPlayer->GetNanoSuit();
	if (pSuit)
	{
		pSuit->SetMode(NANOMODE_DEFENSE);
		pSuit->SetModeDefect(NANOMODE_CLOAK, true);
		pSuit->SetModeDefect(NANOMODE_SPEED, true);
		pSuit->SetModeDefect(NANOMODE_STRENGTH, true);
	}

	if (pTOSPlayer->IsClient())
	{
		// Убираем лишние действия
		g_pGameActions->FilterZeus()->Enable(true);

		// Скрываем HUD
		ShowHUD(false);
	}

	// Становимся невидимым для ИИ
	auto pAI = pTOSPlayer->GetEntity()->GetAI();
	if (pAI)
	{
		TOS_AI::SendEvent(pAI, AIEVENT_DISABLE);
	}

	pTOSPlayer->GetAnimatedCharacter()->ForceRefreshPhysicalColliderMode();
	pTOSPlayer->GetAnimatedCharacter()->RequestPhysicalColliderMode(eColliderMode_Spectator, eColliderModeLayer_Game, "CTOSZeusModule::ApplyZeusProperties");

	//Включаем мышь
	ShowMouse(true);
}

bool CTOSZeusModule::ExecuteCommand(EZeusCommands command)
{
	bool needUpdateIter = true;
	std::set<EntityId> copiedEntities;

	auto it = m_selectedEntities.begin();
	auto end = m_selectedEntities.end();
	while (it != end)
	{
		const EntityId id = *it;
		const int index = TOS_STL::GetIndexFromMapKey(m_selectedEntities, id);

		IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(id);
		//IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);

		switch (command)
		{
			case eZC_KillSelected:
			{
				string hitType = "normal";

				HitInfo info;
				info.SetDamage(99999.0f);
				info.targetId = id;
				info.type = g_pGame->GetGameRules()->GetHitTypeId(hitType.c_str());

				g_pGame->GetGameRules()->ClientHit(info);

				if (pVehicle)
					TOS_Vehicle::Destroy(pVehicle);

				break;
			}
			case eZC_RemoveSelected:
			{
				m_dragging = false;
				m_select = false;
				m_copying = false;

				gEnv->pEntitySystem->RemoveEntity(id);
				it = DeselectEntity(id);
				m_doubleClickLastSelectedEntities.erase(id);

				needUpdateIter = false;

				break;
			}
			case eZC_CopySelected:
			{
				const auto pEntity = TOS_GET_ENTITY(id);
				if (pEntity)
				{
					STOSEntitySpawnParams params;
					//params.vanilla.bStaticEntityId = true;
					params.vanilla.nFlags = ENTITY_FLAG_CASTSHADOW | ENTITY_FLAG_ON_RADAR;
					params.vanilla.pClass = pEntity->GetClass();
					params.vanilla.vPosition = pEntity->GetWorldPos();
					params.vanilla.qRotation = pEntity->GetRotation();

					if (pEntity->GetArchetype())
						params.vanilla.pArchetype = pEntity->GetArchetype();

					const auto pScriptTable = pEntity->GetScriptTable();
					if (pScriptTable)
					{
						SmartScriptTable props;
						if (pScriptTable->GetValue("Properties", props))
						{
							params.vanilla.pPropertiesTable = props;
						}

						SmartScriptTable propsInstance;
						if (pScriptTable->GetValue("PropertiesInstance", propsInstance))
						{
							params.vanilla.pPropertiesInstanceTable = propsInstance;
						}
					}

					const auto pSpawned = TOS_Entity::Spawn(params);
					if (pSpawned)
					{
						pSpawned->Hide(true);

						it = DeselectEntity(id);
						needUpdateIter = false;

						m_dragging = true;
						//m_select = true;

						const auto spawnedId = pSpawned->GetId();
						char buffer[16];
						sprintf(buffer, "%i", spawnedId);
						pSpawned->SetName(string(pSpawned->GetClass()->GetName()) + "_" + buffer);

						m_lastClickedEntityId = id;
						m_curClickedEntityId = spawnedId;
						m_mouseOveredEntityId = spawnedId;

						const auto pos = pSpawned->GetWorldPos();
						m_selectStartEntitiesPositions[spawnedId] = pos;
						m_storedEntitiesPositions[spawnedId] = pos;
						m_clickedSelectStartPos = pos;

						copiedEntities.insert(spawnedId);
					}
				}
				break;
			}
			default:
				break;
		}

		if (needUpdateIter)
			it++;
	}

	if (command == eZC_CopySelected)
	{
		for (auto it = copiedEntities.cbegin(); it != copiedEntities.cend(); it++)
		{
			SelectEntity(*it);
		}
	}

	return true;
}
