#include "Core/stdafx.h"
#include "World/SubMap.h"
#include "App/GameApp.h"
#include "App/GameAppNet.h"
#include "Services/Trade/CharTrade.h"
#include "NPC/NPC.h"
#include "Script/LuaAPI.h"
#include "NPC/WorldEudemon.h"
#include "Player/Player.h"
#include "Progression/LevelRecord.h"
#include "Services/Forge/CharForge.h"
#include "Character/HairRecord.h"
#include "Db/GameDB.h"

#include "World/Birthplace.h"
#include "Services/Boat/CharBoat.h"
#include "Services/Guild/Guild.h"
#include "Services/Stall/CharStall.h"

#include "Services/Auction/Auction.h"
#include "CommandMessages.h"
extern std::string g_strLogName;

//----------------------------------------------------------
//                    
//----------------------------------------------------------
void CCharacter::ProcessPacket(unsigned short usCmd, Corsairs::Net::RPacket& pk) {
	switch (usCmd) {
	case CMD_CM_RANK: {
		DWORD COOLDOWN = GetTickCount();
		if (ShowRankColD > COOLDOWN) {
			BickerNotice("Please Calm Down Don't Spam! ");
			return;
		}
		game_db.ShowExpRank(*GetPlyMainCha(), 50);
		break;
	}
	case CMD_CM_STALLSEARCH: {
		Corsairs::Net::Msg::CmStallSearchMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_StallSystem.SearchItem(*this, static_cast<std::int32_t>(msg.itemId));
		break;
	}
	case CMD_PM_GUILDBANK: {
		Corsairs::Net::Msg::PmGuildBankMessage gbMsg;
		Corsairs::Net::Msg::deserialize(pk, gbMsg);
		Handle_GuildBankCmd(gbMsg);
		break;
	}

	case CMD_PM_PUSHTOGUILDBANK: {
		std::string strItem = pk.ReadString();
		Handle_PushToGuildBank(strItem);
		break;
	}

	case CMD_CM_PING: {
		Corsairs::Net::Msg::CmPingResponseMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_Ping(msg);
		break;
	}
	case CMD_CM_CHECK_PING: {
		DWORD dwPing = GetTickCount() - m_dwPingSendTick;
		m_dwPing = dwPing;
		SendPreMoveTime();
		break;
	}
	case CMD_CM_CANCELEXIT: {
		CancelExit();
	}
	break;
	case CMD_CM_BEGINACTION: {
		Corsairs::Net::Msg::CmBeginActionMessage actionMsg;
		Corsairs::Net::Msg::deserialize(pk, actionMsg);
		std::uint32_t ulWorldID = static_cast<std::uint32_t>(actionMsg.worldId);

		if (GetPlayer()) {
			if (GetPlayer()->GetCtrlCha() && ulWorldID == GetPlayer()->GetCtrlCha()->GetID())
				GetPlayer()->GetCtrlCha()->BeginAction(actionMsg);
			else if (GetPlayer()->GetMainCha() && ulWorldID == GetPlayer()->GetMainCha()->GetID())
				GetPlayer()->GetMainCha()->BeginAction(actionMsg);
		}
		break;
	}
	case CMD_CM_ENDACTION: {
		EndAction();

		break;
	}
	case CMD_CM_DIE_RETURN: {
		Corsairs::Net::Msg::CmDieReturnMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		m_chSelRelive = static_cast<char>(msg.reliveType);
		GetPlyMainCha()->ResetChaRelive();
		if (m_chSelRelive == enumEPLAYER_RELIVE_NORIGIN)
			SetRelive(enumEPLAYER_RELIVE_ORIGIN, 0);
		break;
	}
	case CMD_CM_SAY: {
		Corsairs::Net::Msg::CmSayMessage sayMsg;
		Corsairs::Net::Msg::deserialize(pk, sayMsg);
		Handle_Say(sayMsg);
		break;
	}
	case CMD_CM_REQUESTTALK:
	case CMD_CM_REQUESTTRADE: {
		const bool bTradeData  = GetTradeData() != nullptr;
		const bool bBoat       = GetBoat() != nullptr;
		const bool bStallData  = GetStallData() != nullptr;
		const bool bTalkCtrl   = GetActControl(ActControl::TALKTO_NPC);
		const bool bKitbagLock = m_CKitbag.IsLock();
		const bool bItemCtrl   = GetActControl(ActControl::ITEM_OPT);
		if (bTradeData || bBoat || bStallData || !bTalkCtrl || bKitbagLock || !bItemCtrl) {
			ToLogService("trade", LogLevel::Error,
				"REQUESTTALK/TRADE rejected for cha={} cmd={}: trade={}, boat={}, stall={}, talkCtrl={}, kitbagLock={}, itemCtrl={}",
				GetLogName(), usCmd, bTradeData, bBoat, bStallData, bTalkCtrl, bKitbagLock, bItemCtrl);
			return;
		}

		std::uint32_t ulID = pk.ReadInt64();
		ToLogService("trade", "REQUESTTALK/TRADE cha={} cmd={} npcId={}", GetLogName(), usCmd, ulID);
		Handle_RequestTalk(ulID, pk);
	}
	break;
	//daily buff request to open ui
	case CMD_CM_DailyBuffRequest: {
		Handle_DailyBuffRequest();
		break;
	}
	case CMD_CM_MISLOG: {
		MisLog();
	}
	break;
	case CMD_CM_MISLOGINFO: {
		Corsairs::Net::Msg::CmMisLogInfoMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		MisLogInfo(static_cast<WORD>(msg.id));
	}
	break;
	case CMD_CM_MISLOG_CLEAR: {
		Corsairs::Net::Msg::CmMisLogClearMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		MisLogClear(static_cast<WORD>(msg.id));
	}
	break;
	case CMD_CM_FORGE: {
		Corsairs::Net::Msg::CmForgeItemMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_ForgeSystem.ForgeItem(*this, static_cast<BYTE>(msg.index));
	}
	break;
	case CMD_CM_CHARTRADE_REQUEST: {
		Corsairs::Net::Msg::CmRequestTradeMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.Request(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId));
	}
	break;
	case CMD_CM_CHARTRADE_ACCEPT: {
		Corsairs::Net::Msg::CmAcceptTradeMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.Accept(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId));
	}
	break;
	case CMD_CM_CHARTRADE_REJECT: {
	}
	break;
	case CMD_CM_CHARTRADE_CANCEL: {
		Corsairs::Net::Msg::CmCancelTradeMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.Cancel(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId));
	}
	break;
	case CMD_CM_CHARTRADE_ITEM: {
		Corsairs::Net::Msg::CmAddItemMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.AddItem(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId),
							  static_cast<BYTE>(msg.opType), static_cast<BYTE>(msg.index),
							  static_cast<BYTE>(msg.itemIndex), static_cast<BYTE>(msg.count));
	}
	break;
	case CMD_CM_CHARTRADE_MONEY: {
		Corsairs::Net::Msg::CmAddMoneyMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		BYTE byType = static_cast<BYTE>(msg.type);
		DWORD dwCharID = static_cast<DWORD>(msg.charId);
		BYTE byOpType = static_cast<BYTE>(msg.opType);
		BYTE currency = static_cast<BYTE>(msg.isImp);
		DWORD dwMondy = static_cast<DWORD>(msg.money);

		if (currency == 0) {
			//gold
			g_TradeSystem.AddMoney(byType, *this, dwCharID, byOpType, dwMondy);
		}
		else if (currency == 1) {
			//IMPS
			g_TradeSystem.AddIMP(byType, *this, dwCharID, byOpType, dwMondy);
		}
	}
	break;
	case CMD_CM_CHARTRADE_VALIDATEDATA: {
		Corsairs::Net::Msg::CmValidateTradeDataMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.ValidateItemData(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId));
	}
	break;
	case CMD_CM_CHARTRADE_VALIDATE: {
		Corsairs::Net::Msg::CmValidateTradeMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		g_TradeSystem.ValidateTrade(static_cast<BYTE>(msg.type), *this, static_cast<DWORD>(msg.charId));
	}
	break;
	case CMD_CM_CREATE_BOAT: {
		Corsairs::Net::Msg::CmCreateBoatMessage boatMsg;
		Corsairs::Net::Msg::deserialize(pk, boatMsg);
		g_CharBoat.MakeBoat(*this, boatMsg);
	}
	break;
	case CMD_CM_UPDATEBOAT_PART: {
		Corsairs::Net::Msg::CmUpdateBoatMessage boatMsg;
		Corsairs::Net::Msg::deserialize(pk, boatMsg);
		g_CharBoat.Update(*this, boatMsg);
	}
	break;
	case CMD_CM_BOAT_GETINFO: {
		if (GetPlayer()->IsLuanchOut()) {
			g_CharBoat.GetBoatInfo(*this, GetPlayer()->GetLuanchID());
		}
		else {
			//SystemNotice( "" );
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00003));
		}
	}
	break;
	case CMD_CM_BOAT_CANCEL: {
		g_CharBoat.Cancel(*this);
	}
	break;
	case CMD_CM_BOAT_LUANCH: {
		Corsairs::Net::Msg::CmBoatLaunchMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		DWORD dwNpcID = static_cast<DWORD>(msg.npcId);
		CCharacter* pCha = m_submap->FindCharacter(dwNpcID, GetShape().Centre);
		if (pCha == NULL) {
			break;
		}
		else if (GetPlayer()->GetBankNpc()) {
			break;
		}
		else if (auto ret = g_luaAPI.CallR<int>("IsSailNpc", static_cast<CCharacter*>(this), pCha)) {
			if (!ret.value()) {
				break;
			}
		}

		BoatSelLuanch(static_cast<BYTE>(msg.index));
	}
	break;
	case CMD_CM_BOAT_SELECT: {
		Corsairs::Net::Msg::CmSelectBoatListMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		DWORD dwNpcID = static_cast<DWORD>(msg.npcId);
		CCharacter* pCha = m_submap->FindCharacter(dwNpcID, GetShape().Centre);
		if (pCha == NULL) {
			break;
		}
		if (auto ret = g_luaAPI.CallR<int>("IsSailBoatNpc", static_cast<CCharacter*>(this), pCha)) {
			if (!ret.value()) {
				break;
			}
		}
		BoatSelected(static_cast<BYTE>(msg.type), static_cast<BYTE>(msg.index));
	}
	break;
	case CMD_CM_BOAT_BAGSEL: {
		Corsairs::Net::Msg::CmBoatBagSelMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		DWORD dwNpcID = static_cast<DWORD>(msg.npcId);
		if (dwNpcID) {
			CCharacter* pCha = m_submap->FindCharacter(dwNpcID, GetShape().Centre);
			if (pCha == NULL)
				break;
		}

		BoatPackBag(static_cast<BYTE>(msg.index));
	}
	break;
	case CMD_CM_ENTITY_EVENT: {
		DWORD dwEntityID = pk.ReadInt64();
		CCharacter* pCha = m_submap->FindCharacter(dwEntityID, GetShape().Centre);
		if (pCha == NULL) break;
		mission::CEventEntity* pEntity = pCha->IsEvent();
		if (pEntity) {
			pEntity->MsgProc(*this, pk);
			break;
		}
	}
	break;
	case CMD_CM_STALL_ALLDATA: {
		Corsairs::Net::Msg::CmStallInfoMessage stallMsg;
		Corsairs::Net::Msg::deserialize(pk, stallMsg);
		g_StallSystem.StartStall(*this, stallMsg);
	}
	break;
	case CMD_CM_STALL_OPEN: {
		Corsairs::Net::Msg::CmStallOpenMessage stallMsg;
		Corsairs::Net::Msg::deserialize(pk, stallMsg);
		g_StallSystem.OpenStall(*this, stallMsg);
	}
	break;
	case CMD_CM_STALL_BUY: {
		Corsairs::Net::Msg::CmStallBuyMessage stallMsg;
		Corsairs::Net::Msg::deserialize(pk, stallMsg);
		g_StallSystem.BuyGoods(*this, stallMsg);
	}
	break;
	case CMD_CM_STALL_CLOSE: {
		g_StallSystem.CloseStall(*this);
	}
	break;
	case CMD_CM_READBOOK_START: {
		CCharacter* pMainCha = GetPlyMainCha();
		if (!IsBoat()) {
			pMainCha->SetReadBookState(true);
			pMainCha->ForgeAction(true);
			pMainCha->m_CKitbag.Lock();
		}
		else
		//pMainCha->SystemNotice("");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00004));
	}
	break;
	case CMD_CM_READBOOK_CLOSE: {
		CCharacter* pMainCha = GetPlyMainCha();
		if (!IsBoat()) {
			pMainCha->SetReadBookState(false);
			pMainCha->ForgeAction(false);
			pMainCha->m_CKitbag.UnLock();
		}
		else
		//pMainCha->SystemNotice("");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00005));
	}
	break;
	case CMD_CM_SYNATTR: {
		Corsairs::Net::Msg::CmSynAttrMessage synMsg;
		Corsairs::Net::Msg::deserialize(pk, synMsg);
		GetPlayer()->GetMainCha()->Cmd_ReassignAttr(synMsg);
	}
	break;
	case CMD_CM_SKILLUPGRADE: {
		Corsairs::Net::Msg::CmSkillUpgradeMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		int16_t sSkillID = static_cast<int16_t>(msg.skillId);
		char chAddGrade = static_cast<char>(msg.addGrade);

		// kong@pkodev.net 09.22.2017
		chAddGrade = 1;

		char chSkillLv = 0;
		CCharacter* pMainCha = GetPlyMainCha();
		SSkillGrid* pSkill = pMainCha->m_CSkillBag.GetSkillContByID(sSkillID);
		if (pSkill)
			chSkillLv = pSkill->chLv;

		if (chSkillLv <= 0) {
			SystemNotice("Unable to upgrade skill without learning!");
			break;
		}

		GetPlayer()->GetMainCha()->LearnSkill(sSkillID, chAddGrade, false);
	}
	break;
	case CMD_CM_REFRESH_DATA: {
		Corsairs::Net::Msg::CmRefreshDataMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_RefreshData(msg);
	}
	break;
	case CMD_TM_CHANGE_PERSONINFO: {
		Corsairs::Net::Msg::TmChangePersonInfoMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		SetMotto(msg.motto.c_str());
		SetIcon(static_cast<int>(msg.icon));
	}
	break;
	case CMD_CM_GUILD_PERM: {
		Corsairs::Net::Msg::CmGuildPermMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		int targetID = static_cast<int>(msg.id);
		unsigned long permission = static_cast<unsigned long>(msg.perms);
		int guild_id = GetPlyMainCha()->GetGuildID();
		if (guild_id == 0 || !(emGldPermMgr & GetPlyMainCha()->guildPermission) || game_db.GetGuildLeaderID(guild_id) ==
			targetID) {
			GetPlyMainCha()->SystemNotice("You do not have permission to do this.");
			return;
		}

		//update in DB
		if (!game_db.SetGuildPermission(targetID, permission, guild_id)) {
			GetPlyMainCha()->SystemNotice("Player not found");
			return;
		}

		//update in game
		CPlayer* targetPly = g_pGameApp->GetPlayerByDBID(targetID);
		if (targetPly) {
			targetPly->GetMainCha()->guildPermission = permission;
		}

		//update for group (sends to players)
		auto wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpGuildPermMessage{
			static_cast<int64_t>(targetID), static_cast<int64_t>(permission)
		});
		ReflectINFof(this, wpk);
		break;
	}
	case CMD_CM_GUILD_PUTNAME: {
		Corsairs::Net::Msg::CmGuildPutNameMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		bool l_confirm = msg.confirm ? true : false;
		if (!msg.guildName.empty() && Guild::IsValidGuildName(msg.guildName.c_str(), std::uint16_t(msg.guildName.length())) &&
			!msg.passwd.empty() && !strchr(msg.passwd.c_str(), '\'')) {
			Guild::cmd_CreateGuild(GetPlyMainCha(), l_confirm, msg.guildName.c_str(), msg.passwd.c_str());
		}
		else {
			//GetPlyMainCha()->SystemNotice("");
			GetPlyMainCha()->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00006));
		}
	}
	break;
	case CMD_CM_GUILD_TRYFOR: {
		Corsairs::Net::Msg::CmGuildTryForMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildTryFor(GetPlyMainCha(), msg.guildId);
	}
	break;
	case CMD_CM_GUILD_TRYFORCFM: {
		Corsairs::Net::Msg::CmGuildTryForCfmMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildTryForComfirm(GetPlyMainCha(), msg.confirm);
	}
	break;
	case CMD_CM_GUILD_LISTTRYPLAYER: {
		Guild::cmd_GuildListTryPlayer(GetPlyMainCha());
	}
	break;
	case CMD_CM_GUILD_APPROVE: {
		Corsairs::Net::Msg::CmGuildApproveMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildApprove(GetPlyMainCha(), msg.chaId);
	}
	break;
	case CMD_CM_GUILD_REJECT: {
		Corsairs::Net::Msg::CmGuildRejectMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildReject(GetPlyMainCha(), msg.chaId);
	}
	break;
	case CMD_CM_GUILD_KICK: {
		Corsairs::Net::Msg::CmGuildKickMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildKick(GetPlyMainCha(), msg.chaId);
	}
	break;
	case CMD_CM_GUILD_LEAVE: {
		if (!(GetPlyCtrlCha()->GetSubMap()->GetMapRes()->CanGuild())) {
			//GetPlyMainCha()->SystemNotice("!");
			GetPlyMainCha()->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00007));
			break;
		}

		Guild::cmd_GuildLeave(GetPlyMainCha());
	}
	break;
	case CMD_CM_GUILD_DISBAND: {
		Corsairs::Net::Msg::CmGuildDisbandMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		int canDisband = (GetPlyMainCha()->guildPermission & emGldPermDisband);
		if (canDisband == emGldPermDisband) {
			if (!msg.passwd.empty() && !strchr(msg.passwd.c_str(), '\'')) {
				Guild::cmd_GuildDisband(GetPlyMainCha(), msg.passwd.c_str());
			}
		}
		break;
	}
	case CMD_CM_GUILD_MOTTO: {
		Corsairs::Net::Msg::CmGuildMottoMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		if (!msg.motto.empty() && msg.motto.length() < 50 &&
			IsValidName(msg.motto)) {
			int canMotto = (GetPlyMainCha()->guildPermission & emGldPermMotto);
			if (canMotto == emGldPermMotto && !strchr(msg.motto.c_str(), '\'')) {
				// Probably not enough
				Guild::cmd_GuildMotto(GetPlyMainCha(), msg.motto.c_str());
			}
		}
	}
	break;
	case CMD_PM_GUILD_DISBAND: {
		Guild::cmd_PMDisband(GetPlyMainCha());
	}
	break;
	case CMD_CM_GUILD_CHALLENGE: {
		Corsairs::Net::Msg::CmGuildChallMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildChallenge(GetPlyMainCha(), static_cast<BYTE>(msg.level), static_cast<DWORD>(msg.money));
	}
	break;
	case CMD_CM_GUILD_LEIZHU: {
		Corsairs::Net::Msg::CmGuildLeizhuMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Guild::cmd_GuildLeizhu(GetPlyMainCha(), static_cast<BYTE>(msg.level), static_cast<DWORD>(msg.money));
	}
	break;
	case CMD_CM_MAP_MASK: {
		Handle_MapMask();
	}
	break;

	case CMD_CM_UPDATEHAIR: // 
	{
		if (!GetSubMap()) break;
		Cmd_ChangeHair(pk);
	}
	break;
	case CMD_CM_TEAM_FIGHT_ASK: // 
	{
		Corsairs::Net::Msg::CmTeamFightAskMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Cmd_FightAsk(static_cast<char>(msg.type), static_cast<std::int32_t>(msg.worldId), static_cast<std::int32_t>(msg.handle));
	}
	break;
	case CMD_CM_TEAM_FIGHT_ASR: // 
	{
		Corsairs::Net::Msg::CmTeamFightAnswerMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Cmd_FightAnswer(msg.accept != 0 ? true : false);
	}
	break;
	case CMD_CM_ITEM_REPAIR_ASK: {
		Corsairs::Net::Msg::CmItemRepairAskMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);

		Cmd_ItemRepairAsk(static_cast<char>(msg.posType), static_cast<char>(msg.posId));
	}
	break;
	case CMD_CM_ITEM_REPAIR_ASR: {
		Corsairs::Net::Msg::CmItemRepairAnswerMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Cmd_ItemRepairAnswer(msg.accept != 0 ? true : false);
	}
	break;
	case CMD_CM_ITEM_FORGE_CANACTION: {
		Corsairs::Net::Msg::CmItemForgeCanActionMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		ForgeAction(msg.canAction != 0);
		break;
	}
	case CMD_CM_ITEM_FORGE_ASK: {
		Corsairs::Net::Msg::CmItemForgeGroupAskMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_ItemForgeAsk(msg);
	}
	break;
	// Add by lark.li 20080515 begin
	case CMD_CM_ITEM_LOTTERY_ASK: {
		Corsairs::Net::Msg::CmItemLotteryGroupAskMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_ItemLotteryAsk(msg);
	}
	break;
	// End
	case CMD_CM_ITEM_FORGE_ASR: {
		Corsairs::Net::Msg::CmItemForgeAnswerMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Cmd_ItemForgeAnswer(msg.accept != 0 ? true : false);
	}
	break;
	case CMD_CM_KITBAG_LOCK: {
		GetPlyMainCha()->Cmd_LockKitbag();
	}
	break;
	case CMD_CM_LIFESKILL_ASK: {
		Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
		if (Corsairs::Net::Msg::deserializeLifeSkillAsk(pk, msg))
			Handle_LifeSkillAsk(msg);
		break;
	}
	case CMD_CM_LIFESKILL_ASR: {
		Corsairs::Net::Msg::CmLifeSkillCraftMessage msg;
		if (Corsairs::Net::Msg::deserializeLifeSkillAsr(pk, msg))
			Handle_LifeSkillAsr(msg);
	}
	break;
	case CMD_CM_KITBAG_UNLOCK: {
		Corsairs::Net::Msg::CmKitbagUnlockMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		GetPlyMainCha()->Cmd_UnlockKitbag(msg.password.c_str());
	}
	break;
	case CMD_CM_KITBAG_CHECK: {
		GetPlyMainCha()->Cmd_CheckKitbagState();
	}
	break;
	case CMD_CM_KITBAG_AUTOLOCK: {
		Corsairs::Net::Msg::CmAutoKitbagLockMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		GetPlyMainCha()->Cmd_SetKitbagAutoLock(static_cast<char>(msg.autoLock));
	}
	break;
	case CMD_CM_STORE_OPEN_ASK: {
		Corsairs::Net::Msg::CmStoreOpenAskMessage storeMsg;
		Corsairs::Net::Msg::deserialize(pk, storeMsg);
		const std::string& szPwd = storeMsg.password;
		CCharacter* pMainCha = GetPlyMainCha();
		if (pMainCha->IsReadBook()) {
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00008));
			break;
		}

		if (pMainCha->IsStoreEnable()) {
			break;
		}

		if (!pMainCha->CheckStoreTime(1000)) {
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00009));
			break;
		}
		else {
			pMainCha->ResetStoreTime();
		}

		CPlayer* pCply = pMainCha->GetPlayer();
		const char* szPwd2 = pCply->GetPassword();

		if ((szPwd2[0] == 0) || (!strcmp(szPwd.c_str(), szPwd2)) || g_Config.m_bInstantIGS) {
			pMainCha->SetStoreEnable(true);
		}
		else {
			pMainCha->PopupNotice(RES_STRING(GM_CHARACTERPRL_CPP_00010));
			break;
		}
	}
	case CMD_CM_STORE_LIST_ASK:
	case CMD_CM_STORE_BUY_ASK:
	case CMD_CM_STORE_CHANGE_ASK:
	case CMD_CM_STORE_QUERY:
	case CMD_CM_STORE_CLOSE:
	case CMD_CM_STORE_VIP: {
		Handle_StoreCommand(usCmd, pk);
		break;
	}

	case CMD_CM_TIGER_START: {
		Corsairs::Net::Msg::CmTigerStartMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_TigerStart(msg);
	}
	break;
	case CMD_CM_TIGER_STOP: {
		Corsairs::Net::Msg::CmTigerStopMessage msg;
		Corsairs::Net::Msg::deserialize(pk, msg);
		Handle_TigerStop(msg);
	}
	break;
	case CMD_CM_VOLUNTER_OPEN: {
		Corsairs::Net::Msg::CmVolunteerOpenMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_VolunteerOpen(cmMsg);
	}
	break;
	case CMD_CM_VOLUNTER_LIST: {
		Corsairs::Net::Msg::CmVolunteerListMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_VolunteerList(cmMsg);
	}
	break;
	case CMD_CM_VOLUNTER_ADD: {
		CCharacter* pMainCha = GetPlyMainCha();
		pMainCha->Cmd_AddVolunteer();
		pMainCha->SynVolunteerState(pMainCha->IsVolunteer());
	}
	break;
	case CMD_CM_VOLUNTER_DEL: {
		CCharacter* pMainCha = GetPlyMainCha();
		pMainCha->Cmd_DelVolunteer();
		pMainCha->SynVolunteerState(pMainCha->IsVolunteer());
	}
	break;
	case CMD_CM_VOLUNTER_SEL: {
		Corsairs::Net::Msg::CmVolunteerSelMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_VolunteerSel(cmMsg);
	}
	break;
	case CMD_CM_VOLUNTER_ASR: {
		Corsairs::Net::Msg::CmVolunteerAsrMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_VolunteerAsr(cmMsg);
	}
	break;
	case CMD_CM_KITBAGTEMP_SYNC: {
		Handle_KitbagTempSync();
	}
	break;
	case CMD_CM_ITEM_LOCK_ASK: {
		Corsairs::Net::Msg::CmItemLockAskMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_ItemLockAsk(cmMsg);
	}
	break;
	case CMD_CM_GAME_REQUEST_PIN: {
		Corsairs::Net::Msg::CmGameRequestPinMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_GameRequestPin(cmMsg);
		break;
	}
	case CMD_CM_ITEM_UNLOCK_ASK: {
		Corsairs::Net::Msg::CmItemUnlockAskMessage unlockMsg;
		Corsairs::Net::Msg::deserialize(pk, unlockMsg);
		ItemUnlockRequest(unlockMsg);
	}
	break;
	case CMD_CM_MASTER_INVITE: {
		Corsairs::Net::Msg::CmMasterInviteMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_MasterInvite(cmMsg);
	}
	break;
	case CMD_CM_MASTER_ASR: {
		Corsairs::Net::Msg::CmMasterAsrMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_MasterAsr(cmMsg);
	}
	break;
	case CMD_CM_MASTER_DEL: {
		Corsairs::Net::Msg::CmMasterDelMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_MasterDel(cmMsg);
	}
	break;
	case CMD_CM_PRENTICE_DEL: {
		Corsairs::Net::Msg::CmPrenticeDelMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		Handle_PrenticeDel(cmMsg);
	}
	break;
	case CMD_CM_PRENTICE_INVITE: {
		CCharacter* pMainCha = GetPlyMainCha();
		Corsairs::Net::Msg::CmPrenticeInviteMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		std::string szName = cmMsg.name;
		DWORD dwCharID = static_cast<DWORD>(cmMsg.chaId);

		if (IsBoat()) {
			//SystemNotice("!");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00023));
			break;
		}

		CCharacter* pTarCha = pMainCha->GetSubMap()->FindCharacter(dwCharID, pMainCha->GetShape().Centre);
		if (!pTarCha) {
			//pMainCha->SystemNotice("%s !", szName);
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
			break;
		}

		if (pMainCha->GetLevel() < 41) {
			//pMainCha->SystemNotice("!");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00024));
			break;
		}

		if (pTarCha->GetLevel() > 40) {
			//pMainCha->SystemNotice("!");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00025));
			break;
		}

		if (pTarCha->IsInvited()) {
			//pMainCha->SystemNotice("!");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00022));
			break;
		}
		if (!pTarCha->GetPlayer()->CanReceiveRequests()) {
			pMainCha->SystemNotice("%s is currently offline. Unable to send request!", pMainCha->GetName());
			break;
		}
		pTarCha->SetInvited(true);

		//  :  
		auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McPrenticeAskMessage{pMainCha->GetName(), pMainCha->GetID()});
		pTarCha->ReflectINFof(pTarCha, packet);
	}
	break;
	case CMD_CM_PRENTICE_ASR: {
		CCharacter* pMainCha = GetPlyMainCha();
		Corsairs::Net::Msg::CmPrenticeAsrMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		short sRet = static_cast<short>(cmMsg.agree);
		std::string szName = cmMsg.name;
		DWORD dwCharID = static_cast<DWORD>(cmMsg.chaId);

		pMainCha->SetInvited(false);

		if (IsBoat()) {
			//SystemNotice("!");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00019));
			break;
		}

		CCharacter* pSrcCha = pMainCha->GetSubMap()->FindCharacter(dwCharID, pMainCha->GetShape().Centre);
		if (!pSrcCha) {
			//pMainCha->SystemNotice("%s !", szName);
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
			break;
		}

		if (pSrcCha->GetLevel() < 41) {
			//pSrcCha->SystemNotice("!");
			pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00024));
			//pMainCha->SystemNotice("!");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00017));
			break;
		}

		if (pMainCha->GetLevel() > 40) {
			//pSrcCha->SystemNotice("!");
			pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00025));
			//pMainCha->SystemNotice("!");
			pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00020));
			break;
		}

		if (sRet == 0) {
			//pSrcCha->SystemNotice("%s !", pMainCha->GetName());
			pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00030), pMainCha->GetName());
			break;
		}

		//  :   - (  )
		auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpMasterCreateMessage{
			pMainCha->GetName(), pMainCha->GetPlayer()->GetDBChaId(),
			pSrcCha->GetName(), pSrcCha->GetPlayer()->GetDBChaId()
		});
		pMainCha->ReflectINFof(pMainCha, l_wpk);
	}
	break;
	case CMD_CM_SAY2CAMP: {
		CCharacter* pMainCha = GetPlyMainCha();
		Corsairs::Net::Msg::CmSay2CampMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		std::string szContent = cmMsg.content;
		CCharacter* pCha = NULL;
		SubMap* pSubMap = GetPlyCtrlCha()->GetSubMap();

		BOOL bHasGuild = pMainCha->HasGuild();
		if (!bHasGuild) {
			//SystemNotice("!");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00031));
			break;
		}

		SystemNotice("Channel disabled; used for communicating with guild members within sacred war map.");
		break;
		/*
		if(pSubMap->GetMapRes()->CanGuildWar())
		{
			char cGuildType = pMainCha->GetGuildType();

			pSubMap->BeginGetPlyCha();
			while(pCha = pSubMap->GetNextPlyCha())
			{
				if(pCha->HasGuild() && pCha->GetGuildType() == cGuildType)
				{
					auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McSay2CampMessage{pMainCha->GetName(), szContent});
					pCha->ReflectINFof(pCha, l_wpk);
				}
			}
		}
		else
		{
			//SystemNotice("!");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00032));
		}
		*/
		//if (g_Config.m_bBlindChaos && IsPlayerCha() && IsPKSilver())
		//{
		//	SystemNotice("Unable to chat in this map!");
		//	break;
		//}
	}
	break;
	case CMD_CM_GM_SEND:
	case CMD_CM_GM_RECV:
		// GM mail   InfoServer (Trade Server) --
		break;
	case CMD_CM_PK_CTRL: {
		CCharacter* pMainCha = GetPlyMainCha();

		Corsairs::Net::Msg::CmPkCtrlMessage cmMsg;
		Corsairs::Net::Msg::deserialize(pk, cmMsg);
		if (cmMsg.ctrl)
			Cmd_SetInPK();
		else
			Cmd_SetInPK(false);
		SynPKCtrl();
	}
	break;
	case CMD_CM_CHEAT_CHECK: {
		//
		/*CCharacter *pMainCha = GetPlyMainCha();

		const char *answer = pk.ReadString();
		pMainCha->CheatCheck(answer);*/
	}
	break;
	case CMD_CM_BIDUP:
		//add by ALLEN 2007-10-19
	{
		//
		CCharacter* pMainCha = GetPlyMainCha();
		if (auto yornRet = g_luaAPI.CallR<int>("YORN", pMainCha)) {
			if (yornRet.value()) {
				Corsairs::Net::Msg::CmBidUpMessage cmMsg;
				Corsairs::Net::Msg::deserialize(pk, cmMsg);
				DWORD dwNpcID = static_cast<DWORD>(cmMsg.npcId);
				CCharacter* pNpc = m_submap->FindCharacter(dwNpcID, GetShape().Centre);
				if (pNpc == NULL) {
					//SystemNotice( "NPCID%d", dwNpcID );
					SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00034), dwNpcID);
					break;
				}
				g_AuctionSystem.BidUp(pMainCha, static_cast<short>(cmMsg.itemId), static_cast<std::uint32_t>(cmMsg.price));
				g_AuctionSystem.NotifyAuction(this, pNpc);
			}
		}
	}
	break;
	case CMD_CM_ANTIINDULGENCE: {
		GetPlyMainCha()->SetScaleFlag();
	}
	break;
	case CMD_CM_REQUEST_DROP_RATE: {
		CCharacter* pCha = GetPlyCtrlCha();
		if (pCha) {
			//  :     
			auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McRequestDropRateMessage{pCha->GetDropRate()});
			pCha->ReflectINFof(pCha, pk);
		}
		break;
	}
	case CMD_CM_REQUEST_EXP_RATE: {
		CCharacter* pCha = GetPlyMainCha();
		if (pCha) {
			//  :     
			auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McRequestExpRateMessage{pCha->GetExpRate()});
			pCha->ReflectINFof(pCha, pk);
		}
		break;
	}
	default:
		break;
	}
}

//    (  ProcessPacket) 

void CCharacter::Handle_GuildBankCmd(const Corsairs::Net::Msg::PmGuildBankMessage& msg) {
	char bankType = static_cast<char>(msg.bankType);

	if (const DWORD COOLDOWN = GetTickCount(); GetPlyMainCha()->GuildBankCD > COOLDOWN) {
		BickerNotice("Please calm down Don't spam! %ds left!", (GetPlyMainCha()->GuildBankCD - COOLDOWN) / 1000);
	}
	else if (!IsLiveing()) {
		SystemNotice("Dead pirates are unable to trade.");
	}
	else if (GetPlyCtrlCha()->IsBoat()) {
		SystemNotice("Must be on land to use the guild bank.");
	}
	else if (!IsInArea(2)) {
		SystemNotice("Must be in safe zone to use the guild bank.");
	}
	else {
		const int cdtime = 3000;
		GetPlyMainCha()->GuildBankCD = COOLDOWN + cdtime;

		if (bankType == 0 && std::holds_alternative<Corsairs::Net::Msg::PmGuildBankOperData>(msg.data)) {
			auto& operData = std::get<Corsairs::Net::Msg::PmGuildBankOperData>(msg.data);
			char chSrcType = static_cast<char>(operData.srcType);
			int16_t sSrcGrid = static_cast<int16_t>(operData.srcGrid);
			int16_t sSrcNum = static_cast<int16_t>(operData.srcNum);
			char chTarType = static_cast<char>(operData.tarType);
			int16_t sTarGrid = static_cast<int16_t>(operData.tarGrid);
			int guildID = GetGuildID();
			std::vector<CTableGuild::BankLog> logs = game_db.GetGuildLog(guildID);

			if (chTarType != chSrcType) {
				CTableGuild::BankLog l;
				CKitbag bag;

				l.time = time(0);
				l.quantity = sSrcNum;
				l.userID = GetPlyMainCha()->m_ID;

				if (chTarType == 0) {
					game_db.GetGuildBank(guildID, &bag);
					l.type = 2;
				}
				else if (chTarType == 1) {
					bag = GetPlyMainCha()->m_CKitbag;
					l.type = 3;
				}
				l.parameter = bag.GetID(sSrcGrid);
				logs.push_back(l);
			}
			int16_t sRet = Cmd_GuildBankOper(chSrcType, sSrcGrid, sSrcNum, chTarType, sTarGrid);
			if (sRet != enumITEMOPT_SUCCESS || !game_db.SetGuildLog(logs, guildID)) {
				ItemOprateFailed(sRet);
			}
		}
		else if (bankType == 1 && std::holds_alternative<Corsairs::Net::Msg::PmGuildBankGoldData>(msg.data)) {
			[&]() {
				auto& goldData = std::get<Corsairs::Net::Msg::PmGuildBankGoldData>(msg.data);
				char action = static_cast<char>(goldData.direction);

				int guildID = GetGuildID();
				int gold = static_cast<int>(goldData.gold);
				int currentgold = getAttr(ATTR_GD);
				unsigned long long guildGold = game_db.GetGuildBankGold(guildID);

				unsigned long long maxGuildGold = 9223372036854775807LL;
				int maxCharGold = 2000000000;
				std::vector<CTableGuild::BankLog> logs = game_db.GetGuildLog(guildID);

				int canTake = (emGldPermTakeBank & guildPermission);
				int canGive = (emGldPermDepoBank & guildPermission);

				CTableGuild::BankLog l;

				if (action == 0 && canTake == emGldPermTakeBank) {
					l.type = 0;
					if (gold + currentgold > maxCharGold) {
						gold = maxCharGold - currentgold;
					}
					if (gold > guildGold) {
						gold = guildGold;
					}
					if (gold < 1) {
						return;
					}
				}
				else if (action == 1 && canGive == emGldPermDepoBank) {
					l.type = 1;
					if (gold > currentgold) {
						gold = currentgold;
					}
					if (gold + guildGold > maxGuildGold) {
						gold = maxGuildGold - guildGold;
					}
					if (gold < 1) {
						return;
					}
					gold = 0 - gold;
				}
				else {
					return;
				}

				if (game_db.UpdateGuildBankGold(guildID, -gold)) {
					l.time = time(0);
					l.parameter = gold > 0 ? gold : -gold;
					l.quantity = 0;
					l.userID = GetPlyMainCha()->m_ID;

					logs.push_back(l);
					if (game_db.SetGuildLog(logs, guildID)) {
						setAttr(ATTR_GD, currentgold + gold);
						SynAttr(enumATTRSYN_TRADE);
						SyncBoatAttr(enumATTRSYN_TRADE);

						auto WtPk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MmUpdateGuildBankGoldMessage{
							static_cast<int64_t>(GetPlyMainCha()->GetGuildID())
						});
						ReflectINFof(this, WtPk);
					}
				}
			}();
		}
	}

	auto WtPk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpGuildBankAckMessage{
		static_cast<int64_t>(GetGuildID())
	});
	ReflectINFof(this, WtPk);
}

void CCharacter::Handle_PushToGuildBank(const std::string& strItem) {
	int guildID = GetGuildID();
	if (guildID == 0) {
		return;
	}
	CKitbag pCSrcBag;
	game_db.GetGuildBank(guildID, &pCSrcBag);
	pCSrcBag.SetChangeFlag(false);

	SItemGrid SPopItem;
	String2Item(strItem.c_str(), &SPopItem);

	short sSrcGridID = defKITBAG_DEFPUSH_POS;
	if (pCSrcBag.Push(&SPopItem, sSrcGridID) == enumKBACT_ERROR_FULL) {
		//drop item next to player?
	}
	else {
		GetPlayer()->SynGuildBank(&pCSrcBag, enumSYN_KITBAG_BANK);
		GetPlayer()->SetBankSaveFlag(0);
		game_db.UpdateGuildBank(guildID, &pCSrcBag);
	}
	//let group know we have finished, so the next guild bank packet can be processed.
	auto WtPk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpGuildBankAckMessage{
		static_cast<int64_t>(guildID)
	});
	ReflectINFof(this, WtPk);
}

void CCharacter::Handle_Ping(const Corsairs::Net::Msg::CmPingResponseMessage& msg) {
	std::uint32_t ulPing = GetTickCount() - static_cast<std::uint32_t>(msg.v1);
	std::int32_t lGateSvr = static_cast<std::int32_t>(msg.v2);
	std::int32_t lSrcID = static_cast<std::int32_t>(msg.v3);
	std::int32_t lGatePlayerID = static_cast<std::int32_t>(msg.v4);
	std::int32_t lGatePlayerAddr = static_cast<std::int32_t>(msg.v5);

	BEGINGETGATE();
	GateServer* pNoGate;
	GateServer* pGate = 0;
	while (pNoGate = GETNEXTGATE()) {
		if (Corsairs::Util::ToAddress(pNoGate) == lGateSvr) {
			pGate = pNoGate;
			break;
		}
	}
	if (!pGate)
		return;

	auto WtPk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McQueryChaPingRouteMessage{
		lSrcID, std::string(GetName()), std::string(GetSubMap()->GetName()),
		static_cast<int64_t>(ulPing), lGatePlayerID, lGatePlayerAddr, 1
	});
	pGate->SendData(WtPk);
}

void CCharacter::Handle_Say(const Corsairs::Net::Msg::CmSayMessage& sayMsg) {
	DWORD dwNowTick = GetTickCount();
	if (dwNowTick - _dwLastSayTick < (DWORD)g_Config.m_lSayInterval) {
		SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00001));
		return;
	}
	_dwLastSayTick = dwNowTick;

	if (!GetSubMap()) {
		ToLogService("errors", LogLevel::Error, "when character{} is dialog, the map is null", GetLogName());
		return;
	}
	if (sayMsg.content.empty())
		return;
	else if (sayMsg.content[0] == '&') {
		char chGMLv = GetPlayer()->GetGMLev();
		if (chGMLv == 0 || chGMLv > 150)
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00002));
		else
			DoCommand(sayMsg.content.c_str() + 1, static_cast<std::uint32_t>(sayMsg.content.size()));
	}
	else if (sayMsg.content.size() > 2 && sayMsg.content[0] == '$' && sayMsg.content[1] == '$') {
		DoCommand_CheckStatus(sayMsg.content.c_str() + 3, static_cast<std::uint32_t>(sayMsg.content.size() - 2));
	}
	else {
		auto chatRet = g_luaAPI.CallR<int>("HandleChat", static_cast<CCharacter*>(this), sayMsg.content.c_str());
		if (!chatRet.value_or(0))
			return;
		if (g_Config.m_bBlindChaos && IsPlayerCha() && IsPKSilver()) {
			SystemNotice("Unable to chat in this map!");
			return;
		}

		auto wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McSayMessage{
			m_ID, sayMsg.content, static_cast<int64_t>(chatColour)
		});
		NotiChgToEyeshot(wpk);
	}
}

void CCharacter::Handle_RequestTalk(std::uint32_t npcId, Corsairs::Net::RPacket& pk) {
	if (npcId == mission::g_WorldEudemon.GetID()) {
		ToLogService("trade", "Handle_RequestTalk cha={} npcId={} -> WorldEudemon", GetLogName(), npcId);
		mission::g_WorldEudemon.MsgProc(*this, pk);
		return;
	}
	CCharacter* pCha = m_submap->FindCharacter(npcId, GetShape().Centre);
	if (pCha == NULL) {
		ToLogService("trade", LogLevel::Error,
			"Handle_RequestTalk cha={} npcId={} -> NPC not found on submap (pos {},{})",
			GetLogName(), npcId, GetShape().Centre.X, GetShape().Centre.Y);
		return;
	}
	mission::CNpc* pNpc = pCha->IsNpc();
	if (pNpc) {
		ToLogService("trade", "Handle_RequestTalk cha={} npcId={} -> CNpc::MsgProc [{}] packet={}",
			GetLogName(), npcId, pCha->GetLogName(), pk.PrintCommand());
		pNpc->MsgProc(*this, pk);
		return;
	}
	else {
		lua_getglobal(g_pLuaState, "extNpcNpcProc");
		if (!lua_isfunction(g_pLuaState, -1)) {
			ToLogService("trade", LogLevel::Error,
				"Handle_RequestTalk cha={} npcId={} -> lua fallback extNpcNpcProc NOT FOUND ({})",
				GetLogName(), npcId, pCha->GetLogName());
			lua_pop(g_pLuaState, 1);
			return;
		}

		ToLogService("trade", "Handle_RequestTalk cha={} npcId={} -> lua extNpcNpcProc ({})",
			GetLogName(), npcId, pCha->GetLogName());

		extern luabridge::LuaRef BuildNpcActionTable(lua_State*, Corsairs::Net::RPacket&);
		luabridge::LuaRef action = BuildNpcActionTable(g_pLuaState, pk);

		luabridge::push(g_pLuaState, static_cast<CCharacter*>(this));
		luabridge::push(g_pLuaState, static_cast<CCharacter*>(pCha));
		action.push(g_pLuaState);

		int nStatus = lua_pcall(g_pLuaState, 3, 0, 0);
		if (nStatus != 0) {
			const char* err = lua_tostring(g_pLuaState, -1);
			ToLogService("trade", LogLevel::Error,
				"Handle_RequestTalk cha={} npcId={} -> lua extNpcNpcProc FAILED: {}",
				GetLogName(), npcId, err ? err : "<null>");
		}
		lua_settop(g_pLuaState, 0);
	}
}

void CCharacter::Handle_DailyBuffRequest() {
	CCharacter* pCMainCha = GetPlyMainCha();
	CPlayer* pCPly = GetPlayer();
	if (pCMainCha->m_CKitbag.IsLock() || pCMainCha->m_CKitbag.IsPwdLocked() || pCPly->GetStallData() || pCPly->
		GetMainCha()->GetTradeData()) {
		SystemNotice("Bag is currently locked.");
		return;
	}
	if (!IsLiveing()) {
		SystemNotice("Ahoy there, matey! Looks like you took a tumble. Up and at 'em!");
		return;
	}
	if (pCPly->GetCtrlCha()->IsBoat()) {
		SystemNotice("Can't Use While Sailing .");
		return;
	}

	g_luaAPI.Call("DailyBuffRequest", static_cast<CCharacter*>(this));
}

void CCharacter::Handle_RefreshData(const Corsairs::Net::Msg::CmRefreshDataMessage& msg) {
	std::int32_t lWorldID = static_cast<std::int32_t>(msg.worldId);
	std::int32_t lHandle = static_cast<std::int32_t>(msg.handle);
	Entity* pCEnt = g_pGameApp->IsLiveingEntity(lWorldID, lHandle);
	if (pCEnt) {
		CCharacter* pCCha = pCEnt->IsCharacter();
		if (pCCha && pCCha->GetPlayer() == GetPlayer()) {
			pCCha->SynAttr(enumATTRSYN_ITEM_EQUIP);
		}
	}
}

void CCharacter::Handle_MapMask() {
	if (!GetSubMap())
		return;
	//const char	*szMapName = pk.ReadString();
	const char* szMapName = GetSubMap()->GetName();

	Corsairs::Net::Msg::McMapMaskMessage msg;
	msg.worldId = m_ID;
	msg.fogOfWarEnabled = (g_Config.m_chMapMask > 0);

	if (!msg.fogOfWarEnabled) {
		msg.hasData = false;
	}
	else {
		auto wireBlob = GetPlayer()->GetMapMaskWire();
		if (wireBlob.empty()) {
			msg.hasData = false;
		}
		else {
			msg.hasData = true;
			msg.data.assign(reinterpret_cast<const char*>(wireBlob.data()),
			                reinterpret_cast<const char*>(wireBlob.data()) + wireBlob.size());
		}
	}
	auto wpk = Corsairs::Net::Msg::serialize(msg);
	ReflectINFof(this, wpk);
}

void CCharacter::Handle_ItemForgeAsk(const Corsairs::Net::Msg::CmItemForgeGroupAskMessage& msg) {
	if (!msg.sure) {
		ForgeAction(false);
		return;
	}
	SForgeItem SFgeItem;
	for (int i = 0; i < defMAX_ITEM_FORGE_GROUP; i++) {
		SFgeItem.SGroup[i].sGridNum = static_cast<short>(msg.groups[i].cells.size());
		if (SFgeItem.SGroup[i].sGridNum < 0 || SFgeItem.SGroup[i].sGridNum > defMAX_KBITEM_NUM_PER_TYPE) {
			ForgeAction(false);
			return;
		}
		for (short j = 0; j < SFgeItem.SGroup[i].sGridNum; j++) {
			SFgeItem.SGroup[i].SGrid[j].sGridID = static_cast<short>(msg.groups[i].cells[j].posId);
			SFgeItem.SGroup[i].SGrid[j].sItemNum = static_cast<short>(msg.groups[i].cells[j].num);
		}
	}
	Cmd_ItemForgeAsk(static_cast<char>(msg.type), &SFgeItem);
}

void CCharacter::Handle_ItemLotteryAsk(const Corsairs::Net::Msg::CmItemLotteryGroupAskMessage& msg) {
	if (!msg.sure) {
		ForgeAction(false);
		return;
	}

	SLotteryItem SLtrItem;
	for (int i = 0; i < defMAX_ITEM_LOTTERY_GROUP; i++) {
		SLtrItem.SGroup[i].sGridNum = static_cast<short>(msg.groups[i].cells.size());
		if (SLtrItem.SGroup[i].sGridNum < 0 || SLtrItem.SGroup[i].sGridNum > defMAX_KBITEM_NUM_PER_TYPE) {
			return;
		}
		for (short j = 0; j < SLtrItem.SGroup[i].sGridNum; j++) {
			SLtrItem.SGroup[i].SGrid[j].sGridID = static_cast<short>(msg.groups[i].cells[j].posId);
			SLtrItem.SGroup[i].SGrid[j].sItemNum = static_cast<short>(msg.groups[i].cells[j].num);
		}
	}
	Cmd_ItemLotteryAsk(&SLtrItem);
}

void CCharacter::Handle_LifeSkillAsk(const Corsairs::Net::Msg::CmLifeSkillCraftMessage& craftMsg) {
	long type = static_cast<long>(craftMsg.skillType);
	int posCount = Corsairs::Net::Msg::kLifeSkillNeedItemNum[type];

	SLifeSkillItem LifeSkillItem;
	LifeSkillItem.sbagCount = static_cast<short>(posCount);
	for (int i = 0; i < posCount; i++) {
		LifeSkillItem.sGridID[i] = static_cast<short>(craftMsg.positions[i]);
	}
	switch (type) {
	case 0: {
		LifeSkillItem.sReturn = atoi(GetPlayer()->GetLifeSkillinfo().c_str());
		break;
	}
	case 1: {
		string strVer[2];
		Corsairs::Util::ResolveTextLine(GetPlayer()->GetLifeSkillinfo().c_str(), strVer, 2, ',');
		if (atoi(strVer[0].c_str()) > atoi(strVer[1].c_str()))
			LifeSkillItem.sReturn = 1;
		else
			LifeSkillItem.sReturn = 0;
		break;
	}
	case 2: {
		short sret = static_cast<short>(craftMsg.extraParam);
		string strVer[3];
		Corsairs::Util::ResolveTextLine(GetPlayer()->GetLifeSkillinfo().c_str(), strVer, 3, ',');
		int count = atoi(strVer[0].c_str()) + atoi(strVer[1].c_str()) + atoi(strVer[2].c_str());
		count -= 9;
		if (count > 0)
			count = 1;
		else
			count = 0;
		if (count == sret)
			LifeSkillItem.sReturn = 1;
		else
			LifeSkillItem.sReturn = 0;
		break;
	}
	case 3: {
		LifeSkillItem.sReturn = static_cast<short>(craftMsg.extraParam);
		break;
	}
	}
	Cmd_LifeSkillItemAsk(type, &LifeSkillItem);
}

void CCharacter::Handle_LifeSkillAsr(const Corsairs::Net::Msg::CmLifeSkillCraftMessage& craftAsrMsg) {
	long type = static_cast<long>(craftAsrMsg.skillType);
	int posCount = Corsairs::Net::Msg::kLifeSkillNeedItemNum[type];

	SLifeSkillItem LifeSkillItem;
	LifeSkillItem.sbagCount = static_cast<short>(posCount);
	for (int i = 0; i < posCount; i++) {
		LifeSkillItem.sGridID[i] = static_cast<short>(craftAsrMsg.positions[i]);
	}

	switch (type) {
	case 0: {
		LifeSkillItem.sReturn = 1;
	}
	case 1: {
		LifeSkillItem.sReturn = 0;
	}
	case 2: {
		LifeSkillItem.sReturn = static_cast<short>(craftAsrMsg.extraParam);
		break;
	}
	case 3: {
		LifeSkillItem.sReturn = static_cast<short>(craftAsrMsg.extraParam);
		break;
	}
	}

	Cmd_LifeSkillItemAsR(type, &LifeSkillItem);
}

void CCharacter::Handle_StoreCommand(std::uint16_t usCmd, Corsairs::Net::RPacket& pk) {
	CCharacter* pMainCha = GetPlyMainCha();
	if (!pMainCha->IsStoreEnable()) {
		return;
	}
	lua_getglobal(g_pLuaState, "operateIGS");
	if (!lua_isfunction(g_pLuaState, -1)) {
		lua_pop(g_pLuaState, 1);
		return;
	}

	extern luabridge::LuaRef BuildStoreActionTable(lua_State*, Corsairs::Net::RPacket&);
	luabridge::LuaRef action = BuildStoreActionTable(g_pLuaState, pk);

	luabridge::push(g_pLuaState, static_cast<CCharacter*>(this));
	action.push(g_pLuaState);
	int nStatus = lua_pcall(g_pLuaState, 2, 0, 0);
	lua_settop(g_pLuaState, 0);

	if (usCmd == CMD_CM_STORE_CLOSE) {
		CCharacter* pMainCha = GetPlyMainCha();
		pMainCha->SetStoreEnable(false);
		pMainCha->ForgeAction(false);
	}
}

void CCharacter::Handle_TigerStart(const Corsairs::Net::Msg::CmTigerStartMessage& msg) {
	m_sTigerSel[0] = (msg.sel1 > 0) ? 1 : 0;
	m_sTigerSel[1] = (msg.sel2 > 0) ? 1 : 0;
	m_sTigerSel[2] = (msg.sel3 > 0) ? 1 : 0;

	CCharacter* pCha = m_submap->FindCharacter(static_cast<DWORD>(msg.npcId), GetShape().Centre);
	if (pCha == NULL)
		return;

	CCharacter* pMainCha = GetPlyMainCha();
	pMainCha->DoTigerScript("TigerStart");
}

void CCharacter::Handle_TigerStop(const Corsairs::Net::Msg::CmTigerStopMessage& msg) {
	CCharacter* pCha = m_submap->FindCharacter(static_cast<DWORD>(msg.npcId), GetShape().Centre);
	if (pCha == NULL)
		return;

	CCharacter* pMainCha = GetPlyMainCha();
	short sNum = static_cast<short>(msg.num);

	if (sNum < 1 || sNum > 3) {
		pMainCha->ForgeAction(false);
		memset(m_sTigerItemID, 0, sizeof(m_sTigerItemID));
		memset(m_sTigerSel, 0, sizeof(m_sTigerSel));
		return;
	}

	short sIndex = 3 * (sNum - 1);
	bool bSucc = true;
	//  :   
	int64_t ids[3];
	for (int i = 0; i < 3; i++) {
		if (pMainCha->m_sTigerItemID[sIndex] <= 0) {
			bSucc = false;
		}
		ids[i] = pMainCha->m_sTigerItemID[sIndex++];
	}
	auto wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McTigerItemIdMessage{
		static_cast<int64_t>(sNum), ids[0], ids[1], ids[2]
	});
	ReflectINFof(this, wpk);

	if (bSucc) {
		if (sNum == 3) {
			pMainCha->DoTigerScript("TigerStop");
			memset(m_sTigerItemID, 0, sizeof(m_sTigerItemID));
			memset(m_sTigerSel, 0, sizeof(m_sTigerSel));
		}
	}
}

void CCharacter::Handle_VolunteerOpen(const Corsairs::Net::Msg::CmVolunteerOpenMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	short sNum = static_cast<short>(cmMsg.num);

	int nVolNum = g_pGameApp->GetVolNum();
	int nStart = 0;
	short sRetNum = (nVolNum - nStart < sNum) ? (nVolNum - nStart) : sNum;
	if (sRetNum < 0)
		sRetNum = 0;
	short sPageNum = (nVolNum % sNum == 0) ? (nVolNum / sNum) : (nVolNum / sNum + 1);

	char chState = (pMainCha->IsVolunteer() ? 1 : 0);
	//  :   
	Corsairs::Net::Msg::McVolunteerOpenMessage openMsg;
	openMsg.state = static_cast<int64_t>(chState);
	openMsg.pageTotal = static_cast<int64_t>(sPageNum);
	for (int i = 0; i < sRetNum; i++) {
		SVolunteer* pVolunteer = g_pGameApp->GetVolInfo(nStart + i);
		openMsg.volunteers.push_back({
			pVolunteer->szName, pVolunteer->lLevel, pVolunteer->lJob, pVolunteer->szMapName
		});
	}
	auto packet = Corsairs::Net::Msg::serialize(openMsg);
	ReflectINFof(this, packet);
}

void CCharacter::Handle_VolunteerList(const Corsairs::Net::Msg::CmVolunteerListMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	short sPage = static_cast<short>(cmMsg.page);
	short sNum = static_cast<short>(cmMsg.num);

	int nVolNum = g_pGameApp->GetVolNum();
	int nStart = (sPage - 1) * sNum;
	short sRetNum = (nVolNum - nStart < sNum) ? (nVolNum - nStart) : sNum;
	if (sRetNum < 0)
		sRetNum = 0;
	short sPageNum = (nVolNum % sNum == 0) ? (nVolNum / sNum) : (nVolNum / sNum + 1);

	//  :  
	Corsairs::Net::Msg::McVolunteerListMessage listMsg;
	listMsg.pageTotal = static_cast<int64_t>(sPageNum);
	listMsg.page = static_cast<int64_t>(sPage);
	for (int i = 0; i < sRetNum; i++) {
		SVolunteer* pVolunteer = g_pGameApp->GetVolInfo(nStart + i);
		listMsg.volunteers.push_back({
			pVolunteer->szName, pVolunteer->lLevel, pVolunteer->lJob, pVolunteer->szMapName
		});
	}
	auto packet = Corsairs::Net::Msg::serialize(listMsg);
	ReflectINFof(this, packet);
}

void CCharacter::Handle_VolunteerSel(const Corsairs::Net::Msg::CmVolunteerSelMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	if (pMainCha->GetLevel() < 8) {
		pMainCha->PopupNotice("Only players lv8 and above can request party!");
		return;
	}

	std::string szName = cmMsg.name;
	CCharacter* pTarCha = FindVolunteer(szName.c_str());
	if (!pTarCha) {
		//pMainCha->SystemNotice("%s !", szName);
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
		return;
	}

	if (pTarCha == pMainCha) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00013));
		return;
	}

	if (strcmp(pTarCha->GetPlyCtrlCha()->GetSubMap()->GetName(), GetPlyCtrlCha()->GetSubMap()->GetName())) {
		//pMainCha->SystemNotice(", !");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00014));
		return;
	}

	if (!(GetPlyCtrlCha()->GetSubMap()->GetMapRes()->CanTeam())) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00015));
		return;
	}

	//pMainCha->SystemNotice(",!");
	pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00016));

	//  :  
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McVolunteerAskMessage{pMainCha->GetName()});
	pTarCha->ReflectINFof(pTarCha, packet);
}

void CCharacter::Handle_VolunteerAsr(const Corsairs::Net::Msg::CmVolunteerAsrMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	short sRet = static_cast<short>(cmMsg.ret);
	std::string szName = cmMsg.name;
	CCharacter* pSrcCha = g_pGameApp->FindChaByName(szName.c_str());
	if (!pSrcCha) {
		//pMainCha->SystemNotice("%s !", szName);
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
		return;
	}

	if (sRet == 0) {
		//pSrcCha->SystemNotice("%s !", pMainCha->GetName());
		pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00018), pMainCha->GetName());
		return;
	}

	auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpTeamCreateMessage{
		std::string(pSrcCha->GetName()), std::string(pMainCha->GetName())
	});
	pMainCha->ReflectINFof(pMainCha, l_wpk);
}

void CCharacter::Handle_KitbagTempSync() {
	CCharacter* pMainCha = GetPlyMainCha();

	if (!pMainCha->m_pCKitbagTmp) {
		return;
	}

	Corsairs::Net::Msg::McKitbagTempSyncMessage msg;
	msg.kitbag = pMainCha->BuildKitbagInfo(*(pMainCha->m_pCKitbagTmp), enumSYN_KITBAG_INIT);
	auto pkret = Corsairs::Net::Msg::serialize(msg);
	pMainCha->ReflectINFof(pMainCha, pkret);
}

void CCharacter::Handle_ItemLockAsk(const Corsairs::Net::Msg::CmItemLockAskMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	CPlayer* pCPly = GetPlayer();

	if (pMainCha) {
		if (pMainCha->m_CKitbag.IsLock() || pMainCha->m_CKitbag.IsPwdLocked() || pCPly->GetStallData() || pCPly->
			GetMainCha()->GetTradeData()) {
			SystemNotice("Bag is currently locked.");
			return;
		}

		char chPosType = static_cast<char>(cmMsg.slot);
		SItemGrid* item = pMainCha->m_CKitbag.GetGridContByID(chPosType);
		if (item) {
			CItemRecord* pCItemRec = GetItemRecordInfo(item->sID);
			if (pCItemRec) {
				CPlayer* pPlayer = pMainCha->GetPlayer();
				if (pPlayer) {
					item->dwDBID = 1;
					this->m_CKitbag.SetChangeFlag();
					this->SynKitbagNew(enumSYN_KITBAG_SWITCH);
					auto rpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McItemLockAsrMessage{1});
					this->ReflectINFof(pMainCha, rpk);
					return;
				};
			};
		};
	};
	auto rpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McItemLockAsrMessage{0});
	pMainCha->ReflectINFof(pMainCha, rpk);
}

void CCharacter::Handle_GameRequestPin(const Corsairs::Net::Msg::CmGameRequestPinMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	if (!pMainCha)
		return;

	if (requestType == NULL)
		return;

	if (!IsReqPosEqualRealPos()) {
		requestType = NULL;
		return;
	}

	std::string szPwd = cmMsg.password;
	if (szPwd.empty())
		return;

	CPlayer* pCply = pMainCha->GetPlayer();
	const char* szPwd2 = pCply->GetPassword();
	if ((szPwd2[0] == 0) || (!strcmp(szPwd.c_str(), szPwd2))) {
		auto pinRet = g_luaAPI.CallR<int>("HandlePinRequest", static_cast<CCharacter*>(this), (int)requestType);
		if (!pinRet.value_or(0))
			return;
	}
	else {
		pMainCha->PopupNotice(RES_STRING(GM_CHARACTERPRL_CPP_00010));
	}
}

void CCharacter::Handle_MasterInvite(const Corsairs::Net::Msg::CmMasterInviteMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	std::string szName = cmMsg.name;
	DWORD dwCharID = static_cast<DWORD>(cmMsg.chaId);

	if (IsBoat()) {
		//SystemNotice("!");
		SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00019));
		return;
	}

	CCharacter* pTarCha = pMainCha->GetSubMap()->FindCharacter(dwCharID, pMainCha->GetShape().Centre);
	if (!pTarCha) {
		//pMainCha->SystemNotice("%s !", szName);
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
		return;
	}

	if (pTarCha->GetLevel() < 41) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00017));
		return;
	}

	if (pMainCha->GetLevel() > 40) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00020));
		return;
	}

	if (pMainCha->GetMasterDBID() != 0) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00021));
		return;
	}

	if (pTarCha->IsInvited()) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00022));
		return;
	}
	if (!pTarCha->GetPlayer()->CanReceiveRequests()) {
		pMainCha->SystemNotice("%s is currently offline. Unable to send request!", pMainCha->GetName());
		return;
	}

	pTarCha->SetInvited(true);

	//  :  
	auto packet = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McMasterAskMessage{pMainCha->GetName(), pMainCha->GetID()});
	pTarCha->ReflectINFof(pTarCha, packet);
}

void CCharacter::Handle_MasterAsr(const Corsairs::Net::Msg::CmMasterAsrMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	short sRet = static_cast<short>(cmMsg.agree);
	std::string szName = cmMsg.name;
	DWORD dwCharID = static_cast<DWORD>(cmMsg.chaId);

	pMainCha->SetInvited(false);

	if (IsBoat()) {
		//SystemNotice("!");
		SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00023));
		return;
	}

	CCharacter* pSrcCha = pMainCha->GetSubMap()->FindCharacter(dwCharID, pMainCha->GetShape().Centre);
	if (!pSrcCha) {
		//pMainCha->SystemNotice("%s !", szName);
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00012), szName.c_str());
		return;
	}

	if (pMainCha->GetLevel() < 41) {
		//pSrcCha->SystemNotice("!");
		pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00017));
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00024));
		return;
	}

	if (pSrcCha->GetLevel() > 40) {
		//pSrcCha->SystemNotice("!");
		pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00020));
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00025));
		return;
	}

	if (sRet == 0) {
		//pSrcCha->SystemNotice("%s !", pMainCha->GetName());
		pSrcCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00026), pMainCha->GetName());
		return;
	}

	//  :   - (  )
	auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpMasterCreateMessage{
		pSrcCha->GetName(), pSrcCha->GetPlayer()->GetDBChaId(),
		pMainCha->GetName(), pMainCha->GetPlayer()->GetDBChaId()
	});
	pMainCha->ReflectINFof(pMainCha, l_wpk);
}

void CCharacter::Handle_MasterDel(const Corsairs::Net::Msg::CmMasterDelMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	std::string szName = cmMsg.name;
	std::uint32_t ulChaID = static_cast<std::uint32_t>(cmMsg.chaId);

	if (pMainCha->GetLevel() > 40) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00027));
		return;
	}

	long lDelMoney = 0; //* pMainCha->GetLevel();
	if (!pMainCha->HasMoney(lDelMoney)) {
		//pMainCha->SystemNotice("!");
		pMainCha->SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00028));
		return;
	}
	//pMainCha->TakeMoney("", lDelMoney);
	//pMainCha->TakeMoney(RES_STRING(GM_CHARSCRIPT_CPP_00001), lDelMoney);
	pMainCha->SystemNotice("Your Mentor Deleted Successfully ");

	//  :  
	auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpMasterDelMessage{
		pMainCha->GetName(), pMainCha->GetPlayer()->GetDBChaId(),
		std::string(szName), static_cast<int64_t>(ulChaID)
	});
	pMainCha->ReflectINFof(pMainCha, l_wpk);
}

void CCharacter::Handle_PrenticeDel(const Corsairs::Net::Msg::CmPrenticeDelMessage& cmMsg) {
	CCharacter* pMainCha = GetPlyMainCha();
	std::string szName = cmMsg.name;
	std::uint32_t ulChaID = static_cast<std::uint32_t>(cmMsg.chaId);

	//long lDelMoney = 10000 * pMainCha->GetLevel();
	//if(!pMainCha->HasMoney(lDelMoney))
	//{
	//	pMainCha->SystemNotice("!");
	//	break;
	//}
	//pMainCha->TakeMoney("", lDelMoney);
	long lCredit = (long)pMainCha->GetCredit(); //- 5 * pMainCha->GetLevel();
	if (lCredit < 0) {
		lCredit = 0;
	}
	pMainCha->SetCredit(lCredit);
	pMainCha->SynAttr(enumATTRSYN_TASK);
	//pMainCha->SystemNotice("!");
	pMainCha->SystemNotice("Your Disciple Deleted Successfully ");

	//  :  
	auto l_wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::MpMasterDelMessage{
		std::string(szName), static_cast<int64_t>(ulChaID),
		pMainCha->GetName(), pMainCha->GetPlayer()->GetDBChaId()
	});
	pMainCha->ReflectINFof(pMainCha, l_wpk);
}

void CCharacter::BeginAction(const Corsairs::Net::Msg::CmBeginActionMessage& msg) {
	const long clPing = 300;

	if (!IsLiveing()) {
		return;
	}
	if (GetPlayer()->GetCtrlCha() == this && !GetSubMap()) {
		return;
	}

	std::uint32_t ulPacketId = static_cast<std::uint32_t>(msg.packetId);
	char chActionType = static_cast<char>(msg.actionType);


	m_ulPacketID = ulPacketId;
	switch (chActionType) {
	case enumACTION_MOVE: {
		if (!GetSubMap()) {
			return;
		}

		if (m_CAction.GetCurActionNo() >= 0) // 
		{
			FailedActionNoti(enumACTION_MOVE, enumFACTION_EXISTACT);
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035));
			break;
		}

		if (m_sPoseState == enumPoseSeat) {
			FailedActionNoti(enumACTION_MOVE, enumFACTION_EXISTACT);
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035));
			break;
		}
		ResetPosState();

		const auto& moveData = std::get<Corsairs::Net::Msg::CmActionMoveInputData>(msg.data);
		const char* pData = moveData.pathData.data();
		std::uint16_t ulTurnNum = static_cast<std::uint16_t>(moveData.pathData.size());
		Corsairs::Util::Point Path[defMOVE_INFLEXION_NUM];
		char chPointNum;
		if (!pData) {
			FailedActionNoti(enumACTION_MOVE, enumFACTION_MOVEPATH);
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00036));
			break;
		}
		if ((chPointNum = char(ulTurnNum / sizeof(Corsairs::Util::Point))) > defMOVE_INFLEXION_NUM) {
			FailedActionNoti(enumACTION_MOVE, enumFACTION_MOVEPATH);
			//SystemNotice("%d%d\n", ulTurnNum / sizeof(Corsairs::Util::Point), defMOVE_INFLEXION_NUM);
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00037), ulTurnNum / sizeof(Corsairs::Util::Point), defMOVE_INFLEXION_NUM);
			break;
		}
		memcpy(Path, pData, chPointNum * sizeof(Corsairs::Util::Point));

		Cmd_BeginMove((int16_t)m_dwPing, Path, chPointNum);
	}
	break;
	case enumACTION_SKILL: {
		if (GetPlyMainCha()->m_CKitbag.IsLock()) {
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00038));
			FailedActionNoti(enumACTION_SKILL, enumFACTION_ACTFORBID);
			break;
		}

		if (!GetSubMap()) {
			return;
		}

		if (GetPlayer()->GetBankNpc()) {
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00039));
			FailedActionNoti(enumACTION_SKILL, enumFACTION_ACTFORBID);
			break;
		}

		if (m_CAction.GetCurActionNo() >= 0) // 
		{
			FailedActionNoti(enumACTION_SKILL, enumFACTION_EXISTACT);
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035));
			break;
		}

		if (m_sPoseState == enumPoseSeat) {
			FailedActionNoti(enumACTION_SKILL, enumFACTION_EXISTACT);
			//SystemNotice("\n");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035));
			break;
		}
		ResetPosState();

		const auto& skillData = std::get<Corsairs::Net::Msg::CmActionSkillInputData>(msg.data);
		char chMove = static_cast<char>(skillData.chMove);
		if (chMove == 2) // 
		{
			// 
			Corsairs::Util::Point Path[defMOVE_INFLEXION_NUM];
			char chPointNum;
			const char* pData = skillData.pathData.data();
			std::uint16_t ulTurnNum = static_cast<std::uint16_t>(skillData.pathData.size());
			if (!pData) {
				FailedActionNoti(enumACTION_SKILL, enumFACTION_MOVEPATH);
				//SystemNotice("\n");
				SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00036));
				break;
			}

			if ((chPointNum = char(ulTurnNum / sizeof(Corsairs::Util::Point))) > defMOVE_INFLEXION_NUM) {
				FailedActionNoti(enumACTION_SKILL, enumFACTION_MOVEPATH);
				//SystemNotice("%d%d\n", ulTurnNum / sizeof(Corsairs::Util::Point), defMOVE_INFLEXION_NUM);
				SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00037), ulTurnNum / sizeof(Corsairs::Util::Point), defMOVE_INFLEXION_NUM);
				break;
			}
			memcpy(Path, pData, chPointNum * sizeof(Corsairs::Util::Point));
			// 
			std::uint32_t ulSkillID = static_cast<std::uint32_t>(skillData.skillId);
			std::int32_t lTarInfo1 = static_cast<std::int32_t>(skillData.tarInfo1);
			std::int32_t lTarInfo2 = static_cast<std::int32_t>(skillData.tarInfo2);

			CSkillRecord* pRec = GetSkillRecordInfo(ulSkillID);
			if (!pRec) {
				//LG( "", "%s1: %d[PacketID: %u]\n", GetName(), ulSkillID, ulPacketId);
				ToLogService("common", "character{}1skill inexistenceskill number: {}[PacketID: {}]", GetName(),
							 ulSkillID, ulPacketId);
				FailedActionNoti(enumACTION_SKILL, enumFACTION_NOSKILL);
				//LG( "", "%s2: %d[PacketID: %u]\n", GetName(), ulSkillID, ulPacketId);
				ToLogService("common", "character{}2skill inexistenceskill number: {}[PacketID: {}]", GetName(),
							 ulSkillID, ulPacketId);
				//SystemNotice(": %d\n", ulSkillID);
				SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00040), ulSkillID);
				break;
			}
			Cmd_BeginSkill((int16_t)m_dwPing, Path, chPointNum, pRec, 1, lTarInfo1, lTarInfo2);
		}
		else {
			//SystemNotice("");
			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00041));
			break;
		}
	}
	break;
	case enumACTION_STOP_STATE: {
		if (!GetSubMap()) {
			return;
		}

		const auto& d = std::get<Corsairs::Net::Msg::CmActionStopStateData>(msg.data);
		int16_t sStateID = static_cast<int16_t>(d.stateId);

		CSkillStateRecord* pSSkillState = GetCSkillStateRecordInfo((std::uint8_t)sStateID);
		if (!pSSkillState)
			break;
		if (!pSSkillState->bCanCancel)
			break;
		DelSkillState((std::uint8_t)sStateID);
	}
	break;
	case enumACTION_LEAN: // 
	{
		if (!GetSubMap()) {
			return;
		}

		m_sPoseState = enumPoseLean;
		m_SSeat.chIsSeat = 0;

		m_SLean.ulPacketID = ulPacketId;
		const auto& d = std::get<Corsairs::Net::Msg::CmActionLeanData>(msg.data);
		m_SLean.lPose = static_cast<std::int32_t>(d.pose);
		m_SLean.lAngle = static_cast<std::int32_t>(d.angle);
		m_SLean.lPosX = static_cast<std::int32_t>(d.posX);
		m_SLean.lPosY = static_cast<std::int32_t>(d.posY);
		m_SLean.lHeight = static_cast<std::int32_t>(d.height);
		m_SLean.chState = 0;

		//  : /  std::variant
		{
			Corsairs::Net::Msg::McCharacterActionMessage actionMsg;
			actionMsg.worldId = m_ID;
			actionMsg.packetId = m_SLean.ulPacketID;
			actionMsg.actionType = Corsairs::Net::Msg::ActionType::LEAN;
			actionMsg.data = Corsairs::Net::Msg::ActionLeanData{
				m_SLean.chState, m_SLean.lPose, m_SLean.lAngle,
				m_SLean.lPosX, m_SLean.lPosY, m_SLean.lHeight
			};
			auto WtPk = Corsairs::Net::Msg::serialize(actionMsg);
			NotiChgToEyeshot(WtPk);
		}
		//

		// log
		//
	}
	break;
	///item picks
	case enumACTION_ITEM_PICK: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemPickData>(msg.data);
		std::int32_t lWorldID = static_cast<std::int32_t>(d.worldId);
		std::int32_t lHandle = static_cast<std::int32_t>(d.handle);

		int16_t sRet = Cmd_PickupItem(lWorldID, lHandle);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_THROW: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemThrowData>(msg.data);
		int16_t sGridID = static_cast<int16_t>(d.gridId);
		int16_t sNum = static_cast<int16_t>(d.num);
		std::int32_t lPosX = static_cast<std::int32_t>(d.posX);
		std::int32_t lPosY = static_cast<std::int32_t>(d.posY);

		int16_t sRet = Cmd_ThrowItem(0, sGridID, &sNum, lPosX, lPosY);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_USE: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemUseData>(msg.data);
		int16_t sFromGridID = static_cast<int16_t>(d.fromGridId);
		int16_t sToGridID = static_cast<int16_t>(d.toGridId);

		int16_t sRet = Cmd_UseItem(0, sFromGridID, 0, sToGridID);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_UNFIX: // 
	{
		m_CChaAttr.ResetChangeFlag();

		char chDir;
		std::int32_t lParam1, lParam2;

		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemUnfixData>(msg.data);
		char chLinkID = static_cast<char>(d.linkId);
		int16_t sGridID = static_cast<int16_t>(d.gridId);
		if (sGridID == -2) // 
		{
			chDir = 0;
			lParam1 = static_cast<std::int32_t>(d.param1);
			lParam2 = static_cast<std::int32_t>(d.param2);
		}
		else if (sGridID == -1) // 
		{
			chDir = 1;
			lParam1 = 0;
			lParam2 = -1;
		}
		else if (sGridID >= 0) // 
		{
			chDir = 1;
			lParam1 = 0;
			lParam2 = sGridID;
		}


		int16_t sUnfixNum = 0;
		int16_t sRet = Cmd_UnfixItem(chLinkID, &sUnfixNum, chDir, lParam1, lParam2);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_POS: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemPosData>(msg.data);
		int16_t sSrcGrid = static_cast<int16_t>(d.srcGrid);
		int16_t sSrcNum = static_cast<int16_t>(d.srcNum);
		int16_t sTarGrid = static_cast<int16_t>(d.tarGrid);

		int16_t sRet = Cmd_ItemSwitchPos(0, sSrcGrid, sSrcNum, sTarGrid);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_KITBAGTMP_DRAG: //
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemPosData>(msg.data);
		int16_t sSrcGrid = static_cast<int16_t>(d.srcGrid);
		int16_t sSrcNum = static_cast<int16_t>(d.srcNum);
		int16_t sTarGrid = static_cast<int16_t>(d.tarGrid);

		int16_t sRet = Cmd_DragItem(sSrcGrid, sSrcNum, sTarGrid);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_DELETE: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionItemDeleteData>(msg.data);
		int16_t sFromGridID = static_cast<int16_t>(d.fromGridId);

		int16_t sOptNum = 0;
		int16_t sRet = Cmd_DelItem(0, sFromGridID, &sOptNum);
		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_ITEM_INFO: // 
	{
		const auto& d = std::get<Corsairs::Net::Msg::CmActionViewItemData>(msg.data);
		ViewItemInfo(d);
	}
	break;
	case enumACTION_BANK: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionBankData>(msg.data);
		char chSrcType = static_cast<char>(d.srcType);
		int16_t sSrcGrid = static_cast<int16_t>(d.srcGrid);
		int16_t sSrcNum = static_cast<int16_t>(d.srcNum);
		char chTarType = static_cast<char>(d.tarType);
		int16_t sTarGrid = static_cast<int16_t>(d.tarGrid);
		int16_t sRet;

		sRet = Cmd_BankOper(chSrcType, sSrcGrid, sSrcNum, chTarType, sTarGrid);

		if (sRet != enumITEMOPT_SUCCESS)
			ItemOprateFailed(sRet);
	}
	break;
	case enumACTION_CLOSE_BANK: {
		GetPlayer()->CloseBank();
	}
	break;
	case enumACTION_REQUESTGUILDBANK: {
		if (GetGuildID() == 0) {
			return;
		}
		GetPlayer()->OpenGuildBank();
		GetPlayer()->GetGuildGold();
		break;
	}
	case enumACTION_UPDATEGUILDLOGS: {
		int guildID = GetGuildID();
		if (guildID == 0) {
			return;
		}

		std::vector<CTableGuild::BankLog> logs = game_db.GetGuildLog(guildID);
		Corsairs::Net::Msg::McUpdateGuildLogsMessage logsMsg;
		logsMsg.worldId = m_ID;
		logsMsg.packetId = ulPacketId;
		logsMsg.totalSize = logs.size();
		for (int i = 1; i <= 13; i++) {
			if (i > (int)logs.size()) {
				logsMsg.terminated = true;
				break;
			}
			Corsairs::Net::Msg::GuildBankLogEntry e;
			e.type = logs.at(logs.size() - i).type;
			e.time = logs.at(logs.size() - i).time;
			e.parameter = logs.at(logs.size() - i).parameter;
			e.quantity = logs.at(logs.size() - i).quantity;
			e.userId = logs.at(logs.size() - i).userID;
			logsMsg.logs.push_back(e);
		}
		auto WtPk = Corsairs::Net::Msg::serialize(logsMsg);

		ReflectINFof(this, WtPk);
		break;
	}
	case enumACTION_REQUESTGUILDLOGS: {
		int guildID = GetGuildID();
		if (guildID == 0) {
			return;
		}
		std::vector<CTableGuild::BankLog> logs = game_db.GetGuildLog(guildID);

		const auto& reqData = std::get<Corsairs::Net::Msg::CmActionRequestGuildLogsData>(msg.data);
		std::uint16_t curSize = static_cast<std::uint16_t>(reqData.curSize);

		Corsairs::Net::Msg::McRequestGuildLogsMessage respMsg;
		respMsg.worldId = m_ID;
		respMsg.packetId = ulPacketId;
		for (int i = 1; i <= 13; i++) {
			if ((int)(curSize + i) > (int)logs.size()) {
				respMsg.terminated = true;
				break;
			}
			Corsairs::Net::Msg::GuildBankLogEntry e;
			e.type = logs.at(logs.size() - curSize - i).type;
			e.time = logs.at(logs.size() - curSize - i).time;
			e.parameter = logs.at(logs.size() - curSize - i).parameter;
			e.quantity = logs.at(logs.size() - curSize - i).quantity;
			e.userId = logs.at(logs.size() - curSize - i).userID;
			respMsg.logs.push_back(e);
		}
		auto WtPk = Corsairs::Net::Msg::serialize(respMsg);

		ReflectINFof(this, WtPk);
		break;
	}
	case enumACTION_SHORTCUT: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionShortcutData>(msg.data);
		char chIndex = static_cast<char>(d.index);
		char chType = static_cast<char>(d.type);
		short sGrid = static_cast<short>(d.grid);

		if (chIndex < 0 || chIndex >= SHORT_CUT_NUM)
			break;
		m_CShortcut.chType[chIndex] = chType;
		m_CShortcut.byGridID[chIndex] = sGrid;
	}
	break;
	case enumACTION_LOOK: {
		//m_SChaPart.sTypeID = pk.ReadInt64();
		//for (int i = 0; i < enumEQUIP_NUM; i++)
		//	m_SChaPart.SLink[i].sID = pk.ReadInt64();

		//// 
		//Corsairs::Net::WPacket WtPk	=g_gmsvr->GetWPacket();
		//WtPk.WriteCmd(CMD_MC_NOTIACTION);	//
		//WtPk.WriteInt64(m_ID);
		//WtPk.WriteInt64(ulPacketId);
		//WtPk.WriteInt64(enumACTION_LOOK);
		//WtPk.WriteInt64(m_SChaPart.sTypeID);
		//for (int i = 0; i < enumEQUIP_NUM; i++)
		//	WtPk.WriteInt64(m_SChaPart.sLink[i]);
		//NotiChgToEyeshot(WtPk);//
	}
	break;
	case enumACTION_TEMP: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionTempData>(msg.data);
		m_STempChaPart.sItemID = static_cast<short>(d.itemId);
		m_STempChaPart.sPartID = static_cast<short>(d.partId);

		//  :     std::variant
		{
			Corsairs::Net::Msg::McCharacterActionMessage actionMsg;
			actionMsg.worldId = m_ID;
			actionMsg.packetId = ulPacketId;
			actionMsg.actionType = Corsairs::Net::Msg::ActionType::TEMP;
			actionMsg.data = Corsairs::Net::Msg::ActionTempData{
				static_cast<int64_t>(m_STempChaPart.sItemID),
				static_cast<int64_t>(m_STempChaPart.sPartID)
			};
			auto WtPk = Corsairs::Net::Msg::serialize(actionMsg);
			NotiChgToEyeshot(WtPk);
		}
	}
	break;
	case enumACTION_EVENT: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionEventData>(msg.data);
		std::int32_t lID = static_cast<std::int32_t>(d.worldId);
		std::int32_t lHandle = static_cast<std::int32_t>(d.handle);
		Entity* pCObj = g_pGameApp->IsLiveingEntity(lID, lHandle);
		if (!pCObj) {
			break;
		}
		std::uint16_t usEventID = static_cast<std::uint16_t>(d.eventId);
		ExecuteEvent(pCObj, usEventID);
	}
	break;
	case enumACTION_FACE: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionFaceData>(msg.data);
		int16_t sAngle = static_cast<int16_t>(d.angle);
		int16_t sPose = static_cast<int16_t>(d.pose);

		//  :    std::variant
		{
			Corsairs::Net::Msg::McCharacterActionMessage actionMsg;
			actionMsg.worldId = m_ID;
			actionMsg.packetId = ulPacketId;
			actionMsg.actionType = Corsairs::Net::Msg::ActionType::FACE;
			actionMsg.data = Corsairs::Net::Msg::ActionFaceData{static_cast<int64_t>(sAngle), static_cast<int64_t>(sPose)};
			auto WtPk = Corsairs::Net::Msg::serialize(actionMsg);
			NotiChgToEyeshot(WtPk);
		}
	}
	break;
	case enumACTION_SKILL_POSE: {
		if (!GetSubMap()) {
			return;
		}

		if (IsBoat())
			break;
		if (GetMoveState() == enumMSTATE_ON || GetFightState() == enumFSTATE_ON || !GetActControl(ActControl::MOVE))
			break;

		const auto& d = std::get<Corsairs::Net::Msg::CmActionFaceData>(msg.data);
		int16_t sAngle = static_cast<int16_t>(d.angle);
		int16_t sPose = static_cast<int16_t>(d.pose);

		//  :    std::variant
		Corsairs::Net::Msg::McCharacterActionMessage actionMsg;
		actionMsg.worldId = m_ID;
		actionMsg.packetId = ulPacketId;
		actionMsg.actionType = Corsairs::Net::Msg::ActionType::SKILL_POSE;
		actionMsg.data = Corsairs::Net::Msg::ActionFaceData{static_cast<int64_t>(sAngle), static_cast<int64_t>(sPose)};
		auto WtPk = Corsairs::Net::Msg::serialize(actionMsg);
		NotiChgToEyeshot(WtPk);

		bool bToSeat = g_IsSeatPose(sPose);
		if ((bToSeat && m_SSeat.chIsSeat) || (!bToSeat && !m_SSeat.chIsSeat))
			break;

		// 
		std::uint32_t ulSkillID = 202;
		CSkillRecord* pCSkill = GetSkillRecordInfo(ulSkillID);
		if (!pCSkill) {
			break;
		}

		if (bToSeat) // 
		{
			m_SSeat.chIsSeat = 1;
			m_SSeat.sAngle = sAngle;
			m_SSeat.sPose = sPose;
			g_luaAPI.Call(pCSkill->szActive.c_str(), static_cast<CCharacter*>(this), 1);
		}
		else //
		{
			m_SSeat.chIsSeat = 0;
			g_luaAPI.Call(pCSkill->szInactive.c_str(), static_cast<CCharacter*>(this), 1);
		}
		if (bToSeat)
			m_sPoseState = enumPoseSeat;
		else
			m_sPoseState = enumPoseStand;
	}
	break;
	case enumACTION_PK_CTRL: {
		const auto& d = std::get<Corsairs::Net::Msg::CmActionPkCtrlData>(msg.data);
		if (d.ctrl)
			Cmd_SetInPK();
		else
			Cmd_SetInPK(false);
		SynPKCtrl();
	}
	break;
	default:
		break;
	}
}


//  : 
void CCharacter::Cmd_ChangeHair(Corsairs::Net::RPacket& pk) {
	char szRes[128];

	Corsairs::Net::Msg::CmUpdateHairMessage hairMsg;
	Corsairs::Net::Msg::deserialize(pk, hairMsg);
	short sScriptID = static_cast<short>(hairMsg.scriptId);
	int64_t gridLocs[4] = {hairMsg.gridLoc0, hairMsg.gridLoc1, hairMsg.gridLoc2, hairMsg.gridLoc3};

	TradeAction(false);
	HairAction(false);

	if (sScriptID == 0) {
		return;
	}

	if (m_CKitbag.IsPwdLocked()) {
		std::snprintf(szRes, sizeof(szRes), "%s", RES_STRING(GM_CHARACTERPRL_CPP_00042));
		Prl_ChangeHairResult(0, szRes);
		return;
	}

	HairRecord* pHair = GetHairRecordInfo(sScriptID);
	if (!pHair) {
		std::snprintf(szRes, sizeof(szRes), RES_STRING(GM_CHARACTERPRL_CPP_00043), sScriptID);
		Prl_ChangeHairResult(0, szRes);
		return;
	}

	short sValidCnt = 0;
	short sValidGrid[Corsairs::Common::Character::kHairMaxNeedItems][3];
	int gridIdx = 0;

	for (short i = 0; i < Corsairs::Common::Character::kHairMaxNeedItems; i++) {
		short sNeedItemID = (short)(pHair->NeedItems[i].Id);
		if (sNeedItemID > 0) {
			BOOL bOK = TRUE;
			short sGridLoc = (gridIdx < 4) ? static_cast<short>(gridLocs[gridIdx++]) : -1;
			if (sGridLoc == -1) bOK = FALSE;

			if (bOK) {
				// 
				short sNowItemID = m_CKitbag.GetID(sGridLoc);
				if (sNowItemID != sNeedItemID) {
					bOK = FALSE;
				}
			}

			if (!bOK) {
				std::snprintf(szRes, sizeof(szRes), "%s", RES_STRING(GM_CHARACTERPRL_CPP_00044));
				Prl_ChangeHairResult(0, szRes);
				return;
			}
			sValidGrid[sValidCnt][0] = sGridLoc;
			sValidGrid[sValidCnt][1] = sNeedItemID;
			sValidGrid[sValidCnt][2] = (short)(pHair->NeedItems[i].Count); // 
			sValidCnt++;
		}
	}


	// , 
	m_CKitbag.SetChangeFlag(false);
	/*if(!TakeMoney("", pHair->Cost))
	{
		SystemNotice(", !");
		return;
	}*/
	if (!TakeMoney(RES_STRING(GM_CHARACTERPRL_CPP_00045), pHair->Cost)) {
		SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00046));
		return;
	}

	SItemGrid item;
	for (short i = 0; i < sValidCnt; i++) {
		item.sID = sValidGrid[i][1];
		item.sNum = sValidGrid[i][2];

		short sRet = KbPopItem(true, false, &item, sValidGrid[i][0]);
		if (sRet != enumKBACT_SUCCESS) {
			//SystemNotice(", !");

			SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00047));
			return;
		}
	}

	// 
	SynKitbagNew(enumSYN_KITBAG_FROM_NPC);

	// , 

	SetLookChangeFlag(true);
	// 10%
	if (rand() % 100 < 10 && pHair->GetFailItemCount() > 0) {
		int nRandFail = rand() % pHair->GetFailItemCount();
		short sFailHair = (short)(pHair->FailItemIds[nRandFail]);
		m_SChaPart.sHairID = sFailHair;
		//SystemNotice(", !");
		SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00048));
		Prl_ChangeHairResult(sScriptID, "fail", true);
	}
	else {
		// , 
		m_SChaPart.sHairID = (short)(pHair->ResultItemId); // 
		Prl_ChangeHairResult(sScriptID, "ok", true);
	}

	// 
	if (g_Config.m_bBlindChaos && IsPlayerCha() && IsPKSilver())
		SynLook(LOOK_SELF, true); // sync to self (changing hair)
	else
		SynLook();
}

// 
// 1 : ID, 0
// 2 : 
void CCharacter::Prl_ChangeHairResult(int nScriptID, const char* szReason, BOOL bNoticeAll) {
	//  :   
	auto wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McUpdateHairResMessage{
		GetID(), static_cast<int64_t>(nScriptID), szReason
	});
	if (bNoticeAll) {
		NotiChgToEyeshot(wpk);
	}
	else {
		ReflectINFof(this, wpk);
	}
}

// 
void CCharacter::Prl_OpenHair() {
	HairAction(true);

	//  :  UI 
	auto wpk = Corsairs::Net::Msg::serializeMcOpenHairCutCmd();
	ReflectINFof(this, wpk);
}
