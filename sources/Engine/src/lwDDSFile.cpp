//
#include "stdafx.h"
#include "lwDDSFile.h"
#include "lwErrorCode.h"
#include "lwGraphicsutil.h"
namespace Corsairs::Engine::Render {
	LW_STD_IMPLEMENTATION(lwDDSFile)

	lwDDSFile::lwDDSFile()
		: _dev(0), _origin_tex(0), _dds_tex(0) {
		_cubemap_flag = 0;
		_volume_depth = 0;
		_tex_width = 0;
		_tex_height = 0;
		_mip_level = D3DX_DEFAULT;
	}

	lwDDSFile::~lwDDSFile() {
		LW_IF_RELEASE(_origin_tex);
		LW_IF_RELEASE(_dds_tex);
	}

	LW_RESULT lwDDSFile::Clear() {
		_cubemap_flag = 0;
		_volume_depth = 0;
		_tex_width = 0;
		_tex_height = 0;
		_mip_level = D3DX_DEFAULT;

		LW_SAFE_RELEASE(_origin_tex);
		LW_SAFE_RELEASE(_dds_tex);

		return LW_RET_OK;
	}

	LW_RESULT lwDDSFile::LoadOriginTexture(std::string_view file, DWORD mip_level, D3DFORMAT format, DWORD colorkey) {
		LW_RESULT ret = LW_RET_FAILED;

		IDirect3DTextureX* tex = 0;

		{
			const std::string fileStr{file};
			HRESULT hr = D3DXCreateTextureFromFileEx(
				_dev,
				fileStr.c_str(),
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				mip_level,
				0,
				format,
				D3DPOOL_MANAGED,
				D3DX_FILTER_POINT,
				D3DX_FILTER_POINT,
				colorkey,
				NULL,
				NULL,
				&tex);
			if (FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] D3DXCreateTextureFromFileEx failed: file='{}', mip_level={}, format={}, hr=0x{:08X}",
							 __FUNCTION__, (file.empty() ? std::string_view{"(null)"} : file),
							 static_cast<long long>(mip_level), static_cast<long long>(format),
							 static_cast<std::uint32_t>(hr));
				goto __ret;
			}
		}

		{
			// Ensure that source image dimensions are power of 2
			D3DSURFACE_DESC sd;
			tex->GetLevelDesc(0, &sd);
			_tex_width = sd.Width;
			_tex_height = sd.Height;

			LONG w = sd.Width;
			LONG h = sd.Height;
			LONG pow_w = 1;
			LONG pow_h = 1;

			while ((w & 1) == 0) {
				pow_w++;
				w = w >> 1;
			}

			while ((h & 1) == 0) {
				pow_h++;
				h = h >> 1;
			}

			if (w != 1 || h != 1)
				goto __ret;

			_mip_level = D3DX_DEFAULT;

			LW_IF_RELEASE(_origin_tex);
			_origin_tex = tex;
		}
		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT lwDDSFile::Compress(D3DFORMAT new_fmt) {
		LW_RESULT ret = LW_RET_FAILED;

		D3DFORMAT src_fmt;
		IDirect3DTextureX* pmiptex;
		IDirect3DCubeTextureX* pcubetex;
		IDirect3DVolumeTextureX* pvoltex;
		IDirect3DTextureX* pmiptexNew;
		IDirect3DCubeTextureX* pcubetexNew;
		IDirect3DVolumeTextureX* pvoltexNew;

		if (IsVolumeMap()) {
			if (new_fmt == D3DFMT_DXT1 ||
				new_fmt == D3DFMT_DXT2 ||
				new_fmt == D3DFMT_DXT3 ||
				new_fmt == D3DFMT_DXT4 ||
				new_fmt == D3DFMT_DXT5) {
				goto __ret;
			}
			pvoltex = (IDirect3DVolumeTextureX*)_origin_tex;
			D3DVOLUME_DESC vd;
			pvoltex->GetLevelDesc(0, &vd);
			src_fmt = vd.Format;
		}
		else if (IsCubeMap()) {
			pcubetex = (IDirect3DCubeTextureX*)_origin_tex;
			D3DSURFACE_DESC sd;
			pcubetex->GetLevelDesc(0, &sd);
			src_fmt = sd.Format;
		}
		else {
			pmiptex = (IDirect3DTextureX*)_origin_tex;
			D3DSURFACE_DESC sd;
			pmiptex->GetLevelDesc(0, &sd);
			src_fmt = sd.Format;
		}

		if (src_fmt == D3DFMT_DXT2 || src_fmt == D3DFMT_DXT4) {
			if (new_fmt == D3DFMT_DXT1)
				goto __ret;
			else if (new_fmt != D3DFMT_DXT2 && new_fmt != D3DFMT_DXT4)
				goto __ret;
		}

		if (IsVolumeMap()) {
			if (HRESULT hr = _dev->CreateVolumeTextureX(
				_tex_width,
				_tex_height,
				_volume_depth,
				_mip_level,
				0,
				new_fmt,
				D3DPOOL_SYSTEMMEM,
				&pvoltexNew,
				NULL); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] CreateVolumeTextureX failed: w={}, h={}, depth={}, mip={}, fmt={}, hr=0x{:08X}",
							 __FUNCTION__, static_cast<long long>(_tex_width),
							 static_cast<long long>(_tex_height), static_cast<long long>(_volume_depth),
							 static_cast<long long>(_mip_level), static_cast<long long>(new_fmt),
							 static_cast<std::uint32_t>(hr));
				goto __ret;
			}

			LW_IF_RELEASE(_dds_tex);
			_dds_tex = pvoltexNew;

			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_FORCE_DWORD, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(volume) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
		}
		else if (IsCubeMap()) {
			if (HRESULT hr = _dev->CreateCubeTextureX(
				_tex_width,
				_mip_level,
				0,
				new_fmt,
				D3DPOOL_MANAGED,
				&pcubetexNew,
				NULL); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] CreateCubeTextureX failed: edge={}, mip={}, fmt={}, hr=0x{:08X}",
							 __FUNCTION__, static_cast<long long>(_tex_width),
							 static_cast<long long>(_mip_level), static_cast<long long>(new_fmt),
							 static_cast<std::uint32_t>(hr));
				goto __ret;
			}

			LW_IF_RELEASE(_dds_tex);
			_dds_tex = pcubetexNew;

			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_X, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube NEG_X) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_X, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube POS_X) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_Y, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube NEG_Y) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_Y, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube POS_Y) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_Z, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube NEG_Z) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_Z, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(cube POS_Z) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
		}
		else {
			if (HRESULT hr = _dev->CreateTextureX(
				_tex_width,
				_tex_height,
				_mip_level,
				0,
				new_fmt,
				D3DPOOL_MANAGED,
				&pmiptexNew,
				NULL); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] CreateTextureX failed: w={}, h={}, mip={}, fmt={}, hr=0x{:08X}",
							 __FUNCTION__, static_cast<long long>(_tex_width),
							 static_cast<long long>(_tex_height), static_cast<long long>(_mip_level),
							 static_cast<long long>(new_fmt), static_cast<std::uint32_t>(hr));
				goto __ret;
			}

			LW_IF_RELEASE(_dds_tex);
			_dds_tex = pmiptexNew;

			if (HRESULT hr = BltAllLevels(D3DCUBEMAP_FACE_FORCE_DWORD, _origin_tex, _dds_tex); FAILED(hr)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BltAllLevels(generic) failed: hr=0x{:08X}",
							 __FUNCTION__, static_cast<std::uint32_t>(hr));
				goto __ret;
			}
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	HRESULT lwDDSFile::BltAllLevels(D3DCUBEMAP_FACES FaceType, IDirect3DBaseTextureX* ptexSrc,
									IDirect3DBaseTextureX* ptexDest) {
		IDirect3DTextureX* pmiptexSrc;
		IDirect3DTextureX* pmiptexDest;
		IDirect3DCubeTextureX* pcubetexSrc;
		IDirect3DCubeTextureX* pcubetexDest;
		IDirect3DVolumeTextureX* pvoltexSrc;
		IDirect3DVolumeTextureX* pvoltexDest;
		IDirect3DSurfaceX* psurfSrc;
		IDirect3DSurfaceX* psurfDest;
		IDirect3DVolumeX* pvolSrc;
		IDirect3DVolumeX* pvolDest;
		DWORD iLevel;

		if (IsVolumeMap()) {
			pvoltexSrc = (IDirect3DVolumeTextureX*)ptexSrc;
			pvoltexDest = (IDirect3DVolumeTextureX*)ptexDest;
		}
		else if (IsCubeMap()) {
			pcubetexSrc = (IDirect3DCubeTextureX*)ptexSrc;
			pcubetexDest = (IDirect3DCubeTextureX*)ptexDest;
		}
		else {
			pmiptexSrc = (IDirect3DTextureX*)ptexSrc;
			pmiptexDest = (IDirect3DTextureX*)ptexDest;
		}

		for (iLevel = 0; iLevel < _mip_level; iLevel++) {
			if (IsVolumeMap()) {
				pvoltexSrc->GetVolumeLevel(iLevel, &pvolSrc);
				pvoltexDest->GetVolumeLevel(iLevel, &pvolDest);
				D3DXLoadVolumeFromVolume(pvolDest, NULL, NULL,
										 pvolSrc, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
				pvolSrc->Release();
				pvolDest->Release();
			}
			if (IsCubeMap()) {
				pcubetexSrc->GetCubeMapSurface(FaceType, iLevel, &psurfSrc);
				pcubetexDest->GetCubeMapSurface(FaceType, iLevel, &psurfDest);
				D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL,
										   psurfSrc, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
				psurfSrc->Release();
				psurfDest->Release();
			}
			else {
				pmiptexSrc->GetSurfaceLevel(iLevel, &psurfSrc);
				pmiptexDest->GetSurfaceLevel(iLevel, &psurfDest);

				HRESULT hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL,
														psurfSrc, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);

				psurfSrc->Release();
				psurfDest->Release();
			}
		}

		return LW_RET_OK;
	}


	LW_RESULT lwDDSFile::Convert(std::string_view file, D3DFORMAT src_fmt, D3DFORMAT dds_fmt, DWORD mip_level,
								 DWORD src_colorkey) {
		LW_RESULT ret = LW_RET_FAILED;

		if (LW_RESULT r = LoadOriginTexture(file, mip_level, src_fmt, src_colorkey); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] LoadOriginTexture failed: file='{}', mip_level={}, src_fmt={}, ret={}",
						 __FUNCTION__, (file.empty() ? std::string_view{"(null)"} : file),
						 static_cast<long long>(mip_level), static_cast<long long>(src_fmt),
						 static_cast<long long>(r));
			goto __ret;
		}

		if (LW_RESULT r = Compress(dds_fmt); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] Compress failed: dds_fmt={}, ret={}",
						 __FUNCTION__, static_cast<long long>(dds_fmt),
						 static_cast<long long>(r));
			goto __ret;
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	// Save (.dds) перенесён в Corsairs::Engine::Render::DdsLoader::Save
	// (см. AssetLoaders.h, реализация в ImageLoaders.cpp). SaveDDSHeader/
	// SaveAllMipSurfaces/SaveAllVolumeSurfaces переехали туда же как free-
	// функции в anon-namespace.
} // namespace Corsairs::Engine::Render
