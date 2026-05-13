//

#include "stdafx.h"
#include "System.h"
#include "SysGraphics.h"
#include "PathInfo.h"
#include "SystemInfo.h"
#include "Timer.h"

namespace Corsairs::Engine::Render {
	ISystem* System::__system = LW_NULL;

	LW_STD_RELEASE(System)

	LW_RESULT System::GetInterface(LW_VOID** i, lwGUID guid) {
		LW_RESULT ret;

		switch (guid) {
		case LW_GUID_PATHINFO:
			*i = (LW_VOID*)_path_info;
			ret = LW_RET_OK;
			break;
		case LW_GUID_OPTIONMGR:
			*i = (LW_VOID*)_option_mgr;
			ret = LW_RET_OK;
			break;
		case LW_GUID_TIMER:
			*i = (LW_VOID*)_internal_timer;
			ret = LW_RET_OK;
			break;
		case LW_GUID_SYSTEMINFO:
			*i = (LW_VOID*)_system_info;
			ret = LW_RET_OK;
			break;
		default:
			ret = LW_RET_NULL;
		}

		return ret;
	}

	System::System() {
		_path_info = 0;
		_option_mgr = 0;
		_system_info = 0;
		_internal_timer = 0;
	}

	System::~System() {
		LW_SAFE_RELEASE(_path_info);
		LW_SAFE_RELEASE(_option_mgr);
		LW_SAFE_RELEASE(_system_info);
		LW_SAFE_RELEASE(_internal_timer);
	}

	LW_RESULT System::Initialize() {
		LW_RESULT ret = LW_RET_FAILED;

		_path_info = LW_NEW(PathInfo);
		_option_mgr = LW_NEW(OptionMgr);

		_system_info = LW_NEW(SystemInfo);
		_internal_timer = LW_NEW(Timer);

		if (LW_RESULT r = _system_info->CheckDirectXVersion(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] CheckDirectXVersion failed: ret={}",
						 __FUNCTION__, static_cast<long long>(r));
			goto __ret;
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT System::CreateGraphicsSystem(ISysGraphics** sys) {
		*sys = LW_NEW(SysGraphics( this ));

		return LW_RET_OK;
	}


} // namespace Corsairs::Engine::Render
