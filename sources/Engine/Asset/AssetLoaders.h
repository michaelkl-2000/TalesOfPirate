#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>

#include "lwClassDecl.h"

// AssetLoaders.h — единственный API чтения/записи .lgo и .lmo. Пользуется
// типами lwGeomObjInfo, lwModelObjInfo, lwModelInfo и т.д. — потому
// инклюдим полный заголовок lwExpObj.h (в т.ч. ради внутреннего типа
// lwModelObjInfo::lwModelObjInfoHeader).
#include "lwExpObj.h"

namespace Corsairs::Engine::Render {

// Причина отказа `LgoLoader::LoadEx`. `Ok` при успешной загрузке; всё остальное —
// конкретный вид поломки, который тулза/тест может отобразить пользователю.
enum class LgoLoadStatus : std::uint32_t {
    Ok = 0,
    OkWithTrailingData,    // success: info валиден, но в файле остался хвост, который наш парсер не читает
    FileOpenFailed,        // fopen(file) не удался (нет файла, прав и т.п.)
    VersionTruncated,      // меньше 4 байт в файле — DWORD version не прочитан
    VersionUnknown,        // version не из {0x0000, 0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005}
    HeaderTruncated,       // не удалось прочитать lwGeomObjInfoHeader (116 байт)
    BlockSizesInconsistent,// header.{mtl,mesh,helper,anim}_size + 120 > file_size — заявлено больше чем есть
    ParseFailed,           // LoadFromStream вернул LW_RET_FAILED (mtl_size > 100000 и т.п.)
    UnconsumedTrailingData // после LoadFromStream остались непрочитанные байты файла (legacy enum, теперь не используется как fail)
};

// Диагностический отчёт `LoadEx`. После успеха `status==Ok`, `version` заполнен;
// после провала — `status` ≠ Ok, `detail` содержит человекочитаемый текст.
struct LgoLoadDiagnostics {
    LgoLoadStatus status{LgoLoadStatus::Ok};
    std::string detail;
    std::uint32_t version{0};
};

// Все алгоритмы сериализации .lgo и .lmo живут здесь. Никаких свободных функций
// рядом с классом нет — всё, что использует только сам loader, — приватный
// статический член. Data-структуры (lwGeomObjInfo, lwMeshInfo, lwAnimDataInfo,
// HelperInfo, lwModelInfo, lwModelNodeInfo, HelperDummyObjInfo) обязаны
// оставаться без I/O-методов; для доступа к приватным полям сами data-классы
// объявляют LgoLoader другом (см. `friend class Corsairs::Engine::Render::LgoLoader;`
// в lwExpObj.h).
class LgoLoader {
public:
    // -----------------------------------------------------------------------
    // Константы и enum'ы
    // -----------------------------------------------------------------------

    // Sanity-лимит на одну аллокацию массива при чтении .lgo. Реальные ассеты
    // упираются в единицы мегабайт (самый большой `mesh_size` в датасете
    // ~4.5 МБ). Любое требование сверх этого порога — почти всегда битое поле
    // `*_num` в файле, и LW_NEW попытается выделить мусорный объём.
    static constexpr std::size_t kMaxArrayBytes = 32u * 1024u * 1024u;

    // -----------------------------------------------------------------------
    // .lgo (одиночный lwGeomObjInfo)
    // -----------------------------------------------------------------------

    // Старая совместимая обёртка: возвращает nullptr при любой ошибке без объяснений.
    [[nodiscard]] static Corsairs::Engine::Render::lwGeomObjInfo* Load(std::string_view file);

    // Расширенная загрузка с диагностикой — для тулзы и тестов. При неуспехе
    // возвращает nullptr и заполняет `diag.status` + `diag.detail`.
    [[nodiscard]] static Corsairs::Engine::Render::lwGeomObjInfo* LoadEx(std::string_view file,
                                                              LgoLoadDiagnostics& diag);

    static LW_RESULT Save(Corsairs::Engine::Render::lwGeomObjInfo* info, std::string_view file);

    static LW_RESULT LoadFromStream(Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveToStream(Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp);

    // -----------------------------------------------------------------------
    // Подсчёт размеров (используется и сторонним кодом — `lwPrimitive::ExtractMeshInfo`,
    // `LoadModelObj/SaveModelObj` ниже).
    // -----------------------------------------------------------------------

    static DWORD GetMtlTexInfoSize(const Corsairs::Engine::Render::lwGeomObjInfo* info);
    static DWORD GetMeshInfoSize(const Corsairs::Engine::Render::lwGeomObjInfo* info);
    static DWORD GetHelperInfoSize(const Corsairs::Engine::Render::HelperInfo& info);
    static DWORD GetAnimDataInfoSize(const Corsairs::Engine::Render::lwAnimDataInfo& info);

    // -----------------------------------------------------------------------
    // Сериализация helper-блока. Дёргается из LoadFromStream/SaveToStream и
    // из .lmo-сериализаторов ниже.
    // -----------------------------------------------------------------------

    static LW_RESULT LoadHelperInfo(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveHelperInfo(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);

    // -----------------------------------------------------------------------
    // FILE*-сериализация data-классов (раньше жили как невиртуальные методы
    // Load(FILE*, DWORD)/Save(FILE*) на самих классах). Перенесены сюда,
    // чтобы все алгоритмы чтения/записи .lgo были в одном месте.
    // -----------------------------------------------------------------------

    static LW_RESULT LoadAnimDataBone(Corsairs::Engine::Render::lwAnimDataBone& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataBone(const Corsairs::Engine::Render::lwAnimDataBone& info, std::FILE* fp);
    // Path-обёртки: открывают файл, читают/пишут DWORD-версию, делегируют в FILE*-вариант.
    // Раньше жили как виртуальные `lwIAnimDataBone::Load/Save(string_view)`. Перенесены сюда —
    // I/O `.lab`-файлов теперь полностью контролируется LgoLoader.
    static LW_RESULT LoadAnimDataBone(Corsairs::Engine::Render::lwAnimDataBone& info, std::string_view file);
    static LW_RESULT SaveAnimDataBone(const Corsairs::Engine::Render::lwAnimDataBone& info, std::string_view file);

    static LW_RESULT LoadAnimDataMatrix(Corsairs::Engine::Render::lwAnimDataMatrix& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataMatrix(const Corsairs::Engine::Render::lwAnimDataMatrix& info, std::FILE* fp);

    static LW_RESULT LoadAnimDataTexUV(Corsairs::Engine::Render::lwAnimDataTexUV& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataTexUV(const Corsairs::Engine::Render::lwAnimDataTexUV& info, std::FILE* fp);

    static LW_RESULT LoadAnimDataTexImg(Corsairs::Engine::Render::lwAnimDataTexImg& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataTexImg(const Corsairs::Engine::Render::lwAnimDataTexImg& info, std::FILE* fp);

    static LW_RESULT LoadAnimDataMtlOpacity(Corsairs::Engine::Render::lwAnimDataMtlOpacity& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataMtlOpacity(Corsairs::Engine::Render::lwAnimDataMtlOpacity& info, std::FILE* fp);

    // Сериализация одного lwMtlTexInfo (раньше — свободные функции
    // lwMtlTexInfo_Load/lwMtlTexInfo_Save в Corsairs::Engine::Render).
    static LW_RESULT LoadMtlTexInfoSingle(Corsairs::Engine::Render::lwMtlTexInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveMtlTexInfoSingle(const Corsairs::Engine::Render::lwMtlTexInfo& info, std::FILE* fp, DWORD version);

    // Сериализация lwAnimKeySetPRS (раньше — свободные функции
    // lwLoadAnimKeySetPRS/lwSaveAnimKeySetPRS в Corsairs::Engine::Render).
    static LW_RESULT LoadAnimKeySetPRS(Corsairs::Engine::Render::lwAnimKeySetPRS& info, std::FILE* fp);
    static LW_RESULT SaveAnimKeySetPRS(const Corsairs::Engine::Render::lwAnimKeySetPRS& info, std::FILE* fp);

    // -----------------------------------------------------------------------
    // .lmo (lwModelObjInfo — array geom_obj + helper по offsets)
    // -----------------------------------------------------------------------

    static LW_RESULT LoadModelObj(Corsairs::Engine::Render::lwModelObjInfo& info, std::string_view file);
    static LW_RESULT SaveModelObj(Corsairs::Engine::Render::lwModelObjInfo& info, std::string_view file);
    static DWORD     GetModelObjSize(const Corsairs::Engine::Render::lwModelObjInfo& info);
    static LW_RESULT GetModelObjHeader(Corsairs::Engine::Render::lwModelObjInfo::lwModelObjInfoHeader* out_seq,
                                       DWORD* out_num,
                                       std::string_view file);

    // Расширенная диагностика для тулз и валидаторов: при неуспехе заполняет
    // `diag.status`+`diag.detail`. По смыслу аналогична `LoadEx` для .lgo.
    static LW_RESULT LoadModelObjEx(Corsairs::Engine::Render::lwModelObjInfo& info, std::string_view file,
                                    LgoLoadDiagnostics& diag);
    static LW_RESULT LoadAnimDataBoneEx(Corsairs::Engine::Render::lwAnimDataBone& info, std::string_view file,
                                        LgoLoadDiagnostics& diag);

    // -----------------------------------------------------------------------
    // Tree-based .lmo / .lxo (lwModelInfo с lwITreeNode-деревом lwModelNodeInfo)
    // -----------------------------------------------------------------------

    static LW_RESULT LoadModel(Corsairs::Engine::Render::lwModelInfo& info, std::string_view file);
    static LW_RESULT SaveModel(Corsairs::Engine::Render::lwModelInfo& info, std::string_view file);

    // Расширенная диагностика для tree-based .lxo (используется PkoTool/тулзами).
    // На успех — `diag.status=Ok`, `diag.version=info._head.version`. На неуспех —
    // конкретный `LgoLoadStatus` + текстовый detail.
    static LW_RESULT LoadModelEx(Corsairs::Engine::Render::lwModelInfo& info, std::string_view file,
                                 LgoLoadDiagnostics& diag);

    // -----------------------------------------------------------------------
    // Прочее
    // -----------------------------------------------------------------------

    // Применить рантайм-инварианты к свежезагруженному info: STATE_FRAMECULLING := 0,
    // STATE_UPDATETRANSPSTATE := 1. В файлах эти поля хранятся как «после экспорта»
    // (обычно 0/0), но рантайм требует другие значения, чтобы primitive после Load
    // корректно пересчитал прозрачность и не отсекался frustum-culling'ом. В Load/
    // LoadFromStream этого делать НЕЛЬЗЯ, иначе ломается round-trip Load→Save.
    static void ApplyRuntimeDefaults(Corsairs::Engine::Render::lwGeomObjInfo* info);

    // Безопасная замена `LW_NEW(T[N])` для loader/copy-кода .lgo: тот же
    // `new T[N]`, но при превышении `kMaxArrayBytes` логирует контекст и
    // бросает `std::length_error`, чтобы битое поле `*_num` не превратилось
    // в десятки гигабайт `new T[N]` и в последующий `fread` поверх кучи.
    // Бросок одинаков в Debug и Release.
    template <typename T>
    [[nodiscard]] static T* CheckedNewArray(std::size_t count, const char* typeName) {
        const std::size_t bytes = count * sizeof(T);
        if (bytes > kMaxArrayBytes) {
            ThrowSuspiciousAlloc(typeName, count, sizeof(T), bytes);
        }
        return new T[count];
    }

private:
    // -----------------------------------------------------------------------
    // Приватные подблоки .lgo. Раньше жили как свободные функции (`lwLoadMtlTexInfo`,
    // `lwMeshInfo_Load`) или методы data-классов (`lwAnimDataInfo::Load`/`Save`).
    // -----------------------------------------------------------------------

    static LW_RESULT LoadMtlTexInfo(Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveMtlTexInfo(const Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp);
    static LW_RESULT LoadMeshInfo(Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveMeshInfo(const Corsairs::Engine::Render::lwGeomObjInfo* info, std::FILE* fp);
    static LW_RESULT LoadAnimDataInfo(Corsairs::Engine::Render::lwAnimDataInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveAnimDataInfo(Corsairs::Engine::Render::lwAnimDataInfo& info, std::FILE* fp);

    // 5+5 разделов HelperInfo (dummy/box/mesh/bbox/bsphere).
    static LW_RESULT LoadHelperDummySection(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT LoadHelperBoxSection(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT LoadHelperMeshSection(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT LoadBoundingBoxSection(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT LoadBoundingSphereSection(Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp, DWORD version);

    static LW_RESULT SaveHelperDummySection(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);
    static LW_RESULT SaveHelperBoxSection(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);
    static LW_RESULT SaveHelperMeshSection(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);
    static LW_RESULT SaveBoundingBoxSection(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);
    static LW_RESULT SaveBoundingSphereSection(const Corsairs::Engine::Render::HelperInfo& info, std::FILE* fp);

    // .lmo tree-узлы и dummy-helper-объект.
    static LW_RESULT LoadModelNode(Corsairs::Engine::Render::lwModelNodeInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveModelNode(Corsairs::Engine::Render::lwModelNodeInfo& info, std::FILE* fp);
    static LW_RESULT LoadHelperDummyObj(Corsairs::Engine::Render::HelperDummyObjInfo& info, std::FILE* fp, DWORD version);
    static LW_RESULT SaveHelperDummyObj(Corsairs::Engine::Render::HelperDummyObjInfo& info, std::FILE* fp);

    // Утилиты.
    [[nodiscard]] static bool          IsKnownVersion(DWORD v);
    [[nodiscard]] static std::int64_t  FileSize(std::FILE* fp);
    static void                        LogLoadFailure(std::string_view file, const LgoLoadDiagnostics& diag);

    [[noreturn]] static void ThrowSuspiciousAlloc(const char* typeName,
                                                  std::size_t count,
                                                  std::size_t elemSize,
                                                  std::size_t totalBytes);
};

[[nodiscard]] constexpr std::string_view ToString(LgoLoadStatus s) noexcept {
    switch (s) {
        case LgoLoadStatus::Ok:                     return "Ok";
        case LgoLoadStatus::OkWithTrailingData:     return "OkWithTrailingData";
        case LgoLoadStatus::FileOpenFailed:         return "FileOpenFailed";
        case LgoLoadStatus::VersionTruncated:       return "VersionTruncated";
        case LgoLoadStatus::VersionUnknown:         return "VersionUnknown";
        case LgoLoadStatus::HeaderTruncated:        return "HeaderTruncated";
        case LgoLoadStatus::BlockSizesInconsistent: return "BlockSizesInconsistent";
        case LgoLoadStatus::ParseFailed:            return "ParseFailed";
        case LgoLoadStatus::UnconsumedTrailingData: return "UnconsumedTrailingData";
    }
    return "?";
}

// Макрос для всех LGO_NEW_ARRAY(T, N) call-sites. Раскрывается в
// LgoLoader::CheckedNewArray<T>(N, "T"). Сделан макросом ради автоматического
// stringify-имени типа в логе.
#define LGO_NEW_ARRAY(type, count) (::Corsairs::Engine::Render::LgoLoader::CheckedNewArray<type>(static_cast<std::size_t>(count), #type))

// =============================================================================
// .eff — Effect-файл MindPower3D (header + массив I_Effect-элементов)
// =============================================================================

} // namespace Corsairs::Engine::Render

// Forward-decl в глобальном пространстве (полные определения в MPModelEff.h /
// I_Effect.h). EffectLoader работает по ссылке; клиенты, конструирующие
// EffectFileInfo или передающие I_Effect&, должны включить эти заголовки.
struct EffectFileInfo;
class  I_Effect;

namespace Corsairs::Engine::Render {

enum class EffectLoadStatus : std::uint32_t {
    Ok = 0,
    FileOpenFailed,        // fopen(file) не удался
    VersionTruncated,      // < 4 байт в файле — version DWORD не прочитан
    VersionUnknown,        // version вне поддерживаемого диапазона (1..7 на момент написания)
    HeaderTruncated,       // не дочитан header (idxTech / paths / rotation block)
    ParseFailed            // I_Effect::LoadFromFile вернул false
};

struct EffectLoadDiagnostics {
    EffectLoadStatus status{EffectLoadStatus::Ok};
    std::string detail;
    std::uint32_t version{0};
};

// Все алгоритмы сериализации .eff живут здесь. По смыслу — параллель LgoLoader,
// но для другого формата данных. Само сохранение использует существующие
// `I_Effect::Save/LoadFromFile`-методы поэлементно (миграция этих методов
// внутрь EffectLoader — отдельная задача; см. план в issue tracker).
class EffectLoader {
public:
    // Текущая on-disk version, которую пишет Save (latest format).
    static constexpr std::uint32_t kCurrentVersion = 7;

    [[nodiscard]] static LW_RESULT Load(::EffectFileInfo& info, std::string_view file);
    [[nodiscard]] static LW_RESULT LoadEx(::EffectFileInfo& info, std::string_view file,
                                          EffectLoadDiagnostics& diag);
    static LW_RESULT Save(::EffectFileInfo& info, std::string_view file);

    // YAML-сериализация .eff для round-trip-тестов и человекочитаемого
    // редактирования. Формат — индент-структурированный YAML с inline-массивами
    // и hex-float'ами (`{:a}`), чтобы текстовое представление давало побайтно
    // точный обратный binary при пересохранении. Парсер поддерживает только
    // подмножество YAML, нужное для собственного эмиттера. На неуспех — пишет
    // в errors-канал и возвращает LW_RET_FAILED. Реализация в EffectYaml.cpp.
    [[nodiscard]] static LW_RESULT ExportToYaml(const ::EffectFileInfo& info,
                                                std::string_view file);
    [[nodiscard]] static LW_RESULT ImportFromYaml(::EffectFileInfo& info,
                                                  std::string_view file);

private:
    // Сериализация одного элемента .eff. Раньше жили как методы I_Effect
    // (`SaveToFile`/`LoadFromFile`); по правилу проекта I/O в data-классах
    // запрещён, поэтому перенесены сюда. Вызываются только из
    // EffectLoader::Save/LoadEx (целый .eff = header + N элементов).
    [[nodiscard]] static bool LoadElement(::I_Effect& effect, std::FILE* fp, DWORD version);
    static bool SaveElement(::I_Effect& effect, std::FILE* fp);
};

[[nodiscard]] constexpr std::string_view ToString(EffectLoadStatus s) noexcept {
    switch (s) {
        case EffectLoadStatus::Ok:                return "Ok";
        case EffectLoadStatus::FileOpenFailed:    return "FileOpenFailed";
        case EffectLoadStatus::VersionTruncated:  return "VersionTruncated";
        case EffectLoadStatus::VersionUnknown:    return "VersionUnknown";
        case EffectLoadStatus::HeaderTruncated:   return "HeaderTruncated";
        case EffectLoadStatus::ParseFailed:       return "ParseFailed";
    }
    return "?";
}

// =============================================================================
// .par — Particle-controller файл (CMPPartCtrl: name, particle systems, strips, models)
// =============================================================================

} // namespace Corsairs::Engine::Render

class CMPPartCtrl;
class CMPPartSys;
class CMPStrip;
class CChaModel;
class CEffPath;

namespace Corsairs::Engine::Render {

enum class PartCtrlLoadStatus : std::uint32_t {
    Ok = 0,
    FileOpenFailed,        // fopen(file) не удался
    VersionTruncated,      // < 4 байт в файле — version DWORD не прочитан
    VersionUnknown,        // version вне поддерживаемого диапазона (2..ParVersion=15)
    ParseFailed            // CMPPartCtrl::LoadFromFile вернул false
};

struct PartCtrlLoadDiagnostics {
    PartCtrlLoadStatus status{PartCtrlLoadStatus::Ok};
    std::string detail;
    std::uint32_t version{0};
};

// Чтение и запись .par-файла (CMPPartCtrl + per-element CMPPartSys/CMPStrip/
// CChaModel). Все методы Load/Save data-классов перенесены сюда; loader имеет
// доступ к их полям через friend-объявление в каждом из них.
class PartCtrlLoader {
public:
    [[nodiscard]] static LW_RESULT Load(::CMPPartCtrl& ctrl, std::string_view file);
    [[nodiscard]] static LW_RESULT LoadEx(::CMPPartCtrl& ctrl, std::string_view file,
                                          PartCtrlLoadDiagnostics& diag);
    static LW_RESULT Save(::CMPPartCtrl& ctrl, std::string_view file);

    // YAML-сериализация .par для round-trip-тестов и человекочитаемого
    // редактирования. Реализация — PartCtrlYaml.cpp; общий YAML-движок —
    // YamlCommon.h. Round-trip побайтно точный после первого binary save'а.
    [[nodiscard]] static LW_RESULT ExportToYaml(const ::CMPPartCtrl& ctrl,
                                                std::string_view file);
    [[nodiscard]] static LW_RESULT ImportFromYaml(::CMPPartCtrl& ctrl,
                                                  std::string_view file);

    // Per-element сериализация — приватная (вызывается только из Load/Save
    // выше). public-API остаётся минимальным.
private:
    static bool LoadCtrl(::CMPPartCtrl& ctrl, std::string_view file);
    static bool SaveCtrl(::CMPPartCtrl& ctrl, std::string_view file);

    static bool LoadPartSys(::CMPPartSys& ps, std::FILE* fp, DWORD version);
    static bool SavePartSys(::CMPPartSys& ps, std::FILE* fp);

    static bool LoadStrip(::CMPStrip& s, std::FILE* fp, DWORD version);
    static bool SaveStrip(::CMPStrip& s, std::FILE* fp);

    static void LoadCharModel(::CChaModel& m, std::FILE* fp);
    static void SaveCharModel(::CChaModel& m, std::FILE* fp);
};

[[nodiscard]] constexpr std::string_view ToString(PartCtrlLoadStatus s) noexcept {
    switch (s) {
        case PartCtrlLoadStatus::Ok:                return "Ok";
        case PartCtrlLoadStatus::FileOpenFailed:    return "FileOpenFailed";
        case PartCtrlLoadStatus::VersionTruncated:  return "VersionTruncated";
        case PartCtrlLoadStatus::VersionUnknown:    return "VersionUnknown";
        case PartCtrlLoadStatus::ParseFailed:       return "ParseFailed";
    }
    return "?";
}

// =============================================================================
// Анимационные траектории для CEffPath — два соседних формата:
//   • .csf — собственный бинарь движка: header "csf\0", DWORD version,
//     DWORD num, далее num × D3DXVECTOR3 (right-handed, при чтении Y/Z
//     меняем местами с инверсией Z). Продакшн-формат, читается из
//     model/effect/*.csf при старте через CMPResManger::LoadTotalPath().
//   • .let — поверх анимационного matrix-track (EfxTrackLoader → EfxTrack).
//     Редакторский путь импорта пути из 3D-сцены; runtime его не зовёт.
// CEffPath — это keyframed translation для эффекта (до 200 точек): задаёт
// траекторию движения визуального эффекта/частиц во времени (например,
// летящий снаряд с кривой траекторией, орбитальные партиклы, шлейфы).
// =============================================================================

enum class EffPathLoadStatus : std::uint32_t {
    Ok = 0,
    FileOpenFailed,        // fopen(file) не удался
    HeaderTruncated,       // < 4 байт magic, либо < 8 байт version+num
    BadMagic,              // первые 3 байта ≠ "csf"
    FrameCountOutOfRange,  // num == 0 или num > 200 (_vecPath limit)
    BodyTruncated          // size файла < 4+8+num*12 — кадры не дочитываются
};

struct EffPathLoadDiagnostics {
    EffPathLoadStatus status{EffPathLoadStatus::Ok};
    std::string detail;
    std::uint32_t version{0};
    std::uint32_t frameCount{0};
};

class EffPathLoader {
public:
    static constexpr std::uint32_t kMaxFrames = 200;       // CEffPath::_vecPath[200]
    static constexpr std::uint32_t kCurrentVersion = 1;    // version, которую пишет Save

    // Прочитать .csf в готовый CEffPath. Поля _vecPath/_vecDir/_vecDist/
    // _iFrameCount наполняются как раньше делал CEffPath::LoadPathFromFile.
    [[nodiscard]] static LW_RESULT Load(::CEffPath& path, std::string_view file);
    [[nodiscard]] static LW_RESULT LoadEx(::CEffPath& path, std::string_view file,
                                          EffPathLoadDiagnostics& diag);

    // Прочитать .let-формат через EfxTrackLoader::Load и спроецировать
    // matrix-кадры в CEffPath::_vecPath.
    [[nodiscard]] static LW_RESULT LoadLet(::CEffPath& path, std::string_view file);

    // Записать CEffPath в .csf (header "csf\0" + version=1 + num + кадры с
    // обратной инверсией Y/Z для round-trip с Load). Симметрия с Load на
    // случай редакторских инструментов (PkoTool и т.п.) — runtime-клиент
    // .csf не пишет.
    [[nodiscard]] static LW_RESULT Save(const ::CEffPath& path, std::string_view file);

    // Поточные методы для in-place embedding CEffPath в .par (внутри
    // PartCtrlLoader::{Save,Load}PartSys, под флагом busepsth). Раньше
    // жили как CEffPath::SavePath/LoadPath, перенесены сюда по правилу
    // «I/O в data-классах запрещён».
    static void WritePath(const ::CEffPath& path, std::FILE* fp);
    static void ReadPath(::CEffPath& path, std::FILE* fp);
};

[[nodiscard]] constexpr std::string_view ToString(EffPathLoadStatus s) noexcept {
    switch (s) {
        case EffPathLoadStatus::Ok:                   return "Ok";
        case EffPathLoadStatus::FileOpenFailed:       return "FileOpenFailed";
        case EffPathLoadStatus::HeaderTruncated:      return "HeaderTruncated";
        case EffPathLoadStatus::BadMagic:             return "BadMagic";
        case EffPathLoadStatus::FrameCountOutOfRange: return "FrameCountOutOfRange";
        case EffPathLoadStatus::BodyTruncated:        return "BodyTruncated";
    }
    return "?";
}

// =============================================================================
// .map — MindPower3D terrain map (header + per-section offset table + tile data)
// =============================================================================

} // namespace Corsairs::Engine::Render

// MPMapFileHeader / SNewFileTile — 20- и 15-байтные C-структуры из
// Libraries/Util/include/MPMapDef.h. Нужны по значению внутри MapInfo, поэтому
// подключаем полное определение.
#include "MPMapDef.h"

#include <cstdio>
#include <vector>

// MPTile живёт в глобальном namespace (Engine/include/MPTile.h). Полное
// определение тянет MPRender.h → DirectX, что не нужно для тулз вроде
// AssetLoaderTests/PkoTool, поэтому здесь — только forward-decl: в API
// MapLoader/MapStream MPTile фигурирует только через указатели.
struct MPTile;

// ZRBlockData (Engine/include/ZRBlock.h) — `short Region + uint8_t Block[4]`,
// тот «лёгкий» срез тайла, который нужен ZRBlock для коллизий/региональных
// атрибутов. Forward-decl, чтобы MapLoader::ReadSectionBlockData принимал
// его без втягивания ZRBlock.h в публичный заголовок.
class ZRBlockData;

namespace Corsairs::Engine::Render {

// Используем типы формата файла без квалификации (using-declaration,
// не using-directive — глобально namespace Corsairs::Util::Map не открываем).
using ::Corsairs::Util::Map::MPMapFileHeader;
using ::Corsairs::Util::Map::SNewFileTile;

enum class MapLoadStatus : std::uint32_t {
    Ok = 0,
    FileOpenFailed,         // fopen(file) не удался (нет файла, прав)
    HeaderTruncated,        // < 20 байт в файле — MPMapFileHeader не дочитан
    BadMagic,               // header.MapFlag не из {kMapFlagLegacy, kMapFlagCurrent}; чужой формат или MapTool-файл
    InconsistentDimensions, // SectionWidth/Height равны 0 либо не делят Width/Height нацело
    OffsetTableTruncated,   // не удалось прочитать uint32[sectionCount] таблицу оффсетов
    BodyTruncated,          // EOF до конца секции, на которую указывает один из offsets[i]
    UnknownVersion          // зарезервировано: header принят как валидный, но runtime его не поддерживает
};

struct MapLoadDiagnostics {
    MapLoadStatus status{MapLoadStatus::Ok};
    std::string detail;
    std::int32_t mapFlag{0};
};

// In-memory снимок одного .map-файла. Содержит ровно те же байты, что и файл
// на диске:
//   [MPMapFileHeader (20 байт): {MapFlag, Width, Height, SectionWidth, SectionHeight}]
//   [uint32[sectionCount] offsets — 0 значит «секция пустая, на диске её нет»]
//   [body: конкатенация tile-данных секций; конкретный кусок берётся по
//          offset[i] − (sizeof(header) + offsets.size()*4), вычитая префикс,
//          чтобы получить индекс внутри body.]
//
// MapLoader::Load читает все три блока «как есть» и не разбирает body на
// SNewFileTile-структуры — для round-trip-теста это не нужно, а «сырая»
// секция делает Save(Load(file)) побайтно детерминированным для любого
// валидного .map.
struct MapInfo {
    MPMapFileHeader header{};
    std::vector<std::uint32_t> offsets;
    std::vector<std::byte> body;
};

// MapStream — открытое состояние .map-файла, нужное MPMap'у для двух
// сценариев:
//  • runtime (edit=false): file всё равно открыт `rb` для совместимости с
//    `MPMap::FullLoading`/`DynamicLoading` (используют `IsOpen()` как «карта
//    загружена»), но всё содержимое body уже скопировано в `_bulkData`,
//    последующее чтение секций идёт по memcpy из памяти, не дисковыми
//    fread'ами.
//  • editor (edit=true): file открыт `r+b`. `_bulkData` пуст, чтение секции
//    делает `fseek+fread`, запись — `fseek+fwrite` плюс обновление on-disk
//    offset entry в tablice оффсетов.
//
// MapStream не копируется. RAII: деструктор закрывает FILE*; Close() можно
// вызвать вручную. Конструктор в private — собирается только через MapLoader.
//
// `MPTile` (struct в глобальном namespace, объявлен в `MPTile.h`) — точка
// сериализации. Forward-decl ниже, полное определение нужно лишь в
// MapLoader.cpp.
class MapStream {
public:
    MapStream() = default;
    ~MapStream();

    MapStream(const MapStream&) = delete;
    MapStream& operator=(const MapStream&) = delete;
    MapStream(MapStream&& other) noexcept;
    MapStream& operator=(MapStream&& other) noexcept;

    [[nodiscard]] bool IsOpen() const noexcept;
    [[nodiscard]] bool IsEdit() const noexcept { return _edit; }
    [[nodiscard]] const MPMapFileHeader& Header() const noexcept { return _header; }
    [[nodiscard]] std::int32_t SectionCountX() const noexcept { return _sectionCntX; }
    [[nodiscard]] std::int32_t SectionCountY() const noexcept { return _sectionCntY; }
    [[nodiscard]] std::int32_t SectionCount() const noexcept {
        return _sectionCntX * _sectionCntY;
    }
    [[nodiscard]] std::uint32_t SectionOffset(int sectionX, int sectionY) const noexcept;

    void Close() noexcept;

private:
    friend class MapLoader;

    std::FILE* _fp{nullptr};
    bool _edit{false};
    MPMapFileHeader _header{};
    std::int32_t _sectionCntX{0};
    std::int32_t _sectionCntY{0};
    std::vector<std::uint32_t> _offsets;

    // Только для non-edit: байты body (всё после offset-таблицы). В edit —
    // пуст, чтение идёт через _fp.
    std::vector<std::byte> _bulkData;
    // Смещение, по которому начинается body в файле; равно
    // sizeof(MPMapFileHeader) + offsets.size() * 4. Дублируется отдельным
    // полем, чтобы не вычислять при каждом Read.
    std::uint32_t _bulkBaseOffset{0};
};

// Текущий формат записи .map. См. Corsairs::Util::Map::kMapFlagCurrent.
class MapLoader {
public:
    // kMapFlagBase (780624) + 3. Совпадает с kMapFlagCurrent из MPMapDef.h —
    // именно эта версия пишется обратно в Save и принимается runtime'ом.
    static constexpr std::int32_t kCurrentMapFlag = ::Corsairs::Util::Map::kMapFlagCurrent;
    // kMapFlagBase + 2 (kMapFlagLegacy). Принимается на чтение, но в
    // round-trip-тестах считается legacy: Save всегда сохраняет в актуальной
    // версии, runtime читает оба значения.
    static constexpr std::int32_t kLegacyMapFlag = ::Corsairs::Util::Map::kMapFlagLegacy;

    // Снапшот-API (round-trip-тесты, тулзы).
    [[nodiscard]] static LW_RESULT Load(MapInfo& info, std::string_view file);
    [[nodiscard]] static LW_RESULT LoadEx(MapInfo& info, std::string_view file,
                                          MapLoadDiagnostics& diag);
    static LW_RESULT Save(const MapInfo& info, std::string_view file);

    // Stream-API (используется MPMap для runtime'а и редактора).
    //
    // OpenStream разбирает header + offsets и при edit=false читает body в
    // память. На неуспех возвращает LW_RET_FAILED, заполняет diag, stream
    // остаётся в default-состоянии (IsOpen()==false).
    [[nodiscard]] static LW_RESULT OpenStream(MapStream& stream, std::string_view file,
                                              bool edit, MapLoadDiagnostics& diag);

    // ReadSection декодирует одну секцию из stream'а в outTiles. Размер
    // outTiles — header.SectionWidth * header.SectionHeight (вызывающий
    // выделяет буфер). Возвращает LW_RET_FAILED при offset==0 (пустая секция;
    // вызывающий обязан проверить SectionOffset перед вызовом).
    [[nodiscard]] static LW_RESULT ReadSection(const MapStream& stream,
                                                int sectionX, int sectionY,
                                                ::MPTile* outTiles);

    // ReadSectionBlockData — облегчённый вариант ReadSection для ZRBlock'а.
    // Из каждого SNewFileTile берёт только `Region + Block[4]` (6 из 15 байт),
    // не выполняя LW_RGB565TODWORD/TileInfo_Unpack — block/region-часть от них
    // не зависит, и тянуть MPTile/lwgraphicsutil ради 6 байт на тайл незачем.
    // Размер outBlocks — header.SectionWidth*SectionHeight.
    [[nodiscard]] static LW_RESULT ReadSectionBlockData(
        const MapStream& stream, int sectionX, int sectionY,
        ::ZRBlockData* outBlocks);

    // WriteSection пишет тайлы в stream (только edit=true). Если offset для
    // данной (sx,sy) == 0, секция дописывается в конец файла, иначе пишется
    // на месте. Обновляет offset entry на диске и в stream._offsets.
    [[nodiscard]] static LW_RESULT WriteSection(MapStream& stream,
                                                 int sectionX, int sectionY,
                                                 const ::MPTile* tiles);

    // ClearSection обнуляет offset entry для секции (на диске и в кеше).
    // Тайл-данные физически в файле остаются — fseek/append-семантика
    // оригинального формата это допускает.
    [[nodiscard]] static LW_RESULT ClearSection(MapStream& stream,
                                                 int sectionX, int sectionY);
};

[[nodiscard]] constexpr std::string_view ToString(MapLoadStatus s) noexcept {
    switch (s) {
        case MapLoadStatus::Ok:                     return "Ok";
        case MapLoadStatus::FileOpenFailed:         return "FileOpenFailed";
        case MapLoadStatus::HeaderTruncated:        return "HeaderTruncated";
        case MapLoadStatus::BadMagic:               return "BadMagic";
        case MapLoadStatus::InconsistentDimensions: return "InconsistentDimensions";
        case MapLoadStatus::OffsetTableTruncated:   return "OffsetTableTruncated";
        case MapLoadStatus::BodyTruncated:          return "BodyTruncated";
        case MapLoadStatus::UnknownVersion:         return "UnknownVersion";
    }
    return "?";
}

// =============================================================================
// EfxTrackLoader — бинарный effect-track (lwAnimDataMatrix-payload без version-
// заголовка; формат у EfxTrack тривиален: просто SaveAnimDataMatrix → файл).
// =============================================================================

// Forward-decls для классов из Corsairs::Engine::Render — полные определения в
// EfxTrack.h / lwPoseCtrl.h / DDSFile.h. Подключение этих заголовков из
// AssetLoaders.h всё ещё избыточно для тулз (DDSFile тянет lwDirectX → DDK),
// потому ограничиваемся forward-decl и работаем через ссылки.

class EfxTrackLoader {
public:
    [[nodiscard]] static LW_RESULT Load(Corsairs::Engine::Render::EfxTrack& track, std::string_view file);
    [[nodiscard]] static LW_RESULT Save(const Corsairs::Engine::Render::EfxTrack& track, std::string_view file);
};

// =============================================================================
// PoseCtrlLoader — pose-controller-файл: [DWORD version=1][DWORD pose_num]
// [lwPoseInfo[pose_num]]. Раньше I/O жил методами на data-классе lwPoseCtrl.
// =============================================================================

class PoseCtrlLoader {
public:
    static constexpr std::uint32_t kCurrentVersion = 1;

    // Path-обёртки: открывают файл, валидируют version DWORD, делегируют в FILE*.
    [[nodiscard]] static LW_RESULT Load(Corsairs::Engine::Render::lwPoseCtrl& ctrl, std::string_view file);
    [[nodiscard]] static LW_RESULT Save(const Corsairs::Engine::Render::lwPoseCtrl& ctrl, std::string_view file);

    // FILE*-вариант: читает/пишет «тело» (pose_num + pose_seq[]) без version-
    // заголовка. Используется path-обёртками.
    [[nodiscard]] static LW_RESULT LoadBody(Corsairs::Engine::Render::lwPoseCtrl& ctrl, std::FILE* fp);
    [[nodiscard]] static LW_RESULT SaveBody(const Corsairs::Engine::Render::lwPoseCtrl& ctrl, std::FILE* fp);
};

// =============================================================================
// DdsLoader — запись .dds файла из DDSFile (origin или сжатой текстуры).
// Чтение .dds живёт отдельно (DDSFile::LoadOriginTexture использует
// D3DXCreateTextureFromFileEx, не наш fopen-путь).
// =============================================================================

class DdsLoader {
public:
    [[nodiscard]] static LW_RESULT Save(Corsairs::Engine::Render::DDSFile& dds, std::string_view file);

private:
    // Внутренние шаги — приватные static-методы DdsLoader (не free-функции),
    // чтобы единого friend-объявления `friend class DdsLoader;` в DDSFile
    // хватало для доступа к приватным полям (_tex_width, _mip_level и т.д.)
    // и приватным IsVolumeMap/IsCubeMap.
    [[nodiscard]] static long SaveDDSHeader(Corsairs::Engine::Render::DDSFile& dds,
                                             struct IDirect3DBaseTexture9* tex, std::FILE* fp);
    [[nodiscard]] static long SaveAllMipSurfaces(Corsairs::Engine::Render::DDSFile& dds,
                                                  struct IDirect3DBaseTexture9* ptex,
                                                  unsigned int faceType,
                                                  std::FILE* fp);
    [[nodiscard]] static long SaveAllVolumeSurfaces(Corsairs::Engine::Render::DDSFile& dds,
                                                     struct IDirect3DVolumeTexture9* pvoltex,
                                                     std::FILE* fp);
};

// =============================================================================
// ScreenshotSaver — дамп IDirect3DSurfaceX в файл. Формат — PNG (D3DX встроен
// поддерживает D3DXIFF_PNG; никаких stb_image_write не подключаем). Раньше
// MPRender::CaptureScreen писал .bmp вручную через std::ofstream — заменено
// на единый path через ScreenshotSaver.
// =============================================================================

class ScreenshotSaver {
public:
    // Сохранить surface в файл. Формат определяется по расширению; по умолчанию
    // (если расширение не .bmp/.dds/.jpg) — PNG.
    [[nodiscard]] static LW_RESULT SaveSurface(IDirect3DSurfaceX* surface, std::string_view file);
};

} // namespace Corsairs::Engine::Render
