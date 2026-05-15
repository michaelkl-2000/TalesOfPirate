//=============================================================================
// FileName: JobType.h
// Creater: ZhangXuedong
// Date: 2005.03.18
// Comment: Job Type
//=============================================================================

#pragma once

#include <cstdint>

// Класс персонажа (Job). Имена-пиньинь сохраняются для совместимости с
// серверным CharacterPacket.cpp / NetProtocol.cpp (case-метки свитчей) и с
// уже написанными Lua-скриптами (AttrCalculate.lua и др.).
// Underlying-тип uint8_t — значения 0..18, прицельно компактно для сетевых пакетов.
//
// SINGLE SOURCE OF TRUTH — макрос JOB_TYPE_LIST(X). Из него генерируется:
//   - enum class Corsairs::Common::Character::JobType
//   - legacy-алиасы JOB_TYPE_XINSHOU ... JOB_TYPE_GONGCHENGSHI (int32_t)
//   - таблица для регистрации в Lua (см. Server/.../Script/JobTypeScript.cpp)
// При добавлении новой профессии достаточно добавить одну строку в JOB_TYPE_LIST.
//
// enum class живёт в Corsairs::Common::Character; legacy-алиасы и MAX_JOB_TYPE /
// g_szJobName остаются в глобальной области для совместимости с многочисленными
// callsite'ами, не находящимися в этом namespace.
// TODO: после wrapping `Item` и др. потребителей — перенести MAX_JOB_TYPE
// и g_szJobName в Corsairs::Common::Character и убрать legacy-алиасы.

// X(имя, числовое значение, английское описание)
#define JOB_TYPE_LIST(X)                  \
    X(XINSHOU,      0,  "Newbie")         \
    X(JIANSHI,      1,  "Swordsman")      \
    X(LIEREN,       2,  "Hunter")         \
    X(SHUISHOU,     3,  "Sailor")         \
    X(MAOXIANZHE,   4,  "Explorer")       \
    X(QIYUANSHI,    5,  "Herbalist")      \
    X(JISHI,        6,  "Artisan")        \
    X(SHANGREN,     7,  "Merchant")       \
    X(JUJS,         8,  "Champion")       \
    X(SHUANGJS,     9,  "Crusader")       \
    X(JIANDUNSHI,   10, "WhiteKnight")    \
    X(XUNSHOUSHI,   11, "AnimalTamer")    \
    X(JUJISHOU,     12, "Sharpshooter")   \
    X(SHENGZHIZHE,  13, "Cleric")         \
    X(FENGYINSHI,   14, "SealMaster")     \
    X(CHUANZHANG,   15, "Captain")        \
    X(HANGHAISHI,   16, "Voyager")        \
    X(BAOFAHU,      17, "Upstart")        \
    X(GONGCHENGSHI, 18, "Engineer")

namespace Corsairs::Common::Character {

enum class JobType : std::uint8_t {
#define X(name, value, comment) name = value,
    JOB_TYPE_LIST(X)
#undef X
};

} // namespace Corsairs::Common::Character

inline constexpr std::int32_t MAX_JOB_TYPE = 19;

// Совместимостные алиасы для case-меток в switch'ах CharacterPacket.cpp /
// NetProtocol.cpp и для legacy-кода, который ещё ожидает integer-константы.
// Новый код использовать `Corsairs::Common::Character::JobType::JUJS` напрямую.
#define X(name, value, comment) inline constexpr std::int32_t JOB_TYPE_##name = value;
JOB_TYPE_LIST(X)
#undef X

extern const char* g_szJobName[MAX_JOB_TYPE];
