#pragma once

#include "NetIF.h"
#include "PacketCmd.h"

/****************************************************************
		Packet
		:
		:
		:
		"NetPkXXX.cpp"(:Pk,cpp)
*****************************************************************/
extern void CM_GUILD_PUTNAME(bool confirm, std::string_view guildname, std::string_view passwd); //
extern void CM_GUILD_TRYFOR(std::uint32_t guildid); //
extern void CM_GUILD_TRYFORCFM(bool confirm); //confirm =true;
extern void CM_GUILD_LISTTRYPLAYER(); //CMD_CM_GUILD_LISTTRYMEMBER
extern void CM_GUILD_APPROVE(std::uint32_t chaid); //
extern void CM_GUILD_REJECT(std::uint32_t chaid);
extern void CM_GUILD_KICK(std::uint32_t chaid);
extern void CM_GUILD_LEAVE();
extern void CM_GUILD_DISBAND(std::string_view passwd);
extern void CM_GUILD_MOTTO(std::string_view motto);
extern void CM_GUILD_CHALL(BYTE byLevel, DWORD dwMoney);
extern void CM_GUILD_LEIZHU(BYTE byLevel, DWORD dwMoney);


/****************************************************************
		Packet
		:
		:
		:Packet
		"NetXXX.cpp"(:Pk,NetPkXXX,)
*****************************************************************/
extern void NetMC_GUILD_GETNAME();
extern void NetMC_LISTGUILD_BEGIN();
extern void NetMC_LISTGUILD_END();
extern void NetMC_LISTGUILD(std::uint32_t id, std::string_view name, std::string_view motto, std::string_view leadername, std::uint16_t memtotal, std::int64_t exp);
extern void NetMC_GUILD_TRYFORCFM(std::string_view oldgldname);
extern void NetMC_LISTTRYPLAYER_BEGIN(std::uint32_t gldid, std::string_view gldname, std::string_view motto, char stat, std::string_view ldrname,
									  std::uint16_t memnum, std::uint16_t maxmem, std::int64_t exp, std::uint32_t remain);
extern void NetMC_LISTTRYPLAYER_END();
extern void NetMC_LISTTRYPLAYER(std::uint32_t chaid, std::string_view chaname, std::string_view job, std::uint16_t degree);

extern void NetPC_GUILD_ONLINE(std::uint32_t chaid);
extern void NetPC_GUILD_OFFLINE(std::uint32_t chaid);
extern void NetPC_GUILD_UPDATEPERM(std::uint32_t chaid, std::uint32_t perm);
extern void NetPC_GUILD_START_BEGIN(std::uint32_t guildid, std::string_view guildname, std::uint32_t leaderid);
extern void NetPC_GUILD_START_END();
extern void NetPC_GUILD_START(bool online, std::uint32_t chaid, std::string_view chaname, std::string_view motto, std::string_view job, std::uint16_t degree,
							  std::uint16_t icon, std::uint32_t permission);
extern void NetPC_GUILD_STOP();

extern void NetPC_GUILD_ADD(bool online, std::uint32_t chaid, std::string_view chaname, std::string_view motto, std::string_view job, std::uint16_t degree,
							std::uint16_t icon, std::uint32_t permission);
extern void NetPC_GUILD_DEL(std::uint32_t chaid);

extern void NetMC_GUILD_MOTTO(std::string_view motto);
extern void NetMC_GUILD_INFO(DWORD dwCharID, DWORD dwGuildID, const char szGuildName[], const char szGuildMotto[],
							 std::uint32_t chGuildPermission);
extern void NetMC_GUILD_CHALLINFO(const NET_GUILD_CHALLINFO& Info);


/****************************************************************
		Packet
		:
		:
		:NetIF::HandlePacketMessage
		"NetPkXXX.cpp"(:Pk,cpp)
*****************************************************************/
extern BOOL MC_GUILD_GETNAME(LPRPACKET pk);
extern bool g_listguild_begin;
extern BOOL MC_LISTGUILD(LPRPACKET pk);
extern BOOL MC_GUILD_TRYFORCFM(LPRPACKET pk);
extern BOOL MC_GUILD_LISTTRYPLAYER(LPRPACKET pk);
extern bool g_guild_start_begin;
extern BOOL PC_GUILD(LPRPACKET pk);
extern BOOL PC_GUILD_PERM(LPRPACKET pk);
extern BOOL MC_GUILD_MOTTO(LPRPACKET pk);
extern BOOL MC_GUILD_LEAVE(LPRPACKET pk);
extern BOOL MC_GUILD_KICK(LPRPACKET pk);
extern BOOL MC_GUILD_INFO(LPRPACKET pk);
extern BOOL MC_GUILD_LISTCHALL(LPRPACKET pk);
