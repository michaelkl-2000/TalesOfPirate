#include "Validator.h"

#include "AssetLoaders.h"
#include "SceneFileLoaders.h"
#include "lwEfxTrack.h"      // EfxTrackLoader::Load — для .let
#include "lwExpObj.h"
#include "MPModelEff.h"      // EffectFileInfo, CEffPath

#include "MPParticleCtrl.h"  // CMPPartCtrl

#include "stb_image.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

namespace pkotool {

namespace fs = std::filesystem;

namespace {

[[nodiscard]] std::string ExtLower(const fs::path& p) {
    std::string ext = p.extension().string();
    if (!ext.empty() && ext.front() == '.') {
        ext.erase(ext.begin());
    }
    for (auto& ch : ext) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return ext;
}

// Маппинг диагностики LgoLoader → ValidationRecord.
// Возвращает пару (status, recommendation): problem/version подставляются caller'ом.
[[nodiscard]] std::pair<ValidationStatus, std::string>
ClassifyLgoStatus(Corsairs::Engine::Render::LgoLoadStatus s) {
    using S = Corsairs::Engine::Render::LgoLoadStatus;
    switch (s) {
    case S::Ok:
        return {ValidationStatus::Ok, ""};
    case S::OkWithTrailingData:
        return {ValidationStatus::Warning,
                "File parses, but a trailing block exists and will be dropped on re-save. "
                "Run `pkotool --mode=fix` to strip it."};
    case S::FileOpenFailed:
        return {ValidationStatus::Error,
                "Failed to open file (missing file or permissions)."};
    case S::VersionTruncated:
        return {ValidationStatus::Error,
                "File too short: even the DWORD version is missing. Re-export from source."};
    case S::VersionUnknown:
        return {ValidationStatus::Error,
                "Unknown format version. File was created by another tool; re-export needed."};
    case S::HeaderTruncated:
        return {ValidationStatus::Error,
                "Header truncated: file corrupted. Re-export from source."};
    case S::BlockSizesInconsistent:
        return {ValidationStatus::Error,
                "Header block sizes do not match actual file size. Re-export from source."};
    case S::ParseFailed:
        return {ValidationStatus::Error,
                "Parser returned an error (corrupt data inside a block). "
                "Re-export from source."};
    case S::UnconsumedTrailingData:
        return {ValidationStatus::Warning,
                "Unconsumed trailing data in the file."};
    }
    return {ValidationStatus::Error, "Unknown LgoLoader status."};
}

ValidationRecord ValidateLgo(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "lgo";

    Corsairs::Engine::Render::LgoLoadDiagnostics diag;
    auto* info = Corsairs::Engine::Render::LgoLoader::LoadEx(file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyLgoStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);

    delete info;
    return rec;
}

ValidationRecord ValidateLmo(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "lmo";

    MindPower::lwModelObjInfo info;
    Corsairs::Engine::Render::LgoLoadDiagnostics diag;
    Corsairs::Engine::Render::LgoLoader::LoadModelObjEx(info, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyLgoStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);

    return rec;
}

ValidationRecord ValidateLxo(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "lxo";

    MindPower::lwModelInfo info;
    Corsairs::Engine::Render::LgoLoadDiagnostics diag;
    Corsairs::Engine::Render::LgoLoader::LoadModelEx(info, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyLgoStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);

    return rec;
}

ValidationRecord ValidateLab(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "lab";

    MindPower::lwAnimDataBone info;
    Corsairs::Engine::Render::LgoLoadDiagnostics diag;
    Corsairs::Engine::Render::LgoLoader::LoadAnimDataBoneEx(info, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyLgoStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);

    return rec;
}

[[nodiscard]] std::pair<ValidationStatus, std::string>
ClassifyEffectStatus(Corsairs::Engine::Render::EffectLoadStatus s) {
    using S = Corsairs::Engine::Render::EffectLoadStatus;
    switch (s) {
    case S::Ok:
        return {ValidationStatus::Ok, ""};
    case S::FileOpenFailed:
        return {ValidationStatus::Error,
                "Failed to open file (missing file or permissions)."};
    case S::VersionTruncated:
        return {ValidationStatus::Error,
                "File too short: even the DWORD version is missing. Re-export from source."};
    case S::VersionUnknown:
        return {ValidationStatus::Error,
                "Unknown effect format version. Re-export from source."};
    case S::HeaderTruncated:
        return {ValidationStatus::Error,
                "Effect header truncated. Re-export from source."};
    case S::ParseFailed:
        return {ValidationStatus::Error,
                "Effect parser failed (corrupt element). Re-export from source."};
    }
    return {ValidationStatus::Error, "Unknown EffectLoader status."};
}

ValidationRecord ValidateEff(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "eff";

    EffectFileInfo info;
    Corsairs::Engine::Render::EffectLoadDiagnostics diag;
    Corsairs::Engine::Render::EffectLoader::LoadEx(info, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyEffectStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);
    return rec;
}

[[nodiscard]] std::pair<ValidationStatus, std::string>
ClassifyPartCtrlStatus(Corsairs::Engine::Render::PartCtrlLoadStatus s) {
    using S = Corsairs::Engine::Render::PartCtrlLoadStatus;
    switch (s) {
    case S::Ok:
        return {ValidationStatus::Ok, ""};
    case S::FileOpenFailed:
        return {ValidationStatus::Error,
                "Failed to open file (missing file or permissions)."};
    case S::VersionTruncated:
        return {ValidationStatus::Error,
                "File too short: even the DWORD version is missing. Re-export from source."};
    case S::VersionUnknown:
        return {ValidationStatus::Error,
                "Unknown particle-ctrl format version. Re-export from source."};
    case S::ParseFailed:
        return {ValidationStatus::Error,
                "PartCtrl parser failed (corrupt block). Re-export from source."};
    }
    return {ValidationStatus::Error, "Unknown PartCtrlLoader status."};
}

[[nodiscard]] std::pair<ValidationStatus, std::string>
ClassifyMapStatus(Corsairs::Engine::Render::MapLoadStatus s) {
    using S = Corsairs::Engine::Render::MapLoadStatus;
    switch (s) {
    case S::Ok:
        return {ValidationStatus::Ok, ""};
    case S::FileOpenFailed:
        return {ValidationStatus::Error,
                "Failed to open file (missing file or permissions)."};
    case S::HeaderTruncated:
        return {ValidationStatus::Error,
                "Map header truncated. Re-export from MapTool."};
    case S::BadMagic:
        return {ValidationStatus::Error,
                "Bad nMapFlag (not a MindPower .map). Foreign or corrupt file."};
    case S::InconsistentDimensions:
        return {ValidationStatus::Error,
                "Inconsistent dimensions: width/height non-positive or smaller than section. "
                "Re-export from MapTool."};
    case S::OffsetTableTruncated:
        return {ValidationStatus::Error,
                "Section offset table truncated. Re-export from MapTool."};
    case S::BodyTruncated:
        return {ValidationStatus::Error,
                "Section body truncated or out-of-range offset. Re-export from MapTool."};
    case S::UnknownVersion:
        return {ValidationStatus::Error,
                "Unknown map version (NEW_VERSION mismatch). Re-export from MapTool."};
    }
    return {ValidationStatus::Error, "Unknown MapLoader status."};
}

ValidationRecord ValidateMap(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "map";

    Corsairs::Engine::Render::MapInfo info;
    Corsairs::Engine::Render::MapLoadDiagnostics diag;
    Corsairs::Engine::Render::MapLoader::LoadEx(info, file.string(), diag);
    rec.version = static_cast<std::uint32_t>(diag.mapFlag);

    auto [status, recommendation] = ClassifyMapStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);
    return rec;
}

// .obj — header-only валидация: title-magic, version, lFileSize.
// Полный re-parse (sections+per-section objs) опускаем: формат с переменными
// per-section массивами, дешёвой проверки на «всё прочиталось» нет, а наша
// цель — отсеять заведомо битые / чужие файлы.
ValidationRecord ValidateObj(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "obj";

    using ObjFileLoader = Corsairs::Engine::Scene::ObjFileLoader;

    std::FILE* fp = std::fopen(file.string().c_str(), "rb");
    if (!fp) {
        rec.status = ValidationStatus::Error;
        rec.problem = "fopen failed";
        rec.recommendation = "Check path and access permissions.";
        return rec;
    }
    SFileHead head{};
    long fileSize = 0;
    const bool readOk = ObjFileLoader::ReadHeader(fp, head, fileSize);
    std::fclose(fp);

    if (!readOk) {
        rec.status = ValidationStatus::Error;
        rec.problem = "header truncated";
        rec.recommendation = "File too small to contain SFileHead. Re-export.";
        return rec;
    }
    rec.version = static_cast<std::uint32_t>(head.lVersion);

    if (std::strncmp(head.tcsTitle, ObjFileLoader::kMagicTitle,
                     std::strlen(ObjFileLoader::kMagicTitle)) != 0) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("bad magic title: '{:.16}'", head.tcsTitle);
        rec.recommendation = "Not a MindPower .obj. Re-export from editor.";
        return rec;
    }
    if (head.lVersion != ObjFileLoader::kCurrentVersion
        && head.lVersion != ObjFileLoader::kLegacyVersion500) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("unsupported version: {}", head.lVersion);
        rec.recommendation = "Only versions 500 (legacy) and 600 are supported.";
        return rec;
    }
    if (head.iSectionCntX <= 0 || head.iSectionCntY <= 0
        || head.iSectionWidth <= 0 || head.iSectionHeight <= 0
        || head.iSectionObjNum <= 0) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format(
            "invalid header dimensions: cnt={}x{}, sec={}x{}, objNum={}",
            head.iSectionCntX, head.iSectionCntY,
            head.iSectionWidth, head.iSectionHeight, head.iSectionObjNum);
        rec.recommendation = "Header fields must be positive.";
        return rec;
    }
    if (head.lFileSize != fileSize) {
        // lFileSize даёт editor при WriteSectionObjInfo; mismatch — признак
        // обрезанного / частично перезаписанного файла.
        rec.status = ValidationStatus::Error;
        rec.problem = std::format(
            "header.lFileSize={} mismatches actual={}", head.lFileSize, fileSize);
        rec.recommendation = "File partially written or truncated. Re-export.";
        return rec;
    }
    if (head.lVersion == ObjFileLoader::kLegacyVersion500) {
        rec.status = ValidationStatus::Warning;
        rec.problem = "legacy version 500 (will be upgraded to 600 on first edit)";
        rec.recommendation = "Run pkotool --mode=fix to convert in-place.";
        return rec;
    }

    rec.status = ValidationStatus::Ok;
    return rec;
}

// .rbo — текстовый sidecar; валидация = успешная попытка распарсить файл.
// Пустой файл считается Ok (не все карты имеют RBO).
ValidationRecord ValidateRbo(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "rbo";
    rec.version = 0;

    std::set<ReallyBigObjectInfo> items;
    if (!Corsairs::Engine::Scene::RboLoader::Load(file.string(), items)) {
        rec.status = ValidationStatus::Error;
        rec.problem = "RboLoader::Load failed (I/O error)";
        rec.recommendation = "Check that the file is a text RBO sidecar.";
        return rec;
    }

    if (items.empty()) {
        // Файл может физически отсутствовать (Load возвращает true с пустым
        // out) — это нормальный случай. Но если файл существует и пуст —
        // это легаси-баг (старый _Serialize_RBO создавал 0-байтные .rbo).
        std::error_code ec;
        if (fs::exists(file, ec) && fs::file_size(file, ec) == 0) {
            rec.status = ValidationStatus::Warning;
            rec.problem = "empty .rbo file (legacy bug)";
            rec.recommendation = "Safe to delete: pkotool --mode=fix removes it.";
            return rec;
        }
    }
    rec.status = ValidationStatus::Ok;
    return rec;
}

[[nodiscard]] std::pair<ValidationStatus, std::string>
ClassifyEffPathStatus(Corsairs::Engine::Render::EffPathLoadStatus s) {
    using S = Corsairs::Engine::Render::EffPathLoadStatus;
    switch (s) {
    case S::Ok:
        return {ValidationStatus::Ok, ""};
    case S::FileOpenFailed:
        return {ValidationStatus::Error,
                "Failed to open file (missing file or permissions)."};
    case S::HeaderTruncated:
        return {ValidationStatus::Error,
                "Header truncated (need 4-byte magic + DWORD version + DWORD num). Re-export."};
    case S::BadMagic:
        return {ValidationStatus::Error,
                "Bad magic (expected 'csf'). Foreign or corrupt file."};
    case S::FrameCountOutOfRange:
        return {ValidationStatus::Error,
                "Frame count is zero or exceeds CEffPath limit (200). Re-export."};
    case S::BodyTruncated:
        return {ValidationStatus::Error,
                "Path body truncated (frames cut off). Re-export."};
    }
    return {ValidationStatus::Error, "Unknown EffPathLoader status."};
}

ValidationRecord ValidateCsf(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "csf";

    CEffPath path;
    Corsairs::Engine::Render::EffPathLoadDiagnostics diag;
    Corsairs::Engine::Render::EffPathLoader::LoadEx(path, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyEffPathStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}",
                      Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);
    return rec;
}

// .let — matrix-track анимации, тонкая обёртка над LgoLoader::LoadAnimDataMatrix.
// EfxTrackLoader не имеет diag-варианта, поэтому ограничиваемся return-кодом:
// успех/неуспех. Деталь — в логах самого Loader (errors-канал) при провале.
ValidationRecord ValidateLet(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "let";

    MindPower::lwEfxTrack track;
    const LW_RESULT r = Corsairs::Engine::Render::EfxTrackLoader::Load(
        track, file.string());
    if (LW_FAILED(r)) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("EfxTrackLoader::Load failed (ret={})",
                                   static_cast<long long>(r));
        rec.recommendation =
            "Re-export from animation source. See errors.log for details.";
        return rec;
    }
    rec.status = ValidationStatus::Ok;
    return rec;
}

ValidationRecord ValidatePar(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "par";

    CMPPartCtrl ctrl;
    Corsairs::Engine::Render::PartCtrlLoadDiagnostics diag;
    Corsairs::Engine::Render::PartCtrlLoader::LoadEx(ctrl, file.string(), diag);
    rec.version = diag.version;

    auto [status, recommendation] = ClassifyPartCtrlStatus(diag.status);
    rec.status = status;
    rec.problem = (status == ValidationStatus::Ok)
        ? std::string{}
        : std::format("{}: {}", Corsairs::Engine::Render::ToString(diag.status), diag.detail);
    rec.recommendation = std::move(recommendation);
    return rec;
}

// .dds — DDS-текстура. Полная валидация требует D3D-девайса (для парсинга
// pixel format / mip-цепочки), но для отсева повреждённых/чужих файлов
// достаточно проверить magic 'DDS ' + header.size==124 + положительные
// width/height. Структура header известна как lwDDSHeader (124 байта).
ValidationRecord ValidateDds(const fs::path& file) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = "dds";
    rec.version = 0;

    std::ifstream is(file, std::ios::binary);
    if (!is) {
        rec.status = ValidationStatus::Error;
        rec.problem = "fopen failed";
        rec.recommendation = "Check path and access permissions.";
        return rec;
    }

    // 4 байта magic + минимум sizeof(DDS_HEADER)=124 байт.
    constexpr std::size_t kMinDdsBytes = 4 + 124;
    is.seekg(0, std::ios::end);
    const auto sz = is.tellg();
    if (sz < static_cast<std::streampos>(kMinDdsBytes)) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("file too small ({} bytes < {} required)",
                                   static_cast<std::int64_t>(sz), kMinDdsBytes);
        rec.recommendation = "DDS header truncated. Re-export texture.";
        return rec;
    }

    is.seekg(0, std::ios::beg);
    char magic[4]{};
    is.read(magic, sizeof(magic));
    if (!(magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ')) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("bad magic: '{}{}{}{}'",
                                   magic[0], magic[1], magic[2], magic[3]);
        rec.recommendation = "Not a DDS file. Foreign or corrupt.";
        return rec;
    }

    // lwDDSHeader: первое поле — DWORD size (= 124 для стандартного DDS).
    // Положение width/height: после size, header_flag (offsets +0, +4, +8, +12).
    std::uint32_t headerSize = 0;
    std::uint32_t headerFlag = 0;
    std::uint32_t height = 0;
    std::uint32_t width = 0;
    is.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
    is.read(reinterpret_cast<char*>(&headerFlag), sizeof(headerFlag));
    is.read(reinterpret_cast<char*>(&height), sizeof(height));
    is.read(reinterpret_cast<char*>(&width), sizeof(width));
    if (!is) {
        rec.status = ValidationStatus::Error;
        rec.problem = "short read of DDS header";
        rec.recommendation = "DDS header truncated. Re-export texture.";
        return rec;
    }

    if (headerSize != 124) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("unexpected DDS header size: {} (must be 124)",
                                   headerSize);
        rec.recommendation = "Non-standard DDS or corrupt file.";
        return rec;
    }
    if (width == 0 || height == 0) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("invalid dimensions: {}x{}", width, height);
        rec.recommendation = "DDS dimensions must be positive.";
        return rec;
    }

    rec.status = ValidationStatus::Ok;
    return rec;
}

// Считывает файл целиком в память. ifstream берёт fs::path напрямую (wide-путь под
// Windows), без конверсии через ANSI codepage — иначе путь с не-ANSI символами
// валит process через std::system_error из path::string().
[[nodiscard]] bool ReadFileToBuffer(const fs::path& file, std::vector<unsigned char>& out) {
    std::ifstream fs(file, std::ios::binary);
    if (!fs) {
        return false;
    }
    fs.seekg(0, std::ios::end);
    const auto sz = fs.tellg();
    if (sz < 0) {
        return false;
    }
    fs.seekg(0, std::ios::beg);
    out.resize(static_cast<std::size_t>(sz));
    fs.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()));
    return fs.good() || fs.eof();
}

// stb_image не распознаёт некоторые экзотические BMP (8-битные с unusual
// palette, или DIB v4/v5 с alpha-масками). Делаем минимальный sanity-check по
// заголовку — если 'BM' и width/height в разумных пределах, считаем валидным.
[[nodiscard]] bool LooksLikePlausibleBmp(const std::vector<unsigned char>& buf) {
    if (buf.size() < 26) {
        return false;
    }
    if (buf[0] != 'B' || buf[1] != 'M') {
        return false;
    }
    auto rd32 = [&](std::size_t off) {
        return static_cast<std::uint32_t>(buf[off])
             | (static_cast<std::uint32_t>(buf[off + 1]) << 8)
             | (static_cast<std::uint32_t>(buf[off + 2]) << 16)
             | (static_cast<std::uint32_t>(buf[off + 3]) << 24);
    };
    const auto dibSize = rd32(14);
    if (dibSize < 12 || dibSize > 256) {
        return false;
    }
    const auto width  = rd32(18);
    const auto height = rd32(22);
    return width > 0 && width <= 16384 && height > 0 && height <= 16384;
}

// Базовая валидация текстур: stbi_info_from_memory читает только заголовок,
// не декодирует пиксели; возвращает (width, height, channels).
ValidationRecord ValidateTexture(const fs::path& file, std::string ext) {
    ValidationRecord rec;
    rec.file = file;
    rec.extension = std::move(ext);
    rec.version = 0;

    std::vector<unsigned char> buf;
    if (!ReadFileToBuffer(file, buf)) {
        rec.status = ValidationStatus::Error;
        rec.problem = "failed to read file";
        rec.recommendation = "Check path and access permissions.";
        return rec;
    }
    if (buf.empty()) {
        rec.status = ValidationStatus::Error;
        rec.problem = "empty file";
        rec.recommendation = "Zero-length file — replace with a valid one.";
        return rec;
    }

    int x = 0, y = 0, comp = 0;
    if (stbi_info_from_memory(buf.data(), static_cast<int>(buf.size()), &x, &y, &comp) == 0) {
        if (ext == "bmp" && LooksLikePlausibleBmp(buf)) {
            // stb_image не понимает этот вариант BMP, но заголовок выглядит
            // здраво — движок (через D3DX/lwTex) такие BMP грузит. Warning,
            // чтобы было видно в отчёте, но Fixer не удалит.
            rec.status = ValidationStatus::Warning;
            rec.problem = std::format("stbi_info_from_memory rejected BMP (likely unsupported "
                                      "subformat): {}",
                                      stbi_failure_reason() ? stbi_failure_reason() : "unknown");
            rec.recommendation = "Manually verify; engine may still load it.";
            return rec;
        }
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("stbi_info_from_memory failed: {}",
                                  stbi_failure_reason() ? stbi_failure_reason() : "unknown");
        rec.recommendation = "File is not a recognised texture format.";
        return rec;
    }
    if (x <= 0 || y <= 0 || comp < 1 || comp > 4) {
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("invalid dimensions: {}x{}x{}", x, y, comp);
        rec.recommendation = "Header parsed but dimensions/channels are out of range.";
        return rec;
    }

    rec.status = ValidationStatus::Ok;
    return rec;
}

} // namespace

std::string_view ToString(ValidationStatus s) noexcept {
    switch (s) {
    case ValidationStatus::Ok:      return "Ok";
    case ValidationStatus::Warning: return "Warning";
    case ValidationStatus::Error:   return "Error";
    }
    return "?";
}

ValidationRecord ValidateFile(const fs::path& file) {
    const std::string ext = ExtLower(file);

    // LgoLoader::CheckedNewArray бросает std::length_error при подозрительно
    // большом *_num (битое поле в .lgo) — нам нужно перевести это исключение в
    // ValidationStatus::Error для текущего файла, чтобы scanner не падал и
    // продолжил обработку остальных. Та же защита покрывает std::bad_alloc от
    // обычных new[] на гигантских массивах и любые иные std::exception.
    try {
        if (ext == "lgo") return ValidateLgo(file);
        if (ext == "lmo") return ValidateLmo(file);
        if (ext == "lxo") return ValidateLxo(file);
        if (ext == "lab") return ValidateLab(file);
        if (ext == "eff") return ValidateEff(file);
        if (ext == "par") return ValidatePar(file);
        if (ext == "csf") return ValidateCsf(file);
        if (ext == "let") return ValidateLet(file);
        if (ext == "map") return ValidateMap(file);
        if (ext == "obj") return ValidateObj(file);
        if (ext == "rbo") return ValidateRbo(file);
        if (ext == "dds") return ValidateDds(file);
        if (ext == "bmp" || ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "tga") {
            return ValidateTexture(file, ext);
        }

        ValidationRecord rec;
        rec.file = file;
        rec.extension = ext;
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("unsupported extension: .{}", ext);
        rec.recommendation = "Remove file from --scope or add support in Validator.";
        return rec;
    }
    catch (const std::exception& e) {
        ValidationRecord rec;
        rec.file = file;
        rec.extension = ext;
        rec.status = ValidationStatus::Error;
        rec.problem = std::format("loader threw {}: {}",
                                   typeid(e).name(), e.what());
        rec.recommendation =
            "Loader detected a corrupt field (likely *_num overflow) and "
            "aborted. Re-export from source.";
        return rec;
    }
}

} // namespace pkotool
