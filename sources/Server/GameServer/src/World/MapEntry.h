//=============================================================================
// FileName: MapEntry.h
// Creater: ZhangXuedong
// Date: 2005.10.21
// Comment: Map Entry class
//=============================================================================

#ifndef MAPENTRY_H
#define MAPENTRY_H

#include <list>

#include "Entity/Entity.h"
#include "SlotMap.h"
#include "World/MapRes.h"
#include "World/EventRecord.h"

class	CMapEntryCopyCell
{
public:
	CMapEntryCopyCell(int16_t sMaxPlyNum = 0, int16_t sCurPlyNum = 0);

	void		SetMaxPlyNum(int16_t sPlyNum);
	int16_t	GetMaxPlyNum(void);
	void		SetCurPlyNum(int16_t sPlyNum);
	int16_t	GetCurPlyNum(void);
	bool		AddCurPlyNum(int16_t sAddNum);
	bool		HasFreePlyCount(int16_t sRequestNum);

	std::int32_t	GetParam(char chParamID);
	bool		SetParam(char chParamID, std::int32_t lParamVal);

	void		WriteParamPacket(Corsairs::Net::WPacket &pk);

	void		SetPosID(std::int32_t lPosID);
	std::int32_t	GetPosID(void);

private:
	int16_t	m_sMaxPlyNum;
	int16_t	m_sCurPlyNum;

	std::int32_t	m_lParam[defMAPCOPY_INFO_PARAM_NUM];

	int16_t	m_sPosID;

};

//
class	CDynMapEntryCell
{
public:
	CDynMapEntryCell();

	void		    SetMapName(const char *cszMapName);
	const char*	    GetMapName(void) const;
	void		    SetTMapName(const char *cszTMapName);
	const char*	    GetTMapName(void) const;
	const Corsairs::Util::Point*    GetEntiPos(void) const;
	void            SetEntiPos(const Corsairs::Util::Point* cpSPos);
	void            SetEntiPos(std::int32_t lPosX, std::int32_t lPosY);
	void		    SetEntiID(std::int32_t lEntiID);
	std::int32_t	    GetEntiID(void);
	void		    SetEventID(std::int32_t lEventID);
	std::int32_t	    GetEventID(void);
	void		    SetEventName(const char *cszEventName);
	void		    SetEnti(Entity *pCEnt);
	void		    GetPosInfo(const char** pMapN, std::int32_t* lpPosX, std::int32_t* lpPosY, const char** pTMapN);
	CEvent*		    GetEvent(void);
	void		    SetCopyNum(int16_t sCopyNum);
	int16_t	    GetCopyNum(void);
	void		    SetCopyPlyNum(int16_t sCopyNum);
	int16_t	    GetCopyPlyNum(void);
	CMapEntryCopyCell* AddCopy(CMapEntryCopyCell *pCCpyCell);
	CMapEntryCopyCell* GetCopy(int16_t sCopyID);
	bool		    ReleaseCopy(CMapEntryCopyCell *pCCpyCell);
	bool		    ReleaseCopy(std::int32_t lCopyNO);
	void		    FreeEnti(void);

	void		    SynCopyParam(int16_t sCopyID);
	void		    SynCopyRun(int16_t sCopyID, char chCdtType, std::int32_t chCdtVal);

private:
	char	m_szMapName[MAX_MAPNAME_LENGTH];
	Corsairs::Util::Point		m_SEntiPos;
	std::int32_t	m_lEntiID;
	char	m_szTMapName[MAX_MAPNAME_LENGTH];
	CEvent		m_CEvtObj;
	Entity		*m_pCEnt;

	int16_t	m_sMapCopyNum;
	int16_t	m_sCopyPlyNum;
	Corsairs::Util::SlotMap<CMapEntryCopyCell, defMAX_MAP_COPY_NUM>	m_LCopyInfo;
};

//
class	CDynMapEntry
{
public:
	CDynMapEntry();
	~CDynMapEntry();

	CDynMapEntryCell*	Add(CDynMapEntryCell *pCCell);
	bool	Del(CDynMapEntryCell *pCCell);
	bool	Del(const char *cszTMapName);
	CDynMapEntryCell*	GetEntry(const char *cszTMapN);
	void	AfterPlayerLogin(const char *cszName);

private:
	std::list<CDynMapEntryCell>	m_LEntryList;
};

extern CDynMapEntry g_CDMapEntry;

//
extern void	g_SetTeamFightMapName(const char *cszMapName);

extern char	g_szTFightMapName[MAX_MAPNAME_LENGTH];	//
//

#endif // MAPENTRY_H
