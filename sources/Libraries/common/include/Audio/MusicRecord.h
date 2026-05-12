#pragma once

#include "Database/TableData.h"

// Запись таблицы музыки/звуков

namespace Corsairs::Common::Audio {

class CMusicInfo : public EntityData
{
public:
	int nType{0};
};

} // namespace Corsairs::Common::Audio

