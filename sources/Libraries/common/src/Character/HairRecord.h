//======================================================================================================================
// FileName: HairRecord.h
// Creater: Jerry li
// Date: 2005.08.29
// Comment: CHairRecordSet class
//======================================================================================================================

#ifndef	HAIRRECORD_H
#define	HAIRRECORD_H

#include <tchar.h>
#include <string>
#include <source_location>
#include "util.h"
#include "Database/TableData.h"

#define defHAIR_MAX_ITEM		4
#define defHAIR_MAX_FAIL_ITEM	3


namespace Corsairs::Common::Character {

class CHairRecord : public EntityData
{
public:
	CHairRecord();
	std::string	szColor;

	DWORD	dwNeedItem[defHAIR_MAX_ITEM][2];		// ID,
	DWORD	dwMoney;								// 
	DWORD	dwItemID;								// ()ID
	DWORD	dwFailItemID[defHAIR_MAX_FAIL_ITEM];	// 
	bool	IsChaUse[4];							// 4
	
	int		GetFailItemNum()		{ return _nFailNum;		}
	void	RefreshPrivateData();

private:
	int		_nFailNum;
	
};

CHairRecord* GetHairRecordInfo(int nTypeID, const std::source_location& loc = std::source_location::current());


} // namespace Corsairs::Common::Character

#endif //HAIRRECORD_H
