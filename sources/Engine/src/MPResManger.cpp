#include "StdAfx.h"
#include "GlobalInc.h"
#include "MPModelEff.h"
#include "AssetLoaders.h"
#include "GeomObjCache.h"
#include "EffPathStore.h"
#include "EffectDeviceCallbacks.h"
#include "EffectFxRenderer.h"
#include "EffectMeshStore.h"
#include "EffectRenderContext.h"
#include "EffectShaderStore.h"
#include "EffectStore.h"
#include "ParticleCtrlStore.h"
#include "ParticleInstancePool.h"
#include "TextureManager.h"
#include "TobMeshStore.h"

#include "mpresmanger.h"
#include "lwSysGraphics.h"
#include "mpeffectctrl.h"
#include "lwIUtil.h"
#include "ShaderLoad.h"
#include "MPRender.h"
#include "MPResourceSet.h"
#include "lwExpObj.h"
#include "lwPhysique.h"

using namespace std;
using Corsairs::Engine::Render::EffPathStore;
using Corsairs::Engine::Render::EffectDeviceCallbacks;
using Corsairs::Engine::Render::EffectFxRenderer;
using Corsairs::Engine::Render::EffectMeshStore;
using Corsairs::Engine::Render::EffectRenderContext;
using Corsairs::Engine::Render::EffectShaderStore;
using Corsairs::Engine::Render::EffectStore;
using Corsairs::Engine::Render::ParticleCtrlStore;
using Corsairs::Engine::Render::ParticleInstancePool;
using Corsairs::Engine::Render::TobMeshStore;

//  Default — true: при старте прогреваем bone-кэш (animation/*.lab) и
//  geom-кэш для character meshes (model/character/*.lgo). Клиент читает
//  [Resources] preload_at_start из system.ini и зовёт SetResourcePreload(...)
//  ДО InitRes/InitRes3.
bool CMPResManger::s_resourcePreload = true;

CMPResManger& CMPResManger::Instance() {
	static CMPResManger instance;
	return instance;
}

CMPResManger::CMPResManger() {
	m_pDev = NULL;

	_iTexNum = 0;

	_vecTexName.clear();

	_vecTexGlobalId.clear();
	_mapTexture.clear();

	EffectStore::Instance().Clear();
	EffectShaderStore::Instance().Clear();
	EffectMeshStore::Instance().Clear();
	ParticleCtrlStore::Instance().Clear();
	TobMeshStore::Instance().Clear();
	ParticleInstancePool::Instance().Clear();

	_fSaveTime = 0;
	_fDailTime = 0;
	_fCurTime = 0;


	EffPathStore::Instance().Clear();


	m_bUseSoft = FALSE;

	m_pSys = 0;
	m_pSysGraphics = 0;

	m_bCanFrame = false;
	m_iCurFrame = 0;
}

CMPResManger::~CMPResManger() {
	EffectMeshStore::Instance().Clear();
	TobMeshStore::Instance().Clear();
	ParticleInstancePool::Instance().Clear();
}

void CMPResManger::ReleaseTotalRes() {
	ParticleCtrlStore::Instance().Clear();
	ParticleInstancePool::Instance().Clear();

	EffectStore::Instance().Clear();

	EffectFxRenderer::Instance().Free();

	EffectShaderStore::Instance().Clear();

	EffPathStore::Instance().Clear();


	// Effect-текстуры теперь во владении TextureManager (Phase 2). Глобальный
	// DynamicRelease(bClearAll=true) делается из ~TextureManager; здесь просто
	// сбрасываем local-ID-маппинг.

	EffectMeshStore::Instance().Clear();
	TobMeshStore::Instance().Clear();

	_iTexNum = 0;

	_vecTexName.clear();

	_mapTexture.clear();

	_vecTexGlobalId.clear();
}

bool CMPResManger::InitRes2() {
	if (!LoadTotalMesh())
		return false;

	if (!LoadTotalEffect())
		return false;

	LoadTotalPath();

	return true;
}

bool CMPResManger::InitRes3() {
	LoadTotalRes();
	return true;
}

bool CMPResManger::InitRes(MPRender* pDev, D3DXMATRIX* pmat, D3DXMATRIX* pMatviewproj) {
	m_pDev = pDev;

	auto& renderCtx = EffectRenderContext::Instance();
	renderCtx.UpdateBackBuffer(pDev);

	RECT rc_client;
	m_pDev->GetInterfaceMgr()->dev_obj->GetWindowRect(NULL, &rc_client);
	renderCtx.GetFontBkWidth()  = (rc_client.right - rc_client.left) / 2;
	renderCtx.GetFontBkHeight() = (rc_client.bottom - rc_client.top) / 2;

	m_caps = m_pDev->GetOrgCap();
	if (m_caps.VertexShaderVersion < D3DVS_VERSION(1, 1) || m_caps.PixelShaderVersion < D3DPS_VERSION(1, 4))
		m_bUseSoftOrg = true;
	else
		m_bUseSoftOrg = false;


	m_pDev->GetDevice()->GetDeviceCaps(&m_caps);
	if (m_caps.VertexShaderVersion < D3DVS_VERSION(1, 1) || m_caps.PixelShaderVersion < D3DPS_VERSION(1, 4)) {
		m_bUseSoft = true;
	}
	else {
		m_bUseSoft = false;
	}

	EffectShaderStore::Instance().SetSoftFallback(m_bUseSoft);

	if (!EffectFxRenderer::Instance().Init(pDev, "shader\\dx9\\eff.fx")) {
		MessageBox(NULL, "shader\\eff.fx", "ERROR", 0);
		return false;
	}

	_pszTexPath = "texture\\effect";
	_pszMeshPath = "model";
	_pszEFFectPath = "effect";

	LoadTotalTexture();

	LoadTotalData();

	renderCtx.Init(pDev, pmat, pMatviewproj);

	// Контекст для EffectStore: dev + this (для BoundingRes) + billboard-mat
	// (применяется автоматически при LoadInto/AddUnited — в legacy InitRes здесь
	// шёл ручной цикл по уже загруженным эффектам, но эффекты грузятся ПОЗЖЕ
	// в InitRes2, поэтому цикл фактически работал по пустому списку).
	auto& effectStore = EffectStore::Instance();
	effectStore.SetDevice(pDev);
	effectStore.SetResMgr(this);
	effectStore.SetBillboardMatrix(renderCtx.GetBBoardMat());

	auto& meshStore = EffectMeshStore::Instance();
	meshStore.SetDevice(pDev);
	meshStore.SetSystem(m_pSys, m_pSysGraphics);

	auto& tobStore = TobMeshStore::Instance();
	tobStore.SetDevice(pDev);
	tobStore.SetSysGraphics(m_pSysGraphics);

	ParticleInstancePool::Instance().SetResMgr(this);

	EffectDeviceCallbacks::Instance().Install(pDev, m_pSysGraphics);

	return true;
}

int CMPResManger::GetTextureID(const s_string& sName) {
	TEXTURE_MAP::iterator pos = _mapTexture.find(sName);
	if (pos != _mapTexture.end()) {
		return (*pos).second;
	}

	ToLogService("errors", LogLevel::Error, "[{}]()", sName);
	return -1;
}

//-----------------------------------------------------------------------------
IDirect3DTextureX* CMPResManger::GetTextureByID(int iID) {
	if (iID < 0 || iID >= static_cast<int>(_vecTexGlobalId.size())) {
		return NULL;
	}
	return TextureManager::I()->GetD3DTexture(_vecTexGlobalId[iID]);
}

//-----------------------------------------------------------------------------
lwITex* CMPResManger::GetTextureByIDlw(int iID) {
	if (iID < 0 || iID >= static_cast<int>(_vecTexGlobalId.size())) {
		return nullptr;
	}
	return TextureManager::I()->GetTexture(_vecTexGlobalId[iID]);
}

//-----------------------------------------------------------------------------
lwITex* CMPResManger::GetTextureByNamelw(const s_string& sName) {
	int iTexID = GetTextureID(sName);
	if (iTexID == -1) {
		return 0;
	}

	return GetTextureByIDlw(iTexID);
}

//-----------------------------------------------------------------------------
int CMPResManger::GetMeshID(const s_string& sName) {
	return EffectMeshStore::Instance().GetID(sName);
}

//-----------------------------------------------------------------------------
CEffectModel* CMPResManger::GetMeshByID(int iID) {
	return EffectMeshStore::Instance().GetByID(iID);
}

//-----------------------------------------------------------------------------
CEffectModel* CMPResManger::GetMeshByName(const s_string& sName) {
	return EffectMeshStore::Instance().GetByName(sName);
}

//-----------------------------------------------------------------------------
void CMPResManger::DeleteMesh(CEffectModel& rEffectModel) {
	EffectMeshStore::Instance().DeleteMesh(rEffectModel);
}

int CMPResManger::GetMeshNum() {
	return EffectMeshStore::Instance().Count();
}

VEC_string& CMPResManger::GetTotalMeshName() {
	return EffectMeshStore::Instance().GetAllNames();
}

int CMPResManger::GetEffectID(const s_string& pszName) {
	return EffectStore::Instance().GetID(pszName);
}

std::vector<I_Effect>& CMPResManger::GetEffectByID(int iID) {
	return EffectStore::Instance().GetByID(iID);
}

I_Effect* CMPResManger::GetSubEffectByID(int iID, int iSubIdx) {
	return EffectStore::Instance().GetSubByID(iID, iSubIdx);
}

int CMPResManger::GetEffectNum() {
	return EffectStore::Instance().Count();
}

int CMPResManger::GetSubEffectNum(int idx) {
	return EffectStore::Instance().GetSubCount(idx);
}

VEC_string& CMPResManger::GetTotalEffectName() {
	return EffectStore::Instance().GetAllNames();
}

IDirect3DVertexShaderX* CMPResManger::GetVShaderByID(int iID) {
	return EffectShaderStore::Instance().GetVShaderByID(iID);
}

IDirect3DVertexDeclarationX* CMPResManger::GetVDeclByID(int iID) {
	return EffectShaderStore::Instance().GetVDeclByID(iID);
}

IDirect3DVertexShaderX* CMPResManger::GetShadeVS() {
	return EffectShaderStore::Instance().GetShadeVS();
}

IDirect3DVertexDeclarationX* CMPResManger::GetShadeVDecl() {
	return EffectShaderStore::Instance().GetShadeVDecl();
}

IDirect3DVertexShaderX* CMPResManger::GetMinimapVS() {
	return EffectShaderStore::Instance().GetMinimapVS();
}

CEffectModel* CMPResManger::GetShadeMesh() {
	return EffectMeshStore::Instance().GetShadeMesh();
}

EffParameter* CMPResManger::GetEffectParamByID(int iID) {
	return EffectStore::Instance().GetParamByID(iID);
}

IDirect3DVertexDeclarationX* CMPResManger::GetMinimapVDecl() {
	return EffectShaderStore::Instance().GetMinimapVDecl();
}

bool CMPResManger::LoadTotalTexture() {
	WIN32_FIND_DATA t_sfd{};
	HANDLE t_hFind = NULL;

	const std::string pattern = _pszTexPath + "\\*.tga";

	if ((t_hFind = FindFirstFile(pattern.c_str(), &t_sfd)) == INVALID_HANDLE_VALUE)
		return false;

	std::string sFileName;
	do {
		if (t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		sFileName = t_sfd.cFileName;
		std::transform(sFileName.begin(), sFileName.end(), sFileName.begin(),
		               [](unsigned char c) {
			               return static_cast<char>(std::tolower(c));
		               });

		// _vecTexName/_mapTexture хранят имя без расширения — local-effect-ID
		// маппинг используется .par/.eff/UI-кодом через GetTextureID.
		const std::string nameNoExt = sFileName.substr(0, sFileName.rfind('.'));
		_mapTexture[nameNoExt] = static_cast<int>(_vecTexName.size());
		_vecTexName.push_back(nameNoExt.c_str());

		// Эффект-текстуры регистрируются в общем TextureManager с двумя
		// policy-полями (Phase 2 migration):
		//   forceFormat=A8R8G8B8 — повторяет legacy `lwLoadTex(..., A8R8G8B8)`.
		//   pinned=true — исключает из LRU-DynamicRelease (паузы > 8s — норма).
		const std::string fullPath = std::format("{}\\{}", _pszTexPath, sFileName);
		const int globalId = TextureManager::I()->GetOrCreateID(
			fullPath, D3DFMT_A8R8G8B8, /*pinned=*/true);
		_vecTexGlobalId.push_back(globalId);
	}
	while (FindNextFile(t_hFind, &t_sfd));
	FindClose(t_hFind);

	_iTexNum = static_cast<int>(_vecTexName.size());
	return true;
}

void CMPResManger::LoadTotalData() {
	if (!s_resourcePreload)
		return;

	WIN32_FIND_DATA t_sfd;
	HANDLE t_hFind = NULL;

	// Pre-warm bone-кэша для animation\*.lab.
	constexpr std::string_view t_Path = "animation\\\\*.lab";

	int count = 0;
	if ((t_hFind = FindFirstFile(t_Path.data(), &t_sfd)) == INVALID_HANDLE_VALUE)
		return;
	do {
		if (!(t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			const std::string_view fileName{t_sfd.cFileName};
			if (!fileName.ends_with(".lab")) {
				continue;
			}

			g_GeomManager.LoadBoneData(t_sfd.cFileName);
			count++;
			if (count == 50) {
				break;
			}
		}
	}
	while (FindNextFile(t_hFind, &t_sfd));
	FindClose(t_hFind);
}

void CMPResManger::LoadTotalRes() {
	LoadTotalPartCtrl();

	if (!s_resourcePreload)
		return;

	WIN32_FIND_DATA t_sfd;
	HANDLE t_hFind = NULL;

	// Pre-warm GeomObjCache для character meshes (`model\character\*.lgo`).
	// Старый лимит 900 (фактически 450 из-за двойного ++) сохранён, чтобы
	// не менять поведение старта; полный прогрев — отдельным шагом.
	constexpr std::string_view t_Path = "model\\character\\*.lgo";
	const std::string_view chaPrefix =
		Corsairs::Engine::Render::GeomObjCache::CategoryPrefix(Corsairs::Engine::Render::GeomCategory::Character);

	static int nNum = 0;
	if ((t_hFind = FindFirstFile(t_Path.data(), &t_sfd)) == INVALID_HANDLE_VALUE)
		return;
	do {
		if (!(t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			const std::string_view fileName{t_sfd.cFileName};
			if (!fileName.ends_with(".lgo")) {
				continue;
			}

			nNum++;
			if (nNum++ >= 900)
				break;

			const std::string fullPath = std::format("{}{}", chaPrefix, fileName);
			Corsairs::Engine::Render::GeomObjCache::Instance().GetOrLoad(fullPath);
		}
	}
	while (FindNextFile(t_hFind, &t_sfd));
	FindClose(t_hFind);
}

bool CMPResManger::LoadTotalMesh() {
	return EffectMeshStore::Instance().LoadAllFrom(_pszMeshPath);
}

I_Effect* CMPResManger::AddEffectToMgr(const s_string& strName) {
	return EffectStore::Instance().AddNamed(strName);
}

void CMPResManger::AddUniteEffectToMgr(std::vector<I_Effect>& vecEffArray) {
	EffectStore::Instance().AddUnited(vecEffArray);
}

bool CMPResManger::LoadTotalEffect() {
	return EffectStore::Instance().LoadAllFrom(_pszEFFectPath);
}

bool CMPResManger::LoadTotalVShader(lwISysGraphics* sys_graphics) {
	return EffectShaderStore::Instance().LoadAll(sys_graphics);
}

int CMPResManger::GetEffPathID(const s_string& pszName) {
	return EffPathStore::Instance().GetID(pszName);
}

CEffPath* CMPResManger::GetEffPath(int iID) {
	return EffPathStore::Instance().GetByID(iID);
}

bool CMPResManger::LoadTotalPath() {
	return EffPathStore::Instance().LoadAllFrom(_pszEFFectPath);
}

int CMPResManger::GetPartCtrlID(const s_string& pszName) {
	return ParticleCtrlStore::Instance().GetID(pszName);
}

CMPPartCtrl* CMPResManger::GetPartCtrlByID(int iID) {
	return ParticleCtrlStore::Instance().GetByID(iID);
}

int CMPResManger::GetPartCtrlNum() {
	return ParticleCtrlStore::Instance().Count();
}

void CMPResManger::LoadTotalPartCtrl() {
	ParticleCtrlStore::Instance().LoadAllFrom(_pszEFFectPath);
}

CMPPartCtrl* CMPResManger::NewPartCtrl(const s_string& strName) {
	return ParticleCtrlStore::Instance().NewNamed(strName);
}

void CMPResManger::DeletePartCtrl(int iID) {
}

CEffectModel* CMPResManger::NewTobMesh() {
	return TobMeshStore::Instance().NewTobMesh();
}

bool CMPResManger::DeleteTobMesh(CEffectModel& rEffectModel) {
	return TobMeshStore::Instance().DeleteTobMesh(rEffectModel);
}

int CMPResManger::GetTobMeshNum() {
	return TobMeshStore::Instance().Count();
}

//-----------------------------------------------------------------------------
void CMPResManger::FrameMove(DWORD dwTime) {
	m_iCurFrame += 1;
	if (m_iCurFrame > 1)
		return;


	_fCurTime = (float)GetTickCount() / 1000;
	if (!_bInitTime) {
		_fSaveTime = _fCurTime;
		_bInitTime = true;
	}
	_fDailTime = _fCurTime - _fSaveTime;
	_fSaveTime = _fCurTime;

	EffectRenderContext::Instance().UpdateMatrices();
	ParticleInstancePool::Instance().FrameMove(dwTime);
}

//-----------------------------------------------------------------------------
void CMPResManger::Render() {
	if (m_iCurFrame < 1)
		return;
	m_iCurFrame = 0;
	ParticleInstancePool::Instance().Render();
}

void CMPResManger::Clear() {
	ParticleInstancePool::Instance().Reset();
}

void CMPResManger::UpdateMatrix() {
	EffectRenderContext::Instance().UpdateMatrices();
}

void CMPResManger::BeginEffect(int iIdx) {
	EffectFxRenderer::Instance().BeginEffect(iIdx);
}

void CMPResManger::EndEffect() {
	EffectFxRenderer::Instance().EndEffect();
}

void CMPResManger::RestoreEffect() {
	EffectFxRenderer::Instance().RestoreEffect();
}

CMPEffectFile* CMPResManger::GetEffectFile() {
	return EffectFxRenderer::Instance().GetEffectFile();
}

D3DXMATRIX* CMPResManger::GetBBoardMat() {
	return EffectRenderContext::Instance().GetBBoardMat();
}

D3DXMATRIX* CMPResManger::GetViewProjMat() {
	return EffectRenderContext::Instance().GetViewProjMat();
}

D3DXMATRIX* CMPResManger::Get2DViewProjMat() {
	return EffectRenderContext::Instance().Get2DViewProjMat();
}

int CMPResManger::GetBackBufferWidth() {
	return EffectRenderContext::Instance().GetBackBufferWidth();
}

int CMPResManger::GetBackBufferHeight() {
	return EffectRenderContext::Instance().GetBackBufferHeight();
}

int& CMPResManger::GetFontBkWidth() {
	return EffectRenderContext::Instance().GetFontBkWidth();
}

int& CMPResManger::GetFontBkHeight() {
	return EffectRenderContext::Instance().GetFontBkHeight();
}

void CMPResManger::SendResMessage(const s_string& strPartName, D3DXVECTOR3 vPos, MPMap* pMap) {
	const int id = GetPartCtrlID(strPartName);
	if (id < 0) {
		return;
	}
	CMPPartCtrl* tctrl = GetPartCtrlByID(id);
	if (!tctrl) {
		return;
	}
	ParticleInstancePool::Instance().Spawn(tctrl, vPos, pMap);
}
