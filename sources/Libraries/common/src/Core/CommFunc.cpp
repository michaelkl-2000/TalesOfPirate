//=============================================================================
// FileName: CommFunc.cpp
// Creater: Jerry Li
// Date: 2005.02.23
// Comment:
//	2005.4.28	Arcol	add the text filter manager class: CTextFilter
//=============================================================================

#include "Core/CommFunc.h"
using namespace Corsairs::Common::Skill;
#include "Localization/i18n.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <regex>
#include <string_view>
#include <vector>

using namespace std;

const char* g_szJobName[MAX_JOB_TYPE] = {
	"Newbie",
	"Swordsman",
	"Hunter",
	"Sailor",
	"Explorer",
	"Herbalist",
	"Engineer",
	"Merchant",
	"Champion",
	"Crusader",
	"White Knight",
	"Animal Tamer",
	"Sharpshooter",
	"Cleric",
	"Seal master",
	"Captain",
	"Voyager",
	"Upstart",
	"Engineer",
};

namespace Corsairs::Common::Network {
const char* g_szCityName[defMAX_CITY_NUM] = {
	"Argent city",
	"Thundoria Castle",
	"Shaitan City",
	"Icicle",
};
} // namespace Corsairs::Common::Network

const long g_PartIdRange[PLAY_NUM][enumEQUIP_NUM + 1][2] = 
{
						/**/		/**/
	//
		/**/			0,					0,
		/*HEAD */	2554,				2561,
		/*BODY */	0,					0,
		/*GLOVE*/	0,					0,
		/*SHOES*/	0,					0,
		/*NECK */	0,					0,
		/*LHAND*/	0,					0,
		/*HAND1*/	0,					0,
		/*HAND2*/	0,					0,
		/*RHAND*/	0,					0,
		/*FACE */	2000,				2007,

	//
		/**/			0,					0,
		/*HEAD */	2554,				2561,
		/*BODY */	0,					0,
		/*GLOVE*/	0,					0,
		/*SHOES*/	0,					0,
		/*NECK */	0,					0,
		/*LHAND*/	0,					0,
		/*HAND1*/	0,					0,
		/*HAND2*/	0,					0,
		/*RHAND*/	0,					0,
		/*FACE */	2062,				2069,

	//
		/**/			0,					0,
		/*HEAD */	2554,				2561,
		/*BODY */	0,					0,
		/*GLOVE*/	0,					0,
		/*SHOES*/	0,					0,
		/*NECK */	0,					0,
		/*LHAND*/	0,					0,
		/*HAND1*/	0,					0,
		/*HAND2*/	0,					0,
		/*RHAND*/	0,					0,
		/*FACE */	2124,				2131,

	//
		/**/			0,					0,
		/*HEAD */	2554,				2561,
		/*BODY */	0,					0,
		/*GLOVE*/	0,					0,
		/*SHOES*/	0,					0,
		/*NECK */	0,					0,
		/*LHAND*/	0,					0,
		/*HAND1*/	0,					0,
		/*HAND2*/	0,					0,
		/*RHAND*/	0,					0,
		/*FACE */	2291,				2294,
};

bool IsValidLook( int nType, int nPart, long nValue )
{
	if( (nType>=1 && nType<=PLAY_NUM) && (nPart>=0 && nPart<enumEQUIP_NUM + 1)  )
	{
		/*return	(nValue >= g_PartIdRange[nType-1][nPart][0]) && 
				(nValue <= g_PartIdRange[nType-1][nPart][1]);*/
		return true;
	}
	return false;
}

bool IsDist(int x1, int y1, int x2, int y2, std::uint32_t dwDist)
{
	std::uint32_t dwxDist = (x1 - x2) * (x1 - x2);
	std::uint32_t dwyDist = (y1 - y2) * (y1 - y2);
	dwDist *= dwDist;
	return (dwxDist + dwyDist < dwDist * 10000);
}

bool FileExists(std::string_view szFileName)
{
	std::error_code ec;
	return std::filesystem::exists(std::filesystem::path(szFileName.data(), szFileName.data() + szFileName.size()), ec);
}

bool IsEmail(std::string_view email)
{
	static const std::regex pattern(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
	return std::regex_match(email.begin(), email.end(), pattern);
}

const char* GetServerError(int error_code)
{
	switch (error_code) {
	case ERR_AP_INVALIDUSER:   return "Invalid Account";
	case ERR_AP_INVALIDPWD:    return "Password incorrect";
	case ERR_AP_ACTIVEUSER:    return "Account activation failed";
	case ERR_AP_DISABLELOGIN:  return "Your cha is currently in logout save mode, please try logging in again later.";
	case ERR_AP_LOGGED:        return "This account is already online";
	case ERR_AP_BANUSER:       return "Account has been banned";
	case ERR_AP_GPSLOGGED:     return "This GroupServer has login";
	case ERR_AP_GPSAUTHFAIL:   return "This GroupServer Verification failed";
	case ERR_AP_SAVING:        return "Saving your character, please try again in 15 seconds...";
	case ERR_AP_LOGINTWICE:    return "Your account is logged on far away";
	case ERR_AP_ONLINE:        return "Your account is already online";
	case ERR_AP_DISCONN:       return "GroupServer disconnected";
	case ERR_AP_UNKNOWNCMD:    return "unknown agreement, don't deal with";
	case ERR_AP_TLSWRONG:      return "local saving error";
	case ERR_AP_NOBILL:        return "This account has expired, please topup!";

	case ERR_PT_LOGFAIL:       return "GateServer to GroupServer login failed";
	case ERR_PT_SAMEGATENAME:  return "GateServer and login GateServer have similar name";
	case ERR_PT_INVALIDDAT:    return "Ineffective data model";
	case ERR_PT_INERR:         return "server link operation integrality error ";
	case ERR_PT_NETEXCP:       return "Account server has encountered a malfunction";
	case ERR_PT_DBEXCP:        return "database server malfunction";
	case ERR_PT_INVALIDCHA:    return "Current account does not have a request (Select/Delete) to character";
	case ERR_PT_TOMAXCHA:      return "reached the maximum number of characters you can create";
	case ERR_PT_BONUSCHARS:    return "You don't have the necessary levels to create another character";
	case ERR_PT_SAMECHANAME:   return "Character name already exist";
	case ERR_PT_INVALIDBIRTH:  return "illegal birth place";
	case ERR_PT_TOOBIGCHANM:   return "Character name is too long";
	case ERR_PT_ISGLDLEADER:   return "Guild must have a leader, please disband your guild first then delete your character";
	case ERR_PT_ERRCHANAME:    return "Illegal character name";
	case ERR_PT_SERVERBUSY:    return "System is busy, please try again later";
	case ERR_PT_TOOBIGPW2:     return "second code length illegal";
	case ERR_PT_INVALID_PW2:   return "Cha second password not created";
	case ERR_PT_BADBOY:        return "My child, you are very bold. You have been reported to the authority. Please do not commit the offense again!";
	case ERR_PT_BANUSER:       return RES_STRING(CO_COMMFUNC_H_00031);
	case ERR_PT_PBANUSER:      return RES_STRING(CO_COMMFUNC_H_00108);

	case ERR_MC_NETEXCP:       return "Discovered exceptional line error on GateServer";
	case ERR_MC_NOTSELCHA:     return "current not yet handled character state";
	case ERR_MC_NOTPLAY:       return "Currently not in gameplay, unable to send ENDPLAY command";
	case ERR_MC_NOTARRIVE:     return "target map cannot be reached";
	case ERR_MC_TOOMANYPLY:    return "This server is currently full, please select another server!";
	case ERR_MC_NOTLOGIN:      return "Youa re not login";
	case ERR_MC_VER_ERROR:     return "Client version error, server refused connection!";
	case ERR_MC_ENTER_ERROR:   return "failed to enter map!";
	case ERR_MC_ENTER_POS:     return "Map position illegal, you'll be sent back to your birth city, please relog!";
	case ERR_MC_BANUSER:       return RES_STRING(CO_COMMFUNC_H_00031);
	case ERR_MC_PBANUSER:      return RES_STRING(CO_COMMFUNC_H_00108);

	case ERR_TM_OVERNAME:      return "GameServer name repeated";
	case ERR_TM_OVERMAP:       return "GameServerMapNameRepeated";
	case ERR_TM_MAPERR:        return "GameServer map assign language error";

	case ERR_SUCCESS:          return "Jack is too BT, correct also will ask me if anything is wrong!";
	}

	// default-branch — диагностический код ошибки + текст диапазона. Раньше тут жил
	// общий static char[500] с itoa+strcat — UB в многопоточном сервере. thread_local
	// держит буфер per-thread, std::format безопасно собирает строку.
	thread_local std::string buffer;
	const int bucket = (error_code / 500) * 500;

	const char* range = nullptr;
	switch (bucket) {
	case ERR_MC_BASE: range = "(GameServer/GateServer->Client return error code space 1-500)"; break;
	case ERR_PT_BASE: range = "(GroupServer->GateServer return error code range 501-1000)"; break;
	case ERR_AP_BASE: range = "(AccountServer->GroupServe return error code from 1001-1500)"; break;
	case ERR_MT_BASE: range = "(GameServer->GateServer return error code range 1501-2000)"; break;
	default:          range = "(Jack is too insane, he made a mistake that I don't even know.)"; break;
	}

	buffer = std::format("{}{}", error_code, range);
	return buffer.c_str();
}

namespace Corsairs::Common::Network {
const char* g_GetAreaName( int nValue )
{
    switch( nValue )
    {
    case 1:  return "Land / Sea"; //"/";
    case 2:  return "safe zone"; //"";
    case 3:  return "Non PK zone"; //"PK";
    case 4:  return "Bridge"; //"";
    case 5:  return "Forbid monster zone"; //"";
    case 6:  return "Mining Area"; //"";
    default: return "Unknown"; //"";
    }
}
} // namespace Corsairs::Common::Network

bool IsRealItemId(int nItemID)
{
	return nItemID > 0
		&& nItemID != enumEQUIP_BOTH_HAND
		&& nItemID != enumEQUIP_TOTEM;
}

static inline int GetItemType( int nItemID )
{
	if( IsRealItemId(nItemID) )
	{
		CItemRecord* pItem = GetItemRecordInfo( nItemID );
		if( pItem )
		{
			return pItem->sType;
		}

		return -1;
	}

	return nItemID;
}

int GetItemSkill( int nLeftItemID, int nRightItemID )
{
	int nRightType = GetItemType( nRightItemID );
	int nLeftType = GetItemType( nLeftItemID );

	const int nLRSkill[][3] = {
		// 
		0,		0,		25,
		1,		1,		38,
		11,		1,		28,
		9999,	2,		29,
		9999,	6,		33
	};

		// 
	const int nRSkill[][3] = {
		-1,		1,		28,
		-1,		4,		31,
		-1,		5,		32,
		-1,		7,		34,
		-1,		8,		35,
		-1,		9,		36,
		-1,		10,		37,
		-1,		18,		200,
		-1,		19,		201
	};

		// 
	const int nLSkill[][3] = {
		11,		-1,		25,
		3,		-1,		30
	};

	const int nLRCount = sizeof(nLRSkill) / sizeof(nLRSkill[0]);
	for( int i=0; i<nLRCount; i++ )
	{
		if( nRightType==nLRSkill[i][1] && nLeftType==nLRSkill[i][0] )
		{
			return nLRSkill[i][2];
		}
	}

	const int nRCount = sizeof(nRSkill) / sizeof(nRSkill[0]);
	for( int i=0; i<nRCount; i++ )
	{
		if( nRightType==nRSkill[i][1] )
		{
			return nRSkill[i][2];
		}
	}

	const int nLCount = sizeof(nLSkill) / sizeof(nLSkill[0]);
	for( int i=0; i<nLCount; i++ )
	{
		if( nLeftType==nLSkill[i][0] )
		{
			return nLSkill[i][2];
		}
	}
	return -1;
}

int IsUseSkill( stNetChangeChaPart *pSEquip, int nSkillID )
{
	CSkillRecord *p =  GetSkillRecordInfo( nSkillID );
	if( !p )
	{
		return -1;
	}

	int	nLHandID = pSEquip->SLink[enumEQUIP_LHAND].sID;
	int	nRHandID = pSEquip->SLink[enumEQUIP_RHAND].sID;
	int	nBodyID = pSEquip->SLink[enumEQUIP_BODY].sID;
	if (!pSEquip->SLink[enumEQUIP_LHAND].IsValid())
		nLHandID = 0;
	if (!pSEquip->SLink[enumEQUIP_RHAND].IsValid())
		nRHandID = 0;
	if (!pSEquip->SLink[enumEQUIP_BODY].IsValid())
		nBodyID = 0;

	short sLHandType = 0, sRHandType = 0, sBodyType = 0;

	CItemRecord	*pItem;

	if (nLHandID > 0) {
		pItem = GetItemRecordInfo(nLHandID);
		if (pItem) sLHandType = pItem->sType;
	}

	if (nRHandID > 0) {
		pItem = GetItemRecordInfo(nRHandID);
		if (pItem) sRHandType = pItem->sType;
	}

	if (nBodyID > 0) {
		pItem = GetItemRecordInfo(nBodyID);
		if (pItem) sBodyType = pItem->sType;
	}

	bool IsLeft = false;
	bool IsRight = false;
	bool IsBody = false;
	bool IsConch = false;

	// 
	for( int i=0; i<defSKILL_ITEM_NEED_NUM; i++ )
	{	
		if( p->sItemNeed[0][i][0] == cchSkillRecordKeyValue )
			break;

		if(p->sItemNeed[0][i][0] == enumSKILL_ITEM_NEED_TYPE)
		{
			if (p->sItemNeed[0][i][1] == -1 || p->sItemNeed[0][i][1] == sLHandType)
			{
				IsLeft = true;
				break;
			}
		}
		else if (p->sItemNeed[0][i][0] == enumSKILL_ITEM_NEED_ID)
		{
			if (p->sItemNeed[0][i][1] == nLHandID)
			{
				IsLeft = true;
				break;
			}
		}
	}
	if (!IsLeft)
		return 0;

	// 
	for( int i=0; i<defSKILL_ITEM_NEED_NUM; i++ )
	{	
		if( p->sItemNeed[1][i][0] == cchSkillRecordKeyValue )
			break;

		if(p->sItemNeed[1][i][0] == enumSKILL_ITEM_NEED_TYPE)
		{
			if (p->sItemNeed[1][i][1] == -1 || p->sItemNeed[1][i][1] == sRHandType)
			{
				IsRight = true;
				break;
			}
		}
		else if (p->sItemNeed[1][i][0] == enumSKILL_ITEM_NEED_ID)
		{
			if (p->sItemNeed[1][i][1] == nRHandID)
			{
				IsRight = true;
				break;
			}
		}
	}
	if (!IsRight)
		return 0;

	// 
	for( int i = 0; i < defSKILL_ITEM_NEED_NUM; i++ )
	{	
		if( p->sItemNeed[2][i][0] == cchSkillRecordKeyValue )
			break;

		if(p->sItemNeed[2][i][0] == enumSKILL_ITEM_NEED_TYPE)
		{
			if (p->sItemNeed[2][i][1] == -1 || p->sItemNeed[2][i][1] == sBodyType)
			{
				IsBody = true;
				break;
			}
		}
		else if (p->sItemNeed[2][i][0] == enumSKILL_ITEM_NEED_ID)
		{
			if (p->sItemNeed[2][i][1] == nBodyID)
			{
				IsBody = true;
				break;
			}
		}
	}
	if (!IsBody)
		return 0;

	// 
	for (int i = 0; i < defSKILL_ITEM_NEED_NUM; i++)
	{
		if (p->sConchNeed[i][0] == cchSkillRecordKeyValue)
			break;

		if (p->sConchNeed[i][0] == -1) // 
		{
			IsConch = true;
			break;
		}
		if (!pSEquip->SLink[p->sConchNeed[i][0]].IsValid())
			continue;
		pItem = GetItemRecordInfo(pSEquip->SLink[p->sConchNeed[i][0]].sID);
		if (pItem)
		{
			if (p->sConchNeed[i][1] == enumSKILL_ITEM_NEED_TYPE)
			{
				if (pItem->sType == p->sConchNeed[i][2])
				{
					IsConch = true;
					break;
				}
			}
			else if (p->sConchNeed[i][1] == enumSKILL_ITEM_NEED_ID)
			{
				if (pItem->lID == p->sConchNeed[i][2])
				{
					IsConch = true;
					break;
				}
			}
		}
	}
	if (!IsConch)
		return 0;

	return 1;
}

//=============================================================================
// 
// nTChaCtrlType EChaCtrlType
// bTIsDie 
// bTChaBeSkilled 
// nTChaArea EAreaMask
// nSSkillObjType ESkillObjType
// nSSkillObjHabitat ESkillTarHabitatType
// nSSkillEffType ESkillEffType
// bIsTeammate 
// bIsTeammate 
// bIsSelf 
//=============================================================================
int	IsRightSkillTar(int nTChaCtrlType, bool bTIsDie, bool bTChaBeSkilled, int nTChaArea,
					  int nSChaCtrlType, int nSSkillObjType, int nSSkillObjHabitat, int nSSkillEffType,
					  bool bIsTeammate, bool bIsFriend, bool bIsSelf)
{
	const auto eTChaCtrlType = static_cast<EChaCtrlType>(nTChaCtrlType);

	bool bTIsPlayer = IsPlyCtrlCha(eTChaCtrlType);

	if (IsNpcCtrlCha(eTChaCtrlType)) // NPC
		return enumESKILL_FAILD_NPC;
	if (!bTChaBeSkilled) // 
		return enumESKILL_FAILD_NOT_SKILLED;

	if (nTChaArea & enumAREA_TYPE_NOT_FIGHT) // 
	{
		if (nSSkillEffType != enumSKILL_EFF_HELPFUL)
			return enumESKILL_FAILD_SAFETY_BELT;
	}
	if (nTChaArea & enumSKILL_TAR_LAND || nTChaArea & enumAREA_TYPE_BRIDGE ) // 
	{
		if (nSSkillObjHabitat == enumSKILL_TAR_SEA)
			return enumESKILL_FAILD_NOT_LAND;
	}
	else if (!(nTChaArea & enumSKILL_TAR_LAND)) // 
	{
		if (nSSkillObjHabitat == enumSKILL_TAR_LAND)
			return enumESKILL_FAILD_NOT_SEA;
	}

	if (!bIsSelf) // 
	{
		if (nSSkillObjType == enumSKILL_TYPE_SELF)
			return enumESKILL_FAILD_ONLY_SELF;
		else if(nSSkillObjType == enumSKILL_TYPE_EXCEPT_SELF) // 
		{
			return enumESKILL_SUCCESS;
		}
	}
	else
	{
		if(nSSkillObjType == enumSKILL_TYPE_EXCEPT_SELF)
			return enumESKILL_FAILD_SELF;
	}

	if (bTIsDie) // 
	{
		if (!bTIsPlayer)
			return enumESKILL_FAILD_ONLY_DIEPLY;
		if( nSSkillObjType != enumSKILL_TYPE_PLAYER_DIE )
			return enumESKILL_FAILD_ONLY_DIEPLY;
	}

	if (eTChaCtrlType == EChaCtrlType::MONS_TREE) // 
	{
		if (nSSkillObjType != enumSKILL_TYPE_TREE)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if (eTChaCtrlType == EChaCtrlType::MONS_MINE) // 
	{
		if (nSSkillObjType != enumSKILL_TYPE_MINE)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if(eTChaCtrlType == EChaCtrlType::MONS_FISH) // 
	{
		if (nSSkillObjType != enumSKILL_TYPE_FISH)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if (eTChaCtrlType == EChaCtrlType::MONS_DBOAT) // 
	{
		if (nSSkillObjType != enumSKILL_TYPE_SALVAGE)
			return enumESKILL_FAILD_ESP_MONS;
	}

	if (nSSkillObjType == enumSKILL_TYPE_REPAIR)
	{
		if (eTChaCtrlType != EChaCtrlType::MONS_REPAIRABLE)
			return enumESKILL_FAILD_ESP_MONS;
	}
	if (nSSkillObjType == enumSKILL_TYPE_TREE)
	{
		if (eTChaCtrlType != EChaCtrlType::MONS_TREE)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if (nSSkillObjType == enumSKILL_TYPE_MINE)
	{
		if (eTChaCtrlType != EChaCtrlType::MONS_MINE)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if(nSSkillObjType == enumSKILL_TYPE_FISH)
	{
		if (eTChaCtrlType != EChaCtrlType::MONS_FISH)
			return enumESKILL_FAILD_ESP_MONS;
	}
	else if (nSSkillObjType == enumSKILL_TYPE_SALVAGE)
	{
		if (eTChaCtrlType != EChaCtrlType::MONS_DBOAT)
			return enumESKILL_FAILD_ESP_MONS;
	}

	if(nSSkillObjType == enumSKILL_TYPE_ENEMY)
	{
		if (bIsFriend)
			return enumESKILL_FAILD_FRIEND;
	}
	else if(nSSkillObjType == enumSKILL_TYPE_TEAM )
	{
		if (!bIsTeammate)
			return enumESKILL_FAILD_NOT_FRIEND;
	}
	else if(nSSkillObjType == enumSKILL_TYPE_ALL )
	{
		if (nSSkillEffType == enumSKILL_EFF_HELPFUL) // 
		{
			if (!bIsFriend)
				return enumESKILL_FAILD_NOT_FRIEND;
		}
		else
		{
			if (bIsFriend)
				return enumESKILL_FAILD_FRIEND;
		}
	}

	return enumESKILL_SUCCESS;
}

//------------------------------------------------------------------------
//	Перенесённые из CommFunc.h inline-функции
//------------------------------------------------------------------------

bool IsSea(unsigned short usAreaMask)
{
	return !(usAreaMask & enumAREA_TYPE_LAND);
}

bool IsLand(unsigned short usAreaMask)
{
	return (usAreaMask & enumAREA_TYPE_LAND) || (usAreaMask & enumAREA_TYPE_BRIDGE);
}

int IsUseSkill(stNetChangeChaPart* pSEquip, CSkillRecord* p)
{
	if (!p) return -1;
	return IsUseSkill(pSEquip, p->Id);
}

int IsUseSeaLiveSkill(long lFitNo, CSkillRecord* p)
{
	if (!p) return -1;

	for (int i = 0; i < defSKILL_ITEM_NEED_NUM; i++) {
		if (p->sItemNeed[0][i][0] == cchSkillRecordKeyValue)
			break;

		if (p->sItemNeed[0][i][0] == enumSKILL_ITEM_NEED_ID) {
			if (p->sItemNeed[0][i][1] == lFitNo)
				return 1;
		}
	}
	return 0;
}

bool IsPlyCtrlCha(EChaCtrlType eChaCtrlType)
{
	return eChaCtrlType == EChaCtrlType::PLAYER
		|| eChaCtrlType == EChaCtrlType::PLAYER_PET;
}

bool IsMonsCtrlCha(EChaCtrlType eChaCtrlType)
{
	return eChaCtrlType == EChaCtrlType::MONS
		|| eChaCtrlType == EChaCtrlType::MONS_TREE
		|| eChaCtrlType == EChaCtrlType::MONS_MINE
		|| eChaCtrlType == EChaCtrlType::MONS_FISH
		|| eChaCtrlType == EChaCtrlType::MONS_DBOAT
		|| eChaCtrlType == EChaCtrlType::MONS_REPAIRABLE;
}

bool IsNpcCtrlCha(EChaCtrlType eChaCtrlType)
{
	return eChaCtrlType == EChaCtrlType::NPC
		|| eChaCtrlType == EChaCtrlType::NPC_EVENT;
}

bool IsChaEnemyCtrlSide(EChaCtrlType eSCtrlType, EChaCtrlType eTCtrlType)
{
	if (IsPlyCtrlCha(eSCtrlType) && IsPlyCtrlCha(eTCtrlType))
		return false;
	if (IsMonsCtrlCha(eSCtrlType) && IsMonsCtrlCha(eTCtrlType))
		return false;
	return true;
}

long ConvItemAttrTypeToCha(long lItemAttrType)
{
	if (lItemAttrType >= ITEMATTR_COE_STR && lItemAttrType <= ITEMATTR_COE_PDEF)
		return lItemAttrType + (ATTR_ITEMC_STR - ITEMATTR_COE_STR);
	if (lItemAttrType >= ITEMATTR_VAL_STR && lItemAttrType <= ITEMATTR_VAL_PDEF)
		return lItemAttrType + (ATTR_ITEMV_STR - ITEMATTR_VAL_STR);
	return 0;
}

short GetRangeParamNum(char RangeType)
{
	short sParamNum = 0;
	switch (RangeType) {
	case enumRANGE_TYPE_STICK:
	case enumRANGE_TYPE_FAN:
		sParamNum = 2;
		break;
	case enumRANGE_TYPE_SQUARE:
	case enumRANGE_TYPE_CIRCLE:
		sParamNum = 1;
		break;
	}
	return sParamNum + 1;
}

bool IsMoveAble(EChaCtrlType eChaCtrlType, char chChaTerrType, unsigned short usAreaMask)
{
	bool terrainOk = false;
	if (chChaTerrType == defCHA_TERRITORY_DISCRETIONAL) {
		terrainOk = true;
	}
	else if (chChaTerrType == defCHA_TERRITORY_LAND) {
		if ((usAreaMask & enumAREA_TYPE_LAND) || (usAreaMask & enumAREA_TYPE_BRIDGE))
			terrainOk = true;
	}
	else if (chChaTerrType == defCHA_TERRITORY_SEA) {
		if (!(usAreaMask & enumAREA_TYPE_LAND))
			terrainOk = true;
	}

	// В мирной зоне (NOT_FIGHT) монстры не могут двигаться — блокировка агро.
	bool fightOk = true;
	if (usAreaMask & enumAREA_TYPE_NOT_FIGHT) {
		if (IsMonsCtrlCha(eChaCtrlType))
			fightOk = false;
	}

	return terrainOk && fightOk;
}

const char* GetJobName(short sJobID)
{
	if (sJobID < 0 || sJobID >= MAX_JOB_TYPE)
		return g_szJobName[0];
	return g_szJobName[sJobID];
}

short GetJobId(std::string_view szJobName)
{
	for (short i = 0; i < MAX_JOB_TYPE; i++) {
		if (szJobName == g_szJobName[i])
			return i;
	}
	return 0;
}

const char* GetCityName(short sCityID)
{
	if (sCityID < 0 || sCityID >= defMAX_CITY_NUM)
		return "";
	return g_szCityName[sCityID];
}

short GetCityId(std::string_view szCityName)
{
	for (short i = 0; i < defMAX_CITY_NUM; i++) {
		if (szCityName == g_szCityName[i])
			return i;
	}
	return -1;
}

bool IsSeatPose(int pose)
{
	return pose == 16;
}

bool IsValidFightState(int nState)
{
	return nState < enumFSTATE_TARGET_NO;
}

bool ExistStateIsDie(char chState)
{
	return chState >= enumEXISTS_WITHERING;
}

const char* GetItemAttrExplain(int v)
{
	// COE и VAL диапазоны параллельны (см. ItemAttrType.h) — нормализуем VAL→COE,
	// чтобы убрать ранее существовавшее дублирование 21 case'а.
	if (v >= ITEMATTR_VAL_STR && v <= ITEMATTR_VAL_COL) {
		v = v - ITEMATTR_VAL_STR + ITEMATTR_COE_STR;
	}

	switch (v) {
	case ITEMATTR_COE_STR:    return "Strength Bonus";
	case ITEMATTR_COE_AGI:    return "Agility Bonus";
	case ITEMATTR_COE_DEX:    return "Accuracy Bonus";
	case ITEMATTR_COE_CON:    return "Constitution Bonus";
	case ITEMATTR_COE_STA:    return "Spirit Bonus";
	case ITEMATTR_COE_LUK:    return "Luck Bonus";
	case ITEMATTR_COE_ASPD:   return "Attack Speed Bonus";
	case ITEMATTR_COE_ADIS:   return "Attack Range Bonus";
	case ITEMATTR_COE_MNATK:  return "Minimum Attack Bonus";
	case ITEMATTR_COE_MXATK:  return "Maximum Attack Bonus";
	case ITEMATTR_COE_DEF:    return "Defense Bonus";
	case ITEMATTR_COE_MXHP:   return "Maximum HP Bonus";
	case ITEMATTR_COE_MXSP:   return "Maximum SP Bonus";
	case ITEMATTR_COE_FLEE:   return "Dodge Rate Bonus";
	case ITEMATTR_COE_HIT:    return "Hit Rate Bonus";
	case ITEMATTR_COE_CRT:    return "Critical Hitrate Bonus";
	case ITEMATTR_COE_MF:     return "Drop Rate Bonus";
	case ITEMATTR_COE_HREC:   return "HP Recovery Speed Bonus";
	case ITEMATTR_COE_SREC:   return "SP Recovery Speed Bonus";
	case ITEMATTR_COE_MSPD:   return "Movement Speed Bonus";
	case ITEMATTR_COE_COL:    return "Material Mining Speed Bonus";
	case ITEMATTR_VAL_PDEF:   return "Physical Resist Bonus";
	case ITEMATTR_MAXURE:     return "Max Durability";
	case ITEMATTR_MAXENERGY:  return "Max Energy";
	default:                  return "Unknown tools characteristics";
	}
}

bool IsAlphanumeric(std::string_view text)
{
	return std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isalnum(c) != 0; });
}

bool IsNumeric(std::string_view text)
{
	return std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

bool IsValidName(std::string_view name)
{
	// Legacy GBK-проверка имени: latin/digit разрешены, multi-byte (старший бит) парами,
	// 0xA1 0xA1 (двойной "пустой" символ GBK) — запрещён.
	// TODO: после UTF-8 миграции переписать через Corsairs::Util::IsUtf8StartByte —
	// текущая логика ловит UTF-8 continuation-байты как валидные части пары, что почти
	// работает, но запрещает имена с continuation-байтом <=0x80 (которых не бывает в
	// валидном UTF-8, так что в практике безопасно, но логика concept-mismatch).
	bool inMultiByte = false;
	for (std::size_t i = 0; i < name.size(); i++) {
		if (!name[i]) {
			return false;
		}

		if (inMultiByte) {
			if (name[i - 1] == 0xA1 && name[i] == 0xA1) {
				return false;
			}
			if (name[i] > 0x3F && name[i] < 0xFF && name[i] != 0x7F) {
				inMultiByte = false;
			}
			else {
				return false;
			}
		}
		else if (name[i] > 0x80 && name[i] < 0xFF) {
			inMultiByte = true;
		}
		else if ((name[i] >= 'A' && name[i] <= 'Z')
			|| (name[i] >= 'a' && name[i] <= 'z')
			|| (name[i] >= '0' && name[i] <= '9')) {
			// допустимый ASCII-символ
		}
		else {
			return false;
		}
	}
	return !inMultiByte;
}

const char* GetUseItemFailedInfo(short sErrorID)
{
	switch (sErrorID) {
	case enumITEMOPT_SUCCESS:           return "Item operation succesful";
	case enumITEMOPT_ERROR_NONE:        return "Equipment does not exist";
	case enumITEMOPT_ERROR_KBFULL:      return "Inventory is full";
	case enumITEMOPT_ERROR_UNUSE:       return "Failed to use item";
	case enumITEMOPT_ERROR_UNPICKUP:    return "Rl??EC?";
	case enumITEMOPT_ERROR_UNTHROW:     return "Item cannot be thrown";
	case enumITEMOPT_ERROR_UNDEL:       return "Item cannot be destroyed";
	case enumITEMOPT_ERROR_KBLOCK:      return "inventory is currently locked";
	case enumITEMOPT_ERROR_DISTANCE:    return "Distance too far";
	case enumITEMOPT_ERROR_EQUIPLV:     return "Equipment level mismatch";
	case enumITEMOPT_ERROR_EQUIPJOB:    return "Does not meet the class requirement for the equipment";
	case enumITEMOPT_ERROR_STATE:       return "Unable to operate items under the current condition";
	case enumITEMOPT_ERROR_PROTECT:     return "Item is being protected";
	case enumITEMOPT_ERROR_AREA:        return "different region type";
	case enumITEMOPT_ERROR_BODY:        return "type of build does not match";
	case enumITEMOPT_ERROR_TYPE:        return "Unable to store this item";
	case enumITEMOPT_ERROR_INVALID:     return "Item not in used";
	case enumITEMOPT_ERROR_KBRANGE:     return "out of inventory range";
	case enumITEMOPT_ERROR_EXPIRATION:  return "This item is expired.";
	default:                            return "Unknown item usage failure code";
	}
}

// CTextFilter — реализация перенесена в Libraries/Util/src/Text/TextFilter.cpp
// в рамках Ф8 (2026-05).

void String2Item(std::string_view pszData, SItemGrid* SGridCont) {
	const short csSubNum = 8 + enumITEMDBP_MAXNUM + defITEM_INSTANCE_ATTR_NUM_VER110 * 2 + 1;
	std::string strSubList[csSubNum];

	int sTCount = 0;
	Corsairs::Util::ResolveTextLine(pszData, strSubList, csSubNum, ',');
	SGridCont->sID = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->sNum = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->sEndure[0] = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->sEndure[1] = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->sEnergy[0] = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->sEnergy[1] = Corsairs::Util::Str2Int(strSubList[sTCount++]);
	SGridCont->chForgeLv = Corsairs::Util::Str2Int(strSubList[sTCount++]);


	for (int m = 0; m < enumITEMDBP_MAXNUM; m++)
	{
		SGridCont->SetDBParam(m, Corsairs::Util::Str2Int(strSubList[sTCount++]));
	}
	if (Corsairs::Util::Str2Int(strSubList[sTCount++]) > 0) // 
	{
		for (int k = 0; k < defITEM_INSTANCE_ATTR_NUM; k++)
		{
			SGridCont->sInstAttr[k][0] = Corsairs::Util::Str2Int(strSubList[sTCount + k * 2]);
			SGridCont->sInstAttr[k][1] = Corsairs::Util::Str2Int(strSubList[sTCount + k * 2 + 1]);
		}
	}
	else
		SGridCont->SetInstAttrInvalid();
}


//  :    
static std::vector<std::string_view> SplitView(std::string_view sv, char delim)
{
	std::vector<std::string_view> result;
	while (!sv.empty())
	{
		auto pos = sv.find(delim);
		if (pos == std::string_view::npos)
		{
			result.push_back(sv);
			break;
		}
		result.push_back(sv.substr(0, pos));
		sv.remove_prefix(pos + 1);
	}
	return result;
}

static int ParseInt(std::string_view sv)
{
	int value = 0;
	std::from_chars(sv.data(), sv.data() + sv.size(), value);
	return value;
}

static long ParseLong(std::string_view sv)
{
	long value = 0;
	std::from_chars(sv.data(), sv.data() + sv.size(), value);
	return value;
}

bool LookData2String(const stNetChangeChaPart &pLook, std::string &strData)
{
	int64_t checkSum = 0;
	strData.clear();
	strData.reserve(4096);

	// 
	strData += "112#";

	// TypeID, HairID
	strData += std::to_string(pLook.sTypeID);
	strData += ',';
	strData += std::to_string(pLook.sHairID);
	checkSum += pLook.sTypeID + pLook.sHairID;

	// 34  
	for (int i = 0; i < enumEQUIP_NUM; i++)
	{
		const auto &item = pLook.SLink[i];
		strData += ';';

		// expiration, bItemTradable, bIsLock, sNeedLv, dwDBID, sID, sNum,
		// sEndure[0], sEndure[1], sEnergy[0], sEnergy[1], chForgeLv
		strData += std::to_string(item.expiration);
		strData += ','; strData += std::to_string(static_cast<int>(item.bItemTradable));
		strData += ','; strData += std::to_string(static_cast<int>(item.bIsLock));
		strData += ','; strData += std::to_string(item.sNeedLv);
		strData += ','; strData += std::to_string(item.dwDBID);
		strData += ','; strData += std::to_string(item.sID);
		strData += ','; strData += std::to_string(item.sNum);
		strData += ','; strData += std::to_string(item.sEndure[0]);
		strData += ','; strData += std::to_string(item.sEndure[1]);
		strData += ','; strData += std::to_string(item.sEnergy[0]);
		strData += ','; strData += std::to_string(item.sEnergy[1]);
		strData += ','; strData += std::to_string(static_cast<int>(item.chForgeLv));

		checkSum += item.bItemTradable + item.bIsLock + item.sNeedLv + item.dwDBID
			+ item.sID + item.sNum + item.sEndure[0] + item.sEndure[1]
			+ item.sEnergy[0] + item.sEnergy[1] + item.chForgeLv;

		// DB 
		for (int m = 0; m < enumITEMDBP_MAXNUM; m++)
		{
			strData += ','; strData += std::to_string(item.GetDBParam(m));
			checkSum += item.GetDBParam(m);
		}

		// Instance-
		if (item.IsInstAttrValid())
		{
			strData += ",1";
			for (int k = 0; k < defITEM_INSTANCE_ATTR_NUM; k++)
			{
				strData += ','; strData += std::to_string(item.sInstAttr[k][0]);
				strData += ','; strData += std::to_string(item.sInstAttr[k][1]);
				checkSum += item.sInstAttr[k][0] + item.sInstAttr[k][1];
			}
		}
		else
		{
			strData += ",0";
		}
	}

	//  
	strData += ';';
	strData += std::to_string(checkSum);

	return true;
}

bool String2LookData(stNetChangeChaPart &pLook, const std::string &strData)
{
	if (strData.empty())
		return false;

	int64_t checkSum = 0;

	//  : "version#data"   "data"
	int version = 0;
	std::string_view dataView(strData);
	auto hashPos = dataView.find('#');
	if (hashPos != std::string_view::npos)
	{
		version = ParseInt(dataView.substr(0, hashPos));
		dataView.remove_prefix(hashPos + 1);
	}

	//   ';': section[0]=header, section[1..34]=slots, section[35]=checksum
	auto sections = SplitView(dataView, ';');

	//  F# : "typeID;hairID;faceID" (2-3 ,    header)
	if (sections.size() < static_cast<size_t>(enumEQUIP_NUM) + 1)
	{
		if (sections.size() >= 2 && sections[0].find(',') == std::string_view::npos)
		{
			pLook.sTypeID = static_cast<short>(ParseInt(sections[0]));
			pLook.sHairID = static_cast<short>(ParseInt(sections[1]));
			for (auto &item : pLook.SLink)
				item = SItemGrid();
			return true;
		}
		return false;
	}

	// Section 0: "typeID,hairID"
	auto headerFields = SplitView(sections[0], ',');
	if (headerFields.size() < 2)
		return false;
	pLook.sTypeID = static_cast<short>(ParseInt(headerFields[0]));
	pLook.sHairID = static_cast<short>(ParseInt(headerFields[1]));
	checkSum += pLook.sTypeID + pLook.sHairID;

	// Sections 1..34:  
	for (int i = 0; i < enumEQUIP_NUM; i++)
	{
		auto fields = SplitView(sections[i + 1], ',');
		int idx = 0;
		auto &item = pLook.SLink[i];

		auto nextInt = [&]() -> int {
			return (idx < static_cast<int>(fields.size())) ? ParseInt(fields[idx++]) : 0;
		};
		auto nextLong = [&]() -> long {
			return (idx < static_cast<int>(fields.size())) ? ParseLong(fields[idx++]) : 0L;
		};

		item.expiration = nextLong();
		item.bItemTradable = nextInt();

		if (version == 112)
		{
			item.bIsLock = nextInt();
			item.sNeedLv = nextInt();
			item.dwDBID = nextInt();
		}

		item.sID = static_cast<short>(nextInt());
		item.sNum = static_cast<short>(nextInt());
		item.sEndure[0] = static_cast<short>(nextInt());
		item.sEndure[1] = static_cast<short>(nextInt());
		item.sEnergy[0] = static_cast<short>(nextInt());
		item.sEnergy[1] = static_cast<short>(nextInt());
		item.chForgeLv = static_cast<char>(nextInt());

		if (version == 112)
		{
			checkSum += item.bIsLock + item.sNeedLv + item.dwDBID
				+ item.sID + item.sNum + item.sEndure[0] + item.sEndure[1]
				+ item.sEnergy[0] + item.sEnergy[1] + item.chForgeLv;
		}
		else
		{
			checkSum += item.sID + item.sNum + item.sEndure[0] + item.sEndure[1]
				+ item.sEnergy[0] + item.sEnergy[1] + item.chForgeLv;
		}
		checkSum += item.bItemTradable;

		// DB 
		for (int m = 0; m < enumITEMDBP_MAXNUM; m++)
		{
			item.SetDBParam(m, nextLong());
			checkSum += item.GetDBParam(m);
		}

		// Instance-
		bool hasInstAttr = version >= defLOOK_CUR_VER && hashPos != std::string_view::npos;
		if (hasInstAttr)
		{
			if (nextInt() > 0)
			{
				for (int k = 0; k < defITEM_INSTANCE_ATTR_NUM; k++)
				{
					item.sInstAttr[k][0] = static_cast<short>(nextInt());
					item.sInstAttr[k][1] = static_cast<short>(nextInt());
					checkSum += item.sInstAttr[k][0] + item.sInstAttr[k][1];
				}
			}
			else
			{
				item.SetInstAttrInvalid();
			}
		}
		else
		{
			for (int k = 0; k < defITEM_INSTANCE_ATTR_NUM; k++)
			{
				item.sInstAttr[k][0] = static_cast<short>(nextInt());
				item.sInstAttr[k][1] = static_cast<short>(nextInt());
				checkSum += item.sInstAttr[k][0] + item.sInstAttr[k][1];
			}
		}
	}

	//    (     )
	if (hashPos != std::string_view::npos)
	{
		auto checksumIdx = static_cast<size_t>(enumEQUIP_NUM) + 1;
		if (checksumIdx < sections.size())
		{
			int64_t expected = 0;
			std::from_chars(sections[checksumIdx].data(),
				sections[checksumIdx].data() + sections[checksumIdx].size(), expected);
			if (expected != checkSum)
				return false;
		}
	}
	else
	{
		pLook.sVer = defLOOK_CUR_VER;
	}

	return true;
}

// 
char* ShortcutData2String(const stNetShortCut *pShortcut, char *szShortcutBuf, int nLen)
{
	if (!pShortcut || !szShortcutBuf) return NULL;

	char	szData[512];
	int		nBufLen = 0, nDataLen;
	szShortcutBuf[0] = '\0';

	for (int i = 0; i < SHORT_CUT_NUM; i++)
	{
		sprintf(szData, "%d,%d;", pShortcut->chType[i], pShortcut->byGridID[i]);
		nDataLen = (int)strlen(szData);
		if (nBufLen + nDataLen >= nLen) return NULL;
		strcat(szShortcutBuf, szData);
		nBufLen += nDataLen;
	}

	return szShortcutBuf;
}

// 
bool String2ShortcutData(stNetShortCut *pShortcut, std::string &strData)
{
	if (!pShortcut)
		return false;

	std::string strList[SHORT_CUT_NUM + 1];
	const short csSubNum = 2;
	std::string strSubList[csSubNum];
	Corsairs::Util::ResolveTextLine(strData.c_str(), strList, SHORT_CUT_NUM + 1, ';');
	for (int i = 0; i < SHORT_CUT_NUM; i++)
	{
		Corsairs::Util::ResolveTextLine(strList[i].c_str(), strSubList, csSubNum, ',');
		pShortcut->chType[i] = Corsairs::Util::Str2Int(strSubList[0]);
		pShortcut->byGridID[i] = Corsairs::Util::Str2Int(strSubList[1]);
	}

	return true;
}

bool KitbagStringConv(short sKbCapacity, std::string &strData)
{
	int	nInsertPos = 0;
	if (strData == "")
		return true;
	if ((int)strlen(strData.c_str()) < nInsertPos)
		return true;
	char szCap[10];
	sprintf(szCap, "%d@", sKbCapacity);
	strData.insert(nInsertPos, szCap);
	return true;
}
