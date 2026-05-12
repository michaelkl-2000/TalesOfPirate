#pragma once

#include "Database/TableData.h"


//  ID

namespace Corsairs::Common::World {

class CAreaInfo : public EntityData
{
public:

	CAreaInfo()
	{
        dwColor = 0;
    }

    DWORD dwColor;
    int   nMusic;
    DWORD dwEnvColor;
    DWORD dwLightColor;
    float fLightDir[3];
	char  chType;			// 0-,1-,
};

} // namespace Corsairs::Common::World

// GetAreaInfo() определена в AreaRecordStore.h (тот же namespace).
// Подключается за пределами блока, иначе вложение namespace ломает lookup.
#include "World/AreaRecordStore.h"

