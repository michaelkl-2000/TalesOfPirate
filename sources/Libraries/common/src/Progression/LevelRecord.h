#pragma once

#include <tchar.h>
#include "util.h"
#include "Database/TableData.h"


namespace Corsairs::Common::Progression {

class CLevelRecord : public EntityData {
public:
	long lID; //
	short sLevel; //
	unsigned int ulExp; //
};

} // namespace Corsairs::Common::Progression

