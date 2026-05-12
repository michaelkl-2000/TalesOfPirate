#pragma once
#include "Database/TableData.h"


namespace Corsairs::Common::Localization {

class CNotifyInfo : public EntityData {
public:
	char chType{0};
	char szInfo[64]{};
};

} // namespace Corsairs::Common::Localization

