#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffPathStore.h"

#include "MPModelEff.h"
#include "AssetLoaders.h"

#include <algorithm>
#include <cctype>
#include <format>

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

EffPathStore& EffPathStore::Instance() {
    static EffPathStore instance;
    return instance;
}

bool EffPathStore::LoadAllFrom(std::string_view directory) {
    const std::string pattern = std::format("{}\\*.csf", directory);

    WIN32_FIND_DATA findData{};
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        const std::string file = std::format("{}\\{}", directory, findData.cFileName);
        const int id = static_cast<int>(_paths.size());

        _names.emplace_back(findData.cFileName);
        _paths.emplace_back();
        _index.emplace(ToLowerCopy(findData.cFileName), id);

        EffPathLoader::Load(_paths.back(), file);
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
    return true;
}

void EffPathStore::Clear() {
    _names.clear();
    _paths.clear();
    _index.clear();
}

int EffPathStore::GetID(std::string_view name) const {
    const auto it = _index.find(ToLowerCopy(name));
    return it == _index.end() ? -1 : it->second;
}

CEffPath* EffPathStore::GetByID(int id) {
    if (id < 0 || static_cast<std::size_t>(id) >= _paths.size()) {
        return nullptr;
    }
    return &_paths[id];
}

int EffPathStore::Count() const noexcept {
    return static_cast<int>(_paths.size());
}

} // namespace Corsairs::Engine::Render
