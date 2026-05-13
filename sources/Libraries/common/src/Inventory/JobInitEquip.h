//=============================================================================
// FileName: JobInitEquip.h
// Creater: ZhangXuedong
// Date: 2005.01.08
// Comment:
//=============================================================================

#ifndef JOBINITEQUIP_H
#define JOBINITEQUIP_H

#include <tchar.h>
#include "util.h"
#include "Database/TableData.h"

#define defJOB_INIT_EQUIP_MAX	6


namespace Corsairs::Common::Inventory {

class CJobEquipRecord : public EntityData
{
public:
	char	chID;			//
	char	chJob;			//
	short	sItemID[defJOB_INIT_EQUIP_MAX];		//
};


} // namespace Corsairs::Common::Inventory

#endif // JOBINITEQUIP_H
