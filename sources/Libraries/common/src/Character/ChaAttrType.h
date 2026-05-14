//=============================================================================
// FileName: ChaAttrType.h
// Comment: Индексы атрибутов персонажа — LV/HP/SP, базовые статы (STR..LUK),
//          модификаторы от итемов (ITEMC/ITEMV) и состояний (STATEC/STATEV),
//          корабельные данные (BOAT_*), скилл-коэффициенты корабля
//          (BOAT_SKILLC/V) и расширенные атрибуты (EXTEND0..9).
//
//          Атрибуты живут в плоском массиве m_lAttribute[ATTR_MAX_NUM] внутри
//          CChaAttr, доступ — через CChaAttr::Get/SetAttr(int).
//          Группы (BASE0..BASE10) — это семантические разделы;
//          сама нумерация плотная 0..ATTR_MAX_NUM-1, без дыр.
//=============================================================================

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace Corsairs::Common::Character {

// ===== BASE0: Core (динамическое состояние, 25 значений 0..24) =====
inline constexpr std::int32_t ATTR_COUNT_BASE0    = 0;
inline constexpr std::int32_t ATTR_LV             = ATTR_COUNT_BASE0 + 0;
inline constexpr std::int32_t ATTR_HP             = ATTR_COUNT_BASE0 + 1;
inline constexpr std::int32_t ATTR_SP             = ATTR_COUNT_BASE0 + 2;
inline constexpr std::int32_t ATTR_TITLE          = ATTR_COUNT_BASE0 + 3;
inline constexpr std::int32_t ATTR_JOB            = ATTR_COUNT_BASE0 + 4;
inline constexpr std::int32_t ATTR_FAME           = ATTR_COUNT_BASE0 + 5;
inline constexpr std::int32_t ATTR_AP             = ATTR_COUNT_BASE0 + 6;
inline constexpr std::int32_t ATTR_TP             = ATTR_COUNT_BASE0 + 7;
inline constexpr std::int32_t ATTR_GD             = ATTR_COUNT_BASE0 + 8;
inline constexpr std::int32_t ATTR_SPRI           = ATTR_COUNT_BASE0 + 9;
inline constexpr std::int32_t ATTR_CHATYPE        = ATTR_COUNT_BASE0 + 10; // тип сущности (player/NPC/monster)
inline constexpr std::int32_t ATTR_SAILLV         = ATTR_COUNT_BASE0 + 11;
inline constexpr std::int32_t ATTR_LIFELV         = ATTR_COUNT_BASE0 + 12;
inline constexpr std::int32_t ATTR_LIFETP         = ATTR_COUNT_BASE0 + 13;
inline constexpr std::int32_t ATTR_BOAT_BERTH     = ATTR_COUNT_BASE0 + 14;
// EXP-поля (15..24) — не «знаковые» атрибуты, см. IsExpAttr ниже
inline constexpr std::int32_t ATTR_CEXP           = ATTR_COUNT_BASE0 + 15;
inline constexpr std::int32_t ATTR_NLEXP          = ATTR_COUNT_BASE0 + 16;
inline constexpr std::int32_t ATTR_CLEXP          = ATTR_COUNT_BASE0 + 17;
inline constexpr std::int32_t ATTR_CLEFT_SAILEXP  = ATTR_COUNT_BASE0 + 18;
inline constexpr std::int32_t ATTR_CSAILEXP       = ATTR_COUNT_BASE0 + 19;
inline constexpr std::int32_t ATTR_CLV_SAILEXP    = ATTR_COUNT_BASE0 + 20;
inline constexpr std::int32_t ATTR_NLV_SAILEXP    = ATTR_COUNT_BASE0 + 21;
inline constexpr std::int32_t ATTR_CLIFEEXP       = ATTR_COUNT_BASE0 + 22;
inline constexpr std::int32_t ATTR_CLV_LIFEEXP    = ATTR_COUNT_BASE0 + 23;
inline constexpr std::int32_t ATTR_NLV_LIFEEXP    = ATTR_COUNT_BASE0 + 24;

// ===== BASE1: Stat (modified, 25 значений 25..49) =====
inline constexpr std::int32_t ATTR_COUNT_BASE1    = 25;
inline constexpr std::int32_t ATTR_STR            = ATTR_COUNT_BASE1 + 0;
inline constexpr std::int32_t ATTR_DEX            = ATTR_COUNT_BASE1 + 1;
inline constexpr std::int32_t ATTR_AGI            = ATTR_COUNT_BASE1 + 2;
inline constexpr std::int32_t ATTR_CON            = ATTR_COUNT_BASE1 + 3;
inline constexpr std::int32_t ATTR_STA            = ATTR_COUNT_BASE1 + 4;
inline constexpr std::int32_t ATTR_LUK            = ATTR_COUNT_BASE1 + 5;
inline constexpr std::int32_t ATTR_MXHP           = ATTR_COUNT_BASE1 + 6;
inline constexpr std::int32_t ATTR_MXSP           = ATTR_COUNT_BASE1 + 7;
inline constexpr std::int32_t ATTR_MNATK          = ATTR_COUNT_BASE1 + 8;
inline constexpr std::int32_t ATTR_MXATK          = ATTR_COUNT_BASE1 + 9;
inline constexpr std::int32_t ATTR_DEF            = ATTR_COUNT_BASE1 + 10;
inline constexpr std::int32_t ATTR_HIT            = ATTR_COUNT_BASE1 + 11;
inline constexpr std::int32_t ATTR_FLEE           = ATTR_COUNT_BASE1 + 12;
inline constexpr std::int32_t ATTR_MF             = ATTR_COUNT_BASE1 + 13;
inline constexpr std::int32_t ATTR_CRT            = ATTR_COUNT_BASE1 + 14;
inline constexpr std::int32_t ATTR_HREC           = ATTR_COUNT_BASE1 + 15;
inline constexpr std::int32_t ATTR_SREC           = ATTR_COUNT_BASE1 + 16;
inline constexpr std::int32_t ATTR_ASPD           = ATTR_COUNT_BASE1 + 17;
inline constexpr std::int32_t ATTR_ADIS           = ATTR_COUNT_BASE1 + 18;
inline constexpr std::int32_t ATTR_MSPD           = ATTR_COUNT_BASE1 + 19;
inline constexpr std::int32_t ATTR_COL            = ATTR_COUNT_BASE1 + 20;
inline constexpr std::int32_t ATTR_PDEF           = ATTR_COUNT_BASE1 + 21;
inline constexpr std::int32_t ATTR_BOAT_CRANGE    = ATTR_COUNT_BASE1 + 22;
inline constexpr std::int32_t ATTR_BOAT_CSPD      = ATTR_COUNT_BASE1 + 23;
inline constexpr std::int32_t ATTR_BOAT_PRICE     = ATTR_COUNT_BASE1 + 24;

// ===== BASE2: Base stat ('B'-prefix, 24 значения 50..73) =====
// Нет аналога ATTR_BOAT_PRICE — цена корабля не имеет «base value».
inline constexpr std::int32_t ATTR_COUNT_BASE2    = 50;
inline constexpr std::int32_t ATTR_BSTR           = ATTR_COUNT_BASE2 + 0;
inline constexpr std::int32_t ATTR_BDEX           = ATTR_COUNT_BASE2 + 1;
inline constexpr std::int32_t ATTR_BAGI           = ATTR_COUNT_BASE2 + 2;
inline constexpr std::int32_t ATTR_BCON           = ATTR_COUNT_BASE2 + 3;
inline constexpr std::int32_t ATTR_BSTA           = ATTR_COUNT_BASE2 + 4;
inline constexpr std::int32_t ATTR_BLUK           = ATTR_COUNT_BASE2 + 5;
inline constexpr std::int32_t ATTR_BMXHP          = ATTR_COUNT_BASE2 + 6;
inline constexpr std::int32_t ATTR_BMXSP          = ATTR_COUNT_BASE2 + 7;
inline constexpr std::int32_t ATTR_BMNATK         = ATTR_COUNT_BASE2 + 8;
inline constexpr std::int32_t ATTR_BMXATK         = ATTR_COUNT_BASE2 + 9;
inline constexpr std::int32_t ATTR_BDEF           = ATTR_COUNT_BASE2 + 10;
inline constexpr std::int32_t ATTR_BHIT           = ATTR_COUNT_BASE2 + 11;
inline constexpr std::int32_t ATTR_BFLEE          = ATTR_COUNT_BASE2 + 12;
inline constexpr std::int32_t ATTR_BMF            = ATTR_COUNT_BASE2 + 13;
inline constexpr std::int32_t ATTR_BCRT           = ATTR_COUNT_BASE2 + 14;
inline constexpr std::int32_t ATTR_BHREC          = ATTR_COUNT_BASE2 + 15;
inline constexpr std::int32_t ATTR_BSREC          = ATTR_COUNT_BASE2 + 16;
inline constexpr std::int32_t ATTR_BASPD          = ATTR_COUNT_BASE2 + 17;
inline constexpr std::int32_t ATTR_BADIS          = ATTR_COUNT_BASE2 + 18;
inline constexpr std::int32_t ATTR_BMSPD          = ATTR_COUNT_BASE2 + 19;
inline constexpr std::int32_t ATTR_BCOL           = ATTR_COUNT_BASE2 + 20;
inline constexpr std::int32_t ATTR_BPDEF          = ATTR_COUNT_BASE2 + 21;
inline constexpr std::int32_t ATTR_BOAT_BCRANGE   = ATTR_COUNT_BASE2 + 22;
inline constexpr std::int32_t ATTR_BOAT_BCSPD     = ATTR_COUNT_BASE2 + 23;

// ===== BASE3: Item coefficient (22 значения 74..95) =====
inline constexpr std::int32_t ATTR_COUNT_BASE3    = 74;
inline constexpr std::int32_t ATTR_ITEMC_STR      = ATTR_COUNT_BASE3 + 0;
inline constexpr std::int32_t ATTR_ITEMC_AGI      = ATTR_COUNT_BASE3 + 1;
inline constexpr std::int32_t ATTR_ITEMC_DEX      = ATTR_COUNT_BASE3 + 2;
inline constexpr std::int32_t ATTR_ITEMC_CON      = ATTR_COUNT_BASE3 + 3;
inline constexpr std::int32_t ATTR_ITEMC_STA      = ATTR_COUNT_BASE3 + 4;
inline constexpr std::int32_t ATTR_ITEMC_LUK      = ATTR_COUNT_BASE3 + 5;
inline constexpr std::int32_t ATTR_ITEMC_ASPD     = ATTR_COUNT_BASE3 + 6;
inline constexpr std::int32_t ATTR_ITEMC_ADIS     = ATTR_COUNT_BASE3 + 7;
inline constexpr std::int32_t ATTR_ITEMC_MNATK    = ATTR_COUNT_BASE3 + 8;
inline constexpr std::int32_t ATTR_ITEMC_MXATK    = ATTR_COUNT_BASE3 + 9;
inline constexpr std::int32_t ATTR_ITEMC_DEF      = ATTR_COUNT_BASE3 + 10;
inline constexpr std::int32_t ATTR_ITEMC_MXHP     = ATTR_COUNT_BASE3 + 11;
inline constexpr std::int32_t ATTR_ITEMC_MXSP     = ATTR_COUNT_BASE3 + 12;
inline constexpr std::int32_t ATTR_ITEMC_FLEE     = ATTR_COUNT_BASE3 + 13;
inline constexpr std::int32_t ATTR_ITEMC_HIT      = ATTR_COUNT_BASE3 + 14;
inline constexpr std::int32_t ATTR_ITEMC_CRT      = ATTR_COUNT_BASE3 + 15;
inline constexpr std::int32_t ATTR_ITEMC_MF       = ATTR_COUNT_BASE3 + 16;
inline constexpr std::int32_t ATTR_ITEMC_HREC     = ATTR_COUNT_BASE3 + 17;
inline constexpr std::int32_t ATTR_ITEMC_SREC     = ATTR_COUNT_BASE3 + 18;
inline constexpr std::int32_t ATTR_ITEMC_MSPD     = ATTR_COUNT_BASE3 + 19;
inline constexpr std::int32_t ATTR_ITEMC_COL      = ATTR_COUNT_BASE3 + 20;
inline constexpr std::int32_t ATTR_ITEMC_PDEF     = ATTR_COUNT_BASE3 + 21;

// ===== BASE4: Item value (22 значения 96..117) =====
inline constexpr std::int32_t ATTR_COUNT_BASE4    = 96;
inline constexpr std::int32_t ATTR_ITEMV_STR      = ATTR_COUNT_BASE4 + 0;
inline constexpr std::int32_t ATTR_ITEMV_AGI      = ATTR_COUNT_BASE4 + 1;
inline constexpr std::int32_t ATTR_ITEMV_DEX      = ATTR_COUNT_BASE4 + 2;
inline constexpr std::int32_t ATTR_ITEMV_CON      = ATTR_COUNT_BASE4 + 3;
inline constexpr std::int32_t ATTR_ITEMV_STA      = ATTR_COUNT_BASE4 + 4;
inline constexpr std::int32_t ATTR_ITEMV_LUK      = ATTR_COUNT_BASE4 + 5;
inline constexpr std::int32_t ATTR_ITEMV_ASPD     = ATTR_COUNT_BASE4 + 6;
inline constexpr std::int32_t ATTR_ITEMV_ADIS     = ATTR_COUNT_BASE4 + 7;
inline constexpr std::int32_t ATTR_ITEMV_MNATK    = ATTR_COUNT_BASE4 + 8;
inline constexpr std::int32_t ATTR_ITEMV_MXATK    = ATTR_COUNT_BASE4 + 9;
inline constexpr std::int32_t ATTR_ITEMV_DEF      = ATTR_COUNT_BASE4 + 10;
inline constexpr std::int32_t ATTR_ITEMV_MXHP     = ATTR_COUNT_BASE4 + 11;
inline constexpr std::int32_t ATTR_ITEMV_MXSP     = ATTR_COUNT_BASE4 + 12;
inline constexpr std::int32_t ATTR_ITEMV_FLEE     = ATTR_COUNT_BASE4 + 13;
inline constexpr std::int32_t ATTR_ITEMV_HIT      = ATTR_COUNT_BASE4 + 14;
inline constexpr std::int32_t ATTR_ITEMV_CRT      = ATTR_COUNT_BASE4 + 15;
inline constexpr std::int32_t ATTR_ITEMV_MF       = ATTR_COUNT_BASE4 + 16;
inline constexpr std::int32_t ATTR_ITEMV_HREC     = ATTR_COUNT_BASE4 + 17;
inline constexpr std::int32_t ATTR_ITEMV_SREC     = ATTR_COUNT_BASE4 + 18;
inline constexpr std::int32_t ATTR_ITEMV_MSPD     = ATTR_COUNT_BASE4 + 19;
inline constexpr std::int32_t ATTR_ITEMV_COL      = ATTR_COUNT_BASE4 + 20;
inline constexpr std::int32_t ATTR_ITEMV_PDEF     = ATTR_COUNT_BASE4 + 21;

// ===== BASE5: State coefficient (22 значения 118..139) =====
inline constexpr std::int32_t ATTR_COUNT_BASE5    = 118;
inline constexpr std::int32_t ATTR_STATEC_STR     = ATTR_COUNT_BASE5 + 0;
inline constexpr std::int32_t ATTR_STATEC_AGI     = ATTR_COUNT_BASE5 + 1;
inline constexpr std::int32_t ATTR_STATEC_DEX     = ATTR_COUNT_BASE5 + 2;
inline constexpr std::int32_t ATTR_STATEC_CON     = ATTR_COUNT_BASE5 + 3;
inline constexpr std::int32_t ATTR_STATEC_STA     = ATTR_COUNT_BASE5 + 4;
inline constexpr std::int32_t ATTR_STATEC_LUK     = ATTR_COUNT_BASE5 + 5;
inline constexpr std::int32_t ATTR_STATEC_ASPD    = ATTR_COUNT_BASE5 + 6;
inline constexpr std::int32_t ATTR_STATEC_ADIS    = ATTR_COUNT_BASE5 + 7;
inline constexpr std::int32_t ATTR_STATEC_MNATK   = ATTR_COUNT_BASE5 + 8;
inline constexpr std::int32_t ATTR_STATEC_MXATK   = ATTR_COUNT_BASE5 + 9;
inline constexpr std::int32_t ATTR_STATEC_DEF     = ATTR_COUNT_BASE5 + 10;
inline constexpr std::int32_t ATTR_STATEC_MXHP    = ATTR_COUNT_BASE5 + 11;
inline constexpr std::int32_t ATTR_STATEC_MXSP    = ATTR_COUNT_BASE5 + 12;
inline constexpr std::int32_t ATTR_STATEC_FLEE    = ATTR_COUNT_BASE5 + 13;
inline constexpr std::int32_t ATTR_STATEC_HIT     = ATTR_COUNT_BASE5 + 14;
inline constexpr std::int32_t ATTR_STATEC_CRT     = ATTR_COUNT_BASE5 + 15;
inline constexpr std::int32_t ATTR_STATEC_MF      = ATTR_COUNT_BASE5 + 16;
inline constexpr std::int32_t ATTR_STATEC_HREC    = ATTR_COUNT_BASE5 + 17;
inline constexpr std::int32_t ATTR_STATEC_SREC    = ATTR_COUNT_BASE5 + 18;
inline constexpr std::int32_t ATTR_STATEC_MSPD    = ATTR_COUNT_BASE5 + 19;
inline constexpr std::int32_t ATTR_STATEC_COL     = ATTR_COUNT_BASE5 + 20;
inline constexpr std::int32_t ATTR_STATEC_PDEF    = ATTR_COUNT_BASE5 + 21;

// ===== BASE6: State value (22 значения 140..161 + LHAND_ITEMV в 162) =====
// LHAND_ITEMV семантически принадлежит ITEMV (BASE4), но физически живёт здесь.
// Унаследованный hack — единственное свободное место при добавлении dual-wielding.
inline constexpr std::int32_t ATTR_COUNT_BASE6    = 140;
inline constexpr std::int32_t ATTR_STATEV_STR     = ATTR_COUNT_BASE6 + 0;
inline constexpr std::int32_t ATTR_STATEV_AGI     = ATTR_COUNT_BASE6 + 1;
inline constexpr std::int32_t ATTR_STATEV_DEX     = ATTR_COUNT_BASE6 + 2;
inline constexpr std::int32_t ATTR_STATEV_CON     = ATTR_COUNT_BASE6 + 3;
inline constexpr std::int32_t ATTR_STATEV_STA     = ATTR_COUNT_BASE6 + 4;
inline constexpr std::int32_t ATTR_STATEV_LUK     = ATTR_COUNT_BASE6 + 5;
inline constexpr std::int32_t ATTR_STATEV_ASPD    = ATTR_COUNT_BASE6 + 6;
inline constexpr std::int32_t ATTR_STATEV_ADIS    = ATTR_COUNT_BASE6 + 7;
inline constexpr std::int32_t ATTR_STATEV_MNATK   = ATTR_COUNT_BASE6 + 8;
inline constexpr std::int32_t ATTR_STATEV_MXATK   = ATTR_COUNT_BASE6 + 9;
inline constexpr std::int32_t ATTR_STATEV_DEF     = ATTR_COUNT_BASE6 + 10;
inline constexpr std::int32_t ATTR_STATEV_MXHP    = ATTR_COUNT_BASE6 + 11;
inline constexpr std::int32_t ATTR_STATEV_MXSP    = ATTR_COUNT_BASE6 + 12;
inline constexpr std::int32_t ATTR_STATEV_FLEE    = ATTR_COUNT_BASE6 + 13;
inline constexpr std::int32_t ATTR_STATEV_HIT     = ATTR_COUNT_BASE6 + 14;
inline constexpr std::int32_t ATTR_STATEV_CRT     = ATTR_COUNT_BASE6 + 15;
inline constexpr std::int32_t ATTR_STATEV_MF      = ATTR_COUNT_BASE6 + 16;
inline constexpr std::int32_t ATTR_STATEV_HREC    = ATTR_COUNT_BASE6 + 17;
inline constexpr std::int32_t ATTR_STATEV_SREC    = ATTR_COUNT_BASE6 + 18;
inline constexpr std::int32_t ATTR_STATEV_MSPD    = ATTR_COUNT_BASE6 + 19;
inline constexpr std::int32_t ATTR_STATEV_COL     = ATTR_COUNT_BASE6 + 20;
inline constexpr std::int32_t ATTR_STATEV_PDEF    = ATTR_COUNT_BASE6 + 21;
inline constexpr std::int32_t ATTR_LHAND_ITEMV    = ATTR_COUNT_BASE6 + 22;

// ===== BASE7: Boat info (9 значений 163..171) =====
inline constexpr std::int32_t ATTR_COUNT_BASE7    = 163;
inline constexpr std::int32_t ATTR_BOAT_SHIP      = ATTR_COUNT_BASE7 + 0;
inline constexpr std::int32_t ATTR_BOAT_HEADER    = ATTR_COUNT_BASE7 + 1;
inline constexpr std::int32_t ATTR_BOAT_BODY      = ATTR_COUNT_BASE7 + 2;
inline constexpr std::int32_t ATTR_BOAT_ENGINE    = ATTR_COUNT_BASE7 + 3;
inline constexpr std::int32_t ATTR_BOAT_CANNON    = ATTR_COUNT_BASE7 + 4;
inline constexpr std::int32_t ATTR_BOAT_PART      = ATTR_COUNT_BASE7 + 5;
inline constexpr std::int32_t ATTR_BOAT_DBID      = ATTR_COUNT_BASE7 + 6;
inline constexpr std::int32_t ATTR_BOAT_DIECOUNT  = ATTR_COUNT_BASE7 + 7;
inline constexpr std::int32_t ATTR_BOAT_ISDEAD    = ATTR_COUNT_BASE7 + 8;

// ===== BASE8: Boat skill coefficient (15 значений 172..186) =====
inline constexpr std::int32_t ATTR_COUNT_BASE8           = 172;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_MNATK     = ATTR_COUNT_BASE8 + 0;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_MXATK     = ATTR_COUNT_BASE8 + 1;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_ADIS      = ATTR_COUNT_BASE8 + 2;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_MSPD      = ATTR_COUNT_BASE8 + 3;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_CSPD      = ATTR_COUNT_BASE8 + 4;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_ASPD      = ATTR_COUNT_BASE8 + 5;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_CRANGE    = ATTR_COUNT_BASE8 + 6;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_DEF       = ATTR_COUNT_BASE8 + 7;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_RESIST    = ATTR_COUNT_BASE8 + 8;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_MXUSE     = ATTR_COUNT_BASE8 + 9;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_USEREC    = ATTR_COUNT_BASE8 + 10;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_EXP       = ATTR_COUNT_BASE8 + 11;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_CPT       = ATTR_COUNT_BASE8 + 12;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_SPD       = ATTR_COUNT_BASE8 + 13;
inline constexpr std::int32_t ATTR_BOAT_SKILLC_MXSPLY    = ATTR_COUNT_BASE8 + 14;

// ===== BASE9: Boat skill value (15 значений 187..201) =====
inline constexpr std::int32_t ATTR_COUNT_BASE9           = 187;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_MNATK     = ATTR_COUNT_BASE9 + 0;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_MXATK     = ATTR_COUNT_BASE9 + 1;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_ADIS      = ATTR_COUNT_BASE9 + 2;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_MSPD      = ATTR_COUNT_BASE9 + 3;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_CSPD      = ATTR_COUNT_BASE9 + 4;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_ASPD      = ATTR_COUNT_BASE9 + 5;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_CRANGE    = ATTR_COUNT_BASE9 + 6;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_DEF       = ATTR_COUNT_BASE9 + 7;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_RESIST    = ATTR_COUNT_BASE9 + 8;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_MXUSE     = ATTR_COUNT_BASE9 + 9;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_USEREC    = ATTR_COUNT_BASE9 + 10;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_EXP       = ATTR_COUNT_BASE9 + 11;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_CPT       = ATTR_COUNT_BASE9 + 12;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_SPD       = ATTR_COUNT_BASE9 + 13;
inline constexpr std::int32_t ATTR_BOAT_SKILLV_MXSPLY    = ATTR_COUNT_BASE9 + 14;

// ===== BASE10: Extend (10 значений 202..211, lark.li 2008-07-23) =====
// До 2026-05 ATTR_MAX_NUM был 202 — EXTEND молча обрезался защитой
// `if (lNo >= ATTR_MAX_NUM) return 0` во всех Get/Set методах, а БД хранила
// в поле character.extend строку "-1,-1,...,-1". Сейчас включён в работу.
inline constexpr std::int32_t ATTR_COUNT_BASE10   = 202;
inline constexpr std::int32_t ATTR_EXTEND0        = ATTR_COUNT_BASE10 + 0;
inline constexpr std::int32_t ATTR_EXTEND1        = ATTR_COUNT_BASE10 + 1;
inline constexpr std::int32_t ATTR_EXTEND2        = ATTR_COUNT_BASE10 + 2;
inline constexpr std::int32_t ATTR_EXTEND3        = ATTR_COUNT_BASE10 + 3;
inline constexpr std::int32_t ATTR_EXTEND4        = ATTR_COUNT_BASE10 + 4;
inline constexpr std::int32_t ATTR_EXTEND5        = ATTR_COUNT_BASE10 + 5;
inline constexpr std::int32_t ATTR_EXTEND6        = ATTR_COUNT_BASE10 + 6;
inline constexpr std::int32_t ATTR_EXTEND7        = ATTR_COUNT_BASE10 + 7;
inline constexpr std::int32_t ATTR_EXTEND8        = ATTR_COUNT_BASE10 + 8;
inline constexpr std::int32_t ATTR_EXTEND9        = ATTR_COUNT_BASE10 + 9;

// ===== Размеры и derived константы =====

// Полное количество атрибутов = последний EXTEND9 + 1.
inline constexpr std::int32_t ATTR_MAX_NUM    = ATTR_EXTEND9 + 1;          // 212

// Часть атрибутов, синхронизируемая с клиентом (Core+Stat+BaseStat).
// ITEMC и далее живут только на сервере для расчётов.
inline constexpr std::int32_t ATTR_CLIENT_MAX = ATTR_ITEMC_STR;            // 74

// Битовые маски «изменённых атрибутов» — для bitset'а в CChaAttr.
inline constexpr std::int32_t ATTR_CLIENT_SIGN_BYTE_NUM = (ATTR_CLIENT_MAX + 7) / 8;  // 10
inline constexpr std::int32_t ATTR_SIGN_BYTE_NUM        = (ATTR_MAX_NUM    + 7) / 8;  // 27

// ===== Метаданные для логов и диагностики =====

// Текстовые имена индексов 0..ATTR_MAX_NUM-1.
inline constexpr std::array<std::string_view, ATTR_MAX_NUM> kAttrNames = {
    // BASE0: Core (0..24)
    "LV", "HP", "SP", "TITLE", "JOB", "FAME", "AP", "TP", "GD", "SPRI",
    "CHATYPE", "SAILLV", "LIFELV", "LIFETP", "BOAT_BERTH",
    "CEXP", "NLEXP", "CLEXP", "CLEFT_SAILEXP", "CSAILEXP",
    "CLV_SAILEXP", "NLV_SAILEXP", "CLIFEEXP", "CLV_LIFEEXP", "NLV_LIFEEXP",
    // BASE1: Stat (25..49)
    "STR", "DEX", "AGI", "CON", "STA", "LUK",
    "MXHP", "MXSP", "MNATK", "MXATK", "DEF", "HIT", "FLEE", "MF", "CRT",
    "HREC", "SREC", "ASPD", "ADIS", "MSPD", "COL", "PDEF",
    "BOAT_CRANGE", "BOAT_CSPD", "BOAT_PRICE",
    // BASE2: BaseStat (50..73)
    "BSTR", "BDEX", "BAGI", "BCON", "BSTA", "BLUK",
    "BMXHP", "BMXSP", "BMNATK", "BMXATK", "BDEF", "BHIT", "BFLEE", "BMF", "BCRT",
    "BHREC", "BSREC", "BASPD", "BADIS", "BMSPD", "BCOL", "BPDEF",
    "BOAT_BCRANGE", "BOAT_BCSPD",
    // BASE3: ItemCoef (74..95)
    "ITEMC_STR", "ITEMC_AGI", "ITEMC_DEX", "ITEMC_CON", "ITEMC_STA", "ITEMC_LUK",
    "ITEMC_ASPD", "ITEMC_ADIS", "ITEMC_MNATK", "ITEMC_MXATK", "ITEMC_DEF",
    "ITEMC_MXHP", "ITEMC_MXSP", "ITEMC_FLEE", "ITEMC_HIT", "ITEMC_CRT",
    "ITEMC_MF", "ITEMC_HREC", "ITEMC_SREC", "ITEMC_MSPD", "ITEMC_COL", "ITEMC_PDEF",
    // BASE4: ItemVal (96..117)
    "ITEMV_STR", "ITEMV_AGI", "ITEMV_DEX", "ITEMV_CON", "ITEMV_STA", "ITEMV_LUK",
    "ITEMV_ASPD", "ITEMV_ADIS", "ITEMV_MNATK", "ITEMV_MXATK", "ITEMV_DEF",
    "ITEMV_MXHP", "ITEMV_MXSP", "ITEMV_FLEE", "ITEMV_HIT", "ITEMV_CRT",
    "ITEMV_MF", "ITEMV_HREC", "ITEMV_SREC", "ITEMV_MSPD", "ITEMV_COL", "ITEMV_PDEF",
    // BASE5: StateCoef (118..139)
    "STATEC_STR", "STATEC_AGI", "STATEC_DEX", "STATEC_CON", "STATEC_STA", "STATEC_LUK",
    "STATEC_ASPD", "STATEC_ADIS", "STATEC_MNATK", "STATEC_MXATK", "STATEC_DEF",
    "STATEC_MXHP", "STATEC_MXSP", "STATEC_FLEE", "STATEC_HIT", "STATEC_CRT",
    "STATEC_MF", "STATEC_HREC", "STATEC_SREC", "STATEC_MSPD", "STATEC_COL", "STATEC_PDEF",
    // BASE6: StateVal (140..161) + LHAND_ITEMV (162)
    "STATEV_STR", "STATEV_AGI", "STATEV_DEX", "STATEV_CON", "STATEV_STA", "STATEV_LUK",
    "STATEV_ASPD", "STATEV_ADIS", "STATEV_MNATK", "STATEV_MXATK", "STATEV_DEF",
    "STATEV_MXHP", "STATEV_MXSP", "STATEV_FLEE", "STATEV_HIT", "STATEV_CRT",
    "STATEV_MF", "STATEV_HREC", "STATEV_SREC", "STATEV_MSPD", "STATEV_COL", "STATEV_PDEF",
    "LHAND_ITEMV",
    // BASE7: BoatInfo (163..171)
    "BOAT_SHIP", "BOAT_HEADER", "BOAT_BODY", "BOAT_ENGINE", "BOAT_CANNON",
    "BOAT_PART", "BOAT_DBID", "BOAT_DIECOUNT", "BOAT_ISDEAD",
    // BASE8: BoatSkillCoef (172..186)
    "BOAT_SKILLC_MNATK", "BOAT_SKILLC_MXATK", "BOAT_SKILLC_ADIS", "BOAT_SKILLC_MSPD",
    "BOAT_SKILLC_CSPD", "BOAT_SKILLC_ASPD", "BOAT_SKILLC_CRANGE", "BOAT_SKILLC_DEF",
    "BOAT_SKILLC_RESIST", "BOAT_SKILLC_MXUSE", "BOAT_SKILLC_USEREC", "BOAT_SKILLC_EXP",
    "BOAT_SKILLC_CPT", "BOAT_SKILLC_SPD", "BOAT_SKILLC_MXSPLY",
    // BASE9: BoatSkillVal (187..201)
    "BOAT_SKILLV_MNATK", "BOAT_SKILLV_MXATK", "BOAT_SKILLV_ADIS", "BOAT_SKILLV_MSPD",
    "BOAT_SKILLV_CSPD", "BOAT_SKILLV_ASPD", "BOAT_SKILLV_CRANGE", "BOAT_SKILLV_DEF",
    "BOAT_SKILLV_RESIST", "BOAT_SKILLV_MXUSE", "BOAT_SKILLV_USEREC", "BOAT_SKILLV_EXP",
    "BOAT_SKILLV_CPT", "BOAT_SKILLV_SPD", "BOAT_SKILLV_MXSPLY",
    // BASE10: Extend (202..211)
    "EXTEND0", "EXTEND1", "EXTEND2", "EXTEND3", "EXTEND4",
    "EXTEND5", "EXTEND6", "EXTEND7", "EXTEND8", "EXTEND9",
};

// Категории — помогают при логировании и при будущих per-category операциях.
enum class AttrCategory : std::uint8_t {
    Core           = 0,
    Stat           = 1,
    BaseStat       = 2,
    ItemCoef       = 3,
    ItemVal        = 4,
    StateCoef      = 5,
    StateVal       = 6,  // включая LHAND_ITEMV
    BoatInfo       = 7,
    BoatSkillCoef  = 8,
    BoatSkillVal   = 9,
    Extend         = 10,
};

[[nodiscard]] constexpr bool IsValidAttr(std::int32_t attr) noexcept {
    return attr >= 0 && attr < ATTR_MAX_NUM;
}

[[nodiscard]] constexpr bool IsClientAttr(std::int32_t attr) noexcept {
    return attr >= 0 && attr < ATTR_CLIENT_MAX;
}

// EXP-поля не сериализуются как «знаковые» (имеют свою знаковую логику).
[[nodiscard]] constexpr bool IsExpAttr(std::int32_t attr) noexcept {
    return attr >= ATTR_CEXP && attr <= ATTR_NLV_LIFEEXP;
}

[[nodiscard]] constexpr AttrCategory CategoryOf(std::int32_t attr) noexcept {
    if (attr < ATTR_COUNT_BASE1)  return AttrCategory::Core;
    if (attr < ATTR_COUNT_BASE2)  return AttrCategory::Stat;
    if (attr < ATTR_COUNT_BASE3)  return AttrCategory::BaseStat;
    if (attr < ATTR_COUNT_BASE4)  return AttrCategory::ItemCoef;
    if (attr < ATTR_COUNT_BASE5)  return AttrCategory::ItemVal;
    if (attr < ATTR_COUNT_BASE6)  return AttrCategory::StateCoef;
    if (attr < ATTR_COUNT_BASE7)  return AttrCategory::StateVal;  // LHAND_ITEMV включён в StateVal-блок
    if (attr < ATTR_COUNT_BASE8)  return AttrCategory::BoatInfo;
    if (attr < ATTR_COUNT_BASE9)  return AttrCategory::BoatSkillCoef;
    if (attr < ATTR_COUNT_BASE10) return AttrCategory::BoatSkillVal;
    return AttrCategory::Extend;
}

// Текстовое имя атрибута для логов: ToString(ATTR_HP) → "HP".
[[nodiscard]] constexpr std::string_view ToString(std::int32_t attr) noexcept {
    if (!IsValidAttr(attr)) return "<unknown ATTR>";
    return kAttrNames[attr];
}

} // namespace Corsairs::Common::Character
