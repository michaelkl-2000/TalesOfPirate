#pragma once

#include <string>

// Запись таблицы cha_name_filters — запрещённые подстроки в именах персонажей.
// Каждый `_pattern` проверяется `CTextFilter::IsLegalText(NAME_TABLE, ...)`.

namespace Corsairs::Common::Character {

struct ChaNameFilterRecord {
	int         _id{0};       // rowid, для стабильного порядка
	std::string _pattern{};   // запрещённая подстрока
};

} // namespace Corsairs::Common::Character

