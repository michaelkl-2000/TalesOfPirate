#pragma once

#include "Database/TableData.h"

// Запись таблицы карт

namespace Corsairs::Common::World {

class CMapInfo : public EntityData
{
public:
	char  szName[16]{};
	int   nInitX{0};
	int   nInitY{0};
	float fLightDir[3]{1.0f, 1.0f, -1.0f};
	BYTE  btLightColor[3]{255, 255, 255};
	bool  IsShowSwitch{true};
};

} // namespace Corsairs::Common::World

