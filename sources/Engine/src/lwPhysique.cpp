//
#include "stdafx.h"


#include "lwPhysique.h"
#include "lwSystem.h"
#include "lwSysGraphics.h"
#include "lwPathInfo.h"
#include "lwAnimCtrl.h"
#include "lwRenderImp.h"
#include "lwResourceMgr.h"
#include "lwD3D.h"
#include "lwItem.h"
#include "lwExpObj.h"

#include "AssetLoaders.h"
#include "BoneAnimCache.h"
#include "GeomObjCache.h"

#include <format>

using namespace std;
namespace Corsairs::Engine::Render {
	// lwPhysique

	// begin construct
	lwPhysique::lwPhysique(lwIResourceMgr* res_mgr)
		: _res_mgr(res_mgr), _scene_mgr(0), _anim_agent(0) {
		_file_name[0] = '\0';
		lwMatrix44Identity(&_mat_base);
		memset(_obj_seq, 0, sizeof(lwIPrimitive*) * LW_MAX_SUBSKIN_NUM);
		_opacity = 1.0f;
		_start = false;
		_end = false;
	}

	// end construct

	lwPhysique::lwPhysique()
		: _anim_agent(0) {
		_res_mgr = lwSysGraphics::GetActiveIGraphicsSystem()->GetResourceMgr();

		_file_name[0] = '\0';
		lwMatrix44Identity(&_mat_base);
		memset(_obj_seq, 0, sizeof(lwIPrimitive*) * LW_MAX_SUBSKIN_NUM);
	}

	lwPhysique::~lwPhysique() {
		Destroy();
	}

	LW_RESULT lwPhysique::GetLinkCtrlMatrix(lwMatrix44* mat, DWORD link_id) {
		lwAnimCtrlObjTypeInfo type_info;
		type_info.type = ANIM_CTRL_TYPE_BONE;
		type_info.data[0] = LW_INVALID_INDEX;
		type_info.data[1] = LW_INVALID_INDEX;

		auto* bone_ctrl = static_cast<lwIAnimCtrlObjBone*>(_anim_agent->GetAnimCtrlObj(&type_info));
		if (bone_ctrl == nullptr) {
			return LW_RET_FAILED;
		}

		lwMatrix44* rtm = bone_ctrl->GetDummyRTM(link_id);
		if (rtm == nullptr) {
			return LW_RET_FAILED;
		}

		lwMatrix44Multiply(mat, rtm, &_mat_base);
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::DestroyPrimitive(DWORD part_id) {
		if (_obj_seq[part_id] == nullptr) {
			return LW_RET_OK;
		}

		if (LW_RESULT r = _obj_seq[part_id]->Destroy(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] lwIPrimitive::Destroy failed: part_id={}, ret={}",
						 __FUNCTION__, part_id, static_cast<long long>(r));
			return LW_RET_FAILED;
		}

		_obj_seq[part_id]->Release();
		_obj_seq[part_id] = nullptr;
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::Destroy() {
		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			if (LW_RESULT r = DestroyPrimitive(i); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] DestroyPrimitive failed: i={}, ret={}",
							 __FUNCTION__, i, static_cast<long long>(r));
				assert(0 && "call DestroyPrimitive in lwPhysique::Destroy error");
			}
		}

		LW_SAFE_RELEASE(_anim_agent);
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::LoadBone(std::string_view file) {
		const auto fileSv = file.empty() ? std::string_view{"(null)"} : file;

		lwAnimCtrlObjTypeInfo type_info;
		type_info.type = ANIM_CTRL_TYPE_BONE;
		type_info.data[0] = LW_INVALID_INDEX;
		type_info.data[1] = LW_INVALID_INDEX;

		// check existing bone
		if (_anim_agent && _anim_agent->GetAnimCtrlObj(&type_info)) {
			auto* existing = static_cast<lwIAnimCtrlObjBone*>(_anim_agent->RemoveAnimCtrlObj(&type_info));
			LW_SAFE_RELEASE(existing);
		}

		const bool ownsAgent = (_anim_agent == nullptr);
		if (ownsAgent) {
			if (LW_RESULT r = _res_mgr->CreateAnimCtrlAgent(&_anim_agent); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] CreateAnimCtrlAgent failed: file={}, ret={}",
							 __FUNCTION__, fileSv, static_cast<long long>(r));
				return LW_RET_FAILED;
			}
		}

		// Локальный helper: освобождает agent при неудаче, если он только что
		// был создан этим вызовом.
		auto rollback = [this, ownsAgent]() {
			if (ownsAgent) {
				LW_SAFE_RELEASE(_anim_agent);
			}
		};

		lwIAnimCtrlObjBone* bone_ctrl = nullptr;
		if (LW_RESULT r = _res_mgr->CreateAnimCtrlObj(reinterpret_cast<lwIAnimCtrlObj**>(&bone_ctrl), ANIM_CTRL_TYPE_BONE);
			LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] CreateAnimCtrlObj(BONE) failed: file={}, ret={}",
						 __FUNCTION__, fileSv, static_cast<long long>(r));
			rollback();
			return LW_RET_FAILED;
		}

		if (LW_RESULT r = _anim_agent->AddAnimCtrlObj(bone_ctrl); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] AddAnimCtrlObj failed: file={}, ret={}",
						 __FUNCTION__, fileSv, static_cast<long long>(r));
			rollback();
			return LW_RET_FAILED;
		}

		lwISysGraphics* sys_graphics = _res_mgr->GetSysGraphics();
		lwISystem* sys = sys_graphics->GetSystem();
		lwIPathInfo* path_info = nullptr;
		sys->GetInterface(reinterpret_cast<LW_VOID**>(&path_info), LW_GUID_PATHINFO);
		const std::string path = std::format("{}{}", path_info->GetPath(PATH_TYPE_ANIMATION), file);

		lwResFileAnimData res;
		res.obj_id = 0;
		res.res_type = RES_FILE_TYPE_GENERIC;
		res.anim_type = ANIM_CTRL_TYPE_BONE;
		std::memset(res.file_name, 0, sizeof(res.file_name));
		std::memcpy(res.file_name, path.data(),
					std::min<std::size_t>(path.size(), sizeof(res.file_name) - 1));

		DWORD ret_id = 0;
		if (LW_SUCCEEDED(_res_mgr->QueryAnimCtrl(&ret_id, &res))) {
			lwIAnimCtrlBone* anim_ctrl = nullptr;
			if (LW_RESULT r = _res_mgr->GetAnimCtrl(reinterpret_cast<lwIAnimCtrl**>(&anim_ctrl), ret_id); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] GetAnimCtrl failed: file={}, ret_id={}, ret={}",
							 __FUNCTION__, fileSv, ret_id, static_cast<long long>(r));
				rollback();
				return LW_RET_FAILED;
			}
			_res_mgr->AddRefAnimCtrl(anim_ctrl, 1);
			bone_ctrl->AttachAnimCtrl(anim_ctrl);
		}
		else {
			lwIAnimDataBone* i_data = Corsairs::Engine::Render::BoneAnimCache::Instance().GetOrLoad(path);
			if (i_data == nullptr) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] BoneAnimCache::GetOrLoad failed: file={}, path={}",
							 __FUNCTION__, fileSv, path);
				rollback();
				return LW_RET_FAILED;
			}

			lwIAnimCtrlBone* ctrl_bone = nullptr;
			if (LW_RESULT r = _res_mgr->CreateAnimCtrl(reinterpret_cast<lwIAnimCtrl**>(&ctrl_bone), ANIM_CTRL_TYPE_BONE);
				LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] CreateAnimCtrl(BONE) failed: file={}, path={}, ret={}",
							 __FUNCTION__, fileSv, path, static_cast<long long>(r));
				rollback();
				return LW_RET_FAILED;
			}

			if (LW_RESULT r = ctrl_bone->LoadData(i_data); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] ctrl_bone->LoadData failed: file={}, path={}, ret={}",
							 __FUNCTION__, fileSv, path, static_cast<long long>(r));
				rollback();
				return LW_RET_FAILED;
			}

			ctrl_bone->SetResFile(&res);
			bone_ctrl->AttachAnimCtrl(ctrl_bone);
			bone_ctrl->SetTypeInfo(&type_info);
		}

		RegisterSceneMgr(_res_mgr->GetSysGraphics()->GetSceneMgr());
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::LoadPrimitive(DWORD part_id, lwGeomObjInfo* geom_info) {
		if (part_id < 0 || part_id >= LW_MAX_SUBSKIN_NUM)
			return ERR_INVALID_PARAM;


		lwISysGraphics* sys_graphics = _res_mgr->GetSysGraphics();
		lwISystem* sys = sys_graphics->GetSystem();
		lwIPathInfo* path_info = sys->GetPathInfo();
		lwIOptionMgr* opt_mgr = sys->GetOptionMgr();
		BYTE create_helper_primitive = opt_mgr->GetByteFlag(OPTION_FLAG_CREATEHELPERPRIMITIVE);

		const std::string& tex_path = path_info->GetPath(PATH_TYPE_TEXTURE_CHARACTER);

		lwGeomObjInfo* info = geom_info;

		// query mesh pool
		lwIPrimitive* imp;
		lwIMeshAgent* mesh_agent;
		lwIMtlTexAgent* mtltex_agent;

		_res_mgr->CreatePrimitive(&imp);
		_res_mgr->CreateMeshAgent(&mesh_agent);


		// mesh
		if (LW_RESULT r = mesh_agent->LoadMesh(&info->mesh); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] mesh_agent->LoadMesh failed: part_id={}, ret={}",
						 __FUNCTION__, part_id, static_cast<long long>(r));
		}

		imp->SetMeshAgent(mesh_agent);

		// material and texture
		for (DWORD j = 0; j < info->mtl_num; j++) {
			_res_mgr->CreateMtlTexAgent(&mtltex_agent);

			lwMtlTexInfo* mti = &info->mtl_seq[j];

			mtltex_agent->SetTranspType(mti->transp_type);
			mtltex_agent->SetMaterial(&mti->mtl);

			mtltex_agent->GetMtlRenderStateSet()->Load(mti->rs_set, LW_MTL_RS_NUM);

			mtltex_agent->SetOpacity(mti->opacity);

			for (DWORD i = 0; i < LW_MAX_TEXTURESTAGE_NUM; i++) {
				lwTexInfo* tex_info = &mti->tex_seq[i];

				if (tex_info->stage == LW_INVALID_INDEX)
					break;

				mtltex_agent->LoadTextureStage(tex_info, tex_path);
			}

			imp->SetMtlTexAgent(j, mtltex_agent);
		}
		// end

		// =======================
		// create render ctrl agent
		imp->LoadRenderCtrl(&info->rcci);
		// state ctrl
		imp->SetState(&info->state_ctrl);

		// create anim ctrl agent and anim ctrl obj bone
		lwIAnimCtrlObj* ctrl_obj = NULL;
		lwIAnimCtrlAgent* anim_agent = NULL;
		_res_mgr->CreateAnimCtrlObj(&ctrl_obj, ANIM_CTRL_TYPE_BONE);

		if ((anim_agent = imp->GetAnimAgent()) == 0) {
			_res_mgr->CreateAnimCtrlAgent(&anim_agent);
			imp->SetAnimCtrlAgent(anim_agent);
		}

		anim_agent->AddAnimCtrlObj(ctrl_obj);


		imp->SetID(info->id);
		imp->SetParentID(info->parent_id);

		// local matrix
		imp->SetMatrixLocal(&info->mat_local);

		if (info->helper_size > 0) {
			lwIHelperObject* h;
			_res_mgr->CreateHelperObject(&h);
			if (LW_RESULT r = h->LoadHelperInfo(&info->helper_data, create_helper_primitive); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] LoadHelperInfo failed: part_id={}, helper_size={}, ret={}",
							 __FUNCTION__, part_id, info->helper_size, static_cast<long long>(r));
				LG_MSGBOX("LoadHelperInfo error");
			}
			imp->SetHelperObject(h);
		}

		// ObjImpLoadMesh
		if (info->anim_size > 0) {
			if (LW_RESULT r = imp->LoadAnimData(&info->anim_data, tex_path, 0); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] imp->LoadAnimData failed: part_id={}, tex_path={}, ret={}",
							 __FUNCTION__, part_id, (tex_path.empty() ? std::string_view{"(null)"} : tex_path), static_cast<long long>(r));
			}
		}

		LW_SAFE_RELEASE(_obj_seq[ part_id ]);

		_obj_seq[part_id] = imp;
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::LoadPrimitive(DWORD part_id, std::string_view file) {
		if (part_id < 0 || part_id >= LW_MAX_SUBSKIN_NUM)
			return ERR_INVALID_PARAM;


		lwISysGraphics* sys_graphics = _res_mgr->GetSysGraphics();
		lwISystem* sys = sys_graphics->GetSystem();
		lwIPathInfo* path_info = sys->GetPathInfo();
		lwIOptionMgr* opt_mgr = sys->GetOptionMgr();
		BYTE create_helper_primitive = opt_mgr->GetByteFlag(OPTION_FLAG_CREATEHELPERPRIMITIVE);

		const std::string& tex_path = path_info->GetPath(PATH_TYPE_TEXTURE_CHARACTER);

		// query mesh pool
		DWORD ret_id;

		const std::string path = std::format("{}{}", path_info->GetPath(PATH_TYPE_MODEL_CHARACTER), file);

		auto copyToFixedBuf = [](char* dst, std::size_t cap, std::string_view src) {
			std::memset(dst, 0, cap);
			std::memcpy(dst, src.data(), std::min<std::size_t>(src.size(), cap - 1));
		};

		lwResFile res;
		res.obj_id = 0;
		res.res_type = RES_FILE_TYPE_GEOMETRY;
		copyToFixedBuf(res.file_name, sizeof(res.file_name), path);

		// info живёт в общем GeomObjCache, shared_ptr держит запись до конца
		// LoadPrimitive (на самом деле — навсегда, т.к. cache strong-навсегда).
		auto info = Corsairs::Engine::Render::GeomObjCache::Instance().GetOrLoad(path);
		if (!info) {
			return LW_RET_FAILED;
		}
		lwGeomObjInfo* pInfo = info.get();

		lwIPrimitive* imp;
		lwIMeshAgent* mesh_agent;
		lwIMtlTexAgent* mtltex_agent;
		lwIMesh* mesh;
		lwITex* tex;

		_res_mgr->CreatePrimitive(&imp);
		_res_mgr->CreateMeshAgent(&mesh_agent);


		// mesh
		lwResFileMesh rfm;
		rfm.obj_id = 0;
		rfm.res_type = RES_FILE_TYPE_GEOMETRY;
		copyToFixedBuf(rfm.file_name, sizeof(rfm.file_name), path);

		if (LW_SUCCEEDED(_res_mgr->QueryMesh( &ret_id, &rfm ))) {
			_res_mgr->GetMesh(&mesh, ret_id);
			_res_mgr->AddRefMesh(mesh, 1);

			mesh_agent->SetMesh(mesh);
		}
		else {
			if (LW_RESULT r = mesh_agent->LoadMesh(&pInfo->mesh/*&info.mesh*/); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] mesh_agent->LoadMesh (else branch) failed: part_id={}, ret={}",
							 __FUNCTION__, part_id, static_cast<long long>(r));
			}
		}

		mesh_agent->GetMesh()->SetResFile(&rfm);

		imp->SetMeshAgent(mesh_agent);

		// material and texture
		DWORD tex_id;

		for (DWORD j = 0; j < pInfo->mtl_num; j++) {
			_res_mgr->CreateMtlTexAgent(&mtltex_agent);

			lwMtlTexInfo* mti = &pInfo->mtl_seq[j];

			mtltex_agent->SetTranspType(mti->transp_type);
			mtltex_agent->SetMaterial(&mti->mtl);

			mtltex_agent->GetMtlRenderStateSet()->Load(mti->rs_set, LW_MTL_RS_NUM);

			mtltex_agent->SetOpacity(mti->opacity);

			for (DWORD i = 0; i < LW_MAX_TEXTURESTAGE_NUM; i++) {
				lwTexInfo* tex_info = &mti->tex_seq[i];

				if (tex_info->stage == LW_INVALID_INDEX)
					break;

				const std::string texPath = std::format("{}{}", tex_path, tex_info->file_name);

				if (LW_SUCCEEDED(_res_mgr->QueryTex( &tex_id, texPath.c_str() ))) {
					_res_mgr->GetTex(&tex, tex_id);
					_res_mgr->AddRefTex(tex, 1);

					lwITex* ret_tex = 0;
					mtltex_agent->SetTex(i, tex, &ret_tex);
					if (ret_tex) {
						if (ret_tex->GetRegisterID() != LW_INVALID_INDEX)
							_res_mgr->UnregisterTex(ret_tex);
						else
							ret_tex->Release();
					}
				}
				else {
					{
						tex_info->level = D3DX_DEFAULT;
					}

					mtltex_agent->LoadTextureStage(tex_info, tex_path);
				}
			}

			imp->SetMtlTexAgent(j, mtltex_agent);
		}
		// end

		// =======================
		// create render ctrl agent
		imp->LoadRenderCtrl(&pInfo->rcci);
		// state ctrl
		imp->SetState(&pInfo->state_ctrl);

		// create anim ctrl agent and anim ctrl obj bone
		lwIAnimCtrlObj* ctrl_obj = NULL;
		lwIAnimCtrlAgent* anim_agent = NULL;
		_res_mgr->CreateAnimCtrlObj(&ctrl_obj, ANIM_CTRL_TYPE_BONE);

		if ((anim_agent = imp->GetAnimAgent()) == 0) {
			_res_mgr->CreateAnimCtrlAgent(&anim_agent);
			imp->SetAnimCtrlAgent(anim_agent);
		}

		anim_agent->AddAnimCtrlObj(ctrl_obj);


		imp->SetID(pInfo->id);
		imp->SetParentID(pInfo->parent_id);

		// local matrix
		imp->SetMatrixLocal(&pInfo->mat_local);

		if (pInfo->helper_size > 0) {
			lwIHelperObject* h;
			_res_mgr->CreateHelperObject(&h);
			if (LW_RESULT r = h->LoadHelperInfo(&pInfo->helper_data, create_helper_primitive); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] LoadHelperInfo failed: part_id={}, file={}, helper_size={}, ret={}",
							 __FUNCTION__, part_id, (file.empty() ? std::string_view{"(null)"} : file), pInfo->helper_size,
							 static_cast<long long>(r));
				LG_MSGBOX("LoadHelperInfo error");
			}
			imp->SetHelperObject(h);
		}

		// ObjImpLoadMesh
		if (pInfo->anim_size > 0) {
			if (LW_RESULT r = imp->LoadAnimData(&pInfo->anim_data, tex_path, &res); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] imp->LoadAnimData (with res) failed: part_id={}, tex_path={}, ret={}",
							 __FUNCTION__, part_id, (tex_path.empty() ? std::string_view{"(null)"} : tex_path), static_cast<long long>(r));
			}
		}

		LW_SAFE_RELEASE(_obj_seq[ part_id ]);

		_obj_seq[part_id] = imp;
		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::Update() {
		// update physique bone animation
		if (_anim_agent) {
			if (LW_RESULT r = _anim_agent->Update(); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] _anim_agent->Update failed: ret={}",
							 __FUNCTION__, static_cast<long long>(r));
				return LW_RET_FAILED;
			}

			lwAnimCtrlObjTypeInfo type_info;
			type_info.type = ANIM_CTRL_TYPE_BONE;
			type_info.data[0] = LW_INVALID_INDEX;
			type_info.data[1] = LW_INVALID_INDEX;

			auto* ctrl_obj = static_cast<lwIAnimCtrlObjBone*>(_anim_agent->GetAnimCtrlObj(&type_info));

			if (LW_RESULT r = ctrl_obj->UpdateObject(ctrl_obj, nullptr); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] ctrl_obj->UpdateObject(self) failed: ret={}",
							 __FUNCTION__, static_cast<long long>(r));
				return LW_RET_FAILED;
			}

			for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
				lwIPrimitive* pri = _obj_seq[i];
				if (pri == nullptr) {
					continue;
				}

				auto* pri_ctrl = static_cast<lwIAnimCtrlObjBone*>(pri->GetAnimAgent()->GetAnimCtrlObj(&type_info));
				if (pri_ctrl == nullptr) {
					LG_MSGBOX("crash!!!, call jack");
					__debugbreak();
				}

				if (LW_RESULT r = ctrl_obj->UpdateObject(pri_ctrl, pri->GetMeshAgent()->GetMesh()); LW_FAILED(r)) {
					ToLogService("errors", LogLevel::Error,
								 "[{}] ctrl_obj->UpdateObject(pri) failed: subskin={}, ret={}",
								 __FUNCTION__, i, static_cast<long long>(r));
					return LW_RET_FAILED;
				}
			}
		}

		// update object
		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			lwIPrimitive* imp = _obj_seq[i];
			if (imp == nullptr) {
				continue;
			}

			lwMatrix44 mat(_mat_base);
			lwIPrimitive* pp = imp;
			while (pp->GetParentID() != LW_INVALID_INDEX) {
				if (_obj_seq[pp->GetParentID()]) {
					lwMatrix44Multiply(&mat, _obj_seq[pp->GetParentID()]->GetMatrixLocal(), &mat);
					pp = _obj_seq[pp->GetParentID()];
				}
				else {
					break;
				}
			}

			imp->SetMatrixParent(&mat);

			if (LW_RESULT r = imp->Update(); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] imp->Update failed: subskin={}, ret={}",
							 __FUNCTION__, i, static_cast<long long>(r));
			}
		}

		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::Render() {
		if (_state_ctrl.GetState(STATE_VISIBLE) == 0) {
			return LW_RET_OK;
		}

		IDirect3DDeviceX* device = _res_mgr->GetDeviceObject()->GetDevice();
		device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		device->SetTexture(1, 0);

		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			lwIPrimitive* p = _obj_seq[i];
			if (p == nullptr) {
				continue;
			}

			if (_scene_mgr && p->GetState(STATE_TRANSPARENT)) {
				if (LW_RESULT r = _scene_mgr->AddTransparentPrimitive(p); LW_FAILED(r)) {
					ToLogService("errors", LogLevel::Error,
								 "[{}] AddTransparentPrimitive failed: subskin={}, ret={}",
								 __FUNCTION__, i, static_cast<long long>(r));
					return LW_RET_FAILED;
				}
				continue;
			}

			const bool hasFilter = mIndexColourFilterList.find(i) != mIndexColourFilterList.end();
			if (hasFilter) {
				static IDirect3DTextureX* texture = nullptr;
				if (!texture) {
					texture = _res_mgr->getMonochromaticTexture(mIndexColourFilterList[i].first,
																mIndexColourFilterList[i].second);
				}
				device->SetTexture(1, texture);
				device->SetTextureStageState(1, D3DTSS_COLOROP, mIndexTextureOPList[i]);
				device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
				device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
				device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				device->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
			}

			if (LW_RESULT r = p->Render(); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] p->Render failed: subskin={}, ret={}",
							 __FUNCTION__, i, static_cast<long long>(r));
				return LW_RET_FAILED;
			}

			if (hasFilter) {
				device->SetTexture(1, 0);
				device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
				device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTOP_DISABLE);
				device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTOP_DISABLE);
				device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTOP_DISABLE);
				device->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTOP_DISABLE);
				device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
		}

		return LW_RET_OK;
	}

	LW_RESULT lwPhysique::HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) {
		lwIPrimitive* imp_generic;

		lwPickInfo x, xx;
		DWORD obj_id = LW_INVALID_INDEX;

		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			if ((imp_generic = _obj_seq[i]) == 0)
				continue;

			if (LW_SUCCEEDED(imp_generic->HitTest( &x, org, ray ))) {
				if ((obj_id == LW_INVALID_INDEX) || (xx.dis > x.dis)) {
					xx = x;
					obj_id = i;
				}
			}
		}

		if (obj_id == LW_INVALID_INDEX) {
			return LW_RET_FAILED;
		}
		else {
			*info = xx;
			info->obj_id = obj_id;

			return LW_RET_OK;
		}
	}

	LW_RESULT lwPhysique::HitTestPhysique(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) {
		return LW_RET_FAILED;
	}

	void lwPhysique::ShowHelperObject(int show) {
		lwIHelperObject* helper_obj;

		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			if (_obj_seq[i] == 0)
				continue;

			if ((helper_obj = _obj_seq[i]->GetHelperObject()) != 0) {
				helper_obj->SetVisible(show);
			}
		}
	}

	void lwPhysique::ShowBoundingObjectPhysique(int show) {
	}

	LW_RESULT lwPhysique::SetItemLink(const lwItemLinkInfo* info) {
		if (_anim_agent == nullptr) {
			return LW_RET_FAILED;
		}

		lwAnimCtrlObjTypeInfo type_info;
		type_info.type = ANIM_CTRL_TYPE_BONE;
		type_info.data[0] = LW_INVALID_INDEX;
		type_info.data[1] = LW_INVALID_INDEX;

		auto* ctrl_obj = static_cast<lwIAnimCtrlObjBone*>(_anim_agent->GetAnimCtrlObj(&type_info));
		if (ctrl_obj == nullptr) {
			return LW_RET_FAILED;
		}

		auto* ctrl_bone = static_cast<lwIAnimCtrlBone*>(ctrl_obj->GetAnimCtrl());
		if (ctrl_bone == nullptr) {
			return LW_RET_FAILED;
		}

		if (ctrl_bone->GetDummyRTM(info->link_parent_id) == nullptr) {
			return LW_RET_FAILED;
		}

		if (info->obj->GetPrimitive() == nullptr) {
			return LW_RET_FAILED;
		}

		return info->obj->SetLinkCtrl(this, info->link_parent_id, info->link_item_id);
	}

	LW_RESULT lwPhysique::ClearItemLink(lwItem* obj) {
		return obj->ClearLinkCtrl();
	}

	void lwPhysique::SetOpacity(float opacity) {
		_opacity = opacity;

		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			if (_obj_seq[i] == 0)
				continue;

			_obj_seq[i]->SetOpacity(opacity);
		}
	}

	LW_RESULT lwPhysique::SetTextureLOD(DWORD level) {
		for (DWORD i = 0; i < LW_MAX_SUBSKIN_NUM; i++) {
			if (_obj_seq[i] == 0)
				continue;

			_obj_seq[i]->SetTextureLOD(level);
		}

		return LW_RET_OK;
	}

} // namespace Corsairs::Engine::Render
