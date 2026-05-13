//
#include "stdafx.h"
#include "PathInfo.h"
#include "lwStdInc.h"

namespace Corsairs::Engine::Render {
	LW_STD_IMPLEMENTATION(PathInfo)

	PathInfo::PathInfo() = default;

	void PathInfo::SetPath(PathInfoType type, std::string_view path) {
		_path_buf[static_cast<std::size_t>(type)] = path;
	}

	const std::string& PathInfo::GetPath(PathInfoType type) {
		return _path_buf[static_cast<std::size_t>(type)];
	}

	// OptionMgr
	LW_STD_IMPLEMENTATION(OptionMgr)

	OptionMgr::OptionMgr() {
		memset(_byte_flag_seq, 0, sizeof(_byte_flag_seq));

		_ignore_model_tex_flag = 0;
	}

	OptionMgr::~OptionMgr() {
	}


} // namespace Corsairs::Engine::Render
