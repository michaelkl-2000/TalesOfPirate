//
#pragma once

#include "lwHeader.h"
#include "lwInterfaceExt.h"
#include "lwClassDecl.h"

namespace Corsairs::Engine::Render {
	class System : public ISystem {
	public:
		static ISystem* __system;

		static void SetActiveISystem(ISystem* sys) {
			__system = sys;
		}

		static ISystem* GetActiveISystem() {
			return __system;
		}

	private:
		IPathInfo* _path_info;
		IOptionMgr* _option_mgr;
		ITimer* _internal_timer;
		ISystemInfo* _system_info;

		LW_STD_DECLARATION()

	public:
		System();
		~System();

		LW_RESULT Initialize();
		LW_RESULT CreateGraphicsSystem(ISysGraphics** sys);

		IPathInfo* GetPathInfo() {
			return _path_info;
		}

		IOptionMgr* GetOptionMgr() {
			return _option_mgr;
		}

		ISystemInfo* GetSystemInfo() {
			return _system_info;
		}
	};

} // namespace Corsairs::Engine::Render
