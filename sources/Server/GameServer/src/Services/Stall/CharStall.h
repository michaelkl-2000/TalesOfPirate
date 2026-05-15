// CharStall.h Created by knight-gongjian 2005.8.29.
//---------------------------------------------------------
#pragma once

#ifndef _CHARSTALL_H_
#define _CHARSTALL_H_

#include "Character/Character.h"
//---------------------------------------------------------

namespace Corsairs::Common::Mission
{
	typedef struct _STALL_GOODS
	{
		DWORD dwMoney;
		BYTE byGrid;
		BYTE byIndex;
		BYTE byCount;
		USHORT sItemID;

	} STALL_GOODS, *PSTALL_GOODS;

	class CStallSystem;
	class CStallData
	{
		friend CStallSystem;
	public:
		CStallData() { Clear(); }
		~CStallData() = default;

	private:
		void Clear();

		BYTE m_byNum;
		char m_szName[ROLE_MAXNUM_STALL_NUM];
		STALL_GOODS m_Goods[ROLE_MAXNUM_STALL_GOODS];

	};

	class CStallSystem
	{
	public:
		CStallSystem();
		~CStallSystem();

		void StartStall( CCharacter& staller, const Corsairs::Net::Msg::CmStallInfoMessage& msg );
		void CloseStall( CCharacter& staller );
		void OpenStall( CCharacter& character, const Corsairs::Net::Msg::CmStallOpenMessage& msg );
		void BuyGoods( CCharacter& character, const Corsairs::Net::Msg::CmStallBuyMessage& msg );
		void SearchItem(CCharacter& character, int itemID);
	private:
		void SyncData( CCharacter& character, CCharacter& staller );
		void DelGoods( CCharacter& staller, BYTE byGrid, BYTE byCount );

	};
}

extern Corsairs::Common::Mission::CStallSystem g_StallSystem;

//---------------------------------------------------------
#endif // _CHARSTALL_H_
