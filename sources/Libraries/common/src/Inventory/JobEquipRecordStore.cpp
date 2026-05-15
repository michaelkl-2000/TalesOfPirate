#include "Inventory/JobEquipRecordStore.h"
#include <sstream>

// ============================================================================
// Сериализация массива sItemID в строку "id0,id1,id2,..."
// ============================================================================


namespace Corsairs::Common::Inventory {

static std::string SerializeItems(const JobEquipRecord& record) {
	std::string result;
	for (std::size_t i = 0; i < record.ItemIds.size(); ++i) {
		if (i > 0) {
			result += ',';
		}
		result += std::to_string(record.ItemIds[i]);
	}
	return result;
}

// ============================================================================
// ReadRecord — конструирует JobEquipRecord из строки SQLite-запроса
// ============================================================================

GameRecordset<JobEquipRecord>::RecordEntry JobEquipRecordStore::ReadRecord(SqliteStatement& stmt) {
	JobEquipRecord record{};
	int col = 0;

	record.Id  = stmt.GetInt(col++);
	record.Job = static_cast<std::int8_t>(stmt.GetInt(col++));

	// items — "id0,id1,id2,id3,id4,id5"
	{
		std::string text(stmt.GetText(col++));
		std::istringstream ss(text);
		std::string token;
		for (std::size_t i = 0; i < record.ItemIds.size(); ++i) {
			record.ItemIds[i] = 0;
			if (std::getline(ss, token, ',')) {
				record.ItemIds[i] = static_cast<std::int16_t>(std::stoi(token));
			}
		}
	}

	return {record.Id, {}, std::move(record)};
}

// ============================================================================
// Insert — временный метод миграции из CJobEquipRecordSet
// ============================================================================

void JobEquipRecordStore::Insert(SqliteDatabase& db, const JobEquipRecord& record) {
	try {
		EnsureCreated(db, TABLE_NAME, CREATE_TABLE_SQL);
		auto stmt = db.Prepare(
			"INSERT OR REPLACE INTO job_equip (id, job, items) VALUES (?, ?, ?)");
		stmt.Bind(1, record.Id);
		stmt.Bind(2, static_cast<int>(record.Job));
		stmt.Bind(3, SerializeItems(record));
		stmt.Step();
	} catch (const std::exception& e) {
		ToLogService("errors", LogLevel::Error, "JobEquipRecordStore::Insert(id={}) failed: {}", record.Id, e.what());
	}
}

JobEquipRecord* GetJobEquipRecordInfo(int nTypeID, const std::source_location& loc) {
	return JobEquipRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Inventory

