// DdsLoader — запись .dds-файлов из lwDDSFile (тело перенесено из
// lwDDSFile.cpp). Чтение DDS живёт в lwDDSFile::LoadOriginTexture через
// D3DXCreateTextureFromFileEx; никакого fopen в нём нет, потому к loader-
// паттерну подключать его незачем.
//
// ScreenshotSaver — дамп IDirect3DSurfaceX в PNG (через D3DXSaveSurfaceToFile).
// Раньше MPRender::CaptureScreen писал .bmp вручную ofstream'ом.
//
// Вынесено в отдельный TU, чтобы не таскать lwDDS.h и d3dx9.h (для PNG-
// сохранения) в основной AssetLoaders.cpp, и чтобы тулзы вроде
// AssetLoaderTests/PkoTool, не работающие с runtime-текстурами, не пуллили
// эти .obj-ы.

#include "stdafx.h"
#include "AssetLoaders.h"

#include "lwDDSFile.h"
#include "lwgraphicsutil.h"
#include "logutil.h"

#include <cstdio>
#include <memory>
#include <string>

#include <d3dx9tex.h>

namespace Corsairs::Engine::Render {

namespace {

struct FileCloser {
    void operator()(std::FILE* fp) const noexcept {
        if (fp != nullptr) {
            std::fclose(fp);
        }
    }
};
using UniqueFile = std::unique_ptr<std::FILE, FileCloser>;

} // namespace

// =============================================================================
// Внутренние шаги .dds-Save'а (приватные static-методы DdsLoader, чтобы
// friend-объявление в lwDDSFile открывало им доступ к приватным полям).
// =============================================================================

long DdsLoader::SaveDDSHeader(LW_NAMESPACE::lwDDSFile& dds,
                               struct IDirect3DBaseTexture9* texRaw, std::FILE* fp) {
    using namespace LW_NAMESPACE;
    auto* tex = reinterpret_cast<IDirect3DBaseTextureX*>(texRaw);

    lwDDSHeader ddsh{};
    DWORD dwMagic = MAKEFOURCC('D', 'D', 'S', ' ');
    D3DFORMAT fmt = D3DFMT_UNKNOWN;
    DWORD size = 0;
    DWORD dwPitch = 0;

    ddsh.size = sizeof(ddsh);
    ddsh.header_flag = DDS_HEADER_FLAGS_TEXTURE;
    ddsh.width = dds._tex_width;
    ddsh.height = dds._tex_height;
    ddsh.surface_flag = DDS_SURFACE_FLAGS_TEXTURE;
    if (dds._mip_level > 1) {
        ddsh.header_flag |= DDS_HEADER_FLAGS_MIPMAP;
        ddsh.surface_flag |= DDS_SURFACE_FLAGS_MIPMAP;
        ddsh.mipmap_count = dds._mip_level;
    }
    if (dds.IsVolumeMap()) {
        ddsh.header_flag |= DDS_HEADER_FLAGS_VOLUME;
        ddsh.cubemap_flag |= DDS_FLAGS_VOLUME;
        ddsh.volume_depth = dds._volume_depth;
    }
    if (dds.IsCubeMap()) {
        ddsh.surface_flag |= DDS_SURFACE_FLAGS_CUBEMAP;
        ddsh.cubemap_flag = DDS_CUBEMAP_ALLFACES;
    }

    if (dds.IsVolumeMap()) {
        D3DVOLUME_DESC vd;
        D3DLOCKED_BOX lb;
        IDirect3DVolumeTextureX* pvoltex = (IDirect3DVolumeTextureX*)tex;
        pvoltex->GetLevelDesc(0, &vd);
        fmt = vd.Format;
        size = lwGetSurfaceSize(vd.Width, vd.Height, vd.Format);
        if (SUCCEEDED(pvoltex->LockBox(0, &lb, NULL, D3DLOCK_READONLY))) {
            dwPitch = lb.RowPitch;
            pvoltex->UnlockBox(0);
        }
    }
    else if (dds.IsCubeMap()) {
        D3DSURFACE_DESC sd;
        D3DLOCKED_RECT lr;
        IDirect3DCubeTextureX* pcubetex = (IDirect3DCubeTextureX*)tex;
        pcubetex->GetLevelDesc(0, &sd);
        fmt = sd.Format;
        size = lwGetSurfaceSize(sd.Width, sd.Height, sd.Format);
        if (SUCCEEDED(pcubetex->LockRect(D3DCUBEMAP_FACE_POSITIVE_X, 0,
            &lr, NULL, D3DLOCK_READONLY))) {
            dwPitch = lr.Pitch;
            pcubetex->UnlockRect(D3DCUBEMAP_FACE_POSITIVE_X, 0);
        }
    }
    else {
        D3DSURFACE_DESC sd;
        D3DLOCKED_RECT lr;
        IDirect3DTextureX* pmiptex = (IDirect3DTextureX*)tex;
        pmiptex->GetLevelDesc(0, &sd);
        fmt = sd.Format;
        size = lwGetSurfaceSize(sd.Width, sd.Height, sd.Format);
        if (SUCCEEDED(pmiptex->LockRect(0, &lr, NULL, D3DLOCK_READONLY))) {
            dwPitch = lr.Pitch;
            pmiptex->UnlockRect(0);
        }
    }

    switch (fmt) {
    case D3DFMT_DXT1:
        ddsh.ddspf = DDSPF_DXT1;
        ddsh.header_flag |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.pitch_or_linearsize = size;
        break;
    case D3DFMT_DXT2:
        ddsh.ddspf = DDSPF_DXT2;
        ddsh.header_flag |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.pitch_or_linearsize = size;
        break;
    case D3DFMT_DXT3:
        ddsh.ddspf = DDSPF_DXT3;
        ddsh.header_flag |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.pitch_or_linearsize = size;
        break;
    case D3DFMT_DXT4:
        ddsh.ddspf = DDSPF_DXT4;
        ddsh.header_flag |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.pitch_or_linearsize = size;
        break;
    case D3DFMT_DXT5:
        ddsh.ddspf = DDSPF_DXT5;
        ddsh.header_flag |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.pitch_or_linearsize = size;
        break;
    case D3DFMT_A8R8G8B8:
        ddsh.ddspf = DDSPF_A8R8G8B8;
        ddsh.header_flag |= DDS_HEADER_FLAGS_PITCH;
        ddsh.pitch_or_linearsize = dwPitch;
        break;
    case D3DFMT_A1R5G5B5:
        ddsh.ddspf = DDSPF_A1R5G5B5;
        ddsh.header_flag |= DDS_HEADER_FLAGS_PITCH;
        ddsh.pitch_or_linearsize = dwPitch;
        break;
    case D3DFMT_A4R4G4B4:
        ddsh.ddspf = DDSPF_A4R4G4B4;
        ddsh.header_flag |= DDS_HEADER_FLAGS_PITCH;
        ddsh.pitch_or_linearsize = dwPitch;
        break;
    case D3DFMT_R8G8B8:
        ddsh.ddspf = DDSPF_R8G8B8;
        ddsh.header_flag |= DDS_HEADER_FLAGS_PITCH;
        ddsh.pitch_or_linearsize = dwPitch;
        break;
    case D3DFMT_R5G6B5:
        ddsh.ddspf = DDSPF_R5G6B5;
        ddsh.header_flag |= DDS_HEADER_FLAGS_PITCH;
        ddsh.pitch_or_linearsize = dwPitch;
        break;
    default:
        return E_FAIL;
    }

    std::fwrite(&dwMagic, sizeof(dwMagic), 1, fp);
    std::fwrite(&ddsh, sizeof(ddsh), 1, fp);
    return S_OK;
}

long DdsLoader::SaveAllMipSurfaces(LW_NAMESPACE::lwDDSFile& dds,
                                    struct IDirect3DBaseTexture9* ptexRaw,
                                    unsigned int faceTypeRaw,
                                    std::FILE* fp) {
    using namespace LW_NAMESPACE;
    auto* ptex = reinterpret_cast<IDirect3DBaseTextureX*>(ptexRaw);
    const auto FaceType = static_cast<D3DCUBEMAP_FACES>(faceTypeRaw);
    HRESULT hr;
    IDirect3DSurfaceX* psurf;
    D3DSURFACE_DESC sd;
    D3DLOCKED_RECT lr;
    IDirect3DTextureX* pmiptex = nullptr;
    IDirect3DCubeTextureX* pcubetex = nullptr;

    if (FaceType == D3DCUBEMAP_FACE_FORCE_DWORD) {
        pmiptex = (IDirect3DTextureX*)ptex;
    }
    else {
        pcubetex = (IDirect3DCubeTextureX*)ptex;
    }

    for (UINT iLevel = 0; iLevel < dds._mip_level; ++iLevel) {
        if (pmiptex != nullptr) {
            hr = pmiptex->GetSurfaceLevel(iLevel, &psurf);
        }
        else {
            hr = pcubetex->GetCubeMapSurface(FaceType, iLevel, &psurf);
        }
        if (FAILED(hr)) {
            ToLogService("errors", LogLevel::Error,
                         "[DdsLoader] GetSurfaceLevel/GetCubeMapSurface failed: iLevel={}, FaceType={}, hr=0x{:08X}",
                         static_cast<long long>(iLevel),
                         static_cast<long long>(FaceType),
                         static_cast<std::uint32_t>(hr));
            return hr;
        }

        psurf->GetDesc(&sd);

        if (pmiptex != nullptr) {
            hr = pmiptex->LockRect(iLevel, &lr, NULL, 0);
        }
        else {
            hr = pcubetex->LockRect(FaceType, iLevel, &lr, NULL, 0);
        }
        if (FAILED(hr)) {
            ToLogService("errors", LogLevel::Error,
                         "[DdsLoader] LockRect failed: iLevel={}, FaceType={}, hr=0x{:08X}",
                         static_cast<long long>(iLevel),
                         static_cast<long long>(FaceType),
                         static_cast<std::uint32_t>(hr));
            return hr;
        }
        if (sd.Format == D3DFMT_DXT1
            || sd.Format == D3DFMT_DXT2
            || sd.Format == D3DFMT_DXT3
            || sd.Format == D3DFMT_DXT4
            || sd.Format == D3DFMT_DXT5) {
            std::fwrite(lr.pBits, lwGetSurfaceSize(sd.Width, sd.Height, sd.Format), 1, fp);
        }
        else {
            BYTE* pbDest = (BYTE*)lr.pBits;
            LONG dataBytesPerRow = 0;
            if (sd.Format == D3DFMT_A8R8G8B8) {
                dataBytesPerRow = 4 * sd.Width;
            }
            else if (sd.Format == D3DFMT_R8G8B8) {
                dataBytesPerRow = 3 * sd.Width;
            }
            else {
                dataBytesPerRow = 2 * sd.Width;
            }
            for (DWORD yp = 0; yp < sd.Height; ++yp) {
                std::fwrite(pbDest, dataBytesPerRow, 1, fp);
                pbDest += lr.Pitch;
            }
        }
        if (pmiptex != nullptr) {
            pmiptex->UnlockRect(iLevel);
        }
        else {
            pcubetex->UnlockRect(FaceType, iLevel);
        }
        LW_IF_RELEASE(psurf);
    }
    return S_OK;
}

long DdsLoader::SaveAllVolumeSurfaces(LW_NAMESPACE::lwDDSFile& dds,
                                       struct IDirect3DVolumeTexture9* pvoltexRaw,
                                       std::FILE* fp) {
    using namespace LW_NAMESPACE;
    auto* pvoltex = reinterpret_cast<IDirect3DVolumeTextureX*>(pvoltexRaw);

    for (UINT iLevel = 0; iLevel < dds._mip_level; ++iLevel) {
        D3DVOLUME_DESC vd;
        pvoltex->GetLevelDesc(iLevel, &vd);
        D3DBOX box{};
        box.Left = 0;
        box.Right = vd.Width;
        box.Top = 0;
        box.Bottom = vd.Height;
        box.Front = 0;
        box.Back = vd.Depth;
        D3DLOCKED_BOX lb;
        HRESULT hr = pvoltex->LockBox(iLevel, &lb, &box, 0);
        if (FAILED(hr)) {
            ToLogService("errors", LogLevel::Error,
                         "[DdsLoader] LockBox failed: iLevel={}, hr=0x{:08X}",
                         static_cast<long long>(iLevel),
                         static_cast<std::uint32_t>(hr));
            return hr;
        }
        UINT numBytesPerRow = 0;
        switch (vd.Format) {
        case D3DFMT_A8R8G8B8: numBytesPerRow = 4 * vd.Width; break;
        case D3DFMT_R8G8B8:   numBytesPerRow = 3 * vd.Width; break;
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4:
        case D3DFMT_R5G6B5:   numBytesPerRow = 2 * vd.Width; break;
        default:
            return E_FAIL;
        }
        BYTE* pbSlice = (BYTE*)lb.pBits;
        for (UINT zp = 0; zp < vd.Depth; ++zp) {
            BYTE* pbRow = pbSlice;
            for (UINT yp = 0; yp < vd.Height; ++yp) {
                std::fwrite(pbRow, numBytesPerRow, 1, fp);
                pbRow += lb.RowPitch;
            }
            pbSlice += lb.SlicePitch;
        }
        pvoltex->UnlockBox(iLevel);
    }
    return S_OK;
}

// =============================================================================
// DdsLoader::Save
// =============================================================================

LW_RESULT DdsLoader::Save(LW_NAMESPACE::lwDDSFile& dds, std::string_view file) {
    UniqueFile fp{std::fopen(std::string{file}.c_str(), "wb")};
    if (!fp) {
        ToLogService("errors", LogLevel::Error,
                     "[DdsLoader::Save] fopen failed: {}", file);
        return LW_RET_FAILED;
    }

    IDirect3DBaseTextureX* ptex = (dds._dds_tex == nullptr ? dds._origin_tex : dds._dds_tex);
    if (ptex == nullptr) {
        ToLogService("errors", LogLevel::Error,
                     "[DdsLoader::Save] no texture loaded: {}", file);
        return LW_RET_FAILED;
    }

    if (HRESULT hr = SaveDDSHeader(dds, ptex, fp.get()); FAILED(hr)) {
        ToLogService("errors", LogLevel::Error,
                     "[DdsLoader::Save] SaveDDSHeader failed: file={}, hr=0x{:08X}",
                     file, static_cast<std::uint32_t>(hr));
        return LW_RET_FAILED;
    }

    if (dds.IsVolumeMap()) {
        if (HRESULT hr = SaveAllVolumeSurfaces(dds, (IDirect3DVolumeTextureX*)ptex, fp.get());
            FAILED(hr)) {
            ToLogService("errors", LogLevel::Error,
                         "[DdsLoader::Save] SaveAllVolumeSurfaces failed: file={}, hr=0x{:08X}",
                         file, static_cast<std::uint32_t>(hr));
            return LW_RET_FAILED;
        }
        return LW_RET_OK;
    }

    if (dds.IsCubeMap()) {
        constexpr D3DCUBEMAP_FACES kFaces[] = {
            D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
            D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
            D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z,
        };
        for (D3DCUBEMAP_FACES face : kFaces) {
            if (HRESULT hr = SaveAllMipSurfaces(dds, ptex, face, fp.get()); FAILED(hr)) {
                ToLogService("errors", LogLevel::Error,
                             "[DdsLoader::Save] SaveAllMipSurfaces face={} failed: hr=0x{:08X}",
                             static_cast<long long>(face),
                             static_cast<std::uint32_t>(hr));
                return LW_RET_FAILED;
            }
        }
        return LW_RET_OK;
    }

    if (HRESULT hr = SaveAllMipSurfaces(dds, ptex, D3DCUBEMAP_FACE_FORCE_DWORD, fp.get());
        FAILED(hr)) {
        ToLogService("errors", LogLevel::Error,
                     "[DdsLoader::Save] SaveAllMipSurfaces(generic) failed: hr=0x{:08X}",
                     static_cast<std::uint32_t>(hr));
        return LW_RET_FAILED;
    }
    return LW_RET_OK;
}

// =============================================================================
// ScreenshotSaver
// =============================================================================

namespace {

// Определить формат по расширению. По умолчанию — PNG.
[[nodiscard]] D3DXIMAGE_FILEFORMAT InferFormat(std::string_view file) {
    auto ends_with = [&](std::string_view suffix) {
        return file.size() >= suffix.size()
            && std::equal(suffix.rbegin(), suffix.rend(), file.rbegin(),
                          [](char a, char b) {
                              return std::tolower(static_cast<unsigned char>(a))
                                   == std::tolower(static_cast<unsigned char>(b));
                          });
    };
    if (ends_with(".bmp")) return D3DXIFF_BMP;
    if (ends_with(".jpg") || ends_with(".jpeg")) return D3DXIFF_JPG;
    if (ends_with(".tga")) return D3DXIFF_TGA;
    if (ends_with(".dds")) return D3DXIFF_DDS;
    return D3DXIFF_PNG;
}

} // namespace

LW_RESULT ScreenshotSaver::SaveSurface(IDirect3DSurfaceX* surface, std::string_view file) {
    if (surface == nullptr) {
        ToLogService("errors", LogLevel::Error,
                     "[ScreenshotSaver::SaveSurface] surface is null: {}", file);
        return LW_RET_FAILED;
    }

    const std::string fileStr{file};
    const D3DXIMAGE_FILEFORMAT fmt = InferFormat(file);

    HRESULT hr = D3DXSaveSurfaceToFileA(fileStr.c_str(), fmt, surface, nullptr, nullptr);
    if (FAILED(hr)) {
        ToLogService("errors", LogLevel::Error,
                     "[ScreenshotSaver::SaveSurface] D3DXSaveSurfaceToFile failed: file={}, fmt={}, hr=0x{:08X}",
                     file, static_cast<long long>(fmt), static_cast<std::uint32_t>(hr));
        return LW_RET_FAILED;
    }
    return LW_RET_OK;
}

} // namespace Corsairs::Engine::Render
