#include "StdAfx.h"
#include "GlobalInc.h"

#include "ParticleInstancePool.h"

#include "MPParticleCtrl.h"

namespace Corsairs::Engine::Render {

ParticleInstancePool& ParticleInstancePool::Instance() {
    static ParticleInstancePool instance;
    return instance;
}

ParticleInstancePool::~ParticleInstancePool() {
    Clear();
}

void ParticleInstancePool::SetResMgr(CMPResManger* mgr) noexcept {
    _resMgr = mgr;
}

void ParticleInstancePool::EnsureInit() {
    if (_initialized) {
        return;
    }
    _slots.assign(kCapacity, nullptr);
    _freeIDs.clear();
    _freeIDs.reserve(kCapacity);
    // Идём от kCapacity-1 к 0, чтобы pop_back брал индекс 0 первым —
    // как в legacy (S_FVector::pop_front с initial [0..N-1]).
    for (int i = kCapacity - 1; i >= 0; --i) {
        _freeIDs.push_back(static_cast<std::uint16_t>(i));
    }
    _initialized = true;
}

void ParticleInstancePool::Spawn(CMPPartCtrl* templ, const D3DXVECTOR3& vPos, MPMap* pMap) {
    EnsureInit();
    if (!templ || _freeIDs.empty()) {
        return;
    }
    const std::uint16_t idx = _freeIDs.back();
    _freeIDs.pop_back();

    auto* ctrl = new CMPPartCtrl;
    ctrl->CopyPartCtrl(templ);
    ctrl->BindingRes(_resMgr);
    ctrl->Reset();
    ctrl->MoveTo(const_cast<D3DXVECTOR3*>(&vPos), pMap);
    ctrl->Play(1);
    _slots[idx] = ctrl;
}

void ParticleInstancePool::FrameMove(DWORD dwTime) {
    if (!_initialized || LiveCount() == 0) {
        return;
    }
    for (int i = 0; i < kCapacity; ++i) {
        if (_slots[i]) {
            _slots[i]->FrameMove(dwTime);
        }
    }
}

void ParticleInstancePool::Render() {
    if (!_initialized || LiveCount() == 0) {
        return;
    }
    for (int i = 0; i < kCapacity; ++i) {
        auto*& slot = _slots[i];
        if (!slot) {
            continue;
        }
        if (!slot->IsPlaying()) {
            delete slot;
            slot = nullptr;
            _freeIDs.push_back(static_cast<std::uint16_t>(i));
            continue;
        }
        slot->Render();
    }
}

void ParticleInstancePool::Reset() {
    if (!_initialized) {
        return;
    }
    for (auto* slot : _slots) {
        if (slot) {
            slot->Reset();
        }
    }
}

void ParticleInstancePool::Clear() {
    for (auto& slot : _slots) {
        SAFE_DELETE(slot);
    }
    _slots.clear();
    _freeIDs.clear();
    _initialized = false;
}

int ParticleInstancePool::LiveCount() const noexcept {
    return kCapacity - static_cast<int>(_freeIDs.size());
}

int ParticleInstancePool::FreeCount() const noexcept {
    return static_cast<int>(_freeIDs.size());
}

} // namespace Corsairs::Engine::Render
