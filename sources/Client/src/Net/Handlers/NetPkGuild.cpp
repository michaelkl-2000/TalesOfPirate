#include "stdafx.h"
#include "netguild.h"
#include "Network/NetCommand.h"
#include "uiguildmgr.h"
#include "uiglobalvar.h"
#include "CommandMessages.h"

using namespace std;

BOOL MC_GUILD_GETNAME(LPRPACKET pk) {
	NetMC_GUILD_GETNAME();
	return TRUE;
}

void CM_GUILD_PUTNAME(bool confirm, std::string_view guildname, std::string_view passwd) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildPutNameMessage{
		confirm ? 1 : 0,
		std::string(guildname), std::string(passwd)
	});
	g_NetIF->SendPacketMessage(pk);
}

bool g_listguild_begin = false;

BOOL MC_LISTGUILD(LPRPACKET pk) {
	//   McListGuildMessage (count-first)
	Corsairs::Net::Msg::McListGuildMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	if (!g_listguild_begin) {
		g_listguild_begin = true;
		NetMC_LISTGUILD_BEGIN();
	}
	for (const auto& e : msg.entries) {
		NetMC_LISTGUILD(static_cast<std::uint32_t>(e.guildId), e.guildName.c_str(), e.motto.c_str(),
						e.leaderName.c_str(), static_cast<std::uint16_t>(e.memberCount), e.exp);
	}
	if (msg.entries.size() < 20) {
		NetMC_LISTGUILD_END();
		g_listguild_begin = false;
	}
	return TRUE;
}

void CM_GUILD_TRYFOR(std::uint32_t guildid) //
{
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildTryForMessage{(int64_t)guildid});
	g_NetIF->SendPacketMessage(pk);
}

BOOL MC_GUILD_TRYFORCFM(LPRPACKET pk) {
	Corsairs::Net::Msg::McGuildTryForCfmMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	NetMC_GUILD_TRYFORCFM(msg.name.c_str());
	return TRUE;
}

void CM_GUILD_TRYFORCFM(bool confirm) //    
{
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildTryForCfmMessage{confirm ? 1 : 0});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_LISTTRYPLAYER() //   
{
	auto pk = Corsairs::Net::Msg::serializeCmGuildListTryPlayerCmd();
	g_NetIF->SendPacketMessage(pk);
}

BOOL MC_GUILD_LISTTRYPLAYER(LPRPACKET pk) {
	//   McGuildListTryPlayerMessage (count-first)
	Corsairs::Net::Msg::McGuildListTryPlayerMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	std::uint8_t l_stat = CGuildData::eState::normal;
	NetMC_LISTTRYPLAYER_BEGIN(static_cast<std::uint32_t>(msg.guildId), msg.guildName.c_str(), msg.motto.c_str(),
							  l_stat, msg.leaderName.c_str(), static_cast<std::uint16_t>(msg.memberTotal),
							  static_cast<std::uint16_t>(msg.maxMembers), msg.exp,
							  static_cast<std::uint32_t>(msg.reserved));

	for (const auto& p : msg.players) {
		NetMC_LISTTRYPLAYER(static_cast<std::uint32_t>(p.chaId), p.name.c_str(), p.job.c_str(),
							static_cast<std::uint16_t>(p.degree));
	}
	NetMC_LISTTRYPLAYER_END();
	return TRUE;
}

struct stGuildInfo {
	bool bOnline;
	std::uint32_t nChaID;
	char szChaName[65];
	char szMotto[257];
	char szJob[33];
	std::uint16_t sDegree;
	std::uint16_t sIcon;
	std::uint32_t sPreMission;
};


//bool g_guild_start_begin	=false;
vector<stGuildInfo> g_vecGuildInfo;

BOOL PC_GUILD_PERM(LPRPACKET pk) {
	Corsairs::Net::Msg::PcGuildPermMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	NetPC_GUILD_UPDATEPERM(msg.targetChaId, msg.permission);
	return true;
}

BOOL PC_GUILD(LPRPACKET pk) {
	//   PcGuildMessage (std::variant)
	Corsairs::Net::Msg::PcGuildMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);

	switch (msg.msg) {
	case MSG_GUILD_ONLINE: {
		NetPC_GUILD_ONLINE(std::get<Corsairs::Net::Msg::GuildChaIdData>(msg.data).chaId);
	}
	break;
	case MSG_GUILD_OFFLINE: {
		NetPC_GUILD_OFFLINE(std::get<Corsairs::Net::Msg::GuildChaIdData>(msg.data).chaId);
	}
	break;
	case MSG_GUILD_START: {
		auto& startData = std::get<Corsairs::Net::Msg::GuildStartData>(msg.data);
		std::uint8_t l_num = static_cast<std::uint8_t>(startData.members.size());
		std::uint32_t lPacketIndex = static_cast<std::uint32_t>(startData.packetIndex);
		static int nGuildCount = 0;

		if (lPacketIndex == 0 && l_num > 0) //  
		{
			NetPC_GUILD_START_BEGIN(static_cast<std::uint32_t>(startData.guildId),
									startData.guildName.c_str(),
									static_cast<std::uint32_t>(startData.leaderId));
		}
		for (std::uint8_t i = 0; i < l_num; ++i) {
			const auto& m = startData.members[i];

			stGuildInfo info;
			memset(&info, 0, sizeof(stGuildInfo));
			info.bOnline = m.online ? true : false;
			info.nChaID = static_cast<std::uint32_t>(m.chaId);
			info.sDegree = static_cast<std::uint16_t>(m.degree);
			info.sIcon = static_cast<std::uint16_t>(m.icon);
			info.sPreMission = static_cast<std::uint32_t>(m.permission);
			strncpy(info.szChaName, m.chaName.c_str(), sizeof(info.szChaName) - 1);
			strncpy(info.szJob, m.job.c_str(), sizeof(info.szJob) - 1);
			strncpy(info.szMotto, m.motto.c_str(), sizeof(info.szMotto) - 1);

			g_vecGuildInfo.push_back(info);
		}
		if (l_num < 20) //   
		{
			nGuildCount = 20 * lPacketIndex + l_num;
		}

		if (nGuildCount == (int)g_vecGuildInfo.size()) {
			for (int i = 0; i < nGuildCount; ++i) {
				NetPC_GUILD_START(g_vecGuildInfo[i].bOnline,
								  g_vecGuildInfo[i].nChaID,
								  g_vecGuildInfo[i].szChaName,
								  g_vecGuildInfo[i].szMotto,
								  g_vecGuildInfo[i].szJob,
								  g_vecGuildInfo[i].sDegree,
								  g_vecGuildInfo[i].sIcon,
								  g_vecGuildInfo[i].sPreMission);
			}
			NetPC_GUILD_START_END();
			g_vecGuildInfo.clear();
			nGuildCount = 0;
		}
	}
	break;
	case MSG_GUILD_STOP: {
		NetPC_GUILD_STOP();
	}
	break;
	case MSG_GUILD_ADD: {
		const auto& m = std::get<Corsairs::Net::Msg::GuildAddData>(msg.data).member;
		NetPC_GUILD_ADD(m.online ? true : false,
						static_cast<std::uint32_t>(m.chaId),
						m.chaName.c_str(), m.motto.c_str(), m.job.c_str(),
						static_cast<std::uint16_t>(m.degree), static_cast<std::uint16_t>(m.icon),
						static_cast<std::uint32_t>(m.permission));
	}
	break;
	case MSG_GUILD_DEL: {
		NetPC_GUILD_DEL(std::get<Corsairs::Net::Msg::GuildChaIdData>(msg.data).chaId);
	}
	break;
	default:
		break;
	}
	return TRUE;
}

void CM_GUILD_APPROVE(std::uint32_t chaid) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildApproveMessage{(int64_t)chaid});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_REJECT(std::uint32_t chaid) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildRejectMessage{(int64_t)chaid});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_KICK(std::uint32_t chaid) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildKickMessage{(int64_t)chaid});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_LEAVE() {
	auto pk = Corsairs::Net::Msg::serializeCmGuildLeaveCmd();
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_DISBAND(std::string_view passwd) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildDisbandMessage{std::string(passwd)});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_MOTTO(std::string_view motto) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildMottoMessage{std::string(motto)});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_CHALL(BYTE byLevel, DWORD dwMoney) {
	auto pk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::CmGuildChallMessage{
		(int64_t)byLevel, (int64_t)dwMoney
	});
	g_NetIF->SendPacketMessage(pk);
}

void CM_GUILD_LEIZHU(BYTE byLevel, DWORD dwMoney) {
	auto pk = Corsairs::Net::Msg::serialize(
		Corsairs::Net::Msg::CmGuildLeizhuMessage{(int64_t)byLevel, (int64_t)dwMoney});
	g_NetIF->SendPacketMessage(pk);
}

BOOL MC_GUILD_MOTTO(LPRPACKET pk) {
	Corsairs::Net::Msg::McGuildMottoMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	NetMC_GUILD_MOTTO(msg.motto.c_str());
	return TRUE;
}

BOOL MC_GUILD_LEAVE(LPRPACKET pk) {
	return TRUE;
}

BOOL MC_GUILD_KICK(LPRPACKET pk) {
	return TRUE;
}

BOOL MC_GUILD_INFO(LPRPACKET pk) {
	Corsairs::Net::Msg::McGuildInfoMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);
	NetMC_GUILD_INFO(msg.charId, msg.guildId, msg.guildName.c_str(), msg.guildMotto.c_str(), msg.guildPermission);
	return TRUE;
}

BOOL MC_GUILD_LISTCHALL(LPRPACKET pk) {
	Corsairs::Net::Msg::McGuildListChallMessage msg;
	Corsairs::Net::Msg::deserialize(pk, msg);

	//   legacy-  UI
	NET_GUILD_CHALLINFO Info;
	memset(&Info, 0, sizeof(Info));
	Info.byIsLeader = (BYTE)msg.isLeader;
	for (int i = 0; i < MAX_GUILD_CHALLLEVEL; i++) {
		Info.byLevel[i] = (BYTE)msg.entries[i].level;
		if (Info.byLevel[i]) {
			Info.byStart[i] = (BYTE)msg.entries[i].start;
			strncpy(Info.szGuild[i], msg.entries[i].guildName.c_str(), 64 - 1);
			strncpy(Info.szChall[i], msg.entries[i].challName.c_str(), 64 - 1);
			Info.dwMoney[i] = (DWORD)msg.entries[i].money;
		}
	}

	NetMC_GUILD_CHALLINFO(Info);
	return TRUE;
}
