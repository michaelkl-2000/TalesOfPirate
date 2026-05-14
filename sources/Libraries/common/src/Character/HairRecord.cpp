#include "Character/HairRecord.h"
#include "Character/HairRecordStore.h"


namespace Corsairs::Common::Character {

HairRecord* GetHairRecordInfo(int nTypeID, const std::source_location& loc) {
	return HairRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Character
