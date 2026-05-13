#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectMeshStore.h"

#include "I_Effect.h"
#include "MPRender.h"
#include "lwIUtil.h"
#include "SysGraphics.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>

using namespace Corsairs::Engine::Render;namespace Corsairs::Engine::Render {

namespace {

std::string ToLowerCopy(std::string_view src) {
    std::string out;
    out.reserve(src.size());
    for (char c : src) {
        out.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

} // namespace

EffectMeshStore& EffectMeshStore::Instance() {
    static EffectMeshStore instance;
    return instance;
}

EffectMeshStore::~EffectMeshStore() {
    Clear();
}

void EffectMeshStore::SetDevice(MPRender* dev) noexcept {
    _dev = dev;
}

void EffectMeshStore::SetSystem(ISystem* sys, ISysGraphics* sysGraphics) noexcept {
    _sys         = sys;
    _sysGraphics = sysGraphics;
}

bool EffectMeshStore::LoadAllFrom(std::string_view directory) {
    CreateBuiltins();
    return LoadLgoFrom(directory);
}

void EffectMeshStore::CreateBuiltins() {
    if (_meshes.empty()) {
        _meshes.assign(kCapacity, nullptr);
    }
    _names.clear();
    _index.clear();

    IResourceMgr* res = _sysGraphics ? _sysGraphics->GetResourceMgr() : nullptr;

    auto registerPrimitive = [&](const char* name) -> CEffectModel* {
        const int id = static_cast<int>(_names.size());
        auto* mesh = new CEffectModel;
        mesh->InitDevice(_dev, res);
        _meshes[id] = mesh;
        _names.push_back(name);
        _index.emplace(ToLowerCopy(name), id);
        return mesh;
    };

    // 7 fixed primitives. Порядок IDs 0..6 сериализуется в .par — менять нельзя.
    registerPrimitive(MESH_TRI)->CreateTriangle();
    registerPrimitive(MESH_RECT)->CreateRect();
    registerPrimitive(MESH_PLANERECT)->CreatePlaneRect();
    registerPrimitive(MESH_PLANETRI)->CreatePlaneTriangle();
    registerPrimitive(MESH_RECTZ)->CreateRectZ();
    registerPrimitive(MESH_CONE)->CreateCone(8, 3, 2);
    registerPrimitive(MESH_CYLINDER)->CreateCylinder(8, 3, 1, 3);

    _count = static_cast<int>(_names.size());

    if (!_shadeModel) {
        _shadeModel = new CEffectModel;
        _shadeModel->InitDevice(_dev, res);
        _shadeModel->CreateShadeModel();
    }
}

bool EffectMeshStore::LoadLgoFrom(std::string_view directory) {
    const std::string pattern = std::format("{}\\effect\\*.lgo", directory);

    WIN32_FIND_DATA findData{};
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return true;
    }

    IPathInfo* pathInfo = nullptr;
    std::string oldPath;
    if (_sys) {
        _sys->GetInterface(reinterpret_cast<LW_VOID**>(&pathInfo), LW_GUID_PATHINFO);
        if (pathInfo) {
            oldPath = pathInfo->GetPath(PathInfoType::PATH_TYPE_MODEL_ITEM);
            pathInfo->SetPath(PathInfoType::PATH_TYPE_MODEL_ITEM, "model\\effect\\");
        }
    }

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        const std::string_view nameView{findData.cFileName};
        if (nameView.size() < 4 ||
            _stricmp(nameView.data() + nameView.size() - 4, ".lgo") != 0) {
            continue;
        }

        std::string fileName = findData.cFileName;
        std::transform(fileName.begin(), fileName.end(), fileName.begin(),
                       [](unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });

        if (_count >= kCapacity) {
            ToLogService("errors", LogLevel::Error,
                         "[EffectMeshStore] capacity reached at {}", fileName);
            break;
        }

        const int id = _count++;
        auto* mesh = new CEffectModel;
        mesh->InitDevice(_dev);
        mesh->LoadModel(fileName.c_str());

        _meshes[id] = mesh;
        _names.push_back(fileName.c_str());
        _index.emplace(ToLowerCopy(fileName), id);
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    if (pathInfo) {
        pathInfo->SetPath(PathInfoType::PATH_TYPE_MODEL_ITEM, oldPath.c_str());
    }
    return true;
}

void EffectMeshStore::RecreateBuiltins() {
    if (_meshes.size() < 7) {
        return;
    }
    if (_meshes[0]) {
    	_meshes[0]->CreateTriangle();
    }
    if (_meshes[1]) _meshes[1]->CreateRect();
    if (_meshes[2]) {
    	_meshes[2]->CreatePlaneRect();
    }
    if (_meshes[3]) _meshes[3]->CreatePlaneTriangle();
    if (_meshes[4]) {
    	_meshes[4]->CreateRectZ();
    }
    if (_meshes[5]) _meshes[5]->CreateCone(8, 3, 2);
    if (_meshes[6]) {
    	_meshes[6]->CreateCylinder(8, 3, 1, 3);
    }
}

void EffectMeshStore::Clear() {
    for (auto* mesh : _meshes) {
        SAFE_DELETE(mesh);
    }
    _meshes.clear();
    _names.clear();
    _index.clear();
    _count = 0;

    SAFE_DELETE(_shadeModel);
}

int EffectMeshStore::GetID(const s_string& name) const {
    const auto it = _index.find(ToLowerCopy(name));
    return it == _index.end() ? -1 : it->second;
}

CEffectModel* EffectMeshStore::GetByID(int id) {
    if (id < 0 || id >= static_cast<int>(_meshes.size())) {
        ToLogService("errors", LogLevel::Error, "[EffectMeshStore] id out of range: {}", id);
        return nullptr;
    }

    CEffectModel* result = nullptr;

    if (id >= 7) {
        if (!_meshes[id]) {
            _meshes[id] = new CEffectModel;
            _meshes[id]->InitDevice(_dev);

            IPathInfo* pathInfo = nullptr;
            std::string oldPath;
            if (_sys) {
                _sys->GetInterface(reinterpret_cast<LW_VOID**>(&pathInfo), LW_GUID_PATHINFO);
                if (pathInfo) {
                    oldPath = pathInfo->GetPath(PathInfoType::PATH_TYPE_MODEL_ITEM);
                    pathInfo->SetPath(PathInfoType::PATH_TYPE_MODEL_ITEM, "model\\effect\\");
                }
            }

            const bool loaded = _meshes[id]->LoadModel(_names[id].c_str());

            if (pathInfo) {
                pathInfo->SetPath(PathInfoType::PATH_TYPE_MODEL_ITEM, oldPath.c_str());
            }

            if (!loaded) {
                SAFE_DELETE(_meshes[id]);
                ToLogService("errors", LogLevel::Error, "[EffectMeshStore] LoadModel failed: id={}", id);
                return nullptr;
            }
            if (!_meshes[id]->GetObject() || !_meshes[id]->GetObject()->GetPrimitive()) {
                ToLogService("errors", LogLevel::Error,
                             "[EffectMeshStore] no primitive after load: id={}", id);
            } else {
                _meshes[id]->GetObject()->GetPrimitive()->SetState(STATE_TRANSPARENT, 0);
            }
            result = _meshes[id];
        } else if (_meshes[id]->IsUsing()) {
            // Все основные слоты заняты — ищем свободный overflow-slot и копируем туда.
            int n = _count;
            for (; n < kCapacity; ++n) {
                if (_meshes[n] && _meshes[n]->IsUsing()) {
                    continue;
                }
                if (!_meshes[n]) {
                    _meshes[n] = new CEffectModel;
                }
                if (_meshes[n]->_iID != id) {
                    if (!_meshes[n]->Copy(*_meshes[id])) {
                        SAFE_DELETE(_meshes[n]);
                        ToLogService("errors", LogLevel::Error,
                                     "[EffectMeshStore] Copy failed: id={}", id);
                        return nullptr;
                    }
                }
                break;
            }
            if (n >= kCapacity) {
                ToLogService("errors", LogLevel::Error,
                             "[EffectMeshStore] overflow capacity reached at id={}", id);
                return nullptr;
            }
            result = _meshes[n];
        } else {
            result = _meshes[id];
        }
    } else {
        result = _meshes[id];
    }

    if (!result) {
        return nullptr;
    }
    result->_iID = id;
    result->SetUsing(true);
    return result;
}

CEffectModel* EffectMeshStore::GetByName(const s_string& name) {
    const int id = GetID(name);
    if (id == -1) {
        return nullptr;
    }
    return GetByID(id);
}

CEffectModel* EffectMeshStore::GetShadeMesh() const noexcept {
    return _shadeModel;
}

void EffectMeshStore::DeleteMesh(CEffectModel& model) {
    if (model._iID >= 7) {
        model.SetUsing(false);
    }
}

int EffectMeshStore::Count() const noexcept {
    return _count;
}

VEC_string& EffectMeshStore::GetAllNames() noexcept {
    return _names;
}

} // namespace Corsairs::Engine::Render
