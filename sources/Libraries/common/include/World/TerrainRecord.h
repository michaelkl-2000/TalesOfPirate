#pragma once
#include "Database/TableData.h"

#define TERRAIN_TYPE_NORMAL     0
#define TERRAIN_TYPE_UNDERWATER 1
#define MAX_WATER_LOOP          30


namespace Corsairs::Common::World {

class MPTerrainInfo : public EntityData {
public:
	BYTE btType{TERRAIN_TYPE_NORMAL};
	int  nTextureID{0};
	BYTE btAttr{0};
};

} // namespace Corsairs::Common::World

