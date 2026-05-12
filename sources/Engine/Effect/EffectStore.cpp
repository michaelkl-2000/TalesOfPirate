#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectStore.h"

#include "AssetLoaders.h"
#include "EffectFile.h"
#include "I_Effect.h"
#include "MPRender.h"
#include "mpresmanger.h"

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

bool EndsWithIgnoreCase(std::string_view str, std::string_view suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    const std::string tail{str.substr(str.size() - suffix.size())};
    return _stricmp(tail.c_str(), std::string{suffix}.c_str()) == 0;
}

} // namespace

EffectStore& EffectStore::Instance() {
    static EffectStore instance;
    return instance;
}

void EffectStore::SetDevice(MPRender* dev) noexcept {
    _dev = dev;
}

void EffectStore::SetResMgr(CMPResManger* mgr) noexcept {
    _resMgr = mgr;
}

void EffectStore::SetBillboardMatrix(D3DXMATRIX* mat) noexcept {
    _billboardMat = mat;
}

bool EffectStore::LoadAllFrom(std::string_view directory) {
    _directory.assign(directory);

    const std::string pattern = std::format("{}\\*.eff", _directory);
    WIN32_FIND_DATA findData{};
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        const std::string_view nameView{findData.cFileName};
        if (!EndsWithIgnoreCase(nameView, ".eff")) {
            continue;
        }

        const int id = static_cast<int>(_effects.size());
        _effects.emplace_back();
        _params.emplace_back();
        _names.push_back(findData.cFileName);

        const std::string file = std::format("{}\\{}", _directory, findData.cFileName);
        if (!LoadInto(id, file)) {
            const std::string msg = std::format("({})", file);
            MessageBox(nullptr, msg.c_str(), "Error", MB_OK);
        }

        _index.emplace(ToLowerCopy(findData.cFileName), id);
        if (!_effects[id].empty()) {
            _effects[id][0].setEffectName(_names[id]);
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
    return true;
}

void EffectStore::Clear() {
    _effects.clear();
    _params.clear();
    _names.clear();
    _index.clear();
}

int EffectStore::GetID(std::string_view name) const {
    const auto it = _index.find(ToLowerCopy(name));
    return it == _index.end() ? -1 : it->second;
}

std::vector<I_Effect>& EffectStore::GetByID(int id) {
    if (id < 0 || static_cast<std::size_t>(id) >= _effects.size()) {
        return _empty;
    }
    auto& list = _effects[id];
    if (list.empty() && !_directory.empty()) {
        const std::string file = std::format("{}\\{}", _directory, _names[id]);
        if (LoadInto(id, file) && !list.empty()) {
            list[0].setEffectName(_names[id]);
        }
    }
    return list;
}

I_Effect* EffectStore::GetSubByID(int id, int subIdx) {
    auto& list = GetByID(id);
    if (subIdx < 0 || static_cast<std::size_t>(subIdx) >= list.size()) {
        return nullptr;
    }
    return &list[subIdx];
}

EffParameter* EffectStore::GetParamByID(int id) {
    if (id < 0 || static_cast<std::size_t>(id) >= _params.size()) {
        return nullptr;
    }
    return &_params[id];
}

int EffectStore::GetSubCount(int id) const {
    if (id < 0 || static_cast<std::size_t>(id) >= _effects.size()) {
        return 0;
    }
    return static_cast<int>(_effects[id].size());
}

int EffectStore::Count() const noexcept {
    return static_cast<int>(_effects.size());
}

VEC_string& EffectStore::GetAllNames() noexcept {
    return _names;
}

I_Effect* EffectStore::AddNamed(const s_string& name) {
    const int id = static_cast<int>(_effects.size());

    _effects.emplace_back();
    _params.emplace_back();
    _names.push_back(name);
    _index.emplace(ToLowerCopy(name), id);

    _effects[id].resize(1);
    _effects[id][0].ReleaseAll();
    return &_effects[id][0];
}

void EffectStore::AddUnited(std::vector<I_Effect>& effects) {
    const int id = static_cast<int>(_effects.size());
    const s_string name = !effects.empty() ? effects[0].getEffectName() : s_string{};

    _effects.emplace_back(effects);
    _names.push_back(name);
    _index.emplace(ToLowerCopy(name), id);
    // _params для legacy-симметрии (resize в AddUniteEffectToMgr был только по
    // _vecEffectList/_vecEffectName, _vecEffectParam НЕ ресайзился — повторяю поведение).

    if (_resMgr) {
        for (auto& effect : _effects[id]) {
            effect.BoundingRes(_resMgr);
        }
    }
    ApplyBillboardTo(_effects[id]);
}

bool EffectStore::LoadInto(int id, std::string_view file) {
    EffectFileInfo info;
    if (LW_FAILED(EffectLoader::Load(info, file))) {
        return false;
    }

    _params[id]  = std::move(info.param);
    _effects[id] = std::move(info.effects);

    for (auto& effect : _effects[id]) {
        effect.Reset();
        effect._dev = _dev;
    }
    ApplyBillboardTo(_effects[id]);
    return true;
}

void EffectStore::ApplyBillboardTo(std::vector<I_Effect>& effects) const {
    if (!_billboardMat) {
        return;
    }
    for (auto& effect : effects) {
        if (effect.IsBillBoard()) {
            effect.setBillBoardMatrix(_billboardMat);
        }
    }
}

} // namespace Corsairs::Engine::Render
