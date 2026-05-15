#pragma once
#include <array>
#include <cstdint>
#include <string>

#include "Database/TableData.h"


namespace Corsairs::Common::Effect {

// DTO одной записи таблицы `eff_params` в gamedata.sqlite.
// Имя записи лежит в `EntityData::DataName` (из базы).
class EffParamRecord : public EntityData {
public:
	static constexpr std::size_t SLOT_COUNT = 8;

	std::int32_t                              ModelNum{0};
	std::array<std::string, SLOT_COUNT>       Models{};
	std::int32_t                              Vel{0};
	std::int32_t                              PartNum{0};
	std::array<std::string, SLOT_COUNT>       Parts{};
	std::array<std::int32_t, SLOT_COUNT>      Dummies{-1, -1, -1, -1, -1, -1, -1, -1};
	std::int32_t                              RenderIdx{-1};
	std::int32_t                              LightId{-1};
	std::string                               Result{};
};

} // namespace Corsairs::Common::Effect

