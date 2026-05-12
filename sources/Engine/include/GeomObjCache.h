#pragma once

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "lwClassDecl.h"

namespace LW_NAMESPACE {
    struct lwGeomObjInfo;
}

namespace Corsairs::Engine::Render {

// Категории нужны только для удобных операций WarmUp/Drop по каталогу.
// Ключом кеша служит нормализованный полный путь — это снимает коллизии
// одинаковых имён в model\character\ ↔ model\item\ ↔ model\effect\.
enum class GeomCategory : std::uint8_t {
    Character,
    Item,
    Effect,
};

// Единый strong-cache распарсенных .lgo-ассетов. Стратегия — load-once и
// держать запись до Drop()/Clear()/завершения процесса; повторный
// GetOrLoad по тому же ключу не парсит файл заново. lwGeomObjInfo
// трактуется как mostly-immutable: legacy-код (lwPrimitive::Load,
// lwPhysique::LoadPrimitive) допускает идемпотентные записи в info,
// которые уже работают корректно при многократном использовании одного
// character-mesh — этот инвариант сохраняется.
class GeomObjCache {
public:
    static GeomObjCache& Instance();

    GeomObjCache(const GeomObjCache&) = delete;
    GeomObjCache& operator=(const GeomObjCache&) = delete;
    GeomObjCache(GeomObjCache&&) = delete;
    GeomObjCache& operator=(GeomObjCache&&) = delete;

    // Загрузить .lgo по полному пути или вернуть существующую запись.
    // Ключ — lowercased полный путь с нормализованными разделителями ('\\').
    [[nodiscard]] std::shared_ptr<LW_NAMESPACE::lwGeomObjInfo> GetOrLoad(std::string_view fullPath);

    // Pre-scan каталога категории (FindFirstFile <prefix>*.lgo).
    void WarmUp(GeomCategory cat);

    // Снять одну запись. Существующие shared_ptr-владельцы info переживут drop.
    void Drop(std::string_view fullPath);

    // Снять все записи, чей ключ начинается с префикса категории.
    void Drop(GeomCategory cat);

    // Снять все записи.
    void Clear();

    std::size_t Size() const noexcept;

    // Префикс каталога категории ("model\\character\\" и т.п.).
    static std::string_view CategoryPrefix(GeomCategory cat) noexcept;

private:
    GeomObjCache() = default;
    ~GeomObjCache() = default;

    using Entry = std::shared_ptr<LW_NAMESPACE::lwGeomObjInfo>;

    mutable std::shared_mutex _mutex;
    std::unordered_map<std::string, Entry> _cache;
};

} // namespace Corsairs::Engine::Render
