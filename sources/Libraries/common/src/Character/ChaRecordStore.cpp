#include "Character/ChaRecordStore.h"
#include <sstream>

// ============================================================================
// Вспомогательные функции парсинга
// ============================================================================


namespace Corsairs::Common::Character {

void ChaRecordStore::ParseShortArray(std::string_view text, std::int16_t* out, int maxLen, std::int16_t defaultVal) {
	std::fill(out, out + maxLen, defaultVal);
	if (text.empty() || text == "0") return;

	std::string s(text);
	std::istringstream ss(s);
	std::string token;
	int i = 0;
	while (std::getline(ss, token, ',') && i < maxLen) {
		out[i++] = static_cast<std::int16_t>(std::atoi(token.c_str()));
	}
}

void ChaRecordStore::ParseLongArray(std::string_view text, std::int32_t* out, int maxLen, std::int32_t defaultVal) {
	std::fill(out, out + maxLen, defaultVal);
	if (text.empty() || text == "0") return;

	std::string s(text);
	std::istringstream ss(s);
	std::string token;
	int i = 0;
	while (std::getline(ss, token, ',') && i < maxLen) {
		out[i++] = static_cast<std::int32_t>(std::atol(token.c_str()));
	}
}

void ChaRecordStore::ParseCharArray(std::string_view text, std::int8_t* out, int maxLen) {
	std::fill(out, out + maxLen, std::int8_t{0});
	if (text.empty() || text == "0") return;

	std::string s(text);
	std::istringstream ss(s);
	std::string token;
	int i = 0;
	while (std::getline(ss, token, ',') && i < maxLen) {
		out[i++] = static_cast<std::int8_t>(std::atoi(token.c_str()));
	}
}

void ChaRecordStore::ParseIntArray(std::string_view text, std::int32_t* out, int maxLen) {
	std::fill(out, out + maxLen, 0);
	if (text.empty() || text == "0") return;

	std::string s(text);
	std::istringstream ss(s);
	std::string token;
	int i = 0;
	while (std::getline(ss, token, ',') && i < maxLen) {
		out[i++] = std::atoi(token.c_str());
	}
}

void ChaRecordStore::ParseFloatArray(std::string_view text, float* out, int maxLen) {
	std::fill(out, out + maxLen, 0.0f);
	if (text.empty() || text == "0") return;

	std::string s(text);
	std::istringstream ss(s);
	std::string token;
	int i = 0;
	while (std::getline(ss, token, ',') && i < maxLen) {
		out[i++] = static_cast<float>(std::atof(token.c_str()));
	}
}

// ============================================================================
// ReadRecord — конструирует ChaRecord из строки SQLite-запроса
// ============================================================================

GameRecordset<ChaRecord>::RecordEntry ChaRecordStore::ReadRecord(SqliteStatement& stmt) {
	ChaRecord record{};
	int col = 0;

	// id, name — поля EntityData::Id / EntityData::DataName
	record.Id = stmt.GetInt(col++);
	record.DataName = stmt.GetText(col++);

	// icon_name
	record.IconName = stmt.GetText(col++);

	// modal_type, ctrl_type
	record.ModalType = static_cast<EChaModalType>(stmt.GetInt(col++));
	record.CtrlType  = static_cast<EChaCtrlType>(stmt.GetInt(col++));

	// model, suit_id, suit_num
	record.Model   = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.SuitID  = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.SuitNum = static_cast<std::int16_t>(stmt.GetInt(col++));

	// skin_info[8] — TEXT
	record.SkinInfo.fill(static_cast<std::int16_t>(kChaRecordKeyValue));
	ParseShortArray(stmt.GetText(col++), record.SkinInfo.data(), kChaSkinNum, static_cast<std::int16_t>(kChaRecordKeyValue));

	// feff_id[4] — TEXT
	record.FeffID.fill(0);
	ParseShortArray(stmt.GetText(col++), record.FeffID.data(), kChaFeffNum);

	// eeff_id
	record.EeffID = static_cast<std::int16_t>(stmt.GetInt(col++));

	// effect_action_id[3] — TEXT
	record.EffectActionID.fill(0);
	ParseShortArray(stmt.GetText(col++), record.EffectActionID.data(), kChaEffectActionNum);

	// shadow, action_id, diaphaneity
	record.Shadow       = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.ActionId     = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.Diaphaneity  = static_cast<std::int8_t>(stmt.GetInt(col++));

	// footfall, whoop, dirge
	record.Footfall = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.Whoop    = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.Dirge    = static_cast<std::int16_t>(stmt.GetInt(col++));

	// control_able, territory, sea_height
	record.ControlAble = static_cast<std::int8_t>(stmt.GetInt(col++));
	record.Territory   = static_cast<std::int8_t>(stmt.GetInt(col++));
	record.SeaHeight   = static_cast<std::int16_t>(stmt.GetInt(col++));

	// item_type[20] — TEXT
	record.ItemType.fill(static_cast<std::int16_t>(kChaRecordKeyValue));
	ParseShortArray(stmt.GetText(col++), record.ItemType.data(), kChaItemKindNum, static_cast<std::int16_t>(kChaRecordKeyValue));

	// lengh, width, height, radii
	record.Length = static_cast<float>(stmt.GetDouble(col++));
	record.Width  = static_cast<float>(stmt.GetDouble(col++));
	record.Height = static_cast<float>(stmt.GetDouble(col++));
	record.Radii  = static_cast<std::int16_t>(stmt.GetInt(col++));

	// birth_behave[3] — TEXT
	record.BirthBehave.fill(0);
	ParseCharArray(stmt.GetText(col++), record.BirthBehave.data(), kChaBirthEffectNum);

	// died_behave[3] — TEXT
	record.DiedBehave.fill(0);
	ParseCharArray(stmt.GetText(col++), record.DiedBehave.data(), kChaDieEffectNum);

	// born_eff, die_eff, dormancy, die_action
	record.BornEff   = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.DieEff    = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.Dormancy  = static_cast<std::int16_t>(stmt.GetInt(col++));
	record.DieAction = static_cast<std::int8_t>(stmt.GetInt(col++));

	// hp_effect[3] — TEXT
	record.HpEffect.fill(0);
	ParseIntArray(stmt.GetText(col++), record.HpEffect.data(), kChaHpEffectNum);

	// is_face, is_cyclone
	record._IsFace    = stmt.GetInt(col++) != 0;
	record._IsCyclone = stmt.GetInt(col++) != 0;

	// script, weapon
	record.Script = stmt.GetInt(col++);
	record.Weapon = stmt.GetInt(col++);

	// skill_ids[11] — TEXT, skill_lvs[11] — TEXT
	for (auto& row : record.Skill) row.fill(kChaRecordKeyValue);
	{
		std::int32_t ids[kChaInitSkillNum]{};
		ParseLongArray(stmt.GetText(col++), ids, kChaInitSkillNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitSkillNum; i++)
			record.Skill.at(i).at(0) = ids[i];

		std::int32_t lvs[kChaInitSkillNum]{};
		ParseLongArray(stmt.GetText(col++), lvs, kChaInitSkillNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitSkillNum; i++)
			record.Skill.at(i).at(1) = lvs[i];
	}

	// item_ids[15] — TEXT, item_counts[15] — TEXT
	for (auto& row : record.Item) row.fill(kChaRecordKeyValue);
	{
		std::int32_t ids[kChaInitItemNum]{};
		ParseLongArray(stmt.GetText(col++), ids, kChaInitItemNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitItemNum; i++)
			record.Item.at(i).at(0) = ids[i];

		std::int32_t counts[kChaInitItemNum]{};
		ParseLongArray(stmt.GetText(col++), counts, kChaInitItemNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitItemNum; i++)
			record.Item.at(i).at(1) = counts[i];
	}

	// max_show_item, all_show, prefix
	record.MaxShowItem = stmt.GetInt(col++);
	record.AllShow     = static_cast<float>(stmt.GetDouble(col++));
	record.Prefix      = stmt.GetInt(col++);

	// task_item_ids[15] — TEXT, task_item_counts[15] — TEXT
	for (auto& row : record.TaskItem) row.fill(kChaRecordKeyValue);
	{
		std::int32_t ids[kChaInitItemNum]{};
		ParseLongArray(stmt.GetText(col++), ids, kChaInitItemNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitItemNum; i++)
			record.TaskItem.at(i).at(0) = ids[i];

		std::int32_t counts[kChaInitItemNum]{};
		ParseLongArray(stmt.GetText(col++), counts, kChaInitItemNum, static_cast<std::int32_t>(kChaRecordKeyValue));
		for (std::size_t i = 0; i < kChaInitItemNum; i++)
			record.TaskItem.at(i).at(1) = counts[i];
	}

	// ai_no, can_turn, vision, noise, get_exp, light
	record.AiNo    = static_cast<EChaAiType>(stmt.GetInt(col++));
	record.CanTurn = static_cast<std::int8_t>(stmt.GetInt(col++));
	record.Vision  = stmt.GetInt(col++);
	record.Noise   = stmt.GetInt(col++);
	record.GetExp  = stmt.GetInt(col++);
	record.Light   = stmt.GetInt(col++) != 0;

	// mobexp, lv, mx_hp, hp, mx_sp, sp
	record.MobExp = stmt.GetInt(col++);
	record.Lv     = stmt.GetInt(col++);
	record.MxHp   = stmt.GetInt(col++);
	record.Hp     = stmt.GetInt(col++);
	record.MxSp   = stmt.GetInt(col++);
	record.Sp     = stmt.GetInt(col++);

	// mn_atk, mx_atk, p_def, def, hit, flee
	record.MnAtk = stmt.GetInt(col++);
	record.MxAtk = stmt.GetInt(col++);
	record.PDef  = stmt.GetInt(col++);
	record.Def   = stmt.GetInt(col++);
	record.Hit   = stmt.GetInt(col++);
	record.Flee  = stmt.GetInt(col++);

	// crt, mf, h_rec, s_rec, a_spd, a_dis, c_dis, m_spd, col
	record.Crt  = stmt.GetInt(col++);
	record.Mf   = stmt.GetInt(col++);
	record.HRec = stmt.GetInt(col++);
	record.SRec = stmt.GetInt(col++);
	record.ASpd = stmt.GetInt(col++);
	record.ADis = stmt.GetInt(col++);
	record.CDis = stmt.GetInt(col++);
	record.MSpd = stmt.GetInt(col++);
	record.Col  = stmt.GetInt(col++);

	// str, agi, dex, con, sta, luk, l_hand_val
	record.Str      = stmt.GetInt(col++);
	record.Agi      = stmt.GetInt(col++);
	record.Dex      = stmt.GetInt(col++);
	record.Con      = stmt.GetInt(col++);
	record.Sta      = stmt.GetInt(col++);
	record.Luk      = stmt.GetInt(col++);
	record.LHandVal = stmt.GetInt(col++);

	// guild, title, job
	record.Guild = stmt.GetText(col++);
	record.Title = stmt.GetText(col++);
	record.Job   = stmt.GetText(col++);

	// c_exp, n_exp, fame, ap, tp, gd, spri, stor
	record.CExp = stmt.GetInt(col++);
	record.NExp = stmt.GetInt(col++);
	record.Fame = stmt.GetInt(col++);
	record.Ap   = stmt.GetInt(col++);
	record.Tp   = stmt.GetInt(col++);
	record.Gd   = stmt.GetInt(col++);
	record.Spri = stmt.GetInt(col++);
	record.Stor = stmt.GetInt(col++);

	// mx_sail, sail, stasa, scsm
	record.MxSail = stmt.GetInt(col++);
	record.Sail   = stmt.GetInt(col++);
	record.Stasa  = stmt.GetInt(col++);
	record.Scsm   = stmt.GetInt(col++);

	// t_str .. t_scsm (21 бонусных характеристик)
	record.TStr  = stmt.GetInt(col++);
	record.TAgi  = stmt.GetInt(col++);
	record.TDex  = stmt.GetInt(col++);
	record.TCon  = stmt.GetInt(col++);
	record.TSta  = stmt.GetInt(col++);
	record.TLuk  = stmt.GetInt(col++);
	record.TMxHp = stmt.GetInt(col++);
	record.TMxSp = stmt.GetInt(col++);
	record.TAtk  = stmt.GetInt(col++);
	record.TDef  = stmt.GetInt(col++);
	record.THit  = stmt.GetInt(col++);
	record.TFlee = stmt.GetInt(col++);
	record.TMf   = stmt.GetInt(col++);
	record.TCrt  = stmt.GetInt(col++);
	record.THRec = stmt.GetInt(col++);
	record.TSRec = stmt.GetInt(col++);
	record.TASpd = stmt.GetInt(col++);
	record.TADis = stmt.GetInt(col++);
	record.TSpd  = stmt.GetInt(col++);
	record.TSpri = stmt.GetInt(col++);
	record.TScsm = stmt.GetInt(col++);

	// scaling[3] — TEXT
	record.Scaling.fill(0.0f);
	ParseFloatArray(stmt.GetText(col++), record.Scaling.data(), kChaScalingNum);

	record.RefreshPrivateData();

	return {record.Id, record.DataName, std::move(record)};
}

} // namespace Corsairs::Common::Character
