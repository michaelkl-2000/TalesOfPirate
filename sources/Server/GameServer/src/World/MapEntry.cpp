//=============================================================================
// FileName: MapEntry.cpp
// Creater: ZhangXuedong
// Date: 2005.10.21
// Comment: Map Entry class
//=============================================================================
#include "Core/stdafx.h"
#include "World/MapEntry.h"
#include "Script/LuaAPI.h"
#include "Network/NetCommand.h"

using namespace std;

CDynMapEntry	g_CDMapEntry;

char	g_szTFightMapName[MAX_MAPNAME_LENGTH] = "";	// 

//=============================================================================
void CMapEntryCopyCell::WriteParamPacket(Corsairs::Net::WPacket &pk)
{
	for (char i = 0; i < defMAPCOPY_INFO_PARAM_NUM; i++)
		pk.WriteInt64(m_lParam[i]);
}

//=============================================================================
void CDynMapEntryCell::SetCopyNum(int16_t sCopyNum)
{
	if (sCopyNum > defMAX_MAP_COPY_NUM)
	{
		//LG("", "msg %d  %d!\n", sCopyNum, defMAX_MAP_COPY_NUM);
		{ char _buf[256]; std::snprintf(_buf, sizeof(_buf), RES_STRING(GM_GAMEAPP_CPP_00008), sCopyNum, defMAX_MAP_COPY_NUM); g_logManager.InternalLog(LogLevel::Error, "errors", _buf); }
		return;
	}

	if (m_LCopyInfo.GetSize() == 0)
		m_LCopyInfo.Init(defMAX_MAP_COPY_NUM);
	else
		m_LCopyInfo.Reset();

	m_sMapCopyNum = sCopyNum;
	m_LCopyInfo.SetCapacity(m_sMapCopyNum);
}

void CDynMapEntryCell::SetEventName(const char *cszEventName)
{
	m_CEvtObj.SetName(cszEventName);

	if (m_pCEnt)
	{
		m_pCEnt->m_CEvent.SetName(cszEventName);
		m_pCEnt->SynEventInfo();
	}
}

void CDynMapEntryCell::FreeEnti()
{
	if (m_pCEnt)
	{
		m_pCEnt->Free();
		m_pCEnt = NULL;
	}
}

void CDynMapEntryCell::SynCopyParam(int16_t sCopyID)
{
	CMapEntryCopyCell	*pCCopyCell = GetCopy(sCopyID);
	if (!pCCopyCell)
		return;

	//  :   
	Corsairs::Net::Msg::GmMapEntryCopyParamMessage copyMsg;
	copyMsg.targetMapName = GetTMapName();
	copyMsg.srcMapName = GetMapName();
	copyMsg.copyId = sCopyID;
	for (char i = 0; i < defMAPCOPY_INFO_PARAM_NUM; i++)
		copyMsg.params[i] = pCCopyCell->GetParam(i);
	auto wpk = Corsairs::Net::Msg::serialize(copyMsg);

	BEGINGETGATE();
	GateServer	*pGateServer = NULL;
	while (pGateServer = GETNEXTGATE())
	{
		pGateServer->SendData(wpk);
	}
}

void CDynMapEntryCell::SynCopyRun(int16_t sCopyID, char chCdtType, std::int32_t chCdtVal)
{
	CMapEntryCopyCell	*pCCopyCell = GetCopy(sCopyID);
	if (!pCCopyCell)
		return;

	//  :    
	Corsairs::Net::Msg::MapEntryMessage meMsg;
	meMsg.srcMapName = GetTMapName();
	meMsg.targetMapName = GetMapName();
	meMsg.subType = Corsairs::Net::Msg::MAPENTRY_COPYRUN;
	meMsg.copyId = sCopyID;
	meMsg.condType = chCdtType;
	meMsg.condValue = chCdtVal;
	auto wpk = Corsairs::Net::Msg::serialize(meMsg, CMD_MT_MAPENTRY);

	BEGINGETGATE();
	GateServer	*pGateServer = NULL;
	while (pGateServer = GETNEXTGATE())
	{
		pGateServer->SendData(wpk);
		break;
	}
}

//=============================================================================
CDynMapEntryCell* CDynMapEntry::Add(CDynMapEntryCell *pCCell)
{
	CDynMapEntryCell	*pCObj;
	pCObj = GetEntry(pCCell->GetTMapName());
	if (!pCObj)
		pCObj = m_LEntryList.Add(pCCell);
	else
		pCObj->FreeEnti();
	if (!pCObj)
		return NULL;
	CEvent	*pCEvent = pCObj->GetEvent();
	pCEvent->SetTableRec(pCObj);

	return pCObj;
}

bool CDynMapEntry::Del(CDynMapEntryCell *pCCell)
{
	if (!pCCell)
		return false;

	pCCell->FreeEnti();
	return m_LEntryList.Del(pCCell);
}

bool CDynMapEntry::Del(const char *cszTMapName)
{
	if (!cszTMapName)
		return false;

	CDynMapEntryCell	*pCCell;
	m_LEntryList.BeginGet();
	while (pCCell = m_LEntryList.GetNext())
	{
		if (!strncmp(pCCell->GetTMapName(), cszTMapName, MAX_MAPNAME_LENGTH))
		{
			pCCell->FreeEnti();
			return m_LEntryList.Del(pCCell);
		}
	}

	return false;
}

CDynMapEntryCell* CDynMapEntry::GetEntry(const char *cszTMapName)
{
	CDynMapEntryCell	*pCCell;
	m_LEntryList.BeginGet();
	while (pCCell = m_LEntryList.GetNext())
	{
		if (!strncmp(pCCell->GetTMapName(), cszTMapName, MAX_MAPNAME_LENGTH))
			return pCCell;
	}

	return NULL;
}

void CDynMapEntry::AfterPlayerLogin(const char *cszName)
{
	if (!cszName)
		return;

	CDynMapEntryCell	*pCCell;
	m_LEntryList.BeginGet();
	while (pCCell = m_LEntryList.GetNext())
	{
		string	strScript = "after_player_login_";
		strScript += pCCell->GetTMapName();
		g_luaAPI.Call(strScript.c_str(), pCCell, cszName);
	}
}

//=============================================================================
void	g_SetTeamFightMapName(const char *cszMapName)
{
	if (cszMapName)
	{
		strncpy(g_szTFightMapName, cszMapName, MAX_MAPNAME_LENGTH - 1);
		g_szTFightMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
	}
	else
	{
		g_szTFightMapName[0] = '\0';
	}
}

// ============================================================================
// Ранее inline-методы из MapEntry.h, вынесены в .cpp 2026-04-22.
// ============================================================================

CMapEntryCopyCell::CMapEntryCopyCell(int16_t sMaxPlyNum, int16_t sCurPlyNum) {
	m_sMaxPlyNum = sMaxPlyNum;
	m_sCurPlyNum = sCurPlyNum;
	m_sPosID = -1;
}

void       CMapEntryCopyCell::SetMaxPlyNum(int16_t sPlyNum) { m_sMaxPlyNum = sPlyNum; }
int16_t CMapEntryCopyCell::GetMaxPlyNum(void)               { return m_sMaxPlyNum; }
void       CMapEntryCopyCell::SetCurPlyNum(int16_t sPlyNum) { m_sCurPlyNum = sPlyNum; }
int16_t CMapEntryCopyCell::GetCurPlyNum(void)               { return m_sCurPlyNum; }

bool CMapEntryCopyCell::AddCurPlyNum(int16_t sAddNum) {
	int16_t sNum = m_sCurPlyNum + sAddNum;
	if (sNum < 0 || sNum > m_sMaxPlyNum) return false;
	m_sCurPlyNum = sNum;
	return true;
}
bool CMapEntryCopyCell::HasFreePlyCount(int16_t sRequestNum) {
	return GetMaxPlyNum() - GetCurPlyNum() >= sRequestNum;
}

std::int32_t CMapEntryCopyCell::GetParam(char chParamID) {
	if (chParamID < 0 || chParamID >= defMAPCOPY_INFO_PARAM_NUM) return 0;
	return m_lParam[chParamID];
}
bool CMapEntryCopyCell::SetParam(char chParamID, std::int32_t lParamVal) {
	if (chParamID < 0 || chParamID >= defMAPCOPY_INFO_PARAM_NUM) return false;
	m_lParam[chParamID] = lParamVal;
	return true;
}

void       CMapEntryCopyCell::SetPosID(std::int32_t lPosID) { m_sPosID = (int16_t)lPosID; }
std::int32_t  CMapEntryCopyCell::GetPosID(void)             { return m_sPosID; }

// --- CDynMapEntryCell ---

CDynMapEntryCell::CDynMapEntryCell() {
	m_lEntiID       = 0;
	m_szMapName[0]  = '\0';
	m_szTMapName[0] = '\0';
	m_pCEnt         = nullptr;

	m_CEvtObj.Init();
	m_CEvtObj.SetTouchType(Corsairs::Common::World::enumEVENTT_RANGE);
	m_CEvtObj.SetExecType(Corsairs::Common::World::enumEVENTE_DMAP_ENTRY);

	m_pPos = nullptr;
}

void  CDynMapEntryCell::SetPos(void* pPos) { m_pPos = pPos; }
void* CDynMapEntryCell::GetPos(void)       { return m_pPos; }

void CDynMapEntryCell::SetMapName(const char* cszMapName) {
	if (!cszMapName) return;
	strncpy(m_szMapName, cszMapName, MAX_MAPNAME_LENGTH - 1);
	m_szMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
}
const char* CDynMapEntryCell::GetMapName(void) const { return m_szMapName; }

void CDynMapEntryCell::SetTMapName(const char* cszTMapName) {
	if (!cszTMapName) return;
	strncpy(m_szTMapName, cszTMapName, MAX_MAPNAME_LENGTH - 1);
	m_szTMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
}
const char* CDynMapEntryCell::GetTMapName(void) const     { return m_szTMapName; }

const Corsairs::Util::Point* CDynMapEntryCell::GetEntiPos(void) const     { return &m_SEntiPos; }
void         CDynMapEntryCell::SetEntiPos(const Corsairs::Util::Point* p) { m_SEntiPos = *p; }
void         CDynMapEntryCell::SetEntiPos(std::int32_t x, std::int32_t y) { m_SEntiPos.X = x; m_SEntiPos.Y = y; }
void         CDynMapEntryCell::SetEntiID(std::int32_t lEntiID)         { m_lEntiID = lEntiID; }
std::int32_t    CDynMapEntryCell::GetEntiID(void)                      { return m_lEntiID; }

void         CDynMapEntryCell::SetEventID(std::int32_t lEventID)       { m_CEvtObj.SetID((std::uint16_t)lEventID); }
std::int32_t    CDynMapEntryCell::GetEventID(void)                     { return m_CEvtObj.GetID(); }

void CDynMapEntryCell::SetEnti(Entity* pCEnt) { m_pCEnt = pCEnt; }

void CDynMapEntryCell::GetPosInfo(const char** pMapN, std::int32_t* lpPosX, std::int32_t* lpPosY, const char** pTMapN) {
	*pMapN  = m_szMapName;
	*lpPosX = m_SEntiPos.X;
	*lpPosY = m_SEntiPos.Y;
	*pTMapN = m_szTMapName;
}

CEvent*     CDynMapEntryCell::GetEvent(void)                   { return &m_CEvtObj; }
int16_t  CDynMapEntryCell::GetCopyNum(void)                 { return m_sMapCopyNum; }
void        CDynMapEntryCell::SetCopyPlyNum(int16_t sCopyNum) { m_sCopyPlyNum = sCopyNum; }
int16_t  CDynMapEntryCell::GetCopyPlyNum(void)              { return m_sCopyPlyNum; }

CMapEntryCopyCell* CDynMapEntryCell::AddCopy(CMapEntryCopyCell* pCCpyCell) { return m_LCopyInfo.Add(pCCpyCell); }
CMapEntryCopyCell* CDynMapEntryCell::GetCopy(int16_t sCopyID)           { return m_LCopyInfo.Get(sCopyID); }
bool               CDynMapEntryCell::ReleaseCopy(CMapEntryCopyCell* p)     { return m_LCopyInfo.Del(p); }
bool               CDynMapEntryCell::ReleaseCopy(std::int32_t lCopyNO)        { return m_LCopyInfo.Del(lCopyNO); }

// --- CDynMapEntry ---

CDynMapEntry::CDynMapEntry()  { m_LEntryList.Init(); }
CDynMapEntry::~CDynMapEntry() { m_LEntryList.Free(); }