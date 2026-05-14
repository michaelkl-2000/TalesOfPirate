//=============================================================================
// FileName: Player.h
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CPlayer class
//=============================================================================

#ifndef PLAYER_H
#define PLAYER_H


#include "Entity/AttachManage.h"
#include "Inventory/Kitbag.h"
#include "Services/Mission/Mission.h"
#include "App/GameAppNet.h"
#include "Inventory/ShipSet.h"
#include "Character/PlayerMapMask.h"
#include "Core/Timer.h"

#include <source_location>
#include <string>
#include <unordered_set>

namespace mission {
	class CStallData;
}

enum ELoginChaType {
	enumLOGIN_CHA_MAIN,
	enumLOGIN_CHA_BOAT,
};

class GateServer;

#define MAX_TEAM_MEMBER 5
#define MAX_BANK_NUM	1
#define ACT_NAME_LEN	64
#define PLAYER_INVALID_FLAG			0xF0F0F0F0L


//NOTE(Ogge): This should describe a player as a entity
class CPlayer : public GatePlayer, public mission::CCharMission {
public:
	CPlayer();

	void         Init(GateServer* pGate, std::uint32_t ulGtAddr);
	bool         IsValidFlag();
	void         SetPassword(const char szPassword[]);
	const char*  GetPassword();

	// DEBUG-сборка логирует каждый Free с адресом+именем+call-site (source_location).
	void Free(std::source_location loc = std::source_location::current());
	void Initially();
	void Finally();

	void         SetID(std::int32_t lID);
	std::int32_t    GetID(void);

	void         SetHandle(std::int32_t lHandle);
	std::int32_t    GetHandle(void);

	bool         IsPlayer(void);

	virtual void OnLogoff();

	bool         IsValid(void);

	void         SetActLoginID(DWORD id);
	DWORD        GetActLoginID();

	void         SetDBActId(DWORD dwDBActId);
	DWORD        GetDBActId(void);

	void         SetActName(const char* szActName);
	const char*  GetActName(void);

	void         SetGMLev(char chGMLev);
	std::uint8_t   GetGMLev(void);

	void         SetBankDBID(long lID, char chBankNO);
	long         GetBankDBID(char chBankNO);

	void SetIMP(long imp, bool sync = true);

	long         GetIMP();

	long m_lIMP;

	void         SetLoginCha(std::uint32_t ulType, std::uint32_t ulID);
	std::uint32_t   GetLoginChaType(void);
	std::uint32_t   GetLoginChaID(void);

	//mission::CCharMission& GetMission() { return *(mission::CCharMission*)this; }

	void AddTeamMember(uplayer*);
	void ClearTeamMember();

	bool         CanReceiveRequests();
	void         SetCanReceiveRequests(bool x);
	int          GetTeamMemberCnt();

	DWORD        GetTeamMemberDBID(int nNo);

	DWORD        getTeamLeaderID();
	CCharacter* GetTeamMemberCha(int nNo);
	void NoticeTeamMemberData();
	void NoticeTeamLeaderID(void);

	void         setTeamLeaderID(DWORD dwID);

	void LeaveTeam(void);
	void UpdateTeam(void);

	bool         IsTeamLeader(void);

	bool         HasTeam(void);

	void         BeginGetTeamPly(void);

	CPlayer* GetNextTeamPly(void);

	void         SetChallengeType(char chType);
	char    GetChallengeType(void);

	bool SetChallengeParam(char chParamID, std::int32_t lParamVal);
	std::int32_t GetChallengeParam(char chParamID);
	bool HasChallengeObj(void);
	void ClearChallengeObj(bool bAll = true);

	void         StartChallengeTimer(void);

	bool OpenRepair(CCharacter* pCNpc);

	CCharacter*  GetRepairman(void);

	bool SetRepairPosInfo(char chPosType, char chPosID);

	SItemGrid*   GetRepairItem(void);
	bool         CheckRepairItem(void);
	bool         IsRepairEquipPos(void);
	char    GetRepairPosID(void);
	bool         IsInRepair(void);
	void         SetInRepair(bool bInR = true);

	bool OpenForge(CCharacter* pCNpc);

	bool OpenLottery(CCharacter* pCNpc); //Add by lark.li 20080514

	bool OpenUnite(CCharacter* pCNpc);
	bool OpenMilling(CCharacter* pCNpc);
	bool OpenFusion(CCharacter* pCNpc);
	bool OpenUpgrade(CCharacter* pCNpc);
	bool OpenEidolonMetempsychosis(CCharacter* pCNpc);
	bool OpenEidolonFusion(CCharacter* pCNpc);
	bool OpenPurify(CCharacter* pCNpc);
	bool OpenFix(CCharacter* pCNpc);
	bool OpenEnergy(CCharacter* pCNpc);
	bool OpenGetStone(CCharacter* pCNpc);
	bool OpenTiger(CCharacter* pCNpc);

	CCharacter*  GetForgeman(void);
	bool         IsInForge(void);
	bool         IsInLifeSkill(void);
	void         SetInForge(bool bInForge = true);
	void         SetInLifeSkill(bool bInLiftSkill = true);
	void         SetForgeInfo(char chType, SForgeItem* pSItem);
	void         SetLifeSkillInfo(long lType, SLifeSkillItem* pLifeSkill);
	char    GetForgeType(void);
	SForgeItem*  GetForgeItem(void);
	SLifeSkillItem* GetLifeSkillItem();

	void         SetMainCha(CCharacter* pMainCha);
	void         SetCtrlCha(CCharacter* pCtrlCha);
	CCharacter*  GetMainCha(void);
	CCharacter*  GetCtrlCha(void);

	CCharacter*  GetMakingBoat();
	void         SetMakingBoat(CCharacter* pBoat);

	BYTE         GetNumBoat();

	void GetBerthBoat(USHORT sBerthID, BYTE& byNumBoat, BOAT_BERTH_DATA& Data);
	void GetAllBerthBoat(USHORT sBerthID, BYTE& byNumBoat, BOAT_BERTH_DATA& Data);
	void GetDeadBerthBoat(USHORT sBerthID, BYTE& byNumBoat, BOAT_BERTH_DATA& Data);
	void SetBerth(USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir);
	void GetBerth(USHORT& sBerthID, USHORT& sxPos, USHORT& syPos, USHORT& sDir);
	BOOL HasAllBoatInBerth(USHORT sBerthID);
	BOOL HasBoatInBerth(USHORT sBerthID);
	BOOL HasDeadBoatInBerth(USHORT sBerthID);

	BOOL         IsBoatFull();
	BOOL         IsLuanchOut();
	void         SetLuanchOut(DWORD dwID);
	DWORD        GetLuanchID();

	CCharacter* GetLuanchOut();

	CCharacter* GetBoat(BYTE byIndex);

	CCharacter* GetBoat(std::uint32_t dwBoatDBID);
	CCharacter* GetBoat(std::int32_t dwBoatDBID);
	BYTE GetBoatIndexByDBID(DWORD dwBoatDBID);
	BOOL AddBoat(CCharacter& Boat);
	BOOL ClearBoat(DWORD dwBoatDBID);
	void RefreshBoatAttr(void);

	mission::CStallData* GetStallData();
	void                 SetStallData(mission::CStallData* pData);

	// Загрузка/сохранение base64 для конкретной карты (используется БД-слоем).
	bool         LoadMapMaskBase64(std::string_view mapName, std::string_view base64Data);
	std::string  SaveMapMaskBase64(std::string_view mapName) const;
	// Wire-формат текущей маски (m_szMaskMapName) для CMD_MC_MAP_MASK:
	//   [sMapMask 48-byte header][bits] (клиент парсит header).
	std::vector<std::uint8_t> GetMapMaskWire() const;

	bool RefreshMapMask(std::string_view szMapName, std::int32_t lPosX, std::int32_t lPosY);

	void         SetMaskMapName(std::string_view szMapName);
	const char*  GetMaskMapName(void);

	// Per-map dirty tracking: какие маски менялись с момента последнего сейва.
	const std::unordered_set<std::string>& GetDirtyFogMaps() const { return m_dirtyFogMaps; }
	void                                   ClearDirtyFogMaps() { m_dirtyFogMaps.clear(); }
	bool                                   HasDirtyFogMaps() const { return !m_dirtyFogMaps.empty(); }

	float        GetMapMaskOpenScale(std::string_view szMapName);

	char         GetCurBankNum(void);
	bool         AddBankDBID(long lDBID);
	CKitbag*     GetBank(char chBankNO = 0);

	char* BankDBIDData2String(char* szSStateBuf, int nLen);
	bool Strin2BankDBIDData(std::string& strData);
	bool AddBank(void);
	bool SaveBank(char chBankNO = -1);
	bool SetBankChangeFlag(char chBankNO = -1, bool bChange = true);
	bool SynBank(char chBankNO = -1, char chType = enumSYN_KITBAG_INIT);
	bool SynGuildBank(CKitbag* bag, char chType);
	bool OpenBank(CCharacter* pCNpc);
	bool OpenGuildBank();
	bool GetGuildGold();
	bool BankCanOpen(CCharacter* pCNpc);
	void CloseBank(void);

	CCharacter*  GetBankNpc(void);

	bool SetBankSaveFlag(char chBankNO = -1, bool bChange = true);
	bool BankWillSave(char chBankNO = 0);
	bool BankHasItem(USHORT sItemID, USHORT& sCount);

	void SystemNotice(const char szData[], ...);
	void Run(DWORD dwCurTime);
	void CheckChaItemFinalData(void);
	bool String2BankData(char chBankNO, std::string& strData);

	long         GetMoBean();
	void         SetMoBean(long lMoBean);
	long         GetRplMoney();
	void         SetRplMoney(long lRplMoney);
	long         GetVipType();
	void         SetVipType(long lVipType);
	short        IsGarnerWiner();
	void         SetGarnerWiner(short siswiner);

	uplayer _Team[MAX_TEAM_MEMBER];

	std::string& GetLifeSkillinfo();

	char m_szGuildName[defGUILD_NAME_LEN]; //ID0.
	char m_szGuildMotto[defGUILD_MOTTO_LEN];
	char m_szStallName[ROLE_MAXNUM_STALL_NUM];
	bool m_bIsGuildLeader; //0-;1-
	struct {
		uint32_t m_GuildState{}; //,:enum EGuildState

		std::uint32_t m_GuildStatus; //
		std::uint32_t m_lGuildID; //ID.
		std::uint32_t m_lTempGuildID; //
		char m_szTempGuildName[defGUILD_NAME_LEN]; //.
	};

protected:
	int _nTeamMemberCnt;
	DWORD _dwTeamLeaderID;

private:
	std::int32_t m_lID;
	std::int32_t m_lHandle;

	bool bReceiveRequests{true};
	bool bIsValid;
	char m_szPassword[ROLE_MAXSIZE_PASSWORD2 + 1];

	long m_lMoBean; //
	long m_lRplMoney; //
	long m_lVipID; //VIP

	DWORD m_dwLoginID; //  Account DB ID
	DWORD m_dwDBActId; // ID
	char m_chGMLev; // GM
	char m_chActName[ACT_NAME_LEN]; //

	CCharacter* m_pCtrlCha; //

	CCharacter* m_pMainCha; //
	DWORD m_dwLaunchID; // ID
	BYTE m_byNumBoat; //
	CCharacter* m_Boat[MAX_CHAR_BOAT];

	std::uint32_t m_ulLoginCha[2]; // ID.0 1

	//
	struct {
		USHORT m_sBerthID; //
		USHORT m_sxPos;
		USHORT m_syPos;
		USHORT m_sDir;
	};

	//
	struct {
		Corsairs::Common::Character::PlayerMapMask m_CMapMask;
		char m_szMaskMapName[MAX_MAPNAME_LENGTH];
		// Имена карт, чья маска менялась с последнего сейва (per-map dirty set).
		std::unordered_set<std::string> m_dirtyFogMaps;
	};

	//
	struct {
		CCharacter* m_pCBankNpc;
		char m_chBankNum;
		long m_lBankDBID[MAX_BANK_NUM];
		CKitbag m_CBank[MAX_BANK_NUM];
		bool m_bBankChange[MAX_BANK_NUM];
	};

	//
	CCharacter* m_pMakingBoat;

	//
	mission::CStallData* m_pStallData;

	//
	struct {
		char m_chChallengeType; //
		std::int32_t m_lChallengeParam[2 + MAX_TEAM_MEMBER * 2 * 2]; //
		CTimer m_timerChallenge; //
	};

	//
	struct {
		bool m_bInRepair;
		char m_chRepairPosType;
		char m_chRepairPosID;
		SItemGrid m_SRepairItem;
		SItemGrid* m_pSRepairItem;
		CCharacter* m_pCRepairman;
	};

	// /
	struct {
		bool m_bInForge;
		char m_chForgeType;
		SForgeItem m_SForgeItem;
		CCharacter* m_pCForgeman;
	};

	//
	struct {
		CCharacter* m_pCLotteryman;
	};

	//Add by sunny.sun20080716
	struct {
		CCharacter* m_pCAmphitheaterman;
	};

	struct {
		bool m_bInLiftSkill;
		std::int32_t m_lLifeSkillType;
		SLifeSkillItem m_pSLifeSkillItem;
		std::string m_strLifeSkillinfo;
	};

	int16_t m_sGetTeamPlyCount;

	DWORD m_dwValidFlag;
	short m_sGarnerWiner;
};

extern CPlayer* CreatePlayer(GateServer* pGate, std::uint32_t ulGateAddr, std::uint32_t ulChaDBId);
extern void ReleasePlayer(CPlayer* pPlayer);

#endif // PLAYER_H
