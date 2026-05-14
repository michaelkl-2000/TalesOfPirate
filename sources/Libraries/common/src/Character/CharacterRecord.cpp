#include "Character/CharacterRecord.h"
#include "Character/ChaRecordStore.h"


namespace Corsairs::Common::Character {

void ChaRecord::RefreshPrivateData()
{
	_HaveEffectFog = false;
	for (std::size_t i = 0; i < kChaHpEffectNum; i++)
	{
		if (_nHPEffect.at(i) != 0)
		{
			_HaveEffectFog = true;
			break;
		}
	}
}

ChaRecord* GetChaRecordInfo(int nTypeID, const std::source_location& loc) {
	return ChaRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Character

