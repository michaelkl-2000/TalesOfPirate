//=============================================================================
// FileName: ChaAction.h
// Comment: Флаги контроля действий персонажа (move, skill, trade, item, ...).
//          Используются как индекс в std::array<bool, kActControlCount> внутри
//          CCharacter — каждый флаг разрешает/запрещает соответствующее действие.
//=============================================================================

#pragma once

#include <cstdint>
#include <format>
#include <string_view>
#include <utility>

namespace Corsairs::Common::Character {

enum class ActControl : std::uint8_t {
    MOVE         = 0,
    USE_GSKILL   = 1,
    USE_MSKILL   = 2,
    BEUSE_SKILL  = 3,
    TRADE        = 4,
    USE_ITEM     = 5,
    BEUSE_ITEM   = 6,
    INVINCIBLE   = 7,
    EYESHOT      = 8,
    NOHIDE       = 9,
    NOSHOW       = 10,
    ITEM_OPT     = 11,
    TALKTO_NPC   = 12,
};

// Количество элементов в ActControl. Считаем от последнего значения, т.к. enum
// плотный 0..N-1 — поле _actControl[kActControlCount] полагается на этот layout.
inline constexpr std::size_t kActControlCount =
    std::to_underlying(ActControl::TALKTO_NPC) + 1;

// Текстовое имя значения — для логов, диагностики, std::format и потоков.
// switch без default: компилятор выдаст -Wswitch warning, если кто-то добавит
// новое значение в enum и забудет сюда; fallback в конце нужен только чтобы
// функция формально возвращала значение (UB-веток нет).
[[nodiscard]] constexpr std::string_view ToString(ActControl c) noexcept {
    switch (c) {
        case ActControl::MOVE:        return "MOVE";
        case ActControl::USE_GSKILL:  return "USE_GSKILL";
        case ActControl::USE_MSKILL:  return "USE_MSKILL";
        case ActControl::BEUSE_SKILL: return "BEUSE_SKILL";
        case ActControl::TRADE:       return "TRADE";
        case ActControl::USE_ITEM:    return "USE_ITEM";
        case ActControl::BEUSE_ITEM:  return "BEUSE_ITEM";
        case ActControl::INVINCIBLE:  return "INVINCIBLE";
        case ActControl::EYESHOT:     return "EYESHOT";
        case ActControl::NOHIDE:      return "NOHIDE";
        case ActControl::NOSHOW:      return "NOSHOW";
        case ActControl::ITEM_OPT:    return "ITEM_OPT";
        case ActControl::TALKTO_NPC:  return "TALKTO_NPC";
    }
    return "<unknown ActControl>";
}

// Stream-вывод. В namespace для ADL: os << ActControl::MOVE найдёт оператор без
// квалификатора. inline — потому что header-only функция.
inline std::ostream& operator<<(std::ostream& os, ActControl c) {
    return os << ToString(c);
}

} // namespace Corsairs::Common::Character

// Специализация std::formatter — позволяет std::format("{}", ActControl::MOVE).
// Наследование от formatter<string_view> бесплатно даёт спецификаторы выравнивания
// и ширины ({:>10}, {:.5} и т.п.) без ручной реализации parse().
template <>
struct std::formatter<ActControl>
    : std::formatter<std::string_view> {
    auto format(ActControl c, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(
            ToString(c), ctx);
    }
};
