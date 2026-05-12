//
#include "stdafx.h"
#include "lwPathInfo.h"
#include "lwStdInc.h"

namespace Corsairs::Engine::Render {
	LW_STD_IMPLEMENTATION(lwPathInfo)

	lwPathInfo::lwPathInfo() = default;

	void lwPathInfo::SetPath(PathInfoType type, std::string_view path) {
		_path_buf[static_cast<std::size_t>(type)] = path;
	}

	const std::string& lwPathInfo::GetPath(PathInfoType type) {
		return _path_buf[static_cast<std::size_t>(type)];
	}

	// lwOptionMgr
	LW_STD_IMPLEMENTATION(lwOptionMgr)

	lwOptionMgr::lwOptionMgr() {
		memset(_byte_flag_seq, 0, sizeof(_byte_flag_seq));

		_ignore_model_tex_flag = 0;
	}

	lwOptionMgr::~lwOptionMgr() {
	}


} // namespace Corsairs::Engine::Render
