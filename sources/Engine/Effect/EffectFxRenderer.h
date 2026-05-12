#pragma once

#include "MindPower.h"
#include "EffectFile.h"

class MPRender;

namespace Corsairs::Engine::Render {

// Обёртка над CMPEffectFile (shader\dx9\eff.fx) под Corsairs::Engine::Render.
// Владеет CMPEffectFile, проксирует Begin/End/Pass и device-lifecycle callback'и.
// RestoreEffect — сброс fixed-pipeline render-state перед очередным effect-render
// (legacy: CMPResManger::RestoreEffect).
//
// Жизненный цикл:
//   Init(dev, path) — InitDev + LoadEffectFromFile. Зовётся из CMPResManger::InitRes.
//   OnLostDevice/OnResetDevice — диспатчатся из EffectDeviceCallbacks.
//   Free — вызывается из CMPResManger::ReleaseTotalRes.
class EffectFxRenderer {
public:
    static EffectFxRenderer& Instance();

    bool Init(MPRender* dev, const char* fxPath);
    void Free();

    void BeginEffect(int iIdx);
    void EndEffect();

    void RestoreEffect();

    BOOL OnLostDevice();
    BOOL OnResetDevice();

    [[nodiscard]] CMPEffectFile* GetEffectFile() noexcept { return &_effectFile; }

private:
    EffectFxRenderer() = default;
    ~EffectFxRenderer();
    EffectFxRenderer(const EffectFxRenderer&)            = delete;
    EffectFxRenderer& operator=(const EffectFxRenderer&) = delete;

    MPRender*     _dev{nullptr};
    CMPEffectFile _effectFile;
};

} // namespace Corsairs::Engine::Render
