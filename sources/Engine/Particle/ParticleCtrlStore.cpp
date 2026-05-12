#include "StdAfx.h"
#include "GlobalInc.h"

#include "ParticleCtrlStore.h"

#include "AssetLoaders.h"
#include "MPParticleCtrl.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>

namespace Corsairs::Engine::Render {

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

ParticleCtrlStore& ParticleCtrlStore::Instance() {
    static ParticleCtrlStore instance;
    return instance;
}

ParticleCtrlStore::~ParticleCtrlStore() {
    Clear();
}

void ParticleCtrlStore::SetEffectPath(std::string_view directory) {
    _directory.assign(directory);
}

bool ParticleCtrlStore::LoadAllFrom(std::string_view directory) {
    SetEffectPath(directory);

    if (_partCtrls.empty()) {
        _partCtrls.assign(kCapacity, nullptr);
    }

    const std::string pattern = std::format("{}\\*.par", _directory);
    WIN32_FIND_DATA findData{};
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return true;
    }

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        if (_count >= kCapacity) {
            ToLogService("errors", LogLevel::Error,
                         "[ParticleCtrlStore] capacity reached at {}", findData.cFileName);
            break;
        }

        std::string fileName = findData.cFileName;
        std::transform(fileName.begin(), fileName.end(), fileName.begin(),
                       [](unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });

        const int id = _count++;
        _names.push_back(fileName.c_str());
        _index.emplace(ToLowerCopy(fileName), id);

        const std::string file = std::format("{}\\{}", _directory, fileName);
        if (!LoadInto(id, file)) {
            ToLogService("errors", LogLevel::Error, "Load {} error", fileName);
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
    return true;
}

bool ParticleCtrlStore::LoadInto(int id, std::string_view file) {
    auto* ctrl = new CMPPartCtrl;
    if (LW_FAILED(Corsairs::Engine::Render::PartCtrlLoader::Load(*ctrl, file))) {
        delete ctrl;
        _partCtrls[id] = nullptr;
        return false;
    }
    const D3DXVECTOR3 origin{0.0f, 0.0f, 0.0f};
    ctrl->MoveTo(&origin);
    _partCtrls[id] = ctrl;
    return true;
}

void ParticleCtrlStore::Clear() {
    for (int i = 0; i < _count; ++i) {
        SAFE_DELETE(_partCtrls[i]);
    }
    _partCtrls.clear();
    _names.clear();
    _index.clear();
    _count = 0;
}

int ParticleCtrlStore::GetID(const s_string& name) const {
    const auto it = _index.find(ToLowerCopy(name));
    return it == _index.end() ? -1 : it->second;
}

CMPPartCtrl* ParticleCtrlStore::GetByID(int id) {
    if (id < 0 || id >= _count) {
        ToLogService("errors", LogLevel::Error,
                     "[ParticleCtrlStore] id out of range: {}", id);
        return nullptr;
    }
    if (!_partCtrls[id] && !_directory.empty()) {
        const std::string file = std::format("{}\\{}", _directory, _names[id]);
        LoadInto(id, file);
    }
    return _partCtrls[id];
}

int ParticleCtrlStore::Count() const noexcept {
    return _count;
}

CMPPartCtrl* ParticleCtrlStore::NewNamed(const s_string& name) {
    if (_count >= kCapacity) {
        return nullptr;
    }
    if (_partCtrls.empty()) {
        _partCtrls.assign(kCapacity, nullptr);
    }
    const int id = _count++;
    _names.push_back(name);
    _index.emplace(ToLowerCopy(name), id);

    auto* ctrl = new CMPPartCtrl;
    _partCtrls[id] = ctrl;
    return ctrl;
}

} // namespace Corsairs::Engine::Render
