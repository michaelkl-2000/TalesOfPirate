#pragma once

#include "Database/TableData.h"

// Запись таблицы ресурсов (ResourceInfo)

namespace Corsairs::Common::Misc {

class CResourceInfo : public EntityData {
public:
	enum {
		RT_PAR     = 0,
		RT_PATH    = 1,
		RT_EFF     = 2,
		RT_MESH    = 3,
		RT_TEXTURE = 4,
		RT_UNKNOWN = -1,
	};

	int m_iType{RT_UNKNOWN};

	int GetType() const { return m_iType; }
};

} // namespace Corsairs::Common::Misc

