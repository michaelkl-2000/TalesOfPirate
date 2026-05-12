#pragma once

#include "MindPower.h"

#include <unordered_set>

class CEffectModel;
class MPRender;

LW_BEGIN
class lwISysGraphics;
LW_END

namespace Corsairs::Engine::Render {

// Пул динамических tob-mesh'ей (CEffectModel) под Corsairs::Engine::Render.
// Tob-mesh — это вращающийся mesh без stable ID; владение — пока соответствующий
// I_Effect жив. New/Delete вызываются из I_Effect (см. I_Effect.cpp).
//
// Контекст (device/sysGraphics) инжектится из CMPResManger::InitRes ДО первого
// NewTobMesh. unordered_set даёт O(1) erase по указателю.
class TobMeshStore {
public:
    static TobMeshStore& Instance();

    void SetDevice(MPRender* dev) noexcept;
    void SetSysGraphics(LW_NAMESPACE::lwISysGraphics* sysGraphics) noexcept;

    CEffectModel* NewTobMesh();
    bool          DeleteTobMesh(CEffectModel& model);

    [[nodiscard]] int Count() const noexcept;

    void Clear();

private:
    TobMeshStore() = default;
    ~TobMeshStore();
    TobMeshStore(const TobMeshStore&)            = delete;
    TobMeshStore& operator=(const TobMeshStore&) = delete;

    MPRender*                            _dev{nullptr};
    LW_NAMESPACE::lwISysGraphics*        _sysGraphics{nullptr};
    std::unordered_set<CEffectModel*>    _meshes;
};

} // namespace Corsairs::Engine::Render
