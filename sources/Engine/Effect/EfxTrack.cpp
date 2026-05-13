// EfxTrack: только конструктор/деструктор. Загрузка/сохранение перенесена
// в Corsairs::Engine::Render::EfxTrackLoader (AssetLoaders.h/cpp).
#include "stdafx.h"
#include "EfxTrack.h"

namespace Corsairs::Engine::Render {
	EfxTrack::EfxTrack() {
		_data = 0;
	}

	EfxTrack::~EfxTrack() {
		LW_SAFE_RELEASE(_data);
	}
} // namespace Corsairs::Engine::Render
