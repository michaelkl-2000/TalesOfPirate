#include "MPMapDef.h"


namespace Corsairs::Util::Map {

void TileInfo_Pack(const std::uint8_t* pbtTile,
                   std::uint32_t& tileInfo,
                   std::uint8_t& baseTex) noexcept {
    const std::uint32_t tex1   = pbtTile[2];
    const std::uint32_t alpha1 = pbtTile[3];
    const std::uint32_t tex2   = pbtTile[4];
    const std::uint32_t alpha2 = pbtTile[5];
    const std::uint32_t tex3   = pbtTile[6];
    const std::uint32_t alpha3 = pbtTile[7];

    tileInfo = (tex1   << kTileTex1Shift)
             | (alpha1 << kTileAlpha1Shift)
             | (tex2   << kTileTex2Shift)
             | (alpha2 << kTileAlpha2Shift)
             | (tex3   << kTileTex3Shift)
             | (alpha3 << kTileAlpha3Shift);
    baseTex = pbtTile[0];
}

void TileInfo_Unpack(std::uint32_t tileInfo,
                     std::uint8_t baseTex,
                     std::uint8_t* pbtTile) noexcept {
    pbtTile[0] = baseTex;
    pbtTile[1] = kBaseAlphaOpaque;
    pbtTile[2] = static_cast<std::uint8_t>(tileInfo >> kTileTex1Shift);
    pbtTile[3] = static_cast<std::uint8_t>((tileInfo >> kTileAlpha1Shift) & kTileAlphaMask);
    pbtTile[4] = static_cast<std::uint8_t>((tileInfo >> kTileTex2Shift)   & kTileTexMask);
    pbtTile[5] = static_cast<std::uint8_t>((tileInfo >> kTileAlpha2Shift) & kTileAlphaMask);
    pbtTile[6] = static_cast<std::uint8_t>((tileInfo >> kTileTex3Shift)   & kTileTexMask);
    pbtTile[7] = static_cast<std::uint8_t>((tileInfo >> kTileAlpha3Shift) & kTileAlphaMask);
}

} // namespace Corsairs::Util::Map
