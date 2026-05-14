#ifndef NETIF_H
#define NETIF_H

#include "CorsairsNet.h"
#include "Connection.h"
#include <bcrypt.h>
#include <wincrypt.h>

class CProCirculate;

//
#ifndef DS_DISCONN
#define DS_DISCONN  (0xFFFE)
#endif

//   ( Corsairs::Net::RPacket& / Corsairs::Net::WPacket&)
typedef Corsairs::Net::RPacket& LPRPACKET;
typedef Corsairs::Net::WPacket& LPWPACKET;
//    
using WPacket = Corsairs::Net::WPacket;
using RPacket = Corsairs::Net::RPacket;

struct lua_State;

extern inline int lua_HandleNetMessage(lua_State* L);

class NetIF : public Corsairs::Net::ITcpClientHandler, public Corsairs::Net::ICryptoProvider {
public:
	//    (Server  Client)
	BOOL HandlePacketMessage(LPRPACKET pk);
	//   (Client  Server)
	void SendPacketMessage(LPWPACKET pk);

	NetIF();
	virtual ~NetIF();

	//  ITcpClientHandler 
	void OnPacket(Corsairs::Net::RPacket& packet) override;
	void OnDisconnected(int reason) override;

	//  ICryptoProvider 
	bool IsActive() const override;
	bool Encrypt(uint8_t* ciphertext, int ciphertext_len,
				 const uint8_t* plaintext, int& len) override;
	bool Decrypt(uint8_t* data, int& len) override;

	//   API 

	bool IsConnected() {
		return m_connect.IsConnected();
	}

	int GetConnStat() {
		return m_connect.GetConnStat();
	}

	std::string GetDisconnectErrText(int reason) const;

	//  WPacket  
	Corsairs::Net::WPacket GetWPacket() {
		return Corsairs::Net::WPacket(256);
	}

	//      (  game loop)
	void PollPackets(int maxPackets = 1) {
		_client.PollPackets(maxPackets);
	}

	std::uint32_t GetAveragePing();

	CProCirculate* GetProCir() {
		return m_pCProCir;
	}

	void SwitchNet(bool isConnected);

	//   TcpClient ( Connection)
	Corsairs::Net::TcpClient& GetClient() {
		return _client;
	}

	Connection m_connect;

	struct {
		std::uint32_t m_pingid;
		std::uint32_t m_maxdelay, m_curdelay, m_mindelay;
		DWORD dwLatencyTime[20];

		std::uint32_t m_ulCurStatistic;
		std::uint32_t m_ulDelayTime[4];
	};

	std::uint32_t m_ulPacketCount;
	long m_framedelay;

	CProCirculate* m_pCProCir;
	std::recursive_mutex m_mutmov;
	char m_accounts[100];
	char m_passwd[100];

	// RSA-AES Network encryption (BCrypt)
	BCRYPT_KEY_HANDLE hRsaPubKey;
	BYTE cliAesKey[32];
	BCRYPT_ALG_HANDLE hAesAlg;
	BCRYPT_KEY_HANDLE hAesKey;
	bool handshakeDone;

	bool InitAesKey();
	void CleanupCrypto();

	bool _enc;
	int _comm_enc;

private:
	bool EncryptAES(char* ciphertext, std::uint32_t ciphertext_len,
					const char* plaintext, std::uint32_t& ciphersize);
	bool DecryptAES(char* ciphertext, std::uint32_t& len);

	Corsairs::Net::TcpClient _client;
};

extern NetIF* g_NetIF;

#endif
