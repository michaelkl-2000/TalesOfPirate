#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectRenderContext.h"

#include "MPRender.h"
#include "SysGraphics.h"

namespace Corsairs::Engine::Render {

EffectRenderContext& EffectRenderContext::Instance() {
    static EffectRenderContext instance;
    return instance;
}

void EffectRenderContext::Init(MPRender* dev, D3DXMATRIX* viewMat, D3DXMATRIX* viewProjMat) {
    _viewMat     = viewMat;
    _viewProjMat = viewProjMat;

    UpdateBackBuffer(dev);

    D3DXMatrixInverse(&_matBBoard, nullptr, _viewMat);
    _matBBoard._41 = 0.0f;
    _matBBoard._42 = 0.0f;
    _matBBoard._43 = 0.0f;
}

void EffectRenderContext::UpdateMatrices() {
    if (!_viewMat || !_viewProjMat) {
        return;
    }
    D3DXMatrixInverse(&_matBBoard, nullptr, _viewMat);
    _matBBoard._41 = 0.0f;
    _matBBoard._42 = 0.0f;
    _matBBoard._43 = 0.0f;
    D3DXMatrixTranspose(&_matViewProjPose, _viewProjMat);
}

void EffectRenderContext::UpdateBackBuffer(MPRender* dev) {
    if (!dev) {
        return;
    }
    IDirect3DSurfaceX* backBuffer = nullptr;
    dev->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
    backBuffer->GetDesc(&_backBuffer);
    backBuffer->Release();

    D3DXMatrixOrthoLH(&_mat2dViewProj,
                      static_cast<float>(_backBuffer.Width),
                      static_cast<float>(_backBuffer.Height),
                      0.0f, 1.0f);

    _fontBkWidth  = static_cast<int>(_backBuffer.Width)  / 2;
    _fontBkHeight = static_cast<int>(_backBuffer.Height) / 2;
}

} // namespace Corsairs::Engine::Render
