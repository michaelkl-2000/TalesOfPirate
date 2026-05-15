#include "StdAfx.h"
#include "PacketCmd.h"

#include "GameApp.h"
#include "Character.h"
#include "actor.h"
#include "procirculate.h"
#include "UIBoothForm.h"
#include "UIStoreForm.h"
#include "CommandMessages.h"
#include "CryptoUtils.h"
using namespace Corsairs::Client::Crypto;
//------------------------
// C->S :
//------------------------
bool CS_Connect(std::string_view hostname, uint16_t port, uint32_t timeout) {
	std::string host{hostname};
	g_logManager.InternalLog(LogLevel::Debug, "connections", SafeVFormat(GetLanguageString(294), host.c_str()));
	if (g_NetIF->m_pCProCir) {
		delete g_NetIF->m_pCProCir;
	}
	g_NetIF->m_pCProCir = new CProCirculateCS(g_NetIF);

	discordInit();
	updateDiscordPresence("Connecting...", "");


	return g_NetIF->m_pCProCir->Connect(host.c_str(), port, timeout);
}

//------------------------
// C->S : 
//------------------------
void CS_Disconnect(int reason) {
	//fix stall bugs with offline stalls @mothannakh
	g_stUIBooth.PullBoothSuccess();
	g_NetIF->m_pCProCir->Disconnect(reason);
}

void CS_SendPrivateKey() {
	g_NetIF->m_pCProCir->SendPrivateKey();
}


//------------------------
// C->S : 
//------------------------
void CS_Login(const char* accounts, const char* password, const char* passport) {
	g_NetIF->m_pCProCir->Login(accounts, password, passport);
}

//------------------------
// C->S : 
//------------------------
void CS_Logout() {
	//fix stall bugs with offline stalls @mothannakh
	g_stUIBooth.PullBoothSuccess();
	g_NetIF->m_pCProCir->Logout();
	return;
}

void CS_OfflineMode() {
	auto pk = Corsairs::Net::Msg::serializeCmOfflineModeCmd();
	g_NetIF->SendPacketMessage(pk);
	//fix stall bugs with offline stalls @mothannakh
	g_stUIBooth.PullBoothSuccess();
}

void CS_CancelExit() {
	//    
	auto pk = Corsairs::Net::Msg::serializeCmCancelExitCmd();
	g_NetIF->SendPacketMessage(pk);
}

//------------------------
// C->S : 
//------------------------
void CS_BeginPlay(char cha_index) {
	g_NetIF->m_pCProCir->BeginPlay(cha_index);
}

//------------------------
// C->S : 
//------------------------
void CS_EndPlay() {
	//fix stall bugs with offline stalls @mothannakh
	g_stUIBooth.PullBoothSuccess();
	g_NetIF->m_pCProCir->EndPlay();
}

//------------------------
// C->S : 
//------------------------

void CS_NewCha(const char* chaname, const char* birth, int type, int hair, int face) {
	g_NetIF->m_pCProCir->NewCha(chaname, birth, type, hair, face);
}

//------------------------
// C->S : 
//------------------------
void CS_DelCha(uint8_t cha_index, const char szPassword2[]) {
	g_NetIF->m_pCProCir->DelCha(cha_index, szPassword2);
}

//------------------------
// C->S : 
//------------------------
void CS_Say(const char* content) {
	g_NetIF->m_pCProCir->Say(content);
}

void CS_GuildBankOper(stNetBank* pNetBank) {
	if (g_pGameApp->GetCurScene()->GetMainCha()->IsBoat()) {
		g_pGameApp->MsgBox("Not available at sea");
		return;
	}

	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildBankOperMessage{
		0, pNetBank->chSrcType, pNetBank->sSrcID, pNetBank->sSrcNum, pNetBank->chTarType, pNetBank->sTarID
	});
	g_NetIF->SendPacketMessage(pk);
}

void CS_GuildBankTakeGold(int gold) {
	if (g_pGameApp->GetCurScene()->GetMainCha()->IsBoat()) {
		g_pGameApp->MsgBox("Not available at sea");
		return;
	}
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildBankGoldMessage{1, 0, gold});
	g_NetIF->SendPacketMessage(pk);
}

void CS_GuildBankGiveGold(int gold) {
	if (g_pGameApp->GetCurScene()->GetMainCha()->IsBoat()) {
		g_pGameApp->MsgBox("Not available at sea");
		return;
	}
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildBankGoldMessage{1, 1, gold});
	g_NetIF->SendPacketMessage(pk);
}

void CS_ChangePass(const char* pass, const char* pin) {
	//TODO(Ogge): Might need to hash it using blake2s?
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmChangePassMessage{pass, pin});
	g_NetIF->SendPacketMessage(pk);
}

void CS_Register(const char* user, const char* pass, const char* email) {
	/*
	WPacket pk	=g_NetIF->GetWPacket();
	pk.WriteCmd(CMD_CM_REGISTER);
	pk.WriteString(user);
	
	pk.WriteString(HashPassword(pass));
	pk.WriteString(email);
	g_NetIF->SendPacketMessage(pk);
	*/
}

void CS_StallSearch(int ItemID) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStallSearchMessage{ItemID});
	g_NetIF->SendPacketMessage(pk);
}

//   
void CS_CreatePassword2(const char szPassword[]) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCreatePassword2Message{szPassword});
	g_NetIF->SendPacketMessage(pk);
}

//   
void CS_UpdatePassword2(const char szOld[], const char szPassword[]) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmUpdatePassword2Message{szOld, szPassword});
	g_NetIF->SendPacketMessage(pk);
}

//
void CS_LockKitbag() {
	auto pk = Corsairs::Net::Msg::serializeCmKitbagLockCmd();
	g_NetIF->SendPacketMessage(pk);
}

//
void CS_UnlockKitbag(const char szPassword[]) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmKitbagUnlockMessage{szPassword});
	g_NetIF->SendPacketMessage(pk);
}

//
void CS_KitbagCheck() {
	auto pk = Corsairs::Net::Msg::serializeCmKitbagCheckCmd();
	g_NetIF->SendPacketMessage(pk);
}

// 
void CS_AutoKitbagLock(bool bAutoLock) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmAutoKitbagLockMessage{bAutoLock ? 1 : 0});
	g_NetIF->SendPacketMessage(pk);
}

// C->S : 
void CS_BeginAction(CCharacter* pCha, DWORD type, void* param, CActionState* pState) {
	g_NetIF->m_pCProCir->BeginAction(pCha, type, param, pState);
}

// C->S : 
void CS_EndAction(CActionState* pState) {
	g_NetIF->m_pCProCir->EndAction(pState);
}

// C->S : 
void CS_DieReturn(char chReliveType) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmDieReturnMessage{chReliveType});
	g_NetIF->SendPacketMessage(pk);

	// log
	g_logManager.InternalLog(LogLevel::Debug, "common", std::format("###Send(DieReturn):\tTick:[{}]", GetTickCount()));
	//
}

//----------------------------
//C->S	: Ping
//----------------------------
void CS_SendPing() {
	return;
	//WPacket pk	= g_NetIF->GetWPacket();
	//pk.WriteCmd(CMD_CM_PING);

	//g_NetIF->dwLatencyTime[g_NetIF->m_pingid] = GetTickCount();
	//pk.WriteInt64(g_NetIF->m_pingid);
	//++(g_NetIF->m_pingid);
	//if(g_NetIF->m_pingid>=20) g_NetIF->m_pingid = 0;

	//g_NetIF->SendPacketMessage(pk);
}

void CS_MapMask(const char* szMapName) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMapMaskMessage{szMapName});
	g_NetIF->SendPacketMessage(pk);

	// log
	g_logManager.InternalLog(LogLevel::Debug, "common", std::format("###Send(MapMask):\tTick:[{}]", GetTickCount()));
	//
}

void CS_RequestTalk(DWORD dwNpcID, BYTE byCmd) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmRequestTalkMessage{(int64_t)dwNpcID, (int64_t)byCmd});
	g_NetIF->SendPacketMessage(packet);
}

void CS_SelFunction(DWORD dwNpcID, BYTE byPageID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSelFunctionMessage{
		(int64_t)dwNpcID, (int64_t)byPageID, (int64_t)byIndex
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_Sale(DWORD dwNpcID, BYTE byIndex, BYTE byCount) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSaleMessage{(int64_t)dwNpcID, (int64_t)byIndex, (int64_t)byCount});
	g_NetIF->SendPacketMessage(packet);
}

void CS_Buy(DWORD dwNpcID, BYTE byItemType, BYTE byIndex1, BYTE byIndex2, BYTE byCount) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBuyMessage{
		(int64_t)dwNpcID, (int64_t)byItemType, (int64_t)byIndex1, (int64_t)byIndex2, (int64_t)byCount
	});
	g_NetIF->SendPacketMessage(packet);
}

//   
void CS_SelectTradeBoat(DWORD dwNpcID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSelectTradeBoatMessage{(int64_t)dwNpcID, (int64_t)byIndex});
	g_NetIF->SendPacketMessage(packet);
}

//   NPC ( )
void CS_SaleGoods(DWORD dwNpcID, DWORD dwBoatID, BYTE byIndex, BYTE byCount) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSaleGoodsMessage{
		(int64_t)dwNpcID, (int64_t)dwBoatID, (int64_t)byIndex, (int64_t)byCount
	});
	g_NetIF->SendPacketMessage(packet);
}

//   NPC ( )
void CS_BuyGoods(DWORD dwNpcID, DWORD dwBoatID, BYTE byItemType, BYTE byIndex1, BYTE byIndex2, BYTE byCount) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBuyGoodsMessage{
		(int64_t)dwNpcID, (int64_t)dwBoatID, (int64_t)byItemType, (int64_t)byIndex1, (int64_t)byIndex2, (int64_t)byCount
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_BlackMarketExchangeReq(DWORD dwNpcID, short sSrcID, short sSrcNum, short sTarID, short sTarNum, short sTimeNum,
							   BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBlackMarketExchangeReqMessage{
		(int64_t)dwNpcID, (int64_t)sTimeNum, (int64_t)sSrcID, (int64_t)sSrcNum, (int64_t)sTarID, (int64_t)sTarNum,
		(int64_t)byIndex
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_RequestTrade(TradeCharType byType, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmRequestTradeMessage{byType, dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_AcceptTrade(TradeCharType byType, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmAcceptTradeMessage{byType, dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_AddItem(TradeCharType byType, DWORD dwCharID, TradeOpType byOpType, BYTE byIndex, BYTE byItemIndex, BYTE byCount) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmAddItemMessage{
		byType, dwCharID, byOpType, byIndex, byItemIndex, byCount
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_AddMoney(TradeCharType byType, DWORD dwCharID, TradeOpType byOpType, DWORD dwMoney) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmAddMoneyMessage{byType, dwCharID, byOpType, 0, dwMoney});
	g_NetIF->SendPacketMessage(packet);
}

void CS_AddIMP(TradeCharType byType, DWORD dwCharID, TradeOpType byOpType, DWORD dwMoney) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmAddMoneyMessage{byType, dwCharID, byOpType, 1, dwMoney});
	g_NetIF->SendPacketMessage(packet);
}

void CS_CancelTrade(TradeCharType byType, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCancelTradeMessage{byType, dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_ValidateTradeData(TradeCharType byType, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmValidateTradeDataMessage{byType, dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_ValidateTrade(TradeCharType byType, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmValidateTradeMessage{byType, dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_MissionPage(DWORD dwNpcID, BYTE byCmd, BYTE bySelItem, BYTE byParam) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMissionPageMessage{
		(int64_t)dwNpcID, (int64_t)byCmd, (int64_t)bySelItem, (int64_t)byParam
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_SelMission(DWORD dwNpcID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSelMissionMessage{(int64_t)dwNpcID, (int64_t)byIndex});
	g_NetIF->SendPacketMessage(packet);
}

//    NPC
void CS_MissionTalk(DWORD dwNpcID, BYTE byCmd) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMissionTalkMessage{(int64_t)dwNpcID, (int64_t)byCmd});
	g_NetIF->SendPacketMessage(packet);
}

//     NPC
void CS_SelMissionFunc(DWORD dwNpcID, BYTE byPageID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSelMissionFuncMessage{
		(int64_t)dwNpcID, (int64_t)byPageID, (int64_t)byIndex
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_MisLog() {
	auto packet = Corsairs::Net::Msg::serializeCmMisLogCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_MisLogInfo(WORD wID) {
	if (wID == -1)
		return;
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMisLogInfoMessage{wID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_MisClear(WORD wID) {
	if (wID == -1)
		return;
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMisLogClearMessage{wID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_ForgeItem(BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmForgeItemMessage{byIndex});
	g_NetIF->SendPacketMessage(packet);
}

void CS_CreateBoat(const char szBoat[], char szHeader, char szEngine, char szCannon, char szEquipment) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCreateBoatMessage{szBoat, szHeader, szEngine, szCannon, szEquipment});
	g_NetIF->SendPacketMessage(packet);
}

void CS_SelectBoatList(DWORD dwNpcID, BYTE byType, BYTE byIndex) {
	if (byType == +Corsairs::Common::Mission::BoatListType::BERTH_TRADE_LIST) {
		CS_SelectTradeBoat(dwNpcID, byIndex);
	}
	else {
		auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSelectBoatListMessage{
			(int64_t)dwNpcID, static_cast<Corsairs::Common::Mission::BoatListType>(byType), (int64_t)byIndex
		});
		g_NetIF->SendPacketMessage(packet);
	}
}

void CS_SelectBoat(DWORD dwNpcID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBoatLaunchMessage{(int64_t)dwNpcID, (int64_t)byIndex});
	g_NetIF->SendPacketMessage(packet);
}

void CS_SelectBoatBag(DWORD dwNpcID, BYTE byIndex) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBoatBagSelMessage{(int64_t)dwNpcID, (int64_t)byIndex});
	g_NetIF->SendPacketMessage(packet);
}

void CS_UpdateBoat(char szHeader, char szEngine, char szCannon, char szEquipment) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmUpdateBoatMessage{szHeader, szEngine, szCannon, szEquipment});
	g_NetIF->SendPacketMessage(packet);
}

void CS_CancelBoat() {
	auto packet = Corsairs::Net::Msg::serializeCmBoatCancelCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_GetBoatInfo() {
	auto packet = Corsairs::Net::Msg::serializeCmBoatGetInfoCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_EntityEvent(DWORD dwEntityID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmEntityEventMessage{(int64_t)dwEntityID});
	g_NetIF->SendPacketMessage(packet);

	g_logManager.InternalLog(LogLevel::Debug, "common",
							 std::format("###Send(Event-Entyty):\tTick:[{}]", GetTickCount()));
}

//    ( )
void CS_StallInfo(const char szName[], Corsairs::Common::Mission::NetStallAllData& Data) {
	Corsairs::Net::Msg::CmStallInfoMessage msg;
	msg.name = szName;
	msg.num = Data.Num;
	msg.items.resize(Data.Num);
	for (BYTE i = 0; i < Data.Num; ++i) {
		msg.items[i] = {Data.Info[i].Grid, Data.Info[i].Money, Data.Info[i].Count, Data.Info[i].Index};
	}
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

void CS_StallOpen(DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStallOpenMessage{(int64_t)dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StallBuy(DWORD dwCharID, BYTE byIndex, BYTE byCount, int gridID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStallBuyMessage{
		(int64_t)dwCharID, (int64_t)byIndex, (int64_t)byCount, (int64_t)gridID
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StallClose() {
	auto packet = Corsairs::Net::Msg::serializeCmStallCloseCmd();
	g_NetIF->SendPacketMessage(packet);
}

//add by jilinlee 2007/4/20/////////////////////
void CS_ReadBookStart() {
	auto packet = Corsairs::Net::Msg::serializeCmReadBookStartCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_ReadBookClose() {
	auto packet = Corsairs::Net::Msg::serializeCmReadBookCloseCmd();
	g_NetIF->SendPacketMessage(packet);
}

///////////////////////////////////////////////

void CS_UpdateHair(stNetUpdateHair& stData) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmUpdateHairMessage{
		stData.sScriptID, stData.sGridLoc[0], stData.sGridLoc[1], stData.sGridLoc[2], stData.sGridLoc[3]
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_TeamFightAsk(unsigned long ulWorldID, long lHandle, char chType) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmTeamFightAskMessage{chType, (int64_t)ulWorldID, lHandle});
	g_NetIF->SendPacketMessage(packet);
}

void CS_TeamFightAnswer(bool bAccess) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmTeamFightAnswerMessage{bAccess ? 1 : 0});
	g_NetIF->SendPacketMessage(packet);
}

// lRepairmanID,lRepairmanHandle 
// chPosType12
// chPosID
void CS_ItemRepairAsk(long lRepairmanID, long lRepairmanHandle, char chPosType, char chPosID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemRepairAskMessage{
		lRepairmanID, lRepairmanHandle, chPosType, chPosID
	});
	g_NetIF->SendPacketMessage(packet);
}

void CS_ItemRepairAnswer(bool bAccess) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemRepairAnswerMessage{bAccess ? 1 : 0});
	g_NetIF->SendPacketMessage(packet);
}

//  :   ( )
void CS_ItemForgeAsk(bool bSure, stNetItemForgeAsk* pSForge) {
	Corsairs::Net::Msg::CmItemForgeGroupAskMessage msg;
	msg.sure = bSure ? 1 : 0;
	if (bSure) {
		msg.type = pSForge->chType;
		for (int i = 0; i < defMAX_FORGE_GROUP_NUM; i++) {
			msg.groups[i].cells.resize(pSForge->SGroup[i].sCellNum);
			for (short j = 0; j < pSForge->SGroup[i].sCellNum; j++) {
				msg.groups[i].cells[j] = {pSForge->SGroup[i].pCell->sPosID, pSForge->SGroup[i].pCell->sNum};
			}
		}
	}
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

void CS_ItemForgeAnswer(bool bAccess) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemForgeAnswerMessage{bAccess ? 1 : 0});
	g_NetIF->SendPacketMessage(packet);
}

// Add by lark.li 20080514 begin
void CS_ItemLotteryAnswer(bool bAccess) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemLotteryAnswerMessage{bAccess ? 1 : 0});
	g_NetIF->SendPacketMessage(packet);
}

//  :   ( )
void CS_ItemLotteryAsk(bool bSure, stNetItemLotteryAsk* pSLottery) {
	Corsairs::Net::Msg::CmItemLotteryGroupAskMessage msg;
	msg.sure = bSure ? 1 : 0;
	if (bSure) {
		for (int i = 0; i < defMAX_LOTTERY_GROUP_NUM; i++) {
			msg.groups[i].cells.resize(pSLottery->SGroup[i].sCellNum);
			for (short j = 0; j < pSLottery->SGroup[i].sCellNum; j++) {
				msg.groups[i].cells[j] = {pSLottery->SGroup[i].pCell->sPosID, pSLottery->SGroup[i].pCell->sNum};
			}
		}
	}
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

// End
//   ()
void CS_ItemAmphitheaterAsk(bool bSure, int ReID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemAmphitheaterAskMessage{bSure ? 1 : 0, (int64_t)ReID});
	g_NetIF->SendPacketMessage(packet);
}

//  :   (  )
void CS_ItemForgeAsk(bool bSure, int nType, int arPacketPos[], int nPosCount) {
	Corsairs::Net::Msg::CmItemForgePosAskMessage msg;
	msg.sure = bSure ? 1 : 0;
	if (bSure) {
		msg.type = (int64_t)(char)(nType);
		msg.posCount = (int64_t)nPosCount;
		for (int i = 0; i < nPosCount && i < 6; ++i)
			msg.positions[i] = (int64_t)arPacketPos[i];
	}
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}


// 
void CS_StoreOpenAsk(const char szPassword[]) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStoreOpenAskMessage{szPassword});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StoreClose() {
	auto packet = Corsairs::Net::Msg::serializeCmStoreCloseCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_StoreListAsk(long lClsID, short sPage, short sNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStoreListAskMessage{lClsID, sPage, sNum});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StoreBuyAsk(long lComID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStoreBuyAskMessage{lComID});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StoreChangeAsk(long lNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStoreChangeAskMessage{lNum});
	g_NetIF->SendPacketMessage(packet);
}

void CS_StoreQuery(long lNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmStoreQueryMessage{lNum});
	g_NetIF->SendPacketMessage(packet);
}

//void CS_StoreVIP(short sVipID, short sMonth)
//{
//	WPacket packet = g_NetIF->GetWPacket();
//	packet.WriteCmd( CMD_CM_STORE_VIP );
//	packet.WriteInt64(sVipID);
//	packet.WriteInt64(sMonth);
//	g_NetIF->SendPacketMessage( packet );
//}

void CS_ReportWG(const char szInfo[]) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmReportWgMessage{szInfo});
	g_NetIF->SendPacketMessage(packet);
}

void CS_TigerStart(DWORD dwNpcID, short sSel1, short sSel2, short sSel3) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmTigerStartMessage{(int64_t)dwNpcID, sSel1, sSel2, sSel3});
	g_NetIF->SendPacketMessage(packet);
}

void CS_TigerStop(DWORD dwNpcID, short sNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmTigerStopMessage{(int64_t)dwNpcID, sNum});
	g_NetIF->SendPacketMessage(packet);
}

void CS_RequestDailyBuffInfo() {
	auto packet = Corsairs::Net::Msg::serializeCmDailyBuffRequestCmd();
	g_NetIF->SendPacketMessage(packet);
}


void CS_VolunteerList(short sPage, short sNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmVolunteerListMessage{sPage, sNum});
	g_NetIF->SendPacketMessage(packet);
}

void CS_VolunteerAdd() {
	auto packet = Corsairs::Net::Msg::serializeCmVolunteerAddCmd();
	g_NetIF->SendPacketMessage(packet);
}


void CS_VolunteerDel() {
	auto packet = Corsairs::Net::Msg::serializeCmVolunteerDelCmd();
	g_NetIF->SendPacketMessage(packet);
}

void CS_VolunteerSel(const char* szName) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmVolunteerSelMessage{szName});
	g_NetIF->SendPacketMessage(packet);
}

void CS_VolunteerOpen(short sNum) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmVolunteerOpenMessage{sNum});
	g_NetIF->SendPacketMessage(packet);
}

void CS_VolunteerAsr(BOOL bRet, const char* szName) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmVolunteerAsrMessage{bRet ? 1 : 0, szName});
	g_NetIF->SendPacketMessage(packet);
}

void CS_SyncKitbagTemp() {
	auto packet = Corsairs::Net::Msg::serializeCmKitbagTempSyncCmd();
	g_NetIF->SendPacketMessage(packet);
}

// Add by lark.li 20080707 begin
void CS_CaptainConfirmAsr(short sRet, DWORD dwTeamID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCaptainConfirmAsrMessage{sRet, (int64_t)dwTeamID});
	g_NetIF->SendPacketMessage(packet);
}

// End

//  
void CS_MasterInvite(const char* szName, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMasterInviteMessage{szName, (int64_t)dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

//    
void CS_MasterAsr(short sRet, const char* szName, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMasterAsrMessage{(int64_t)sRet, szName, (int64_t)dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

//  
void CS_PrenticeInvite(const char* szName, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmPrenticeInviteMessage{szName, (int64_t)dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

//    
void CS_PrenticeAsr(short sRet, const char* szName, DWORD dwCharID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmPrenticeAsrMessage{(int64_t)sRet, szName, (int64_t)dwCharID});
	g_NetIF->SendPacketMessage(packet);
}

//  
void CS_MasterDel(const char* szName, std::uint32_t ulChaID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmMasterDelMessage{szName, (int64_t)ulChaID});
	g_NetIF->SendPacketMessage(packet);
}

//  
void CS_PrenticeDel(const char* szName, std::uint32_t ulChaID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmPrenticeDelMessage{szName, (int64_t)ulChaID});
	g_NetIF->SendPacketMessage(packet);
}

//     
void CP_Master_Refresh_Info(unsigned long chaid) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCpMasterRefreshInfoMessage{(int64_t)chaid});
	g_NetIF->SendPacketMessage(pk);
}

//     
void CP_Prentice_Refresh_Info(unsigned long chaid) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCpPrenticeRefreshInfoMessage{(int64_t)chaid});
	g_NetIF->SendPacketMessage(pk);
}

//    
void CS_Say2Camp(const char* szContent) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmSay2CampMessage{szContent});
	g_NetIF->SendPacketMessage(pk);
}

//  GM-
void CS_GMSend(DWORD dwNPCID, const char* szTitle, const char* szContent) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGmSendMessage{(int64_t)dwNPCID, szTitle, szContent});
	g_NetIF->SendPacketMessage(pk);
}

//  GM-
void CS_GMRecv(DWORD dwNPCID) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGmRecvMessage{(int64_t)dwNPCID});
	g_NetIF->SendPacketMessage(pk);
}

//void CS_PKCtrl(bool bCanPK)
//{
//	WPacket l_wpk	=g_NetIF->GetWPacket();
//	l_wpk.WriteCmd(CMD_CM_PK_CTRL);
//	l_wpk.WriteInt64(bCanPK? 1 : 0);
//	g_NetIF->SendPacketMessage(l_wpk);
//}

//    
void CS_CheatCheck(std::string_view answer) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmCheatCheckMessage{std::string{answer}});
	g_NetIF->SendPacketMessage(pk);
}

//  
//void CS_PKSilverSort(DWORD dwNPCID, short sItemPos)
//{
//    WPacket packet = g_NetIF->GetWPacket();
//    packet.WriteCmd(CMD_CM_GARNER2_REORDER);
//    packet.WriteInt64(dwNPCID);
//    packet.WriteInt64(sItemPos);
//    g_NetIF->SendPacketMessage(packet);
//}


//   
void CS_LifeSkill(long type, DWORD dwNPCID) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmLifeSkillMessage{(int64_t)type, (int64_t)dwNPCID});
	g_NetIF->SendPacketMessage(packet);
}


//  :  (Compose)
void CS_Compose(DWORD dwNPCID, int* iPos, int iCount, bool asr /* = false */) {
	Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
	msg.isAnswer = asr;
	msg.skillType = 0;
	msg.npcId = (int64_t)dwNPCID;
	msg.positions.resize(iCount);
	for (int i = 0; i < iCount; i++)
		msg.positions[i] = (int64_t)(short)iPos[i];
	msg.hasExtra = false;
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

//  :  (Break)
void CS_Break(DWORD dwNPCID, int* iPos, int iCount, bool asr /* = false */) {
	Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
	msg.isAnswer = asr;
	msg.skillType = 1;
	msg.npcId = (int64_t)dwNPCID;
	msg.positions.resize(iCount);
	for (int i = 0; i < iCount; i++)
		msg.positions[i] = (int64_t)(short)iPos[i];
	msg.hasExtra = false;
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

//  :  (Found)
void CS_Found(DWORD dwNPCID, int* iPos, int iCount, short big, bool asr /* = false */) {
	Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
	msg.isAnswer = asr;
	msg.skillType = 2;
	msg.npcId = (int64_t)dwNPCID;
	msg.positions.resize(iCount);
	for (int i = 0; i < iCount; i++)
		msg.positions[i] = (int64_t)(short)iPos[i];
	msg.hasExtra = true;
	msg.extraParam = (int64_t)big;
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

//  :  (Cooking)
void CS_Cooking(DWORD dwNPCID, int* iPos, int iCount, short percent, bool asr /* = false */) {
	Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
	msg.isAnswer = asr;
	msg.skillType = 3;
	msg.npcId = (int64_t)dwNPCID;
	msg.positions.resize(iCount);
	for (int i = 0; i < iCount; i++)
		msg.positions[i] = (int64_t)(short)iPos[i];
	msg.hasExtra = true;
	msg.extraParam = (int64_t)percent;
	auto packet = Corsairs::Net::Msg::serialize(msg);
	g_NetIF->SendPacketMessage(packet);
}

//    
void CS_UnlockCharacter() {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmUnlockCharacterMessage{0});
	g_NetIF->SendPacketMessage(packet);
}

//   
void CS_AutionBidup(DWORD dwNPCID, short sItemID, std::uint32_t price) {
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmBidUpMessage{(int64_t)dwNPCID, (int64_t)sItemID, (int64_t)price});
	g_NetIF->SendPacketMessage(packet);
}

//   
void CS_AntiIndulgence_Close() {
	auto packet = Corsairs::Net::Msg::serializeCmAntiIndulgenceCmd();
	g_NetIF->SendPacketMessage(packet);
}

//    
void CS_DropLock(int slot) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemLockAskMessage{(int64_t)slot});
	g_NetIF->SendPacketMessage(pk);
}

//    
void CS_UnlockItem(const char szPassword[], int slot) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmItemUnlockAskMessage{szPassword, (int64_t)slot});
	g_NetIF->SendPacketMessage(pk);
}

//  PIN-
void CS_SendGameRequest(const char szPassword[]) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGameRequestPinMessage{szPassword});
	g_NetIF->SendPacketMessage(pk);
}


//    
void CS_SetGuildPerms(DWORD ID, std::uint32_t Perms) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildPermMessage{(int64_t)ID, (int64_t)Perms});
	g_NetIF->SendPacketMessage(pk);
}

//    
void CS_RequestDropRate() {
	auto pk = Corsairs::Net::Msg::serializeCmRequestDropRateCmd();
	g_NetIF->SendPacketMessage(pk);
}

//    
void CS_RequestExpRate() {
	auto pk = Corsairs::Net::Msg::serializeCmRequestExpRateCmd();
	g_NetIF->SendPacketMessage(pk);
}
