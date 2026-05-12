#pragma once

#include "MindPower.h"

class MPRender;

namespace Corsairs::Engine::Render {

// Состояние кадра подсистемы эффектов под Corsairs::Engine::Render:
// матрицы вид/проекция/billboard, размеры back-buffer'а и font-overlay.
// Это **состояние кадра**, не "менеджер" — не владеет ресурсами.
//
// Жизненный цикл:
//   Init(dev, viewMat, viewProjMat) — из CMPResManger::InitRes. Запоминает
//     внешние указатели на view/viewProj (владеет MPRender, нам — только
//     наблюдать), сразу вычисляет billboard и обновляет back-buffer.
//   UpdateMatrices() — каждый кадр (FrameMove): пересчёт billboard и
//     viewProj-pose; Get*Mat() возвращает свежие значения.
//   UpdateBackBuffer(dev) — при OnResetDevice/реinit (размер окна изменился):
//     забирает D3DSURFACE_DESC и пересчитывает 2D-ортогональную проекцию +
//     font-overlay центр.
//
// Геттеры — non-const и возвращают указатели/ссылки на члены, потому что
// legacy-callsite берут адрес для D3DXVec3TransformCoord и т.п.
class EffectRenderContext {
public:
    static EffectRenderContext& Instance();

    void Init(MPRender* dev, D3DXMATRIX* viewMat, D3DXMATRIX* viewProjMat);
    void UpdateMatrices();
    void UpdateBackBuffer(MPRender* dev);

    [[nodiscard]] D3DXMATRIX* GetBBoardMat() noexcept       { return &_matBBoard; }
    [[nodiscard]] D3DXMATRIX* GetViewProjMat() noexcept     { return &_matViewProjPose; }
    [[nodiscard]] D3DXMATRIX* Get2DViewProjMat() noexcept   { return &_mat2dViewProj; }

    [[nodiscard]] int GetBackBufferWidth()  const noexcept  { return static_cast<int>(_backBuffer.Width); }
    [[nodiscard]] int GetBackBufferHeight() const noexcept  { return static_cast<int>(_backBuffer.Height); }

    [[nodiscard]] int& GetFontBkWidth() noexcept            { return _fontBkWidth; }
    [[nodiscard]] int& GetFontBkHeight() noexcept           { return _fontBkHeight; }

    [[nodiscard]] const D3DSURFACE_DESC& GetBackBuffer() const noexcept { return _backBuffer; }

private:
    EffectRenderContext() = default;
    EffectRenderContext(const EffectRenderContext&)            = delete;
    EffectRenderContext& operator=(const EffectRenderContext&) = delete;

    D3DXMATRIX*       _viewMat{nullptr};      // not owned
    D3DXMATRIX*       _viewProjMat{nullptr};  // not owned

    D3DXMATRIX        _matBBoard{};
    D3DXMATRIX        _matViewProjPose{};
    D3DXMATRIX        _mat2dViewProj{};

    D3DSURFACE_DESC   _backBuffer{};
    int               _fontBkWidth{0};
    int               _fontBkHeight{0};
};

} // namespace Corsairs::Engine::Render
