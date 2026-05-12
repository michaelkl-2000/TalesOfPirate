//
#pragma once

#include "lwHeader.h"

namespace Corsairs::Engine::Render {
	class lwLinkCtrl {
	public:
		virtual LW_RESULT GetLinkCtrlMatrix(lwMatrix44* mat, DWORD link_id) = 0;
	};


} // namespace Corsairs::Engine::Render
