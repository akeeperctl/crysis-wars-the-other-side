#include <StdAfx.h>

#include "Game.h"
#include "GameUtils.h"
#include "GameActions.h"

#include "HUD/HUD.h"
#include "HUD/HUDScopes.h"

#include "Player.h"
#include "PlayerInput.h"
#include "Alien.h"
#include "Trooper.h"
#include "Scout.h"
#include "Hunter.h"

#include "Weapon.h"
#include "Single.h"

#include "TheOtherSide/Squad/SquadSystem.h"
#include "TheOtherSide/Abilities/AbilityOwner.h"
#include "TheOtherSide/Abilities/AbilitiesSystem.h"
#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Helpers/TOS_Inventory.h"

void CControlClient::OnAction(const ActionId& action, int activationMode, float value)
{
	assert(m_pControlledActor);
	if (!m_pControlledActor)
		return;

	if (m_pControlledActor->GetHealth() <= 0)
		return;

	//if (m_pAbilitiesSystem)
	//	m_pAbilitiesSystem->OnAction(action, activationMode, value);

	m_actionsMap[action] = activationMode;

	const bool showActionInfo = false;
	if (showActionInfo)
	{
		string mode = "";
		if (activationMode == 1)
			mode = "OnPress";
		else if (activationMode == 2)
			mode = "OnRelease";
		CryLogAlways("[Action Debug] Action %s ActivationMode %s", action.c_str(), mode.c_str());
	}

	const string className = m_pControlledActor->GetEntity()->GetClass()->GetName();
	const CGameActions& rGameActions = g_pGame->Actions();

	if (!s_actionHandler.Dispatch(this, m_pControlledActor->GetEntityId(), action, activationMode, value))
	{
		if (action == rGameActions.crouch)
			OnActionCrouch(className, action, activationMode, value);

		if (action == rGameActions.jump)
			OnActionJump(className, action, activationMode, value);

		if (action == rGameActions.sprint)
			OnActionSprint(className, action, activationMode, value);

		if (action == rGameActions.attack1)
			OnActionAttack(className, action, activationMode, value);

		if (action == rGameActions.zoom)
			OnActionAim(className, action, activationMode, value);

		if (action == rGameActions.nextitem)
		{
			if (className == "Scout")
			{
				OnActionSpeedUp(className, action, activationMode, value);
			}
			else if (className == "Hunter")
			{
				OnActionNextItem(className, action, activationMode, value);
			}
			else
			{
				if (SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
					OnActionZoomIn(className, action, activationMode, value);
			}
		}

		if (action == rGameActions.previtem)
		{
			if (className == "Scout")
			{
				OnActionSpeedDown(className, action, activationMode, value);
			}
			else if (className == "Hunter")
			{
				OnActionPrevItem(className, action, activationMode, value);
			}
			else
			{
				if (SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
					OnActionZoomOut(className, action, activationMode, value);
			}
		}

		if (action == rGameActions.leanleft)
		{
			OnActionLeanLeft(className, action, activationMode, value);
		}
		else if (action == rGameActions.leanright)
		{
			OnActionLeanRight(className, action, activationMode, value);
		}

		//if (action == rGameActions.ctrl_usecenterability)
		//	OnActionCenterAbility(className, action, activationMode, value);

		if (action == rGameActions.binoculars)
			if (className != "Scout")
			{
				OnActionBinoculars(className, action, activationMode, value);
			}

		if (action == rGameActions.use)
			OnActionUse(className, action, activationMode, value);
	}
}

void CControlClient::OnActionBinoculars(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		if (!SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
		{
			SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(true, true));

			if (classname == "Trooper")
				m_trooper.isBinocular = true;

			m_generic.isUsingBinocular = true;
			m_generic.zoomScale = 1;
		}
		else if (SAFE_HUD_FUNC_RET(GetScopes()->IsBinocularsShown()))
		{
			SAFE_HUD_FUNC(GetScopes()->ShowBinoculars(false));

			if (classname == "Trooper")
				m_trooper.isBinocular = false;

			m_generic.isUsingBinocular = false;
			m_generic.zoomScale = 1;
		}
	}
}

void CControlClient::OnActionAttack(string classname, const ActionId& action, int activationMode, float value)
{
	const IInventory* pInventory = m_pControlledActor->GetInventory();
	if (!pInventory)
		return;

	CAlien* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);

	if (m_pControlledActor->IsAlien())
	{
		if (pAlien->GetEMPInfo().isEmpState)
			return;
	}

	if (activationMode == eAAM_OnPress)
		m_generic.isShooting = true;
	else if (activationMode == eAAM_OnRelease)
		m_generic.isShooting = false;

	const EntityId pItemId = pInventory->GetCurrentItem();
	if (pItemId)
	{
		IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pItemId);
		if (pItem)
		{
			IAnimatedCharacter* pAnimatedCharacter = m_pControlledActor->GetAnimatedCharacter();
			IWeapon* pWeapon = pItem->GetIWeapon();
			if (pWeapon && pAnimatedCharacter)
			{
				if (activationMode == eAAM_OnPress)
				{
					if (GetActorClassName() == "Alien")
					{
						if (m_generic.canShoot)
						{
							pWeapon->OnAction(m_pControlledActor->GetEntityId(), action, eAAM_OnPress, value);
							pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 1);
						}
						else
						{
							pWeapon->OnAction(m_pControlledActor->GetEntityId(), action, eAAM_OnRelease, value);
							pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 0);
						}
					}
					else if (GetActorClassName() == "Trooper")
					{
						pWeapon->OnAction(m_pControlledActor->GetEntityId(), action, eAAM_OnPress, value);
					}
					else
					{
						if (m_generic.canShoot)
							pWeapon->OnAction(m_pControlledActor->GetEntityId(), action, eAAM_OnPress, value);
					}
				}
				else if (activationMode == eAAM_OnRelease)
				{
					pWeapon->OnAction(m_pControlledActor->GetEntityId(), action, eAAM_OnRelease, value);
					pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 0);
				}
			}
		}
	}
}

void CControlClient::OnActionLeanRight(string classname, const ActionId& action, int activationMode, float value)
{
	if (m_generic.cannotMove)
		return;

	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	if (classname == "Scout")
	{
		/*const float scoutImpulseForce = 5.f;

		if (activationMode == eAAM_OnPress && m_scout.canDodge && m_pAbilitiesSystem->scout.isGyroEnabled)
		{
			StoreCurrTime();
			m_scout.canDodge = false;

			pe_action_impulse pImpulse;
			pImpulse.impulse += m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(0) * scoutImpulseForce * 100000.0f;
			m_pControlledActor->GetEntity()->GetPhysics()->Action(&pImpulse);

			PlayAnimAction("dodgeRight", false);
			SpawnParticleEffect("alien_special.scout.dodge");
		}*/
	}
	else if (classname == "Trooper")
	{
		if (activationMode == eAAM_OnPress && m_trooper.canDodge && m_pControlledActor->GetActorStats()->onGround > 0)
		{
			//Trooper can't jump when dodge
			m_trooper.canDodge = false;
			m_trooper.canJump = false;

			StoreCurrTime();

			if (m_generic.moveDir.y > 0.0f)
				NetPlayAnimAction("dodgeRightFwd", false);
			else
				NetPlayAnimAction("dodgeRight", false);
		}
	}
}

void CControlClient::OnActionLeanLeft(string classname, const ActionId& action, int activationMode, float value)
{
	if (m_generic.cannotMove)
		return;

	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	if (classname == "Scout")
	{
		//const float scoutImpulseForce = 30.f;

		//if (activationMode == eAAM_OnPress && m_scout.canDodge && m_pAbilitiesSystem->scout.isGyroEnabled)
		//{
		//	StoreCurrTime();
		//	m_scout.canDodge = false;

		//	//pe_action_impulse pImpulse;
		//	pe_action_move pMove;
		//	pMove.iJump = 2;
		//	pMove.dir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(0) * -scoutImpulseForce;//* -100000.0f;
		//	m_pControlledActor->GetEntity()->GetPhysics()->Action(&pMove);

		//	PlayAnimAction("dodgeLeft", false);
		//	SpawnParticleEffect("alien_special.scout.dodge");
		//}
	}
	else if (classname == "Trooper")
	{
		if (activationMode == eAAM_OnPress && m_trooper.canDodge && m_pControlledActor->GetActorStats()->onGround > 0)
		{
			//Trooper can't jump when dodge
			m_trooper.canDodge = false;
			m_trooper.canJump = false;

			StoreCurrTime();
			if (m_generic.moveDir.y > 0.0f)
			{
				NetPlayAnimAction("dodgeLeftFwd", false);
			}
			else
			{
				NetPlayAnimAction("dodgeLeft", false);
			}
		}
	}
}

void CControlClient::OnActionAim(string classname, const ActionId& action, int activationMode, float value)
{
	CAlien* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
	SActorStats* pActorStats = m_pControlledActor->GetActorStats();

	if (classname == "Alien")
	{
		m_generic.isAiming = activationMode == eAAM_OnPress ? 1 : 0;

		//IAnimatedCharacter* pAnimatedCharacter = pAlien->GetAnimatedCharacter();
		//if (pAnimatedCharacter)
		//{
		//	
		//	if (activationMode == eAAM_OnPress)
		//	{
		//		//PlayAnimAction("aim", true);
		//		pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 1);
		//		//m_lookRequest.SetStance(STANCE_STAND);

		//		//m_generic.canShoot = true;
		//		//m_nakedAlien.aimSpeedMult = 0; //mult for set speed and not .zero

		//		
		//	}
		//	else if (activationMode == eAAM_OnRelease)
		//	{
		//		//PlayAnimAction("idle", true);
		//		pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 0);
		//		//m_lookRequest.SetStance(STANCE_PRONE);
		//		//m_generic.canShoot = false;
		//		//m_nakedAlien.aimSpeedMult = 1.0f;

		//		//pAnimatedCharacter->GetAnimationGraphState()->SetInput(pAlien->m_inputAiming, 0);
		//	}
		//}
	}
	else if (classname == "Trooper")
	{
		if (activationMode == eAAM_OnPress)
			m_trooper.isAiming = true;
		else if (activationMode == eAAM_OnRelease)
			m_trooper.isAiming = false;

		if (activationMode == eAAM_OnPress)
		{
			if (m_generic.meleeTargetIsFriend)
				return;

			if (IInventory* pInventory = pAlien->GetInventory())
			{
				if (!m_pAbilitiesSystem->trooper.isCeiling)
				{
					if (m_trooper.canMelee && pAlien->GetActorStats()->onGround && pAlien->GetAlienEnergy() > TROOPER_MELEE_ENERGY_COST)
					{
						if (NetPlayAnimAction("meleeAttack", false))
						{
							m_trooper.canMelee = false;
							StoreCurrTime();

							m_trooper.doMelee = true;
							SubEnergy(TROOPER_MELEE_ENERGY_COST);
						}
					}
					else if (pAlien->GetActorStats()->inAir && m_trooper.canJumpMelee && pAlien->GetAlienEnergy() > TROOPER_MELEE_JUMP_ENERGY_COST)
					{
						//when trooper melee in jump

						pe_action_impulse pImpulse;
						pImpulse.impulse = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(1) * 300.f;

						if (NetPlayAnimAction("meleeJumpAttack", false))
						{
							m_trooper.canJumpMelee = false;
							StoreCurrTime();//store time when click attack key

							NetSpawnParticleEffect("alien_special.Trooper.doubleJumpAttack");

							if (GetCurrentWeapon(pAlien))
								GetCurrentWeapon(pAlien)->MeleeAttack();

							m_trooper.doJumpMelee = true;// this make dont double jump when melee jump attack
							pAlien->GetEntity()->GetPhysics()->Action(&pImpulse);

							SubEnergy(TROOPER_MELEE_JUMP_ENERGY_COST);
							//m_nJumpCount = 0;
						}
					}

					const auto pAbilOwner = g_pControlSystem->GetAbilitiesSystem()->GetAbilityOwner(pAlien->GetEntityId());
					if (pAbilOwner)
					{
						const auto pAbility = pAbilOwner->GetAbility("TrCloak");
						if (pAbility && pAbility->state == eAbilityState_Activated)
							pAbilOwner->ToggleAbility(pAbility->index, 0);
					}
				}
			}
		}
	}
	else if (classname == "Scout")
	{
		if (activationMode == eAAM_OnPress)
		{
			TOS_Inventory::SelectSecondary(pAlien);
		}
		else if (activationMode == eAAM_OnRelease)
		{
			TOS_Inventory::SelectPrimary(pAlien);
		}

		//if (IInventory* pInventory = pAlien->GetInventory())
		//{
		//	static EntityId currentItemId = 0;

		//	if (activationMode == eAAM_OnPress)
		//	{
		//		EntityId singularityItemId = 0;

		//		if (!currentItemId)
		//			currentItemId = pInventory->GetCurrentItem();

		//		if (IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry())
		//		{
		//			pClassRegistry->IteratorMoveFirst();

		//			IEntityClass* pEntityClass = pClassRegistry->FindClass("ScoutSingularity_PLAY");
		//			if (pEntityClass)
		//			{
		//				singularityItemId = pInventory->GetItemByClass(pEntityClass);

		//				if (singularityItemId)
		//					pAlien->SelectItem(singularityItemId, false);
		//			}
		//		}
		//	}
		//	else if (activationMode == eAAM_OnRelease)
		//	{
		//		pAlien->SelectItem(currentItemId, false);
		//		currentItemId = 0;
		//	}
		//}
	}
}

bool CControlClient::OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	ApplyMovement(Vec3(0, value * 2.0f - 1.0f, 0));
	return false;
}

bool CControlClient::OnActionMoveBack(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	ApplyMovement(Vec3(0, -(value * 2.0f - 1.0f), 0));
	return false;
}

bool CControlClient::OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	ApplyMovement(Vec3(-(value * 2.0f - 1.0f), 0, 0));
	return false;
}

bool CControlClient::OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	ApplyMovement(Vec3(value * 2.0f - 1.0f, 0, 0));
	return false;
}

void CControlClient::OnActionCrouch(string classname, const ActionId& action, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

	if (classname == "Scout" || classname == "Alien" || classname == "Drone")
	{
		ApplyMovement(Vec3(0, 0, -(value * 2.0f - 1.0f)));
	}
	else if (classname == "Trooper")
	{
		if (activationMode == eAAM_OnPress)
		{
			m_trooper.speedMult = 0.3f;
		}
		else if (activationMode == eAAM_OnRelease)
		{
			m_trooper.speedMult = 1.0f;
		}
	}
}

void CControlClient::OnActionJump(string classname, const ActionId& action, int activationMode, float value)
{
	if (m_generic.cannotMove)
		return;

	if (m_pControlledActor->IsAlien())
	{
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);

		if (classname == "Scout" || classname == "Alien" || classname == "Drone")
		{
			const CAlien* pAlien = dynamic_cast<CAlien*>(m_pControlledActor);
			if (pAlien->m_empInfo.isEmpState)
				return;

			ApplyMovement(Vec3(0, 0, (value * 2.0f - 1.0f)));
		}
		else if (classname == "Trooper")
		{
			if (!m_trooper.canJump)
				return;

			CTrooper* pTrooper = dynamic_cast<CTrooper*>(m_pControlledActor);
			pe_action_impulse impulse;

			const Vec3& upDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(2);
			const Vec3& forwardDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(1);
			const Vec3& rightDir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(0);

			if (activationMode == eAAM_OnPress)
			{
				// When the controlled trooper is doing a first jump
				if (pTrooper->GetActorStats()->onGround > 0.25f)
				{
					// +1 to jump counter
					m_generic.jumpCount++;

					impulse.impulse.z = upDir.z * 400.f;
					m_pControlledActor->GetEntity()->GetPhysics()->Action(&impulse);

					NetPlayAnimAction("CTRL_JumpStart", false);
				}
				else if (m_generic.jumpCount > 0 && pTrooper->GetActorStats()->inAir && pTrooper->GetAlienEnergy() > TROOPER_JUMP_ENERGY_COST) // Trooper can make a double jump when he in air but not on ceiling
				{
					if (!m_trooper.doJumpMelee)// Trooper can make a double jump if he did not attack after the first jump
					{
						if (m_generic.moveDir.IsZero())
							impulse.impulse = forwardDir * 300.f;
						else
						{
							impulse.impulse += m_generic.moveDir.x * 300.f * rightDir / 1.5f;
							impulse.impulse += m_generic.moveDir.y * 300.f * forwardDir / 1.5f;
						}
						impulse.impulse.z = upDir.z * 250.f;

						NetSpawnParticleEffect("alien_special.Trooper.doubleJumpAttack");

						SubEnergy(TROOPER_JUMP_ENERGY_COST);

						m_pControlledActor->GetEntity()->GetPhysics()->Action(&impulse);
						m_generic.jumpCount = 0;

						//The controlled trooper cannot to do jump attack after double jump
						m_trooper.canJumpMelee = false;
					}
				}
				else
					m_generic.jumpCount = 0;
			}
		}
	}
}

//shift key
void CControlClient::OnActionSprint(string classname, const ActionId& action, int activationMode, float value)
{
	if (m_pControlledActor)
		m_pControlledActor->GetGameObject()->ChangedNetworkState(CAlien::ASPECT_INPUT);


	if (classname == "Scout" || classname == "Drone")
	{
		const float scoutImpulseForce = 30.f;
		const bool isMoving = m_generic.moveDir.x || m_generic.moveDir.y || m_generic.moveDir.z;
		const bool can = m_scout.canDodge &&
			m_pAbilitiesSystem->scout.isGyroEnabled && 
			isMoving &&
			m_pControlledActor->GetGrabStats()->grabbedIds.size() == 0;

		if (activationMode == eAAM_OnPress && can)
		{
			StoreCurrTime();
			m_scout.canDodge = false;

			//pe_action_impulse pImpulse;
			pe_action_move pMove;
			pMove.iJump = 2;
			//pMove.dir = m_pControlledActor->GetEntity()->GetWorldTM().GetColumn(0) * -scoutImpulseForce;//* -100000.0f;
			//m_pControlledActor->GetEntity()->GetPhysics()->Action(&pMove);

			//углы поворота добываются в функциях GetAngles в виде Radian. 
			//Для удобства работы с углами, специально для юзера они преобразуются в градусы/Degrees (0-360)  

			const Ang3 angles(m_pControlledActor->GetAngles());
			const Matrix33 baseMtx = Matrix33::CreateRotationXYZ(angles);

			const Vec3 goal = baseMtx * m_generic.moveDir;

			pMove.dir = goal * scoutImpulseForce;
			m_pControlledActor->GetEntity()->GetPhysics()->Action(&pMove, 1);

			string dodgeAnim = "";
			if (pMove.dir.x > 0.0f)
				dodgeAnim = "dodgeLeft";
			else
				dodgeAnim = "dodgeRight";

			NetPlayAnimAction(dodgeAnim.c_str(), false);
			NetSpawnParticleEffect("alien_special.scout.dodge");
		}
	}
	else if (classname == "Trooper")
	{
		const auto pTrooper = dynamic_cast<CTrooper*>(m_pControlledActor);
		const auto noEnergy = pTrooper->GetAlienEnergy() <= ALIEN_MAX_ENERGY * 0.2;

		if (activationMode == eAAM_OnPress && !noEnergy)
		{
			m_trooper.isSprinting = true;
			m_trooper.sprintMult = 2.0f;
		}
		else if(activationMode == eAAM_OnRelease)
		{
			m_trooper.isSprinting = false;
			m_trooper.sprintMult = 1.0f;
		}

		pTrooper->GetAlienStats()->isSprintig = m_trooper.isSprinting;
		//pTrooper->m_energyParams.cannotRegen = m_trooper.isSprinting;
	}
		//m_trooper.speedMult = 0.3f;
	//else if (classname == "Alien")
		//m_nakedAlien.speedMult = 0.3f;		

	//else if (activationMode == eAAM_OnRelease)
	//{
	//	m_nakedAlien.speedMult = 1.0f;
	//	m_trooper.speedMult = 1.0f;
	//}
}

void CControlClient::OnActionSpeedUp(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		m_scout.speedMult += 0.2f;
		m_scout.speedMult = clamp_tpl(m_scout.speedMult, 0.2f, 1.0f);
	}
}

void CControlClient::OnActionSpeedDown(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		m_scout.speedMult -= 0.2f;
		m_scout.speedMult = clamp_tpl(m_scout.speedMult, 0.2f, 1.0f);
	}
}

void CControlClient::OnActionZoomIn(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		switch (m_generic.zoomScale)
		{
		case 1:
			m_generic.zoomScale++;
			m_currentFov = m_currentFov / m_generic.zoomScale;
			break;
		case 2:
			m_generic.zoomScale++;
			m_currentFov = m_currentFov / m_generic.zoomScale;
			break;
		case 3:
			break;
		}
	}
}

void CControlClient::OnActionZoomOut(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		switch (m_generic.zoomScale)
		{
		case 1:
			break;
		case 2:
			m_currentFov = m_currentFov * m_generic.zoomScale;
			m_generic.zoomScale--;
			break;
		case 3:
			m_currentFov = m_currentFov * m_generic.zoomScale;
			m_generic.zoomScale--;
			break;
		}
	}
}

void CControlClient::OnActionNextItem(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		if (classname == "Trooper" && m_pAbilitiesSystem->trooper.doCeiling)
			return;

		m_pControlledActor->SelectNextItem(1, true);
	}
}

void CControlClient::OnActionPrevItem(string classname, const ActionId& action, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
	{
		if (classname == "Trooper" && m_pAbilitiesSystem->trooper.doCeiling)
			return;

		m_pControlledActor->SelectNextItem(-1, true);
	}
}

bool CControlClient::OnActionViewLock(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
	if (activationMode == eAAM_OnPress)
		m_generic.cannotLookAt = true;
	else if (activationMode == eAAM_OnRelease)
		m_generic.cannotLookAt = false;
	return false;
}

void CControlClient::OnActionUse(string classname, const ActionId& action, int activationMode, float value)
{
	if (const IEntity* pTarget = GetMeleeTarget())
	{
		const IVehicle* pVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pTarget->GetId());
		const IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pTarget->GetId());

		if (const IActor* pClient = g_pGame->GetIGameFramework()->GetClientActor())
		{
			if (!pVehicle && !pItem)
			{
				IScriptTable* pTable = pClient->GetEntity()->GetScriptTable();
				IScriptTable* pTargetTable = pTarget->GetScriptTable();
				if (pTable && pTargetTable)
				{
					bool result = false;

					HSCRIPTFUNCTION IsUsable = 0;
					if (pTargetTable->GetValueType("IsUsable") == svtFunction &&
						pTargetTable->GetValue("IsUsable", IsUsable))
					{
						Script::CallReturn(gEnv->pScriptSystem, IsUsable, pTargetTable, result);
						gEnv->pScriptSystem->ReleaseFunc(IsUsable);
						//CryLogAlways("%s is Usable %1.f", pTarget->GetName());
					}

					const float fDist = (pTarget->GetWorldPos() - m_pControlledActor->GetEntity()->GetWorldPos()).GetLength();
					if (fDist <= 3.f && result)
					{
						Script::CallMethod(pTable, "UseEntity", pTarget->GetId(), 1, activationMode == eAAM_OnPress);
						//CryLogAlways("Use entity %s", (string)pTarget->GetName());
					}
				}
			}
		}
	}
}