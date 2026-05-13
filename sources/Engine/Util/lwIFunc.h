//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwClassDecl.h"
#include "lwInterface.h"
#include "lwITypes.h"


namespace Corsairs::Engine::Render {
	enum lwInitMeshLibErrorType {
		INIT_ERR_CREATE_D3D = -1000,
		INIT_ERR_CREATE_DEVICE = -1001,
		INIT_ERR_DX_VERSION = -1002,
	};

	void lwSetActiveIGraphicsSystem(ISysGraphics* sys_graphics);
	ISysGraphics* lwGetActiveIGraphicsSystem();

	LW_RESULT lwAdjustD3DCreateParam(IDirect3DX* d3d, lwD3DCreateParam* param, lwD3DCreateParamAdjustInfo* adjust_info);
	LW_RESULT lwInitMeshLibSystem(ISystem** ret_sys, ISysGraphics** ret_sys_graphics);
	LW_RESULT lwInitMeshLibSystem(ISystem** ret_sys, ISysGraphics** ret_sys_graphics, IDirect3DX* d3d,
								  IDirect3DDeviceX* dev, HWND hwnd);
	LW_RESULT lwInitMeshLibSystem(ISystem** ret_sys, ISysGraphics** ret_sys_graphics, lwD3DCreateParam* param,
								  lwD3DCreateParamAdjustInfo* param_info);
	LW_RESULT lwReleaseMeshLibSystem();

	void lwSetActiveISystem(ISystem* sys);
	ISystem* lwGetActiveISystem();

	LW_RESULT lwReleaseD3DObject(ISystem* sys, ISysGraphics* sys_graphics);

	LW_RESULT lwHitTestBox(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, const lwBox* box,
						   const lwMatrix44* mat);
	void HelperSetForceIgnoreTexFlag(DWORD flag);

	void lwWorldToScreen(int* x, int* y, float* z, const lwVector3* vec, int width, int height,
						 const lwMatrix44* mat_proj, const lwMatrix44* mat_view);
	void lwUpdateSceneTransparentObject();

	LW_RESULT lwRegisterOutputLoseDeviceProc(lwOutputLoseDeviceProc proc);
	LW_RESULT lwRegisterOutputResetDeviceProc(lwOutputResetDeviceProc proc);
	LW_RESULT lwUnregisterOutputLoseDeviceProc(lwOutputLoseDeviceProc proc);
	LW_RESULT lwUnregisterOutputResetDeviceProc(lwOutputResetDeviceProc proc);

	LW_RESULT LoadResModelBuf(IResourceMgr* res_mgr, std::string_view file);

	struct InterfaceMgr {
		ISystem* sys;
		ISysGraphics* sys_graphics;
		IDeviceObject* dev_obj;
		IResourceMgr* res_mgr;
		IThreadPool* tp_loadres;
	};

} // namespace Corsairs::Engine::Render
