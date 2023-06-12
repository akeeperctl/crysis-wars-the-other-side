#include "StdAfx.h"

#include "TheOtherSide/Control/ControlSystem.h"
#include "TheOtherSide/Squad/SquadSystem.h"

#include "Player.h"
#include "Scout.h"

#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"

#include "Single.h"

//Air squad mate defines
#define SAFE_FLY_DISTANCE 15.f
#define SAFE_FLY_DISTANCE_HUNTER 25.f

//AI defines
#define AI_CLEAN_START_3SEC 300 //300 = 3 seconds
#define AI_CLEAN_START_HALFSEC 50 //50 = 0.5 second
#define AI_CLEAN_FINISH 0

SSquad::SSquad()
{
	m_eLeaderCurrentOrder = eSquadOrders_GoTo;
	m_squadMembers.clear();
	m_squadSelectedMembers.clear();
	m_searchRadius = 20;
	m_pLeader = 0;
	m_squadId = -1;
	m_pSquadSystem = g_pControlSystem->pSquadSystem;
};

SSquad::SSquad(IActor* _Leader, uint _squadId)
{
	m_eLeaderCurrentOrder = eSquadOrders_GoTo;
	m_squadMembers.clear();
	m_squadSelectedMembers.clear();
	m_searchRadius = 20;
	m_pLeader = _Leader;
	m_squadId = _squadId;
	m_pSquadSystem = g_pControlSystem->pSquadSystem;
};

SSquad::~SSquad()
{
	m_eLeaderCurrentOrder = eSquadOrders_None;
	m_squadId = -1;
	m_pLeader = 0;
	m_squadMembers.clear();
	m_squadSelectedMembers.clear();
	m_pSquadSystem = 0;
};

void SSquad::AddMemberToSelected(const SMember& member)
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	//SMember member = GetMemberFromIndex(index);
	if (!GetActor(member.actorId))
		return;

	//We cannot select the player
	if (GetActor(member.actorId) == pPlayer)
		return;

	TSMembers::const_iterator it = m_squadSelectedMembers.begin();
	TSMembers::const_iterator end = m_squadSelectedMembers.end();

	bool finded = false;
	for (; it != end; it++)
	{
		if (*it == member.actorId)
		{
			finded = true;
			break;
		}
	}

	if (!finded)
		m_squadSelectedMembers.push_back(member.actorId);
	else
		CryLogAlways("[SSquad::AddMemberToSelected] try to add already selected member");
}

bool SSquad::isPlayerMember() const
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pPlayer)
	{
		TMembers::const_iterator it = m_squadMembers.begin();
		TMembers::const_iterator end = m_squadMembers.end();
		for (; it != end; it++)
		{
			if (pPlayer->GetEntityId() == it->actorId)
				return true;
		}
	}

	return false;
}

bool SSquad::isPlayerLeader() const
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (pPlayer)
	{
		if (pPlayer == GetLeader())
			return true;
	}

	return false;
}

void SSquad::OnPlayerAdded()
{
	m_pSquadSystem->m_animSquadMembers.Reload(true);
	m_pSquadSystem->m_animSquadMembers.SetVisible(true);

	UpdateMembersHUD();
}

void SSquad::PlayerRemoved()
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		m_pSquadSystem->ShowSquadMember(false, GetIndexFromMember(*it));
	}

	m_pSquadSystem->m_animSquadMembers.SetVisible(false);
	//CryLogAlways("PlayerRemoved");
}

void SSquad::Update()
{
	//static bool bDebugDraw = false;
	//static float color[] = { 1,1,1,1 };

	//int yoffset = 30;
	//if (m_squadMembers.size() > 0 && isPlayerLeader())
	//{
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset, 1.2f, color, false, "Members 0 Name = %s", (GetActor(m_squadMembers[0].actorId)->GetEntity()->GetName()));
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 20, 1.2f, color, false, "Members 0 Order = %d", int(m_squadMembers[0].currentOrder));
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 40, 1.2f, color, false, "Members 0 GotoState = %d", int(m_squadMembers[0].currentGotoState));
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 60, 1.2f, color, false, "Members 0 aiCleanDuration = %d", int(m_squadMembers[0].aiCleanDuration));
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 80, 1.2f, color, false, "Members 0 index = %d", int(m_squadMembers[0].index));
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 100, 1.2f, color, false, "Members 0 guardpos = (%1.f,%1.f,%1.f)", m_squadMembers[0].guardPos.x, m_squadMembers[0].guardPos.y, m_squadMembers[0].guardPos.z);
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 120, 1.2f, color, false, "Members 0 searchpos = (%1.f,%1.f,%1.f)", m_squadMembers[0].searchPos.x, m_squadMembers[0].searchPos.y, m_squadMembers[0].searchPos.z);
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 140, 1.2f, color, false, "Members 0 TargetThreat = %d", m_squadMembers[0].pActor->GetEntity()->GetAI()->CastToIPipeUser()->GetAttentionTargetThreat());
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 160, 1.2f, color, false, "Members 0 Target = %d", m_squadMembers[0].pActor->GetEntity()->GetAI()->CastToIPipeUser()->GetAttentionTarget() ? 1 : 0);
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 180, 1.2f, color, false, "Members 0 Alertness = %d", m_squadMembers[0].pActor->GetEntity()->GetAI()->GetProxy()->GetAlertnessState());
	//	gEnv->pRenderer->Draw2dLabel(10, yoffset + 200, 1.2f, color, false, "Members 0 Character = %s", m_squadMembers[0].pActor->GetEntity()->GetAI()->GetProxy()->GetCharacter());
	//}
	//bool pl = isPlayerLeader() || isPlayerMember();
	//gEnv->pRenderer->Draw2dLabel(10, 10, 1.2f, color, false, "Squad TestMembers %d", int(m_testMembers.size()));

	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 20, 1.2f, color, false, "Squad IsPlayerLeader %d", isPlayerLeader());
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 40, 1.2f, color, false, "Squad IsPlayerMember %d", isPlayerMember());
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 60, 1.2f, color, false, "Squad Id %d", GetId());
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 80, 1.2f, color, false, "Squad Leader name %s", GetLeader()->GetEntity()->GetName());
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 100, 1.2f, color, false, "Squad IsIncludePlayer %d", pl);
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 120, 1.2f, color, false, "SquadSystem m_animSquadMembers.GetVisible %d", g_pControlSystem->GetSquadSystem()->m_animSquadMembers.GetVisible());
	//gEnv->pRenderer->Draw2dLabel(10, yoffset + 140, 1.2f, color, false, "SquadSystem m_animSquadMembers.IsLoaded %d", g_pControlSystem->GetSquadSystem()->m_animSquadMembers.IsLoaded());

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	for (; it != end; ++it)
	{
		if (it->actorId && GetActor(it->actorId)->GetHealth() < 0.1f)
		{
			RemoveMember(*it);
		}

		//By default in the squad which not including the player, all members follow its leader
		const bool isSquadWithPlayer = isPlayerLeader() || isPlayerMember();
		const bool isSquadWithDude = GetLeader() == g_pGame->GetIGameFramework()->GetClientActor();
		if (!isSquadWithPlayer && !isSquadWithDude)
		{
			if (it->currentOrder != eSquadOrders_FollowLeader)
			{
				it->currentOrder = eSquadOrders_FollowLeader;
				ExecuteOrder(eSquadOrders_FollowLeader, *it);
			}
		}

		IActor* pMember = GetActor(it->actorId); //g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it);
		if (pMember && pMember != GetLeader())
		{
			IVehicle* pMemberVehicle = pMember->GetLinkedVehicle();
			if (IAIObject* pAI = !pMemberVehicle ? pMember->GetEntity()->GetAI() : pMemberVehicle->GetEntity()->GetAI())
			{
				if (IUnknownProxy* pAIProxy = pAI->CastToIAIActor()->GetProxy())
				{
					IPipeUser* pPU = pAI->CastToIPipeUser();
					if (pPU && pMember && pMember->GetHealth() > 0)
					{
						//if (bDebugDraw)
						//{
							//bool bIsNotFollowOrder = pPU->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW_QUICKLY && pPU->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW;
							//bool bIsCalm = (int)pPU->GetAttentionTargetThreat() < 2;

							//static float clr[] = { 1,1,1,1 };
							//gEnv->pRenderer->Draw2dLabel(20, 160, 1.5, clr, false,"");
						//}

						string memberClassName = pMember->GetEntity()->GetClass()->GetName();

						if (pAIProxy->GetAlertnessState() > 1 && !pPU->GetAttentionTarget())
						{
							pAI->Event(AIEVENT_CLEAR, 0);
						}

						//if (m_membersOrders[pMember] == ORD_SEARCH_ENEMY)
						if (it->currentOrder == eSquadOrders_SearchEnemy)
						{
							if (pPU->GetGoalPipeId() == GOALPIPEID_ORDER_SEARCH)
							{
								Vec3 pos = it->searchPos;
								bool bCalculateFlyHeight = false;

								if (memberClassName == "Trooper" || memberClassName == "PlayerTrooper")
									m_searchRadius = 20.f;
								else if (memberClassName == "Scout")
								{
									m_searchRadius = 90.f;
									bCalculateFlyHeight = true;

									if (CScout* pScoutMember = (CScout*)pMember)
									{
										if (!pScoutMember->m_searchbeam.isActive)
										{
											pScoutMember->EnableSearchBeam(true);

											const Vec3 vScoutPos = pScoutMember->GetEntity()->GetWorldPos();
											Vec3 vScoutPos2 = pScoutMember->GetEntity()->GetWorldPos();

											vScoutPos2.y += 3.f;
											vScoutPos2.z -= 4.f;

											const Vec3 vScoutBeamDir = (vScoutPos2 - vScoutPos).GetNormalized();
											pScoutMember->SetSearchBeamGoal(vScoutBeamDir);
										}
									}

									if (!pPU->GetAttentionTarget() && pAIProxy->GetAlertnessState() > 0)
									{
										gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "TO_SCOUTMOAC_IDLE", pAI);
									}
								}
								else if (pMemberVehicle)
								{
									if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
									{
										m_searchRadius = 90.f;
										bCalculateFlyHeight = true;
									}
									else
										m_searchRadius = 75.f;
								}

								pos.x += Random(-m_searchRadius, m_searchRadius);
								pos.y += Random(-m_searchRadius, m_searchRadius);
								// calculate pos.z coord
								if (bCalculateFlyHeight)
								{
									//DONT CHANGE, provide stable calculations
									pos.z = 2000.f;

									ray_hit hit;
									IPhysicalEntity* pSkipEntities[10];
									int nSkip = 0;
									IItem* pItem = pMember->GetCurrentItem();
									if (pItem)
									{
										CWeapon* pWeapon = (CWeapon*)pItem->GetIWeapon();
										if (pWeapon)
											nSkip = CSingle::GetSkipEntities(pWeapon, pSkipEntities, 10);
									}

									Vec3 castPos = pos;
									castPos.z -= 1.f;

									Vec3 castDir = (castPos - pos).GetNormalizedSafe() * 2000.f;

									if (gEnv->pPhysicalWorld->RayWorldIntersection(pos, castDir, ent_terrain | ent_static,
										rwi_ignore_noncolliding | rwi_stop_at_pierceable, &hit, 1, pSkipEntities, nSkip))
									{
										if (hit.pCollider)
										{
											if (hit.dist < SAFE_FLY_DISTANCE)
											{
												pos.z += SAFE_FLY_DISTANCE - hit.dist;
											}
											else if (hit.dist > SAFE_FLY_DISTANCE)
											{
												pos.z -= hit.dist - SAFE_FLY_DISTANCE;
											}
										}
									}
								}

								pPU->SetRefPointPos(pos);

								//CryLogAlways("pos(x%1.f, y%1.f,z%1.f)", (float)pos.x, (float)pos.y, (float)pos.z);
							}
							else if (pPU->GetGoalPipeId() != GOALPIPEID_ORDER_SEARCH)
							{
								if (!pPU->GetAttentionTarget())
									pPU->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH);
							}
						}
						//else if (m_membersOrders[pMember] == ORD_GOTO)
						else if (it->currentOrder == eSquadOrders_GoTo)
						{
							static bool mustOffAIPerception = false;
							static string pipeName, debugState = "";
							Vec3 guardPos = it->guardPos;//m_membersGuardPoses[pMember];

							if (memberClassName == "Scout")
							{
								guardPos.z += SAFE_FLY_DISTANCE;
								if (CScout* pScoutMember = (CScout*)pMember)
								{
									if (pScoutMember->m_searchbeam.isActive)
										pScoutMember->EnableSearchBeam(false);
								}
							}
							else if (pMemberVehicle)
							{
								if (pMemberVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
									guardPos.z += SAFE_FLY_DISTANCE;
								else
									m_searchRadius = 75.f;
							}

							//switch (m_membersGotoState[pMember])
							switch (it->currentGotoState)
							{
							case eGotoUpdateState_CleanAI:
								if (/*!pPU->IsUsingPipe("ord_guard") && !pPU->IsUsingPipe("ord_goto") &&*/
									it->aiCleanDuration != AI_CLEAN_FINISH)
								{
									//Preparing entity, resetting it

									debugState = "CLEAN_AI";
									mustOffAIPerception = true;

									static string sEntityClass = pMember->GetEntity()->GetClass()->GetName();
									if (sEntityClass == "Trooper" || sEntityClass == "PlayerTrooper")
										pipeName = "ord_cooldown_trooper";
									else
										pipeName = "ord_cooldown";

									pPU->SelectPipe(0, pipeName, 0, GOALPIPEID_ORDER_COOLDOWN);
									if (pPU->IsUsingPipe(pipeName))
										it->aiCleanDuration--;
								}
								else if (pPU->IsUsingPipe("ord_cooldown") ||
									(pPU->IsUsingPipe("ord_cooldown_trooper")) &&
									it->aiCleanDuration == AI_CLEAN_FINISH)
								{
									//if resetted and ready to action

									pPU->SelectPipe(0, "do_nothing");
									pPU->SetRefPointPos(guardPos);
									if (pPU->SelectPipe(0, "ord_goto", 0, GOALPIPEID_ORDER_GOTO))
										it->currentGotoState = eGotoUpdateState_GoTo;
								}
								break;
							case eGotoUpdateState_GoTo:
								debugState = "GOTO";
								if (pPU->IsUsingPipe("ord_ready_guard"))
								{
									it->currentGotoState = eGotoUpdateState_Guard;

									pPU->SetRefPointPos(guardPos);
									pPU->SelectPipe(0, "ord_guard", 0, GOALPIPEID_ORDER_GOTO_GUARD);

									mustOffAIPerception = false;
								}
								break;
							case eGotoUpdateState_Guard:
								debugState = "GUARD";
								if (pPU->GetAttentionTarget() &&
									(int)pPU->GetAttentionTargetThreat() > 2)
									it->currentGotoState = eGotoUpdateState_Combat;
								break;
							case eGotoUpdateState_Combat:
								debugState = "COMBAT";
								if (!pPU->GetAttentionTarget() &&
									(int)pPU->GetAttentionTargetThreat() == 0)
								{
									it->aiCleanDuration = AI_CLEAN_START_HALFSEC;
									it->currentGotoState = eGotoUpdateState_CleanAI;
								}

								break;
							}

							AgentParameters agentParams = pAI->CastToIAIActor()->GetParameters();
							if (mustOffAIPerception)
							{
								agentParams.m_PerceptionParams.perceptionScale.audio = 0;
								agentParams.m_PerceptionParams.perceptionScale.visual = 0;
								pAI->CastToIAIActor()->SetParameters(agentParams);
							}
							else
							{
								agentParams.m_PerceptionParams.perceptionScale.audio = 1;
								agentParams.m_PerceptionParams.perceptionScale.visual = 1;
								pAI->CastToIAIActor()->SetParameters(agentParams);
							}

							//static float clr[] = { 1,1,1,1 };
							//gEnv->pRenderer->Draw2dLabel(20, 200, 1.0f, clr, false,
							//	"%s m_iGotoTimer %1.f State %s",pMember->GetEntity()->GetName(), it->aiCleanDuration, debugState);
						}
						//else if (m_membersOrders[pMember] == ORD_FOLLOW_PLAYER)
						else if (it->currentOrder == eSquadOrders_FollowLeader)
						{
							//Member must be relaxed and when he have not target and alertess != 0 --> alertness must be 0
							const bool isRelaxed = (int)pPU->GetAttentionTargetThreat() < 2 && !pPU->GetAttentionTarget() && pAI->GetProxy()->GetAlertnessState() != 0;
							if (isRelaxed)
							{
								//CryLogAlways("[ExecuteOrder] member is scout from update");
								//pAI->CastToIAIActor()->SetSignal(0, "TO_SCOUTMOAC_IDLE", pMember->GetEntity());
								pAI->CastToIAIActor()->SetSignal(0, "RETURN_TO_FIRST", pMember->GetEntity());
							}

							if (memberClassName == "Scout")
							{
								if (CScout* pScoutMember = (CScout*)pMember)
								{
									if (pScoutMember->m_searchbeam.isActive)
										pScoutMember->EnableSearchBeam(false);
								}
							}

							IEntity* pLeader = GetLeader() ? GetLeader()->GetEntity() : 0;
							if (pLeader)
							{
								const bool isNotFollowOrder = pPU->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW_QUICKLY || pPU->GetGoalPipeId() != GOALPIPEID_ORDER_FOLLOW;
								const bool isCalm = (int)pPU->GetAttentionTargetThreat() < 2 && !pPU->GetAttentionTarget();

								if (isCalm)
								{
									const float distance = (pMember->GetEntity()->GetWorldPos() - pLeader->GetWorldPos()).GetLength();
									if (distance > 2.5f)
									{
										Vec3 leaderPos = pLeader->GetWorldPos();
										string leaderClassName = GetLeader()->GetEntity()->GetClass()->GetName();

										if (memberClassName == "Scout")
											leaderPos.z += (leaderClassName == "Hunter") ? SAFE_FLY_DISTANCE_HUNTER : SAFE_FLY_DISTANCE;

										pPU->SetRefPointPos(leaderPos);
									}

									if (isNotFollowOrder)
										pPU->SelectPipe(0, "ord_follow_player", 0, GOALPIPEID_ORDER_FOLLOW);
								}
							}
						}
					}
				}
			}
		}
	}

	/////////////////////// Update Remove Member when die ///////////////////////

	//TMembers::iterator itmem = m_squadMembers.begin();
	//TMembers::iterator itmemend = m_squadMembers.end();

	//for (; itmem != itmemend; itmem++)
	//{
	//	SMember& member = *itmem;
	//	if (member.pActor && member.pActor->GetHealth() < 0.1f)
	//	{
	//		RemoveMember(member);
	//	}
	//}

	/////////////////////// ~Update Remove Member when die ///////////////////////

	/////////////////////// Demiurg mode /////////////////////////////////////////

	//when the controlled actor in the squad is dead, switch him to a alive actor in the squad.
	bool bDemiurgIsEnabled = false;
	if (!bDemiurgIsEnabled)
		return;

	if (CActor* pActor = g_pControlSystem->GetControlClient()->GetControlledActor())
	{
		if (pActor->GetHealth() < 0.1f)
		{
			static float timer = 1.0f;

			if (timer != 0)
			{
				timer -= gEnv->pTimer->GetFrameTime();
				if (timer < 0)
					timer = 0;
			}

			if (timer == 0)
			{
				SSquad& squad = g_pControlSystem->pSquadSystem->GetSquadFromMember(pActor, 1);

				if (CActor* pMember = static_cast<CActor*>(squad.GetActor(squad.GetMemberAlive().actorId)))
				{
					g_pControlSystem->GetControlClient()->SetActor(pMember);
					SetLeader(pActor);

					timer = 1.0f;
				}
			}
		}
	}

	/////////////////////// ~Demiurg mode /////////////////////////////////////////
}

void SSquad::UpdateMembersHUD()
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		m_pSquadSystem->ShowSquadMember(true, GetIndexFromMember(*it));
	}
}

void SSquad::Serialize(TSerialize ser)
{
	ser.BeginGroup("ctrlSquad");
	SER_VALUE_ENUM(m_eLeaderCurrentOrder, eSquadOrders_GoTo, eSquadOrders_ForSync);
	SER_VALUE(m_searchRadius);
	SER_VALUE(m_squadId);

	TMembers members;
	TSMembers selectedMembers;
	if (ser.IsWriting())
	{
		members = m_squadMembers;
		selectedMembers = m_squadSelectedMembers;

		ser.Value("squadMembers", members);
		ser.Value("squadSelectedMembers", selectedMembers);
	}
	else
	{
		ser.Value("squadMembers", members);

		TMembers::iterator it = members.begin();
		TMembers::iterator end = members.end();
		for (; it != end; it++)
		{
			AddMember(*it);
		}

		ser.Value("squadSelectedMembers", selectedMembers);

		TSMembers::iterator it2 = selectedMembers.begin();
		TSMembers::iterator end2 = selectedMembers.end();
		for (; it2 != end2; it2++)
		{
			AddMemberToSelected(*it2);
		}
	}

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();
	for (; it != end; it++)
		it->Serialize(ser);
	ser.EndGroup();
}

float SSquad::GetMinDistance() const
{
	if (!GetLeader())
		return 0.0f;

	IEntity* pLeaderEnt = GetLeader()->GetEntity();
	std::vector<float> values;

	for (int i = 0; i <= m_squadMembers.size(); i++)
	{
		IEntity* pMemberEnt = gEnv->pEntitySystem->GetEntity(m_squadMembers[i].actorId);

		if (pMemberEnt)
		{
			Vec3 leaderPos = pLeaderEnt->GetWorldPos();
			Vec3 memberPos = pMemberEnt->GetWorldPos();

			float dist = (leaderPos - memberPos).GetLength();
			values.push_back(dist);
		}
	}

	float min = *std::min_element(values.begin(), values.end());
	return min;
}

void SSquad::RemoveMemberFromSelected(const IActor* pMember)
{
	if (!pMember)
		return;

	if (IsMember(pMember))
	{
		TSMembers::const_iterator it = m_squadSelectedMembers.begin();
		TSMembers::const_iterator end = m_squadSelectedMembers.end();

		bool finded = false;
		for (; it != end; it++)
		{
			if (*it == pMember->GetEntityId())
			{
				finded = true;
				break;
			}
		}

		if (finded)
			m_squadSelectedMembers.erase(it);
	}
}

void SSquad::RemoveMemberFromSelected(const int index)
{
	SMember& member = GetMemberFromIndex(index);
	if (!GetActor(member.actorId))
		return;

	TSMembers::const_iterator it = m_squadSelectedMembers.begin();
	TSMembers::const_iterator end = m_squadSelectedMembers.end();

	bool finded = false;
	for (; it != end; it++)
	{
		if (*it == member.actorId)
		{
			finded = true;
			break;
		}
	}

	if (finded)
		m_squadSelectedMembers.erase(it);
}

int SSquad::GetOrder(const EntityId id)
{
	SMember& member = GetMember(id);
	if (GetActor(member.actorId))
		return member.currentOrder;

	return eSquadOrders_None;
}

int SSquad::GetOrder(const IActor* act)
{
	SMember& member = GetMember(act);
	if (GetActor(member.actorId))
		return member.currentOrder;

	return eSquadOrders_None;
}

int SSquad::GetOrder(const int index)
{
	//if (!IsEnabled())
	//	return 0;

	/*if (IActor* pMember = GetMemberFromSlot(slot))
	{
		if (ESquadOrders currentOrder = m_membersOrders.find(pMember)->second)
			return (int)currentOrder;
	}*/

	SMember& member = GetMemberFromIndex(index);
	if (GetActor(member.actorId))
		return member.currentOrder;

	return eSquadOrders_None;
}

bool SSquad::RemoveMember(IActor* pActor)
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		int lastMemberIndex = GetMembersCount() - 1;
		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowDeadSquadMember(lastMemberIndex);

		RemoveMemberFromSelected(pActor);

		CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
		if (!pPlayer)
			pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (pActor == pPlayer)
			PlayerRemoved();

		m_squadMembers.erase(it);

		TMembers::iterator it = m_squadMembers.begin();
		TMembers::iterator end = m_squadMembers.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->UpdateSelectedHUD();

		return true;
	}
	return false;
}

bool SSquad::RemoveMember(EntityId id)
{
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (!pActor)
		return false;

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		int lastMemberIndex = GetMembersCount() - 1;
		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowDeadSquadMember(lastMemberIndex);

		RemoveMemberFromSelected(pActor);

		CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
		if (!pPlayer)
			pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (pActor == pPlayer)
			PlayerRemoved();

		m_squadMembers.erase(it);

		TMembers::iterator it = m_squadMembers.begin();
		TMembers::iterator end = m_squadMembers.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->UpdateSelectedHUD();

		return true;
	}
	return false;
}

SMember& SSquad::GetMember(const EntityId id)
{
	SMember nullMember(EntityId(0));

	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (!pActor)
	{
		CryLogAlways("[SMember SSquad::GetMember id] Get nullMember");
		return nullMember;
	}

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
			return *it;
	}

	CryLogAlways("[SMember SSquad::GetMember id] Get nullMember");
	CRY_ASSERT_MESSAGE(nullMember.pActor == 0, "Get nullMember");
	return nullMember;
}

SMember& SSquad::GetMember(const IActor* pActor)
{
	SMember nullMember(EntityId(0));

	if (!pActor)
	{
		CryLogAlways("[SMember SSquad::GetMember actor] Warning! Get nullMember");
		return nullMember;
	}

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
			return *it;
	}

	CryLogAlways("[SMember SSquad::GetMember actor] Warning! Get nullMember");
	CRY_ASSERT_MESSAGE(nullMember.pActor == 0, " Warning! Get nullMember");
	return nullMember;
}

SMember& SSquad::GetMemberFromIndex(const int index)
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		if (it->index == index)
		{
			return *it;
		}
	}

	CryLogAlways("[SSquad::GetMemberFromIndex] Warning! Get null member");
	return SMember();
}

bool SSquad::IsMember(const SMember& Member) const
{
	TMembers::const_iterator it = m_squadMembers.begin();
	TMembers::const_iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		if (it->actorId == Member.actorId)
		{
			//CryLogAlways("SSquad::IsMember 1");
			return true;
		}
	}

	return false;
}

bool SSquad::IsMember(const IActor* pActor) const
{
	if (!pActor)
		return false;

	TMembers::const_iterator it = m_squadMembers.begin();
	TMembers::const_iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
			return true;
	}

	return false;
}

bool SSquad::IsMember(const EntityId id) const
{
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (!pActor)
		return false;

	TMembers::const_iterator it = m_squadMembers.begin();
	TMembers::const_iterator end = m_squadMembers.end();
	for (; it != end; it++)
	{
		if (it->actorId == pActor->GetEntityId())
			return true;
	}

	return false;
}

int SSquad::GetIndexFromMember(const SMember& Member)
{
	return Member.index;
}

int SSquad::GetIndexFromMember(const IActor* pMember)
{
	SMember& member = GetMember(pMember);
	return member.index;
}

int SSquad::GetIndexFromMember(const EntityId id)
{
	SMember& member = GetMember(id);
	return member.index;
}

int SSquad::GetFreeMemberIndex() const
{
	TMembers::const_iterator it = m_squadMembers.begin();
	TMembers::const_iterator end = m_squadMembers.end();

	for (; it != end; it++)
	{
		if (it->actorId == 0)
			return it - m_squadMembers.begin();
	}

	return -1;
}

SMember& SSquad::GetMemberAlive()
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	for (; it != end; it++)
	{
		if (it->actorId != 0 && GetActor(it->actorId)->GetHealth() > 0.1f)
			return *it;
	}

	SMember nullMember(EntityId(0));
	CryLogAlways("[1236 SMember SSquad::GetAliveMember] Get nullMember");
	CRY_ASSERT_MESSAGE(nullMember.pActor == 0, "Get nullMember");
	return nullMember;
}

bool SSquad::IsMemberSelected(const IActor* pActor)
{
	if (!pActor)
		return false;

	TSMembers::const_iterator it = m_squadSelectedMembers.begin();
	TSMembers::const_iterator end = m_squadSelectedMembers.end();

	for (; it != end; it++)
	{
		if (IsMember(pActor) && pActor->GetEntityId() == *it)
			return true;
	}

	return false;
}

bool SSquad::IsMemberSelected(const int index)
{
	SMember& member = GetMemberFromIndex(index);
	if (!GetActor(member.actorId))
		return false;

	return stl::find(m_squadSelectedMembers, GetActor(member.actorId)->GetEntityId());
}

void SSquad::AddMemberToSelected(IActor* pActor)
{
	if (!pActor)
		return;

	if (IsMember(pActor))
	{
		TSMembers::const_iterator it = m_squadSelectedMembers.begin();
		TSMembers::const_iterator end = m_squadSelectedMembers.end();

		bool finded = false;
		for (; it != end; it++)
		{
			if (*it == pActor->GetEntityId())
			{
				finded = true;
				break;
			}
		}

		if (!finded)
		{
			m_squadSelectedMembers.push_back(pActor->GetEntityId());
		}
		else
			CryLogAlways("[SSquad::AddMemberToSelected] try to add already selected member");
	}
}

void SSquad::AddMemberToSelected(int index)
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	SMember& member = GetMemberFromIndex(index);
	if (!GetActor(member.actorId))
		return;

	//We cannot select the player
	if (GetActor(member.actorId) == pPlayer)
		return;

	TSMembers::const_iterator it = m_squadSelectedMembers.begin();
	TSMembers::const_iterator end = m_squadSelectedMembers.end();

	bool finded = false;
	for (; it != end; it++)
	{
		if (*it == member.actorId)
		{
			finded = true;
			break;
		}
	}

	if (!finded)
		m_squadSelectedMembers.push_back(member.actorId);
	else
		CryLogAlways("[SSquad::AddMemberToSelected] try to add already selected member");
}


void SSquad::SetLeader(IActor* pLeaderCandidate)
{
	if (!pLeaderCandidate)
		return;

	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (m_pLeader == pPlayer)
		PlayerRemoved();

	m_pLeader = pLeaderCandidate;

	if (m_pLeader == pPlayer)
		OnPlayerAdded();
}

bool SSquad::AddMember(SMember& member)
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (IsMember(member))
		return false;

	if (GetActor(member.actorId) && GetActor(member.actorId)->GetHealth() > 0.1f)
	{
		if (GetActor(member.actorId) == pPlayer)
			OnPlayerAdded();

		member.guardPos = member.searchPos = GetActor(member.actorId)->GetEntity()->GetWorldPos();
		member.currentOrder = eSquadOrders_FollowLeader;
		member.currentGotoState = eGotoUpdateState_Guard;
		member.index = m_squadMembers.size();

		if (IAIObject* pMemberAI = GetActor(member.actorId)->GetEntity()->GetAI())
		{
			if (pMemberAI->CastToIPipeUser())
			{
				if (member.currentGotoState != eGotoUpdateState_CleanAI)
					member.currentGotoState = eGotoUpdateState_CleanAI;

				member.aiCleanDuration = 10;
			}
		}

		CHUDRadar* pRadar = SAFE_HUD_FUNC_RET(GetRadar());
		if (pRadar)
		{
			pRadar->AddEntityToRadar(GetActor(member.actorId)->GetEntityId());
			pRadar->SetTeamMate(GetActor(member.actorId)->GetEntityId(), true);
		}
		m_squadMembers.push_back(member);

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowSquadMember(true, member.index);
		return true;
	}

	return false;
}

bool SSquad::AddMember(IActor* pActor)
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	if (IsMember(pActor))
		return false;

	if (pActor && pActor->GetHealth() > 0)
	{
		if (pActor == pPlayer)
			OnPlayerAdded();

		SMember member;
		member.actorId = pActor->GetEntityId();
		member.guardPos = member.searchPos = pActor->GetEntity()->GetWorldPos();
		member.currentOrder = eSquadOrders_FollowLeader;
		member.currentGotoState = eGotoUpdateState_Guard;
		member.index = m_squadMembers.size();

		if (IAIObject* pMemberAI = GetActor(member.actorId)->GetEntity()->GetAI())
		{
			if (pMemberAI->CastToIPipeUser())
			{
				if (member.currentGotoState != eGotoUpdateState_CleanAI)
					member.currentGotoState = eGotoUpdateState_CleanAI;

				member.aiCleanDuration = 10;
			}
		}

		CHUDRadar* pRadar = SAFE_HUD_FUNC_RET(GetRadar());
		if (pRadar)
		{
			pRadar->AddEntityToRadar(GetActor(member.actorId)->GetEntityId());
			pRadar->SetTeamMate(GetActor(member.actorId)->GetEntityId(), true);
		}

		m_squadMembers.push_back(member);

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowSquadMember(true, member.index);
		return true;
	}

	return false;
}

bool SSquad::AddMember(EntityId id)
{
	CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
	if (!pPlayer)
		pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (IsMember(pActor))
		return false;

	if (pActor && pActor->GetHealth() > 0)
	{
		if (pActor == pPlayer)
			OnPlayerAdded();

		SMember member;
		member.actorId = pActor->GetEntityId();
		member.guardPos = member.searchPos = pActor->GetEntity()->GetWorldPos();
		member.currentOrder = eSquadOrders_FollowLeader;
		member.currentGotoState = eGotoUpdateState_Guard;
		member.index = m_squadMembers.size();

		if (IAIObject* pMemberAI = GetActor(member.actorId)->GetEntity()->GetAI())
		{
			if (pMemberAI->CastToIPipeUser())
			{
				//if (m_membersGotoState[pActor] != STATE_CLEAN_AI)
				//	m_membersGotoState[pActor] = STATE_CLEAN_AI;
				if (member.currentGotoState != eGotoUpdateState_CleanAI)
					member.currentGotoState = eGotoUpdateState_CleanAI;

				member.aiCleanDuration = 10;
			}
		}

		CHUDRadar* pRadar = SAFE_HUD_FUNC_RET(GetRadar());
		if (pRadar)
		{
			pRadar->AddEntityToRadar(GetActor(member.actorId)->GetEntityId());
			pRadar->SetTeamMate(GetActor(member.actorId)->GetEntityId(), true);
		}

		//m_testMembers.push_back(member);
		m_squadMembers.push_back(member);
		//CryLogAlways("m_squadMembers.pushback --> new size is %d", int(m_squadMembers.size()));

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowSquadMember(true, member.index);
		//UpdateMembersHUD();

	//if (isPlayerMember() || isPlayerLeader())
		{
			//m_pSquadSystem->ShowSquadMember(true, GetIndexFromMember(member.pActor));
		}
		return true;
	}

	return false;
}

bool SSquad::RemoveMember(SMember& member)
{
	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	bool isFinded = false;

	for (; it != end; it++)
	{
		if (it->actorId == member.actorId)
		{
			isFinded = true;
			break;
		}
	}

	if (isFinded)
	{
		int lastMemberIndex = GetMembersCount() - 1;
		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->ShowDeadSquadMember(lastMemberIndex);

		CActor* pPlayer = g_pControlSystem->GetControlClient()->GetControlledActor();
		if (!pPlayer)
			pPlayer = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());

		if (GetActor(member.actorId) == pPlayer)
			PlayerRemoved();

		RemoveMemberFromSelected(member.actorId);
		m_squadMembers.erase(it);

		TMembers::iterator it = m_squadMembers.begin();
		TMembers::iterator end = m_squadMembers.end();
		for (; it != end; it++)
		{
			if (it->index > 0)
				it->index -= 1;
		}

		if (isPlayerMember() || isPlayerLeader())
			m_pSquadSystem->UpdateSelectedHUD();

		return true;
	}
	return false;
}


void SSquad::ExecuteOrder(ESquadOrders order, SMember& member)
{
	IAISystem* pAISys = gEnv->pAISystem;
	if (!pAISys || !GetActor(member.actorId))
		return;

	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		SControlClient* pControlCl = pDude->GetControlClient();
		if (pControlCl)
		{
			if (IAIObject* pActorAI = GetActor(member.actorId)->GetEntity()->GetAI())
			{
				IVehicle* pVehicle = GetActor(member.actorId)->GetLinkedVehicle();
				if (pVehicle)
				{
					if (pVehicle->GetDriver() == GetActor(member.actorId))
						pActorAI = pVehicle->GetEntity()->GetAI();
				}

				if (pActorAI)
				{
					const string memberClsName = GetActor(member.actorId)->GetEntity()->GetClass()->GetName();
					if (memberClsName == "Scout")
					{
						//CryLogAlways("[ExecuteOrder] member is scout");
						pActorAI->CastToIAIActor()->SetSignal(0, "TO_SCOUTMOAC_IDLE", GetActor(member.actorId)->GetEntity());
					}

					if (order == eSquadOrders_GoTo)
					{
						//member.currentOrder = ORD_GOTO;
						member.guardPos = pControlCl->m_crosshairPos;
						//m_membersGuardPoses[Member.pActor] = pControlCl->m_crosshairPos;
						Vec3 pos = member.guardPos; //m_membersGuardPoses[Member.pActor];

						if (memberClsName == "Scout")
							pos.z += SAFE_FLY_DISTANCE;
						else if (pVehicle)
						{
							if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
								pos.z += SAFE_FLY_DISTANCE;
						}

						if (member.currentGotoState != eGotoUpdateState_CleanAI)
							member.currentGotoState = eGotoUpdateState_CleanAI;

						/*if (m_membersGotoState[Member.pActor] != STATE_CLEAN_AI)
							m_membersGotoState[Member.pActor] = STATE_CLEAN_AI;*/
						if (pActorAI->CastToIPipeUser())
						{
							const EAITargetThreat targetThreat = pActorAI->CastToIPipeUser()->GetAttentionTargetThreat();
							switch (targetThreat)
							{
							case AITHREAT_NONE:
								member.aiCleanDuration = 25;//100 = 1 second
								break;
							case AITHREAT_INTERESTING:
								member.aiCleanDuration = 75;
								break;
							case AITHREAT_THREATENING:
								member.aiCleanDuration = 100;
								break;
							case AITHREAT_AGGRESSIVE:
								member.aiCleanDuration = 300;
								break;
							}
						}
					}
					else if (order == eSquadOrders_SearchEnemy)
					{
						//member.currentOrder = ORD_SEARCH_ENEMY;
						//m_membersSearchPoses[Member.pActor] = pControlCl->m_crosshairPos;
						member.searchPos = pControlCl->m_crosshairPos;

						const string memberClsName = GetActor(member.actorId)->GetEntity()->GetClass()->GetName();
						Vec3 pos = member.searchPos; //m_membersSearchPoses[member.pActor];

						if (memberClsName != "PlayerTrooper")
						{
							if (memberClsName == "Scout")
								pos.z += SAFE_FLY_DISTANCE;
							else if (pVehicle)
							{
								if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
									pos.z += SAFE_FLY_DISTANCE;
							}
						}
						pActorAI->CastToIPipeUser()->SetRefPointPos(pos);
						pActorAI->CastToIPipeUser()->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH, false);
					}
					else if (order == eSquadOrders_FollowLeader)
					{
						//member.currentOrder = ORD_FOLLOW_PLAYER;
						if (GetLeader())
							pActorAI->CastToIPipeUser()->SelectPipe(0, "ord_follow_player", 0, GOALPIPEID_ORDER_FOLLOW, false);
					}
				}
			}
		}
	}
}

bool SSquad::ExecuteOrderFG(ESquadOrders order, SMember& member, Vec3& refPoint)
{
	IAISystem* pAISys = gEnv->pAISystem;

	if (!pAISys || !GetActor(member.actorId))
		return false;

	CPlayer* pDude = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pDude)
	{
		SControlClient* pControlCl = pDude->GetControlClient();
		if (pControlCl)
		{
			if (IAIObject* pExecutorAI = GetActor(member.actorId)->GetEntity()->GetAI())
			{
				const string memberClsName = GetActor(member.actorId)->GetEntity()->GetClass()->GetName();

				IVehicle* pVehicle = GetActor(member.actorId)->GetLinkedVehicle();
				if (pVehicle)
				{
					if (pVehicle->GetDriver() == GetActor(member.actorId))
						pExecutorAI = pVehicle->GetEntity()->GetAI();
				}

				if (pExecutorAI)
				{
					const EAITargetThreat targetThreat = pExecutorAI->CastToIPipeUser()->GetAttentionTargetThreat();

					if (memberClsName == "Scout")
						pExecutorAI->CastToIAIActor()->SetSignal(0, "TO_SCOUTMOAC_IDLE", GetActor(member.actorId)->GetEntity());

					if (order == eSquadOrders_GoTo)
					{
						member.currentOrder = eSquadOrders_GoTo;
						member.guardPos = (refPoint.IsZero() ? pControlCl->m_crosshairPos : refPoint);
						//m_membersOrders[member.pActor] = ORD_GOTO;
						//m_membersGuardPoses[member.pActor] = (vRefPoint.IsZero() ? pControlCl->m_crosshairPos : vRefPoint);

						Vec3& pos = member.guardPos;
						//Vec3 pos = m_membersGuardPoses[member.pActor];

						if (memberClsName == "Scout")
							pos.z += SAFE_FLY_DISTANCE;
						else if (pVehicle)
						{
							if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
								pos.z += SAFE_FLY_DISTANCE;
						}

						if (member.currentGotoState != eGotoUpdateState_CleanAI)
							member.currentGotoState = eGotoUpdateState_CleanAI;
						//if (m_membersGotoState[member.pActor] != STATE_CLEAN_AI)
						//	m_membersGotoState[member.pActor] = STATE_CLEAN_AI;

						switch (targetThreat)
						{
						case AITHREAT_NONE:
							member.aiCleanDuration = 25;//100 = 1 second
							break;
						case AITHREAT_INTERESTING:
							member.aiCleanDuration = 75;
							break;
						case AITHREAT_THREATENING:
							member.aiCleanDuration = 100;
							break;
						case AITHREAT_AGGRESSIVE:
							member.aiCleanDuration = 300;
							break;
						}
						return true;
					}
					else if (order == eSquadOrders_SearchEnemy)
					{
						member.searchPos = refPoint.IsZero() ? pControlCl->m_crosshairPos : refPoint;
						//m_membersSearchPoses[member.pActor] = (vRefPoint.IsZero() ? pControlCl->m_crosshairPos : vRefPoint);

						Vec3& pos = member.searchPos;
						//Vec3 pos = m_membersSearchPoses[member.pActor];
						const string memberClsName = GetActor(member.actorId)->GetEntity()->GetClass()->GetName();

						if (memberClsName != "PlayerTrooper")
						{
							if (memberClsName == "Scout")
								pos.z += SAFE_FLY_DISTANCE;
							else if (pVehicle)
							{
								if (pVehicle->GetMovement()->GetMovementType() == IVehicleMovement::eVMT_Air)
									pos.z += SAFE_FLY_DISTANCE;
							}
						}
						pExecutorAI->CastToIPipeUser()->SetRefPointPos(pos);
						pExecutorAI->CastToIPipeUser()->SelectPipe(0, "ord_search_enemy", 0, GOALPIPEID_ORDER_SEARCH, false);
						return true;
					}
					else if (order == eSquadOrders_FollowLeader)
					{
						if (GetLeader())
							pExecutorAI->CastToIPipeUser()->SelectPipe(0, "ord_follow_player", 0, GOALPIPEID_ORDER_FOLLOW, false);

						return true;
					}
				}
				member.currentOrder = order;
			}
		}
	}
	return false;
}

void SSquad::GetMemoryStatistics(ICrySizer* s)
{
	s->Add(*this);

	TMembers::iterator it = m_squadMembers.begin();
	TMembers::iterator end = m_squadMembers.end();

	for (; it != end; it++)
		it->GetMemoryStatistics(s);
}