#pragma once

#include "MindPower.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class CMPPartCtrl;

namespace Corsairs::Engine::Render {

// Хранилище CMPPartCtrl (.par) под Corsairs::Engine::Render.
// Capacity = 1500 (MAXPART_COUNT). LoadAllFrom грузит все *.par из заданной
// директории; имена и слоты пред-аллокированы. GetByID(id) умеет в lazy
// reload, если первая загрузка не удалась (slot остался nullptr).
//
// Контекст (effect-path) инжектится из CMPResManger::InitRes ДО LoadAllFrom —
// нужен для lazy reload по имени.
//
// ID-семантика стабильна: порядок FindFirstFile (сериализуется в .eff/
// runtime, не сохранять).
class ParticleCtrlStore {
public:
    static constexpr int kCapacity = 1500;

    static ParticleCtrlStore& Instance();

    void SetEffectPath(std::string_view directory);

    bool LoadAllFrom(std::string_view directory);
    void Clear();

    [[nodiscard]] int          GetID(const s_string& name) const;
    [[nodiscard]] CMPPartCtrl* GetByID(int id);
    [[nodiscard]] int          Count() const noexcept;

    // Создаёт пустой CMPPartCtrl и регистрирует имя — внешний код заполняет
    // содержимое сам (см. legacy NewPartCtrl).
    CMPPartCtrl* NewNamed(const s_string& name);

private:
    ParticleCtrlStore() = default;
    ~ParticleCtrlStore();
    ParticleCtrlStore(const ParticleCtrlStore&)            = delete;
    ParticleCtrlStore& operator=(const ParticleCtrlStore&) = delete;

    bool LoadInto(int id, std::string_view file);

    std::string                          _directory;
    std::vector<CMPPartCtrl*>            _partCtrls;   // capacity kCapacity
    VEC_string                           _names;
    std::unordered_map<std::string, int> _index;       // ключ — lowercase
    int                                  _count{0};
};

} // namespace Corsairs::Engine::Render
