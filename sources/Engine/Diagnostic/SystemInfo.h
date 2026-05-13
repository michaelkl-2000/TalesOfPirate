//
#pragma once

#include "lwHeader.h"
#include "lwInterfaceExt.h"
#include "lwClassDecl.h"

namespace Corsairs::Engine::Render {
	class SystemInfo : public ISystemInfo {
		LW_STD_DECLARATION();

	private:
		lwDxVerInfo _dx_ver_info;

	public:
		SystemInfo();
		~SystemInfo();

		LW_RESULT CheckDirectXVersion();

		LW_RESULT GetDirectXVersion(lwDxVerInfo* o_info) {
			*o_info = _dx_ver_info;
			return LW_RET_OK;
		}

		DWORD GetDirectXVersion();
	};

} // namespace Corsairs::Engine::Render
