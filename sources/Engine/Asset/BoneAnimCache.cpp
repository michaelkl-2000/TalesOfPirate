#include "BoneAnimCache.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <utility>

#include <windows.h>

#include "AssetLoaders.h"
#include "lwExpObj.h"
#include "lwHeap.h"

namespace Corsairs::Engine::Render {

namespace {

std::string NormalizeKey(std::string_view path) {
    std::string out;
    out.reserve(path.size());
    for (char c : path) {
        if (c == '/') {
            c = '\\';
        }
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

} // namespace

BoneAnimCache& BoneAnimCache::Instance() {
    static BoneAnimCache instance;
    return instance;
}

BoneAnimCache::~BoneAnimCache() {
    for (auto& [_, p] : _cache) {
        if (p) {
            p->Release();
        }
    }
}

Corsairs::Engine::Render::lwIAnimDataBone* BoneAnimCache::GetOrLoad(std::string_view fullPath) {
    std::string key = NormalizeKey(fullPath);

    {
        std::shared_lock lock(_mutex);
        if (auto it = _cache.find(key); it != _cache.end()) {
            return it->second;
        }
    }

    // Парсинг — вне lock'а (I/O).
    Corsairs::Engine::Render::lwAnimDataBone* data = LW_NEW(Corsairs::Engine::Render::lwAnimDataBone);
    if (LW_RESULT r = LgoLoader::LoadAnimDataBone(*data, fullPath); LW_FAILED(r)) {
        data->Release();
        return nullptr;
    }

    Corsairs::Engine::Render::lwIAnimDataBone* iface = data;  // upcast

    std::unique_lock lock(_mutex);
    auto [it, inserted] = _cache.try_emplace(std::move(key), iface);
    if (!inserted) {
        // Гонка: параллельный вызов уже вставил запись — отпускаем свежий
        // экземпляр, отдаём кешированный.
        iface->Release();
    }
    return it->second;
}

void BoneAnimCache::WarmUp() {
    const std::string pattern = std::format("{}*.lab", DirectoryPrefix());

    WIN32_FIND_DATA fd{};
    HANDLE h = FindFirstFile(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        const std::string_view name{fd.cFileName};
        if (name.size() < 4) {
            continue;
        }
        if (_stricmp(name.data() + name.size() - 4, ".lab") != 0) {
            continue;
        }
        const std::string full = std::format("{}{}", DirectoryPrefix(), name);
        GetOrLoad(full);
    }
    while (FindNextFile(h, &fd));
    FindClose(h);
}

void BoneAnimCache::Drop(std::string_view fullPath) {
    const std::string key = NormalizeKey(fullPath);
    std::unique_lock lock(_mutex);
    if (auto it = _cache.find(key); it != _cache.end()) {
        if (it->second) {
            it->second->Release();
        }
        _cache.erase(it);
    }
}

void BoneAnimCache::Clear() {
    std::unique_lock lock(_mutex);
    for (auto& [_, p] : _cache) {
        if (p) {
            p->Release();
        }
    }
    _cache.clear();
}

std::size_t BoneAnimCache::Size() const noexcept {
    std::shared_lock lock(_mutex);
    return _cache.size();
}

} // namespace Corsairs::Engine::Render
