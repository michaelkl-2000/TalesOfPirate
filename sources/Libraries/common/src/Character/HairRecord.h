//======================================================================================================================
// FileName: HairRecord.h
// Comment: CHairRecord — DTO одной строки таблицы причёсок (цвет, цена, набор ингредиентов,
//          альтернативные предметы при неудаче, флаги доступности по типу персонажа).
//======================================================================================================================

#pragma once

#include "Database/TableData.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <source_location>
#include <string>


namespace Corsairs::Common::Character {

inline constexpr std::size_t kHairMaxNeedItems       = 4;
inline constexpr std::size_t kHairMaxFailItems       = 3;
inline constexpr std::size_t kHairCharacterTypeCount = 4;

struct HairNeedItem {
	std::uint32_t Id{};
	std::uint32_t Count{};
};

struct HairRecord : public EntityData
{
	std::string Color;

	std::array<HairNeedItem, kHairMaxNeedItems>  NeedItems{};
	std::uint32_t                                Cost{};
	std::uint32_t                                ResultItemId{};
	std::array<std::uint32_t, kHairMaxFailItems> FailItemIds{};
	std::array<bool, kHairCharacterTypeCount>    IsUsableByCharacterType{};

	// Возвращает число валидных fail-предметов (соответствует старой логике
	// «считаем до первого нулевого id» — массив трактуется как prefix-заполненный).
	[[nodiscard]] int GetFailItemCount() const noexcept {
		const auto it = std::ranges::find(FailItemIds, std::uint32_t{0});
		return static_cast<int>(it - FailItemIds.begin());
	}
};

HairRecord* GetHairRecordInfo(int nTypeID, const std::source_location& loc = std::source_location::current());


} // namespace Corsairs::Common::Character
