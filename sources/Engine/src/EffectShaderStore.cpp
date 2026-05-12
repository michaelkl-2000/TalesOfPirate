#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectShaderStore.h"

#include "lwInterface.h"
#include "lwSysGraphics.h"
#include "ShaderLoad.h"

#include <format>
#include <string>

namespace Corsairs::Engine::Render {

namespace {

// Единая таблица типов VS/decl/файлов и vertex-element'ов для effect-подсистемы.
// Раньше эти массивы дублировались в LoadTotalVShader и OnResetDevice — теперь
// одна точка истины.
constexpr DWORD kShaderTypes[] = {
    VSTU_EFFECT_E1,
    VSTU_EFFECT_E2,
    VSTU_EFFECT_E3,
    VSTU_EFFECT_E4,
    VSTU_SHADE_E6,
    VSTU_MINIMAP_E6,
};

constexpr DWORD kDeclTypes[] = {
    VDT_EFF_134,
    VDT_EFF_2,
    VDT_EFF_134,
    VDT_EFF_134,
    VDT_EFF_SHADE,
    VDT_EFF_MINIMAP,
};

constexpr const char* kShaderFiles[] = {
    "eff1.hlsl",
    "eff2.hlsl",
    "eff3.hlsl",
    "eff4.hlsl",
    "shadeeff.hlsl",
    "minimap.hlsl",
};

constexpr int kShaderCount = static_cast<int>(LW_ARRAY_LENGTH(kShaderTypes));
constexpr int kEffectShaderCount = 4;          // [0..3] — effect1..4
constexpr int kShadeDeclIndex    = 4;
constexpr int kMinimapDeclIndex  = 5;

D3DVERTEXELEMENT9 kDeclEffect1[] = {
    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,      0},
    {0, 12, D3DDECLTYPE_FLOAT1,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES,  0},
    {0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,         0},
    {0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,      0},
    {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},
};

D3DVERTEXELEMENT9 kDeclEffect2[] = {
    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,      0},
    {0, 12, D3DDECLTYPE_FLOAT1,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES,  0},
    {0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,         0},
    {0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,      0},
    {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},
};

D3DVERTEXELEMENT9 kDeclShade[] = {
    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
    {0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
    {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},
};

D3DVERTEXELEMENT9 kDeclMinimap[] = {
    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0},
    {0, 12, D3DDECLTYPE_FLOAT1,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,  0},
    {0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,        0},
    {0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     0},
    {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},
};

D3DVERTEXELEMENT9* const kDeclTables[] = {
    kDeclEffect1,
    kDeclEffect2,
    kDeclEffect1,
    kDeclEffect1,
    kDeclShade,
    kDeclMinimap,
};

lwIShaderMgr* GetShaderMgr(lwISysGraphics* sysGraphics) {
    lwIResourceMgr* resMgr = nullptr;
    sysGraphics->GetInterface(reinterpret_cast<LW_VOID**>(&resMgr), LW_GUID_RESOURCEMGR);
    return resMgr ? resMgr->GetShaderMgr() : nullptr;
}

} // namespace

EffectShaderStore& EffectShaderStore::Instance() {
    static EffectShaderStore instance;
    return instance;
}

void EffectShaderStore::SetSoftFallback(bool useSoft) noexcept {
    _useSoft = useSoft;
}

bool EffectShaderStore::LoadAll(lwISysGraphics* sysGraphics) {
    lwISystem* sys = sysGraphics->GetSystem();

    lwIPathInfo* pathInfo = nullptr;
    sys->GetInterface(reinterpret_cast<LW_VOID**>(&pathInfo), LW_GUID_PATHINFO);

    lwIShaderMgr* shaderMgr = GetShaderMgr(sysGraphics);
    if (!shaderMgr || !pathInfo) {
        return false;
    }

    _decls.assign(kShaderCount, nullptr);

    for (int i = 0; i < kShaderCount; ++i) {
        IDirect3DVertexDeclarationX* decl = nullptr;
        if (LW_SUCCEEDED(shaderMgr->QueryVertexDeclaration(&decl, kDeclTypes[i]))) {
            _decls[i] = decl;
            continue;
        }

        if (LW_RESULT r = shaderMgr->RegisterVertexDeclaration(kDeclTypes[i], kDeclTables[i]);
            LW_FAILED(r)) {
            ToLogService("errors", LogLevel::Error,
                         "[{}] RegisterVertexDeclaration failed: i={}, decl_type={}, ret={}",
                         __FUNCTION__, i, kDeclTypes[i], static_cast<long long>(r));
            return false;
        }
        shaderMgr->QueryVertexDeclaration(&decl, kDeclTypes[i]);
        _decls[i] = decl;
    }

    _shaders.clear();
    _shadeMapVS = nullptr;
    _minimapVS  = nullptr;

    for (int i = 0; i < kShaderCount; ++i) {
        const std::string path = std::format("{}{}", pathInfo->GetPath(PATH_TYPE_SHADER), kShaderFiles[i]);
        if (LW_RESULT r = shaderMgr->RegisterVertexShader(kShaderTypes[i], path.c_str());
            LW_FAILED(r)) {
            ToLogService("errors", LogLevel::Error,
                         "[{}] RegisterVertexShader failed: i={}, shader_type={}, path={}, ret={}",
                         __FUNCTION__, i, kShaderTypes[i], path, static_cast<long long>(r));
            return false;
        }

        if (i < kEffectShaderCount) {
            _shaders.emplace_back(nullptr);
            shaderMgr->QueryVertexShader(&_shaders.back(), kShaderTypes[i]);
        }
        else if (i == kShadeDeclIndex) {
            shaderMgr->QueryVertexShader(&_shadeMapVS, kShaderTypes[i]);
        }
        else if (i == kMinimapDeclIndex) {
            shaderMgr->QueryVertexShader(&_minimapVS, kShaderTypes[i]);
        }
    }

    return true;
}

bool EffectShaderStore::Restore(lwISysGraphics* sysGraphics) {
    lwIShaderMgr* shaderMgr = GetShaderMgr(sysGraphics);
    if (!shaderMgr) {
        return false;
    }

    if (_useSoft) {
        // Soft-fallback: восстанавливаем только effect2 (slot 0).
        if (!_shaders.empty()) {
            shaderMgr->QueryVertexShader(&_shaders[0], kShaderTypes[1]);
        }
        return true;
    }

    for (int i = 0; i < kShaderCount; ++i) {
        if (i < kEffectShaderCount) {
            if (i < static_cast<int>(_shaders.size())) {
                shaderMgr->QueryVertexShader(&_shaders[i], kShaderTypes[i]);
            }
        }
        else if (i == kShadeDeclIndex) {
            shaderMgr->QueryVertexShader(&_shadeMapVS, kShaderTypes[i]);
        }
        else if (i == kMinimapDeclIndex) {
            shaderMgr->QueryVertexShader(&_minimapVS, kShaderTypes[i]);
        }
        if (i < static_cast<int>(_decls.size())) {
            shaderMgr->QueryVertexDeclaration(&_decls[i], kDeclTypes[i]);
        }
    }
    return true;
}

void EffectShaderStore::Clear() {
    _shaders.clear();
    _decls.clear();
    _shadeMapVS = nullptr;
    _minimapVS  = nullptr;
}

IDirect3DVertexShaderX* EffectShaderStore::GetVShaderByID(int id) const {
    if (_shaders.empty()) {
        return nullptr;
    }
    const int idx = _useSoft ? 0 : id;
    if (idx < 0 || idx >= static_cast<int>(_shaders.size())) {
        return nullptr;
    }
    return _shaders[idx];
}

IDirect3DVertexDeclarationX* EffectShaderStore::GetVDeclByID(int id) const {
    if (_decls.empty()) {
        return nullptr;
    }
    const int idx = _useSoft ? 0 : id;
    if (idx < 0 || idx >= static_cast<int>(_decls.size())) {
        return nullptr;
    }
    return _decls[idx];
}

IDirect3DVertexShaderX* EffectShaderStore::GetShadeVS() const noexcept {
    return _shadeMapVS;
}

IDirect3DVertexDeclarationX* EffectShaderStore::GetShadeVDecl() const {
    return kShadeDeclIndex < static_cast<int>(_decls.size()) ? _decls[kShadeDeclIndex] : nullptr;
}

IDirect3DVertexShaderX* EffectShaderStore::GetMinimapVS() const noexcept {
    return _minimapVS;
}

IDirect3DVertexDeclarationX* EffectShaderStore::GetMinimapVDecl() const {
    return kMinimapDeclIndex < static_cast<int>(_decls.size()) ? _decls[kMinimapDeclIndex] : nullptr;
}

} // namespace Corsairs::Engine::Render
