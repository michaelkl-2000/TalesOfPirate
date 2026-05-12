#pragma once

#include "MindPower.h"

#include <cstdint>
#include <vector>

class CMPPartCtrl;
class CMPResManger;
class MPMap;

namespace Corsairs::Engine::Render {

// Пул живых CMPPartCtrl-экземпляров под Corsairs::Engine::Render.
// Capacity = 150 (MAXMSG_COUNT). Slot-array + LIFO-freelist (back==pop) —
// освобождённый при Render() слот переиспользуется первым при следующем Spawn.
// Каждый slot владеет CMPPartCtrl*; null = свободный.
//
// Контекст (resMgr) инжектится из CMPResManger::InitRes ДО первого Spawn —
// нужен для BindingRes() свеже-склонированного CMPPartCtrl.
//
// Жизненный цикл слота:
//   Spawn(template, pos, map) → new CMPPartCtrl + CopyPartCtrl + BindingRes +
//     Reset + MoveTo + Play(1). Slot занят.
//   FrameMove(dt) — продвигает анимацию всех живых.
//   Render() — рендерит; завершившие (!IsPlaying) — освобождает slot.
//   Reset() — Reset на каждом живом slot'е, но владение сохраняется.
//   Clear() — освобождает все slot'ы и обнуляет freelist.
class ParticleInstancePool {
public:
    static constexpr int kCapacity = 150;

    static ParticleInstancePool& Instance();

    void SetResMgr(CMPResManger* mgr) noexcept;

    void Spawn(CMPPartCtrl* templ, const D3DXVECTOR3& vPos, MPMap* pMap);

    void FrameMove(DWORD dwTime);
    void Render();
    void Reset();
    void Clear();

    [[nodiscard]] int LiveCount() const noexcept;
    [[nodiscard]] int FreeCount() const noexcept;

private:
    ParticleInstancePool() = default;
    ~ParticleInstancePool();
    ParticleInstancePool(const ParticleInstancePool&)            = delete;
    ParticleInstancePool& operator=(const ParticleInstancePool&) = delete;

    void EnsureInit();

    CMPResManger*               _resMgr{nullptr};
    std::vector<CMPPartCtrl*>   _slots;     // capacity kCapacity, null = free
    std::vector<std::uint16_t>  _freeIDs;   // LIFO (back==pop)
    bool                        _initialized{false};
};

} // namespace Corsairs::Engine::Render
