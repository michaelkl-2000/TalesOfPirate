#include "GeomObjCache.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>
#include <utility>

#include <windows.h>

#include "AssetLoaders.h"
#include "lwExpObj.h"

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

std::string_view GeomObjCache::CategoryPrefix(GeomCategory cat) noexcept {
    switch (cat) {
        case GeomCategory::Character: return "model\\character\\";
        case GeomCategory::Item:      return "model\\item\\";
        case GeomCategory::Effect:    return "model\\effect\\";
    }
    return {};
}

GeomObjCache& GeomObjCache::Instance() {
    static GeomObjCache instance;
    return instance;
}

std::shared_ptr<Corsairs::Engine::Render::lwGeomObjInfo> GeomObjCache::GetOrLoad(std::string_view fullPath) {
    std::string key = NormalizeKey(fullPath);

    {
        std::shared_lock lock(_mutex);
        if (auto it = _cache.find(key); it != _cache.end()) {
            return it->second;
        }
    }

    // Парсинг — вне lock'а, чтобы не блокировать читателей на диск-I/O.
    Corsairs::Engine::Render::lwGeomObjInfo* raw = LgoLoader::Load(fullPath);
    if (raw == nullptr) {
        return nullptr;
    }
    LgoLoader::ApplyRuntimeDefaults(raw);

    Entry entry(raw);

    std::unique_lock lock(_mutex);
    auto [it, inserted] = _cache.try_emplace(std::move(key), std::move(entry));
    // Если параллельный вызов уже вставил запись — наш entry уйдёт в release
    // (raw удалится через shared_ptr-deleter), возвращаем кешированную.
    return it->second;
}

void GeomObjCache::WarmUp(GeomCategory cat) {
    const std::string_view prefix = CategoryPrefix(cat);
    if (prefix.empty()) {
        return;
    }
    const std::string pattern = std::format("{}*.lgo", prefix);

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
        if (_stricmp(name.data() + name.size() - 4, ".lgo") != 0) {
            continue;
        }
        const std::string full = std::format("{}{}", prefix, name);
        GetOrLoad(full);
    }
    while (FindNextFile(h, &fd));
    FindClose(h);
}

void GeomObjCache::Drop(std::string_view fullPath) {
    const std::string key = NormalizeKey(fullPath);
    std::unique_lock lock(_mutex);
    _cache.erase(key);
}

void GeomObjCache::Drop(GeomCategory cat) {
    const std::string prefix = NormalizeKey(CategoryPrefix(cat));
    if (prefix.empty()) {
        return;
    }
    std::unique_lock lock(_mutex);
    std::erase_if(_cache, [&](const auto& kv) {
        return kv.first.compare(0, prefix.size(), prefix) == 0;
    });
}

void GeomObjCache::Clear() {
    std::unique_lock lock(_mutex);
    _cache.clear();
}

std::size_t GeomObjCache::Size() const noexcept {
    std::shared_lock lock(_mutex);
    return _cache.size();
}

} // namespace Corsairs::Engine::Render
