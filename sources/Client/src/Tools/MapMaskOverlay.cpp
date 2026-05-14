#include "Tools/MapMaskOverlay.h"

#include <cstring>

namespace Corsairs::Client::Tools {

MapMaskOverlay* g_pMapMaskOverlay = nullptr;

namespace {
constexpr std::size_t LegacyNameLen = 32; // sizeof(sMapMask::pszName)
}

bool MapMaskOverlay::Init(std::span<const std::uint8_t> wireBlob) {
    if (wireBlob.size() < LegacyHeaderSize) {
        return false;
    }

    std::int32_t lenx = 0;
    std::int32_t leny = 0;
    std::int32_t llen = 0;
    std::memcpy(&lenx, wireBlob.data() + LegacyNameLen,        sizeof(lenx));
    std::memcpy(&leny, wireBlob.data() + LegacyNameLen + 4,    sizeof(leny));
    std::memcpy(&llen, wireBlob.data() + LegacyNameLen + 12,   sizeof(llen));

    if (llen < static_cast<std::int32_t>(LegacyHeaderSize) ||
        static_cast<std::size_t>(llen) > wireBlob.size()) {
        return false;
    }

    _numX = lenx;
    _numY = leny;
    _data.assign(wireBlob.begin(), wireBlob.begin() + llen);
    return true;
}

bool MapMaskOverlay::GetMask(std::int32_t cellX, std::int32_t cellY) const {
    if (!_fogOfWarEnabled) {
        return true; // На сервере fog-of-war выключен — рисуем всё открытым.
    }

    if (cellX < 0 || cellY < 0 || cellX >= _numX || cellY >= _numY) {
        return false;
    }
    if (_data.size() <= LegacyHeaderSize) {
        return false;
    }

    const auto* bits = _data.data() + LegacyHeaderSize;
    const std::size_t pos = static_cast<std::size_t>(cellY) * _numX + cellX;
    const auto byteIdx = pos / 8;
    const auto bitIdx = pos % 8;
    return (bits[byteIdx] & (std::uint8_t{1} << bitIdx)) != 0;
}

} // namespace Corsairs::Client::Tools
