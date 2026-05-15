// RoleCommon.h — общие константы и перечисления подсистемы миссий/торговли/стола.
// Created by knight-gongjian 2004.12.1.
// Refactored to Corsairs::Common::Mission, C++23 (2026-05-15).
//
// Структура файла:
//   * ROLE_*-константы (лимиты, флаги, sentinel'ы) — в global scope.
//     Исторически они были global `#define` и используются без квалификации
//     (191+ use-site). Перенос в namespace ломает include-цепочки (файл
//     может подключаться до PCH using-директивы), поэтому оставлены global.
//   * Enum'ы и struct'ы — в namespace Corsairs::Common::Mission.
//   * EM_OK/EM_FAILER — в Corsairs::Common (generic-error sentinels).
//---------------------------------------------------------
#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace Corsairs::Common {

    // Generic-error sentinels (legacy "EM_*"). Используются как код возврата
    // в местах, где ещё не введён enum class Status.
    inline constexpr std::int32_t EM_OK     =  0;
    inline constexpr std::int32_t EM_FAILER = -1;

} // namespace Corsairs::Common

//---------------------------------------------------------
// Лимиты слотов и буферов подсистемы миссий (global scope).
//---------------------------------------------------------
inline constexpr std::int32_t ROLE_MAXNUM_DESCPAGE        = 12;
inline constexpr std::int32_t ROLE_MAXNUM_FUNCPAGE        = 4;
inline constexpr std::int32_t ROLE_MAXNUM_FLAGSIZE        = 16;
inline constexpr std::int32_t ROLE_MAXNUM_RECORDSIZE      = 256;
inline constexpr std::int32_t ROLE_MAXSIZE_DBMISSION      = 2048;
inline constexpr std::int32_t ROLE_MAXSIZE_DBTRIGGER      = 2048;
inline constexpr std::int32_t ROLE_MAXSIZE_DBMISCOUNT     = 512;
inline constexpr std::int32_t ROLE_MAXSIZE_DBRECORD       = 1024;
inline constexpr std::int32_t ROLE_MAXNUM_INDEXSIZE       = 8;
inline constexpr std::int32_t ROLE_MAXNUM_FUNCITEM        = 8;
inline constexpr std::int32_t ROLE_MAXNUM_DESPSIZE        = 1024;
inline constexpr std::int32_t ROLE_MAXNUM_NEEDDESPSIZE    = 256;
inline constexpr std::int32_t ROLE_MAXNUM_FUNCITEMSIZE    = 64;
inline constexpr std::int32_t ROLE_MAXNUM_CAPACITY        = 64;
inline constexpr std::int32_t ROLE_MAXNUM_TRADEITEM       = 120;
inline constexpr std::int32_t ROLE_MAXNUM_TRADEDATA       = 18;
inline constexpr std::int32_t ROLE_MAXNUM_ITEMTRADE       = 99;
inline constexpr std::int32_t ROLE_MAXNUM_CHARTRIGGER     = 64;
inline constexpr std::int32_t ROLE_MAXNUM_NPCTRIGGER      = 4;
inline constexpr std::int32_t ROLE_MAXNUM_FLAG            = 16;
inline constexpr std::int32_t ROLE_MAXNUM_MISSIONSTATE    = 32;
inline constexpr std::int32_t ROLE_MAXNUM_MISSION_STEP    = 16;
inline constexpr std::int32_t ROLE_MAXNUM_RANDMISSION     = 10;
inline constexpr std::int32_t ROLE_MAXNUM_INSIDE_NPCCOUNT = 24;
inline constexpr std::int32_t ROLE_MAXSIZE_TRADEDATA      = 2000;
inline constexpr std::int32_t ROLE_MAXSIZE_STALLDATA      = 2000;
inline constexpr std::int32_t ROLE_MAXSIZE_MSGPROC        = 16;
inline constexpr std::int32_t ROLE_MAXVALUE_PARAM         = 16;
inline constexpr std::int32_t ROLE_MAXNUM_MISNEED         = 8;
inline constexpr std::int32_t ROLE_MAXNUM_MISPRIZE        = 8;
inline constexpr std::int32_t ROLE_MAXSIZE_MISNAME        = 32;
inline constexpr std::int32_t ROLE_MAXNUM_FORGE           = 12;
inline constexpr std::int32_t ROLE_MAXNUM_MISSION         = 10;
inline constexpr std::int32_t ROLE_MAXNUM_RAND_DATA       = 4;
inline constexpr std::int32_t ROLE_MAXNUM_MISSIONCOUNT    = 32;
inline constexpr std::int32_t ROLE_MAXNUM_EUDEMON         = 4;
inline constexpr std::int32_t ROLE_MAXNUM_MAPNPC          = 512;
inline constexpr std::int32_t ROLE_MAXNUM_STALL_GOODS     = 24;
inline constexpr std::int32_t ROLE_MAXNUM_STALL_NUM       = 64;
inline constexpr std::int32_t ROLE_MAXSIZE_PASSWORD2      = 32;

// Маркер «закрыть описание» для NPC-диалогов
inline constexpr std::string_view ROLE_CLOSE_DESC = "";

//---------------------------------------------------------
// Состояния страниц диалога
//---------------------------------------------------------
inline constexpr std::int32_t ROLE_FIRSTPAGE = 0;
inline constexpr std::int32_t ROLE_CLOSEPAGE = -1;

//---------------------------------------------------------
// Параметры торговой сессии
//---------------------------------------------------------
inline constexpr std::int32_t ROLE_TRADE_START       = 1;
inline constexpr std::int32_t ROLE_MAXNUM_TRADETIME  = 50000;
inline constexpr std::int32_t ROLE_MAXSIZE_TRADEDIST = 80 * 80;

//---------------------------------------------------------
// Битовые флаги состояний миссии у NPC
//---------------------------------------------------------
inline constexpr std::uint32_t ROLE_MIS_ACCEPT   = 1u << 0;
inline constexpr std::uint32_t ROLE_MIS_DELIVERY = 1u << 1;
inline constexpr std::uint32_t ROLE_MIS_PENDING  = 1u << 2;
inline constexpr std::uint32_t ROLE_MIS_IGNORE   = 1u << 3;

//---------------------------------------------------------
// Коды действий пользователя в окне миссии
//---------------------------------------------------------
inline constexpr std::int32_t ROLE_MIS_PREV         = 0;
inline constexpr std::int32_t ROLE_MIS_NEXT         = 1;
inline constexpr std::int32_t ROLE_MIS_PREV_END     = 2;
inline constexpr std::int32_t ROLE_MIS_NEXT_END     = 3;
inline constexpr std::int32_t ROLE_MIS_SEL          = 4;
inline constexpr std::int32_t ROLE_MIS_TALK         = 5;
inline constexpr std::int32_t ROLE_MIS_BTNACCEPT    = 6;
inline constexpr std::int32_t ROLE_MIS_BTNDELIVERY  = 7;
inline constexpr std::int32_t ROLE_MIS_BTNPENDING   = 8;
inline constexpr std::int32_t ROLE_MIS_LOG          = 9;

inline constexpr std::int32_t ROLE_MIS_PENDING_FLAG  = 0;
inline constexpr std::int32_t ROLE_MIS_COMPLETE_FLAG = 1;
inline constexpr std::int32_t ROLE_MIS_FAILURE_FALG  = 2;

//---------------------------------------------------------
// FOURCC-маркеры заголовков бинарных файлов миссий.
// Используем std::bit_cast — устраняет UB по strict-aliasing,
// которое было в C-style `DWORD(*(LPDWORD)"trig")`.
//---------------------------------------------------------
namespace Corsairs::Common::Mission::Detail {
    constexpr std::uint32_t Fourcc(char a, char b, char c, char d) noexcept {
        return std::bit_cast<std::uint32_t>(std::array<char, 4>{a, b, c, d});
    }
} // namespace Corsairs::Common::Mission::Detail

inline constexpr std::uint32_t ROLE_MIS_TRIGGER_HEADER =
    Corsairs::Common::Mission::Detail::Fourcc('t', 'r', 'i', 'g');
inline constexpr std::uint32_t ROLE_MIS_MISINFO_HEADER =
    Corsairs::Common::Mission::Detail::Fourcc('m', 'i', 's', 'n');

inline constexpr std::uint16_t ROLE_MIS_RECORD_EDITION   = 0x0003;
inline constexpr std::uint16_t ROLE_MIS_TRIGGER_EDITION  = 0x0003;
inline constexpr std::uint16_t ROLE_MIS_MISINFO_EDITION  = 0x0003;
inline constexpr std::uint16_t ROLE_MIS_MISCOUNT_EDITION = 0x0003;

//---------------------------------------------------------
// Sentinel'ы количеств — выделены из enum'ов в global scope,
// чтобы можно было использовать как array size без явного cast'а.
//---------------------------------------------------------
inline constexpr std::int32_t MAXTRADE_ITEMTYPE = 4;  // sentinel TradeItemType
inline constexpr std::int32_t TE_MAXNUM_TYPE    = 9;  // sentinel TriggerEvent

namespace Corsairs::Common::Mission {

    //---------------------------------------------------------
    // Utility: unary `+` для enum class возвращает underlying type.
    // Это идиома C++ для краткой конверсии в integer:
    //   `BYTE x = +TradeOpType::TRADE_SALE;`  вместо
    //   `BYTE x = static_cast<BYTE>(TradeOpType::TRADE_SALE);`
    // Сужено через requires до enum'ов из этого namespace через ADL.
    //---------------------------------------------------------
    template <typename TEnum>
        requires std::is_enum_v<TEnum>
    constexpr auto operator+(TEnum value) noexcept {
        return static_cast<std::underlying_type_t<TEnum>>(value);
    }

    //---------------------------------------------------------
    // Перечисления
    //---------------------------------------------------------

    enum class TradeOpType : std::uint32_t {
        TRADE_SALE             = 0,
        TRADE_BUY              = 1,
        TRADE_GOODS            = 2,
        TRADE_DRAGTO_ITEM      = 3,
        TRADE_DRAGTO_TRADE     = 4,
        TRADE_DRAGMONEY_ITEM   = 5,
        TRADE_DRAGMONEY_TRADE  = 6,
        TRADE_SUCCESS          = 7,
        TRADE_FAILER           = 8,
    };

    enum class TradeItemType : std::uint32_t {
        TI_WEAPON    = 0,
        TI_DEFENCE   = 1,
        TI_OTHER     = 2,
        TI_SYNTHESIS = 3,
    };

    enum class TradeCharType : std::uint32_t {
        TRADE_CHAR = 0,
        TRADE_BOAT = 1,
    };

    enum class TriggerTimeType : std::uint32_t {
        TT_CYCLETIME = 0,
        TT_MULTITIME = 1,
    };

    enum class TriggerEvent : std::uint32_t {
        TE_MAP_INIT   = 0,  // инициализация карты
        TE_NPC        = 1,  // npc-триггер
        TE_KILL       = 2,  // убийство (1 — кол-во, 2 — ID существа, 3 — флаг)
        TE_GAME_TIME  = 3,  // игровое время (1 — год, 2 — месяц, 3 — день, 4 — час)
        TE_CHAT       = 4,  // чат (1 — канал, 2 — ID получателя, 3 — текст)
        TE_GET_ITEM   = 5,  // подбор предмета (1 — кол-во, 2 — ID, 3 — флаг)
        TE_EQUIP_ITEM = 6,  // экипировка (1 — ID, 2 — слот, 3 — флаг)
        TE_GOTO_MAP   = 7,  // переход на карту (1 — ID, 2 — x, 3 — y, 4-5 — диапазон)
        TE_LEVEL_UP   = 8,  // получение уровня (1 — новый уровень, 2 — флаг)
    };

    enum class MissionType : std::uint32_t {
        MIS_TYPE_NOMAL = 0,
        MIS_TYPE_RAND  = 1,
        MIS_TYPE_WORLD = 2,
    };

    enum class MissionShowType : std::uint32_t {
        MIS_ALLWAYS_SHOW  = 0,
        MIS_COMPLETE_SHOW = 1,
    };

    enum class MissionNeedType : std::uint32_t {
        MIS_NEED_ITEM    = 0,  // принести предмет (1 — ID, 2 — кол-во)
        MIS_NEED_KILL    = 1,  // убить (1 — ID, 2 — кол-во)
        MIS_NEED_SEND    = 2,  // доставить (1 — npc-id)
        MIS_NEED_CONVOY  = 3,  // сопроводить (1 — ID, 2 — x, 3 — y)
        MIS_NEED_EXPLORE = 4,  // разведать (1 — ID, 2 — x, 3 — y)
        MIS_NEED_DESP    = 5,
    };

    enum class MissionPrizeType : std::uint32_t {
        MIS_PRIZE_ITEM  = 0,  // предмет (1 — ID, 2 — кол-во)
        MIS_PRIZE_MONEY = 1,  // деньги (1 — сумма)
        MIS_PRIZE_FAME  = 2,  // слава (1 — кол-во)
        MIS_PRIZE_CESS  = 3,  // налог (1 — кол-во)
    };

    enum class MissionPrizeSelectType : std::uint32_t {
        PRIZE_SELONE = 0,  // выбрать один из набора
        PRZIE_SELALL = 1,  // получить всё (опечатка PRZIE сохранена для совместимости)
    };

    enum class MissionRandType : std::uint32_t {
        MIS_RAND_KILL    = 0,  // убийство (1 — ID, 2 — кол-во)
        MIS_RAND_GET     = 1,  // подбор предмета (1 — ID, 2 — кол-во)
        MIS_RAND_SEND    = 2,  // доставка (1 — ID, 2 — кол-во)
        MIS_RAND_CONVOY  = 3,  // сопровождение NPC (1 — ID, 2 — кол-во)
        MIS_RAND_EXPLORE = 4,  // разведка (1 — ID, 2 — x, 3 — y)
    };

    enum class MissionExpType : std::uint32_t {
        MIS_EXP_NOMAL = 0,
        MIS_EXP_SAIL  = 1,
        MIS_EXP_LIFE  = 2,
    };

    enum class MissionHelpType : std::uint32_t {
        MIS_HELP_DESP   = 0,
        MIS_HELP_IMAGE  = 1,
        MIS_HELP_SOUND  = 2,
        MIS_HELP_BICKER = 3,
    };

    enum class MissionTreeNodeType : std::uint32_t {
        MIS_TREENODE_INVALID = 0,
        MIS_TREENODE_NORMAL  = 1,
        MIS_TREENODE_HISTORY = 2,
        MIS_TREENODE_GUILD   = 3,
    };

    enum class EntityType : std::uint32_t {
        BASE_ENTITY     = 0,
        RESOURCE_ENTITY = 1,
        TRANSIT_ENTITY  = 2,
        BERTH_ENTITY    = 3,
    };

    enum class EntityState : std::uint32_t {
        ENTITY_DISABLE = 0,
        ENTITY_ENABLE  = 1,
    };

    enum class EntityAction : std::uint32_t {
        ENTITY_START_ACTION    = 0,
        ENTITY_END_ACTION      = 1,
        ENTITY_INTERMIT_ACTION = 2,
    };

    enum class GoodsType : std::uint32_t {
        RES_WOOD = 0,
        RES_MINE = 1,
    };

    enum class BoatListType : std::uint32_t {
        BERTH_TRADE_LIST     = 0,
        BERTH_LUANCH_LIST    = 1,
        BERTH_BAG_LIST       = 2,
        BERTH_REPAIR_LIST    = 3,
        BERTH_SALVAGE_LIST   = 4,
        BERTH_SUPPLY_LIST    = 5,
        BERTH_BOATLEVEL_LIST = 6,
    };

    enum class ViewItemType : std::uint32_t {
        VIEW_CHAR_BAG        = 0,
        VIEW_CHARTRADE_SELF  = 1,
        VIEW_CHARTRADE_OTHER = 2,
    };

    //---------------------------------------------------------
    // Сетевые структуры игрового стола (POD-обёртки).
    // Поля layout-эквивалентны прежним DWORD/BYTE — wire-формат
    // сохраняется. Имена полей — PascalCase согласно CLAUDE.md.
    //---------------------------------------------------------
    struct NetStallGoods {
        std::uint32_t Money;
        std::uint8_t  Count;
        std::uint8_t  Index;
        std::uint8_t  Grid;
    };

    struct NetStallAllData {
        std::uint8_t  Num;
        NetStallGoods Info[ROLE_MAXNUM_STALL_GOODS];
    };

} // namespace Corsairs::Common::Mission

//---------------------------------------------------------
