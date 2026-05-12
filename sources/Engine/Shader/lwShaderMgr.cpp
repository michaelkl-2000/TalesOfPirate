#include "stdafx.h"
#include "lwShaderMgr.h"
#include "lwInterface.h"
#include "lwSystem.h"
#include "lwSysGraphics.h"
#include "lwResourceMgr.h"
#include "lwShaderDeclMgr.h"
#include "ShaderLoader.h"

namespace Corsairs::Engine::Render {
	// =================================

	LW_STD_IMPLEMENTATION(lwShaderMgr9)

	lwShaderMgr9::lwShaderMgr9(lwIDeviceObject* dev_obj)
		: _dev_obj(dev_obj), _vs_seq(0), _vs_size(0), _vs_num(0),
		  _decl_seq(0), _decl_size(0), _decl_num(0), _decl_mgr(0) {
	}

	lwShaderMgr9::~lwShaderMgr9() {
		for (DWORD i = 0; _vs_num > 0; i++) {
			if (_vs_seq[i].handle) {
				LW_DELETE_A(_vs_seq[i].data);
				LW_RELEASE(_vs_seq[i].handle);
				_vs_num -= 1;
			}
		}

		for (DWORD i = 0; _decl_num > 0; i++) {
			if (_decl_seq[i].handle) {
				LW_DELETE_A(_decl_seq[i].data);
				LW_RELEASE(_decl_seq[i].handle);
				_decl_num -= 1;
			}
		}

		LW_IF_RELEASE(_decl_mgr);
	}

	LW_RESULT lwShaderMgr9::Init(DWORD vs_buf_size, DWORD decl_buf_size, DWORD ps_buf_size) {
		_vs_num = 0;
		_vs_size = vs_buf_size;
		_vs_seq = LW_NEW(lwVertexShaderInfo[_vs_size]);
		memset(_vs_seq, 0, sizeof(lwVertexShaderInfo) * _vs_size);

		_decl_num = 0;
		_decl_size = decl_buf_size;
		_decl_seq = LW_NEW(lwVertDeclInfo9[_decl_size]);
		memset(_decl_seq, 0, sizeof(lwVertDeclInfo9) * _decl_size);

		_decl_mgr = LW_NEW(lwShaderDeclMgr(this));

		return LW_RET_OK;
	}

	LW_RESULT lwShaderMgr9::RegisterVertexShader(DWORD type, BYTE* data, DWORD size) {
		LW_RESULT ret = LW_RET_FAILED;
		IDirect3DDeviceX* dev = _dev_obj->GetDevice();
		IDirect3DVertexShaderX* handle = 0;
		lwVertexShaderInfo* i = 0; // << declarado antes de qualquer goto

		if (type >= _vs_size) // DWORD  unsigned; "type < 0" nunca  verdadeiro
			goto __ret;

		if (_vs_seq[type].handle)
			goto __ret;

		if (!data || size == 0)
			goto __ret;

		if (HRESULT hr = dev->CreateVertexShader((DWORD*)data, &handle); FAILED(hr)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] dev->CreateVertexShader failed: type={}, size={}, hr=0x{:08X}",
						 __FUNCTION__, type, size, static_cast<std::uint32_t>(hr));
			goto __ret;
		}

		i = &_vs_seq[type];
		i->handle = handle;
		i->size = size;
		i->data = LW_NEW(BYTE[size]);
		memcpy(i->data, data, size);

		_vs_num += 1;
		ret = LW_RET_OK;

	__ret:
		return ret;
	}

	LW_RESULT lwShaderMgr9::RegisterVertexShader(DWORD type, std::string_view file,
												 const D3DXMACRO* defines) {
		ID3DXBuffer* buf_code = nullptr;
		if (LW_RESULT r = Corsairs::Engine::Render::ShaderLoader::CompileVertexShader(
				file, defines, &buf_code);
			LW_FAILED(r)) {
			return LW_RET_FAILED;
		}

		const DWORD size = buf_code->GetBufferSize();
		BYTE* code = static_cast<BYTE*>(buf_code->GetBufferPointer());

		LW_RESULT ret = RegisterVertexShader(type, code, size);
		if (LW_FAILED(ret)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] RegisterVertexShader failed: type={}, size={}, ret={}",
						 __FUNCTION__, type, size, static_cast<long long>(ret));
		}

		LW_SAFE_RELEASE(buf_code);
		return ret;
	}

	LW_RESULT lwShaderMgr9::RegisterVertexDeclaration(DWORD type, D3DVERTEXELEMENT9* data) {
		LW_RESULT ret = LW_RET_FAILED;

		IDirect3DVertexDeclarationX* handle = 0;
		IDirect3DDeviceX* dev = _dev_obj->GetDevice();
		int i = 0;
		D3DVERTEXELEMENT9* p = 0;

		// (Com DWORD, "type < 0" nunca  verdadeiro; pode remover se quiser)
		if (type >= _decl_size)
			goto __ret;

		if (_decl_seq[type].handle)
			goto __ret;

		if (!data) // segurana
			goto __ret;

		if (HRESULT hr = dev->CreateVertexDeclaration(data, &handle); FAILED(hr)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] dev->CreateVertexDeclaration failed: type={}, hr=0x{:08X}",
						 __FUNCTION__, type, static_cast<std::uint32_t>(hr));
			goto __ret;
		}

		_decl_seq[type].handle = handle;

		// Agora sim inicializa p e usa
		p = data;
		while (p->Stream != 0xFF) {
			++i;
			++p;
		}
		++i;

		_decl_seq[type].data = LW_NEW(D3DVERTEXELEMENT9[i]);
		memcpy(_decl_seq[type].data, data, sizeof(D3DVERTEXELEMENT9) * i);

		_decl_num += 1;
		ret = LW_RET_OK;

	__ret:
		return ret;
	}

	LW_RESULT lwShaderMgr9::LoseDevice() {
		LW_RESULT ret = LW_RET_FAILED;

		IDirect3DDeviceX* dev = _dev_obj->GetDevice();

		lwVertexShaderInfo* s;

		for (DWORD i = 0; i < _vs_size; i++) {
			s = &_vs_seq[i];

			LW_SAFE_RELEASE(s->handle);
		}

		ret = LW_RET_OK;

		//__ret:
		return ret;
	}

	LW_RESULT lwShaderMgr9::ResetDevice() {
		LW_RESULT ret = LW_RET_FAILED;

		IDirect3DDeviceX* dev = _dev_obj->GetDevice();

		lwVertexShaderInfo* s;

		for (DWORD i = 0; i < _vs_size; i++) {
			s = &_vs_seq[i];

			if (s->handle == 0 && s->data) {
				if (HRESULT hr = dev->CreateVertexShader((DWORD*)s->data, &s->handle); FAILED(hr)) {
					ToLogService("errors", LogLevel::Error,
								 "[{}] dev->CreateVertexShader failed: index={}, hr=0x{:08X}",
								 __FUNCTION__, i, static_cast<std::uint32_t>(hr));
					goto __ret;
				}
			}
		}

		ret = LW_RET_OK;

	__ret:
		return ret;
	}

	LW_RESULT lwShaderMgr9::QueryVertexShader(IDirect3DVertexShaderX** ret_obj, DWORD type) {
		LW_RESULT ret = LW_RET_FAILED;

		if (type < 0 || type >= _vs_size)
			goto __ret;

		if (_vs_seq[type].handle == 0)
			goto __ret;

		*ret_obj = _vs_seq[type].handle;

		ret = LW_RET_OK;

	__ret:
		return ret;
	}

	LW_RESULT lwShaderMgr9::QueryVertexDeclaration(IDirect3DVertexDeclarationX** ret_obj, DWORD type) {
		LW_RESULT ret = LW_RET_FAILED;

		if (type < 0 || type >= _decl_size)
			goto __ret;

		if (_decl_seq[type].handle == 0)
			goto __ret;

		*ret_obj = _decl_seq[type].handle;

		ret = LW_RET_OK;

	__ret:
		return ret;
	}


} // namespace Corsairs::Engine::Render
