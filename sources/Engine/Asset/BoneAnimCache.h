#pragma once

#include <cstdint>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "lwClassDecl.h"

namespace Corsairs::Engine::Render {
    class lwIAnimDataBone;
}

namespace Corsairs::Engine::Render {

// Strong-cache распарсенных .lab (bone-анимаций). Симметричен GeomObjCache,
// но владение через ref-count (Release/AddRef), а не shared_ptr, потому что
// lwIAnimDataBone — COM-style объект с собственным счётчиком.
//
// Ключ — нормализованный полный путь (lowercase, '/'→'\\'). Default-каталог:
// `animation\`.
class BoneAnimCache {
public:
    static BoneAnimCache& Instance();

    BoneAnimCache(const BoneAnimCache&) = delete;
    BoneAnimCache& operator=(const BoneAnimCache&) = delete;
    BoneAnimCache(BoneAnimCache&&) = delete;
    BoneAnimCache& operator=(BoneAnimCache&&) = delete;

    // Возвращает borrowed-указатель; cache держит 1 ref. Указатель валиден,
    // пока запись не уйдёт через Drop/Clear/shutdown.
    [[nodiscard]] Corsairs::Engine::Render::lwIAnimDataBone* GetOrLoad(std::string_view fullPath);

    void WarmUp();

    void Drop(std::string_view fullPath);
    void Clear();

    std::size_t Size() const noexcept;

    static constexpr std::string_view DirectoryPrefix() noexcept {
        return "animation\\";
    }

private:
    BoneAnimCache() = default;
    ~BoneAnimCache();

    mutable std::shared_mutex _mutex;
    std::unordered_map<std::string, Corsairs::Engine::Render::lwIAnimDataBone*> _cache;
};

} // namespace Corsairs::Engine::Render
