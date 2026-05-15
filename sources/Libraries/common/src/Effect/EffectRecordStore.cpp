#include "Effect/EffectRecordStore.h"
#include <sstream>


namespace Corsairs::Common::Effect {

GameRecordset<CEffectRecord>::RecordEntry EffectRecordStore::ReadRecord(SqliteStatement& stmt) {
	CEffectRecord record{};
	int col = 0;

	record.Id        = stmt.GetInt(col++);
	record.DataName  = stmt.GetText(col++);
	record.PhotoName = stmt.GetText(col++);

	record.Kind      = static_cast<EffectKind>(stmt.GetInt(col++));
	record.AttachId  = stmt.GetInt(col++);

	// dummies — "d0,d1,..."
	{
		std::string text(stmt.GetText(col++));
		std::istringstream ss(text);
		std::string token;
		record.DummyCount = 0;
		for (int i = 0; i < 8; i++) {
			record.Dummies[i] = -1;
			if (std::getline(ss, token, ',')) {
				record.Dummies[i] = std::stoi(token);
				record.DummyCount++;
			}
		}
	}

	record.Dummy2    = stmt.GetInt(col++);
	record.HeightOff = stmt.GetInt(col++);
	record.PlayTime  = static_cast<float>(stmt.GetDouble(col++));
	record.LightId   = stmt.GetInt(col++);
	record.BaseSize  = static_cast<float>(stmt.GetDouble(col++));

	return {record.Id, record.DataName, std::move(record)};
}

CEffectRecord* GetEffectInfo(int nTypeID, const std::source_location& loc) {
	return EffectRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Effect
