// Реализация MapLoader (.map). Снапшот-семантика: Load читает заголовок,
// таблицу оффсетов и body «как есть» в MapInfo, Save пишет это же побайтно.
//
// Round-trip Load→Save детерминирован: тело файла копируется без переинтерпретации
// SNewFileTile/SFileTile-структур, а заголовок и offsets[] возвращаются ровно в
// той же раскладке, что и пришли. Что сломано — будет поймано в ParseHeader/
// ValidateOffsets, а не «зашьётся» в копии.
//
// Парсинг заголовка решает три вопроса: распознать чужой/повреждённый формат
// (BadMagic), отбросить геометрически бессмысленные размеры
// (InconsistentDimensions) и проверить, что каждая ссылка из offsets[]
// помещается в body (BodyTruncated).
//
// Интеграция с MPMap::Load — отдельная задача: текущая реализация в MPMapData.cpp
// держит FILE* открытым ради edit-режима с lazy-загрузкой секций; сюда мигрирует
// только runtime-путь (bEdit=FALSE). Эта реализация уже готова к такому использованию,
// но MPMap пока не переведён, чтобы не ломать редактор отдельным изменением.

#include "AssetLoaders.h"

#include "logutil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Engine::Render {

namespace {

struct FileCloser {
    void operator()(std::FILE* fp) const noexcept {
        if (fp != nullptr) {
            std::fclose(fp);
        }
    }
};
using UniqueFile = std::unique_ptr<std::FILE, FileCloser>;

// Поддерживаемые значения header.nMapFlag. Файл с MP_MAP_FLAG+1 — это незавершённый
// MapTool-файл (см. MPMap::Load в MPMapData.cpp), он явно отвергается.
[[nodiscard]] constexpr bool IsKnownMapFlag(std::int32_t flag) noexcept {
    return flag == MapLoader::kCurrentMapFlag || flag == MapLoader::kLegacyMapFlag;
}

// Проверка геометрии: размеры должны быть положительные, секции —
// строго больше нуля. Делимости nWidth/nSectionWidth НЕ требуем: рантайм
// (`MPMap::Load`) считает count через целочисленное деление и игнорирует
// «хвост». В датасете есть валидный пример room.map (52×52 при секции 8×8 →
// 6×6=36 секций, 4 столбца/строки тайлов на правом и нижнем краях не
// загружаются). Если бы мы reject'или такие файлы, тест бы их пометил
// невалидными — но MPMap их грузит.
[[nodiscard]] bool DimensionsValid(const MPMapFileHeader& h) noexcept {
    if (h.nWidth <= 0 || h.nHeight <= 0) {
        return false;
    }
    if (h.nSectionWidth <= 0 || h.nSectionHeight <= 0) {
        return false;
    }
    if (h.nSectionWidth > h.nWidth || h.nSectionHeight > h.nHeight) {
        return false;
    }
    return true;
}

[[nodiscard]] std::size_t SectionCount(const MPMapFileHeader& h) noexcept {
    const std::size_t cntX = static_cast<std::size_t>(h.nWidth / h.nSectionWidth);
    const std::size_t cntY = static_cast<std::size_t>(h.nHeight / h.nSectionHeight);
    return cntX * cntY;
}

// Размер одной секции в байтах. Зависит от того, какая раскладка SFileTile
// активна на данный момент. NEW_VERSION ON в MPMapDef.h → SNewFileTile = 15 байт.
// Сохраняем формулу как функцию, чтобы при включении legacy-сборки (#undef
// NEW_VERSION) она автоматически подхватила SFileTile.
[[nodiscard]] std::size_t TileBytesPerSection(const MPMapFileHeader& h) noexcept {
#ifdef NEW_VERSION
    constexpr std::size_t kTileSize = sizeof(SNewFileTile);
#else
    constexpr std::size_t kTileSize = sizeof(SFileTile);
#endif
    return static_cast<std::size_t>(h.nSectionWidth) *
           static_cast<std::size_t>(h.nSectionHeight) *
           kTileSize;
}

[[nodiscard]] std::int64_t FileSize(std::FILE* fp) noexcept {
    if (std::fseek(fp, 0, SEEK_END) != 0) {
        return -1;
    }
    const long sz = std::ftell(fp);
    if (std::fseek(fp, 0, SEEK_SET) != 0) {
        return -1;
    }
    return static_cast<std::int64_t>(sz);
}

void LogLoadFailure(std::string_view file, const MapLoadDiagnostics& diag) {
    ToLogService("errors", LogLevel::Error,
                 "[MapLoader] {} ({}): file={}, mapFlag={}",
                 ToString(diag.status), diag.detail,
                 (file.empty() ? std::string_view{"(null)"} : file),
                 diag.mapFlag);
}

} // namespace

// =============================================================================
// MapLoader (.map)
// =============================================================================

LW_RESULT MapLoader::Load(MapInfo& info, std::string_view file) {
    MapLoadDiagnostics diag;
    return LoadEx(info, file, diag);
}

LW_RESULT MapLoader::LoadEx(MapInfo& info, std::string_view file,
                            MapLoadDiagnostics& diag) {
    diag = {};
    info = MapInfo{};

    UniqueFile fp{std::fopen(std::string{file}.c_str(), "rb")};
    if (!fp) {
        diag.status = MapLoadStatus::FileOpenFailed;
        diag.detail = "fopen failed";
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    const std::int64_t totalSize = FileSize(fp.get());
    if (totalSize < 0) {
        diag.status = MapLoadStatus::FileOpenFailed;
        diag.detail = "fseek/ftell failed";
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    if (totalSize < static_cast<std::int64_t>(sizeof(MPMapFileHeader))) {
        diag.status = MapLoadStatus::HeaderTruncated;
        diag.detail = std::format("file size {} < sizeof(MPMapFileHeader)={}",
                                   totalSize, sizeof(MPMapFileHeader));
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    if (std::fread(&info.header, sizeof(MPMapFileHeader), 1, fp.get()) != 1) {
        diag.status = MapLoadStatus::HeaderTruncated;
        diag.detail = "short fread of MPMapFileHeader";
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }
    diag.mapFlag = info.header.nMapFlag;

    if (!IsKnownMapFlag(info.header.nMapFlag)) {
        diag.status = MapLoadStatus::BadMagic;
        diag.detail = std::format("nMapFlag={} (expected {} или {})",
                                   info.header.nMapFlag,
                                   kCurrentMapFlag, kLegacyMapFlag);
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    if (!DimensionsValid(info.header)) {
        diag.status = MapLoadStatus::InconsistentDimensions;
        diag.detail = std::format(
            "width={}, height={}, sectionWidth={}, sectionHeight={}",
            info.header.nWidth, info.header.nHeight,
            info.header.nSectionWidth, info.header.nSectionHeight);
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    const std::size_t sectionCount = SectionCount(info.header);
    const std::size_t offsetTableBytes = sectionCount * sizeof(std::uint32_t);
    const std::size_t prefixBytes = sizeof(MPMapFileHeader) + offsetTableBytes;
    if (static_cast<std::int64_t>(prefixBytes) > totalSize) {
        diag.status = MapLoadStatus::OffsetTableTruncated;
        diag.detail = std::format(
            "header+offsets={} > file_size={}", prefixBytes, totalSize);
        LogLoadFailure(file, diag);
        return LW_RET_FAILED;
    }

    info.offsets.resize(sectionCount);
    if (sectionCount > 0) {
        if (std::fread(info.offsets.data(), sizeof(std::uint32_t),
                       sectionCount, fp.get()) != sectionCount) {
            diag.status = MapLoadStatus::OffsetTableTruncated;
            diag.detail = std::format("short fread of offsets[{}]", sectionCount);
            LogLoadFailure(file, diag);
            return LW_RET_FAILED;
        }
    }

    const std::size_t bodyBytes = static_cast<std::size_t>(totalSize) - prefixBytes;
    info.body.resize(bodyBytes);
    if (bodyBytes > 0) {
        if (std::fread(info.body.data(), 1, bodyBytes, fp.get()) != bodyBytes) {
            diag.status = MapLoadStatus::BodyTruncated;
            diag.detail = std::format("short fread of body, expected {} bytes",
                                       bodyBytes);
            LogLoadFailure(file, diag);
            return LW_RET_FAILED;
        }
    }

    // Sanity: каждый ненулевой offsets[i] должен попадать в [prefix, file_size − tileBytes].
    // Битый offset — признак повреждения файла; runtime будет читать мусор.
    const std::size_t tileBytes = TileBytesPerSection(info.header);
    for (std::size_t i = 0; i < sectionCount; ++i) {
        const std::uint32_t off = info.offsets[i];
        if (off == 0) {
            continue;
        }
        if (off < prefixBytes || off + tileBytes > static_cast<std::uint64_t>(totalSize)) {
            diag.status = MapLoadStatus::BodyTruncated;
            diag.detail = std::format(
                "offsets[{}]={} вне допустимого диапазона [{}, {}]",
                i, off, prefixBytes,
                static_cast<std::uint64_t>(totalSize) - tileBytes);
            LogLoadFailure(file, diag);
            return LW_RET_FAILED;
        }
    }

    diag.status = MapLoadStatus::Ok;
    return LW_RET_OK;
}

LW_RESULT MapLoader::Save(const MapInfo& info, std::string_view file) {
    UniqueFile fp{std::fopen(std::string{file}.c_str(), "wb")};
    if (!fp) {
        ToLogService("errors", LogLevel::Error,
                     "[MapLoader::Save] fopen failed: {}", file);
        return LW_RET_FAILED;
    }

    if (std::fwrite(&info.header, sizeof(MPMapFileHeader), 1, fp.get()) != 1) {
        ToLogService("errors", LogLevel::Error,
                     "[MapLoader::Save] fwrite header failed: {}", file);
        return LW_RET_FAILED;
    }

    if (!info.offsets.empty()) {
        if (std::fwrite(info.offsets.data(), sizeof(std::uint32_t),
                        info.offsets.size(), fp.get()) != info.offsets.size()) {
            ToLogService("errors", LogLevel::Error,
                         "[MapLoader::Save] fwrite offsets failed: {}", file);
            return LW_RET_FAILED;
        }
    }

    if (!info.body.empty()) {
        if (std::fwrite(info.body.data(), 1, info.body.size(), fp.get())
            != info.body.size()) {
            ToLogService("errors", LogLevel::Error,
                         "[MapLoader::Save] fwrite body failed: {}", file);
            return LW_RET_FAILED;
        }
    }

    return LW_RET_OK;
}

} // namespace Corsairs::Engine::Render
