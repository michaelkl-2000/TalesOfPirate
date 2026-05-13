//
#pragma once

#include "lwHeader.h"
#include "lwMath.h"
#include "lwInterfaceExt.h"
#include "lwExpObj.h"

// Forward-decl loader из AssetLoaders.h — самостоятельный include тут излишен.
namespace Corsairs::Engine::Render { class EfxTrackLoader; }

namespace Corsairs::Engine::Render {
	class EfxTrack {
	private:
		lwAnimDataMatrix* _data;

		// Все .track-I/O делегировано Corsairs::Engine::Render::EfxTrackLoader
		// (см. AssetLoaders.h). Loader пишет напрямую в _data, чтобы не плодить
		// сеттеров под единственный сериализационный путь.
		friend class Corsairs::Engine::Render::EfxTrackLoader;

	public:
		EfxTrack();
		~EfxTrack();

		void SetData(lwAnimDataMatrix* data) {
			_data = data;
		}

		lwAnimDataMatrix* GetData() {
			return _data;
		}
	};

} // namespace Corsairs::Engine::Render
