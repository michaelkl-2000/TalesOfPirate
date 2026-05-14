//=============================================================================
// FileName: ChaAttr.h
// Comment: CChaAttr — хранилище атрибутов персонажа (LV/HP/SP/STR/DEX/...).
//          Атрибуты живут в std::array<int32, ATTR_MAX_NUM>; bitset _dirty
//          отмечает изменённые с момента последней синхронизации с клиентом.
//          Глобальный std::array g_attrMax задаёт верхние границы (читается
//          из Lua-конфига при старте сервера).
//=============================================================================

#pragma once

#include "Character/ChaAttrType.h"

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <string_view>

namespace Corsairs::Common::Character {

// Верхние границы атрибутов, заполняемые из Lua при инициализации сервера.
// std::array — для bounds-checked доступа через .at() в SetAttr.
extern std::array<std::int32_t, ATTR_MAX_NUM> g_attrMax;

class CChaAttr
{
public:
    CChaAttr();
    ~CChaAttr() = default;

    void           Clear();
    void           Init(std::int32_t id, bool applyProgressionDefaults = true);

    [[nodiscard]] std::int32_t GetAttr(std::int32_t no) const;
    std::int32_t   SetAttr(std::int32_t no, std::int32_t val);
    [[nodiscard]] std::int32_t GetAttrMaxVal(std::int32_t no) const;
    std::int32_t   DirectSetAttr(std::int32_t no, std::int32_t val);
    std::int32_t   AddAttr(std::int32_t no, std::int32_t val);

    void           SetChangeFlag();
    void           ResetChangeFlag();
    void           SetChangeBitFlag(std::int32_t bit);
    [[nodiscard]] bool GetChangeBitFlag(std::int32_t bit) const;

    [[nodiscard]] std::int16_t GetChangeNumClient() const noexcept { return _changeNumClient; }

    [[nodiscard]] std::int32_t GetId() const noexcept { return _id; }
    void           SetId(std::int32_t id) noexcept { _id = id; }

    [[nodiscard]] std::string_view GetName() const noexcept { return _name; }
    void           SetName(std::string_view name) { _name = name; }

private:
    std::int32_t                              _id{};
    std::string                               _name;
    std::array<std::int32_t, ATTR_MAX_NUM>    _attribute{};
    std::bitset<ATTR_MAX_NUM>                 _dirty;
    std::int16_t                              _changeNumClient{};
};

} // namespace Corsairs::Common::Character
