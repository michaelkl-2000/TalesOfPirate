#include "Character/CharacterRecord.h"
#include "Character/ChaRecordStore.h"


namespace Corsairs::Common::Character {

void ChaRecord::RefreshPrivateData()
{
	_haveEffectFog = false;
	for (std::size_t i = 0; i < kChaHpEffectNum; i++)
	{
		if (HpEffect.at(i) != 0)
		{
			_haveEffectFog = true;
			break;
		}
	}
}

ChaRecord* GetChaRecordInfo(int nTypeID, const std::source_location& loc) {
	return ChaRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Character

