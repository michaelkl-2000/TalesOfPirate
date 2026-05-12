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
	// Был интерфейс lwIItem с единственным реализатором — удалён 2026-05-12.
	// lwItem теперь standalone, без виртуальной диспетчеризации.
	class lwItem {
	private:
		lwIResourceMgr* _res_mgr;
		lwISceneMgr* _scene_mgr;

		lwLinkCtrl* _link_ctrl;
		DWORD _linkParentId;
		DWORD _linkItemId;

		lwStateCtrl _state_ctrl;
		std::string _file_name;
		lwIPrimitive* _obj;
		lwMatrix44 _mat_base;
		float _opacity;

	public:
		lwItem(lwIResourceMgr* res_mgr);
		~lwItem();

		// Был частью lwInterface; сохранён как удобный legacy-вызов
		// (`obj->Release()` вместо `delete obj`).
		void Release() {
			delete this;
		}

		void SetFileName(std::string_view file) {
			_file_name = file;
		}

		const std::string& GetFileName() {
			return _file_name;
		}

		lwIPrimitive* GetPrimitive() {
			return _obj;
		}

		LW_RESULT Copy(const lwItem* src_obj);
		LW_RESULT Clone(lwItem** ret_obj);

		lwMatrix44* GetMatrix() {
			return &_mat_base;
		}

		void SetMatrix(const lwMatrix44* mat) {
			_mat_base = *mat;
		}

		void SetOpacity(float opacity);

		LW_RESULT Load(std::string_view file);
		LW_RESULT Update();
		LW_RESULT Render();
		LW_RESULT Destroy();

		LW_RESULT HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray);

		void SetMaterial(const lwMaterial* mtl);

		void ShowBoundingObject(int show);

		const lwMatrix44* GetObjDummyMatrix(DWORD id);
		const lwMatrix44* GetObjBoneDummyMatrix(DWORD id);

		LW_RESULT PlayDefaultAnimation(float velocity = 1.0f);


		// dummy
		LW_RESULT GetDummyMatrix(lwMatrix44* mat, DWORD id);
		LW_RESULT GetObjDummyRunTimeMatrix(lwMatrix44* mat, DWORD id);

		LW_RESULT SetLinkCtrl(lwLinkCtrl* ctrl, DWORD link_parent_id, DWORD link_item_id);
		LW_RESULT ClearLinkCtrl();

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

		LW_RESULT SetTextureLOD(DWORD level);

		float GetOpacity() {
			return _opacity;
		}
	};


} // namespace Corsairs::Engine::Render
