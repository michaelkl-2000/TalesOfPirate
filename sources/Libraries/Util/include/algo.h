#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Util {

std::string Base64Encode(std::span<const std::uint8_t> data);
std::string Base64Encode(std::string_view text);

std::vector<std::uint8_t> Base64Decode(std::string_view src);

std::array<std::uint8_t, 16> Md5(std::string_view msg);
std::string Md5Hex(std::string_view msg);

} // namespace Corsairs::Util
