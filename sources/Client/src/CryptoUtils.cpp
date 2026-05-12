#include "stdafx.h"
#include "CryptoUtils.h"

#include "Blake2s.h"

#include <windows.h>
#include <wincrypt.h>
#include <vector>

#pragma comment(lib, "Crypt32.lib")

std::string HashPassword(const std::string& password) {
	return Corsairs::Common::Crypto::Blake2sHex(password);
}

namespace {

std::string BytesToHex(const BYTE* data, DWORD len) {
	std::string out;
	out.reserve(static_cast<std::size_t>(len) * 2);
	constexpr char kHex[] = "0123456789abcdef";
	for (DWORD i = 0; i < len; ++i) {
		out.push_back(kHex[data[i] >> 4]);
		out.push_back(kHex[data[i] & 0xF]);
	}
	return out;
}

[[nodiscard]] int HexValue(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

std::vector<BYTE> HexToBytes(std::string_view hex) {
	if (hex.size() % 2 != 0) return {};
	std::vector<BYTE> out;
	out.reserve(hex.size() / 2);
	for (std::size_t i = 0; i < hex.size(); i += 2) {
		const int hi = HexValue(hex[i]);
		const int lo = HexValue(hex[i + 1]);
		if (hi < 0 || lo < 0) return {};
		out.push_back(static_cast<BYTE>((hi << 4) | lo));
	}
	return out;
}

} // namespace

std::string ProtectStringDpapi(std::string_view plain) {
	if (plain.empty()) return {};

	DATA_BLOB in{};
	in.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(plain.data()));
	in.cbData = static_cast<DWORD>(plain.size());

	DATA_BLOB out{};
	if (!CryptProtectData(&in, L"TalesOfPirate Login", nullptr, nullptr, nullptr, 0, &out)) {
		return {};
	}

	std::string hex = BytesToHex(out.pbData, out.cbData);
	LocalFree(out.pbData);
	return hex;
}

std::string UnprotectStringDpapi(std::string_view hex) {
	if (hex.empty()) return {};

	auto bytes = HexToBytes(hex);
	if (bytes.empty()) return {};

	DATA_BLOB in{};
	in.pbData = bytes.data();
	in.cbData = static_cast<DWORD>(bytes.size());

	DATA_BLOB out{};
	if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) {
		return {};
	}

	std::string plain(reinterpret_cast<const char*>(out.pbData), out.cbData);
	LocalFree(out.pbData);
	return plain;
}
