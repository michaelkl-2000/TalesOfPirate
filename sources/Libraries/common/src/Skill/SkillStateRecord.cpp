#include "Skill/SkillStateRecord.h"
#include "Skill/SkillStateRecordStore.h"


namespace Corsairs::Common::Skill {

void CSkillStateRecord::RefreshPrivateData() {
	_nActNum = 0;
	for (int i = 0; i < defSKILLSTATE_ACT_NUM; i++) {
		if (nActBehave[i] != 0) {
			_nActNum++;
		}
		else {
			break;
		}
	}
}

CSkillStateRecord* GetCSkillStateRecordInfo(int nTypeID, const std::source_location& loc) {
	return SkillStateRecordStore::Instance()->Get(nTypeID, loc);
}

} // namespace Corsairs::Common::Skill

