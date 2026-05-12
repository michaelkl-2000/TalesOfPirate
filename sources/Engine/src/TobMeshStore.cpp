#include "StdAfx.h"
#include "GlobalInc.h"

#include "TobMeshStore.h"

#include "I_Effect.h"
#include "MPRender.h"
#include "lwSysGraphics.h"

using namespace Corsairs::Engine::Render;namespace Corsairs::Engine::Render {

TobMeshStore& TobMeshStore::Instance() {
    static TobMeshStore instance;
    return instance;
}

TobMeshStore::~TobMeshStore() {
    Clear();
}

void TobMeshStore::SetDevice(MPRender* dev) noexcept {
    _dev = dev;
}

void TobMeshStore::SetSysGraphics(lwISysGraphics* sysGraphics) noexcept {
    _sysGraphics = sysGraphics;
}

CEffectModel* TobMeshStore::NewTobMesh() {
    auto* model = new CEffectModel;
    model->InitDevice(_dev, _sysGraphics ? _sysGraphics->GetResourceMgr() : nullptr);
    _meshes.insert(model);
    return model;
}

bool TobMeshStore::DeleteTobMesh(CEffectModel& model) {
    auto it = _meshes.find(&model);
    if (it == _meshes.end()) {
        return false;
    }
    delete *it;
    _meshes.erase(it);
    return true;
}

int TobMeshStore::Count() const noexcept {
    return static_cast<int>(_meshes.size());
}

void TobMeshStore::Clear() {
    for (auto* model : _meshes) {
        delete model;
    }
    _meshes.clear();
}

} // namespace Corsairs::Engine::Render
