#pragma once

#include "Database/TableData.h"
#define NOMINMAX
#include <windows.h>

#define MONSTER_LIST_MAX 8

// Запись таблицы зон монстров (MonsterInfo)

namespace Corsairs::Common::NPC {

class CMonsterInfo : public EntityData {
public:
	char  szName[32]{};
	POINT ptStart{};
	POINT ptEnd{};
	int   nMonsterList[MONSTER_LIST_MAX]{};
	char  szArea[32]{};
};

} // namespace Corsairs::Common::NPC

