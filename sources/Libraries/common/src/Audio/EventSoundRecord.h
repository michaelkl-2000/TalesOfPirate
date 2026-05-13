#pragma once

#include "Database/TableData.h"

// Запись таблицы звуков событий

namespace Corsairs::Common::Audio {

class CEventSoundInfo : public EntityData
{
public:
	int SoundID{-1};
};

} // namespace Corsairs::Common::Audio

