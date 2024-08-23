#pragma once

#include <unordered_map>

#include "../GenericModule.h"
#include "../../TOSGame.h"
#include "TheOtherSideMP/TOSSmartStruct.h"
#include "IGameRulesSystem.h"

class CTOSMasterClient;
class CTOSMasterSynchronizer;
class CTOSGenericSynchronizer;

/**
 * \brief хранит информацию о мастер-клиенте на сервере
 */
struct STOSMasterClientSavedParams
{
	EntityId                                 masterId;
	Vec3                                     pos;
	Quat                                     rot;
	float                                    suitEnergy;
	uint                                     suitMode;
	int                                      species;
	std::unordered_map<unsigned int, string> inventoryItems;
	string                                   currentItemClass;

	STOSMasterClientSavedParams()
		: masterId(0),
		suitEnergy(0),
		suitMode(0),
		species(-1) {};
};


/**
 * \brief хранит информацию о канале мастера во время перезапуска игры.
   \n Нужен для фикса бага: https://github.com/akeeperctl/crysis-wars-the-other-side/issues/9
 */
struct STOSScheduledMasterInfo  : STOSSmartStruct  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSScheduledMasterInfo()
		: masterChannelId(0),
		inGameDelay(0) {}

	explicit STOSScheduledMasterInfo(const uint channelId, const float _delay) :
		masterChannelId(channelId),
		inGameDelay(_delay)
	{}

	uint masterChannelId;
	float inGameDelay; ///< Задержка, которая начинает свой отсчёт (от x до 0) после того как \n канал мастера пребудет в состояние eCVS_InGame
};

/**
 * \brief хранит информацию о работе одного мастера
 */
struct STOSMasterInfo  : STOSSmartStruct  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSMasterInfo() :
		slaveId(0), flags(0)
	{}

	explicit STOSMasterInfo(const EntityId _slaveId) :
		slaveId(_slaveId), flags(0)
	{}

	bool Serialize(TSerialize ser)
	{
		ser.Value("slaveId", slaveId);
		ser.Value("desiredSlaveClassName", desiredSlaveClassName);
		ser.Value("flags", flags);

		return true;
	}

	EntityId slaveId;
	string desiredSlaveClassName;
	uint flags;
	STOSMasterClientSavedParams mcSavedParams;///< Сохраненные параметры, которые отправил мастер-клиент на сервер перед началом управления рабом. \n mc - мастер клиент
};

struct STOSStartControlInfo
{
	STOSStartControlInfo()
		: slaveId(0),
		masterChannelId(0),
		startDelay(0) {}

	explicit STOSStartControlInfo(const EntityId _slaveId, const uint _masterChannelId, const float _delay)
		: slaveId(_slaveId),
		masterChannelId(_masterChannelId),
		startDelay(_delay) {}

	EntityId slaveId;
	uint masterChannelId;
	float startDelay; ///< задержка в секундах перед передачей мастеру управления рабом \n посмотри CTOSMasterModule::ScheduleMasterStartControl
};

/**
 * \brief Модуль хранит информацию о том, кого контролирует мастер.
 * \note Также модуль создаёт контролируемую сущность в такт геймплея.
 * \note Например: модуль создаст \a раба для управления, \n когда присоединившийся игрок переходит из зрителя в игру.
 * \note И наоборот: модуль удалит \a раба, когда контролирующий игрок покинет игру или перейдёт в режим зрителя. \n
 * \note \a Мастер - это игрок, контролирующий в данный момент \a раба (в частности пришельца). (консольная команда tos_cl_JoinAsAlien)
 * \note \a Раб - это сущность, которую контролирует \a мастер в данный момент.
 */
class CTOSMasterModule  : 
	public CTOSGenericModule,
	public IHitListener
{
	friend class CTOSMasterSynchronizer;

public:
	CTOSMasterModule();
	~CTOSMasterModule() ;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) ;
	const char* GetName()  {return "CTOSMasterModule";};
	void Init() ;
	void Update(float frametime) ;
	void Serialize(TSerialize ser) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;
	void InitCVars(IConsole* pConsole) ;
	void InitCCommands(IConsole* pConsole) ;
	void ReleaseCVars() ;
	void ReleaseCCommands() ;
	//ITOSGameModule

	//IHitListener
	void OnHit(const HitInfo&);
	void OnExplosion(const ExplosionInfo&) {};
	void OnServerExplosion(const ExplosionInfo&) {};
	//~IHitListener

	void Reset();

	/**
	 * \brief Возвращает локальный мастер-клиент, доступный только для локальной машины
		\note Мастер-клиент создаётся при инициализации локального игрока
	 * \return Указатель на локальный мастер-клиент
	 */
	CTOSMasterClient* GetMasterClient() const
	{
		//assert(m_pLocalMasterClient);
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
	IEntity* GetMaster(const IEntity* pSlaveEntity) const;
	bool     SetMasterDesiredSlaveCls(const IEntity* pEntity, const char* slaveDesiredClass);

	IEntity* GetCurrentSlave(const IEntity* pMasterEntity);
	void     SetCurrentSlave(const IEntity* pMasterEntity, const IEntity* pSlaveEntity, uint masterFlags);
	void     ClearCurrentSlave(const IEntity* pMasterEntity);
	bool     IsSlave(const IEntity* pPotentialSlave) const;
	bool	 ReviveSlave(const IEntity* pSlaveEntity, const Vec3& revivePos, const Ang3& angles, const int teamId, const bool resetWeapons) const;

	void     DebugDraw(const Vec2& screenPos = Vec2(20,300), float fontSize = 1.2f, float interval = 20.0f, int maxElemNum = 5);

	//Console command's functions
	static void CmdDumpMastersList(IConsoleCmdArgs* pArgs);
	static void CmdIsMaster(IConsoleCmdArgs* pArgs);
	static void CmdMCStopControl(IConsoleCmdArgs* pArgs);
	static void CmdDumpDudeItems(IConsoleCmdArgs* pArgs);
	static void CmdDumpActorItems(IConsoleCmdArgs* pArgs);
	static void CmdSetPhysProfile(IConsoleCmdArgs* pArgs);
	static void CmdSetActorHealth(IConsoleCmdArgs* pArgs);
	static void CmdGetActorHealth(IConsoleCmdArgs* pArgs);
	static void CmdGetActorCurrentItem(IConsoleCmdArgs* pArgs);
	static void CmdPlaySound2D(IConsoleCmdArgs* pArgs);

	//Console variable's functions
	static void CVarSetDesiredSlaveCls(ICVar* pVar);

	bool GetMasterInfo(const IEntity* pMasterEntity, STOSMasterInfo& info);

private:

	/**
	 * \brief Берёт параметры сущности с сервера и сохраняет их там-же на сервере
	 * \param pMasterEntity - указатель на сущность мастера
	 */
	void SaveMasterClientParams(IEntity* pMasterEntity);
	void ApplyMasterClientParams(IEntity* pMasterEntity);

	/**
	 * \brief Функция предназначена дял конкретного случая на сервере. \n Она планирует отправку RMI'шку на клиент, \n когда тот будет полностью готов её принять.
	 * \param info - информация о сущности раба и о канале клиента, \n которому передастся управление над рабом.
	 */
	void ScheduleMasterStartControl(const STOSStartControlInfo& info);



public:

	//Console variables
	// cl - client
	// sv - server
	// mc - master client
	// pl - player

	//client variables
	int    tos_cl_JoinAsMaster;
	//ICVar* tos_cl_SlaveEntityClass;
	int	   tos_cl_playerFeedbackSoundsVersion;
	int	   tos_cl_nanosuitSoundsVersion;

	//server variables
	int    tos_sv_mc_LookDebugDraw;
	float  tos_sv_SlaveSpawnDelay;
	float  tos_sv_mc_StartControlDelay;// not used
	float  tos_sv_pl_inputAccel;

	//trooper server variables
	float tos_tr_jump_force;
	float tos_tr_double_jump_force;
	float tos_tr_charged_jump_mul;
	float tos_tr_double_jump_energy_cost;
	float tos_tr_double_jump_melee_energy_cost;
	float tos_tr_double_jump_melee_rest_seconds;
	float tos_tr_melee_energy_costs;
	float tos_tr_regen_energy_start_delay_sp;
	float tos_tr_regen_energy_start_delay_mp;
	float tos_tr_regen_energy_start_delay_20boundary;
	float tos_tr_regen_energy_recharge_time_sp;
	float tos_tr_regen_energy_recharge_time_mp;

private:
	std::map<EntityId, STOSMasterInfo> m_masters; //ключ - мастер, значение - структура, хранящая имя класса раба, который должен будет заспавниться
	CTOSMasterClient* m_pLocalMasterClient;
	std::map<EntityId, STOSScheduledMasterInfo> m_scheduledTakeControls; ///< \b Что \b хранит: \n ключ - ID сущности раба \n значение - структуру канала клиента через который подключен мастер
};