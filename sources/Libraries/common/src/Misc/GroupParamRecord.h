#pragma once
#include "Database/TableData.h"


namespace Corsairs::Common::Misc {

class Group_Param : public EntityData {
public:
	char szName[32]{};
	int  nTypeNum{0};
	int  nTypeID[8]{-1,-1,-1,-1,-1,-1,-1,-1};
	int  nNum[8]{};
	int  nTotalNum{0};
	int  nRenderIdx{-1};
};

} // namespace Corsairs::Common::Misc

