#pragma once

#include "MindPower.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class CEffectModel;
class MPRender;

LW_BEGIN
class lwISystem;
class lwISysGraphics;
LW_END

namespace Corsairs::Engine::Render {

// Хранилище CEffectModel под Corsairs::Engine::Render.
// Содержит 7 сгенерированных primitive (Triangle/Rect/RectPlane/TrianglePlane/
// RectZ/Cone/Cylinder, ID 0..6) + загруженные из model/effect/*.lgo (ID 7..N) +
// shade-mesh отдельным слотом. Capacity = 800 (MAXMESH_COUNT): свободные слоты
// от _count до capacity используются для copy-on-use в GetByID при IsUsing().
//
// Контекст (device/system/sysGraphics) инжектится из CMPResManger::InitRes до
// LoadAllFrom — InitDevice применяется ко всем созданным CEffectModel:
//   * MPRender* dev — для InitDevice.
//   * lwISystem* sys — для смены PATH_TYPE_MODEL_ITEM при LoadModel(.lgo).
//   * lwISysGraphics* sysGraphics — для GetResourceMgr() в InitDevice primitive'ов.
//
// ID-семантика стабильна: 0..6 — primitive в фиксированном порядке, 7+ —
// порядок FindFirstFile в model/effect/*.lgo (сериализуется в .par/.eff —
// менять нельзя).
class EffectMeshStore {
public:
    static constexpr int kCapacity = 800;

    static EffectMeshStore& Instance();

    void SetDevice(MPRender* dev) noexcept;
    void SetSystem(LW_NAMESPACE::lwISystem* sys,
                   LW_NAMESPACE::lwISysGraphics* sysGraphics) noexcept;

    // Создаёт 7 primitive + shade-mesh и грузит .lgo из directory.
    // Вторая фаза reset device — см. RecreateBuiltins().
    bool LoadAllFrom(std::string_view directory);

    // Пере-создаёт 7 primitive после OnResetDevice (vertex/index buffer'ы).
    void RecreateBuiltins();

    void Clear();

    [[nodiscard]] int           GetID(const s_string& name) const;
    [[nodiscard]] CEffectModel* GetByID(int id);
    [[nodiscard]] CEffectModel* GetByName(const s_string& name);
    [[nodiscard]] CEffectModel* GetShadeMesh() const noexcept;

    // Снимает SetUsing с динамических слотов (ID >= 7). Primitive не трогаются.
    void DeleteMesh(CEffectModel& model);

    [[nodiscard]] int          Count() const noexcept;
    [[nodiscard]] VEC_string&  GetAllNames() noexcept;

private:
    EffectMeshStore() = default;
    ~EffectMeshStore();
    EffectMeshStore(const EffectMeshStore&)            = delete;
    EffectMeshStore& operator=(const EffectMeshStore&) = delete;

    void CreateBuiltins();
    bool LoadLgoFrom(std::string_view directory);

    MPRender*                       _dev{nullptr};
    LW_NAMESPACE::lwISystem*        _sys{nullptr};
    LW_NAMESPACE::lwISysGraphics*   _sysGraphics{nullptr};

    std::vector<CEffectModel*>           _meshes;   // capacity kCapacity
    VEC_string                           _names;    // только заполненные [0.._count)
    std::unordered_map<std::string, int> _index;    // ключ — lowercase
    int                                  _count{0};

    CEffectModel*                        _shadeModel{nullptr};
};

} // namespace Corsairs::Engine::Render
