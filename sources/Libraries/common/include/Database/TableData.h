#pragma once

// EntityData — общая DTO-база для всех Record-типов (CNpcRecord,
// CMonRefRecord, CSwitchMapRecord, CItemRecord, ...).
// После сноса CRawDataSet остались только поля, читаемые потребителями:
//   nID       — числовой ID записи
//   DataName  — строковое имя записи (std::string)

#include <string>


namespace Corsairs::Common::Database {

class EntityData {
public:
	int Id{0};
	std::string DataName{};
};

} // namespace Corsairs::Common::Database

// Foundational тип — пробрасываем в Corsairs::Common, чтобы все sub-namespace'ы
// (Item/Skill/Character/...) находили его через parent-namespace lookup без
// массовой правки сигнатур (`class CItemRecord : public EntityData` и т.п.).
namespace Corsairs::Common {
	using Database::EntityData;
}

