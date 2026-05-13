//
#pragma once

#include <cstdint>

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwShaderTypes.h"
#include "lwInterfaceExt.h"

namespace Corsairs::Engine::Render {
	// ===============================================
	// directX8 shader manager

	// ===============================================
	// directX9 shader manager

	class ShaderMgr9 : public IShaderMgr {
		LW_STD_DECLARATION()

	private:
		IDeviceObject* _dev_obj;
		lwVertexShaderInfo* _vs_seq;
		lwVertDeclInfo9* _decl_seq;
		std::uint32_t _vs_size;
		std::uint32_t _vs_num;
		std::uint32_t _decl_size;
		std::uint32_t _decl_num;

		IShaderDeclMgr* _decl_mgr;

	public:
		ShaderMgr9(IDeviceObject* dev_obj);
		~ShaderMgr9();


		LW_RESULT Init(DWORD vs_buf_size, DWORD decl_buf_size, DWORD ps_buf_size);
		LW_RESULT RegisterVertexShader(DWORD type, BYTE* data, DWORD size);
		LW_RESULT RegisterVertexShader(DWORD type, std::string_view file,
									   const D3DXMACRO* defines = NULL);
		LW_RESULT RegisterVertexDeclaration(DWORD type, D3DVERTEXELEMENT9* data);
		LW_RESULT LoseDevice();
		LW_RESULT ResetDevice();

		LW_RESULT QueryVertexShader(IDirect3DVertexShaderX** ret_obj, DWORD type);
		LW_RESULT QueryVertexDeclaration(IDirect3DVertexDeclarationX** ret_obj, DWORD type);

		lwVertexShaderInfo* GetVertexShaderInfo(DWORD type) {
			return &_vs_seq[type];
		}

		IShaderDeclMgr* GetShaderDeclMgr() {
			return _decl_mgr;
		}
	};

	typedef ShaderMgr9 ShaderMgr;


} // namespace Corsairs::Engine::Render
