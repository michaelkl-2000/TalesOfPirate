#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Common::Character {

class PlayerMapMask {
public:
    static constexpr std::int32_t CellSize = 80;

    struct MapBitmap {
        std::string Name;
        std::int32_t Width = 0;
        std::int32_t Height = 0;
        std::int32_t CellsX = 0;
        std::int32_t CellsY = 0;
        std::vector<std::uint8_t> Bits;
    };

    void AddMap(std::string_view name, std::int32_t width, std::int32_t height);

    bool LoadBase64(std::string_view mapName, std::string_view base64Data);
    std::string SaveBase64(std::string_view mapName) const;

    std::span<const std::uint8_t> GetBits(std::string_view mapName) const;

    // Сериализация в legacy wire-формат [sMapMask 48-byte header][bits] для отправки клиенту.
    // Клиент (sources/Client/src/Tools/SMallMap.h::CMaskData::InitMaskData) парсит header
    // и оттуда читает lenx/leny/llen; bits идут со смещения 48.
    std::vector<std::uint8_t> SerializeLegacyWire(std::string_view mapName) const;

    float GetOpenScalePercent(std::string_view mapName) const;

    // Открывает клетку, в которой находится игрок (worldX, worldY в мировых единицах),
    // плюс квадрат вокруг радиусом radiusCells клеток. radiusCells = 0 — только текущая клетка.
    // Возвращает true, если хотя бы одна клетка перешла из закрытого в открытое состояние.
    bool Update(std::string_view mapName, std::int32_t worldX, std::int32_t worldY,
                std::int32_t radiusCells = 0);

private:
    static constexpr std::size_t LegacyHeaderSize = 48;

    const MapBitmap* Find(std::string_view name) const;
    MapBitmap* Find(std::string_view name);

    std::vector<MapBitmap> _maps;
};

} // namespace Corsairs::Common::Character
