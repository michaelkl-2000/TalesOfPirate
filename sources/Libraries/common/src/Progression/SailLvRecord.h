#pragma once

#include <tchar.h>
#include "util.h"
#include "Database/TableData.h"


namespace Corsairs::Common::Progression {

class CSailLvRecord : public EntityData
{
public:
	long	lID;			//
	short	sLevel;			//
	unsigned long	ulExp;	//
};

} // namespace Corsairs::Common::Progression

