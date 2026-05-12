// DXEffectFile.cpp: implementation of the CMPEffectFile class.

#include "StdAfx.h"
#include "GlobalInc.h"

#include "EffectFile.h"
#include "MPRender.h"
#include "ShaderLoader.h"

// Construction/Destruction
CMPEffectFile::CMPEffectFile() {
	_effect = NULL;
	_dev = NULL;
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
}

CMPEffectFile::CMPEffectFile(MPRender* pDev) {
	_dev = pDev;
	_effect = NULL;
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
}

CMPEffectFile::~CMPEffectFile() {
	free();
}

void CMPEffectFile::InitDev(MPRender* pDev) {
	_dev = pDev;
}

BOOL CMPEffectFile::LoadEffectFromFile(LPCSTR pszfile) {
	const std::string_view file = pszfile ? std::string_view{pszfile} : std::string_view{};
	if (LW_RESULT r = Corsairs::Engine::Render::ShaderLoader::LoadEffect(
			_dev->GetDevice(), file, &_effect);
		LW_FAILED(r)) {
		return FALSE;
	}

	D3DXHANDLE technique;
	std::string t_psz = "t0";
	while (SUCCEEDED(_effect->FindNextValidTechnique(t_psz.c_str(), &technique))) {
		_vecTechniques.push_back(technique);
		_iTechNum++;

		t_psz = std::format("t{}", _iTechNum);
	}

	return TRUE;
}

//                        _dev, pvData, cbData, 
//						&_effect,NULL) ) )
//                        return FALSE;
//						return TRUE;
//		return FALSE;
//	return FALSE;


void CMPEffectFile::free() {
	SAFE_RELEASE(_effect);
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
}

BOOL CMPEffectFile::OnLostDevice() {
	if (_effect) {
		if (HRESULT hr = _effect->OnLostDevice(); FAILED(hr)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] OnLostDevice failed: hr=0x{:08X}",
						 __FUNCTION__, static_cast<std::uint32_t>(hr));
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CMPEffectFile::OnResetDevice() {
	if (_effect) {
		if (HRESULT hr = _effect->OnResetDevice(); FAILED(hr)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] OnResetDevice failed: hr=0x{:08X}",
						 __FUNCTION__, static_cast<std::uint32_t>(hr));
			return FALSE;
		}
	}
	return TRUE;
}

MPRender* CMPEffectFile::GetDev() {
	return _dev;
}

BOOL CMPEffectFile::SetTechnique(int iIdx) {
	if (HRESULT hr = _effect->SetTechnique(_vecTechniques[iIdx]); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] SetTechnique failed: iIdx={}, tech_num={}, hr=0x{:08X}",
					 __FUNCTION__, iIdx, _iTechNum, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	return TRUE;
}

BOOL CMPEffectFile::SetTexture(LPCSTR TextureValue, IDirect3DTextureX* pTexture) {
	if (HRESULT hr = _effect->SetTexture(TextureValue, pTexture); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] SetTexture failed: name={}, tex_ptr=0x{:X}, hr=0x{:08X}",
					 __FUNCTION__, TextureValue ? TextureValue : "(null)",
					 reinterpret_cast<std::uintptr_t>(pTexture), static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	return TRUE;
}

BOOL CMPEffectFile::SetDword(LPCSTR DwName, DWORD dwvalue) {
	return TRUE;
}

BOOL CMPEffectFile::Begin(DWORD dwIsSave) {
	if (HRESULT hr = _effect->Begin(&_passes, dwIsSave); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] Effect::Begin failed: dwIsSave={}, hr=0x{:08X}",
					 __FUNCTION__, dwIsSave, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	return TRUE;
}

BOOL CMPEffectFile::Pass(UINT ipass) {
	if (HRESULT hr = _effect->BeginPass(ipass); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] BeginPass failed: ipass={}, hr=0x{:08X}",
					 __FUNCTION__, ipass, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	if (HRESULT hr = _effect->CommitChanges(); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] CommitChanges failed: ipass={}, hr=0x{:08X}",
					 __FUNCTION__, ipass, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	return TRUE;
}

BOOL CMPEffectFile::End() {
	if (HRESULT hr = _effect->EndPass(); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] EndPass failed: hr=0x{:08X}",
					 __FUNCTION__, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	if (HRESULT hr = _effect->End(); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] Effect::End failed: hr=0x{:08X}",
					 __FUNCTION__, static_cast<std::uint32_t>(hr));
		return FALSE;
	}
	return TRUE;
}
