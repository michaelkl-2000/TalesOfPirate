//
#pragma once

#include "lwHeader.h"
#include "lwMath.h"
#include "lwInterfaceExt.h"
#include "lwClassDecl.h"
#include "lwITypes.h"
#include "lwPrimitive.h"
#include "lwLinkCtrl.h"


namespace Corsairs::Engine::Render {
	// Был интерфейс lwIModel с единственным реализатором — удалён 2026-05-13.
	// lwModel теперь standalone, наследует только lwLinkCtrl ради виртуального
	// GetLinkCtrlMatrix.
	class lwModel : public lwLinkCtrl {
	private:
		IResourceMgr* _res_mgr;
		lwISceneMgr* _scene_mgr;

		lwStateCtrl _state_ctrl;
		std::string _file_name;
		lwIPrimitive* _obj_seq[LW_MAX_MODEL_GEOM_OBJ_NUM];
		DWORD _obj_num;
		lwMatrix44 _mat_base;
		DWORD _id;
		DWORD _model_id;

		IHelperObject* _helper_object;
		float _opacity;

	public:
		lwModel(IResourceMgr* res_mgr);
		~lwModel();

		// Был частью lwInterface — оставлен ради legacy-вызовов obj->Release().
		void Release() {
			delete this;
		}

		DWORD GetModelID() const {
			return _model_id;
		}

		void SetFileName(std::string_view file) {
			_file_name = file;
		}

		const std::string& GetFileName() {
			return _file_name;
		}

		LW_RESULT Clone(lwModel** ret_obj);
		LW_RESULT Copy(const lwModel* src_obj);

		lwMatrix44* GetMatrix() {
			return &_mat_base;
		}

		void SetMatrix(const lwMatrix44* mat) {
			_mat_base = *mat;
		}

		LW_RESULT Load(lwIModelObjInfo* info);
		LW_RESULT Load(std::string_view file, DWORD model_id = LW_INVALID_INDEX);
		LW_RESULT Update();
		LW_RESULT Render();
		LW_RESULT RenderPrimitive(DWORD id);
		LW_RESULT RenderHelperObject();
		LW_RESULT Destroy();

		void SetMaterial(const lwMaterial* mtl);
		void SetOpacity(float opacity);


		LW_RESULT HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray);
		LW_RESULT HitTestPrimitiveHelperMesh(lwPickInfo* info, const lwVector3* org, const lwVector3* ray,
											 std::string_view type_name);
		LW_RESULT HitTestPrimitiveHelperBox(lwPickInfo* info, const lwVector3* org, const lwVector3* ray,
											std::string_view type_name);
		LW_RESULT HitTest(lwPickInfo* info, const lwVector3* org, const lwVector3* ray);
		LW_RESULT HitTestHelperMesh(lwPickInfo* info, const lwVector3* org, const lwVector3* ray,
									std::string_view type_name);
		LW_RESULT HitTestHelperBox(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, std::string_view type_name);

		LW_RESULT PlayDefaultAnimation(float velocity = 1.0f);

		LW_RESULT SortPrimitiveObj();


		void ShowHelperObject(int show);
		void ShowHelperMesh(int show);
		void ShowHelperBox(int show);
		void ShowBoundingObject(int show);

		IHelperObject* GetHelperObject() {
			return _helper_object;
		}

		DWORD GetPrimitiveNum() const {
			return _obj_num;
		}

		lwIPrimitive* GetPrimitive(DWORD id) {
			return _obj_seq[id];
		}

		void SetObjState(DWORD state, BYTE value) {
			return _state_ctrl.SetState(state, value);
		}

		DWORD GetObjState(DWORD state) const {
			return _state_ctrl.GetState(state);
		}

		LW_RESULT RegisterSceneMgr(lwISceneMgr* scene_mgr) {
			_scene_mgr = scene_mgr;
			return LW_RET_OK;
		}

		LW_RESULT SetItemLink(const lwItemLinkInfo* info);
		LW_RESULT ClearItemLink(lwItem* obj);

		// link ctrl method
		virtual LW_RESULT GetLinkCtrlMatrix(lwMatrix44* mat, DWORD link_id);

		LW_RESULT SetTextureLOD(DWORD level);

		float GetOpacity() {
			return _opacity;
		}

		LW_RESULT CullPrimitive();
		DWORD GetCullingPrimitiveNum();
		LW_RESULT ExtractModelInfo(lwIModelObjInfo* out_info);
	};


} // namespace Corsairs::Engine::Render
