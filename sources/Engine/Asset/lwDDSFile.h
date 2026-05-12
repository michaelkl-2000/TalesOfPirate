//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwDirectX.h"
#include "lwInterfaceExt.h"

#include "lwDDS.h"

// Forward-decl loader из AssetLoaders.h — самостоятельный include тут излишен.
namespace Corsairs::Engine::Render { class DdsLoader; }

namespace Corsairs::Engine::Render {
	class lwDDSFile : public lwIDDSFile {
		LW_STD_DECLARATION()

		// Все .dds-Save живёт в Corsairs::Engine::Render::DdsLoader; loader
		// читает приватные поля (текстуры, размер, mip-уровень) и вызывает
		// приватные SaveDDSHeader/SaveAllMipSurfaces/SaveAllVolumeSurfaces.
		friend class Corsairs::Engine::Render::DdsLoader;

	private:
		IDirect3DDeviceX* _dev;
		IDirect3DBaseTextureX* _dds_tex;
		IDirect3DBaseTextureX* _origin_tex;
		DWORD _cubemap_flag;
		DWORD _volume_depth;
		DWORD _tex_width;
		DWORD _tex_height;
		DWORD _mip_level;

	private:
		LW_RESULT Compress(D3DFORMAT new_fmt);

		inline BOOL IsVolumeMap() const {
			return _volume_depth > 0;
		}

		inline BOOL IsCubeMap() const {
			return _cubemap_flag;
		}

		HRESULT BltAllLevels(D3DCUBEMAP_FACES FaceType, IDirect3DBaseTextureX* ptexSrc,
							 IDirect3DBaseTextureX* ptexDest);
		// SaveDDSHeader/SaveAllMipSurfaces/SaveAllVolumeSurfaces переехали в
		// ImageLoaders.cpp (free-функции в anon-namespace), вызываются из
		// DdsLoader::Save через friend-доступ к приватным полям.

	public:
		lwDDSFile();
		~lwDDSFile();

		void SetDevice(IDirect3DDeviceX* dev) {
			_dev = dev;
		}

		LW_RESULT Clear();
		LW_RESULT LoadOriginTexture(std::string_view file, DWORD mip_level, D3DFORMAT format, DWORD colorkey);
		LW_RESULT Convert(std::string_view file, D3DFORMAT src_fmt, D3DFORMAT dds_fmt, DWORD mip_level, DWORD src_colorkey);
	};

} // namespace Corsairs::Engine::Render
