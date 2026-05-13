// Реализация EffectLoader (.eff) и PartCtrlLoader (.par). Вынесено в отдельный
// TU из AssetLoaders.cpp специально: эти лоадеры тянут MPParticleCtrl/MPModelEff/
// I_Effect (которые транзитом тянут TextureManager + Cryptopp + Version.lib).
// Тулзы вроде PkoTool/AssetLoaderTests, которые работают только с LgoLoader,
// не дёргают EffectLoader — и линкер не пуллит эти .obj-ы из MindPower3D.lib,
// что избавляет от навешивания Cryptopp.lib/Version.lib в их .vcxproj.

#include "AssetLoaders.h"

#include "MPModelEff.h"      // EffectFileInfo, EffParameter, I_Effect, CEffPath
#include "MPParticleCtrl.h"  // CMPPartCtrl
#include "EfxTrack.h"      // EffPathLoader::LoadLet — matrix-track формат

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <format>
#include <memory>
#include <string>
#include <string_view>

namespace Corsairs::Engine::Render {

namespace {

// RAII для std::FILE*: тот же helper, что и в AssetLoaders.cpp, повторён локально
// (анонимный namespace в каждом TU независим).
struct FileCloser {
    void operator()(std::FILE* fp) const noexcept {
        if (fp != nullptr) {
            std::fclose(fp);
        }
    }
};
using UniqueFile = std::unique_ptr<std::FILE, FileCloser>;

// Записать строку в фиксированный 32-байтный буфер (формат .eff требует
// именно столько). Излишек обрезается, [31] всегда нулевой.
void WriteFixedName32(const std::string& src, std::FILE* fp) {
    char buf[32]{};
    std::memcpy(buf, src.data(),
                std::min<std::size_t>(src.size(), sizeof(buf) - 1));
    std::fwrite(buf, 1, sizeof(buf), fp);
}

constexpr std::uint32_t kMinEffectVersion = 1;
constexpr std::uint32_t kMaxEffectVersion = EffectLoader::kCurrentVersion; // = 7

} // namespace

// =============================================================================
// EffectLoader (.eff)
// =============================================================================

LW_RESULT EffectLoader::Load(::EffectFileInfo& info, std::string_view file) {
    EffectLoadDiagnostics diag;
    return LoadEx(info, file, diag);
}

LW_RESULT EffectLoader::LoadEx(::EffectFileInfo& info, std::string_view file,
                               EffectLoadDiagnostics& diag) {
    diag = {};
    info = ::EffectFileInfo{};

    UniqueFile fp{std::fopen(std::string{file}.c_str(), "rb")};
    if (!fp) {
        diag.status = EffectLoadStatus::FileOpenFailed;
        diag.detail = "fopen failed";
        return LW_RET_FAILED;
    }

    DWORD version = 0;
    if (std::fread(&version, sizeof(version), 1, fp.get()) != 1) {
        diag.status = EffectLoadStatus::VersionTruncated;
        diag.detail = "short read of version DWORD";
        return LW_RET_FAILED;
    }
    info.version = version;
    diag.version = version;

    if (version < kMinEffectVersion || version > kMaxEffectVersion) {
        diag.status = EffectLoadStatus::VersionUnknown;
        diag.detail = std::format("version=0x{:08X} (expected {}..{})",
                                  version, kMinEffectVersion, kMaxEffectVersion);
        return LW_RET_FAILED;
    }

    int idxTech = 0;
    if (std::fread(&idxTech, sizeof(int), 1, fp.get()) != 1) {
        diag.status = EffectLoadStatus::HeaderTruncated;
        diag.detail = "short read of idxTech";
        return LW_RET_FAILED;
    }
    info.param._idxTech = idxTech;

    char nameBuf[32]{};

    if (std::fread(&info.param._usePath, sizeof(bool), 1, fp.get()) != 1 ||
        std::fread(nameBuf, 1, sizeof(nameBuf), fp.get()) != sizeof(nameBuf)) {
        diag.status = EffectLoadStatus::HeaderTruncated;
        diag.detail = "short read of path block";
        return LW_RET_FAILED;
    }
    info.param._pathName = nameBuf;

    if (std::fread(&info.param._useSound, sizeof(bool), 1, fp.get()) != 1 ||
        std::fread(nameBuf, 1, sizeof(nameBuf), fp.get()) != sizeof(nameBuf)) {
        diag.status = EffectLoadStatus::HeaderTruncated;
        diag.detail = "short read of sound block";
        return LW_RET_FAILED;
    }
    info.param._soundName = nameBuf;

    if (std::fread(&info.param._rotating, sizeof(bool), 1, fp.get()) != 1 ||
        std::fread(&info.param._verRota, sizeof(D3DXVECTOR3), 1, fp.get()) != 1 ||
        std::fread(&info.param._rotaVel, sizeof(float), 1, fp.get()) != 1) {
        diag.status = EffectLoadStatus::HeaderTruncated;
        diag.detail = "short read of rotation block";
        return LW_RET_FAILED;
    }

    int count = 0;
    if (std::fread(&count, sizeof(int), 1, fp.get()) != 1) {
        diag.status = EffectLoadStatus::HeaderTruncated;
        diag.detail = "short read of effect count";
        return LW_RET_FAILED;
    }
    if (count < 0) {
        diag.status = EffectLoadStatus::ParseFailed;
        diag.detail = std::format("negative effect count={}", count);
        return LW_RET_FAILED;
    }

    info.effects.resize(static_cast<std::size_t>(count));
    for (int n = 0; n < count; ++n) {
        if (!LoadElement(info.effects[n], fp.get(), version)) {
            diag.status = EffectLoadStatus::ParseFailed;
            diag.detail = std::format("I_Effect[{}].Load failed", n);
            return LW_RET_FAILED;
        }
    }

    diag.status = EffectLoadStatus::Ok;
    return LW_RET_OK;
}

LW_RESULT EffectLoader::Save(::EffectFileInfo& info, std::string_view file) {
    UniqueFile fp{std::fopen(std::string{file}.c_str(), "wb")};
    if (!fp) {
        return LW_RET_FAILED;
    }

    // Save всегда пишет latest format. Если info.version < kCurrentVersion —
    // старые файлы при пересохранении подтягиваются к актуальной структуре.
    DWORD version = kCurrentVersion;
    std::fwrite(&version, sizeof(version), 1, fp.get());

    int idxTech = info.param._idxTech;
    std::fwrite(&idxTech, sizeof(int), 1, fp.get());

    std::fwrite(&info.param._usePath, sizeof(bool), 1, fp.get());
    WriteFixedName32(info.param._pathName, fp.get());

    std::fwrite(&info.param._useSound, sizeof(bool), 1, fp.get());
    WriteFixedName32(info.param._soundName, fp.get());

    std::fwrite(&info.param._rotating, sizeof(bool), 1, fp.get());
    std::fwrite(&info.param._verRota, sizeof(D3DXVECTOR3), 1, fp.get());
    std::fwrite(&info.param._rotaVel, sizeof(float), 1, fp.get());

    int count = static_cast<int>(info.effects.size());
    std::fwrite(&count, sizeof(int), 1, fp.get());

    for (int n = 0; n < count; ++n) {
        if (!SaveElement(info.effects[n], fp.get())) {
            return LW_RET_FAILED;
        }
    }

    return LW_RET_OK;
}

// =============================================================================
// I_Effect (один элемент .eff). Перенесено сюда из I_Effect::Load/SaveToFile —
// data-классы по соглашению проекта не должны содержать I/O.
// =============================================================================

bool EffectLoader::SaveElement(::I_Effect& e, std::FILE* fp) {
    // Имена в .eff-файле — фиксированный 32-байтный буфер.
    auto writeFixedName = [&](std::string_view src) {
        char t_pszName[32]{};
        std::memcpy(t_pszName, src.data(),
                    std::min<std::size_t>(src.size(), sizeof(t_pszName) - 1));
        std::fwrite(t_pszName, sizeof(char), 32, fp);
    };
    writeFixedName(e._strEffectName);

    int t_temp = (int)e._eEffectType;
    std::fwrite(&t_temp, sizeof(int), 1, fp);

    t_temp = (int)e._eSrcBlend;
    std::fwrite(&t_temp, sizeof(int), 1, fp);
    t_temp = (int)e._eDestBlend;
    std::fwrite(&t_temp, sizeof(int), 1, fp);

    std::fwrite(&e._fLength, sizeof(float), 1, fp);
    std::fwrite(&e._wFrameCount, sizeof(WORD), 1, fp);

    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fwrite(&e._vecFrameTime[n], sizeof(float), 1, fp);
    }
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fwrite(&e._vecFrameSize[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fwrite(&e._vecFrameAngle[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fwrite(&e._vecFramePos[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fwrite(&e._vecFrameColor[n], sizeof(D3DXCOLOR), 1, fp);
    }

    std::fwrite(&e._CTexCoordlist._wVerCount, sizeof(WORD), 1, fp);
    std::fwrite(&e._CTexCoordlist._wCoordCount, sizeof(WORD), 1, fp);
    std::fwrite(&e._CTexCoordlist._fFrameTime, sizeof(float), 1, fp);

    for (WORD n = 0; n < e._CTexCoordlist._wCoordCount; ++n) {
        std::fwrite(&e._CTexCoordlist._vecCoordList[n].front(),
                    sizeof(D3DXVECTOR2), e._CTexCoordlist._wVerCount, fp);
    }

    std::fwrite(&e._CTextruelist._wTexCount, sizeof(WORD), 1, fp);
    std::fwrite(&e._CTextruelist._fFrameTime, sizeof(float), 1, fp);

    writeFixedName(e._CTextruelist._vecTexName);

    for (WORD n = 0; n < e._CTextruelist._wTexCount; n++) {
        std::fwrite(&e._CTextruelist._vecTexList[n].front(),
                    sizeof(D3DXVECTOR2), e._CTexCoordlist._wVerCount, fp);
    }

    // Имя модели и cylinder-параметры пишем из собственных полей I_Effect
    // (LoadElement наполняет именно их). Раньше Save обращался через
    // _pCModel->_strName / ->_nSegments — что валится с nullptr-deref'ом
    // на тулз-/тестовом round-trip'е, где engine-binding (Reset) пропущен.
    // При нормальной работе клиента _pCModel и I_Effect-поля синхронны.
    writeFixedName(e._strModelName);

    std::fwrite(&e._bBillBoard, sizeof(bool), 1, fp);
    std::fwrite(&e._iVSIndex, sizeof(int), 1, fp);

    std::fwrite(&e._nSegments, sizeof(int), 1, fp);
    std::fwrite(&e._rHeight, sizeof(float), 1, fp);
    std::fwrite(&e._rRadius, sizeof(float), 1, fp);
    std::fwrite(&e._rBotRadius, sizeof(float), 1, fp);

    std::fwrite(&e._CTexFrame._wTexCount, sizeof(WORD), 1, fp);
    std::fwrite(&e._CTexFrame._fFrameTime, sizeof(float), 1, fp);
    for (WORD n = 0; n < e._CTexFrame._wTexCount; ++n) {
        writeFixedName(e._CTexFrame._vecTexName[n]);
    }
    std::fwrite(&e._CTexFrame._fFrameTime, sizeof(float), 1, fp);

    std::fwrite(&e._iUseParam, sizeof(int), 1, fp);
    if (e._iUseParam > 0) {
        for (int n = 0; n < e._wFrameCount; ++n) {
            std::fwrite(&e._CylinderParam[n].iSegments, sizeof(int), 1, fp);
            std::fwrite(&e._CylinderParam[n].fHei, sizeof(float), 1, fp);
            std::fwrite(&e._CylinderParam[n].fTopRadius, sizeof(float), 1, fp);
            std::fwrite(&e._CylinderParam[n].fBottomRadius, sizeof(float), 1, fp);
        }
    }
    std::fwrite(&e._bRotaLoop, sizeof(bool), 1, fp);
    std::fwrite(&e._vRotaLoop, sizeof(D3DXVECTOR4), 1, fp);

    std::fwrite(&e._bAlpha, sizeof(bool), 1, fp);
    std::fwrite(&e._bRotaBoard, sizeof(bool), 1, fp);

    return true;
}

bool EffectLoader::LoadElement(::I_Effect& e, std::FILE* fp, DWORD dwVersion) {
    e.ReleaseAll();

    char t_pszName[32];
    std::fread(t_pszName, sizeof(char), 32, fp);
    e._strEffectName = t_pszName;

    int t_temp;
    std::fread(&t_temp, sizeof(int), 1, fp);
    e._eEffectType = (EFFECT_TYPE)t_temp;

    std::fread(&t_temp, sizeof(int), 1, fp);
    e._eSrcBlend = (D3DBLEND)t_temp;
    std::fread(&t_temp, sizeof(int), 1, fp);
    e._eDestBlend = (D3DBLEND)t_temp;

    std::fread(&e._fLength, sizeof(float), 1, fp);
    std::fread(&e._wFrameCount, sizeof(WORD), 1, fp);

    e._vecFrameTime.resize(e._wFrameCount);
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fread(&e._vecFrameTime[n], sizeof(float), 1, fp);
    }
    e._vecFrameSize.resize(e._wFrameCount);
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fread(&e._vecFrameSize[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    e._vecFrameAngle.resize(e._wFrameCount);
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fread(&e._vecFrameAngle[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    e._vecFramePos.resize(e._wFrameCount);
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fread(&e._vecFramePos[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    e._vecFrameColor.resize(e._wFrameCount);
    for (WORD n = 0; n < e._wFrameCount; n++) {
        std::fread(&e._vecFrameColor[n], sizeof(D3DXCOLOR), 1, fp);
    }

    std::fread(&e._CTexCoordlist._wVerCount, sizeof(WORD), 1, fp);
    std::fread(&e._CTexCoordlist._wCoordCount, sizeof(WORD), 1, fp);
    std::fread(&e._CTexCoordlist._fFrameTime, sizeof(float), 1, fp);
    e._CTexCoordlist._vecCoordList.resize(e._CTexCoordlist._wCoordCount);
    for (WORD n = 0; n < e._CTexCoordlist._wCoordCount; n++) {
        e._CTexCoordlist._vecCoordList[n].resize(e._CTexCoordlist._wVerCount);
        std::fread(&e._CTexCoordlist._vecCoordList[n].front(),
                   sizeof(D3DXVECTOR2), e._CTexCoordlist._wVerCount, fp);
    }

    std::fread(&e._CTextruelist._wTexCount, sizeof(WORD), 1, fp);
    std::fread(&e._CTextruelist._fFrameTime, sizeof(float), 1, fp);

    std::fread(t_pszName, sizeof(char), 32, fp);
    std::string lowerName{t_pszName};
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::string_view lowerView{lowerName};
    if (lowerView.ends_with(".dds") || lowerView.ends_with(".tga") || lowerView.ends_with(".png")) {
        e._CTextruelist._vecTexName.assign(lowerView.substr(0, lowerView.size() - 4));
    }
    else {
        e._CTextruelist._vecTexName = lowerName;
    }

    e._CTextruelist._vecTexList.resize(e._CTextruelist._wTexCount);
    for (WORD n = 0; n < e._CTextruelist._wTexCount; n++) {
        e._CTextruelist._vecTexList[n].resize(e._CTexCoordlist._wVerCount);
        std::fread(&e._CTextruelist._vecTexList[n].front(),
                   sizeof(D3DXVECTOR2), e._CTexCoordlist._wVerCount, fp);
    }

    std::fread(t_pszName, sizeof(char), 32, fp);
    e._strModelName = t_pszName;

    std::fread(&e._bBillBoard, sizeof(bool), 1, fp);
    std::fread(&e._iVSIndex, sizeof(int), 1, fp);

    if (dwVersion > 1) {
        std::fread(&e._nSegments, sizeof(int), 1, fp);
        std::fread(&e._rHeight, sizeof(float), 1, fp);
        std::fread(&e._rRadius, sizeof(float), 1, fp);
        std::fread(&e._rBotRadius, sizeof(float), 1, fp);
    }
    if (dwVersion > 2) {
        std::fread(&e._CTexFrame._wTexCount, sizeof(WORD), 1, fp);
        std::fread(&e._CTexFrame._fFrameTime, sizeof(float), 1, fp);

        e._CTexFrame._vecTexName.resize(e._CTexFrame._wTexCount);
        e._CTexFrame._vecTexs.resize(e._CTexFrame._wTexCount);

        for (WORD n = 0; n < e._CTexFrame._wTexCount; ++n) {
            std::fread(t_pszName, sizeof(char), 32, fp);
            e._CTexFrame._vecTexName[n] = t_pszName;
        }
        std::fread(&e._CTexFrame._fFrameTime, sizeof(float), 1, fp);
    }
    e._iUseParam = 0;
    e._CylinderParam.resize(e._wFrameCount);

    if (dwVersion > 3) {
        std::fread(&e._iUseParam, sizeof(int), 1, fp);
        if (e._iUseParam > 0) {
            for (int n = 0; n < e._wFrameCount; ++n) {
                std::fread(&e._CylinderParam[n].iSegments, sizeof(int), 1, fp);
                std::fread(&e._CylinderParam[n].fHei, sizeof(float), 1, fp);
                std::fread(&e._CylinderParam[n].fTopRadius, sizeof(float), 1, fp);
                std::fread(&e._CylinderParam[n].fBottomRadius, sizeof(float), 1, fp);

                e._CylinderParam[n].Create();
            }
        }
        else {
            if (IsCylinderMesh(e._strModelName)) {
                for (int n = 0; n < e._wFrameCount; ++n) {
                    e._CylinderParam[n].iSegments = e._nSegments;
                    e._CylinderParam[n].fTopRadius = e._rRadius;
                    e._CylinderParam[n].fBottomRadius = e._rBotRadius;
                    e._CylinderParam[n].fHei = e._rHeight;
                    e._CylinderParam[n].Create();
                }
            }
        }
    }
    else {
        if (IsCylinderMesh(e._strModelName)) {
            e._iUseParam = 0;

            for (int n = 0; n < e._wFrameCount; ++n) {
                e._CylinderParam[n].iSegments = e._nSegments;
                e._CylinderParam[n].fTopRadius = e._rRadius;
                e._CylinderParam[n].fBottomRadius = e._rBotRadius;
                e._CylinderParam[n].fHei = e._rHeight;
                e._CylinderParam[n].Create();
            }
        }
    }
    if (dwVersion > 4) {
        std::fread(&e._bRotaLoop, sizeof(bool), 1, fp);
        std::fread(&e._vRotaLoop, sizeof(D3DXVECTOR4), 1, fp);
    }
    if (dwVersion > 5) {
        std::fread(&e._bAlpha, sizeof(bool), 1, fp);
    }
    if (dwVersion > 6) {
        std::fread(&e._bRotaBoard, sizeof(bool), 1, fp);
    }

    e.IsSame();
    return true;
}

// =============================================================================
// PartCtrlLoader (.par) — чтение/запись CMPPartCtrl и его per-element данных
// (CMPPartSys/CMPStrip/CChaModel). Все методы data-классов перенесены сюда
// через friend-доступ; data-классы стали POD-данными без I/O.
// =============================================================================

// --- CMPPartSys (один particle system внутри .par) ----------------------------

bool PartCtrlLoader::SavePartSys(::CMPPartSys& ps, std::FILE* fp) {
    auto writeFixedName = [&](std::string_view src) {
        char buf[32]{};
        std::memcpy(buf, src.data(),
                    std::min<std::size_t>(src.size(), sizeof(buf) - 1));
        std::fwrite(buf, sizeof(char), 32, fp);
    };

    int tm;
    std::fwrite(&ps._iType, sizeof(int), 1, fp);
    writeFixedName(ps._strPartName);
    std::fwrite(&ps._iParNum, sizeof(int), 1, fp);
    writeFixedName(ps._strTexName);
    writeFixedName(ps._strModelName);

    std::fwrite(&ps._fRange[0], sizeof(float), 1, fp);
    std::fwrite(&ps._fRange[1], sizeof(float), 1, fp);
    std::fwrite(&ps._fRange[2], sizeof(float), 1, fp);

    std::fwrite(&ps._wFrameCount, sizeof(WORD), 1, fp);

    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fwrite(ps._vecFrameSize[n], sizeof(float), 1, fp);
    }
    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fwrite(ps._vecFrameAngle[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fwrite(ps._vecFrameColor[n], sizeof(D3DXCOLOR), 1, fp);
    }
    std::fwrite(&ps._bBillBoard, sizeof(bool), 1, fp);

    tm = (int)ps._eSrcBlend;
    std::fwrite(&tm, sizeof(int), 1, fp);
    tm = (int)ps._eDestBlend;
    std::fwrite(&tm, sizeof(int), 1, fp);
    tm = (int)ps._eMinFilter;
    std::fwrite(&tm, sizeof(int), 1, fp);
    tm = (int)ps._eMagFilter;
    std::fwrite(&tm, sizeof(int), 1, fp);

    std::fwrite(&ps._fLife, sizeof(float), 1, fp);
    std::fwrite(&ps._fVecl, sizeof(float), 1, fp);
    std::fwrite(&ps._vDir, sizeof(D3DXVECTOR3), 1, fp);
    std::fwrite(&ps._vAccel, sizeof(D3DXVECTOR3), 1, fp);
    std::fwrite(&ps._fStep, sizeof(float), 1, fp);

    std::fwrite(&ps._bModelRange, sizeof(bool), 1, fp);
    writeFixedName(ps._strVirualModel);

    std::fwrite(&ps._vOffset, sizeof(D3DXVECTOR3), 1, fp);
    std::fwrite(&ps._fDelayTime, sizeof(float), 1, fp);
    std::fwrite(&ps._fPlayTime, sizeof(float), 1, fp);

    bool busepsth = ps._pcPath ? true : false;
    std::fwrite(&busepsth, sizeof(bool), 1, fp);
    if (busepsth) {
        EffPathLoader::WritePath(*ps._pcPath, fp);
    }

    std::fwrite(&ps._bShade, sizeof(bool), 1, fp);

    writeFixedName(ps._strHitEff);

    if (ps._bModelRange) {
        std::fwrite(&ps._wVecNum, sizeof(WORD), 1, fp);
        std::fwrite(&ps._vecPointRange.front(), sizeof(D3DXVECTOR3), ps._wVecNum, fp);
    }
    std::fwrite(&ps._iRoadom, sizeof(int), 1, fp);
    std::fwrite(&ps._bModelDir, sizeof(bool), 1, fp);
    std::fwrite(&ps._bMediay, sizeof(bool), 1, fp);

    return true;
}

bool PartCtrlLoader::LoadPartSys(::CMPPartSys& ps, std::FILE* fp, DWORD dwVersion) {
    int tm;
    std::fread(&ps._iType, sizeof(int), 1, fp);

    char t_pszName[32];
    std::fread(t_pszName, sizeof(char), 32, fp);
    ps._strPartName = t_pszName;

    std::fread(&ps._iParNum, sizeof(int), 1, fp);
    ps._vecParticle.setsize(ps._iParNum);

    std::fread(t_pszName, sizeof(char), 32, fp);
    s_string sFileName = t_pszName;
    std::transform(sFileName.begin(), sFileName.end(),
                   sFileName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (sFileName.rfind(".dds") == std::string::npos
        && sFileName.rfind(".tga") == std::string::npos
        && sFileName.rfind(".png") == std::string::npos) {
        ps._strTexName = sFileName;
    }
    else {
        ps._strTexName = sFileName.substr(0, sFileName.rfind('.'));
    }

    std::fread(t_pszName, sizeof(char), 32, fp);
    if (IsDefaultMesh(t_pszName)) {
        ps._strModelName = t_pszName;
    }
    else {
        sFileName = t_pszName;
        std::transform(sFileName.begin(), sFileName.end(),
                       sFileName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        ps._strModelName = sFileName;
    }
    std::fread(&ps._fRange[0], sizeof(float), 1, fp);
    std::fread(&ps._fRange[1], sizeof(float), 1, fp);
    std::fread(&ps._fRange[2], sizeof(float), 1, fp);

    std::fread(&ps._wFrameCount, sizeof(WORD), 1, fp);

    ps._vecFrameSize.setsize(ps._wFrameCount);
    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fread(ps._vecFrameSize[n], sizeof(float), 1, fp);
    }
    ps._vecFrameAngle.setsize(ps._wFrameCount);
    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fread(ps._vecFrameAngle[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    ps._vecFrameColor.setsize(ps._wFrameCount);
    for (WORD n = 0; n < ps._wFrameCount; n++) {
        std::fread(ps._vecFrameColor[n], sizeof(D3DXCOLOR), 1, fp);
    }
    std::fread(&ps._bBillBoard, sizeof(bool), 1, fp);

    std::fread(&tm, sizeof(int), 1, fp);
    ps._eSrcBlend = (D3DBLEND)tm;
    std::fread(&tm, sizeof(int), 1, fp);
    ps._eDestBlend = (D3DBLEND)tm;
    std::fread(&tm, sizeof(int), 1, fp);
    ps._eMinFilter = (D3DTEXTUREFILTERTYPE)tm;
    std::fread(&tm, sizeof(int), 1, fp);
    ps._eMagFilter = (D3DTEXTUREFILTERTYPE)tm;

    std::fread(&ps._fLife, sizeof(float), 1, fp);
    std::fread(&ps._fVecl, sizeof(float), 1, fp);
    std::fread(&ps._vDir, sizeof(D3DXVECTOR3), 1, fp);
    std::fread(&ps._vAccel, sizeof(D3DXVECTOR3), 1, fp);
    std::fread(&ps._fStep, sizeof(float), 1, fp);

    if (dwVersion > 3) {
        std::fread(&ps._bModelRange, sizeof(bool), 1, fp);
        std::fread(t_pszName, sizeof(char), 32, fp);
    }
    if (dwVersion > 4) {
        std::fread(&ps._vOffset, sizeof(D3DXVECTOR3), 1, fp);
    }
    if (dwVersion > 5) {
        std::fread(&ps._fDelayTime, sizeof(float), 1, fp);
        std::fread(&ps._fPlayTime, sizeof(float), 1, fp);
    }
    if (dwVersion > 8) {
        SAFE_DELETE(ps._pcPath);
        bool busepsth;
        std::fread(&busepsth, sizeof(bool), 1, fp);
        if (busepsth) {
            ps._pcPath = new CEffPath;
            EffPathLoader::ReadPath(*ps._pcPath, fp);
            ps._pcPath->Reset();
        }
    }
    if (dwVersion > 9) {
        std::fread(&ps._bShade, sizeof(bool), 1, fp);
    }
    if (dwVersion > 10) {
        std::fread(t_pszName, sizeof(char), 32, fp);
        ps._strHitEff = t_pszName;
    }
    if (dwVersion > 11) {
        if (ps._bModelRange) {
            std::fread(&ps._wVecNum, sizeof(WORD), 1, fp);
            ps._vecPointRange.resize(ps._wVecNum);
            std::fread(&ps._vecPointRange.front(), sizeof(D3DXVECTOR3), ps._wVecNum, fp);
        }
    }
    if (dwVersion > 12) {
        std::fread(&ps._iRoadom, sizeof(int), 1, fp);
    }
    else {
        if (ps._iType == PARTTICLE_SNOW
            || ps._iType == PARTTICLE_FIRE
            || ps._iType == PARTTICLE_DUMMY) {
            ps._iRoadom = 4;
        }
        else {
            ps._iRoadom = 2;
        }
    }
    if (dwVersion > 13) {
        std::fread(&ps._bModelDir, sizeof(bool), 1, fp);
    }
    if (dwVersion > 14) {
        std::fread(&ps._bMediay, sizeof(bool), 1, fp);
    }

    return true;
}

// --- CMPStrip (один strip внутри .par, версия >= 7) ---------------------------

bool PartCtrlLoader::SaveStrip(::CMPStrip& s, std::FILE* fp) {
    std::fwrite(&s.m_iMaxLen, sizeof(int), 1, fp);
    std::fwrite(&s._iDummy, sizeof(int), 2, fp);
    std::fwrite(&s._dwColor, sizeof(D3DXCOLOR), 1, fp);
    std::fwrite(&s._fLife, sizeof(float), 1, fp);
    std::fwrite(&s._fStep, sizeof(float), 1, fp);

    char buf[32]{};
    std::memcpy(buf, s._strTexName.data(),
                std::min<std::size_t>(s._strTexName.size(), sizeof(buf) - 1));
    std::fwrite(buf, sizeof(char), 32, fp);

    int te = (int)s._eSrcBlend;
    std::fwrite(&te, sizeof(int), 1, fp);
    te = (int)s._eDestBlend;
    std::fwrite(&te, sizeof(int), 1, fp);
    return true;
}

bool PartCtrlLoader::LoadStrip(::CMPStrip& s, std::FILE* fp, DWORD /*dwVersion*/) {
    std::fread(&s.m_iMaxLen, sizeof(int), 1, fp);
    std::fread(&s._iDummy, sizeof(int), 2, fp);
    std::fread(&s._dwColor, sizeof(D3DXCOLOR), 1, fp);
    std::fread(&s._fLife, sizeof(float), 1, fp);
    std::fread(&s._fStep, sizeof(float), 1, fp);

    char buf[32]{};
    std::fread(buf, sizeof(char), 32, fp);

    std::string_view nameView{buf};
    if (nameView.ends_with(".dds") || nameView.ends_with(".tga") || nameView.ends_with(".png")) {
        s._strTexName.assign(nameView.substr(0, nameView.size() - 4));
    }
    else {
        s._strTexName = nameView;
    }
    int te;
    std::fread(&te, sizeof(int), 1, fp);
    s._eSrcBlend = (D3DBLEND)te;
    std::fread(&te, sizeof(int), 1, fp);
    s._eDestBlend = (D3DBLEND)te;
    return true;
}

// --- CChaModel (один character-model внутри .par, версия >= 8) ----------------

void PartCtrlLoader::SaveCharModel(::CChaModel& m, std::FILE* fp) {
    std::fwrite(&m._iID, sizeof(int), 1, fp);
    std::fwrite(&m.m_fVel, sizeof(float), 1, fp);
    std::fwrite(&m._iPlayType, sizeof(int), 1, fp);
    std::fwrite(&m._iCurPose, sizeof(int), 1, fp);

    int eblend = (int)m._eSrcBlend;
    std::fwrite(&eblend, sizeof(int), 1, fp);
    eblend = (int)m._eDestBlend;
    std::fwrite(&eblend, sizeof(int), 1, fp);
    std::fwrite(&m.m_dwCurColor, sizeof(D3DXCOLOR), 1, fp);
}

void PartCtrlLoader::LoadCharModel(::CChaModel& m, std::FILE* fp) {
    std::fread(&m._iID, sizeof(int), 1, fp);
    std::fread(&m.m_fVel, sizeof(float), 1, fp);
    std::fread(&m._iPlayType, sizeof(int), 1, fp);
    std::fread(&m._iCurPose, sizeof(int), 1, fp);

    int eblend;
    std::fread(&eblend, sizeof(int), 1, fp);
    m._eSrcBlend = (D3DBLEND)eblend;
    std::fread(&eblend, sizeof(int), 1, fp);
    m._eDestBlend = (D3DBLEND)eblend;
    std::fread(&m.m_dwCurColor, sizeof(D3DXCOLOR), 1, fp);

    // Engine-binding после чтения raw-данных. Те же действия, что делал
    // CChaModel::LoadFromFile до миграции (script binding + initial pose).
    const std::string psID = std::format("{}", m._iID);
    m.LoadScript(psID);
    m.SetVel((int)(m.m_fVel * 1000));
    m.SetPlayType(m._iPlayType);
    m.SetCurPose(m._iCurPose);
    m.PlayPose(m._iCurPose, PLAY_PAUSE);
}

bool PartCtrlLoader::LoadCtrl(::CMPPartCtrl& ctrl, std::string_view pszName) {
    const std::string nameStr{pszName};
    std::FILE* t_pFile = std::fopen(nameStr.c_str(), "rb");
    if (t_pFile == nullptr) {
        ToLogService("errors", LogLevel::Error,
                     "[{}] was not opened.(PartCtrlLoader::Load)", pszName);
        return false;
    }
    UniqueFile fp{t_pFile};

    DWORD t_dwVersion = 0;
    std::fread(&t_dwVersion, sizeof(t_dwVersion), 1, fp.get());
    if (t_dwVersion > static_cast<DWORD>(::CMPPartCtrl::ParVersion)) {
        ToLogService("errors", LogLevel::Error,
                     "[{}][{}][{}] (PartCtrlLoader::Load)", pszName, t_dwVersion,
                     static_cast<int>(::CMPPartCtrl::ParVersion));
        return false;
    }
    if (t_dwVersion < 2) {
        ToLogService("errors", LogLevel::Error,
                     "[{}][{}][{}] (PartCtrlLoader::Load)", pszName, t_dwVersion, 2);
        return false;
    }

    char pszPartName[32];
    std::fread(pszPartName, sizeof(char), 32, fp.get());
    ctrl._strName = pszPartName;

    std::fread(&ctrl._iPartNum, sizeof(int), 1, fp.get());
#ifdef USE_GAME
    ctrl._vecPartSys.resize(ctrl._iPartNum);
#endif
    ctrl._vecPartSys.setsize(ctrl._iPartNum);

    if (t_dwVersion >= 3) {
        std::fread(&ctrl._fLength, sizeof(float), 1, fp.get());
    }
    else {
        ctrl._fLength = 0;
    }
    for (int n = 0; n < ctrl._iPartNum; ++n) {
        if (!LoadPartSys(*ctrl._vecPartSys[n], fp.get(), t_dwVersion)) {
            ToLogService("errors", LogLevel::Error,
                         "[{}][{}].(PartCtrlLoader::Load)", pszName, n);
            return false;
        }
        if (t_dwVersion < 6) {
            ctrl._vecPartSys[n]->SetPlayTime(ctrl._fLength);
        }
    }
    if (t_dwVersion >= 7) {
        SAFE_DELETE_ARRAY(ctrl._pcStrip);

        std::fread(&ctrl._iStripNum, sizeof(int), 1, fp.get());

        ctrl._pcStrip = new ::CMPStrip[ctrl._iStripNum];

        for (int n = 0; n < ctrl._iStripNum; ++n) {
            if (!LoadStrip(ctrl._pcStrip[n], fp.get(), t_dwVersion)) {
                ToLogService("errors", LogLevel::Error,
                             "[{}][{}]Strip.(PartCtrlLoader::Load)", pszName, n);
                delete[] ctrl._pcStrip;
                ctrl._pcStrip = nullptr;
                return false;
            }
        }
    }
    if (t_dwVersion >= 8) {
        std::fread(&ctrl._iModelNum, sizeof(int), 1, fp.get());
        ctrl._vecModel.resize(ctrl._iModelNum);
        for (int n = 0; n < ctrl._iModelNum; ++n) {
            ctrl._vecModel[n] = new ::CChaModel;
            LoadCharModel(*ctrl._vecModel[n], fp.get());
        }
    }
    return true;
}

bool PartCtrlLoader::SaveCtrl(::CMPPartCtrl& ctrl, std::string_view pszName) {
    const std::string nameStr{pszName};
    std::FILE* t_pFile = std::fopen(nameStr.c_str(), "wb");
    if (t_pFile == nullptr) {
        ToLogService("errors", LogLevel::Error,
                     "[{}] was not opened.(PartCtrlLoader::Save)", pszName);
        return false;
    }
    UniqueFile fp{t_pFile};

    DWORD t_dwVersion = static_cast<DWORD>(::CMPPartCtrl::ParVersion);
    std::fwrite(&t_dwVersion, sizeof(t_dwVersion), 1, fp.get());

    char pszPartName[32]{};
    std::memcpy(pszPartName, ctrl._strName.data(),
                std::min<std::size_t>(ctrl._strName.size(), sizeof(pszPartName) - 1));
    std::fwrite(pszPartName, sizeof(char), 32, fp.get());

    std::fwrite(&ctrl._iPartNum, sizeof(int), 1, fp.get());
    std::fwrite(&ctrl._fLength, sizeof(float), 1, fp.get());

    for (int n = 0; n < ctrl._iPartNum; ++n) {
        SavePartSys(*ctrl._vecPartSys[n], fp.get());
    }
    std::fwrite(&ctrl._iStripNum, sizeof(int), 1, fp.get());
    for (int n = 0; n < ctrl._iStripNum; ++n) {
        SaveStrip(ctrl._pcStrip[n], fp.get());
    }
    std::fwrite(&ctrl._iModelNum, sizeof(int), 1, fp.get());
    for (int n = 0; n < ctrl._iModelNum; ++n) {
        SaveCharModel(*ctrl._vecModel[n], fp.get());
    }
    return true;
}

LW_RESULT PartCtrlLoader::Load(::CMPPartCtrl& ctrl, std::string_view file) {
    return LoadCtrl(ctrl, file) ? LW_RET_OK : LW_RET_FAILED;
}

LW_RESULT PartCtrlLoader::LoadEx(::CMPPartCtrl& ctrl, std::string_view file,
                                  PartCtrlLoadDiagnostics& diag) {
    diag = {};

    // Сначала пытаемся прочитать только version-DWORD — для осмысленной
    // диагностики (FileOpenFailed / VersionTruncated / VersionUnknown) до
    // того, как дёрнем сам CMPPartCtrl::LoadFromFile.
    {
        UniqueFile fp{std::fopen(std::string{file}.c_str(), "rb")};
        if (!fp) {
            diag.status = PartCtrlLoadStatus::FileOpenFailed;
            diag.detail = "fopen failed";
            return LW_RET_FAILED;
        }

        DWORD version = 0;
        if (std::fread(&version, sizeof(version), 1, fp.get()) != 1) {
            diag.status = PartCtrlLoadStatus::VersionTruncated;
            diag.detail = "short read of version DWORD";
            return LW_RET_FAILED;
        }
        diag.version = version;

        // Минимум 2 — раньше CMPPartCtrl::LoadFromFile требует именно это;
        // максимум — текущий ParVersion (15 на момент написания).
        constexpr DWORD kMinParVersion = 2;
        const DWORD     kMaxParVersion = static_cast<DWORD>(::CMPPartCtrl::ParVersion);
        if (version < kMinParVersion || version > kMaxParVersion) {
            diag.status = PartCtrlLoadStatus::VersionUnknown;
            diag.detail = std::format("version=0x{:08X} (expected {}..{})",
                                      version, kMinParVersion, kMaxParVersion);
            return LW_RET_FAILED;
        }
    }

    if (!LoadCtrl(ctrl, file)) {
        diag.status = PartCtrlLoadStatus::ParseFailed;
        diag.detail = "PartCtrlLoader::Load returned false";
        return LW_RET_FAILED;
    }

    diag.status = PartCtrlLoadStatus::Ok;
    return LW_RET_OK;
}

LW_RESULT PartCtrlLoader::Save(::CMPPartCtrl& ctrl, std::string_view file) {
    return SaveCtrl(ctrl, file) ? LW_RET_OK : LW_RET_FAILED;
}

// =============================================================================
// EffPathLoader — два соседних формата path-кадров для CEffPath:
//   • .csf — продакшн-формат (model/effect/*.csf), Load/Save;
//   • .let — редакторский формат поверх matrix-track, LoadLet.
// Плюс поточные WritePath/ReadPath для эмбединга CEffPath в .par.
// =============================================================================

LW_RESULT EffPathLoader::Load(::CEffPath& path, std::string_view file) {
    EffPathLoadDiagnostics diag;
    return LoadEx(path, file, diag);
}

LW_RESULT EffPathLoader::LoadEx(::CEffPath& path, std::string_view file,
                                EffPathLoadDiagnostics& diag) {
    diag = {};

    UniqueFile fp{std::fopen(std::string{file}.c_str(), "rb")};
    if (!fp) {
        diag.status = EffPathLoadStatus::FileOpenFailed;
        diag.detail = "fopen failed";
        return LW_RET_FAILED;
    }

    // Заголовок — три ASCII-байта "csf" + 1 нулевой; формат правят редакторы,
    // поэтому проверяем чётко 3 первых символа.
    char header[4]{};
    if (std::fread(header, sizeof(char), 4, fp.get()) != 4) {
        diag.status = EffPathLoadStatus::HeaderTruncated;
        diag.detail = "short read of 4-byte magic";
        return LW_RET_FAILED;
    }
    if (std::string_view{header, 3} != "csf") {
        diag.status = EffPathLoadStatus::BadMagic;
        diag.detail = std::format("magic='{}{}{}' (expected 'csf')",
                                  header[0], header[1], header[2]);
        return LW_RET_FAILED;
    }

    DWORD version = 0;
    DWORD num = 0;
    if (std::fread(&version, sizeof(DWORD), 1, fp.get()) != 1 ||
        std::fread(&num, sizeof(DWORD), 1, fp.get()) != 1) {
        diag.status = EffPathLoadStatus::HeaderTruncated;
        diag.detail = "short read of version/num";
        return LW_RET_FAILED;
    }
    diag.version = version;
    diag.frameCount = num;

    if (num == 0 || num > kMaxFrames) {
        diag.status = EffPathLoadStatus::FrameCountOutOfRange;
        diag.detail = std::format("num={} (expected 1..{})", num, kMaxFrames);
        return LW_RET_FAILED;
    }

    path._iFrameCount = static_cast<int>(num);

    // Файл хранит right-handed координаты: на лету меняем Y/Z.
    for (DWORD n = 0; n < num; ++n) {
        D3DXVECTOR3 tvec{};
        if (std::fread(&tvec, sizeof(D3DXVECTOR3), 1, fp.get()) != 1) {
            diag.status = EffPathLoadStatus::BodyTruncated;
            diag.detail = std::format("short read of frame[{}] of {}", n, num);
            return LW_RET_FAILED;
        }
        const float ftemp = tvec.y;
        tvec.y = -tvec.z;
        tvec.z = ftemp;
        path._vecPath[n] = tvec;
    }

    for (DWORD n = 0; n + 1 < num; ++n) {
        path._vecDir[n] = path._vecPath[n + 1] - path._vecPath[n];
        path._vecDist[n] = D3DXVec3Length(&path._vecDir[n]);
        D3DXVec3Normalize(&path._vecDir[n], &path._vecDir[n]);
    }

    diag.status = EffPathLoadStatus::Ok;
    return LW_RET_OK;
}

LW_RESULT EffPathLoader::LoadLet(::CEffPath& path, std::string_view file) {
    Corsairs::Engine::Render::EfxTrack et;
    const std::string fileStr{file};
    if (LW_RESULT r = EfxTrackLoader::Load(et, fileStr); LW_FAILED(r)) {
        return r;
    }

    Corsairs::Engine::Render::lwIAnimDataMatrix* data = et.GetData();
    const int j = data->GetFrameNum();
    for (int i = 0; i < j; ++i) {
        ::lwMatrix44 mat;
        data->GetValue(&mat, static_cast<float>(i));
        path._vecPath[i].x = mat._41;
        path._vecPath[i].y = mat._42;
        path._vecPath[i].z = mat._43;
    }
    for (int i = 0; i + 1 < j; ++i) {
        path._vecDir[i] = path._vecPath[i + 1] - path._vecPath[i];
        path._vecDist[i] = D3DXVec3Length(&path._vecDir[i]);
        D3DXVec3Normalize(&path._vecDir[i], &path._vecDir[i]);
    }

    path._iFrameCount = j;
    return LW_RET_OK;
}

LW_RESULT EffPathLoader::Save(const ::CEffPath& path, std::string_view file) {
    if (path._iFrameCount <= 0
        || static_cast<std::uint32_t>(path._iFrameCount) > kMaxFrames) {
        return LW_RET_FAILED;
    }

    UniqueFile fp{std::fopen(std::string{file}.c_str(), "wb")};
    if (!fp) {
        return LW_RET_FAILED;
    }

    constexpr char header[4]{'c', 's', 'f', '\0'};
    if (std::fwrite(header, sizeof(char), 4, fp.get()) != 4) {
        return LW_RET_FAILED;
    }

    constexpr DWORD kVersion = kCurrentVersion;
    const DWORD num = static_cast<DWORD>(path._iFrameCount);
    if (std::fwrite(&kVersion, sizeof(DWORD), 1, fp.get()) != 1 ||
        std::fwrite(&num, sizeof(DWORD), 1, fp.get()) != 1) {
        return LW_RET_FAILED;
    }

    // Обратная инверсия Y/Z из left-hand runtime → right-hand на диске.
    // Симметрично Load: runtime{x,y,z} ↔ disk{x, z, -y}.
    for (DWORD n = 0; n < num; ++n) {
        D3DXVECTOR3 tvec;
        tvec.x = path._vecPath[n].x;
        tvec.y = path._vecPath[n].z;
        tvec.z = -path._vecPath[n].y;
        if (std::fwrite(&tvec, sizeof(D3DXVECTOR3), 1, fp.get()) != 1) {
            return LW_RET_FAILED;
        }
    }

    return LW_RET_OK;
}

void EffPathLoader::WritePath(const ::CEffPath& path, std::FILE* fp) {
    std::fwrite(&path._iFrameCount, sizeof(int), 1, fp);
    std::fwrite(&path.m_fVel, sizeof(float), 1, fp);

    for (int n = 0; n < path._iFrameCount; ++n) {
        std::fwrite(&path._vecPath[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    // Историческая особенность: _vecDist — float[200], но размер записи
    // указан sizeof(D3DXVECTOR3) (12 байт). Сохраняем legacy-layout, чтобы
    // существующие .par-файлы оставались совместимы.
    for (int n = 0; n + 1 < path._iFrameCount; ++n) {
        std::fwrite(&path._vecDir[n], sizeof(D3DXVECTOR3), 1, fp);
        std::fwrite(&path._vecDist[n], sizeof(D3DXVECTOR3), 1, fp);
    }
}

void EffPathLoader::ReadPath(::CEffPath& path, std::FILE* fp) {
    std::fread(&path._iFrameCount, sizeof(int), 1, fp);
    std::fread(&path.m_fVel, sizeof(float), 1, fp);

    for (int n = 0; n < path._iFrameCount; ++n) {
        std::fread(&path._vecPath[n], sizeof(D3DXVECTOR3), 1, fp);
    }
    for (int n = 0; n + 1 < path._iFrameCount; ++n) {
        std::fread(&path._vecDir[n], sizeof(D3DXVECTOR3), 1, fp);
        std::fread(&path._vecDist[n], sizeof(D3DXVECTOR3), 1, fp);
    }
}

} // namespace Corsairs::Engine::Render
