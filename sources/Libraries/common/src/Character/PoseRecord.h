#pragma once

#include "Database/TableData.h"

// Запись таблицы поз персонажа

namespace Corsairs::Common::Character {

class CPoseInfo : public EntityData
{
public:
	short sRealPoseID[7]{};
};

} // namespace Corsairs::Common::Character

