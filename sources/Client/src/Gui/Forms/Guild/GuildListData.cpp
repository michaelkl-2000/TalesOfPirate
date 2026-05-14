//------------------------------------------------------------------------
//	2005.4.25	Arcol	create this file
//------------------------------------------------------------------------

#include "stdafx.h"
#include ".\guildlistdata.h"

CGuildListData::CGuildListData(void) {
	m_dwID = 0;
	m_strName = "";
	m_strMottoName = "";
	m_strMasterName = "";
	m_dwMembers = 0;
	m_i64Exp = 0;
}

CGuildListData::CGuildListData(DWORD dwID, std::string_view strName, std::string_view strMottoName,
							   std::string_view strMasterName,
							   DWORD dwMemberCount, __int64 i64Exp) {
	SetGuildID(dwID);
	SetGuildName(strName);
	SetGuildMottoName(strMottoName);
	SetGuildMasterName(strMasterName);
	SetExperence(i64Exp);
	SetMembers(dwMemberCount);
}

CGuildListData::~CGuildListData(void) {
}
