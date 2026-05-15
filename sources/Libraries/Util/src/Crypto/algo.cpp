#include "algo.h"
#include "md5.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Util {

namespace {

constexpr std::string_view kBase64Alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

constexpr std::array<std::int8_t, 128> kBase64Index = [] {
    std::array<std::int8_t, 128> idx{};
    for (auto& v : idx) {
        v = -1;
    }
    for (std::int8_t i = 0; i < 64; ++i) {
        idx[static_cast<std::uint8_t>(kBase64Alphabet[i])] = i;
    }
    return idx;
}();

constexpr std::int8_t DecodeChar(char c) noexcept {
    const auto uc = static_cast<std::uint8_t>(c);
    return uc < 128 ? kBase64Index[uc] : -1;
}

} // namespace

std::string Base64Encode(std::span<const std::uint8_t> data) {
    const std::size_t srcLen = data.size();
    std::string out;
    out.resize(((srcLen + 2) / 3) * 4);

    const std::uint8_t* in = data.data();
    char* dst = out.data();
    std::size_t remaining = srcLen;

    while (remaining >= 3) {
        *dst++ = kBase64Alphabet[in[0] >> 2];
        *dst++ = kBase64Alphabet[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *dst++ = kBase64Alphabet[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *dst++ = kBase64Alphabet[in[2] & 0x3f];
        in += 3;
        remaining -= 3;
    }

    if (remaining > 0) {
        *dst++ = kBase64Alphabet[in[0] >> 2];
        std::uint8_t oval = (in[0] << 4) & 0x30;
        if (remaining > 1) {
            oval |= in[1] >> 4;
        }
        *dst++ = kBase64Alphabet[oval];
        *dst++ = (remaining < 2) ? '=' : kBase64Alphabet[(in[1] << 2) & 0x3c];
        *dst++ = '=';
    }

    return out;
}

std::string Base64Encode(std::string_view text) {
    return Base64Encode(std::span(reinterpret_cast<const std::uint8_t*>(text.data()), text.size()));
}

std::vector<std::uint8_t> Base64Decode(std::string_view src) {
    if (src.size() >= 2 && src[0] == '+' && src[1] == ' ') {
        src.remove_prefix(2);
    }
    if (src.empty() || src.front() == '\r') {
        return {};
    }

    std::vector<std::uint8_t> out;
    out.reserve((src.size() / 4) * 3);

    const char* p = src.data();
    const std::size_t groups = src.size() / 4;
    for (std::size_t g = 0; g < groups; ++g, p += 4) {
        const std::int8_t c1 = DecodeChar(p[0]);
        const std::int8_t c2 = DecodeChar(p[1]);
        if (c1 < 0 || c2 < 0) {
            return {};
        }

        const bool c3IsPad = p[2] == '=';
        const bool c4IsPad = p[3] == '=';
        const std::int8_t c3 = c3IsPad ? 0 : DecodeChar(p[2]);
        const std::int8_t c4 = c4IsPad ? 0 : DecodeChar(p[3]);
        if ((!c3IsPad && c3 < 0) || (!c4IsPad && c4 < 0)) {
            return {};
        }

        out.push_back(static_cast<std::uint8_t>((c1 << 2) | (c2 >> 4)));
        if (!c3IsPad) {
            out.push_back(static_cast<std::uint8_t>(((c2 << 4) & 0xf0) | (c3 >> 2)));
            if (!c4IsPad) {
                out.push_back(static_cast<std::uint8_t>(((c3 << 6) & 0xc0) | c4));
            }
        }
    }

    return out;
}

std::array<std::uint8_t, 16> Md5(std::string_view msg) {
    MD5_CTX context;
    MD5Init(&context);
    MD5Update(&context,
              reinterpret_cast<unsigned char*>(const_cast<char*>(msg.data())),
              static_cast<unsigned int>(msg.size()));

    std::array<std::uint8_t, 16> digest{};
    MD5Final(digest.data(), &context);
    return digest;
}

std::string Md5Hex(std::string_view msg) {
    const auto digest = Md5(msg);
    std::string out;
    out.reserve(32);
    for (const auto byte : digest) {
        std::format_to(std::back_inserter(out), "{:02X}", byte);
    }
    return out;
}

} // namespace Corsairs::Util
