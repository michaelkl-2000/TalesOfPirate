//=============================================================================
// FileName: JobInitEquip.h
// Creater: ZhangXuedong
// Date: 2005.01.08
// Comment: Запись таблицы начального снаряжения по классам персонажа.
//=============================================================================

#pragma once

#include <array>
#include <cstdint>

#include "Database/TableData.h"


namespace Corsairs::Common::Inventory {

inline constexpr std::size_t JOB_INIT_EQUIP_MAX = 6;

struct JobEquipRecord : public EntityData
{
	std::int8_t                                       Job{0};
	std::array<std::int16_t, JOB_INIT_EQUIP_MAX>      ItemIds{};
};

} // namespace Corsairs::Common::Inventory
