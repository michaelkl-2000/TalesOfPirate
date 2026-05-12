#pragma once

#include "lwDirectX.h"
#include "lwHeader.h"

#include <vector>

namespace Corsairs::Engine::Render {
class lwISysGraphics;
} // namespace Corsairs::Engine::Render

namespace Corsairs::Engine::Render {

// Хранилище vertex-shader'ов и vertex-declaration'ов для effect-подсистемы.
// Раньше эти таблицы (shader_type/decl_type/shader_file/decl_tab) дублировались
// в CMPResManger::LoadTotalVShader и CMPResManger::OnResetDevice — теперь
// одна точка истины внутри стора.
//
// Индексация:
//   shaders[0..3] = VSTU_EFFECT_E1..E4 (effect1..4)
//   decls[0..3]   = vertex declarations для effect1..4
//   decls[4]      = декларация для shade effect
//   decls[5]      = декларация для minimap
//   shadeMapVS, minimapVS — отдельные shader'ы (E6).
//
// Soft-fallback: если устройство не тянет VS1.1/PS1.4, Get*ByID возвращают
// нулевой shader (эффект effect1 как baseline). m_bUseSoft определяется в
// CMPResManger::InitRes через GetDeviceCaps, но LoadAll вызывается раньше
// (из MPRender::Init), поэтому грузим всегда полный набор; fallback
// инжектится через SetSoftFallback(bool) уже после.
class EffectShaderStore {
public:
    static EffectShaderStore& Instance();

    void SetSoftFallback(bool useSoft) noexcept;

    bool LoadAll(Corsairs::Engine::Render::lwISysGraphics* sysGraphics);
    bool Restore(Corsairs::Engine::Render::lwISysGraphics* sysGraphics);
    void Clear();

    [[nodiscard]] IDirect3DVertexShaderX*      GetVShaderByID(int id) const;
    [[nodiscard]] IDirect3DVertexDeclarationX* GetVDeclByID(int id) const;
    [[nodiscard]] IDirect3DVertexShaderX*      GetShadeVS() const noexcept;
    [[nodiscard]] IDirect3DVertexDeclarationX* GetShadeVDecl() const;
    [[nodiscard]] IDirect3DVertexShaderX*      GetMinimapVS() const noexcept;
    [[nodiscard]] IDirect3DVertexDeclarationX* GetMinimapVDecl() const;

private:
    EffectShaderStore() = default;
    EffectShaderStore(const EffectShaderStore&)            = delete;
    EffectShaderStore& operator=(const EffectShaderStore&) = delete;

    std::vector<IDirect3DVertexShaderX*>      _shaders;  // [0..3]
    std::vector<IDirect3DVertexDeclarationX*> _decls;    // [0..3] effect, [4] shade, [5] minimap
    IDirect3DVertexShaderX*                   _shadeMapVS{nullptr};
    IDirect3DVertexShaderX*                   _minimapVS{nullptr};
    bool                                      _useSoft{false};
};

} // namespace Corsairs::Engine::Render
