#include "Validator.h"

#include "AssetLoaders.h"
#include "lwExpObj.h"
#include "MPModelEff.h"      // EffectFileInfo
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
