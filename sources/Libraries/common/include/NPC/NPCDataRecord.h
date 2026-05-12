#pragma once
#include "Database/TableData.h"

#define NPC_MAXSIZE_NAME 128


namespace Corsairs::Common::NPC {

class NPCData : public EntityData {
public:
	char szName[NPC_MAXSIZE_NAME]{};
	char szArea[NPC_MAXSIZE_NAME]{};
	DWORD dwxPos0{0};
	DWORD dwyPos0{0};
	char szMapName[NPC_MAXSIZE_NAME]{};
};

} // namespace Corsairs::Common::NPC

