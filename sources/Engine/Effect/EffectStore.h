#pragma once

#include "MindPower.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class I_Effect;
class EffParameter;
class CMPResManger;
class MPRender;

namespace Corsairs::Engine::Render {

// Хранилище загруженных .eff под Corsairs::Engine::Render.
// Имена хранятся в lowercase, lookup case-insensitive через unordered_map.
// Порядок ID = порядок FindFirstFile (сериализуется в .par/.eff — менять нельзя).
//
// Контекст (device/resMgr/billboardMat) инжектится из CMPResManger::InitRes
// ДО LoadAllFrom — стор применяет его автоматически ко всем свеже-загруженным
// I_Effect:
//   * Reset() + m_pDev = dev — пост-load fix-up (как раньше делал LoadEffectFromFile).
//   * setBillBoardMatrix(mat) — для эффектов с IsBillBoard() (раньше делал
//     ручной цикл в InitRes ДО загрузки эффектов; теперь применяется
//     при LoadInto/AddUnited).
//   * BoundingRes(resMgr) — для AddUniteEffectToMgr (нужен mgr-callback).
//
// Lazy-reload: GetByID(id) на пустом списке перечитывает .eff из _directory.
class EffectStore {
public:
    static EffectStore& Instance();

    void SetDevice(MPRender* dev) noexcept;
    void SetResMgr(CMPResManger* mgr) noexcept;
    void SetBillboardMatrix(D3DXMATRIX* mat) noexcept;

    bool LoadAllFrom(std::string_view directory);
    void Clear();

    [[nodiscard]] int                    GetID(std::string_view name) const;
    [[nodiscard]] std::vector<I_Effect>& GetByID(int id);
    [[nodiscard]] I_Effect*              GetSubByID(int id, int subIdx);
    [[nodiscard]] EffParameter*          GetParamByID(int id);
    [[nodiscard]] int                    GetSubCount(int id) const;
    [[nodiscard]] int                    Count() const noexcept;
    [[nodiscard]] VEC_string&            GetAllNames() noexcept;

    I_Effect* AddNamed(const s_string& name);
    void      AddUnited(std::vector<I_Effect>& effects);

private:
    EffectStore() = default;
    EffectStore(const EffectStore&)            = delete;
    EffectStore& operator=(const EffectStore&) = delete;

    bool LoadInto(int id, std::string_view file);
    void ApplyBillboardTo(std::vector<I_Effect>& effects) const;

    std::string                          _directory;
    MPRender*                            _dev{nullptr};
    CMPResManger*                        _resMgr{nullptr};
    D3DXMATRIX*                          _billboardMat{nullptr};

    VEC_string                           _names;     // оригинальные имена (как в FindFirstFile)
    std::vector<std::vector<I_Effect>>   _effects;
    std::vector<EffParameter>            _params;
    std::unordered_map<std::string, int> _index;     // ключ — lowercase

    std::vector<I_Effect>                _empty;     // возвращается при out-of-range
};

} // namespace Corsairs::Engine::Render
