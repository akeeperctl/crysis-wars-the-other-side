/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "IFlashPlayer.h"
#include "HUD/HUD.h"
#include "HUD/HUDSilhouettes.h"
#include "GameActions.h"
#include "ZeusModule.h"
#include "ZeusSynchronizer.h"
#include <TheOtherSideMP\HUD\TOSCrosshair.h>
#include <TheOtherSideMP\Helpers\TOS_AI.h>
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_NET.h>
#include <TheOtherSideMP\Helpers\TOS_Inventory.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>
#include <TheOtherSideMP\Helpers\TOS_Vehicle.h>
#include <TheOtherSideMP\Helpers\TOS_STL.h>
#include <TheOtherSideMP\Helpers\TOS_Screen.h>
#include <Cry_Camera.h>

std::map<string, string> CTOSZeusModule::s_classToConsoleVar;

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

	auto pFirstActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(pFirstEntity->GetId()));
	auto pSecondActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(pSecondEntity->GetId()));
	if (pFirstActor && pSecondActor)
	{
		auto pFirstVehicle = pFirstActor->GetLinkedVehicle();
		auto pSecondVehicle = pSecondActor->GetLinkedVehicle();
		if (pFirstVehicle != pSecondVehicle)
			return false;
	}

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

CTOSZeusModule::CTOSZeusModule()
	:
	m_pPersistantDebug(nullptr),
	m_pZeusScriptBind(nullptr),

	m_network(this),
	m_local(this),
	m_hud(this)
{}

CTOSZeusModule::~CTOSZeusModule()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->RemoveListener(this);
}

void CTOSZeusModule::Reset()
{
	m_local.Reset();
	m_hud.Reset();

	auto pPlayer = GetPlayer();
	if (pPlayer)
		pPlayer->SetMeZeus(false);
}

void CTOSZeusModule::Init()
{
	if (gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->AddListener(this);

	InitSelectionFilterClasses();
	Reset();

	m_pPersistantDebug = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
}

void CTOSZeusModule::InitScriptBinds()
{
	m_pZeusScriptBind = new CScriptBind_Zeus(gEnv->pSystem, g_pGame->GetIGameFramework());
}

void CTOSZeusModule::ReleaseScriptBinds()
{
	SAFE_DELETE(m_pZeusScriptBind);
}

bool CTOSZeusModule::OnInputEvent(const SInputEvent& event)
{
	if (!gEnv->bClient)
		return false;

	if (!GetPlayer() || !m_local.GetFlag(EFlag::Zeusing))
		return false;

	if (event.deviceId == EDeviceId::eDI_Keyboard)
	{
		if (event.keyId == EKeyId::eKI_LCtrl)
		{
			if (event.state == eIS_Pressed)
				m_local.m_ctrlModifier = true;
			else if (event.state == eIS_Released)
				m_local.m_ctrlModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_C)
		{
			if (event.state == eIS_Pressed)
			{
				if (m_local.m_ctrlModifier && !m_local.m_copying)
				{
					m_local.m_copying = true;
					m_local.ExecuteCommand(ECommand::CopySelected);
				}
			}
		}
		else if (event.keyId == EKeyId::eKI_LAlt)
		{
			if (event.state == eIS_Pressed)
			{
				m_local.m_altModifier = true;
				m_local.m_select = false;
			}
			else if (event.state == eIS_Released)
			{
				if (m_local.m_altModifier)
				{
					for (auto it = m_local.m_selectedEntities.cbegin(); it != m_local.m_selectedEntities.cend(); it++)
					{
						const bool movedOnHeight = m_local.m_draggingDelta.len() > 1;

						auto pVehicle = TOS_GET_VEHICLE(*it);
						if (pVehicle && TOS_Vehicle::IsAir(pVehicle) && movedOnHeight)
						{
							SVehicleMovementEventParams params;
							params.fValue = pVehicle->GetEntity()->GetWorldPos().z; // желаемая высота

							TOS_Vehicle::BroadcastMovementEvent(pVehicle, IVehicleMovement::eVME_WarmUpEngine, params);
						}
					}

				}

				m_local.m_altModifier = false;
			}
		}
		else if (event.keyId == EKeyId::eKI_Z)
		{
			if (event.state == eIS_Pressed)
				m_local.m_debugZModifier = true;
			else if (event.state == eIS_Released)
			{
				if (m_local.m_debugZModifier)
				{
					m_local.m_debugZModifier = false;

					for (auto it = m_local.m_selectedEntities.cbegin(); it != m_local.m_selectedEntities.cend(); it++)
					{
						if (!m_local.SelectionFilter(*it))
							it = m_local.DeselectEntity(*it);
					}
				}
			}
		}
		else if (event.keyId == EKeyId::eKI_LShift)
		{
			if (event.state == eIS_Pressed)
				m_local.m_shiftModifier = true;
			else if (event.state == eIS_Released)
				m_local.m_shiftModifier = false;
		}
		else if (event.keyId == EKeyId::eKI_End)
		{
			m_local.ExecuteCommand(ECommand::KillSelected);
		}
		else if (event.keyId == EKeyId::eKI_Delete)
		{
			m_local.ExecuteCommand(ECommand::RemoveSelected);
		}
		else if (event.keyId == EKeyId::eKI_P)
		{
			if (event.state == eAAM_OnPress)
			{
				m_hud.ShowZeusMenu(!m_hud.m_menuShow);
			}
		}
		else if (event.keyId == EKeyId::eKI_Space)
		{
			if (event.state == eAAM_OnPress)
			{
				m_local.m_spaceFreeCam = !m_local.m_spaceFreeCam;
				m_local.SetFlag(EFlag::CanRotateCamera, m_local.m_spaceFreeCam);
				m_local.ShowMouse(!m_local.m_spaceFreeCam);
			}
		}
		else if (event.keyId == EKeyId::eKI_X)
		{
			if (event.state == EInputState::eIS_Pressed)
			{
				auto it = m_local.m_selectedEntities.cbegin();
				auto end = m_local.m_selectedEntities.cend();
				for (; it != end; it++)
					m_local.StopOrder(*it);
			}
		}
	}
	else if (event.deviceId == EDeviceId::eDI_Mouse)
	{
		if (m_local.GetFlag(EFlag::CanUseMouse) == false)
			return false;

		if (event.keyId == EKeyId::eKI_Mouse3) // Средняя кнопка мыши
		{
			if (m_local.m_spaceFreeCam == false)
				m_local.SetFlag(EFlag::CanRotateCamera, event.state == eIS_Down);
		}
		else if (event.keyId == EKeyId::eKI_Mouse2) // Правая кнопка мыши
		{
			if (event.state == EInputState::eIS_Pressed)
			{
				m_local.ExecuteCommand(ECommand::OrderSelected);
			}
		}
	}

	return true;
}

bool CTOSZeusModule::OnInputEventUI(const SInputEvent& event)
{
	return false;
}

void CTOSZeusModule::OnHardwareMouseEvent(int iX, int iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent)
{
	m_local.m_mouseIPos.x = iX;
	m_local.m_mouseIPos.y = iY;

	auto mod_iY = gEnv->pRenderer->GetHeight() - iY;
	gEnv->pRenderer->UnProjectFromScreen(iX, mod_iY, 0.0f, &m_local.m_worldMousePos.x, &m_local.m_worldMousePos.y, &m_local.m_worldMousePos.z);

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

		if (m_hud.m_animZeusUnitIcons.IsLoaded())
		{
			int x(iX), y(iY);
			m_hud.m_animZeusUnitIcons.GetFlashPlayer()->ScreenToClient(x, y);
			m_hud.m_animZeusUnitIcons.GetFlashPlayer()->SendCursorEvent(SFlashCursorEvent(eCursorState, x, y));
		}

		if (m_hud.m_animZeusMenu.IsLoaded())
		{
			int x(iX), y(iY);
			m_hud.m_animZeusMenu.GetFlashPlayer()->ScreenToClient(x, y);
			m_hud.m_animZeusMenu.GetFlashPlayer()->SendCursorEvent(SFlashCursorEvent(eCursorState, x, y));
		}

		if (!pHUD->IsHaveModalHUD() && m_local.GetFlag(EFlag::CanUseMouse))
		{
			if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOUBLECLICK)
			{
				m_local.m_doubleClick = true;
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONDOWN)
			{
				if (!m_local.m_spaceFreeCam)
				{
					m_local.m_select = true;
					m_local.m_selectStartPos = Vec2i(iX, iY);

					if (m_hud.m_menuSpawnHandling == false)
					{
						m_local.HandleOnceSelection(m_local.GetMouseEntityId());
					}
				}
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_LBUTTONUP)
			{
				m_local.m_select = false;
				m_local.m_selectStopPos = Vec2i(iX, iY);

				if (!m_local.m_dragging && m_local.CanSelectMultiplyWithBox())
					m_local.GetSelectedEntities();
				else
				{
					// Множественное выделение c зажатым модификатором
					if (m_local.m_ctrlModifier)
					{
						if (!m_local.m_dragging && !m_local.m_doubleClick && m_local.m_curClickedEntityId != 0)
						{
							if (m_local.m_selectedEntities.count(m_local.m_curClickedEntityId) > 0)
							{
								//if (delta <= 0.15f)
								m_local.DeselectEntity(m_local.m_curClickedEntityId);
							}
							else
							{
								if (m_local.SelectionFilter(m_local.m_curClickedEntityId))
									m_local.SelectEntity(m_local.m_curClickedEntityId);
							}
						}
					}
				}

				if (m_local.m_doubleClick)
				{
					const auto pClickedEntity = TOS_GET_ENTITY(m_local.m_curClickedEntityId);
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

						const auto clickedIter = stl::binary_find(m_local.m_doubleClickLastSelectedEntities.cbegin(), m_local.m_doubleClickLastSelectedEntities.cend(), m_local.m_curClickedEntityId);
						const bool clickedSelected = clickedIter != m_local.m_doubleClickLastSelectedEntities.cend();
						if (clickedSelected)
						{
							// Снимаем выделение последних выделенных подобных сущностей
							for (auto it = m_local.m_doubleClickLastSelectedEntities.begin(); it != m_local.m_doubleClickLastSelectedEntities.end();)
							{
								auto pEntity = TOS_GET_ENTITY(*it);

								if (EntityIsSimilarToEntity(pEntity, pClickedEntity) || pEntity == pClickedEntity)
								{
									m_local.DeselectEntity(*it);
									it = m_local.m_doubleClickLastSelectedEntities.erase(it);
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

									if (!m_local.SelectionFilter(id))
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

									m_local.SelectEntity(id);
									m_local.m_doubleClickLastSelectedEntities.insert(id);
								}
							}
						}
					}



					m_local.m_doubleClick = false;
				}

				// Перед тем как закончить копирование..
				// Показываем все скопированные сущности
				if (m_local.m_copying)
				{
					for (auto it = m_local.m_selectedEntities.cbegin(); it != m_local.m_selectedEntities.cend(); it++)
					{
						auto pEntity = TOS_GET_ENTITY(*it);
						if (pEntity)
						{
							pEntity->Hide(false);

							bool hostile = true;
							TOS_Script::GetEntityProperty(pEntity, "bSpeciesHostility", hostile);
							TOS_AI::MakeHostile(pEntity->GetAI(), hostile);
						}
					}
				}
				m_local.m_copying = false;

				if (m_local.m_dragging)
				{
					for (auto it = m_local.m_selectedEntities.begin(); it != m_local.m_selectedEntities.end();)
					{
						const EntityId selectedEntId = *it;
						// Сущность, которую перенаскивают
						auto pSelectedEntity = TOS_GET_ENTITY(selectedEntId);
						if (!pSelectedEntity)
						{
							it++;
							continue;
						}

						bool moveSelectedEnt = true;
						bool needDeselect = false;

						// сущность, на которую перетаскивают 
						const auto pDragTarget = TOS_GET_ENTITY(m_local.m_dragTargetId);
						if (pDragTarget)
						{
							auto pSelectedActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(selectedEntId));
							auto pSelectedItem = static_cast<CItem*>(TOS_GET_ITEM(selectedEntId));

							const EntityId dragTargetId = pDragTarget->GetId();

							if (pSelectedActor)
							{
								// Actor перетаскивают на Vehicle
								IVehicle* pDragVehicle = TOS_GET_VEHICLE(dragTargetId);
								if (pDragVehicle && TOS_Vehicle::Enter(pSelectedActor, pDragVehicle, true))
								{
									moveSelectedEnt = false;
									needDeselect = true;
								}
							}
							else if (pSelectedItem)
							{
								// Item перетаскивают на Actor
								auto pDragActor = static_cast<CTOSActor*>(TOS_GET_ACTOR(dragTargetId));
								if (pDragActor && pDragActor->GetHealth() > 0)
								{
									if (pDragActor->PickUpItem(pSelectedItem->GetEntityId(), true))
									{
										TOS_Inventory::SelectItemByClass(pDragActor, pSelectedItem->GetEntity()->GetClass()->GetName());
										moveSelectedEnt = false;
										needDeselect = true;
									}
								}

							}
						}

						// Пинаем физику выделенных сущностей после того как закончили их перетаскивать
						auto pPhys = pSelectedEntity->GetPhysics();
						if (pPhys)
						{
							pe_action_awake awake;
							awake.bAwake = 1;
							pPhys->Action(&awake);
						}

						if (moveSelectedEnt)
						{
							// Применяем сдвинутые позиции боксов на сущности
							const auto& pBox = m_local.m_boxes[pSelectedEntity->GetId()];
							pSelectedEntity->SetWorldTM(Matrix34::CreateTranslationMat(pBox->wPos));
							pSelectedEntity->SetRotation(Quat(pBox->obb.m33));
						}

						if (needDeselect)
						{
							it = m_local.DeselectEntity(pSelectedEntity->GetId());
							//deselectionSet.insert(pSelectedEntity->GetId());
						}
						else
						{
							it++;
						}
					}
				}

				m_local.m_dragTargetId = 0;
				m_local.m_dragging = false;

				if (m_hud.m_menuSpawnHandling)
				{
					if (m_local.m_curClickedEntityId != 0)
					{
						auto pEntity = TOS_GET_ENTITY(m_local.m_curClickedEntityId);
						if (pEntity)
						{
							pEntity->Hide(false);
						}
						else
						{
							m_local.m_curClickedEntityId = 0;
						}
					}
				}
				m_hud.m_menuSpawnHandling = false;
			}
			else if (eHardwareMouseEvent == HARDWAREMOUSEEVENT_MOVE)
			{
				// При перетаскивании кликнутая сущность должна быть выделена
				const auto clickedIter = stl::binary_find(m_local.m_selectedEntities.begin(), m_local.m_selectedEntities.end(), m_local.m_curClickedEntityId);
				const bool clickedSelected = clickedIter != m_local.m_selectedEntities.end();

				// Мышь находится в диапазоне иконки кликнутой сущности. True - да
				const bool clickedOveredByMouse = m_local.m_mouseOveredEntityId == m_local.m_curClickedEntityId;

				if ((m_local.m_select) && m_local.m_curClickedEntityId != 0 && clickedSelected && clickedOveredByMouse)
				{
					// Перед тем как начать перемещение сущностей...
					if (m_local.m_dragging == false)
					{
						// Сохраняем начальное положение каждой выделенной сущности
						m_local.SaveEntitiesStartPositions();

						// Запуск таймера 
						m_local.m_draggingMoveStartTimer = TOS_Console::GetSafeFloatVar("tos_sv_zeus_dragging_move_start_delay", 0.05f);
					}

					m_local.m_dragging = true;
				}
			}

		}
	}
}

void CTOSZeusModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	auto pHUD = g_pGame->GetHUD();
	auto pPlayer = GetPlayer();

	const bool bNoModalOrNoHUD = !pHUD || (pHUD && !pHUD->IsHaveModalHUD());
	const bool bZeusing = m_local.GetFlag(EFlag::Zeusing);

	TOS_INIT_EVENT_VALUES(pEntity, event);

	switch (event.event)
	{
	case eEGE_ActorRevived:
	{
		if (!bZeusing)
			return;

		if (pPlayer && pPlayer->GetEntityId() == pEntity->GetId())
		{
			m_local.Reset();
			if (bNoModalOrNoHUD)
				m_local.ShowMouse(false);
			m_hud.ShowZeusMenu(false);
		}
		break;
	}
	case eEGE_MasterClientOnStartControl:
	{
		if (!bZeusing)
			return;

		if (pPlayer)
		{
			m_hud.ShowPlayerHUD(true);
			if (bNoModalOrNoHUD)
				m_local.ShowMouse(false);

			m_local.SetFlag(EFlag::Possessing, true);
		}
		break;
	}
	case eEGE_MasterClientOnStopControl:
	{
		if (!bZeusing)
			return;

		if (pPlayer)
		{
			m_network.MakeZeus(pPlayer, true);
			m_local.SetFlag(EFlag::Possessing, false);
		}
		break;
	}
	case eEGE_HUDInit:
	{
		m_hud.Init();
		break;
	}
	case eEGE_HUDInGamePostUpdate:
	{
		m_hud.InGamePostUpdate(event.value);
		break;
	}
	case eEGE_HUDUnloadSimpleAssets:
	{
		m_hud.UnloadSimpleAssets(event.int_value);
		break;
	}
	case eEGE_EditorGameExit:
	{
		if (!bZeusing)
			return;

		if (pPlayer)
		{
			if (m_local.m_mouseDisplayed == false)
			{
				m_local.ShowMouse(true);
			}

			auto it = m_local.m_orders.cbegin();
			auto end = m_local.m_orders.cend();
			for (; it != end; it++)
			{
				m_local.StopOrder(it->first);
			}
		}

		gEnv->pScriptSystem->ResetTimers();
		break;
	}
	case eEGE_ActorEnterVehicle:
	{
		if (!bZeusing)
			return;

		if (pPlayer && m_local.IsSelectedEntity(pEntity->GetId()))
		{
			const auto pVehEntity = TOS_GET_ENTITY(event.int_value);
			if (pVehEntity)
			{
				if (!m_local.m_dragging)
					m_local.DeselectEntity(pEntity->GetId());

				m_local.SelectEntity(pVehEntity->GetId());
			}
		}

		break;
	}
	case eEGE_ActorDead:
	case eEGE_VehicleDestroyed:
	{
		if (!bZeusing)
			return;

		m_local.RemoveOrder(pEntity->GetId());

		auto orderIt = m_local.m_orders.begin();
		auto orderEnd = m_local.m_orders.end();
		for (; orderIt != orderEnd; orderIt++)
		{
			if (orderIt->second.targetId == pEntity->GetId())
			{
				m_local.StopOrder(orderIt->first);
			}
		}
		break;
	}
	case eEGE_SynchronizerCreated:
	{
		if (pGO)
		{
			RegisterSynchronizer(static_cast<CTOSZeusSynchronizer*>(pGO->AcquireExtension("TOSZeusSynchronizer")));
			assert(GetSynchronizer() != nullptr);
		}

		TOS_RECORD_EVENT(entId, STOSGameEvent(eEGE_SynchronizerRegistered, "For Zeus Module", true));

		break;
	}
	default:
		break;
	}
}

void CTOSZeusModule::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);
	//s->AddContainer(m_selectedEntities);
	//s->AddContainer(m_doubleClickLastSelectedEntities);
	//s->AddContainer(m_selectStartEntitiesPositions);
	//s->AddContainer(m_storedEntitiesPositions);
	//s->AddContainer(m_boxes);
}

const char* CTOSZeusModule::GetName()
{
	return "ModuleZeus";
}

void CTOSZeusModule::Update(float frametime)
{
	if (tos_sv_zeus_update == 0)
		return;

	// TODO: 01.12.2024 здесь говно, вылетает при спавне транспортаw
	if (!GetPlayer() || !m_local.GetFlag(EFlag::Zeusing))
		return;

	auto pMouse = gEnv->pHardwareMouse;
	if (pMouse)
	{
		// Привязка мыши к позиции, когда крутится камера
		///////////////////////////////////////////////////////////////////////
		if (m_local.GetFlag(EFlag::CanRotateCamera) && !(m_local.GetFlag(EFlag::Possessing)))
		{
			if (m_local.m_anchoredMousePos == Vec2(0, 0))
			{
				Vec2 mousePos;
				pMouse->GetHardwareMousePosition(&mousePos.x, &mousePos.y);

				m_local.m_anchoredMousePos = mousePos;
			}

			pMouse->SetHardwareMousePosition(m_local.m_anchoredMousePos.x, m_local.m_anchoredMousePos.y);
		}
		else
		{
			if (m_local.m_anchoredMousePos != Vec2(0, 0))
				m_local.m_anchoredMousePos.zero();
		}
	}

	if (m_local.m_select)
		m_local.m_mouseDownDurationSec += frametime;
	else
		m_local.m_mouseDownDurationSec = 0.0f;

	m_local.MouseProjectToWorld(m_local.m_mouseRay, m_local.m_worldMousePos, m_local.m_mouseRayEntityFlags, true);
	m_local.m_worldProjectedMousePos = m_local.m_mouseRay.pt;

	// Обработка таймера задержки перемещения выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_local.m_dragging && m_local.m_draggingMoveStartTimer > 0.0f)
		m_local.m_draggingMoveStartTimer -= frametime;

	if (m_local.m_draggingMoveStartTimer <= 0.0f)
		m_local.m_draggingMoveStartTimer = 0.0f;

	pe_status_dynamics zeus_dyn;
	const auto pZeusPhys = GetPlayer()->GetEntity()->GetPhysics();
	if (pZeusPhys)
		pZeusPhys->GetStatus(&zeus_dyn);

	const bool zeusMoving = zeus_dyn.v.len() > 0.1f;

	//Перемещение боксов выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	if (m_local.m_dragging && !zeusMoving && m_local.m_draggingMoveStartTimer == 0.0f)
	{
		const bool autoEntitiesHeight = TOS_Console::GetSafeIntVar("tos_sv_zeus_dragging_entities_auto_height", 0) == 1; // Расчет высоты для каждой сущности отдельно
		const auto pClickedEntity = TOS_GET_ENTITY(m_local.m_curClickedEntityId);

		m_local.m_mouseRayEntityFlags = ZEUS_DRAGGING_MOUSE_ENT_FLAGS;
		m_local.m_draggingDelta = m_local.m_worldProjectedMousePos - m_local.m_clickedSelectStartPos;

		for (auto it = m_local.m_selectedEntities.cbegin(); it != m_local.m_selectedEntities.cend(); it++)
		{
			const auto id = *it;
			// TODO добавить отображение линии и позиции текущего приказа выделенной сущности
			// если приказа нет, то ничего не выводить
			// TODO сделать дублирование иконки приказа, по аналогии с иконками сущностей

			if (!m_local.UpdateDraggedEntity(id, pClickedEntity, pZeusPhys, m_local.m_boxes, autoEntitiesHeight))
				continue;
		}

		m_local.m_dragTargetId = m_local.GetMouseEntityId();
		if (m_local.m_dragTargetId == m_local.m_curClickedEntityId)
			m_local.m_dragTargetId = 0;
	}
	else if (!m_local.m_dragging)
	{

		for (auto it = m_local.m_selectedEntities.cbegin(); it != m_local.m_selectedEntities.cend(); it++)
		{
			const EntityId id = *it;
			const IEntity* pEntity = TOS_GET_ENTITY(id);
			if (pEntity)
			{
				m_local.m_boxes[id]->wPos = pEntity->GetWorldPos();
				m_local.m_boxes[id]->obb.m33 = Matrix33(pEntity->GetRotation());
			}
		}
	}
	else
	{
		m_local.m_mouseRayEntityFlags = ZEUS_DEFAULT_MOUSE_ENT_FLAGS;
	}

	// Отрисовка границ выделения
	///////////////////////////////////////////////////////////////////////
	if (m_local.m_select && m_local.CanSelectMultiplyWithBox() && !m_local.m_dragging)
	{
		if (IRenderAuxGeom* pGeom = gEnv->pRenderer->GetIRenderAuxGeom())
		{
			//calculate the four selection boundary points
			Vec3 vTopLeft(m_local.m_selectStartPos.x, m_local.m_selectStartPos.y, 0.0f);
			Vec3 vTopRight(m_local.m_mouseIPos.x, m_local.m_selectStartPos.y, 0.0f);
			Vec3 vBottomLeft(m_local.m_selectStartPos.x, m_local.m_mouseIPos.y, 0.0f);
			Vec3 vBottomRight(m_local.m_mouseIPos.x, m_local.m_mouseIPos.y, 0.0f);

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
	IActor* pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pClientActor)
		return;

	m_local.UpdateUnitIcons(pClientActor);
	m_local.UpdateOrderIcons();

	// Отрисовка квадрата выделенных сущностей
	///////////////////////////////////////////////////////////////////////
	const auto& color = ColorB(255, 255, 255, 255);
	const auto mode = eBBD_Faceted;
	const auto solid = false;

	for (auto it = m_local.m_boxes.cbegin(); it != m_local.m_boxes.cend(); it++)
	{
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawOBB(it->second->obb, it->second->wPos, solid, color, mode);
	}

	// Отрисовка отладки
	///////////////////////////////////////////////////////////////////////
	m_local.UpdateDebug(zeusMoving, zeus_dyn.v);
}

void CTOSZeusModule::Serialize(TSerialize ser)
{

}

int CTOSZeusModule::GetDebugLog()
{
	return m_debugLogMode;
}

bool CTOSZeusModule::IsPhysicsAllowed(const IEntity* pEntity)
{
	if (!pEntity)
		return false;

	IPhysicalEntity* physEnt = pEntity->GetPhysics();
	if (!physEnt)
		return false;

	// допустимые сущности
	const auto type = physEnt->GetType();
	if (!(type == PE_LIVING || type == PE_RIGID || type == PE_STATIC || type == PE_WHEELEDVEHICLE || type == PE_ARTICULATED))
		return false;
}

CTOSPlayer* CTOSZeusModule::GetPlayer() const
{
	return static_cast<CTOSPlayer*>(TOS_GET_CLIENT_ACTOR);
}

CTOSZeusModule::Network& CTOSZeusModule::GetNetwork()
{
	return m_network;
}

CTOSZeusModule::Local& CTOSZeusModule::GetLocal()
{
	return m_local;
}

CTOSZeusModule::HUD& CTOSZeusModule::GetHUD()
{
	return m_hud;
}
