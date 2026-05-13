//
#pragma once

#include "lwHeader.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"

namespace Corsairs::Engine::Render {
	class PathInfo : public IPathInfo {
	public:
		std::string _path_buf[static_cast<std::size_t>(PathInfoType::PATH_TYPE_NUM)];

		LW_STD_DECLARATION()

	public:
		PathInfo();

		void SetPath(PathInfoType type, std::string_view path);
		const std::string& GetPath(PathInfoType type);
	};

	class OptionMgr : public IOptionMgr {
		LW_STD_DECLARATION()

	private:
		BYTE _ignore_model_tex_flag;
		BYTE _byte_flag_seq[static_cast<std::size_t>(OptionByteFlag::MAX_OPTION_BYTE_FLAG)];

	public:
		OptionMgr();
		~OptionMgr();

		void SetIgnoreModelTexFlag(BYTE flag) {
			_ignore_model_tex_flag = flag;
		}

		BYTE GetIgnoreModelTexFlag() const {
			return _ignore_model_tex_flag;
		}

		void SetByteFlag(OptionByteFlag type, BYTE value) {
			_byte_flag_seq[static_cast<std::size_t>(type)] = value;
		}

		BYTE GetByteFlag(OptionByteFlag type) const {
			return _byte_flag_seq[static_cast<std::size_t>(type)];
		}
	};


} // namespace Corsairs::Engine::Render
