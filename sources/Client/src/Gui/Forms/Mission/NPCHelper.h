#pragma once

#include "NPC/NPCDataRecord.h"
#include "NPC/MonsterListRecordStore.h"
#include "NPC/NPCListRecordStore.h"

enum class NPCHelperType { MonsterList, NPCList };

inline Corsairs::Common::NPC::NPCData* GetNPCDataInfo(int nTypeID, NPCHelperType type) {
	if (type == NPCHelperType::MonsterList)
		return Corsairs::Common::NPC::MonsterListRecordStore::Instance()->Get(nTypeID);
	else
		return Corsairs::Common::NPC::NPCListRecordStore::Instance()->Get(nTypeID);
}

inline int GetNPCMaxId(NPCHelperType type) {
	if (type == NPCHelperType::MonsterList)
		return Corsairs::Common::NPC::MonsterListRecordStore::Instance()->GetMaxId();
	else
		return Corsairs::Common::NPC::NPCListRecordStore::Instance()->GetMaxId();
}
