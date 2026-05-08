#include "Fixer.h"

#include "AssetLoaders.h"
#include "SceneFileLoaders.h"
#include "lwExpObj.h"
#include "lwDDSFile.h"       // ResaveDds — нужен полный тип для NULLREF-pipeline
#include "MPModelEff.h"      // EffectFileInfo
#include "MPParticleCtrl.h"  // CMPPartCtrl
#include "logutil.h"

#include <d3d9.h>

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <typeinfo>

namespace pkotool {

namespace fs = std::filesystem;

namespace {

constexpr const char* kLogChannel = "pkotool";

// Резулдьтат одной попытки re-save: успех, либо причина сбоя.
struct ResaveResult {
    bool ok{false};
    std::string detail;
};

// Universal scheme: переименовать original → original.bak, затем загрузить .bak,
// сохранить под original. Если save упал — откатить (.bak → original).
//
// LgoLoader::CheckedNewArray бросает std::length_error при подозрительно большом
// *_num (битое поле в .lgo). Любой такой throw из load()/save() ловим здесь,
// откатываем .bak и возвращаем ResaveResult{false, ...}, чтобы Fixer не уронил
// процесс при первом же corrupt-файле.
template <typename LoadFn, typename SaveFn>
ResaveResult ResaveViaBak(const fs::path& path, LoadFn&& load, SaveFn&& save) {
    const fs::path bak = path.string() + ".bak";

    std::error_code ec;
    fs::rename(path, bak, ec);
    if (ec) {
        return {false, std::format("rename → .bak failed: {}", ec.message())};
    }

    auto rollbackBak = [&]() {
        std::error_code ec2;
        fs::remove(path, ec2);
        ec2.clear();
        fs::rename(bak, path, ec2);
    };

    bool loaded = false;
    try {
        loaded = load(bak.string());
    }
    catch (const std::exception& e) {
        rollbackBak();
        return {false, std::format("load threw {}: {} (rolled back)",
                                    typeid(e).name(), e.what())};
    }
    if (!loaded) {
        // Откатываем: возвращаем .bak на место.
        std::error_code ec2;
        fs::rename(bak, path, ec2);
        return {false, "load from .bak failed (rolled back)"};
    }

    bool saved = false;
    try {
        saved = save(path.string());
    }
    catch (const std::exception& e) {
        rollbackBak();
        return {false, std::format("save threw {}: {} (rolled back)",
                                    typeid(e).name(), e.what())};
    }
    if (!saved) {
        rollbackBak();
        return {false, "save failed (rolled back)"};
    }

    return {true, {}};
}

ResaveResult ResaveLgo(const fs::path& path) {
    using LgoLoader = Corsairs::Engine::Render::LgoLoader;
    MindPower::lwGeomObjInfo* info = nullptr;
    auto cleanup = [&](){ delete info; info = nullptr; };

    auto load = [&](const std::string& f) {
        info = LgoLoader::Load(f);
        return info != nullptr;
    };
    auto save = [&](const std::string& f) {
        const LW_RESULT r = LgoLoader::Save(info, f);
        cleanup();
        return !LW_FAILED(r);
    };

    auto result = ResaveViaBak(path, load, save);
    cleanup();
    return result;
}

ResaveResult ResaveLmo(const fs::path& path) {
    using LgoLoader = Corsairs::Engine::Render::LgoLoader;
    MindPower::lwModelObjInfo info;

    auto load = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::LoadModelObj(info, f));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::SaveModelObj(info, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveLxo(const fs::path& path) {
    using LgoLoader = Corsairs::Engine::Render::LgoLoader;
    MindPower::lwModelInfo info;

    auto load = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::LoadModel(info, f));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::SaveModel(info, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveLab(const fs::path& path) {
    using LgoLoader = Corsairs::Engine::Render::LgoLoader;
    MindPower::lwAnimDataBone info;

    auto load = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::LoadAnimDataBone(info, f));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(LgoLoader::SaveAnimDataBone(info, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveEff(const fs::path& path) {
    using EffectLoader = Corsairs::Engine::Render::EffectLoader;
    EffectFileInfo info;

    auto load = [&](const std::string& f) {
        return !LW_FAILED(EffectLoader::Load(info, f));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(EffectLoader::Save(info, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveMap(const fs::path& path) {
    using MapLoader = Corsairs::Engine::Render::MapLoader;
    Corsairs::Engine::Render::MapInfo info;

    auto load = [&](const std::string& f) {
        Corsairs::Engine::Render::MapLoadDiagnostics diag;
        return !LW_FAILED(MapLoader::LoadEx(info, f, diag));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(MapLoader::Save(info, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveRbo(const fs::path& path) {
    using RboLoader = Corsairs::Engine::Scene::RboLoader;

    // Спецслучай: 0-байтный .rbo (legacy-баг) — просто удаляем, без .bak.
    std::error_code ec;
    if (fs::exists(path, ec) && fs::file_size(path, ec) == 0) {
        if (fs::remove(path, ec)) {
            return {true, "removed empty .rbo (legacy bug artifact)"};
        }
        return {false, std::format("remove failed: {}", ec.message())};
    }

    std::set<ReallyBigObjectInfo> items;
    auto load = [&](const std::string& f) {
        return RboLoader::Load(f, items);
    };
    auto save = [&](const std::string& f) {
        return RboLoader::Save(f, items);
    };
    return ResaveViaBak(path, load, save);
}

// Минимальный D3D9 NULLREF-девайс — нужен только для D3DXCreateTextureFromFileEx
// внутри lwDDSFile::LoadOriginTexture. Окно — desktop-handle, реальный
// рендеринг не происходит. RAII-обёртка чтобы освободить и device, и
// IDirect3D9 в правильном порядке.
class NullRefDevice {
public:
    NullRefDevice() {
        _d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!_d3d) {
            return;
        }
        D3DPRESENT_PARAMETERS pp{};
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.BackBufferFormat = D3DFMT_UNKNOWN;
        pp.hDeviceWindow = GetDesktopWindow();
        const HRESULT hr = _d3d->CreateDevice(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF,
            pp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES,
            &pp, &_dev);
        if (FAILED(hr)) {
            _dev = nullptr;
        }
    }
    ~NullRefDevice() {
        if (_dev != nullptr) _dev->Release();
        if (_d3d != nullptr) _d3d->Release();
    }
    NullRefDevice(const NullRefDevice&) = delete;
    NullRefDevice& operator=(const NullRefDevice&) = delete;

    [[nodiscard]] IDirect3DDevice9* Device() const noexcept { return _dev; }
    [[nodiscard]] bool IsValid() const noexcept { return _dev != nullptr; }

private:
    IDirect3D9*       _d3d{nullptr};
    IDirect3DDevice9* _dev{nullptr};
};

// .dds re-save: load → save через DdsLoader. Требует D3D9-устройство для
// D3DXCreateTextureFromFileEx внутри lwDDSFile::LoadOriginTexture; используем
// NULLREF-девайс, никакого реального рендеринга.
ResaveResult ResaveDds(const fs::path& path) {
    using DdsLoader = Corsairs::Engine::Render::DdsLoader;

    NullRefDevice d3d;
    if (!d3d.IsValid()) {
        return {false, "Direct3D9 NULLREF device creation failed"};
    }
    MindPower::lwDDSFile dds;
    dds.SetDevice(d3d.Device());

    auto load = [&](const std::string& f) {
        return !LW_FAILED(dds.LoadOriginTexture(f, D3DX_DEFAULT, D3DFMT_UNKNOWN, 0));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(DdsLoader::Save(dds, f));
    };
    return ResaveViaBak(path, load, save);
}

ResaveResult ResaveObj(const fs::path& path) {
    using ObjFileLoader = Corsairs::Engine::Scene::ObjFileLoader;

    // Сейчас единственный поддерживаемый «фикс» для .obj — апгрейд legacy 500→600.
    // Полный re-save (header + section index + per-section objs) как у других
    // форматов потребовал бы загрузки всех секций через CSceneObjFile или
    // отдельного MapInfo-аналога; пока не реализовано.
    std::FILE* fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        return {false, "fopen failed"};
    }
    SFileHead head{};
    long sz = 0;
    const bool readOk = ObjFileLoader::ReadHeader(fp, head, sz);
    std::fclose(fp);
    if (!readOk) {
        return {false, "header read failed"};
    }
    if (head.lVersion == ObjFileLoader::kCurrentVersion) {
        return {true, "already at current version (no-op)"};
    }
    if (head.lVersion != ObjFileLoader::kLegacyVersion500) {
        return {false, std::format("unsupported version {} (expected 500 or 600)", head.lVersion)};
    }

    const long convRet = ObjFileLoader::ConvertVer500ToVer600(path.string(), /*keepBackup=*/true);
    if (convRet == 2) {
        return {true, "converted 500 → 600 (kept .bak)"};
    }
    if (convRet == 1) {
        return {true, "already not version 500 (no-op)"};
    }
    return {false, std::format("ConvertVer500ToVer600 failed: {}", convRet)};
}

ResaveResult ResavePar(const fs::path& path) {
    using PartCtrlLoader = Corsairs::Engine::Render::PartCtrlLoader;
    CMPPartCtrl ctrl;

    auto load = [&](const std::string& f) {
        return !LW_FAILED(PartCtrlLoader::Load(ctrl, f));
    };
    auto save = [&](const std::string& f) {
        return !LW_FAILED(PartCtrlLoader::Save(ctrl, f));
    };
    return ResaveViaBak(path, load, save);
}

[[nodiscard]] bool DeleteFile(const fs::path& path) {
    std::error_code ec;
    fs::remove(path, ec);
    return !ec;
}

} // namespace

FixSummary ApplyFix(const std::vector<ValidationRecord>& records) {
    FixSummary s;

    for (const auto& r : records) {
        switch (r.status) {
        case ValidationStatus::Ok:
            ++s.skippedOk;
            break;

        case ValidationStatus::Error:
            if (DeleteFile(r.file)) {
                ToLogService(kLogChannel, LogLevel::Warning,
                             "DELETED: {} ({})", r.file.string(), r.problem);
                ++s.deleted;
            }
            else {
                ToLogService(kLogChannel, LogLevel::Error,
                             "delete failed: {}", r.file.string());
                ++s.failed;
            }
            break;

        case ValidationStatus::Warning: {
            // Re-save поддержан для бинарных MindPower-форматов и сцен-файлов.
            const bool supported = (r.extension == "lgo" || r.extension == "lmo"
                                    || r.extension == "lxo" || r.extension == "lab"
                                    || r.extension == "eff" || r.extension == "par"
                                    || r.extension == "map" || r.extension == "rbo"
                                    || r.extension == "obj" || r.extension == "dds");
            if (!supported) {
                ToLogService(kLogChannel, LogLevel::Info,
                             "skip (re-save for .{} is not implemented): {}",
                             r.extension, r.file.string());
                ++s.skippedUnsupported;
                break;
            }

            ResaveResult res;
            if (r.extension == "lgo")      res = ResaveLgo(r.file);
            else if (r.extension == "lmo") res = ResaveLmo(r.file);
            else if (r.extension == "lxo") res = ResaveLxo(r.file);
            else if (r.extension == "lab") res = ResaveLab(r.file);
            else if (r.extension == "eff") res = ResaveEff(r.file);
            else if (r.extension == "par") res = ResavePar(r.file);
            else if (r.extension == "map") res = ResaveMap(r.file);
            else if (r.extension == "rbo") res = ResaveRbo(r.file);
            else if (r.extension == "obj") res = ResaveObj(r.file);
            else                           res = ResaveDds(r.file);

            if (res.ok) {
                ToLogService(kLogChannel, LogLevel::Warning,
                             "RESAVED (with .bak): {} ({})",
                             r.file.string(), r.problem);
                ++s.resaved;
            }
            else {
                ToLogService(kLogChannel, LogLevel::Error,
                             "resave failed: {}: {}", r.file.string(), res.detail);
                ++s.failed;
            }
            break;
        }
        }
    }

    return s;
}

} // namespace pkotool
