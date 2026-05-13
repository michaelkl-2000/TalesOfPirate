#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectDeviceCallbacks.h"

#include "EffectFxRenderer.h"
#include "EffectMeshStore.h"
#include "EffectRenderContext.h"
#include "EffectShaderStore.h"
#include "MPRender.h"
#include "lwIUtil.h"
#include "SysGraphics.h"

using namespace Corsairs::Engine::Render;namespace Corsairs::Engine::Render {

EffectDeviceCallbacks& EffectDeviceCallbacks::Instance() {
    static EffectDeviceCallbacks instance;
    return instance;
}

void EffectDeviceCallbacks::Install(MPRender* dev, ISysGraphics* sysGraphics) {
    _dev         = dev;
    _sysGraphics = sysGraphics;
    lwRegisterOutputLoseDeviceProc(&EffectDeviceCallbacks::TrampolineOnLost);
    lwRegisterOutputResetDeviceProc(&EffectDeviceCallbacks::TrampolineOnReset);
}

LW_RESULT EffectDeviceCallbacks::OnLost() {
    // Соглашение legacy g_OnLostDevice: возвращали BOOL (TRUE/FALSE) как
    // LW_RESULT. LW_FAILED проверяет <0, поэтому оба считаются success;
    // сохраняем то же поведение.
    return static_cast<LW_RESULT>(EffectFxRenderer::Instance().OnLostDevice());
}

LW_RESULT EffectDeviceCallbacks::OnReset() {
    if (!EffectFxRenderer::Instance().OnResetDevice()) {
        return static_cast<LW_RESULT>(FALSE);
    }
    EffectShaderStore::Instance().Restore(_sysGraphics);
    EffectRenderContext::Instance().UpdateBackBuffer(_dev);
    EffectMeshStore::Instance().RecreateBuiltins();
    return static_cast<LW_RESULT>(TRUE);
}

LW_RESULT EffectDeviceCallbacks::TrampolineOnLost() {
    return Instance().OnLost();
}

LW_RESULT EffectDeviceCallbacks::TrampolineOnReset() {
    return Instance().OnReset();
}

} // namespace Corsairs::Engine::Render
