#include "Character/PoseRecordStore.h"

#include <charconv>
#include <climits>
#include <cstdint>
#include <string_view>


namespace Corsairs::Common::Character {

std::int16_t PoseInfo::GetRealPoseId(WieldMode mode, const std::source_location& loc) const {
	return GetRealPoseId(static_cast<std::size_t>(mode), loc);
}

std::int16_t PoseInfo::GetRealPoseId(std::size_t variant, const std::source_location& loc) const {
	if (variant >= kPoseVariantCount) {
		ToLogService("errors", LogLevel::Warning,
			"CPoseInfo::GetRealPoseId(id={}) variant={} out of range [0,{}) at {}:{}",
			Id, variant, kPoseVariantCount, loc.file_name(), loc.line());
		return 0;
	}
	return RealPoseId[variant];
}

GameRecordset<PoseInfo>::RecordEntry PoseRecordStore::ReadRecord(SqliteStatement& stmt) {
	PoseInfo record{};
	int col = 0;

	record.Id       = stmt.GetInt(col++);
	record.DataName = stmt.GetText(col++);

	// pose_ids — "p0,p1,p2,p3,p4,p5,p6"
	const std::string_view text = stmt.GetText(col++);
	std::size_t pos = 0;
	std::size_t parsed = 0;
	while (pos <= text.size() && parsed < kPoseVariantCount) {
		const std::size_t commaPos = text.find(',', pos);
		const std::size_t end = (commaPos == std::string_view::npos) ? text.size() : commaPos;
		const std::string_view tok = text.substr(pos, end - pos);

		int value = 0;
		const auto* first = tok.data();
		const auto* last  = tok.data() + tok.size();
		auto [ptr, ec] = std::from_chars(first, last, value);
		if (ec != std::errc{} || ptr != last) {
			ToLogService("errors", LogLevel::Warning,
				"poses.id={} pose_ids[{}] token='{}' is not a valid integer; using 0",
				record.Id, parsed, tok);
			value = 0;
		} else if (value < INT16_MIN || value > INT16_MAX) {
			ToLogService("errors", LogLevel::Warning,
				"poses.id={} pose_ids[{}]={} out of int16 range; clamping",
				record.Id, parsed, value);
			value = (value < INT16_MIN) ? INT16_MIN : INT16_MAX;
		}
		record.RealPoseId[parsed++] = static_cast<std::int16_t>(value);

		if (commaPos == std::string_view::npos) {
			break;
		}
		pos = commaPos + 1;
	}

	if (parsed != kPoseVariantCount) {
		ToLogService("errors", LogLevel::Warning,
			"poses.id={} pose_ids has {} tokens, expected {}; missing filled with 0",
			record.Id, parsed, kPoseVariantCount);
	}

	std::string name(record.DataName);
	return {record.Id, std::move(name), std::move(record)};
}

PoseInfo* GetPoseInfo(short sPoseID, const std::source_location& loc) {
	return PoseRecordStore::Instance()->Get(sPoseID, loc);
}

} // namespace Corsairs::Common::Character
