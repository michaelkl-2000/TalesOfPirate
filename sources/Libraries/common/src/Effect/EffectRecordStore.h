#pragma once

#include <format>
#include <source_location>
#include <string_view>

#include "Database/GameRecordset.h"
#include "Effect/EffectRecord.h"

// Хранилище таблицы эффектов (магия) на базе SQLite.

namespace Corsairs::Common::Effect {

class EffectRecordStore : public GameRecordset<CEffectRecord> {
public:
	static EffectRecordStore* Instance() {
		static EffectRecordStore instance{};
		return &instance;
	}

	static constexpr const char* TABLE_NAME = "effects";

	static constexpr const char* CREATE_TABLE_SQL = R"(
		CREATE TABLE IF NOT EXISTS effects (
			id         INTEGER PRIMARY KEY,
			data_name  TEXT,
			photo_name TEXT,
			eff_type   INTEGER,
			obj_type   INTEGER,
			dummies    TEXT,
			dummy2     INTEGER,
			height_off INTEGER,
			play_time  REAL,
			light_id   INTEGER,
			base_size  REAL
		)
	)";

	// Явный список колонок: в legacy-БД может присутствовать колонка `name`,
	// которая больше не читается (поле Name снесено как дубль DataName).
	static constexpr const char* SELECT_ALL_SQL =
		"SELECT id, data_name, photo_name, eff_type, obj_type, dummies, dummy2, "
		"height_off, play_time, light_id, base_size FROM effects ORDER BY id";

	bool Load(SqliteDatabase& db, int(*getTexId)(const char*) = nullptr) {
		EnsureCreated(db, TABLE_NAME, CREATE_TABLE_SQL);
		if (!GameRecordset::Load(db, SELECT_ALL_SQL)) return false;

		if (getTexId) {
			ForEach([&](CEffectRecord& info) {
				const auto photoPath = std::format("texture/photo/{}.bmp", info.PhotoName);
				info.PhotoTexId = getTexId(photoPath.c_str());
			});
		}
		return true;
	}

protected:
	RecordEntry ReadRecord(SqliteStatement& stmt) override;
};

CEffectRecord* GetEffectInfo(int nTypeID, const std::source_location& loc = std::source_location::current());

} // namespace Corsairs::Common::Effect
