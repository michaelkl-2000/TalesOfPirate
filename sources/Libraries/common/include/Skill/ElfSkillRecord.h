#pragma once

#include "Database/TableData.h"

// Запись таблицы навыков эльфов (ElfSkillInfo)

namespace Corsairs::Common::Skill {

class CElfSkillInfo : public EntityData {
public:
	int nIndex{};
	int nTypeID{};
};

} // namespace Corsairs::Common::Skill

