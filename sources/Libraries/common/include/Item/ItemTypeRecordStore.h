#pragma once
#include "Database/GameRecordset.h"
#include "Item/ItemTypeRecord.h"


namespace Corsairs::Common::Item {

class ItemTypeRecordStore : public GameRecordset<CItemTypeInfo> {
public:
	static ItemTypeRecordStore* Instance() { static ItemTypeRecordStore i{}; return &i; }
	static constexpr const char* TABLE_NAME = "item_types";
	static constexpr const char* CREATE_TABLE_SQL = R"(CREATE TABLE IF NOT EXISTS item_types (id INTEGER PRIMARY KEY, name TEXT))";
	static constexpr const char* SELECT_ALL_SQL = "SELECT * FROM item_types ORDER BY id";
	bool Load(SqliteDatabase& db) { EnsureCreated(db, TABLE_NAME, CREATE_TABLE_SQL); return GameRecordset::Load(db, SELECT_ALL_SQL); }
	static void Insert(SqliteDatabase& db, const CItemTypeInfo& r);
protected:
	RecordEntry ReadRecord(SqliteStatement& stmt) override;
};

CItemTypeInfo* GetItemTypeInfo(int nTypeID, const std::source_location& loc = std::source_location::current());

} // namespace Corsairs::Common::Item

