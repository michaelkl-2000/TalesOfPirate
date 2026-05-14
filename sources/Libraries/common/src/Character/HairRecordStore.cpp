#include "Character/HairRecordStore.h"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <string>

// ============================================================================
// Вспомогательные функции парсинга CSV
// ============================================================================


namespace Corsairs::Common::Character {

void HairRecordStore::ParseUint32Array(std::string_view text, std::span<std::uint32_t> out) {
	std::ranges::fill(out, std::uint32_t{0});
	if (text.empty() || text == "0") {
		return;
	}

	std::size_t i = 0;
	const char* begin = text.data();
	const char* end   = text.data() + text.size();
	while (i < out.size() && begin <= end) {
		const char* comma = std::find(begin, end, ',');
		std::uint32_t value = 0;
		std::from_chars(begin, comma, value);
		out[i++] = value;
		if (comma == end) {
			break;
		}
		begin = comma + 1;
	}
}

void HairRecordStore::ParseBoolArray(std::string_view text, std::span<bool> out) {
	std::ranges::fill(out, false);
	if (text.empty() || text == "0") {
		return;
	}

	std::size_t i = 0;
	const char* begin = text.data();
	const char* end   = text.data() + text.size();
	while (i < out.size() && begin <= end) {
		const char* comma = std::find(begin, end, ',');
		int value = 0;
		std::from_chars(begin, comma, value);
		out[i++] = (value != 0);
		if (comma == end) {
			break;
		}
		begin = comma + 1;
	}
}

// ============================================================================
// ReadRecord — конструирует CHairRecord из строки SQLite-запроса
// ============================================================================

GameRecordset<HairRecord>::RecordEntry HairRecordStore::ReadRecord(SqliteStatement& stmt) {
	HairRecord record{};
	int col = 0;

	// id
	record.Id    = stmt.GetInt(col++);

	// color
	record.Color = stmt.GetText(col++);

	// need_items — NeedItems[4], хранится как "id0,cnt0,id1,cnt1,id2,cnt2,id3,cnt3"
	{
		std::array<std::uint32_t, kHairMaxNeedItems * 2> flat{};
		ParseUint32Array(stmt.GetText(col++), flat);
		for (std::size_t i = 0; i < kHairMaxNeedItems; ++i) {
			record.NeedItems[i].Id    = flat[i * 2];
			record.NeedItems[i].Count = flat[i * 2 + 1];
		}
	}

	// money
	record.Cost = static_cast<std::uint32_t>(stmt.GetInt(col++));

	// item_id
	record.ResultItemId = static_cast<std::uint32_t>(stmt.GetInt(col++));

	// fail_item_ids — FailItemIds[3]
	ParseUint32Array(stmt.GetText(col++), record.FailItemIds);

	// is_cha_use — bool[4]
	ParseBoolArray(stmt.GetText(col++), record.IsUsableByCharacterType);

	return {record.Id, {}, std::move(record)};
}

} // namespace Corsairs::Common::Character

