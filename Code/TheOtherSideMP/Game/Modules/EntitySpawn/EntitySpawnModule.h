// ReSharper disable CppClangTidyCppcoreguidelinesSpecialMemberFunctions
// ReSharper disable CppInconsistentNaming
#pragma once

#include "../GenericModule.h"
#include "IEntity.h"

#include "TheOtherSideMP/TOSSmartStruct.h"


enum ETOSEntityFlags
{
	TOS_ENTITY_FLAG_MUST_RECREATED = (1 << 0),
	TOS_ENTITY_FLAG_SCHEDULED_RECREATION = (1 << 1),
	//TOS_ENTITY_FLAG_ALREADY_RECREATED = (1 << 2),
};

struct STOSScheduleDelegateAuthorityParams
{
	STOSScheduleDelegateAuthorityParams()
		: scheduledTime(0) {}

	string playerName; // имя игрока, который получит власть
	float scheduledTime; // штамп времени, когда свершилось планирование
};

struct STOSEntitySpawnParams : public STOSSmartStruct
{
	STOSEntitySpawnParams()
		: pSavedScript(nullptr),
		tosFlags(0)
	{
		m_refs = 0;
		vanilla = SEntitySpawnParams();
	}

	explicit STOSEntitySpawnParams(const SEntitySpawnParams& _vanillaParams)
		: pSavedScript(nullptr),
		tosFlags(0)
	{
		m_refs = 0;
		vanilla = _vanillaParams;
	}

	explicit STOSEntitySpawnParams(const STOSEntitySpawnParams& params)
		: STOSSmartStruct(params)
	{
		this->m_refs = 0;
		this->tosFlags = params.tosFlags;
		this->vanilla = params.vanilla;
		this->pSavedScript = params.pSavedScript;
		this->authorityPlayerName = params.authorityPlayerName;
		this->savedName = params.savedName;
	}

	~STOSEntitySpawnParams() override { }

	IScriptTable* pSavedScript;
	SEntitySpawnParams vanilla;
	string authorityPlayerName; // Имя персонажа игрока, которому будет передана власть над сущностью после её пересоздания 
	string savedName; //ИСПРАВИЛО БАГ https://github.com/akeeperctl/crysis-wars-the-other-side/issues/6
	uint32 tosFlags;

private:
};


typedef std::vector<EntityId> TVecEntities;
typedef std::map<EntityId, STOSScheduleDelegateAuthorityParams> TMapAuthorityParams;
//typedef std::map<EntityId, SEntitySpawnParams*> TMapParams;
typedef std::map<EntityId, _smart_ptr<STOSEntitySpawnParams>> TMapTOSParams;

/**
 * \brief Модуль создания сущностей, используемых в моде The Other Side
 * \note Также модуль предназначен для пересоздания сущностей, удаленных во время работы консольной команды sv_restart.
 */
class CTOSEntitySpawnModule final :
	public CTOSGenericModule
{
public:
	CTOSEntitySpawnModule();
	~CTOSEntitySpawnModule() override;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override { return "CTOSEntitySpawnModule"; };
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//~ITOSGameModule

	/**
	 * \brief Создаёт сущность по определенным параметрам
	 * \param params - параметры создания сущности
	 * \param sendTosEvent - если True, то при спавне произойдет отправка события eEGE_TOSEntityOnSpawn
	 * \return Если успешно, то указатель IEntity* на созданную сущность, иначе nullptr
	 */
	static IEntity* SpawnEntity(STOSEntitySpawnParams& params, bool sendTosEvent = true);

	/**
	 * \brief Проверяет, должна ли быть воссоздана сущность после sv_restart
	 * \param pEntity - указатель на проверяемую сущность
	 * \return Если сущность должна быть воссоздана после sv_restart, то True, иначе False
	 */
	bool MustBeRecreated(const IEntity* pEntity) const;

private:

	/**
	 * \brief Запланировать пересоздание сущности после sv_restart 
	 * \param pEntity - указатель на сущность, которую нужно пересоздать
	 */
	void ScheduleRecreation(const IEntity* pEntity);

	/**
	 * \brief Проверяет, спавнилась ли сущность через EntitySpawnModule::SpawnEntity()
	 * \param pEntity - указатель на сущность, у которого проверяется наличие сохраненных параметров
	 * \return Если сущность pEntity когда либо спавнилась через EntitySpawnModule::SpawnEntity(), то вернёт True
	 */
	bool HaveSavedParams(const IEntity* pEntity) const;

	void DebugDraw(const Vec2& screenPos, float fontSize, float interval, int maxElemNum, bool draw) const;

	static TVecEntities s_markedForRecreation; // Что хранит: сущности которые должны быть пересозданы после sv_restart
	TMapAuthorityParams m_scheduledAuthorities; // Что хранит: ключ - id сущности, значение - структура, где есть имя игрока, который получит власть над сущностью и штамп времени, когда случилось планирование
	TMapTOSParams m_scheduledRecreations; 
	TMapTOSParams m_savedParams; // Что хранит: ключ - id сущности, которая была заспавнена в этом модуле, значение - её STOSEntitySpawnParams, захваченные при спавне в этом модуле
};