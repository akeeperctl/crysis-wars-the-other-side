/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include "IEntity.h"
#include "IAgent.h"
#include "IVehicleSystem.h"

#include "GameCVars.h"

#include "TheOtherSideMP/Actors/TOSActor.h"
#include "TOS_Debug.h"
#include "TOS_Script.h"

namespace TOS_AI
{
	const auto FORGETTIME_TARGET = "target";
	const auto FORGETTIME_SEEK = "seek";
	const auto FORGETTIME_MEMORY = "memory";
	const auto FORGETTIME_ALL = "all";


	inline void SendEvent(IAIObject* pAI, int eventIdx)
	{
		if (pAI)
			pAI->Event(eventIdx, 0);
	}

	inline void SendSignal(IAIObject* pAI, const int signalFilter, const char* signalName, IEntity* pSender, IAISignalExtraData* pData)
	{
		if (gEnv->bServer && gEnv->pAISystem && pAI)
			pAI->CastToIAIActor()->SetSignal(signalFilter, signalName, pSender, pData);
	}

	inline void CancelSubpipe(IAIObject* pAI, const int goalPipeId)
	{
		if (pAI && pAI->IsEnabled() && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAI->CastToIPipeUser();
			if (pUser)
				pUser->CancelSubPipe(goalPipeId);
		}
	}

	/// @brief Восстановление времени забывания из скрипта ИИ Объекта в ИИ Систему
	/// @param pAI указатель на ИИ Объект
	inline void RestoreForgetTime(IAIObject* pAI)
	{
		if (!pAI)
			return;

		const auto pEntity = pAI->GetEntity();
		if (!pEntity)
			return;

		auto agentParams = pAI->CastToIAIActor()->GetParameters();

		TOS_Script::GetEntityProperty(pEntity, "Perception", "forgetTimeTarget", agentParams.m_PerceptionParams.forgetfulnessTarget);
		TOS_Script::GetEntityProperty(pEntity, "Perception", "forgetTimeSeek", agentParams.m_PerceptionParams.forgetfulnessSeek);
		TOS_Script::GetEntityProperty(pEntity, "Perception", "forgetTimeMemory", agentParams.m_PerceptionParams.forgetfulnessMemory);

		pAI->CastToIAIActor()->SetParameters(agentParams);
	}

	/// @brief Изменить время забывания цели у Объекта ИИ
	/// @param pAI указатель на Объект ИИ
	/// @param timeType тип времени, который будет меняться (FORGETTIME_ALL, FORGETTIME_TARGET, FORGETTIME_SEEK, FORGETTIME_MEMORY)
	/// @param value время в секундах
	inline void SetForgetTime(IAIObject* pAI, const char* timeType, const float value)
	{
		if (!pAI)
			return;

		auto agentParams = pAI->CastToIAIActor()->GetParameters();

		if (strcmp(timeType, FORGETTIME_ALL) == 0)
		{
			agentParams.m_PerceptionParams.forgetfulnessTarget = value;
			agentParams.m_PerceptionParams.forgetfulnessSeek = value;
			agentParams.m_PerceptionParams.forgetfulnessMemory = value;
		}
		else if (strcmp(timeType, FORGETTIME_TARGET) == 0)
		{
			agentParams.m_PerceptionParams.forgetfulnessTarget = value;
		}
		else if (strcmp(timeType, FORGETTIME_SEEK) == 0)
		{
			agentParams.m_PerceptionParams.forgetfulnessSeek = value;
		}
		else if (strcmp(timeType, FORGETTIME_MEMORY) == 0)
		{
			agentParams.m_PerceptionParams.forgetfulnessMemory = value;
		}

		pAI->CastToIAIActor()->SetParameters(agentParams);
	}

	/// @brief Получить ИИ персонажа из свойств скрипта ИИ Объекта.
	/// @brief Этот параметр задается пользователем в редакторе.
	/// @param pAI указатель на ИИ объект
	/// @return название ИИ Персонажа
	inline const char* GetScriptAICharacter(const IAIObject* pAI)
	{
		if (gEnv->bServer && gEnv->pAISystem && pAI && pAI->IsEnabled())
		{
			const char* character;
			TOS_Script::GetEntityProperty(pAI->GetEntity(), "aicharacter_character", character);

			return character;
		}

		return nullptr;
	}

	/// @brief Получить поведение из свойств скрипта ИИ Объекта.
	/// @brief Этот параметр задается пользователем в редакторе.
	/// @param pAI указатель на ИИ объект
	/// @return название поведения
	inline const char* GetScriptAIBehaviour(const IAIObject* pAI)
	{
		if (gEnv->bServer && gEnv->pAISystem && pAI && pAI->IsEnabled())
		{
			const char* behav;
			TOS_Script::GetEntityScriptValue(pAI->GetEntity(), "PropertiesInstance", "aibehavior_behaviour", behav);

			return behav;
		}

		return nullptr;
	}

	/// @brief Получить текущее поведение ИИ Объекта
	/// @param pAI - указатель на ИИ объект
	/// @return название текущего поведения 
	inline const char* GetCurrentAIBehaviour(const IAIObject* pAI)
	{
		if (gEnv->bServer && gEnv->pAISystem && pAI && pAI->IsEnabled())
		{
			const char* behav;
			TOS_Script::GetEntityScriptValue(pAI->GetEntity(), "Behaviour", "name", behav);

			return behav;
		}

		return "NULL";
	}

	inline bool IsUsingPipe(IAIObject* pAI, const char* pipe)
	{
		if (pAI && pAI->IsEnabled() && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAI->CastToIPipeUser();
			if (pUser)
				return pUser->IsUsingPipe(pipe);
		}

		return false;
	}

	inline bool IsUsingPipe(const IActor* pActor, const char* pipe)
	{
		if (gEnv->pAISystem && !gEnv->pAISystem->IsEnabled() || !gEnv->pAISystem)
			return false;

		if (!pActor)
			return false;

		const auto pAI = pActor->GetEntity()->GetAI();
		if (!pAI)
			return false;

		if (pAI && pAI->IsEnabled() && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAI->CastToIPipeUser();
			if (pUser)
				return pUser->IsUsingPipe(pipe);
		}

		return false;
	}

	inline int GetGoalPipeId(const IAIObject* pAIObject)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				return pUser->GetGoalPipeId();
		}
	}

	inline bool InsertSubpipe(IAIObject* pAIObject, const int goalFlag, const int goalPipeId, const char* pipeName, IAIObject* pAITarget, const char* solution)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
			{
				pUser->InsertSubPipe(goalFlag, pipeName, pAITarget, goalPipeId);

				//CryLogAlways("%s[C++][%s Insert Pipe %s][CASE: %s]",
				//	TOS_COLOR_PURPLE, pAIObject->GetName(), pipeName, solution);

				if (pUser->IsUsingPipe(pipeName))
					return true;
			}
		}

		return false;
	}

	inline bool SelectPipe(IAIObject* pAIObject, const int goalPipeId, const char* pipeName, const char* solution)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
			{
				pUser->SelectPipe(0, pipeName, nullptr, goalPipeId);

				//CryLogAlways("%s[C++][%s Select Pipe %s][CASE: %s]",
				//	TOS_COLOR_PURPLE, pAIObject->GetName(), pipeName, solution);

				if (pUser->IsUsingPipe(pipeName))
					return true;
			}
		}

		return false;
	}

	inline bool SelectPipe(IActor* pActor, const int goalPipeId, const char* pipeName, const char* solution)
	{
		if (!pActor)
			return false;

		const auto pAIObject = pActor->GetEntity()->GetAI();
		if (!pAIObject)
			return false;

		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
			{
				pUser->SelectPipe(0, pipeName, nullptr, goalPipeId);

				CryLog("%s[%s] Select pipe '%s' CASE: %s", TOS_COLOR_PURPLE, pAIObject->GetName(), pipeName, solution);

				if (pUser->IsUsingPipe(pipeName))
					return true;
			}
		}

		return false;
	}

	inline void SetRefPoint(IAIObject* pAIObject, const Vec3& pos)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				pUser->SetRefPointPos(pos);
		}
	}

	inline void SetRefPoint(IActor* pActor, const Vec3& pos)
	{
		if (!pActor)
			return;

		const auto pAIObject = pActor->GetEntity()->GetAI();
		if (!pAIObject)
			return;

		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				pUser->SetRefPointPos(pos);
		}
	}

	inline Vec3 GetRefPoint(IAIObject* pAIObject)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				return pUser->GetRefPoint()->GetPos();
		}

		return Vec3(0, 0, 0);
	}

	inline Vec3 GetRefPoint(IActor* pActor)
	{
		if (!pActor)
			return Vec3(0);

		const auto pAIObject = pActor->GetEntity()->GetAI();
		if (!pAIObject)
			return Vec3(0);

		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				return pUser->GetRefPoint()->GetPos();
		}

		return Vec3(0, 0, 0);
	}

	inline bool IsInCombat(const IAIObject* pAIObject)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			//auto pProxy = pAIObject->GetProxy();
			//if (pProxy)
			//	return pProxy->GetAlertnessState() == 2;

			const auto pPipeUser = pAIObject->CastToIPipeUser();
			if (pPipeUser)
			{
				//return pPipeUser->GetAttentionTarget(); //the best reason on 10.12.22

				const auto threat = pPipeUser->GetAttentionTargetThreat();
				const auto type = pPipeUser->GetAttentionTargetType();
				if (threat > 1 && type > 1)
					return true; //the best reason on 02.01.23
			}
		}

		return false;
	}

	inline bool IsInCombat(const EntityId id)
	{
		const auto pEntity = gEnv->pEntitySystem->GetEntity(id);
		if (!pEntity)
			return false;

		const auto pAI = pEntity->GetAI();
		if (!pAI)
			return false;

		return IsInCombat(pAI);
	}

	inline void SetStance(IAIObject* pAIObject, const EStance stance)
	{
		if (pAIObject && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pUser = pAIObject->CastToIPipeUser();
			if (pUser)
				switch (stance)
				{
					case STANCE_STAND:
						pUser->InsertSubPipe(0, "do_it_standing");
						break;
					case STANCE_CROUCH:
						pUser->InsertSubPipe(0, "do_it_crouched");
						break;
					case STANCE_PRONE:
						pUser->InsertSubPipe(0, "do_it_prone");
						break;
					case STANCE_RELAXED:
						pUser->InsertSubPipe(0, "do_it_relaxed");
						break;
					case STANCE_STEALTH:
						pUser->InsertSubPipe(0, "do_it_stealth");
						break;
					default:
						pUser->InsertSubPipe(0, "do_it_standing");
						break;
				}
		}
	}

	inline void ReturnToFirst(IAIObject* pAI, const int once, const int resetStatic, bool saveStance)
	{
		if (!pAI)
			return;

		static auto sended = 0;

		if (resetStatic != 0)
			sended = 0;

		if (once != 0 && sended != 1)
		{
			pAI->CastToIAIActor()->SetSignal(0, "RETURN_TO_FIRST", pAI->GetEntity());
			sended = 1;
		}
		else if (!once)
		{
			pAI->CastToIAIActor()->SetSignal(0, "RETURN_TO_FIRST", pAI->GetEntity());
		}
	}

	inline void ReturnToFirstSimple(IAIObject* pAI, const bool saveStance)
	{
		if (!pAI)
			return;

		const auto pAIActor = pAI->CastToIAIActor();
		if (pAIActor)
			pAIActor->SetSignal(0, "RETURN_TO_FIRST", pAI->GetEntity());
	}

	inline void EnablePerception(IAIObject* pAI, const bool enable)
	{
		if (!pAI)
			return;

		AgentParameters agentParams = pAI->CastToIAIActor()->GetParameters();
		if (!enable)
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
	}

	inline bool IsPerceptionEnabled(IAIObject* pAI)
	{
		if (!pAI)
			return false;

		const auto& agentParams = pAI->CastToIAIActor()->GetParameters();

		const auto audioScale = agentParams.m_PerceptionParams.perceptionScale.audio;
		const auto visualScale = agentParams.m_PerceptionParams.perceptionScale.visual;

		return audioScale > 0 && visualScale > 0;
	}

	inline void DrawPrimaryWeapon(const IAIObject* pAI)
	{
		if (pAI)
		{
			const auto pEntity = pAI->GetEntity();
			if (pEntity)
			{
				const auto pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
				if (pActor && pActor->GetLinkedVehicle())
					return;

				const auto pScriptTable = pEntity->GetScriptTable();
				if (pScriptTable)
					Script::CallMethod(pScriptTable, "DrawWeaponNow", 1);
			}
		}
	}

	/// @brief Регистрирует сущность в ИИ системе.
	/// @brief Требуется определение функции RegisterAIasPlayer в Lua
	/// @param pEntity 
	/// @param player 
	inline void RegisterAI(IEntity* pEntity, const bool player)
	{
		if (pEntity && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const auto pTable = pEntity->GetScriptTable();
			if (pTable)
			{
				if (!player)
					Script::CallMethod(pTable, "RegisterAI");
				else
					Script::CallMethod(pTable, "RegisterAIasPlayer");
			}
		}
	}

	inline void SetSpecies(IAIObject* pAI, const int species)
	{
		if (pAI && gEnv->bServer && gEnv->pAISystem && gEnv->pAISystem->IsEnabled())
		{
			AgentParameters playerParams = pAI->CastToIAIActor()->GetParameters();
			playerParams.m_nSpecies = species;

			if (pAI->CastToIAIActor())
				pAI->CastToIAIActor()->SetParameters(playerParams);

			const auto pTable = pAI->GetEntity()->GetScriptTable();
			if (pTable)
			{
				SmartScriptTable props;
				pTable->GetValue("Properties", props);
				props->SetValue("species", species);
			}
		}
	}

	inline int GetSpecies(const IAIObject* pAI, const bool fromLua)
	{
		int species = -1;

		if (pAI && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const AgentParameters playerParams = pAI->CastToIAIActor()->GetParameters();
			species = playerParams.m_nSpecies;

			if (fromLua)
			{
				const auto pTable = pAI->GetEntity()->GetScriptTable();
				if (pTable)
				{
					SmartScriptTable props;
					pTable->GetValue("Properties", props);
					props->GetValue("species", species);
				}
			}
		}

		return species;
	}

	//object 1 is hostile to object 2?
	inline bool IsHostile(IEntity* pEntity1, IEntity* pEntity2, const bool luaMethod)
	{
		bool hostile = false;

		if (pEntity1 && pEntity1->GetAI() && pEntity2 && pEntity2->GetAI() && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const int species1 = GetSpecies(pEntity1->GetAI(), luaMethod);
			const int species2 = GetSpecies(pEntity2->GetAI(), luaMethod);

			const bool isNeutral = species1 == -1 || species2 == -1;

			hostile = (species1 != species2) && !isNeutral;
		}

		return hostile;
	}

	//object 1 is hostile to object 2?
	inline bool IsHostile(const IAIObject* pAIObject1, const IAIObject* pAIObject2, const bool luaMethod)
	{
		bool hostile = false;

		if (pAIObject1 && pAIObject2 && gEnv->bServer && gEnv->pAISystem->IsEnabled())
		{
			const int species1 = GetSpecies(pAIObject1, luaMethod);
			const int species2 = GetSpecies(pAIObject2, luaMethod);

			const bool isNeutral = species1 == -1 || species2 == -1;

			hostile = (species1 != species2) && !isNeutral;
		}

		return hostile;
	}

	inline float GetAnchorRadius(const IEntity* pAnchorEntity)
	{
		float radius = 0;

		if (gEnv->bServer && gEnv->pAISystem && pAnchorEntity)
		{
			SmartScriptTable props;
			const auto       pTable = pAnchorEntity->GetScriptTable();
			pTable->GetValue("Properties", props);

			props->GetValue("radius", radius);
		}

		return radius;
	}

	inline void MakeHostile(IAIObject* pAI, bool hostile)
	{
		if (!pAI)
			return;

		auto pAIActor = pAI->CastToIAIActor();
		if (!pAIActor)
			return;

		auto params = pAIActor->GetParameters();
		params.m_bSpeciesHostility = hostile;
		pAIActor->SetParameters(params);
	}

	namespace deprecated
	{
		inline void EnableCombat_deprecated(IAIObject* pAI, const bool enable, const bool toFirst, const char* solution)
		{
			if (!pAI)
				return;

			//CryLogAlways("%s[C++][Enable Combat: %i][AI: %s][CASE: %s]",
			//TOS_COLOR_PURPLE, enable, pAI->GetName(), solution);

			const auto pVeh = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(pAI->GetEntityID());

			if (!enable)
			{
				if (toFirst)
				{
					//Clean subpipes
					TOS_AI::CancelSubpipe(pAI, 0);

					gEnv->pAISystem->Devalue(pAI, pAI->CastToIPipeUser()->GetAttentionTarget(), false, 5.0f);

					//Clean a behaviour in the lua
					TOS_AI::ReturnToFirst(pAI, 0, 0, true);

					//EStance stance = STANCE_STAND;
					//auto pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pAI->GetEntityID()));
					//if (pActor)
					//	stance = pActor->GetStance();

					//const char* stancePipe = "do_it_standing";
					//switch (stance)
					//{
					//case STANCE_STAND:
					//	stancePipe = "do_it_standing";
					//	break;
					//case STANCE_CROUCH:
					//	stancePipe = "do_it_crouched";
					//	break;
					//case STANCE_PRONE:
					//	stancePipe = "do_it_prone";
					//	break;
					//case STANCE_RELAXED:
					//	stancePipe = "do_it_relaxed";
					//	break;
					//case STANCE_STEALTH:
					//	stancePipe = "do_it_stealth";
					//	break;
					//}

					//Clean active goals
					TOS_AI::SelectPipe(pAI, 1, "do_nothing", "When combat is disabiling ai must do nothing");

					//if (pActor)
					//	TOS_AI::InsertSubpipe(pAI, AIGOALPIPE_SAMEPRIORITY, 0, stancePipe, "Set actor stance to saved stance");

					//Clean an attention target
					TOS_AI::InsertSubpipe(pAI, AIGOALPIPE_NOTDUPLICATE, 2, "ord_cooldown", nullptr, "Clear attention target, stop fire and then return to first");
					TOS_AI::SetForgetTime(pAI, FORGETTIME_ALL, 0.01f);

					const char* character = TOS_AI::GetScriptAICharacter(pAI);

					//The additional method to relax an AI in the combat
					if (pVeh)
					{
						//STOP_VEHICLE on Car cause driver exiting

						//if (strcmp(character, "AAA") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "APC") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "Boat") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "Car") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "HeliAggressive") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_HELI_IDLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "Heli") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_HELI_IDLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "PatrolBoat") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "Tank") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "TankClose") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "TankFixed") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "STOP_VEHICLE", pAI->GetEntity(), 0);
						//else if (strcmp(character, "Vtol") == 0)
						//	TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_HELI_IDLE", pAI->GetEntity(), 0);

						//Fix for a helicopter
						//TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "MAKE_ME_IGNORANT", pAI->GetEntity(), 0);

						TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_IDLE", pAI->GetEntity(), nullptr);
					}
					else
					{
						//The aliens
						if (strcmp(character, "Drone") == 0 ||
							strcmp(character, "ScoutMOAC") == 0)
							TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_SCOUTMOAC_IDLE", pAI->GetEntity(), nullptr);
						else if (strcmp(character, "GuardNeue") == 0 ||
								 strcmp(character, "Hunter") == 0 ||
								 strcmp(character, "HunterNew") == 0)
							TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "TO_IDLE", pAI->GetEntity(), nullptr);
						else if (strcmp(character, "Trooper") == 0 ||
								 strcmp(character, "TrooperCloak") == 0 ||
								 strcmp(character, "TrooperGuardian") == 0 ||
								 strcmp(character, "TrooperLeader") == 0 ||
								 strcmp(character, "TrooperLeaderMKII") == 0 ||
								 strcmp(character, "TrooperMKII_Sneak") == 0 ||
								 strcmp(character, "TrooperMKII") == 0)
							TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "GO_TO_IDLE", pAI->GetEntity(), nullptr);
						else //The humans
							TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "GO_TO_IDLE", pAI->GetEntity(), nullptr);
					}
				}

				TOS_AI::EnablePerception(pAI, false);
			}
			else
			{
				TOS_AI::EnablePerception(pAI, true);
				TOS_AI::RestoreForgetTime(pAI);

				//Fix for a helicopter
				//if (pVeh)
				//TOS_AI::SendSignal(pAI, SIGNALFILTER_SENDER, "MAKE_ME_UNIGNORANT", pAI->GetEntity(), 0);
			}
		}

		inline void EnableCombat_deprecated(IActor* pActor, const bool enable, const bool toFirst, const char* solution)
		{
			if (!pActor)
				return;

			const auto pAI = pActor->GetEntity()->GetAI();
			if (!pAI)
				return;

			EnableCombat_deprecated(pAI, enable, toFirst, solution);
		}

		inline bool IsCombatEnable_deprecated(IActor* pActor)
		{
			if (!pActor)
				return false;

			const auto pAI = pActor->GetEntity()->GetAI();
			if (!pAI)
				return false;

			return TOS_AI::IsPerceptionEnabled(pAI);
		}

		inline bool IsCombatEnable_deprecated(IAIObject* pAI)
		{
			if (!pAI)
				return false;

			return TOS_AI::IsPerceptionEnabled(pAI);
		}

	}


	//inline float GetDetectionValue(IAIObject* pAI)
	//{
	//	if (pAI && gEnv->bServer && gEnv->pAISystem->IsEnabled())
	//	{
	//		SAIDetectionLevels levels;
	//		float value = 0.f;

	//		if (pAI->GetEntity()->GetId() == g_pGame->GetIGameFramework()->GetClientActorId())
	//		{
	//			gEnv->pAISystem->GetDetectionLevels(pAI, levels);

	//			value = max(
	//				max(levels.puppetExposure, levels.puppetThreat),
	//				max(levels.vehicleExposure, levels.vehicleThreat));
	//		}
	//		else
	//		{
	//			float* pAIActorFloat = (float*)pAI->CastToIAIActor();

	//			// Varies between X86 and X64
	//			int nDataIndex = (sizeof(void*) == 8) ? 502 : 473;

	//			// Create snapshot
	//			pAIActorFloat[nDataIndex + 4] = pAIActorFloat[nDataIndex + 0];
	//			pAIActorFloat[nDataIndex + 5] = pAIActorFloat[nDataIndex + 1];
	//			pAIActorFloat[nDataIndex + 6] = pAIActorFloat[nDataIndex + 2];
	//			pAIActorFloat[nDataIndex + 7] = pAIActorFloat[nDataIndex + 3];

	//			levels.puppetExposure = pAIActorFloat[nDataIndex + 0];
	//			levels.puppetThreat = pAIActorFloat[nDataIndex + 1];
	//			levels.vehicleExposure = pAIActorFloat[nDataIndex + 2];
	//			levels.vehicleThreat = pAIActorFloat[nDataIndex + 3];

	//			// Reset originals
	//			pAIActorFloat[nDataIndex + 0] = 0;
	//			pAIActorFloat[nDataIndex + 1] = 0;
	//			pAIActorFloat[nDataIndex + 2] = 0;
	//			pAIActorFloat[nDataIndex + 3] = 0;

	//			value = max(
	//				max(levels.puppetExposure, levels.puppetThreat),
	//				max(levels.vehicleExposure, levels.vehicleThreat));
	//		}

	//		return value;
	//	}

	//	return 0;
	//}

	//inline bool CreateFormation(IAIObject* pLeaderAI, const char* luaFormationName)
	//{
	//	if (pLeaderAI && gEnv->bServer && gEnv->pAISystem->IsEnabled())
	//		return pLeaderAI->CreateFormation(luaFormationName);

	//	return false;
	//}

	//inline bool JoinFormation(IAIObject* pMemberAI, const char* luaFormationName)
	//{
	//	if (pMemberAI && gEnv->bServer && gEnv->pAISystem->IsEnabled())
	//		return pMemberAI->follow

	//	return false;
	//}

	/*string RequestPipeName(ESquadOrders order, EGotoUpdateState state = eGUS_ForSync, const string& clsName = "")
	{
		if (order == eSO_FollowLeader)
			return "ord_follow_player";
		else if (order == eSO_GoTo)
		{
			if (state == eGUS_CleanAI)
			{
				if (clsName != "PlayerTrooper" && clsName != "Trooper")
				{
					return "ord_cooldown_trooper";
				}
				else
				{
					return "ord_cooldown";
				}
			}
		}
		else if (order == eSO_SearchEnemy)
		{
			return "ord_search_enemy";
		}
	}*/
}
