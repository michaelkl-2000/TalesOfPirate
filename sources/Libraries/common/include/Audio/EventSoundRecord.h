#pragma once

#include "Database/TableData.h"

// Запись таблицы звуков событий

namespace Corsairs::Common::Audio {

class CEventSoundInfo : public EntityData
{
public:
	int nSoundID{-1};
};

} // namespace Corsairs::Common::Audio

