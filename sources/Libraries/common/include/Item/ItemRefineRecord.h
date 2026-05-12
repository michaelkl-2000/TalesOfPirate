#pragma once
#include "Database/TableData.h"

#define ITEM_REFINE_NUM 14


namespace Corsairs::Common::Item {

class CItemRefineInfo : public EntityData {
public:
	short Value[ITEM_REFINE_NUM]{};
	float fChaEffectScale[4]{};
};

} // namespace Corsairs::Common::Item

