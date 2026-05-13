//
#pragma once


#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwInterface.h"
#include "lwITypes.h"
#include "lwITypes2.h"
#include "lwMath.h"
#include "lwShaderTypes.h"
#include "lwIFunc.h"
#include "lwIUtil.h"

using MPAnimCtrlObjTypeInfo       = Corsairs::Engine::Render::lwAnimCtrlObjTypeInfo;
// lwVector2/3, lwMatrix44, lwQuaternion — typedef'ы в global namespace (lwMath.h),
// не в Corsairs::Engine::Render. Поэтому без квалификации.
using MPVector2                   = lwVector2;
using MPVector3                   = lwVector3;
using MPMatrix44                  = lwMatrix44;
using MPBoundingBoxInfo           = Corsairs::Engine::Render::lwBoundingBoxInfo;
using MPPickInfo                  = Corsairs::Engine::Render::lwPickInfo;
using MPPlayPoseInfo              = Corsairs::Engine::Render::lwPlayPoseInfo;
using MPPoseInfo                  = Corsairs::Engine::Render::lwPoseInfo;
using MPInterfaceMgr              = Corsairs::Engine::Render::InterfaceMgr;
using MPMeshInfo                  = Corsairs::Engine::Render::lwMeshInfo;
using MPWatchDevVideoMemInfo      = Corsairs::Engine::Render::lwWatchDevVideoMemInfo;

using MPIAnimCtrl                 = Corsairs::Engine::Render::lwIAnimCtrl;
using MPIAnimCtrlAgent            = Corsairs::Engine::Render::lwIAnimCtrlAgent;
using MPIAnimCtrlBone             = Corsairs::Engine::Render::lwIAnimCtrlBone;
using MPIAnimCtrlObjBone          = Corsairs::Engine::Render::lwIAnimCtrlObjBone;
using MPIBoundingBox              = Corsairs::Engine::Render::lwIBoundingBox;
using MPIDeviceObject             = Corsairs::Engine::Render::IDeviceObject;
using MPStaticStreamMgrDebugInfo  = Corsairs::Engine::Render::StaticStreamMgrDebugInfo;
using MPD3DCreateParamAdjustInfo  = Corsairs::Engine::Render::lwD3DCreateParamAdjustInfo;
using MPDwordByte4                = Corsairs::Engine::Render::lwDwordByte4;
using MPIHelperObject             = Corsairs::Engine::Render::IHelperObject;
using MPIMesh                     = Corsairs::Engine::Render::lwIMesh;
using MPIPathInfo                 = Corsairs::Engine::Render::IPathInfo;
using MPIPhysique                 = Corsairs::Engine::Render::lwPhysique;
using MPIPoseCtrl                 = Corsairs::Engine::Render::lwIPoseCtrl;
using MPIPrimitive                = Corsairs::Engine::Render::lwIPrimitive;
using MPIRenderStateAtomSet       = Corsairs::Engine::Render::lwIRenderStateAtomSet;
using MPIResBufMgr                = Corsairs::Engine::Render::lwIResBufMgr;
using MPIResourceMgr              = Corsairs::Engine::Render::IResourceMgr;
using MPISceneMgr                 = Corsairs::Engine::Render::lwISceneMgr;
using MPIStaticStreamMgr          = Corsairs::Engine::Render::IStaticStreamMgr;
using MPISysGraphics              = Corsairs::Engine::Render::ISysGraphics;
using MPISystemInfo               = Corsairs::Engine::Render::ISystemInfo;
using MPISystem                   = Corsairs::Engine::Render::ISystem;
using MPITex                      = Corsairs::Engine::Render::lwITex;
using MPITimer                    = Corsairs::Engine::Render::ITimer;
using MPITimerPeriod              = Corsairs::Engine::Render::ITimerPeriod;

// macro
#ifndef MP_NEW
#define MP_NEW LW_NEW
#endif

#ifndef MP_DELETE
#define MP_DELETE LW_DELETE
#endif

#ifndef MP_DELETE_A
#define MP_DELETE_A LW_DELETE_A
#endif

// method

#ifndef MPMatrix44Multiply
#define MPMatrix44Multiply lwMatrix44Multiply
#endif

#ifndef MPRegisterOutputLoseDeviceProc
#define MPRegisterOutputLoseDeviceProc lwRegisterOutputLoseDeviceProc
#endif

#ifndef MPRegisterOutputResetDeviceProc
#define MPRegisterOutputResetDeviceProc lwRegisterOutputResetDeviceProc
#endif

#ifndef MPVec3Mat44Mul
#define MPVec3Mat44Mul lwVec3Mat44Mul
#endif

#ifndef MPVector3Normalize
#define MPVector3Normalize lwVector3Normalize
#endif

#ifndef MPVector3Slerp
#define MPVector3Slerp lwVector3Slerp
#endif

#ifndef MPVector3Sub
#define MPVector3Sub lwVector3Sub
#endif
