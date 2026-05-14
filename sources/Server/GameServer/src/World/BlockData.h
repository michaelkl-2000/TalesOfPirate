#pragma once

// CBlockData — карта проходимости тайлов карты (тонкая решётка байт по флагу
// «блокирует прохождение / нет»). Исходные данные на диске лежат упакованными
// в биты по 1bpp; в памяти разворачиваются в 1 байт на тайл для скорости
// доступа в hot-path move-validation.
//
// Файл: 8 байт заголовка (width:int32, height:int32) + (width/8) × height
// байт битовой решётки. Биты идут big-endian внутри байта (старший = x%8 == 0).

#include <cstdint>
#include <cstdio>
#include <vector>

class CBlockData {
public:
    CBlockData() = default;
    ~CBlockData() = default;

    CBlockData(const CBlockData&) = delete;
    CBlockData& operator=(const CBlockData&) = delete;
    CBlockData(CBlockData&&) = default;
    CBlockData& operator=(CBlockData&&) = default;

    bool Load(const char* pszFile) {
        std::FILE* fp = std::fopen(pszFile, "rb");
        if (fp == nullptr) {
            return false;
        }

        std::int32_t width = 0;
        std::int32_t height = 0;
        if (std::fread(&width, sizeof(width), 1, fp) != 1 ||
            std::fread(&height, sizeof(height), 1, fp) != 1) {
            std::fclose(fp);
            return false;
        }

        if (width <= 0 || height <= 0 || (width % 8) != 0) {
            std::fclose(fp);
            return false;
        }

        const std::size_t byteWidth = static_cast<std::size_t>(width) / 8;
        const std::size_t packedSize = byteWidth * static_cast<std::size_t>(height);
        std::vector<std::uint8_t> packed(packedSize);
        const std::size_t readBytes = std::fread(packed.data(), 1, packedSize, fp);
        std::fclose(fp);
        if (readBytes != packedSize) {
            return false;
        }

        _width = width;
        _height = height;
        _byteWidth = static_cast<std::int32_t>(byteWidth);
        _data.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0);

        for (std::int32_t y = 0; y < height; ++y) {
            for (std::int32_t x = 0; x < width; ++x) {
                const std::size_t bitOffset = static_cast<std::size_t>(y) * byteWidth +
                                              static_cast<std::size_t>(x / 8);
                const std::uint8_t mask = static_cast<std::uint8_t>(1u << (7 - (x % 8)));
                const std::size_t locByte = static_cast<std::size_t>(y) * width + x;
                _data[locByte] = (packed[bitOffset] & mask) ? std::uint8_t{1} : std::uint8_t{0};
            }
        }

        _valid = true;
        return true;
    }

    std::uint8_t IsBlock(std::int32_t x, std::int32_t y) const {
        if (!_valid) {
            return 0;
        }
        return _data[static_cast<std::size_t>(y) * _width + x];
    }

    bool IsValid() const {
        return _valid;
    }

    std::int32_t GetWidth() const {
        return _width;
    }

    std::int32_t GetHeight() const {
        return _height;
    }

private:
    std::vector<std::uint8_t> _data;
    std::int32_t _width{0};
    std::int32_t _height{0};
    std::int32_t _byteWidth{0};
    bool _valid{false};
};
