--************************************************************************* 
 --AlienKeeper Source File.
 --Copyright (C), AlienKeeper, 2024.
--*************************************************************************

-- Файл представляет собой определитель синтаксиса для различных функций
-- или значений переменных, которые зареганы в Движке, но подсказки по ним нет.

---@class Vec3
---@field x number
---@field y number
---@field z number

---@diagnostic disable-next-line: lowercase-global
g_ActionData = {
    actionId = "",
    userId = 0,
    objectId = 0,
    maxAlertness = 0,
    goalPipeId = 0
}

SPECIAL_AI_OBJECT = {
    ANIMTARGET = "animtarget",
    ATT_TARGET = "atttarget",
    BEACON = "beacon",
    FORMATION = "formation",
    FORMATION_ID = "formation_id",
    FORMATION_SPECIAL = "formation_special",
    GROUP_TAC_POS = "group_tac_pos",
    GROUP_TAC_LOOK = "group_tac_look",
    HIDEPOINT_LASTOP = "hidepoint_lastop",
    LAST_HIDE_OBJECT = "last_hideobject",
    --LOOKAT_TARGET = "lookat_target",
    PLAYER = "player",
    PROB_TARGET = "probtarget",
    REFPOINT = "refpoint",
    SELF = "self",
    VEHICLE_AVOID = "vehicle_avoid",
}

-- Выражения для goalOp 'branch'

IF_ACTIVE_GOALS = 0 -- Переход выполняется, если есть активные (не завершенные) операции целей.
IF_ACTIVE_GOALS_HIDE = 1 -- Переход выполняется, если есть активные цели или не было найдено укрытие.
IF_NO_PATH = 2 -- Переход выполняется, если в последней операции "pathfind" не было найдено пути.
IF_PATH_STILL_FINDING = 3 -- Переход выполняется, если текущий запрос "pathfind" еще не завершен.
IF_IS_HIDDEN = 4 -- Переход выполняется, если последняя операция "hide" завершилась успешно, и расстояние до точки укрытия мало.
IF_CAN_HIDE = 5 -- Переход выполняется, если последняя операция "hide" завершилась успешно.
IF_CANNOT_HIDE = 6 -- Переход выполняется, если последняя операция "hide" завершилась неудачно.
IF_STANCE_IS = 7 -- Переход выполняется, если поза этого PipeUser совпадает со значением, указанным в качестве пятого аргумента.
IF_FIRE_IS = 8 -- Переход выполняется, если аргумент последней операции "firecmd" для этого PipeUser совпадает со значением, указанным в качестве пятого аргумента.
IF_HAS_FIRED = 9 -- Переход выполняется, если PipeUser только что выстрелил (флаг выстрела передан актору).
IF_NO_LASTOP = 10 -- Переход выполняется, если LastOpResult равен NULL (например, если "locate" goalOp вернул NULL).
IF_SEES_LASTOP = 11 -- Переход выполняется, если LastOpResult виден отсюда.
IF_SEES_TARGET = 12 -- Переход выполняется, если цель внимания видна отсюда.
IF_CAN_SHOOT_TARGET = 13 -- Переход выполняется, если нет препятствий между оружием и целью внимания (можно стрелять).
IF_CAN_MELEE = 14 -- Переход выполняется, если текущее оружие имеет режим ближнего боя.
IF_NO_ENEMY_TARGET = 15 -- Переход выполняется, если нет цели врага.
IF_PATH_LONGER = 16 -- Переход выполняется, если текущий путь длиннее, чем пятый аргумент.
IF_PATH_SHORTER = 17 -- Переход выполняется, если текущий путь короче, чем пятый аргумент.
IF_PATH_LONGER_RELATIVE = 18 -- Переход выполняется, если текущий путь длиннее, чем (пятый аргумент) умножить на расстояние до желаемого пункта назначения.
IF_NAV_WAYPOINT_HUMAN = 19 -- Переход выполняется, если текущий граф навигации является точечным (используется для проверки на нахождение внутри помещения).
IF_NAV_TRIANGULAR = 20 -- Переход выполняется, если текущий граф навигации является треугольным (используется для проверки нахождения вне помещения).
IF_TARGET_DIST_LESS = 21 -- Переход выполняется, если расстояние до цели меньше.
IF_TARGET_DIST_LESS_ALONG_PATH = 22 -- Переход выполняется, если расстояние до цели вдоль пути меньше.
IF_TARGET_DIST_GREATER = 23 -- Переход выполняется, если расстояние до цели больше.
IF_TARGET_IN_RANGE = 24 -- Переход выполняется, если расстояние до цели меньше, чем радиус атаки.
IF_TARGET_OUT_OF_RANGE = 25 -- Переход выполняется, если расстояние до цели больше, чем радиус атаки.
IF_TARGET_TO_REFPOINT_DIST_LESS = 26 -- Переход выполняется, если расстояние между целью и контрольной точкой меньше.
IF_TARGET_TO_REFPOINT_DIST_GREATER = 27 -- Переход выполняется, если расстояние между целью и контрольной точкой больше.
IF_TARGET_LOST_TIME_MORE = 28 -- Переход выполняется, если время потери цели больше.
IF_TARGET_LOST_TIME_LESS = 29 -- Переход выполняется, если время потери цели меньше.
IF_LASTOP_DIST_LESS = 30 -- Переход выполняется, если расстояние до результата последней операции меньше.
IF_LASTOP_DIST_LESS_ALONG_PATH = 31 -- Переход выполняется, если расстояние до результата последней операции вдоль пути меньше.
IF_TARGET_MOVED_SINCE_START = 32 -- Переход выполняется, если расстояние между текущим положением цели и ее положением при начале выполнения конвейера больше, чем порог.
IF_TARGET_MOVED = 33 -- Переход выполняется, если расстояние между текущим положением цели и ее положением при начале выполнения конвейера (или последним разом, когда оно проверялось) больше, чем порог.
IF_EXPOSED_TO_TARGET = 34 -- Переход выполняется, если верхняя часть туловища агента видна в сторону цели.
IF_COVER_COMPROMISED = 35 -- Переход выполняется, если текущее укрытие не может быть использовано для скрытия, или если точки скрытия не существует.
IF_COVER_NOT_COMPROMISED = 36 -- Отрицательная версия IF_COVER_COMPROMISED.
IF_COVER_SOFT = 37 -- Если текущее укрытие является мягким.
IF_COVER_NOT_SOFT = 38 -- Если текущее укрытие не является мягким.
IF_CAN_SHOOT_TARGET_CROUCHED = 39 -- Если цель находится в зоне поражения, присев
IF_COVER_FIRE_ENABLED = 40 -- Если огонь из-за укрытия не доступен
IF_RANDOM = 41 -- Случайный переход с шансом (0.0 до 1.0) из 5го аргумента
IF_LASTOP_FAILED = 42 -- Если последняя операция завершилась неудачно
IF_LASTOP_SUCCEED = 43 -- Если последняя операция завершилась успешно
BRANCH_ALWAYS = 44 -- Безусловный переход.
NOT = 0x100 -- Отрицательный флаг, применяемый в сочетании с другими значениями

-- Типы ИИ объектов

AIOBJECT_NONE = 200
AIOBJECT_DUMMY = 0 -- Фиктивный объект ИИ, не имеющий функциональности.
AIOBJECT_PUPPET = 5 -- Объект ИИ
AIOBJECT_VEHICLE = 7 -- Базовый класс для всех транспортных средств.
AIOBJECT_ATTRIBUTE = 11 -- Например, свет фонарика или луч лазера
AIOBJECT_WAYPOINT = 12
AIOBJECT_SNDSUPRESSOR = 14
AIOBJECT_HELICOPTER = 40
AIOBJECT_CAR = 50
AIOBJECT_BOAT = 60
AIOBJECT_2D_FLY = 80 -- AIOBJECT_ACTOR но с под типом STP_2D_FLY
AIOBJECT_MOUNTEDWEAPON = 90
AIOBJECT_GLOBALALERTNESS = 94
AIOBJECT_ORDER = 96
AIOBJECT_PLAYER = 100
AIOBJECT_GRENADE = 150
AIOBJECT_RPG = 151


AITSR_NONE = 0
AITSR_SEE_STUNT_ACTION = 1
AITSR_SEE_CLOAKED = 2

-- Флаги получения количества единиц ИИ в группе

GROUP_ALL = 1 -- Всех объектов в группе
GROUP_ENABLED = 2 -- Только объектов с включенным ИИ
GROUP_MAX = 4 -- Максимальное кол-во объектов за всё время игры

-- Уведомления группы

GN_INIT = 0 -- Инициализация группы.
GN_MARK_DEFEND_POS = 1 -- Пометить позицию обороны точкой опоры (ref point) вызвавшей функцию сущности
GN_CLEAR_DEFEND_POS = 2 -- Очистить позицию обороны.
GN_AVOID_CURRENT_POS = 3 -- Избегать текущей позиции (ref point) вызвавшей функцию сущности
GN_PREFER_ATTACK = 4 -- Установить поведение по умолчанию, чтобы предпочесть атаку.
GN_PREFER_FLEE = 5 -- Установить поведение по умолчанию, чтобы предпочесть бегство.
GN_NOTIFY_ADVANCING = 6 -- Уведомить группу о том, что она движется вперед.
GN_NOTIFY_COVERING = 7 -- Уведомить группу о том, что она находится в укрытии.
GN_NOTIFY_WEAK_COVERING = 8 -- Уведомить группу о том, что она использует слабое укрытие.
GN_NOTIFY_HIDING = 9 -- Уведомить группу о том, что она скрывается.
GN_NOTIFY_SEEKING = 10 -- Уведомить группу о том, что она ищет.
GN_NOTIFY_ALERTED = 11 -- Уведомить группу о том, что она находится в состоянии боевой готовности.
GN_NOTIFY_UNAVAIL = 12 -- Уведомить группу о том, что цель недоступна.
GN_NOTIFY_IDLE = 13 -- Уведомить группу о том, что она бездействует.
GN_NOTIFY_SEARCHING = 14 -- Уведомить группу о том, что она находится в поиске.
GN_NOTIFY_REINFORCE = 15 -- Уведомить группу о том, что она усиливается.

-- типы запросов, которые можно использовать с функцией AIGroupTactic.GetState()

GE_GROUP_STATE = 0 -- Общее состояние группы, например, “атака”, “оборона”, “отступление” и т.д. Это зависит от конкретной тактики группы, поэтому ее значение нужно изучать в контексте реализации тактики.
GE_UNIT_STATE = 1 -- Состояние конкретного юнита в группе, например, “стрельба”, “бег”, “укрытие” и т.д.
GE_ADVANCE_POS = 2 -- Позиция, к которой движется группа для атаки или преследования.
GE_SEEK_POS = 3 -- Позиция, к которой движется группа в поисках чего-либо.
GE_DEFEND_POS = 4 -- Позиция, где группа должна обороняться.
GE_LEADER_COUNT = 5 -- Количество лидеров в группе.
GE_MOST_LOST_UNIT = 6 -- Самый “потерянный” юнит в группе.
GE_MOVEMENT_SIGNAL = 7 -- Сигнал движения, который группа использует для координации своих действий.
GE_NEAREST_SEEK = 8 -- Позиция ближайшего объекта, к которому группа должна двигаться.

-- Состояние группы
GS_IDLE = 0 -- Группа находится в состоянии покоя, не предпринимая никаких действий.
GS_ALERTED = 1 -- Группа находится в состоянии повышенной бдительности, например, после того, как был замечен враг.
GS_COVER = 2 -- Группа ищет укрытие, чтобы защититься от атаки.
GS_ADVANCE = 3 -- Группа движется вперед к цели.
GS_SEEK = 4 -- Группа ищет что-то, например, врага или объект.
GS_SEARCH = 5 -- Группа активно исследует окружающую среду в поисках чего-то.

-- Тип персонажа в группе
GU_HUMAN_CAMPER = 0
GU_HUMAN_COVER = 1
GU_HUMAN_SNEAKER = 2
GU_HUMAN_LEADER = 3
GU_HUMAN_SNEAKER_SPECOP = 4
GU_ALIEN_MELEE = 5
GU_ALIEN_ASSAULT = 6
GU_ALIEN_MELEE_DEFEND = 7
GU_ALIEN_ASSAULT_DEFEND = 8
GU_ALIEN_EVADE = 9

-- Фильтры для поиска группы
-- Примеры:
--  AI.GetNearestEntitiesOfType( targetPos, AIOBJECT_VEHICLE, 10, objects ,AIOBJECTFILTER_INCLUDEINACTIVE);
--  BasicAI:FormGroup(entity,entity.myTeamList, numPeople, AI:GetGroupOf(entity.id), AIOBJECTFILTER_SAMESPECIES + AIOBJECTFILTER_SAMEGROUP);
AIOBJECTFILTER_SAMESPECIES = 1
AIOBJECTFILTER_SAMEGROUP = 2
AIOBJECTFILTER_NOGROUP = 4
AIOBJECTFILTER_INCLUDEINACTIVE = 8

-- Классы ИИ Персонажей (наверное не используется)
UNIT_CLASS_UNDEFINED = 1
UNIT_CLASS_LEADER = 2
UNIT_CLASS_INFANTRY = 4
UNIT_CLASS_SCOUT = 8
UNIT_CLASS_ENGINEER = 16
UNIT_CLASS_MEDIC = 32
UNIT_CLASS_CIVILIAN = 64
UNIT_CLASS_COMPANION = 128
SHOOTING_SPOT_POINT = 32768

-- id для сигналов
AISIGNAL_INCLUDE_DISABLED = 0 -- Этот id позволяет сигналу быть обработанным даже если объект AI (CAIActor) отключен (disabled).
AISIGNAL_DEFAULT = 1 -- Это базовый id для сигнала, который не имеет дополнительных настроек.
AISIGNAL_PROCESS_NEXT_UPDATE = 3 -- Этот id откладывает обработку сигнала до следующего обновления (update) системы AI.
AISIGNAL_NOTIFY_ONLY = 9 -- Этот id создает сигнал, который только уведомляет объект AI о событии, но не обрабатывается им.
AISIGNAL_ALLOW_DUPLICATES = 10 -- Этот id позволяет дублирующиеся сигналы (с одинаковыми именами и CRC) добавляться в очередь обработки.

-- Флаги для goalOp 'lookat'
AI_LOOKAT_CONTINUOUS = 1
AI_LOOKAT_USE_BODYDIR = 2

-- ФЛАГИ ВЫПОЛНЕНИЯ AI ACTION ЧЕРЕЗ AITracker
IGNORE_COMBAT_DURING_ACTION = 1
JOIN_COMBAT_PAUSE_ACTION = 2

-- Флаги для GoalOp
AILASTOPRES_USE = 1 -- Использовать результат последней операции ( SelectPipe и InsertSubpipe передают сюда targetId)
AILASTOPRES_LOOKAT = 2 -- Использовать последний объект, на который персонаж смотрел.
AI_LOOK_FORWARD = 4
AI_MOVE_BACKWARD = 8
AI_MOVE_RIGHT = 16
AI_MOVE_LEFT = 32
AI_MOVE_FORWARD = 64
AI_MOVE_BACKRIGHT = 128
AI_MOVE_BACKLEFT = 256
AI_MOVE_TOWARDS_GROUP = 512 -- Двигаться в сторону группы.
AI_REQUEST_PARTIAL_PATH = 1024 --  Запросить частичный путь для достижения цели (подходит в случае, когда цель двигается или окружение динамично)
AI_BACKOFF_FROM_TARGET = 2048 -- Уклоняться от цели.
AI_BREAK_ON_LIVE_TARGET = 4096
AI_RANDOM_ORDER = 8192 -- Использовать случайный порядок направления уклонения.
AI_CONSTANT_SPEED = 8192
AI_USE_TIME = 16384 -- Указать время, а не расстояние для приближения.
AI_STOP_ON_ANIMATION_START = 32768 -- Останавливать движение, когда персонаж начинает анимацию.
AI_USE_TARGET_MOVEMENT = 65536
AI_ADJUST_SPEED = 131072 -- Автоматически изменять скорость движения в зависимости от расстояния до цели
AI_CHECK_SLOPE_DISTANCE = 262144 -- Учитывать наклон местности при определении расстояния уклонения.

BODYPOS_PRONE = 2
BODYPOS_CROUCH = 1
BODYPOS_STAND = 0
BODYPOS_RELAX = 3
BODYPOS_STEALTH = 4

-- Стандартные флаги приоритета для InsertSubpipe
AIGOALPIPE_LOOP = 0 -- Повторять цикл до завершения.
AIGOALPIPE_RUN_ONCE = 1 -- Выполнить один раз.
AIGOALPIPE_NOTDUPLICATE = 2 -- Не запускать, если уже запущен.
AIGOALPIPE_HIGHPRIORITY = 4 -- Высокий приоритет.
AIGOALPIPE_SAMEPRIORITY = 8 --  Такой же приоритет, как у предыдущего.
AIGOALPIPE_DONT_RESET_AG = 16 -- Не сбрасывать AnimGraph? или Агрессию?.
AIGOALPIPE_KEEP_LAST_SUBPIPE = 32 --  Сохранять последний подканал. 

-- Режимы стрельбы ИИ
FIREMODE_OFF = 0 -- Не стрелять.
FIREMODE_BURST = 1 -- Стрелять очередями - только по живым целям.
FIREMODE_CONTINUOUS = 2 -- Стрелять непрерывно - только по живым целям.
FIREMODE_FORCED = 3 -- Стрелять непрерывно - допускаются любые цели.
FIREMODE_AIM = 4 -- Целиться в цель - допускаются любые цели.
FIREMODE_SECONDARY = 5 -- Стрелять из вторичного оружия (гранаты, ...).
FIREMODE_SECONDARY_SMOKE = 6 -- Стрелять дымовой гранатой.
FIREMODE_MELEE = 7 -- Ближний бой.
FIREMODE_KILL = 8 -- Без промахов, стрелять прямо в цель, независимо от агрессии, дальности атаки, точности.
FIREMODE_BURST_WHILE_MOVING = 9 -- (Переименовать.
FIREMODE_PANIC_SPREAD = 10 --  
FIREMODE_BURST_DRAWFIRE = 11 --  
FIREMODE_MELEE_FORCED = 12 -- Ближний бой без ограничений по дальности.
FIREMODE_BURST_SNIPE = 13 --  
FIREMODE_AIM_SWEEP = 14 --  

STICK_BREAK = 1 -- Прерывает движение после достижения цели.
STICK_SHORTCUTNAV = 2 -- Использует сокращенные пути для достижения цели

-- Флаги аксессуаров оружия 
-- Пример: 
    -- AI.EnableWeaponAccessory(entity.id, AIWEPA_LASER, true);
AIWEPA_LASER = 1
AIWEPA_COMBAT_LIGHT = 2
AIWEPA_PATROL_LIGHT = 4

-- Параметры ИИ агента
AIPARAM_SIGHTRANGE = 1
AIPARAM_ATTACKRANGE = 2
AIPARAM_ACCURACY = 3
AIPARAM_GROUPID = 4
AIPARAM_FOVPRIMARY = 5
AIPARAM_FOVSECONDARY = 6
AIPARAM_COMMRANGE = 7
AIPARAM_FWDSPEED = 8
AIPARAM_SPECIES = 9
AIPARAM_STRAFINGPITCH = 16
AIPARAM_COMBATCLASS = 17
AIPARAM_INVISIBLE = 18
AIPARAM_PERCEPTIONSCALE_VISUAL = 19
AIPARAM_PERCEPTIONSCALE_AUDIO = 20
AIPARAM_CLOAK_SCALE = 21
AIPARAM_FORGETTIME_TARGET = 22
AIPARAM_FORGETTIME_SEEK = 23
AIPARAM_FORGETTIME_MEMORY = 24
AIPARAM_LOOKIDLE_TURNSPEED = 25
AIPARAM_LOOKCOMBAT_TURNSPEED = 26
AIPARAM_AIM_TURNSPEED = 27
AIPARAM_FIRE_TURNSPEED = 28
AIPARAM_MELEE_DISTANCE = 29
AIPARAM_MIN_ALARM_LEVEL = 30
AIPARAM_SIGHTENVSCALE_NORMAL = 31
AIPARAM_SIGHTENVSCALE_ALARMED = 32

-- События ИИ Агента
AIEVENT_ONBODYSENSOR = 1
AIEVENT_ONVISUALSTIMULUS = 2
AIEVENT_AGENTDIED = 5 -- этот агент мертв, удаляется из всех групп, ИИ выключен 
AIEVENT_SLEEP = 6 -- выключается стрельба, ИИ выключен
AIEVENT_WAKEUP = 7 -- чистка активных goalOp, сброс цели внимания, ИИ включен
AIEVENT_ENABLE = 8 -- ИИ включен
AIEVENT_DISABLE = 9 -- ИИ выключен
AIEVENT_REJECT = 10
AIEVENT_PATHFINDON = 11
AIEVENT_PATHFINDOFF = 12
AIEVENT_CLEAR = 15 -- чистка активных goalOp, сброс цели внимания, может принимать сигналы, ИИ включен
AIEVENT_DROPBEACON = 17 -- обновить маяк для GroupId
AIEVENT_USE = 19
AIEVENT_CLEARACTIVEGOALS = 22 -- чистка активных goalOp
AIEVENT_DRIVER_IN = 23 -- включает ИИ транспорта
AIEVENT_DRIVER_OUT = 24 -- выключает ИИ транспорта
AIEVENT_FORCEDNAVIGATION = 25

AITARGET_NONE = 0
AITARGET_SOUND = 1
AITARGET_MEMORY = 2
AITARGET_ENEMY = 4
AITARGET_FRIENDLY = 5
AITARGET_BEACON = 6
AITARGET_GRENADE = 7
AITARGET_RPG = 8

-- Типы блокировщиков пути (Поиск пути будет избегать объекты с данными типами в определенном радиусе)
PFB_NONE = 0 -- поиск пути не будет избегать 
PFB_ATT_TARGET = 1 -- поиск пути будет избегать цели внимания
PFB_REF_POINT = 2 -- поиск пути будет избегать точки опоры
PFB_BEACON = 3 -- поиск пути будет избегать ИИ-маяк для группы
PFB_DEAD_BODIES = 4 -- поиск пути будет избегать мертвые тела
PFB_EXPLOSIVES = 5 -- поиск пути будет избегать взрывчатку
PFB_PLAYER = 6 -- поиск пути будет избегать игрока
PFB_BETWEEN_NAV_TARGET = 7 -- точка между текущей и следующей точкой навигации.

-- Фильтры сигналов
--[[
    SIGNALFILTER_SENDER = 0
    -- Сигнал отправляется от текущего объекта ИИ.
    -- Полезно для отправки внутренних сигналов.
]]
SIGNALFILTER_SENDER = 0

--[[
    SIGNALFILTER_LASTOP = 1
    -- Сигнал отправляется от последнего оператора этого объекта ИИ.
    -- Полезно для отправки сигналов объекту, который последним его управлял.
]]
SIGNALFILTER_LASTOP = 1

--[[
    SIGNALFILTER_GROUPONLY = 2
    -- Сигнал отправляется только членам той же группы, что и объект ИИ.
    -- Используется для связи внутри группы.
]]
SIGNALFILTER_GROUPONLY = 2

--[[
    SIGNALFILTER_SPECIESONLY = 3
    -- Сигнал отправляется только объектам того же вида, что и объект ИИ.
    -- Используется для связи между объектами одного вида.
]]
SIGNALFILTER_SPECIESONLY = 3

--[[
    SIGNALFILTER_ANYONEINCOMM = 4
    -- Сигнал отправляется любому объекту, находящемуся в радиусе связи объекта ИИ.
    -- Используется для широковещательной рассылки сообщений в определенном радиусе.
]]
SIGNALFILTER_ANYONEINCOMM = 4

--[[
    SIGNALFILTER_TARGET = 5
    -- Сигнал отправляется текущей цели объекта ИИ.
    -- Используется для связи с атакуемым объектом.
]]
SIGNALFILTER_TARGET = 5

--[[
    SIGNALFILTER_SUPERGROUP = 6
    -- Сигнал отправляется всем членам супер-группы (родительской группы) объекта ИИ.
    -- Полезно для связи с более крупной группой.
]]
SIGNALFILTER_SUPERGROUP = 6

--[[
    SIGNALFILTER_SUPERSPECIES = 7
    -- Сигнал отправляется всем объектам того же вида, что и объект ИИ, вне зависимости от их группы.
    -- Используется для широковещательной связи внутри вида.
]]
SIGNALFILTER_SUPERSPECIES = 7

--[[
    SIGNALFILTER_SUPERTARGET = 8
    -- Сигнал отправляется цели цели объекта ИИ.
    -- Используется для связи с объектом, находящимся дальше по цепочке целей.
]]
SIGNALFILTER_SUPERTARGET = 8

--[[
    SIGNALFILTER_NEARESTGROUP = 9
    -- Сигнал отправляется ближайшему члену той же группы, что и объект ИИ.
    -- Полезно для связи с определенным членом группы.
]]
SIGNALFILTER_NEARESTGROUP = 9

--[[
    SIGNALFILTER_NEARESTINCOMM = 11
    -- Сигнал отправляется ближайшему объекту в радиусе связи объекта ИИ.
    -- Используется для связи с определенным объектом в радиусе связи.
]]
SIGNALFILTER_NEARESTINCOMM = 11

--[[
    SIGNALFILTER_HALFOFGROUP = 12
    -- Сигнал отправляется половине членов той же группы, что и объект ИИ, исключая самого объекта ИИ.
    -- Используется для отправки сигналов подмножеству группы.
]]
SIGNALFILTER_HALFOFGROUP = 12

--[[
    SIGNALFILTER_LEADER = 13
    -- Сигнал отправляется лидеру группы объекта ИИ.
    -- Используется для связи с лидером группы.
]]
SIGNALFILTER_LEADER = 13

--[[
    SIGNALFILTER_GROUPONLY_EXCEPT = 14
    -- Сигнал отправляется всем членам той же группы, что и объект ИИ, кроме самого объекта ИИ.
    -- Используется для отправки сигналов всей группе, исключая отправителя.
]]
SIGNALFILTER_GROUPONLY_EXCEPT = 14

--[[
    SIGNALFILTER_LEADERENTITY = 16
    -- Сигнал отправляется объекту, управляемому лидером группы.
    -- Полезно для связи с физическим объектом, управляемым лидером группы.
]]
SIGNALFILTER_LEADERENTITY = 16

--[[
    SIGNALFILTER_NEARESTINCOMM_SPECIES = 17
    -- Сигнал отправляется ближайшему объекту того же вида, что и объект ИИ, находящемуся в радиусе связи.
    -- Используется для связи с определенным объектом в радиусе связи и того же вида.
]]
SIGNALFILTER_NEARESTINCOMM_SPECIES = 17

--[[
    SIGNALFILTER_NEARESTINCOMM_LOOKING = 18
    -- Сигнал отправляется ближайшему объекту в радиусе связи, который смотрит на объект ИИ.
    -- Используется для связи с объектом в радиусе связи, который визуально видит отправителя.
]]
SIGNALFILTER_NEARESTINCOMM_LOOKING = 18

--[[
    SIGNALFILTER_FORMATION = 19
    -- Сигнал отправляется всем объектам, которые являются частью формации объекта ИИ.
    -- Используется для связи с членами определенной формации.
]]
SIGNALFILTER_FORMATION = 19

--[[
    SIGNALFILTER_FORMATION_EXCEPT = 20
    -- Сигнал отправляется всем объектам, которые являются частью формации объекта ИИ, за исключением самого объекта ИИ.
    -- Используется для связи с членами определенной формации, исключая отправителя.
]]
SIGNALFILTER_FORMATION_EXCEPT = 20

GO = {
    --[[
    Эта функция позволяет виртуальному персонажу (кукле) использовать анимационные цели, например, 
    при взаимодействии с SmartObject.

    -- AI.PushGoal("animtarget", blocking, mode, name, start radius, direction tolerance, target radius)

    mode - режим анимации (AIANIM_SIGNAL - для однократной анимации, AIANIM_ACTION - для циклической анимации).
    name - имя анимации (сигнала или действия), которую нужно использовать.
    start radius - радиус, в пределах которого персонаж должен находиться от цели, чтобы начать анимацию.
    direction tolerance - допустимый угол отклонения от направления на цель, при котором персонаж может начать анимацию.
    target radius - радиус цели (используется для определения точного местоположения, где персонаж должен завершить анимацию).
    ]]
    ANIMTARGET = "animtarget",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) установить определенный объект как цель внимания.

    -- AI.PushGoal("acqtarget", blocking, AI object's name or a special AI object)

    AI object's name or a special AI object - имя или тип объекта, который нужно установить как цель внимания. Пустая строка ("") означает "использовать результат предыдущей операции".
    ]]
    ACQUIRE_TARGET = "acqtarget",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) корректировать свою позицию для стрельбы или укрытия, учитывая наличие препятствий.

    -- AI.PushGoal("adjustaim", blocking, aim / hide, use LastOp as backup, allow prone, timeout)

    aim / hide - использовать ли корректировку для стрельбы (0) или для укрытия (1).
    use LastOp as backup - использовать ли последний объект, на который персонаж смотрел, как резервную цель (по умолчанию 0, то есть не использовать).
    allow prone - разрешать ли персонажу ложиться (приседать) (по умолчанию 0, то есть не разрешать).
    timeout - максимальное время для выполнения корректировки позиции (по умолчанию 0, то есть нет ограничения по времени).
    ]]
    ADJUST_AIM = "adjustaim",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) воспроизвести определенную анимацию.

    -- AI.PushGoal("animation", blocking, mode, name)
    
    mode - режим анимации (AIANIM_SIGNAL - для однократной анимации, AIANIM_ACTION - для циклической анимации).
    name - имя анимации (сигнала или действия), которую нужно воспроизвести.
    ]]
    ANIMATION = "animation",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) приблизиться к заданной цели на определенное расстояние.
    Функция approach использует функции Pathfind и Trace для поиска пути и определения расстояния до цели.
    
    -- AI.PushGoal("approach", blocking, approach distance or time, flags, end accuracy)

    approach distance or time - расстояние, на которое нужно приблизиться, или время, которое нужно затратить на приближение (используйте флаг AI_USE_TIME, чтобы указать время, а не расстояние).
    flags - дополнительные флаги для управления поведением, описанные ниже:
        AILASTOPRES_USE: Использовать последний объект, на который персонаж смотрел, как цель.
        AILASTOPRES_LOOKAT: Смотреть на последний объект, на который персонаж смотрел.
        AI_USE_TIME: Указать время, а не расстояние для приближения.
        AI_REQUEST_PARTIAL_PATH: Запросить частичный путь для достижения цели.
        AI_STOP_ON_ANIMATION_START: Останавливать движение, когда персонаж начинает анимацию.
    end accuracy - допустимая погрешность в достижении заданного расстояния (по умолчанию 0).
    ]]
    APPROACH = "approach",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) сделать небольшое уклонение – отступить назад, влево, вправо и т.д.
    
    --AI.PushGoal("backoff", blocking, distance, max duration, options)

    distance - расстояние, на которое персонаж должен уклониться.
    max duration - максимальная продолжительность уклонения (по умолчанию 0, то есть нет ограничения по времени).
    options - дополнительные параметры, которые определяют направление уклонения, описанные ниже:
        AILASTOPRES_USE: Использовать последний объект, на который персонаж смотрел, для определения направления уклонения.
        AI_MOVE_FORWARD: Двигаться вперед.
        AI_MOVE_BACKWARD: Двигаться назад.
        AI_MOVE_LEFT: Двигаться влево.
        AI_MOVE_RIGHT: Двигаться вправо.
        AI_MOVE_BACKLEFT: Двигаться назад влево.
        AI_MOVE_BACKRIGHT: Двигаться назад вправо.
        AI_BACKOFF_FROM_TARGET: Уклоняться от цели.
        AI_MOVE_TOWARDS_GROUP: Двигаться в сторону группы.
        AI_CHECK_SLOPE_DISTANCE: Учитывать наклон местности при определении расстояния уклонения.
        AI_RANDOM_ORDER: Использовать случайный порядок направления уклонения.

    AI.PushGoal("backoff", 1, 2, 0, AI_MOVE_RIGHT); - Заставляет персонажа уклониться вправо на 2 единицы.
    AI.PushGoal("backoff", 1, 2, 1, AI_BACKOFF_FROM_TARGET); - Заставляет персонажа уклониться от цели в течение 1 секунды.
    AI.PushGoal("backoff", 1, 16, 10, AI_MOVE_BACKWARD + AI_MOVE_BACKRIGHT + AI_MOVE_BACKLEFT + AI_MOVE_LEFT + AI_MOVE_RIGHT); - Заставляет персонажа уклониться назад, влево, вправо, назад влево, назад вправо на 16 единиц в течение 10 секунд.
    AI.PushGoal("backoff", 1, 6, 4, AI_BACKOFF_FROM_TARGET + AI_MOVE_LEFT + AI_MOVE_RIGHT + AI_MOVE_BACKWARD + AI_MOVE_BACKLEFT + AI_MOVE_BACKRIGHT + AI_RANDOM_ORDER); - Заставляет персонажа уклониться от цели, двигаясь влево, вправо, назад, назад влево, назад вправо в течение 4 секунд.
    AI.PushGoal("backoff", 1, 14, 8, AILASTOPRES_USE + AI_MOVE_BACKWARD + AI_MOVE_BACKLEFT + AI_MOVE_BACKRIGHT); - Заставляет персонажа уклониться от последнего объекта, на который он смотрел, двигаясь назад, назад влево, назад вправо в течение 8 секунд.
    ]]
    BACKOFF = "backoff",

    --[[
    -- AI.PushGoal("bodypos", blocking, body state, delayed)

    body state - позу персонажа, описанную ниже:
        STANCE_NULL: Без позы.
        STANCE_STAND: Стоять.
        STANCE_CROUCH: Присесть.
        STANCE_PRONE: Лечь.
        STANCE_RELAXED: Расслабленная поза.
        STANCE_STEALTH: Режим скрытности.
        STANCE_ALERTED: Поза готовности.
        STANCE_SWIM: Плавание.
        STANCE_ZEROG: Невесомость.
    delayed - использовать ли задержку перед принятием позы (по умолчанию false).

    AI.PushGoal("bodypos", 1, BODYPOS_STEALTH); - Заставляет персонажа принять режим скрытности.
    AI.PushGoal("bodypos", 1, BODYPOS_STAND, true); - Заставляет персонажа встать, с задержкой.
    ]]
    BODYPOS = "bodypos",

    --[[
    Выполняет переход на метку targetLabel

    -- AI.PushGoal("branch", blocking, targetLabel, branchType, optionalArgument)

    targetLabel - метка, к которой нужно перейти, если условие выполняется.
    blocking - блокировать ли выполнение других команд во время проверки условия (по умолчанию false).
    branchType - тип условия, описанный в EOPBranchType.
    optionalArgument - дополнительный аргумент, используемый для некоторых типов условий (см. EOPBranchType).
        IF_ACTIVE_GOALS: Переход выполняется, если есть активные (не завершенные) операции целей.
        IF_ACTIVE_GOALS_HIDE: Переход выполняется, если есть активные цели или не было найдено укрытие.
        IF_NO_PATH: Переход выполняется, если в последней операции "pathfind" не было найдено пути.
        IF_PATH_STILL_FINDING: Переход выполняется, если текущий запрос "pathfind" еще не завершен.
        IF_IS_HIDDEN: Переход выполняется, если последняя операция "hide" завершилась успешно, и расстояние до точки укрытия мало.
        IF_CAN_HIDE: Переход выполняется, если последняя операция "hide" завершилась успешно.
        IF_CANNOT_HIDE: Переход выполняется, если последняя операция "hide" завершилась неудачно.
        IF_STANCE_IS: Переход выполняется, если поза этого PipeUser совпадает со значением, указанным в качестве optionalArgument.
        IF_FIRE_IS: Переход выполняется, если аргумент последней операции "firecmd" для этого PipeUser совпадает со значением, указанным в качестве optionalArgument.
        IF_HAS_FIRED: Переход выполняется, если PipeUser только что выстрелил (флаг выстрела передан актору).
        IF_NO_LASTOP: Переход выполняется, если m_pLastOpResult равен NULL (например, если "locate" goalOp вернул NULL).
        IF_SEES_LASTOP: Переход выполняется, если m_pLastOpResult виден отсюда.
        IF_SEES_TARGET: Переход выполняется, если цель внимания видна отсюда.
        IF_CAN_SHOOT_TARGET: Переход выполняется, если нет препятствий между оружием и целью внимания (можно стрелять).
        IF_CAN_MELEE: Переход выполняется, если текущее оружие имеет режим ближнего боя.
        IF_NO_ENEMY_TARGET: Переход выполняется, если нет цели врага.
        IF_PATH_LONGER: Переход выполняется, если текущий путь длиннее, чем optionalArgument.
        IF_PATH_SHORTER: Переход выполняется, если текущий путь короче, чем optionalArgument.
        IF_PATH_LONGER_RELATIVE: Переход выполняется, если текущий путь длиннее, чем (optionalArgument) умножить на расстояние до желаемого пункта назначения.
        IF_NAV_WAYPOINT_HUMAN: Переход выполняется, если текущий граф навигации является точечным (используется для проверки нахождения внутри помещения).
        IF_NAV_TRIANGULAR: Переход выполняется, если текущий граф навигации является треугольным (используется для проверки нахождения вне помещения).
        IF_TARGET_DIST_LESS: Переход выполняется, если расстояние до цели меньше, чем optionalArgument.
        IF_TARGET_DIST_LESS_ALONG_PATH: Переход выполняется, если расстояние до цели вдоль пути меньше, чем optionalArgument.
        IF_TARGET_DIST_GREATER: Переход выполняется, если расстояние до цели больше, чем optionalArgument.
        IF_TARGET_IN_RANGE: Переход выполняется, если расстояние до цели меньше, чем радиус атаки.
        IF_TARGET_OUT_OF_RANGE: Переход выполняется, если расстояние до цели больше, чем радиус атаки.
        IF_TARGET_TO_REFPOINT_DIST_LESS: Переход выполняется, если расстояние между целью и контрольной точкой меньше, чем optionalArgument.
        IF_TARGET_TO_REFPOINT_DIST_GREATER: Переход выполняется, если расстояние между целью и контрольной точкой больше, чем optionalArgument.
        IF_TARGET_LOST_TIME_MORE: Переход выполняется, если время потери цели больше, чем optionalArgument.
        IF_TARGET_LOST_TIME_LESS: Переход выполняется, если время потери цели меньше, чем optionalArgument.
        IF_LASTOP_DIST_LESS: Переход выполняется, если расстояние до результата последней операции меньше, чем optionalArgument.
        IF_LASTOP_DIST_LESS_ALONG_PATH: Переход выполняется, если расстояние до результата последней операции вдоль пути меньше, чем optionalArgument.
        IF_TARGET_MOVED_SINCE_START: Переход выполняется, если расстояние между текущим положением цели и ее положением при начале выполнения конвейера больше, чем порог.
        IF_TARGET_MOVED: Переход выполняется, если расстояние между текущим положением цели и ее положением при начале выполнения конвейера (или последним разом, когда оно проверялось) больше, чем порог.
        IF_EXPOSED_TO_TARGET: Переход выполняется, если верхняя часть туловища агента видна в сторону цели.
        IF_COVER_COMPROMISED: Переход выполняется, если текущее укрытие не может быть использовано для скрытия, или если точки скрытия не существует.
        IF_COVER_NOT_COMPROMISED: Отрицательная версия IF_COVER_COMPROMISED.
        IF_COVER_SOFT: Если текущее укрытие является мягким.
        IF_COVER_NOT_SOFT: Если текущее укрытие не является мягким.
        IF_CAN_SHOOT_TARGET_PRONED: Если цель находится в зоне поражения, лежа на животе
        IF_CAN_SHOOT_TARGET_CROUCHED: Если цель находится в зоне поражения, присев
        IF_CAN_SHOOT_TARGET_STANDING: Если цель находится в зоне поражения, стоя
        IF_COVER_FIRE_ENABLED: Если огонь из-за укрытия не доступен
        IF_RANDOM: Случайный переход
        IF_LASTOP_FAILED: Если последняя операция завершилась неудачно
        IF_LASTOP_SUCCEED: Если последняя операция завершилась успешно
        BRANCH_ALWAYS: Безусловный переход.
        NOT: Отрицательный флаг, применяемый в сочетании с другими значениями
    ]]
    BRANCH = "branch",
    
    CLEAR = "clear", -- AI.PushGoal("clear", 0, 0); -- stops approaching - 0 means keep att. target 1 means forget att. target

    --[[
    Эта функция позволяет временно понизить приоритет определенной цели, 
    чтобы она не была выбрана как цель внимания (и/или не участвовала в других расчетах).

    -- AI.PushGoal("devalue", blocking, also devalue puppets, clear [the list of] "devalued")

    also devalue puppets - понизить ли приоритет не только у точки, но и у персонажей, связанных с этой точкой (по умолчанию false).
    clear [the list of] "devalued" - очистить ли список пониженных целей (по умолчанию false).

    AI.CreateGoalPipe("tank_fire_stop");
    AI.PushGoal("tank_fire_stop", "firecmd", 0, 0);
    AI.PushGoal("tank_fire_stop", "locate", 0, "atttarget");
    AI.PushGoal("tank_fire_stop", "lookat", 0, 0, 0, true, 1);
    --
    AI.PushGoal("tank_fire_stop", "devalue", 1, 0, 1);
    ]]
    DEVALUE = "devalue",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) управлять режимом стрельбы.

    -- AI.PushGoal("firecmd", blocking, fire mode, use LastOp, min timeout, max timeout)

    use LastOp - использовать ли последний объект, на который персонаж смотрел, как цель (по умолчанию 0).
    min timeout - минимальное время ожидания между выстрелами (по умолчанию -1, то есть нет времени ожидания).
    max timeout - максимальное время ожидания между выстрелами (по умолчанию -1, то есть нет времени ожидания).
    fire mode - режим стрельбы, описанный ниже:
        FIREMODE_OFF: Не стрелять.
        FIREMODE_BURST: Стрелять очередями.
        FIREMODE_CONTINUOUS: Стрелять непрерывно.
        FIREMODE_FORCED: Стрелять непрерывно, разрешая любые цели.
        FIREMODE_AIM: Прицеливаться только в цель.
        FIREMODE_SECONDARY: Стрелять из вторичного оружия (гранаты и т.д.).
        FIREMODE_SECONDARY_SMOKE: Стрелять дымовой гранатой.
        FIREMODE_MELEE: Использовать рукопашный бой.
        FIREMODE_KILL: Стрелять прямо в цель, игнорируя ограничения по агрессии, дальности стрельбы и точности.
        FIREMODE_BURST_WHILE_MOVING: Стрелять очередями, двигаясь.
        FIREMODE_PANIC_SPREAD: Стрелять с разбросом в панике.
        FIREMODE_BURST_DRAWFIRE: Стрелять очередями, привлекая огонь.
        FIREMODE_MELEE_FORCED: Использовать рукопашный бой, без ограничений по расстоянию.
        FIREMODE_BURST_SNIPE: Стрелять очередями, как снайпер.
        FIREMODE_AIM_SWEEP: Прицеливаться и вести огонь по площади.
        FIREMODE_BURST_ONCE: Выстрелить одну очередь.
    ]]
    FIRECMD = "firecmd",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) следовать по заранее заданному пути, созданному дизайнером.

    -- AI.PushGoal("followpath", blocking, pathfind to start, reverse path, start nearest, number of loops, end accuracy, use point list, avoid dynamic obstacles)

    pathfind to start - использовать ли поиск пути для достижения начала пути (по умолчанию false, то есть персонаж идет к началу пути по прямой).
    reverse path - следовать ли по пути в обратном направлении (по умолчанию false).
    start nearest - начинать ли движение с ближайшей точки пути (по умолчанию false, то есть персонаж начинает с первой точки пути).
    number of loops - количество повторений пути (по умолчанию 1).
    end accuracy - допустимая погрешность в достижении конца пути (по умолчанию 0).
    use point list - использовать ли список точек для следования по пути (по умолчанию false).
    avoid dynamic obstacles - избегать ли динамические препятствия, например, движущиеся объекты (по умолчанию false).

    AI.PushGoal("tankclose_gotopathsub", "followpath", 0, false, bReverse, true, 3, 0, false); - Заставляет персонажа следовать по пути в обратном направлении, используя ближайшую точку пути, три раза, 
    не используя список точек и не избегая динамические препятствия.
    AI.PushGoal("heliDamageAction", "followpath", 0, false, false, false, 0, 10, true); - Заставляет персонажа следовать 
    по пути, используя список точек, не избегая динамические препятствия, с точностью до 10 единиц.
    ]]
    FOLLOWPATH = "followpath",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) найти и переместиться в укрытие, удовлетворяющее заданным критериям.

    -- AI.PushGoal("hide", blocking, search distance, hide method, exactly, min distance, look at LastOp)

    search distance - максимальное расстояние, на котором персонаж будет искать укрытие (по умолчанию 10).
    hide method - метод поиска укрытия, описанный ниже:
        HM_NEAREST: Найти ближайшее укрытие.
        HM_NEAREST_TO_ME: Найти ближайшее укрытие ко мне.
        HM_FARTHEST_FROM_TARGET: Найти укрытие, максимально удаленное от цели.
        HM_NEAREST_TO_TARGET: Найти укрытие, максимально близкое к цели.
        HM_NEAREST_BACKWARDS: Найти ближайшее укрытие позади меня.
        HM_NEAREST_TOWARDS_TARGET: Найти ближайшее укрытие, находящееся в направлении цели.
        HM_FARTHEST_FROM_GROUP: Найти укрытие, максимально удаленное от группы.
        HM_NEAREST_TO_GROUP: Найти укрытие, максимально близкое к группе.
        HM_LEFTMOST_FROM_TARGET: Найти укрытие, максимально слева от цели.
        HM_RIGHTMOST_FROM_TARGET: Найти укрытие, максимально справа от цели.
        HM_RANDOM: Найти случайное укрытие.
        HM_FRONTLEFTMOST_FROM_TARGET: Найти укрытие, максимально слева от цели и спереди.
        HM_FRONTRIGHTMOST_FROM_TARGET: Найти укрытие, максимально справа от цели и спереди.
        HM_NEAREST_TO_FORMATION: Найти укрытие, максимально близкое к формации.
        HM_FARTHEST_FROM_FORMATION: Найти укрытие, максимально удаленное от формации.
        HM_NEAREST_TO_LASTOPRESULT: Найти укрытие, максимально близкое к результату последнего действия.
        HM_RANDOM_AROUND_LASTOPRESULT: Найти случайное укрытие вокруг результата последнего действия.
        HM_USEREFPOINT: Использовать точку “refpoint” как укрытие.
        HM_ASKLEADER: Спросить у лидера о подходящем укрытии.
        HM_ASKLEADER_NOSAME: Спросить у лидера о подходящем укрытии, отличном от того, которое использовал лидер.
        HM_BEHIND_VEHICLES: Найти укрытие за транспортным средством.
        HM_NEAREST_PREFER_SIDES: Найти ближайшее укрытие, отдавая предпочтение боковым позициям.
        HM_NEAREST_TOWARDS_TARGET_PREFER_SIDES: Найти ближайшее укрытие, находящееся в направлении цели, отдавая предпочтение боковым позициям.
        HM_NEAREST_TOWARDS_TARGET_LEFT_PREFER_SIDES: Найти ближайшее укрытие, находящееся в направлении цели, отдавая предпочтение левым боковым позициям.
        HM_NEAREST_TOWARDS_TARGET_RIGHT_PREFER_SIDES: Найти ближайшее укрытие, находящееся в направлении цели, отдавая предпочтение правым боковым позициям.
        HM_NEAREST_TOWARDS_REFPOINT: Найти ближайшее укрытие, находящееся в направлении точки “refpoint”.
    exactly - использовать ли только точное значение расстояния для поиска укрытия (по умолчанию 0, то есть не использовать точное значение).
    min distance - минимальное расстояние до укрытия, которое нужно найти.
    look at LastOp - смотреть ли на последний объект, на который персонаж смотрел (по умолчанию 0), AILASTOPRES_LOOKAT.
    C++: Для реализации этой функциональности используется класс COPHide в C++.
    XML: В XML-файле можно задать параметры для поиска укрытия, такие как:
    searchDistance - максимальное расстояние, на котором персонаж будет искать укрытие (по умолчанию 10).
    evaluationMethod - метод поиска укрытия, описанный ниже.
    minDistance - минимальное расстояние до укрытия, которое нужно найти (по умолчанию 0).
    lookAtHide - смотреть ли на найденное укрытие (по умолчанию false).
    lookAtLastOp - смотреть ли на последний объект, на который персонаж смотрел (по умолчанию false).
    ]]
    HIDE = "hide",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) игнорировать все сигналы.

    -- AI.PushGoal("ignoreall", blocking, false/true)

    false/true - включить или отключить игнорирование сигналов (по умолчанию false).
    ]]
    IGNOREALL = "ignoreall",

    --[[
    
    -- AI.PushGoal("lookaround", blocking, look around range, scan range, interval min, interval max, flags)

    look around range - диапазон углов, в пределах которого персонаж будет оглядываться 
        (от -look around range до +look around range).
    scan range - диапазон углов, в пределах которого персонаж будет сканировать 
        (по умолчанию -1, не используется).
    interval min - минимальный интервал времени между оглядываниями 
        (по умолчанию -1, не используется).
    interval max - максимальный интервал времени между оглядываниями 
        (по умолчанию -1, не используется).
    flags: Дополнительные флаги для управления поведением:
        AI_BREAK_ON_LIVE_TARGET: Прерывать оглядывание, если персонаж обнаружил живую цель.
        AILASTOPRES_USE: Использовать последний объект, на который персонаж смотрел.

    AI.PushGoal("lookaround", 1, 90, 3, 3, 5, AI_BREAK_ON_LIVE_TARGET); - Заставляет персонажа оглядываться по сторонам в диапазоне 
        90 градусов с интервалом от 3 до 5 секунд и прерывать оглядывание, 
        если он обнаружит живую цель.
    AI.PushGoal("lookaround", 1, 30, 2.2, 10005, 10010, AI_BREAK_ON_LIVE_TARGET + AILASTOPRES_USE); - 
        Заставляет персонажа оглядываться по сторонам в диапазоне 30 градусов с интервалом
        от 10005 до 10010 миллисекунд, прерывать оглядывание, если он обнаружит живую цель, 
        и использовать последний объект, на который он смотрел.
    ]]
    LOOKAROUND = "lookaround",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) изменять направление взгляда. 
    По умолчанию персонаж смотрит в направлении объекта, на который обращено его внимание.

    -- AI.PushGoal("lookat", blocking, start angle, end angle, use LastOp, flags)

    start angle - начальный угол поворота (по умолчанию 0).
    end angle - конечный угол поворота (по умолчанию 0).
    use LastOp - использовать ли последний объект, на который персонаж смотрел (по умолчанию 0, то есть не использовать).
    flags - дополнительные флаги для управления поведением:
        AI_LOOKAT_CONTINUOUS: Использовать непрерывный поворот.
        AI_LOOKAT_USE_BODYDIR: Использовать текущее направление тела персонажа как начальный угол поворота.

    AI.PushGoal("tr_lookaround_30seconds", "lookat", 1, -90, 90); - Заставляет персонажа смотреть по сторонам в течение 30 секунд.
    AI.PushGoal("gr_lookat_interested", "lookat", 0, 0, 0, 1); - Заставляет персонажа смотреть на последний объект, на который он смотрел.
    AI.PushGoal("reset_lookat", "lookat", 1, -500); - Сбрасывает направление взгляда персонажа, заставляя его смотреть прямо.
    ]] 
    LOOKAT = "lookat",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) найти по имени и запомнить определенный объект или специальный.
    Функция locate обычно используется в сочетании с другими функциями, например, lookat, approach, follow и т.д.

    -- AI.PushGoal("locate", blocking, AI object type name or entity Name or special AI object, range)

    AI object type name or entity Name or special AI object - имя типа ИИ или имя сущности или специальный объект, который нужно найти.
    range: Максимальное расстояние, на котором персонаж будет искать объект.

    AI.PushGoal("look_at_player_5sec", "locate", 0, "player"); - Заставляет персонажа найти объект с именем “player” и запомнить его.
    AI.PushGoal("look_at_player_5sec", "+lookat", 0, 0, 0, true); - После того, как персонаж нашел объект “player”, заставляет его посмотреть на него.
    ]]
    LOCATE = "locate",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) найти путь до объекта.

    -- AI.PushGoal("pathfind", blocking, AI object's name or a special AI object, end tolerance (0 by default), end distance (0 by default), direction only distance (0 by default), high priority (false by default))

    AI object's name - имя объекта, до которого нужно найти путь. Может быть как название другого персонажа, так и название специального объекта, например, “refpoint”.
    end tolerance - допускаемая погрешность в достижении цели (по умолчанию 0).
    end distance - расстояние до цели, при котором персонаж прекратит движение (по умолчанию 0).
    direction only distance - расстояние, на котором персонаж будет двигаться только в направлении цели, не учитывая препятствия (по умолчанию 0).
    high priority - Поставить запрос на поиск пути в начало очереди запросов. Может быть полезно, например, для быстрого побега от гранат.
    ]]
    PATHFIND = "pathfind",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) отправить сигнал, когда он приближается к объекту на определенное расстояние.
    
    -- AI.PushGoal("proximity", blocking, radius, signal name, flags)

    radius - радиус, в пределах которого персонаж должен находиться от объекта, чтобы отправить сигнал.
    signal name - имя сигнала, который будет отправлен.
    flags - дополнительные флаги для управления поведением:
        AIPROX_VISIBLE_TARGET_ONLY: Отправлять сигнал только если объект виден персонажу.
        AIPROX_SIGNAL_ON_OBJ_DISABLE: Отправлять сигнал, если объект отключен.
    ]]
    PROXIMITY = "proximity",
    RANDOM = "random", -- AI.PushGoal("random", 0, 2, 50);

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) задать скорость движения, например, ходьбу, бег, спринт и т.д.
    --AI.PushGoal("run", blocking, max urgency, min urgency, max length)

    @param max urgency: Максимальная срочность движения 0 (SLOW), 1 (WALK), 2 (RUN), 3 (SPRINT),
    @param min urgency: Минимальная срочность движения (по умолчанию 0).
    @param max length: Максимальная длина пути, которая учитывается при расчете срочности (необязательный параметр).

    AI.PushGoal("run", 0, 2, 0, 20); - Заставляет персонажа бежать с максимальной срочностью 2, если длина пути не превышает 20.
    ]]
    RUN = "run",
    --SCRIPT = "script",
    SEEKCOVER = "seekcover",

    --[[
    Позволяет виртуальному персонажу (кукле) отправлять сигнал другим персонажам, которые находятся в определенном радиусе или принадлежат к определенной группе. 

    AI.PushGoal("signal", blocking, signal ID, signal text, destination filter, data.iValue)
    
    @param signal ID - идентификатор сигнала (по умолчанию 1)
        AISIGNAL_INCLUDE_DISABLED = 0
        AISIGNAL_DEFAULT = 1
        AISIGNAL_PROCESS_NEXT_UPDATE = 3
        AISIGNAL_NOTIFY_ONLY = 9
        AISIGNAL_ALLOW_DUPLICATES = 10
    @param signal text - текст сигнала (по умолчанию пустая строка).
    @param data.iValue - значение, которое отправится в data.iValue
    @param destination filter - фильтр для определения получателей сигнала:
        SIGNALFILTER_SENDER,
        SIGNALFILTER_LASTOP,
        SIGNALFILTER_GROUPONLY,
        SIGNALFILTER_FACTIONONLY,
        SIGNALFILTER_ANYONEINCOMM,
        SIGNALFILTER_TARGET,
        SIGNALFILTER_SUPERGROUP,
        SIGNALFILTER_SUPERFACTION,
        SIGNALFILTER_SUPERTARGET,
        SIGNALFILTER_NEARESTGROUP,
        SIGNALFILTER_NEARESTSPECIES,
        SIGNALFILTER_NEARESTINCOMM,
        SIGNALFILTER_HALFOFGROUP,
        SIGNALFILTER_LEADER,
        SIGNALFILTER_GROUPONLY_EXCEPT,
        SIGNALFILTER_ANYONEINCOMM_EXCEPT,
        SIGNALFILTER_LEADERENTITY,
        SIGNALFILTER_NEARESTINCOMM_FACTION,
        SIGNALFILTER_NEARESTINCOMM_LOOKING,
        SIGNALFILTER_FORMATION,
        SIGNALFILTER_FORMATION_EXCEPT,
        SIGNALFILTER_READIBILITY,
        SIGNALFILTER_READIBILITYAT,
        SIGNALFILTER_READIBILITYRESPONSE 
    ]]
    SIGNAL = "signal", 

    --[[
    Эта операция позволяет виртуальному персонажу (кукле) приблизиться к цели и поддерживать заданное расстояние от нее.
    
    --AI.PushGoal("stick", blocking, distance or duration, flags, stick flags, end accuracy, end distance variation)

    @param distance or duration - расстояние или время, которое персонаж должен поддерживать от цели (по умолчанию дистанция)
    
    @param flags - флаги используются для управления движением персонажа во время поддержания расстояния от цели.:
        AILASTOPRES_USE: Использует предыдущую команду для достижения цели.
        AILASTOPRES_LOOKAT: Заставляет персонажа смотреть на цель, используя предыдущую команду.
        AI_USE_TIME: Использует время, а не расстояние, для поддержания дистанции (см. stick distance or time).
        AI_REQUEST_PARTIAL_PATH: Запрашивает частичный путь к цели.
        AI_STOP_ON_ANIMATION_START: Останавливает движение при начале анимации персонажа.
        AI_DONT_STEER_AROUND_TARGET: Не объезжает цель, даже если это нужно для поддержания расстояния
        AI_CONSTANT_SPEED: Двигается с постоянной скоростью.
        AI_ADJUST_SPEED: Регулирует скорость движения в зависимости от ситуации.
    
    @param stick flags - флаги используются для упр. поведением движения после достижения цели.
        STICK_BREAK: Прерывает движение после достижения цели.
        STICK_SHORTCUTNAV: Использует сокращенные пути для достижения цели.        
    
    @param end accuracy - допустимая погрешность при достижении конечного расстояния от цели.
    @param end distance variation - допустимое отклонение от заданного расстояния (по умолчанию 0).

    AI.PushGoal("stick", 0, 1, AI_DONT_STEER_AROUND_TARGET + AI_CONSTANT_SPEED, STICK_BREAK + STICK_SHORTCUTNAV);
    AI.PushGoal("locate", 0, "atttarget");
    AI.PushGoal("stick", 1, 10, AILASTOPRES_USE, STICK_BREAK);
    ]]
    STICK = "stick",

    --[[
    Включает возможность двигаться в сторону. Сбрасывается после SelectPipe.

    @param distance start - расстояние, которое персонаж должен пройти боком от начала пути.
    @param distance end - расстояние, которое персонаж должен пройти боком от конца пути (по умолчанию 0).
    Пример пользования:

    -- AI.PushGoal("strafe", blocking, distance start, distance end, strafe while moving)

    AI.PushGoal("bodypos", 0, BODYPOS_STAND, 1);
    AI.PushGoal("strafe", 0, 2, 2);
    AI.PushGoal("run", 0, 1);
    --
    AI.BeginGoalPipe("su_fast_bullet_reaction");
    AI.PushGoal("bodypos", 1, BODYPOS_STAND, 1);
    -- Be able to shoot while going anywhere
    -- 100 (meters) result in "strafe all the way"
    --
    AI.PushGoal("strafe", 0, 100, 100);
    --
    AI.PushGoal("firecmd", 0, FIREMODE_BURST_DRAWFIRE);
    AI.PushGoal("locate", 0, "probtarget");
    AI.PushGoal("+seekcover",1, COVER_HIDE, 3, 2, 1);
    AI.PushGoal("signal", 1, 1, "COVER_NORMALATTACK", 0);
    AI.EndGoalPipe();
    ]]
    STRAFE = "strafe",
    MOVE = "move",
    TIMEOUT = "timeout", -- AI.PushGoal("timeout", blocking, min time, max time)

    --[[
        Движение по набору точек до цели. Пример пользования:

        -- AI.PushGoal("trace", blocking, follow exactly)
        
        AI.PushGoal("pathfind", 1, "refpoint"); -- Поиск  пути  к  "refpoint"
        AI.PushGoal("branch", 1, "PATH_FOUND", NOT + IF_NO_PATH); -- Если  путь  найден
        AI.PushGoal("signal", 0, 1, "OnUnitStop", SIGNALFILTER_LEADER); -- Сигнал  о  остановке
        AI.PushGoal("branch", 1, "DONE", BRANCH_ALWAYS); -- Перейти  к  метке  "DONE"
        AI.PushLabel("PATH_FOUND"); -- Метка  "PATH_FOUND"
        AI.PushGoal("signal", 0, 1, "OnUnitMoving", SIGNALFILTER_LEADER); -- Сигнал  о  движении
        AI.PushGoal("trace", 1, 1); --  Следовать  по  пути  точно
        ...
        AI.PushLabel("DONE"); -- Метка  "DONE"
    ]]
    TRACE = "trace",

    USECOVER = "usecover", -- AI.PushGoal("usecover", blocking, hide / unhide, min time, max time, use LastOp as backup)

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) 
    приостановить выполнение других задач до тех пор, 
    пока не завершатся определенные неблокирующие цели.
    Функция wait может быть полезна в редких случаях, 
    когда необходимо использовать неблокирующие цели, 
    но при этом важно, чтобы персонаж не начинал выполнять 
    другие действия до завершения этих целей.

    -- AI.PushGoal("wait", blocking, whom), whom: WAIT_ALL, WAIT_ANY, or WAIT_ANY_2.

    whom/for: Тип ожидания. Возможные значения:
    WAIT_ALL/All: Дождаться завершения всех неблокирующих целей.
    WAIT_ANY/Any: Дождаться завершения хотя бы одной из неблокирующих целей.
    WAIT_ANY_2/Any2: Дождаться завершения хотя бы двух из неблокирующих целей.

    AI.BeginGoalPipe("tr_melee");
    AI.BeginGroup();
    AI.PushGoal("animation", 0, AIANIM_SIGNAL, "meleeAttack");
    AI.PushGoal("timeout", 0, 1.4);
    AI.EndGroup();
    --
    AI.PushGoal("wait", 1, WAIT_ANY);
    --
    AI.PushGoal("signal", 1, 1, "END_MELEE", SIGNALFILTER_SENDER);
    AI.EndGoalPipe();
    --В этом примере персонаж выполняет анимацию “meleeAttack” и одновременно запускает таймер на 1.4 секунды. 
    --После того, как завершится хотя бы одна из этих неблокирующих целей (анимация или таймер), 
    --персонаж переходит к следующей цели в goalpipe, которая отправляет сигнал “END_MELEE”.
    ]]
    WAIT = "wait",

    --[[
    Эта функция позволяет виртуальному персонажу (кукле) ожидать получения определенного сигнала.

    -- AI.PushGoal("waitsignal", blocking, signal to wait, signal data)

    signal to wait - имя сигнала, который нужно ожидать.
    signal data - дополнительные данные сигнала (по умолчанию nil). 
    
    AI.BeginGoalPipe("tr_jump_fire_left");
    AI.PushGoal("strafe", 0, 8);
    AI.PushGoal("firecmd", 1, FIREMODE_CONTINUOUS);
    --
    -- Wait for the execution of OnLand signal
    AI.PushGoal("waitsignal", 1, "OnLand", nil, 4);
    AI.PushGoal("firecmd", 1, 0);
    AI.EndGoalPipe();
    ]]
    WAITSIGNAL = "waitsignal",
}

TOS_AI = {

    ---Возвращает название текущего поведения сущности
    ---@param entity table
    ---@return string
    GetCurrentBehaviourName = function (entity)
        return entity.Behaviour.name
    end,

    ---ИИ-пользователь запускает FG действие
    ---@param actionId string название действия
    ---@param userId userdata entity.id пользователя
    ---@param objectId userdata entity.id объекта над которым будет совершено действие
    ---@param maxAlertness integer (0,1,2) максимальная беспокоенность 
    ---@param goalPipeId integer идентификатор выполняемого действия
    ---@usage ExecuteAction(1,2,3,4,5)
    ExecuteAction = function ( actionId, userId, objectId, maxAlertness, goalPipeId)

        if not goalPipeId then
            AI.ExecuteAction(actionId, userId, objectId, maxAlertness)
        else
            AI.ExecuteAction(actionId, userId, objectId, maxAlertness, goalPipeId)
        end

    end,


    AbortAction = function ( userId, goalPipeId)
        AI.AbortAction(userId, goalPipeId)
    end,
    
    ---Выбирает трубу целей для сущности.
    ---Используется  для  управления  поведением  AI,  позволяя  переключаться  между  разными  трубами  целей  в  зависимости  от  условий  игры.
    ---@param priorityFlag any Флаг приоритета (например, `AIGOALPIPE_RUN_ONCE`, `AIGOALPIPE_HIGH_PRIORITY`).
    ---@param entity any Сущность, для которой выбирается труба целей.
    ---@param pipeName any Имя трубы целей (например, "Attack", "Move").
    ---@param targetId any (Опционально) Идентификатор цели (она должна быть ИИ). Сохраняется как `LastOpResult`
    ---@param pipeId any (Опционально) Идентификатор трубы целей.
    ---@param resetCurrentPipe any (Опционально) `true`,  чтобы  сбросить  состояние  текущей  трубы  целей.
    ---@return boolean true если труба целей была успешно выбрана.
    SelectPipe = function (priorityFlag, entity, pipeName, targetId, pipeId, resetCurrentPipe)
        return entity:SelectPipe(priorityFlag, pipeName, targetId, pipeId, resetCurrentPipe) == true
    end,

    ---@param priorityFlag integer  Приоритет подканала (битовые флаги, используемые в `AIGOALPIPE_...`).
    ---@param entity table  Сущность, в которую добавляется подканал.
    ---@param pipeName string  Имя подканала.
    ---@param targetId userdata Идентификатор цели подканала (она должна быть ИИ), которая при вызове функции сохраняется в `LastOpResult`
    ---@param goalPipeId integer  (Опционально) Идентификатор `AIGoalPipe`, в который добавляется подканал.
    InsertSubpipe = function ( priorityFlag, entity, pipeName, targetId, goalPipeId)
        return entity:InsertSubpipe(priorityFlag, pipeName, targetId, goalPipeId) == true
    end,

    --- Отправка сигнала в систему ИИ
    ---@param filter integer SIGNALFILTER_...
    ---@param signalId integer AISIGNAL_...
    ---@param functionName string имя функции, которая будет вызвана. Смотреть в Behaviour файле и Character файле ИИ объекта
    ---@param senderId userdata идентификатор сущности отправителя
    ---@param data? table
    ---@usage TOS_AI.SendSignal(SIGNALFILTER_SENDER, AISIGNAL_DEFAULT, "GO_TO_TOSSHARED", entity.id)
    SendSignal = function ( filter, signalId, functionName, senderId, data)
        AI.Signal(filter, signalId, functionName, senderId, data)
    end,

    --- Инициирует создание GoalPipe.
    --- Инициация должна быть завершена через EndGoalPipe
    --- @param goalPipeName string
    BeginGoalPipe = function ( goalPipeName)
        AI.BeginGoalPipe(goalPipeName)
    end,

    EndGoalPipe = function ()
        AI.EndGoalPipe()
    end,

    PushLabel = function ( labelName)
        AI.PushLabel(labelName)
    end,

    ---@param goalOp string Любой другой `GoalPipe` или GO.
    ---@param blocking integer Если `true` - блокирует дальнейшее выполнение текущего GoalPipe пока не будет выполнена `goalOp`
    ---@param ... unknown параметры, зависящие от `goalOp`
    PushGoal = function (goalOp, blocking, ...)
        AI.PushGoal(goalOp, blocking, ...)
    end,

    --- Функция конвертирует путь в сплайн, а также использует флаг `avoidObstacles` из `m_movementAbility.pathfindingProperties`
    ---для понимания, избегать ли препятствия на пути при корректировке или нет.
    ---@param entity table
    ---@param bAdjust boolean
    SetAdjustPath = function (entity, bAdjust)
        AI.SetAdjustPath(entity, bAdjust)
    end,

    ---Возвращает таблицу ближайших сущностей определенного типа.
    ---@param position table Позиция, относительно которой ищется ближайшая сущность. Может быть точкой (Vec3), сущностью (IEntity) или именем сущности (string).
    ---@param aiObjectType integer Тип сущности, которую нужно найти (константа, например, AI_HUMAN).
    ---@param numObjects integer Максимальное количество сущностей, которые нужно найти.
    ---@param outputContainer table Таблица, в которую будут добавлены найденные сущности.
    ---@param filter integer  Фильтр для поиска:                            
        ---| '0' # без фильтра.
        ---| 'AIOBJECTFILTER_INCLUDEINACTIVE' # включает неактивные сущности.
        ---| 'AIOBJECTFILTER_SAMESPECIES' # включает только сущности той же фракции, что и исходная.
        ---| 'AIOBJECTFILTER_SAMEGROUP' # включает только сущности той же группы, что и исходная.
        ---| 'AIOBJECTFILTER_NOGROUP' # включает только сущности, не принадлежащие никакой группе.`
    ---@param radius integer Радиус поиска (в метрах). По умолчанию 30.
    ---@return integer integer Количество найденных сущностей.
    GetNearestEntitiesOfType = function (position, aiObjectType, numObjects, outputContainer, filter, radius)
        return AI.GetNearestEntitiesOfType(position, aiObjectType, numObjects, outputContainer, filter, radius)    
    end,

    ---Возвращает позицию опорной точки сущности.
    ---@param entityId    userdata идентификатор сущности, для которой нужно получить позицию опорной точки
    ---@return            Vec3 Vec3 координаты опорной точки
    GetRefPointPosition = function (entityId)
        return AI.GetRefPointPosition(entityId)
    end,

    ---Возвращает тип цели внимания для entityId
    ---@param entityId userdata
    ---@return integer integer тип цели, т.е. AITARGET_...
    GetTargetType = function (entityId)
        return AI.GetTargetType(entityId)
    end,

    ---Возвращает дистанцию до цели внимания
    ---@param entityId userdata
    ---@return number number дистанция
    GetAttentionTargetDistance = function (entityId)
        return AI.GetAttentionTargetDistance(entityId)
    end,

    SetPFBlockerRadius = function (entityId, PFB_type, radius)
        return AI.SetPFBlockerRadius(entityId, PFB_type, radius)
    end,

    NotifyGroupTacticState = function (entityId, HZ_INT, GN_type)
        return AI.NotifyGroupTacticState(entityId, HZ_INT, GN_type)
    end,

    GetGroupTacticState = function (entityId, HZ_INT, GE_type)
        return AI.GetGroupTacticState(entityId, HZ_INT, GE_type)
    end,

    GetGroupTacticPoint = function (entityId, HZ_INT, GE_type_POS)
        return AI.GetGroupTacticPoint(entityId, HZ_INT, GE_type_POS)
    end,

    GetGroupMember = function (entityId, index, GROUP_flag)
        return AI.GetGroupMember(entityId, index, GROUP_flag)
    end,

    GetGroupTarget = function (entityId, HZ_BOOL)
        return AI.GetGroupTarget(entityId, HZ_BOOL)
    end,

    -- Умножает размер формации от центра
    ---@param entityId userdata идентификатор сущности лидера
    ---@param scale number размер формации (любое число)
    ---@return unknown
    ScaleFormation = function (entityId, scale)
        return AI.ScaleFormation(entityId, scale)
    end,

    -- AI.GetGroupAveragePosition(entity.id,UPR_COMBAT_GROUND,avgPos);
    GetGroupAveragePosition = function (entityId, UPR_type, avgPos)
        return AI.GetGroupAveragePosition(entityId, UPR_type, avgPos)
    end,

    --- Получить номер группы ИИ объекта
    ---@param entityId userdata
    ---@return integer
    GetGroupOf = function (entityId)
        return AI.GetGroupOf(entityId)
    end,

    ---Получить кол-во агентов в группе, согласно флагам
    ---@param entityId userdata
    ---@param GROUP_flags integer
    ---@return integer
    GetGroupCount = function (entityId, GROUP_flags)
        return AI.GetGroupCount(entityId, GROUP_flags);
    end,

    ---Получить значение параметра ИИ сущности
    ---@param entity table
    ---@param AIPARAM integer
    ---@return unknown
    GetAIParameter = function (entity, AIPARAM)
        return AI.GetAIParameter( entity.id, AIPARAM);
    end,

    ---Изменяет параметр ИИ объекта
    ---@param AIPARAM number параметр AIPARAM...
    ---@param value any его значение 
    ChangeParameter = function (entity, AIPARAM, value)
        return AI.ChangeParameter( entity.id, AIPARAM, value );
    end
}