// lwEfxTrack: только конструктор/деструктор. Загрузка/сохранение перенесена
// в Corsairs::Engine::Render::EfxTrackLoader (AssetLoaders.h/cpp).
#include "stdafx.h"
#include "lwEfxTrack.h"

namespace Corsairs::Engine::Render {
	lwEfxTrack::lwEfxTrack() {
		_data = 0;
	}

	lwEfxTrack::~lwEfxTrack() {
		LW_SAFE_RELEASE(_data);
	}
} // namespace Corsairs::Engine::Render
