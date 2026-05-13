//
#include "stdafx.h"


#include "lwItem.h"
#include "System.h"
#include "SysGraphics.h"
#include "ResourceMgr.h"
#include "lwAnimCtrl.h"
#include "lwRenderImp.h"
#include "PathInfo.h"
#include "lwExpObj.h"
#include "AssetLoaders.h"
#include "GeomObjCache.h"

namespace Corsairs::Engine::Render {
	lwItem::lwItem(IResourceMgr* res_mgr)
		: _res_mgr(res_mgr),
		  _scene_mgr(nullptr),
		  _link_ctrl(nullptr),
		  _linkParentId(LW_INVALID_INDEX),
		  _linkItemId(LW_INVALID_INDEX),
		  _obj(nullptr),
		  _opacity(1.0f) {
		lwMatrix44Identity(&_mat_base);
	}

	lwItem::~lwItem() {
		Destroy();
	}

	LW_RESULT lwItem::Load(std::string_view file) {
		if (_obj) {
			return LW_RET_FAILED;
		}

		ISysGraphics* sys_graphics = _res_mgr->GetSysGraphics();
		ISystem* sys = sys_graphics->GetSystem();
		IPathInfo* path_info = nullptr;
		sys->GetInterface(reinterpret_cast<LW_VOID**>(&path_info), LW_GUID_PATHINFO);

		const std::string path = std::format("{}{}", path_info->GetPath(PathInfoType::PATH_TYPE_MODEL_ITEM), file);

		lwResFile res;
		res.obj_id = 0;
		res.res_type = ResourceFileType::RES_FILE_TYPE_GEOMETRY;
		std::memset(res.file_name, 0, sizeof(res.file_name));
		std::memcpy(res.file_name, path.data(),
					std::min<std::size_t>(path.size(), sizeof(res.file_name) - 1));

		// info живёт в общем GeomObjCache, shared_ptr держит запись на время Load.
		auto info = Corsairs::Engine::Render::GeomObjCache::Instance().GetOrLoad(path);
		if (!info) {
			return LW_RET_FAILED;
		}

		lwPrimitive* imp = LW_NEW(lwPrimitive(_res_mgr));

		if (LW_RESULT r = imp->Load(info.get(), path_info->GetPath(PathInfoType::PATH_TYPE_TEXTURE_ITEM).c_str(), &res); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] imp->Load failed: path={}, file={}, ret={}",
						 __FUNCTION__, path, (file.empty() ? std::string_view{"(null)"} : file), static_cast<long long>(r));
			LW_DELETE(imp);
			return LW_RET_FAILED;
		}

		_obj = imp;
		SetFileName(file);
		RegisterSceneMgr(_res_mgr->GetSysGraphics()->GetSceneMgr());
		return LW_RET_OK;
	}

	LW_RESULT lwItem::Destroy() {
		if (_obj == nullptr) {
			return LW_RET_FAILED;
		}

		_obj->Destroy();
		LW_DELETE(_obj);
		_obj = nullptr;
		return LW_RET_OK;
	}

	LW_RESULT lwItem::Copy(const lwItem* src_obj) {
		_file_name = src_obj->_file_name;
		src_obj->_obj->Clone(&_obj);

		_mat_base = src_obj->_mat_base;
		_link_ctrl = src_obj->_link_ctrl;
		_linkItemId = src_obj->_linkItemId;
		_linkParentId = src_obj->_linkParentId;

		return LW_RET_OK;
	}

	LW_RESULT lwItem::Clone(lwItem** ret_obj) {
		lwItem* o = nullptr;
		_res_mgr->CreateItem(&o);
		o->Copy(this);

		*ret_obj = o;
		return LW_RET_OK;
	}

	LW_RESULT lwItem::Update() {
		if (_obj == nullptr) {
			return LW_RET_OK;
		}

		if (_link_ctrl) {
			lwMatrix44 mat_parent;
			if (LW_RESULT r = _link_ctrl->GetLinkCtrlMatrix(&mat_parent, _linkParentId); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] _link_ctrl->GetLinkCtrlMatrix failed: link_parent_id={}, ret={}",
							 __FUNCTION__, _linkParentId, static_cast<long long>(r));
				return LW_RET_FAILED;
			}

			lwMatrix44 mat_dummy;
			if (LW_RESULT r = GetDummyMatrix(&mat_dummy, _linkItemId); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] GetDummyMatrix failed: link_item_id={}, ret={}",
							 __FUNCTION__, _linkItemId, static_cast<long long>(r));
				return LW_RET_FAILED;
			}

			lwMatrix44InverseNoScaleFactor(&mat_dummy, &mat_dummy);
			lwMatrix44Multiply(&_mat_base, &mat_dummy, &mat_parent);
		}

		_obj->SetMatrixParent(&_mat_base);

		if (LW_RESULT r = _obj->Update(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] _obj->Update failed: ret={}",
						 __FUNCTION__, static_cast<long long>(r));
			return LW_RET_FAILED;
		}

		return LW_RET_OK;
	}

	LW_RESULT lwItem::Render() {
		if (_state_ctrl.GetState(STATE_VISIBLE) == 0) {
			return LW_RET_OK;
		}
		if (_obj == nullptr) {
			return LW_RET_OK;
		}

		if (_scene_mgr && _obj->GetState(STATE_TRANSPARENT)) {
			if (LW_RESULT r = _scene_mgr->AddTransparentPrimitive(_obj); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] AddTransparentPrimitive failed: ret={}",
							 __FUNCTION__, static_cast<long long>(r));
				return LW_RET_FAILED;
			}
			return LW_RET_OK;
		}

		if (LW_RESULT r = _obj->Render(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] _obj->Render failed: ret={}",
						 __FUNCTION__, static_cast<long long>(r));
			return LW_RET_FAILED;
		}
		return LW_RET_OK;
	}

	void lwItem::SetMaterial(const lwMaterial* mtl) {
		if (_obj) {
			_obj->SetMaterial(mtl);
		}
	}

	LW_RESULT lwItem::HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) {
		return _obj ? _obj->HitTest(info, org, ray) : LW_RET_FAILED;
	}

	void lwItem::ShowBoundingObject(int show) {
		if (_obj && _obj->GetHelperObject()) {
			_obj->GetHelperObject()->SetVisible(show);
		}
	}

	const lwMatrix44* lwItem::GetObjDummyMatrix(DWORD id) {
		if (_obj == nullptr) {
			return nullptr;
		}
		IHelperObject* h = _obj->GetHelperObject();
		if (h == nullptr) {
			return nullptr;
		}
		IHelperDummy* hd = h->GetHelperDummy();
		if (hd == nullptr) {
			return nullptr;
		}
		HelperDummyInfo* info = hd->GetDataInfoWithID(id);
		if (info == nullptr) {
			return nullptr;
		}
		return &info->mat;
	}

	const lwMatrix44* lwItem::GetObjBoneDummyMatrix(DWORD id) {
		lwIAnimCtrlAgent* anim_agent = _obj->GetAnimAgent();

		lwAnimCtrlObjTypeInfo type_info;
		type_info.type = ANIM_CTRL_TYPE_BONE;
		type_info.data[0] = LW_INVALID_INDEX;
		type_info.data[1] = LW_INVALID_INDEX;

		lwIAnimCtrlObj* aca_bone = static_cast<lwIAnimCtrlObj*>(anim_agent->GetAnimCtrlObj(&type_info));
		lwIAnimCtrlBone* c = static_cast<lwIAnimCtrlBone*>(aca_bone->GetAnimCtrl());
		if (c == nullptr) {
			return nullptr;
		}

		const DWORD rtmat_num = c->GetDummyNum();
		lwIndexMatrix44* rtmat_seq = c->GetDummyRTMSeq();

		for (DWORD i = 0; i < rtmat_num; i++) {
			if (rtmat_seq[i].id == id) {
				return &rtmat_seq[i].mat;
			}
		}

		return nullptr;
	}

	LW_RESULT lwItem::GetObjDummyRunTimeMatrix(lwMatrix44* mat, DWORD id) {
		if (_obj == nullptr) {
			return LW_RET_FAILED;
		}

		lwMatrix44 mat_dummy;
		if (LW_RESULT r = GetDummyMatrix(&mat_dummy, id); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] GetDummyMatrix failed: id={}, ret={}",
						 __FUNCTION__, id, static_cast<long long>(r));
			return LW_RET_FAILED;
		}

		lwMatrix44Multiply(mat, &mat_dummy, _obj->GetMatrixGlobal());
		return LW_RET_OK;
	}


	LW_RESULT lwItem::GetDummyMatrix(lwMatrix44* mat, DWORD id) {
		const lwMatrix44* mat_dummy = nullptr;

		lwIAnimCtrlAgent* anim_agent = _obj->GetAnimAgent();
		bool useObjDummy = (anim_agent == nullptr);

		if (!useObjDummy) {
			lwAnimCtrlObjTypeInfo type_info;
			type_info.type = ANIM_CTRL_TYPE_BONE;
			type_info.data[0] = LW_INVALID_INDEX;
			type_info.data[1] = LW_INVALID_INDEX;

			lwIAnimCtrlObj* ctrl_obj = anim_agent->GetAnimCtrlObj(&type_info);
			if (ctrl_obj == nullptr || ctrl_obj->GetAnimCtrl() == nullptr) {
				useObjDummy = true;
			}
			else {
				mat_dummy = GetObjBoneDummyMatrix(id);
			}
		}

		if (useObjDummy) {
			mat_dummy = GetObjDummyMatrix(id);
		}

		if (mat_dummy == nullptr) {
			return LW_RET_FAILED;
		}

		if (mat) {
			*mat = *mat_dummy;
		}
		return LW_RET_OK;
	}

	LW_RESULT lwItem::SetLinkCtrl(lwLinkCtrl* ctrl, DWORD link_parent_id, DWORD link_item_id) {
		if (LW_RESULT r = GetDummyMatrix(NULL, link_item_id); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] GetDummyMatrix failed: link_item_id={}, ret={}",
						 __FUNCTION__, link_item_id, static_cast<long long>(r));
			return LW_RET_FAILED;
		}

		_link_ctrl = ctrl;
		_linkParentId = link_parent_id;
		_linkItemId = link_item_id;

		return LW_RET_OK;
	}

	LW_RESULT lwItem::ClearLinkCtrl() {
		_link_ctrl = nullptr;
		_linkParentId = LW_INVALID_INDEX;
		_linkItemId = LW_INVALID_INDEX;
		return LW_RET_OK;
	}

	LW_RESULT lwItem::PlayDefaultAnimation(float velocity) {
		return lwPrimitivePlayDefaultAnimation(_obj, velocity);
	}

	void lwItem::SetOpacity(float opacity) {
		if (_obj) {
			_opacity = opacity;
			_obj->SetOpacity(opacity);
		}
	}

	LW_RESULT lwItem::SetTextureLOD(DWORD level) {
		if (_obj) {
			_obj->SetTextureLOD(level);
		}
		return LW_RET_OK;
	}

} // namespace Corsairs::Engine::Render
