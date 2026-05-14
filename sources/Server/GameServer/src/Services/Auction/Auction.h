//add by ALLEN 2007-10-19
#pragma once

#include "Services/Auction/AuctionItem.h"
class CAuctionSystem
{
public:
	CAuctionSystem();
	~CAuctionSystem();

	BOOL StartAuction(short sItemID, const std::string& strName, short sCount, std::uint32_t nBasePrice, std::uint32_t nMinBid);
	BOOL EndAuction(short sItemID);
	void ListAuction(CCharacter* pCha, CCharacter* pNpc);
	void NotifyAuction( CCharacter* pCha, CCharacter* pNpc );
	BOOL BidUp(CCharacter *pCha, short sItemID, std::uint32_t price);

private:
	std::map<short, CAuctionItem *> m_mapItemList;

};

extern CAuctionSystem g_AuctionSystem;
