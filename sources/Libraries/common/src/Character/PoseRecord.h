#pragma once

#include "Database/TableData.h"
#include "Character/WieldMode.h"

#include <array>
#include <cstdint>
#include <source_location>

// Запись таблицы поз персонажа.
// RealPoseId[i] — реальный pose-id, который надо отдать в lwPoseCtrl для
// wield-mode со значением i. См. Character/WieldMode.h.

namespace Corsairs::Common::Character {

class PoseInfo : public EntityData {
public:
	std::array<std::int16_t, kPoseVariantCount> RealPoseId{};

	// Bounds-checked accessor: при выходе за [0, kPoseVariantCount) пишет
	// предупреждение в "errors" и возвращает 0.
	[[nodiscard]] std::int16_t GetRealPoseId(
		WieldMode mode,
		const std::source_location& loc = std::source_location::current()) const;

	[[nodiscard]] std::int16_t GetRealPoseId(
		std::size_t variant,
		const std::source_location& loc = std::source_location::current()) const;
};

} // namespace Corsairs::Common::Character
