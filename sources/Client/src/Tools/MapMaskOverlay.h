#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Corsairs::Client::Tools {

// Клиентский overlay миникарты — принимает сериализованный fog-of-war блоб от сервера
// (CMD_MC_MAP_MASK) и отвечает на «открыта ли клетка (x, y)» при отрисовке миникарты.
// Серверный аналог хранения — PlayerMapMask в Libraries/Common.
class MapMaskOverlay {
public:
    MapMaskOverlay() = default;
    MapMaskOverlay(const MapMaskOverlay&) = delete;
    MapMaskOverlay& operator=(const MapMaskOverlay&) = delete;

    // Принимает legacy wire-формат: [sMapMask 48-byte header][bits].
    // Из header'а вычитывает lenx/leny/llen, копирует payload целиком (вместе с header'ом).
    bool Init(std::span<const std::uint8_t> wireBlob);

    // Сервер сообщает, что fog-of-war на стороне сервера отключён. В этом режиме
    // GetMask всегда возвращает true (всё открыто), независимо от содержимого _data.
    void SetFogOfWarEnabled(bool enabled) { _fogOfWarEnabled = enabled; }
    bool IsFogOfWarEnabled() const { return _fogOfWarEnabled; }

    bool GetMask(std::int32_t cellX, std::int32_t cellY) const;

    std::int32_t NumX() const { return _numX; }
    std::int32_t NumY() const { return _numY; }

private:
    static constexpr std::size_t LegacyHeaderSize = 48;

    std::vector<std::uint8_t> _data; // payload c header'ом
    std::int32_t _numX = 0;
    std::int32_t _numY = 0;
    bool _fogOfWarEnabled = true;
};

// Глобальный экземпляр overlay'я — создаётся/удаляется по событиям WorldScene.
// Заменил статический CMaskData::g_MaskData.
extern MapMaskOverlay* g_pMapMaskOverlay;

} // namespace Corsairs::Client::Tools
