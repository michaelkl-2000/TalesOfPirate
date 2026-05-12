#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class CEffPath;

namespace Corsairs::Engine::Render {

// Хранилище .csf-путей (CEffPath) под Corsairs::Engine::Render.
// Имена хранятся в исходном виде (для лога / отладки); индекс по
// lowercase-ключу — case-insensitive O(1) lookup. Порядок добавления
// (= порядок FindFirstFile) сериализуется в .par через ID, поэтому
// его нельзя менять.
class EffPathStore {
public:
    static EffPathStore& Instance();

    // Загружает все *.csf из директории directory. До вызова Clear() —
    // не загружает повторно (вызов из CMPResManger::InitRes одноразовый).
    bool LoadAllFrom(std::string_view directory);
    void Clear();

    [[nodiscard]] int       GetID(std::string_view name) const;
    [[nodiscard]] CEffPath* GetByID(int id);
    [[nodiscard]] int       Count() const noexcept;

private:
    EffPathStore() = default;
    EffPathStore(const EffPathStore&)            = delete;
    EffPathStore& operator=(const EffPathStore&) = delete;

    std::vector<std::string>             _names;
    std::vector<CEffPath>                _paths;
    std::unordered_map<std::string, int> _index;  // ключ — lowercase(name)
};

} // namespace Corsairs::Engine::Render
