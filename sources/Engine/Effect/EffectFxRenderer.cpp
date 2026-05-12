#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectFxRenderer.h"

#include "MPRender.h"

namespace Corsairs::Engine::Render {

EffectFxRenderer& EffectFxRenderer::Instance() {
    static EffectFxRenderer instance;
    return instance;
}

EffectFxRenderer::~EffectFxRenderer() {
    Free();
}

bool EffectFxRenderer::Init(MPRender* dev, const char* fxPath) {
    _dev = dev;
    _effectFile.InitDev(dev);
    return _effectFile.LoadEffectFromFile(fxPath);
}

void EffectFxRenderer::Free() {
    _effectFile.free();
}

void EffectFxRenderer::BeginEffect(int iIdx) {
    _effectFile.SetTechnique(iIdx);
    _effectFile.Begin(D3DXFX_DONOTSAVESTATE);
    _effectFile.Pass(0);
}

void EffectFxRenderer::EndEffect() {
    _effectFile.End();
}

BOOL EffectFxRenderer::OnLostDevice() {
    return _effectFile.OnLostDevice();
}

BOOL EffectFxRenderer::OnResetDevice() {
    return _effectFile.OnResetDevice();
}

void EffectFxRenderer::RestoreEffect() {
    if (!_dev) {
        return;
    }
    _dev->SetRenderStateForced(D3DRS_ZENABLE, TRUE);
    _dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, TRUE);
    _dev->SetRenderStateForced(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    _dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, FALSE);
    _dev->SetRenderStateForced(D3DRS_ALPHATESTENABLE, FALSE);
    _dev->SetRenderStateForced(D3DRS_DITHERENABLE, FALSE);
    _dev->SetRenderStateForced(D3DRS_CULLMODE, D3DCULL_CCW);
    _dev->SetRenderStateForced(D3DRS_SRCBLEND, D3DBLEND_ONE);
    _dev->SetRenderStateForced(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    _dev->SetRenderStateForced(D3DRS_LIGHTING, TRUE);
    _dev->SetRenderStateForced(D3DRS_CLIPPING, TRUE);

    _dev->GetInterfaceMgr()->dev_obj->SetTextureForced(0, 0);
    _dev->GetInterfaceMgr()->dev_obj->SetTextureForced(1, 0);
    _dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    _dev->SetTextureStageStateForced(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    _dev->SetTextureStageStateForced(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    _dev->SetTextureStageStateForced(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    _dev->SetSamplerStateForced(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    _dev->SetSamplerStateForced(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    _dev->SetSamplerStateForced(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    _dev->SetSamplerStateForced(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    _dev->SetTextureStageStateForced(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    _dev->SetTextureStageStateForced(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _dev->SetTextureStageStateForced(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    _dev->SetTextureStageStateForced(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    _dev->SetTextureStageStateForced(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _dev->SetTextureStageStateForced(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
}

} // namespace Corsairs::Engine::Render
