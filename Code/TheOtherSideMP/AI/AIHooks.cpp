/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.

Файл реализации хуков ИИ Системы и всё, что с ней связано
**************************************************************************/

#include "StdAfx.h"
#include "AIHooks.h"

#include "TheOtherSideMP\Game\Modules\Factions\FactionMap.h"
#include "TheOtherSideMP\Game\Modules\Factions\FactionsModule.h"
#include "TheOtherSideMP\Game\TOSGame.h"
#include <TheOtherSideMP\Helpers\TOS_Console.h>
#include <TheOtherSideMP\Helpers\TOS_Entity.h>

#ifdef _WIN64
constexpr size_t IAIOBJECT_VTABLE_ADDRESS = 0x000000003122bda0;
constexpr size_t IAIACTOR_VTABLE_ADDRESS = 0x0000000041acae44;
constexpr size_t IAISYSTEM_VTABLE_ADDRESS = 0x000000003121c290;
#elif _WIN32
constexpr size_t IAIOBJECT_VTABLE_ADDRESS = 0x311ba5d0;
constexpr size_t IAIACTOR_VTABLE_ADDRESS = 0x311ba598;
constexpr size_t IAISYSTEM_VTABLE_ADDRESS = 0x311b0fa0;
#endif

// Типы указателей на функции
using CanAcquireTargetFunc = decltype(&TOS_Hooks::AI::IAIActorHook::CanAcquireTarget);
using IsHostileFunc = decltype(&TOS_Hooks::AI::IAIObjectHook::IsHostile);

// Указатели на оригинальные функции
static CanAcquireTargetFunc originalCanAcquireTarget = nullptr;
static IsHostileFunc originalIsHostileFunc = nullptr;

void TOS_Hooks::AI::ApplyHooks()
{
	// Получение VTables 
	auto pVTableAIActor = Utils::VTables::GetVTableFromAddress(IAIACTOR_VTABLE_ADDRESS);
	auto pVTableAIObject = Utils::VTables::GetVTableFromAddress(IAIOBJECT_VTABLE_ADDRESS);

	// Получение индексов функций
	auto index1 = IndexFinder::getIndexOf(&IAIActor::CanAcquireTarget); // 11
	auto index2 = IndexFinder::getIndexOf(&IAIObject::IsHostile); // 35

	// Сохранение оригинальных функций
	originalCanAcquireTarget = reinterpret_cast<CanAcquireTargetFunc&>(pVTableAIActor[index1]);
	originalIsHostileFunc = reinterpret_cast<IsHostileFunc&>(pVTableAIObject[index2]);

	// Замена функций хуками
	TOS_Hooks::ReplaceFunction(&pVTableAIActor[index1], &IAIActorHook::CanAcquireTarget);
	TOS_Hooks::ReplaceFunction(&pVTableAIObject[index2], &IAIObjectHook::IsHostile);
}

// Определение функций хука обязательно должно быть вне структуры
// Иначе при получении адреса целевой функции будет выдаваться адрес совсем другой функции
/////////////////////////////////////////////////////////////

bool TOS_Hooks::AI::IAIActorHook::CanAcquireTarget(IAIObject* pOther) const
{
	//if (!pOther || !pOther->IsEnabled() || (pOther->GetEntity() && pOther->GetEntity()->IsHidden()))
	//	return false;

	////// Моя Фракция
	//const auto pFactions = g_pTOSGame->GetFactionsModule();
	//auto pMyAI = reinterpret_cast<const IAIActor*>(this);

	//auto myFaction = pFactions->GetAIFaction(pMyAI);
	//auto otherFaction = pFactions->GetAIFaction(pOther);

	//CFactionMap::EReaction MyToOtherReaction = pFactions->GetFactionMap()->GetReaction(myFaction, otherFaction);

	//return MyToOtherReaction == CFactionMap::Hostile && (this->*originalCanAcquireTarget)(pOther);

	//if (pOther->GetAIType() == AIOBJECT_ATTRIBUTE)
	//{
	//	// Аттрибут - это ИИ источника света или лазерной указки
	//	// Ассоциация - это ИИ, который создал этот свет
	//	// Например, Номад нацепил фонарик на оружие. Аттрибут - это свет, Ассоциация - Номад.

	//	// Поиск сущности, с которой ассоциируется аттрибут
	//	string attrName = pOther->GetName();
	//	string token;

	//	// позиция имени сущности ассоциации
	//	int entNamePos = 3;
	//	int curPos = 0;

	//	for (int i = 0; i < entNamePos; i++)
	//		token = attrName.Tokenize(" ", curPos);

	//	// Меняем ИИ света на ИИ ассоциации, т.е. на Номада
	//	string associationEntName = token;
	//	IEntity* pAssociationEnt = gEnv->pEntitySystem->FindEntityByName(associationEntName.c_str());
	//	if (pAssociationEnt)
	//		pOther = pAssociationEnt->GetAI();
	//}
	//else if (pOther->GetAIType() == AIOBJECT_DUMMY)
	//{
	//	// Perception of ...

	//	// Поиск сущности, с которой ассоциируется аттрибут
	//	string attrName = pOther->GetName();
	//	string token;

	//	// позиция имени сущности ассоциации
	//	int entNamePos = 3;
	//	int curPos = 0;

	//	for (int i = 0; i < entNamePos; i++)
	//		token = attrName.Tokenize(" ", curPos);

	//	// Меняем ИИ света на ИИ ассоциации, т.е. на Номада
	//	string associationEntName = token;
	//	IEntity* pAssociationEnt = gEnv->pEntitySystem->FindEntityByName(associationEntName.c_str());
	//	if (pAssociationEnt)
	//		pOther = pAssociationEnt->GetAI();
	//}

	//// Пустой указатель после ассоциации
	//// Так не должно быть
	//assert(pOther);
	//if (!pOther)
	//	return false;

	//return true;

	//IAIActor* pOtherActor = pOther->CastToIAIActor();
	//if (!pOtherActor)
		//return (pOther->GetAIType() == AIOBJECT_TARGET);

	//auto pThis = reinterpret_cast<const IAIActor*>(this);
	//auto& params = pThis->GetParameters();

	// Обрабатываем через стандартную функцию
	return (this->*originalCanAcquireTarget)(pOther);
};

bool TOS_Hooks::AI::IAIObjectHook::IsHostile(const IAIObject* pOther, bool bUsingAIIgnorePlayer) const
{
	bool hostile = false;

	if (!pOther)
		return hostile;

	// Моя Фракция
	const auto pFactions = g_pTOSGame->GetFactionsModule();

	auto debugOtherAIType = pOther->GetAIType();

	if (pOther->GetAIType() == AIOBJECT_ATTRIBUTE || pOther->GetAIType() == AIOBJECT_DUMMY)
	{
		// Perception of ... - AIOBJECT_DUMMY

		// Аттрибут (AIOBJECT_ATTRIBUTE) - это ИИ источника света или лазерной указки
		// Ассоциация - это ИИ, который создал этот свет
		// Например, Номад нацепил фонарик на оружие. Аттрибут - это свет, Ассоциация - Номад.

		// Поиск сущности, с которой ассоциируется аттрибут
		string attrName = pOther->GetName();
		string token;

		// позиция имени сущности ассоциации
		int entNamePos = 3;
		int curPos = 0;

		for (int i = 0; i < entNamePos; i++)
			token = attrName.Tokenize(" ", curPos);

		// Меняем ИИ света на ИИ ассоциации, т.е. на Номада
		string associationEntName = token;
		IEntity* pAssociationEnt = gEnv->pEntitySystem->FindEntityByName(associationEntName.c_str());
		if (pAssociationEnt)
			pOther = pAssociationEnt->GetAI();
	}

	// Пустой указатель после ассоциации
	// Так не должно быть
	assert(pOther);
	if (!pOther)
		return false;

	// Игнор игрока
	if (bUsingAIIgnorePlayer && (pOther->GetAIType() == AIOBJECT_PLAYER) && (TOS_Console::GetSafeIntVar("ai_IgnorePlayer", 0)))
		return false;

	// Моя сущность
	const IAIObject* pMyAI = reinterpret_cast<const IAIObject*>(this);

	// Враждебная фракция
	int myFaction = pFactions->GetAIFaction(pMyAI);
	int otherFaction = pFactions->GetAIFaction(pOther);

	assert(myFaction >= 0);
	assert(otherFaction >= 0);

	CFactionMap::EReaction MyToOtherReaction = pFactions->GetFactionMap()->GetReaction(myFaction, otherFaction);
	if (MyToOtherReaction == CFactionMap::Hostile)
		hostile = true;

	// Враждебность по параметрам
	const IAIActor* pMyAIActor = pMyAI->CastToIAIActor();
	const IAIActor* pOtherAIActor = pOther->CastToIAIActor();
	if (pMyAIActor)
	{
		const auto& myParams = pMyAIActor->GetParameters();

		if (myParams.m_bSpeciesHostility == false)
			hostile = false;

		if (bUsingAIIgnorePlayer && (myParams.m_bAiIgnoreFgNode == true))
			hostile = false;
	}

	if (pOtherAIActor)
	{
		const auto& otherParams = pOtherAIActor->GetParameters();

		if (otherParams.m_bSpeciesHostility == false)
			hostile = false;

		if (bUsingAIIgnorePlayer && otherParams.m_bAiIgnoreFgNode == true)
			hostile = false;
	}

	// return hostile && (this->*originalIsHostileFunc)(pOther, bUsingAIIgnorePlayer);
	return hostile;
}