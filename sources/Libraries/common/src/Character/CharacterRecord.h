//======================================================================================================================
// FileName: CharacterRecord.h
// Creater: ZhangXuedong
// Date: 2004.09.01
// Comment: CChaRecordSet class
//======================================================================================================================

#pragma once

#include <array>
#include <cstdint>
#include <source_location>
#include <string>
#include <tchar.h>
#include "util.h"
#include "Database/TableData.h"

namespace Corsairs::Common::Character {

enum class EChaModalType : std::int32_t {
	MAIN_CHA = 1,
	BOAT,
	EMPL,
	OTHER,
};

enum class EChaCtrlType : std::int32_t {
	NONE			= 0,

	PLAYER			= 1,

	NPC				= 2,
	NPC_EVENT		= 3,

	MONS			= 5,
	MONS_TREE		= 6,
	MONS_MINE		= 7,
	MONS_FISH		= 8,
	MONS_DBOAT		= 9,

	PLAYER_PET		= 10,

	MONS_REPAIRABLE = 17,
};

enum class EChaAiType : std::int32_t {
	NONE			= 0,
	ATTACK_PASSIVE	= 1,
	ATTACK_ACTIVE	= 2,
};

// Sentinel-байт для пустых ячеек массивов lItem/lTaskItem/sSkinInfo/sItemType (0xFF / -1).
inline constexpr std::int8_t kChaRecordKeyValue = static_cast<std::int8_t>(0xFF);

inline constexpr std::size_t kChaSkinNum			= 8;
inline constexpr std::size_t kChaInitSkillNum		= 11;	// 0,1-10
inline constexpr std::size_t kChaInitItemNum		= 15;	// Edit by Mdrst
inline constexpr std::size_t kChaItemKindNum		= 20;
inline constexpr std::size_t kChaDieEffectNum		= 3;
inline constexpr std::size_t kChaBirthEffectNum		= 3;
inline constexpr std::size_t kChaHpEffectNum		= 3;
inline constexpr std::size_t kChaFeffNum			= 4;
inline constexpr std::size_t kChaEffectActionNum	= 3;
inline constexpr std::size_t kChaScalingNum			= 3;

class ChaRecord : public EntityData
{
public:
	long	lID;								//
	std::string	szName;							//
	std::string	szIconName;						//
	char	chModalType;						//
	char	chCtrlType;							//
	short	sModel;								//
	short	sSuitID;							//
	short	sSuitNum;							//
	std::array<short, kChaSkinNum>			sSkinInfo;			//
	std::array<short, kChaFeffNum>			sFeffID;			// FeffID
	short	sEeffID;							// EeffID
	std::array<short, kChaEffectActionNum>	sEffectActionID;	// ,0-,1-,2-dummy
	short	sShadow;							//
    short   sActionID;                          //
    char    chDiaphaneity;                      //
	short	sFootfall;							//
	short	sWhoop;								//
	short	sDirge;								//
	char	chControlAble;						//
	//char	chMoveAble;							//
	char	chTerritory;						//
	short   sSeaHeight;							//
	std::array<short, kChaItemKindNum>		sItemType;			//
	float	fLengh;								//
	float	fWidth;								//
	float	fHeight;							//
	short	sRadii;								//
	std::array<char, kChaBirthEffectNum>	nBirthBehave;		//
	std::array<char, kChaDieEffectNum>		nDiedBehave;		//
    short   sBornEff;                           //
    short   sDieEff;                            //
	short	sDormancy;							//
    char    chDieAction;                        //
	std::array<int, kChaHpEffectNum>		_nHPEffect;			// hp
	bool	_IsFace;							//
	bool	_IsCyclone;							//
	long	lScript;							// ID
	long	lWeapon;							// ID
	std::array<std::array<long, 2>, kChaInitSkillNum>	lSkill;		//
	std::array<std::array<long, 2>, kChaInitItemNum>	lItem;		//
	std::array<std::array<long, 2>, kChaInitItemNum>	lTaskItem;	//
	long	lMaxShowItem;						//
	float	fAllShow;							//
	long	lPrefix;							//
	long	lAiNo;								// AI
	char	chCanTurn;							//
	long	lVision;							//
	long	lNoise;								//
	long	lGetEXP;							// EXP
	bool	bLight;								//

	long	lMobexp;// exp
	long	lLv;	//
	long	lMxHp;	// HP
	long	lHp;	// HP
	long	lMxSp;	// SP
	long	lSp;	// SP
	long	lMnAtk;	//
	long	lMxAtk;	//
	long	lPDef;	//
	long	lDef;	//
	long	lHit;	//
	long	lFlee;	//
	long	lCrt;	//
	long	lMf;	//
	long	lHRec;	// hp
	long	lSRec;	// sp
	long	lASpd;	//
	long	lADis;	//
	long	lCDis;	//
	long	lMSpd;	//
	long	lCol;	//
	long	lStr;	// strength
	long	lAgi;	// agility
	long	lDex;	// dexterity
	long	lCon;	// constitution	hp
	long	lSta;	// stamina		sp
	long	lLuk;	// luck
	long	lLHandVal;	//
	std::string	szGuild;						//
	std::string	szTitle;						//
	std::string	szJob;							//
	std::int32_t	lCExp;	//
	std::int32_t	lNExp;	//
	long	lFame;	//
	long	lAp;	//
	long	lTp;	// technique point
	long	lGd;	//
	long	lSpri;	//
	long	lStor;	// ()
	long	lMxSail;	//
	long	lSail;	//
	long	lStasa;	//
	long	lScsm;	//

	long	lTStr;	//
	long	lTAgi;	//
	long	lTDex;	//
	long	lTCon;	//
	long	lTSta;	//
	long	lTLuk;	//
	long	lTMxHp;	//
	long	lTMxSp;	//
	long	lTAtk;	//
	long	lTDef;	//
	long	lTHit;	//
	long	lTFlee;	//
	long	lTMf;	//
	long	lTCrt;	//
	long	lTHRec;	// hp		hp
	long	lTSRec;	// sp		sp
	long	lTASpd;	//
	long	lTADis;	//
	long	lTSpd;	//
	long	lTSpri;	//
	long	lTScsm;	//

	// added by clp
	std::array<float, kChaScalingNum>	scaling;

public:
	bool	HaveEffectFog() const				{ return _HaveEffectFog;			}
	int		GetEffectFog( int i ) const			{ return _nHPEffect.at(i);			}

	bool	IsFace() const						{ return _IsFace;					}
	bool	IsCyclone() const					{ return _IsCyclone;				}

	void	RefreshPrivateData();				//

private:
	bool	_HaveEffectFog;

};

ChaRecord* GetChaRecordInfo(int nTypeID, const std::source_location& loc = std::source_location::current());


} // namespace Corsairs::Common::Character
