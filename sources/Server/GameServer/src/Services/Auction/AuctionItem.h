//add by ALLEN 2007-10-19
#pragma once

#include "Character/Character.h"
#include "Player/Player.h"
#include <string>
using namespace std;

class CAuctionItem
{
public:
	CAuctionItem(short sItemID, const string& strName, short sCount, std::uint32_t nBasePrice, std::uint32_t nMinBid);
	~CAuctionItem();

	std::uint32_t GetBasePrice() { return m_nBasePrice; }
	void SetBasePrice(std::uint32_t price) { m_nBasePrice = price; }

	std::uint32_t GetMinBid() { return m_nMinBid; }
	void SetMinbid(std::uint32_t price) { m_nMinBid = price; }

	std::uint32_t GetCurPrice() { return m_nCurPrice; }
	void SetCurPrice(std::uint32_t price) { m_nCurPrice = price; }

	DWORD GetCurChaID() { return m_dwCurChaID; }
	void SetCurChaID(DWORD id) { m_dwCurChaID = id; }

	short GetItemID() { return m_sItemID; }
	void SetItemID(short id) { m_sItemID = id; }

	short GetItemCount() { return m_sCount; }
	void SetItemCount(short sCount) { m_sCount = sCount; }

	string GetCurChaName() { return m_strCurChaName; }
	void SetCurChaName(string strName) { m_strCurChaName = strName; }

	string GetName() { return m_strName; }
	void SetName(string strName) { m_strName = strName; }

	BOOL BidUp(CCharacter *pCha, std::uint32_t price);

private:
	std::uint32_t m_nBasePrice;
	std::uint32_t m_nMinBid;
	std::uint32_t m_nCurPrice;
	DWORD m_dwCurChaID;
	string m_strCurChaName;
	string m_strName;

	short m_sItemID;
	short m_sCount;

};
