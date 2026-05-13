//
#pragma once

#include "lwHeader.h"
#include "lwInterfaceExt.h"
#include "lwStdInc.h"
#include "lwDirectX.h"
#include "lwClassDecl.h"
#include "CoordinateSys.h"

namespace Corsairs::Engine::Render {
	class SysGraphics : public ISysGraphics {
	public:
		static ISysGraphics* __sys_graphics;

		static void SetActiveIGraphicsSystem(ISysGraphics* sys) {
			__sys_graphics = sys;
		}

		static ISysGraphics* GetActiveIGraphicsSystem() {
			return __sys_graphics;
		}

	private:
		System* _sys;

		IDeviceObject* _dev_obj;
		IResourceMgr* _res_mgr;
		lwISceneMgr* _scene_mgr;

		CoordinateSys _coord_sys;

		lwOutputLoseDeviceProc _lose_dev_proc;
		lwOutputResetDeviceProc _reset_dev_proc;

		LW_STD_DECLARATION();

	public:
		SysGraphics(System* sys);
		~SysGraphics();

		ISystem* GetSystem() {
			return (ISystem*)_sys;
		}

		IDeviceObject* GetDeviceObject() {
			return _dev_obj;
		}

		IResourceMgr* GetResourceMgr() {
			return _res_mgr;
		}

		lwISceneMgr* GetSceneMgr() {
			return _scene_mgr;
		}

		ICoordinateSys* GetCoordinateSys() {
			return &_coord_sys;
		}

		LW_RESULT CreateDeviceObject(IDeviceObject** ret_obj);
		LW_RESULT CreateResourceManager(IResourceMgr** ret_obj);
		LW_RESULT CreateSceneManager(lwISceneMgr** ret_obj);

		LW_RESULT ToggleFullScreen(D3DPRESENT_PARAMETERS* d3dpp, lwWndInfo* wnd_info);
		LW_RESULT TestCooperativeLevel();

		void SetOutputLoseDeviceProc(lwOutputLoseDeviceProc proc) {
			_lose_dev_proc = proc;
		}

		void SetOutputResetDeviceProc(lwOutputResetDeviceProc proc) {
			_reset_dev_proc = proc;
		}
	};

} // namespace Corsairs::Engine::Render
