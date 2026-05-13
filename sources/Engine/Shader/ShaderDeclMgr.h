//
#pragma once

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwShaderTypes.h"
#include "lwInterfaceExt.h"

namespace Corsairs::Engine::Render {
	// ===============================================
	enum {
		SHADER_DECL_SET_NUM = 16,
		SHADER_DECL_NUM = 32,
	};


	class lwShaderDeclInfo {
	public:
		DWORD shader_id;
		DWORD light_type;
		DWORD anim_type;

	public:
		lwShaderDeclInfo()
			: shader_id(LW_INVALID_INDEX),
			  light_type(LW_INVALID_INDEX),
			  anim_type(LW_INVALID_INDEX) {
		}
	};


	class lwShaderDeclSet {
	public:
		DWORD decl_id;
		DWORD decl_num;
		lwShaderDeclInfo* decl_seq;

	public:
		lwShaderDeclSet(DWORD decl_type, DWORD decl_size) {
			decl_id = decl_type;
			decl_num = decl_size;
			decl_seq = LW_NEW(lwShaderDeclInfo[ decl_num ]);
		}

		~lwShaderDeclSet() {
			LW_SAFE_DELETE_A(decl_seq);
		}
	};


	class ShaderDeclMgr : public IShaderDeclMgr {
		LW_STD_DECLARATION()

	private:
		IShaderMgr* _shader_mgr;
		lwShaderDeclSet* _decl_set[SHADER_DECL_NUM];

	public:
		ShaderDeclMgr(IShaderMgr* shader_mgr);
		virtual ~ShaderDeclMgr();

		LW_RESULT CreateShaderDeclSet(DWORD decl_type, DWORD buf_size);
		LW_RESULT SetShaderDeclInfo(lwShaderDeclCreateInfo* info);
		LW_RESULT QueryShaderHandle(DWORD* shader_handle, const lwShaderDeclQueryInfo* info);
	};

} // namespace Corsairs::Engine::Render
