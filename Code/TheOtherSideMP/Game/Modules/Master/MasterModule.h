#pragma once

#include "../GenericModule.h"

#include "../../TOSGame.h"

#include "TheOtherSideMP/TOSSmartStruct.h"

class CTOSMasterClient;
class CTOSMasterSynchronizer;
class CTOSGenericSynchronizer;

/**
 * \brief хранит информацию о работе одного мастера
 */
struct STOSMasterInfo : STOSSmartStruct  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSMasterInfo() :
		slaveId(0)
	{}

	explicit STOSMasterInfo(const EntityId _slaveId) :
		slaveId(_slaveId)
	{}

	EntityId slaveId;
	string desiredSlaveClassName;
};

struct STOSStartControlInfo
{
	STOSStartControlInfo()
		: slaveId(0),
		masterChannelId(0) {}

	explicit STOSStartControlInfo(const EntityId _slaveId, const uint _masterChannelId)
		: slaveId(_slaveId),
		masterChannelId(_masterChannelId) {}

	EntityId slaveId;
	uint masterChannelId;
};

/**
 * \brief Модуль хранит информацию о том, кого контролирует мастер.
 * \note Также модуль создаёт контролируемую сущность в такт геймплея.
 * \note Например: модуль создаст \a раба для управления, \n когда присоединившийся игрок переходит из зрителя в игру.
 * \note И наоборот: модуль удалит \a раба, когда контролирующий игрок покинет игру или перейдёт в режим зрителя. \n
 * \note \a Мастер - это игрок, способный контролировать \a раба (в частности пришельца). (консольная команда tos_cl_JoinAsAlien)
 * \note \a Раб - это сущность, которую контролирует \a мастер в данный момент.
 */
class CTOSMasterModule final : public CTOSGenericModule  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CTOSMasterModule();
	~CTOSMasterModule() override;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override {return "CTOSMasterModule";};
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	void InitCVars(IConsole* pConsole) override;
	void InitCCommands(IConsole* pConsole) override;
	void ReleaseCVars() override;
	void ReleaseCCommands() override;
	//ITOSGameModule

	/**
	 * \brief Возвращает локальный мастер-клиент, доступный только для локальной машины
	 * \return Указатель на локальный мастер-клиент
	 */
	CTOSMasterClient* GetMasterClient() const
	{
		assert(m_pLocalMasterClient);
		return m_pLocalMasterClient;
	};

	void RegisterMasterClient(CTOSMasterClient* pMC)
	{
		assert(pMC);
		m_pLocalMasterClient = pMC;
	};

	void UnregisterMasterClient()
	{
		m_pLocalMasterClient = nullptr;
	}

	void     MasterAdd(const IEntity* pMasterEntity, const char* slaveDesiredClass);
	void     MasterRemove(const IEntity* pMasterEntity);
	bool     IsMaster(const IEntity* pMasterEntity);
	void     GetMasters(std::map<EntityId, STOSMasterInfo>& masters) const;
	bool     SetMasterDesiredSlaveCls(const IEntity* pEntity, const char* slaveDesiredClass);
	IEntity* GetSlave(const IEntity* pMasterEntity);
	void     SetSlave(const IEntity* pMasterEntity, IEntity* pSlaveEntity);
	void     ClearSlave(const IEntity* pMasterEntity);
	bool     IsSlave(const IEntity* pEntity) const;
	void     DebugDraw(const Vec2& screenPos = {20,300}, float fontSize = 1.2f, float interval = 20.0f, int maxElemNum = 5);

	//Console command's functions
	static void CmdGetMastersList(IConsoleCmdArgs* pArgs);
	static void CmdIsMaster(IConsoleCmdArgs* pArgs);
	static void CmdStopControl(IConsoleCmdArgs* pArgs);

	//Console variable's functions
	static void CVarSetDesiredSlaveCls(ICVar* pVar);

private:
	bool GetMasterInfo(const IEntity* pMasterEntity, STOSMasterInfo& info);

	/**
	 * \brief Функция предназначена дял конкретного случая на сервере. \n Она планирует отправку RMI'шку на клиент, \n когда тот будет полностью готов её принять.
	 * \param info - информация о сущности раба и о канале клиента, \n которому передастся управление над рабом.
	 */
	void ScheduleMasterStartControl(const STOSStartControlInfo& info);


public:

	//Console variables
	int tos_cl_JoinAsMaster;
	ICVar* tos_cl_SlaveEntityClass;
	float tos_sv_SlaveSpawnDelay;

private:
	std::map<EntityId, STOSMasterInfo> m_masters; //ключ - мастер, значение - структура, хранящая имя класса раба, который должен будет заспавниться
	CTOSMasterClient* m_pLocalMasterClient;
	std::map<EntityId, uint> m_scheduledTakeControls; ///< \b Что \b хранит: \n ключ - ID сущности раба \n значение - ID канала клиента через который подключен мастер
};