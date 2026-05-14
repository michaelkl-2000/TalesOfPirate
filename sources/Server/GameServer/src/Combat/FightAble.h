//=============================================================================
// FileName: FightAble.h
// Creater: ZhangXuedong
// Date: 2004.09.15
// Comment: CFightAble class
//=============================================================================

#ifndef FIGHTABLE_H
#define FIGHTABLE_H

#include <array>
#include "Entity/Attachable.h"
#include "Character/ChaAttr.h"
#include "Character/CharacterRecord.h"
#include "Skill/SkillRecord.h"
#include "Core/GameCommon.h"
#include "Combat/SkillState.h"
#include "Inventory/SkillBag.h"
#include "Core/Timer.h"
#include "Combat/SkillTemp.h"

enum EItemInstance // 
{
	enumITEM_INST_BUY	= 0,		// 
	enumITEM_INST_MONS	= 1,		// 
	enumITEM_INST_COMP	= 2,		// 
	enumITEM_INST_TASK	= 3,		// 
};

enum EFightChaType
{
	enumFIGHT_CHA_SRC		= 0,	// 
	enumFIGHT_CHA_TAR		= 1,	// 
	enumFIGHT_CHA_SPLASH	= 2,	// 
};

struct SFireUnit
{
#ifdef defPROTOCOL_HAVE_PACKETID
	std::uint32_t	ulPacketID;	// ID
#endif
	std::uint8_t	uchFightID;

	CFightAble	*pCFightSrc;
	std::uint32_t	ulID;
	Corsairs::Util::Point		SSrcPos;
	std::int32_t	lTarInfo1;
	std::int32_t	lTarInfo2;

	int16_t		sExecTime;	// 
	CSkillRecord	*pCSkillRecord;
	CSkillTempData	*pCSkillTData;
};

struct SFightInit
{
	CSkillRecord	*pCSkillRecord;
	SSkillGrid		*pSSkillGrid;
	CSkillTempData	*pCSkillTData;
	// lInfo1,lInfo2 ,WorldID,Handle x,y
	struct
	{
		char		chTarType;	// 012
		std::int32_t		lTarInfo1;
		std::int32_t		lTarInfo2;
	};

	int16_t		sStopState;		// enumEXISTS_WAITING, enumEXISTS_SLEEPING
};

namespace Corsairs::Net::Msg {
	struct ChaSkillStateInfo;
	struct ChaAttrInfo;
} // namespace Corsairs::Net::Msg

/*
*	
*	lark.li
*/
class	CFightAble : public CAttachable
{
public:
	struct SFightProc
	{
		enum class Request
		{
			None,
			StopAttack,
			StartAttack
		};

		int16_t	sState;
		Request	sRequestState;

		bool		bCrt;			// 
		bool		bMiss;			// Miss

		std::int32_t		lERangeBParam[defSKILL_RANGE_BASEP_NUM];	//
	};

	int16_t	GetFightState(void);
	int16_t	GetFightStopState(void);

	bool	DesireFightBegin(SFightInit *);
	void	DesireFightEnd(void);
	void	OnFight(std::uint32_t ulCurTick);

	void	RangeEffect(SFireUnit *pSFireSrc, SubMap *pCMap, std::int32_t *plRangeBParam);
	void	SkillTarEffect(SFireUnit *pSFire);
	void	NotiSkillSrcToEyeshot(int16_t sExecTime = 0);
	void	NotiSkillSrcToSelf(int16_t sExecTime = 0);
	void	NotiSkillTarToEyeshot(SFireUnit *pSFireSrc);
	void	NotiChangeMainCha(std::uint32_t ulTargetID);
	void	SynAttr(int16_t sType);
	void	SynAttrToSelf(int16_t sType);
	void	SynAttrToEyeshot(int16_t sType);
	void	SynAttrToUnit(CFightAble *pCObj, int16_t sType);
	void	SynAttrToUnit(CFightAble *pCObj, int16_t sStartAttr, int16_t sEndAttr, int16_t sType);
	void	SynSkillStateToSelf(void);
	void	SynSkillStateToEyeshot(void);
	void	SynSkillStateToUnit(CFightAble *pCObj);
	void	SynLookEnergy(void);
	// 
	void	WriteSkillState(Corsairs::Net::WPacket &pk);
	void	WriteAttr(Corsairs::Net::WPacket &pk, int16_t sSynType);
	void	WriteMonsAttr(Corsairs::Net::WPacket &pk, int16_t sSynType);
	void	WriteAttr(Corsairs::Net::WPacket &pk, int16_t sStartAttr, int16_t sEndAttr, int16_t sSynType);
	void	WriteLookEnergy(Corsairs::Net::WPacket &pk);
	// Fill*     (CommandMessages.h)
	void	FillSkillState(Corsairs::Net::Msg::ChaSkillStateInfo &s);
	void	FillAttr(Corsairs::Net::Msg::ChaAttrInfo &a, int16_t sSynType);
	void	FillAttrAll(Corsairs::Net::Msg::ChaAttrInfo &a, int16_t sSynType); //   0..ATTR_CLIENT_MAX-1
	void	FillMonsAttr(Corsairs::Net::Msg::ChaAttrInfo &a, int16_t sSynType); // 5  

	bool	IsRightSkill(CSkillRecord *pSkill);
	bool	IsRightSkillSrc(char chSkillEffType);
	bool	IsRightSkillTar(CFightAble *pSkillSrc, char chSkillObjType, char chSkillObjHabitat, char chSkillEffType, bool bIncHider = false);
	bool	IsTeammate(CFightAble *pCFighter);
	bool	IsFriend(CFightAble *pCFighter);

	void	ResetFight();
	bool	RectifyAttr();

	std::int32_t	GetLevel(void);
	void		AddExp(std::uint32_t);
	bool		AddExpAndNotic(std::int32_t lAddExp, int16_t sNotiType = enumATTRSYN_TASK);

	void	CountLevel(void);
	void	CountSailLevel(void);
	void	CountLifeLevel(void);

	//
	virtual void AfterObjDie(CCharacter *pCAtk, CCharacter *pCDead);
	virtual void OnLevelUp(USHORT sLevel);
	virtual void OnSailLvUp(USHORT sLevel);
	virtual void OnLifeLvUp(USHORT sLevel);

	// 	
	void	SpawnResource( CCharacter *pCAtk, std::int32_t lSkillLv );
	void	ItemCount(CCharacter *pAtk);
	void	ItemInstance(char chType, SItemGrid *pGridContent, BOOL isTradable = 1, LONG expiration = 0);
	bool	GetTrowItemPos(std::int32_t *plPosX, std::int32_t *plPosY);
	bool	SkillExpend(int16_t sExecTime = 1);

	std::uint32_t	GetSkillDist(Entity *pTarEnt, CSkillRecord *pRec);
	bool        SkillTarIsEntity(CSkillRecord *pRec);

	void			BeUseSkill(std::int32_t lPreHp, std::int32_t lNowHp, CCharacter *pCSrcCha, char chSkillEffType);
	void			SetMonsterFightObj(std::uint32_t ulObjWorldID, std::int32_t lObjHandle);
	std::int32_t		GetSkillTime(CSkillTempData *pCSkillTData);
	void			EnrichSkillBag(bool bActive = true);
	virtual bool	AddSkillState(std::uint8_t uchFightID, std::uint32_t ulSrcWorldID, std::int32_t lSrcHandle, char chObjType, char chObjHabitat, char chEffType,
					std::uint8_t uchStateID, std::uint8_t uchStateLv, std::int32_t lOnTick, char chType = enumSSTATE_ADD_UNDEFINED, bool bNotice = true);
	virtual bool	DelSkillState(std::uint8_t uchStateID, bool bNotice = true);
	void			SetItemHostObj(CFightAble *pCObj);

	std::int32_t		setAttr(int nIdx, LONG32 lValue, int nType = 0);
	std::int32_t		getAttr(int nIdx);
	virtual void	AfterAttrChange(int nIdx, std::int32_t lOldVal, std::int32_t lNewVal);
	void			SetDie(CCharacter *pCSkillSrcCha);
	virtual void	Die();

	CCharacter* SkillPopBoat(std::int32_t lPosX, std::int32_t lPosY, int16_t sDir = -1);	//
	bool SkillPopBoat(CCharacter *pCBoat, std::int32_t lPosX, std::int32_t lPosY, int16_t sDir = -1);	//
	bool SkillInBoat(CCharacter* pCBoat);	// 
	bool SkillOutBoat(std::int32_t lPosX, std::int32_t lPosY, int16_t sDir = -1);	//
	bool SkillPushBoat(CCharacter* pCBoat, bool bFree = true);	// 

	std::uint32_t	m_ulPacketID;		// ID
	std::uint8_t	m_uchFightID;		//

	SFightInit	m_SFightInit;
	SFightProc	m_SFightProc;
	SFightInit	m_SFightInitCache;

	CChaAttr		m_CChaAttr;
	ChaRecord		*m_pCChaRecord;
	CSkillState		m_CSkillState;
	CSkillBag		m_CSkillBag;
	int16_t		m_sDefSkillNo;

	//virtual bool IsBoat(void);

protected:
	CFightAble();
	void	Initially();
	void	Finally();

	CFightAble*	IsFightAble();
	bool	GetFightTargetShape(Corsairs::Util::Square *pSTarShape);

	
	void	OnSkillState(DWORD dwCurTick);
	void	RemoveOtherSkillState();
	void	RemoveAllSkillState();

private:
	virtual void BeginFight();
	virtual void EndFight();
	void         OnFightBegin(void);
	void         OnFightEnd(void);

	virtual void SubsequenceFight();

	virtual void BreakAction(Corsairs::Net::RPacket* pk = nullptr);
	virtual void EndAction(Corsairs::Net::RPacket* pk = nullptr);

	bool SkillGeneral(std::int32_t lDistance, int16_t sExecTime = 1); //

	std::uint16_t	m_usTickInterval;	//
	std::uint32_t	m_ulLastTick;		//
	bool		m_bOnFight;

	bool		m_bLookAttrChange;	// 
	CFightAble*	m_pCItemHostObj;	// 

};

class	CTimeSkillMgr
{
public:
	struct SMgrUnit
	{
		SFireUnit	SFireSrc;
		std::uint32_t	ulLeftTick;	//
		SubMap		*pCMap;
		Corsairs::Util::Point		STargetPos;	// 
		std::int32_t		lERangeBParam[defSKILL_RANGE_BASEP_NUM];	// 
		SMgrUnit	*pSNext;
	};

	CTimeSkillMgr(unsigned short usFreq = 1000);
	~CTimeSkillMgr();

	void	Add(SFireUnit *pSFireSrc, std::uint32_t ulLeftTick, SubMap *pCMap, Corsairs::Util::Point *pStarget, std::int32_t *lRangeBParam);
	void	Run(std::uint32_t ulCurTick);
	void	ExecTimeSkill(SMgrUnit *pFireInfo);

private:
	std::uint32_t	m_ulTick;
	unsigned short	m_usFreq;	// 

	SMgrUnit	*m_pSExecQueue;	// 
	SMgrUnit	*m_pSFreeQueue;	// 

};

extern CTimeSkillMgr	g_CTimeSkillMgr;
extern std::array<char, Corsairs::Common::Character::kChaInitItemNum + 1> g_chItemFall;

#endif // FIGHTABLE_H
