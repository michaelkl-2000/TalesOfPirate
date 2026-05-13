#pragma once

#include "MindPower.h"
#include "lwHeader.h"

class MPRender;

namespace Corsairs::Engine::Render {

// Orchestrator OnLost/OnReset device-callback'ов для всех подсистем эффектов.
// Install() регистрирует функции в lwRegisterOutputLoseDeviceProc / Reset —
// они дёргаются движком при потере/восстановлении D3D9-устройства.
//
// При OnLost дёргается EffectFxRenderer::OnLostDevice — больше ничего; vertex
// shader'ы и buffer'ы либо managed-pool, либо пересоздаются на OnReset.
//
// При OnReset:
//   1. EffectFxRenderer::OnResetDevice — переинициализирует D3DXEffect.
//   2. EffectShaderStore::Restore(sysGraphics) — переcоздаёт VS/VDecl.
//   3. EffectRenderContext::UpdateBackBuffer(dev) — забирает новый desc + 2D-projection.
//   4. EffectMeshStore::RecreateBuiltins — VB/IB для 7 primitive'ов.
class EffectDeviceCallbacks {
public:
    static EffectDeviceCallbacks& Instance();

    void Install(MPRender* dev, Corsairs::Engine::Render::ISysGraphics* sysGraphics);

    // Внутренние реализации (вызываются через статические C-trampoline'ы).
    LW_RESULT OnLost();
    LW_RESULT OnReset();

private:
    EffectDeviceCallbacks() = default;
    EffectDeviceCallbacks(const EffectDeviceCallbacks&)            = delete;
    EffectDeviceCallbacks& operator=(const EffectDeviceCallbacks&) = delete;

    static LW_RESULT TrampolineOnLost();
    static LW_RESULT TrampolineOnReset();

    MPRender*                       _dev{nullptr};
    Corsairs::Engine::Render::ISysGraphics*   _sysGraphics{nullptr};
};

} // namespace Corsairs::Engine::Render
