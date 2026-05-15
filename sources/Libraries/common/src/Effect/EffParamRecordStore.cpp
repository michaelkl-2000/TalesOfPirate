#include "Effect/EffParamRecordStore.h"

#include <charconv>
#include <cstdint>
#include <string_view>


namespace Corsairs::Common::Effect {

namespace {
	// Разбить CSV-строку на токены. Пустые токены сохраняются как "".
	// Возвращает число фактически считанных токенов (не больше N).
	template <std::size_t N>
	std::size_t SplitCsvStrings(std::string_view text, std::array<std::string, N>& out) {
		for (auto& s : out) {
			s.clear();
		}
		if (text.empty()) {
			return 0;
		}
		std::size_t i = 0;
		std::size_t pos = 0;
		while (i < N) {
			std::size_t comma = text.find(',', pos);
			std::size_t end   = (comma == std::string_view::npos) ? text.size() : comma;
			out[i++].assign(text.data() + pos, end - pos);
			if (comma == std::string_view::npos) {
				break;
			}
			pos = comma + 1;
		}
		return i;
	}

	// Разбить CSV-строку на N int32. Пустые/неразобранные — fallback.
	template <std::size_t N>
	void SplitCsvInts(std::string_view text, std::array<std::int32_t, N>& out, std::int32_t fallback) {
		out.fill(fallback);
		if (text.empty()) {
			return;
		}
		std::size_t i = 0;
		std::size_t pos = 0;
		while (i < N) {
			std::size_t comma = text.find(',', pos);
			std::size_t end   = (comma == std::string_view::npos) ? text.size() : comma;
			std::int32_t v = fallback;
			std::from_chars(text.data() + pos, text.data() + end, v);
			out[i++] = v;
			if (comma == std::string_view::npos) {
				break;
			}
			pos = comma + 1;
		}
	}
} // namespace

GameRecordset<EffParamRecord>::RecordEntry EffParamRecordStore::ReadRecord(SqliteStatement& stmt) {
	EffParamRecord record{};
	int col = 0;

	record.Id       = stmt.GetInt(col++);
	record.DataName = stmt.GetText(col++);

	std::size_t nModels = SplitCsvStrings(stmt.GetText(col++), record.Models);
	record.ModelNum = 0;
	for (std::size_t i = 0; i < nModels; ++i) {
		if (!record.Models[i].empty()) {
			++record.ModelNum;
		}
	}

	record.Vel = stmt.GetInt(col++);

	std::size_t nParts = SplitCsvStrings(stmt.GetText(col++), record.Parts);
	record.PartNum = 0;
	for (std::size_t i = 0; i < nParts; ++i) {
		if (!record.Parts[i].empty()) {
			++record.PartNum;
		}
	}

	SplitCsvInts(stmt.GetText(col++), record.Dummies, -1);

	record.RenderIdx = stmt.GetInt(col++);
	record.LightId   = stmt.GetInt(col++);
	record.Result    = stmt.GetText(col++);

	std::string name = record.DataName;
	return {record.Id, std::move(name), std::move(record)};
}

} // namespace Corsairs::Common::Effect

