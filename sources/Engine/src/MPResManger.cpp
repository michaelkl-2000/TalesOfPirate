#include "StdAfx.h"
#include "GlobalInc.h"
#include "MPModelEff.h"
#include "AssetLoaders.h"
#include "EffPathStore.h"
#include "EffectShaderStore.h"
#include "EffectStore.h"

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
using Corsairs::Engine::Render::EffectShaderStore;
using Corsairs::Engine::Render::EffectStore;

CMPResManger ResMgr;

//  Default — true: при старте прогреваем bone-кэш (animation/*.lab) и
//  geom-кэш для character meshes (model/character/*.lgo). Клиент читает
//  [Resources] preload_at_start из system.ini и зовёт SetResourcePreload(...)
//  ДО InitRes/InitRes3.
bool CMPResManger::s_resourcePreload = true;

CMPResManger::CMPResManger(void) {
	m_pDev = NULL;

	_iTexNum = 0;
	_iMeshNum = 0;

	_vecTexName.clear();
	_vecMeshName.clear();
	_mapMesh.clear();

	_vecTexList.clear();
	_vecMeshList.clear();
	_mapTexture.clear();

	EffectStore::Instance().Clear();
	EffectShaderStore::Instance().Clear();

	_fSaveTime = 0;
	_fDailTime = 0;
	_fCurTime = 0;
	_pMatView = NULL;
	_pMatViewProj = NULL;


	EffPathStore::Instance().Clear();


	_iTobMeshNum = 0;
	_lstTobMeshs.clear();


	WORD iw;
	_vecMeshList.resize(MAXMESH_COUNT);
	_vecPartCtrl.resize(MAXPART_COUNT);
	_vecPartCtrl.setsize(MAXPART_COUNT);

	for (iw = 0; iw < MAXPART_COUNT; iw++) {
		(*_vecPartCtrl[iw]) = NULL;
	}


	_iPartCtrlNum = 0;

	_vecPartArray.clear();
	_vecPartArray.resize(MAXMSG_COUNT);
	_vecValidID.resize(MAXMSG_COUNT);
	_vecValidID.setsize(MAXMSG_COUNT);
	for (iw = 0; iw < MAXMSG_COUNT; iw++) {
		_vecPartArray[iw] = NULL;
		*_vecValidID[iw] = iw;
	}


	m_bUseSoft = FALSE;

	m_pSys = 0;
	m_pSysGraphics = 0;

	m_bCanFrame = false;
	m_iCurFrame = 0;
}

CMPResManger::~CMPResManger(void) {
	for (size_t i(0); i < _vecMeshList.size(); i++) {
		SAFE_DELETE(_vecMeshList[i]);
	}
}

void CMPResManger::ReleaseTotalRes() {
	int iw;

	for (iw = 0; iw < _iPartCtrlNum; iw++) {
		CMPPartCtrl** C = _vecPartCtrl[iw];
		SAFE_DELETE(*C);
	}
	_vecPartCtrl.resize(0);
	_iPartCtrlNum = 0;

	if (_vecPartArray.size() > 0) {
		for (iw = 0; iw < MAXMSG_COUNT; iw++) {
			SAFE_DELETE(_vecPartArray[iw]);
		}
		_vecValidID.resize(0);
		_vecPartArray.clear();
	}
	EffectStore::Instance().Clear();

	_CEffectFile.free();

	EffectShaderStore::Instance().Clear();

	EffPathStore::Instance().Clear();


	for (iw = 0; iw < _iTexNum; iw++) {
		SAFE_RELEASE(_vecTexList[iw]);
	}

	for (iw = 0; iw < _iMeshNum; iw++) {
		SAFE_DELETE(_vecMeshList[iw]);
	}
	std::list<CEffectModel*>::iterator iter = _lstTobMeshs.begin();
	std::list<CEffectModel*>::iterator end = _lstTobMeshs.end();
	for (; iter != end; ++iter) {
		SAFE_DELETE(*iter);
	}
	SAFE_DELETE(_CShadeModel);

	_iTexNum = 0;
	_iMeshNum = 0;
	_iTobMeshNum = 0;

	_vecTexName.clear();
	_vecMeshName.clear();

	_mapMesh.clear();
	_mapTexture.clear();

	_vecTexList.clear();
	_vecMeshList.clear();
	_lstTobMeshs.clear();
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

	IDirect3DSurfaceX* pBackBuffer;
	m_pDev->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	pBackBuffer->GetDesc(&m_d3dBackBuffer);
	pBackBuffer->Release();

	RECT rc_client;
	m_pDev->GetInterfaceMgr()->dev_obj->GetWindowRect(NULL, &rc_client);
	_iFontBkWidth = (rc_client.right - rc_client.left) / 2;
	_iFontBkHeight = (rc_client.bottom - rc_client.top) / 2;


	D3DXMatrixOrthoLH(&_Mat2dViewProj, float(m_d3dBackBuffer.Width), float(m_d3dBackBuffer.Height), 0.0f, 1.0f);

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

	_CEffectFile.InitDev(pDev);
	if (!_CEffectFile.LoadEffectFromFile("shader\\dx9\\eff.fx")) {
		MessageBox(NULL, "shader\\eff.fx", "ERROR", 0);
		return false;
	}

	_pszTexPath = "texture\\effect";
	_pszMeshPath = "model";
	_pszEFFectPath = "effect";

	LoadTotalTexture();

	LoadTotalData();

	_pMatView = pmat;
	D3DXMatrixInverse(&_MatBBoard, NULL, _pMatView);
	_MatBBoard._41 = 0.0f;
	_MatBBoard._42 = 0.0f;
	_MatBBoard._43 = 0.0f;

	_pMatViewProj = pMatviewproj;

	// Контекст для EffectStore: dev + this (для BoundingRes) + billboard-mat
	// (применяется автоматически при LoadInto/AddUnited — в legacy InitRes здесь
	// шёл ручной цикл по уже загруженным эффектам, но эффекты грузятся ПОЗЖЕ
	// в InitRes2, поэтому цикл фактически работал по пустому списку).
	auto& effectStore = EffectStore::Instance();
	effectStore.SetDevice(pDev);
	effectStore.SetResMgr(this);
	effectStore.SetBillboardMatrix(&_MatBBoard);

	lwRegisterOutputLoseDeviceProc(g_OnLostDevice);
	lwRegisterOutputResetDeviceProc(g_OnResetDevice);

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
	if (_vecTexList[iID])
		return _vecTexList[iID]->GetTex();
	else {
		return NULL;
	}
}

//-----------------------------------------------------------------------------
lwITex* CMPResManger::GetTextureByIDlw(int iID) {
	if (_vecTexList[iID])
		return _vecTexList[iID];
	else {
#ifdef USE_DDS_FILE_EFFECT
		const std::string t_pszFile = std::format("{}\\{}.dds", _pszTexPath, _vecTexName[iID]);
#else
		const std::string t_pszFile = std::format("{}\\{}.tga", _pszTexPath, _vecTexName[iID]);
#endif
		lwITex* tex;

		if (LW_RESULT r = lwLoadTex(&tex, m_pSysGraphics->GetResourceMgr(), t_pszFile, std::string_view{}, D3DFMT_A8R8G8B8);
			LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] lwLoadTex failed: id={}, file={}, ret={}",
						 __FUNCTION__, iID, t_pszFile, static_cast<long long>(r));
			return 0;
		}
		_vecTexList[iID] = tex;

		return _vecTexList[iID];
	}
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
	MESH_MAP::iterator pos = _mapMesh.find(sName);
	if (pos != _mapMesh.end()) {
		return (*pos).second;
	}
	return -1;
}

//-----------------------------------------------------------------------------
CEffectModel* CMPResManger::GetMeshByID(int iID) {
	CEffectModel* pRetMesh(0);

	if (iID >= 7) {
		if (!_vecMeshList[iID]) {
			_vecMeshList[iID] = new CEffectModel;

			_vecMeshList[iID]->InitDevice(m_pDev);
			lwIPathInfo* path_info;
			m_pSys->GetInterface((LW_VOID**)&path_info, LW_GUID_PATHINFO);
			const std::string szOldPath = path_info->GetPath(PATH_TYPE_MODEL_ITEM);
			path_info->SetPath(PATH_TYPE_MODEL_ITEM, "model\\effect\\");
			if (!_vecMeshList[iID]->LoadModel((TCHAR*)_vecMeshName[iID].c_str())) {
				SAFE_DELETE(_vecMeshList[iID]);
				path_info->SetPath(PATH_TYPE_MODEL_ITEM, szOldPath.c_str());

				ToLogService("errors", LogLevel::Error, "[id={}]", iID);
				return 0;
			}
			if (!_vecMeshList[iID]->GetObject() || !_vecMeshList[iID]->GetObject()->GetPrimitive()) {
				ToLogService("errors", LogLevel::Error, ": effectmesh->GetObject(),effectmesh->GetPrimitive()__ID={}",
							 iID);
			}
			else
				_vecMeshList[iID]->GetObject()->GetPrimitive()->SetState(STATE_TRANSPARENT, 0);
			path_info->SetPath(PATH_TYPE_MODEL_ITEM, szOldPath.c_str());
			pRetMesh = _vecMeshList[iID];
		}
		else {
			if (_vecMeshList[iID]->IsUsing()) {
				int n = _iMeshNum;
				for (; n < MAXMESH_COUNT; ++n) {
					if (_vecMeshList[n] && _vecMeshList[n]->IsUsing()) {
						continue;
					}
					if (!_vecMeshList[n]) {
						_vecMeshList[n] = new CEffectModel;
					}

					if (_vecMeshList[n]->m_iID != iID) {
						if (!_vecMeshList[n]->Copy(*_vecMeshList[iID])) {
							SAFE_DELETE(_vecMeshList[n]);
							ToLogService("errors", LogLevel::Error, "[id={}]", iID);
							return 0;
						}
					}

					break;
				}
				if (n >= MAXMESH_COUNT) {
					ToLogService("errors", LogLevel::Error, "");
					return 0;
				}
				pRetMesh = _vecMeshList[n];
			}
			else {
				pRetMesh = _vecMeshList[iID];
			}
		}
	}
	else {
		pRetMesh = _vecMeshList[iID];
	}
	pRetMesh->m_iID = iID;
	pRetMesh->SetUsing(true);
	return pRetMesh;
}

//-----------------------------------------------------------------------------
CEffectModel* CMPResManger::GetMeshByName(const s_string& sName) {
	int iMeshID = GetMeshID(sName);
	if (iMeshID == -1) {
		return 0;
	}
	return GetMeshByID(iMeshID);
}

//-----------------------------------------------------------------------------
void CMPResManger::DeleteMesh(CEffectModel& rEffectModel) {
	if (rEffectModel.m_iID >= 7) {
		rEffectModel.SetUsing(false);
	}
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
	return _CShadeModel;
}

EffParameter* CMPResManger::GetEffectParamByID(int iID) {
	return EffectStore::Instance().GetParamByID(iID);
}

IDirect3DVertexDeclarationX* CMPResManger::GetMinimapVDecl() {
	return EffectShaderStore::Instance().GetMinimapVDecl();
}

bool CMPResManger::LoadTotalTexture() {
	{
		WIN32_FIND_DATA t_sfd;
		HANDLE t_hFind = NULL;

		const std::string t_Path = _pszTexPath + "\\*.tga";

		if ((t_hFind = FindFirstFile(t_Path.c_str(), &t_sfd)) == INVALID_HANDLE_VALUE)
			return false;
		string sFileName;
		do {
			if (!(t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				sFileName = t_sfd.cFileName;
				transform(sFileName.begin(), sFileName.end(),
						  sFileName.begin(),
						  [](unsigned char c) {
							  return std::tolower(c);
						  });
				sFileName = sFileName.substr(0, sFileName.rfind('.'));

				{
					_mapTexture[sFileName] = (int)_vecTexName.size();
					_vecTexName.push_back(sFileName.c_str());
				}
			}
		}
		while (FindNextFile(t_hFind, &t_sfd));
		FindClose(t_hFind);
	}

	_iTexNum = (int)_vecTexName.size();
	_vecTexList.resize(_iTexNum);
	for (int iw = 0; iw < _iTexNum; iw++) {
		_vecTexList[iw] = NULL;
	}
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

	// Pre-warm geom-кэша для character meshes (`model\character\*.lgo`).
	// LoadGeomobj хардкодит этот путь, и здесь итерация совпадает —
	// заполняем m_GeomobjMap, чтобы первое обращение из lwPhysique::LoadPrimitive
	// не ходило на диск.
	constexpr std::string_view t_Path = "model\\character\\*.lgo";

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

			g_GeomManager.LoadGeomobj(t_sfd.cFileName);
		}
	}
	while (FindNextFile(t_hFind, &t_sfd));
	FindClose(t_hFind);
}

bool CMPResManger::LoadTotalMesh() {
	_iMeshNum = 7;

	_mapMesh[MESH_TRI] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_TRI);

	_mapMesh[MESH_RECT] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_RECT);

	_mapMesh[MESH_PLANERECT] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_PLANERECT);

	_mapMesh[MESH_PLANETRI] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_PLANETRI);

	_mapMesh[MESH_RECTZ] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_RECTZ);

	_mapMesh[MESH_CONE] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_CONE);

	_mapMesh[MESH_CYLINDER] = (int)_vecMeshName.size();
	_vecMeshName.push_back(MESH_CYLINDER);

	_vecMeshList.resize(MAXMESH_COUNT);
	for (int n = 0; n < MAXMESH_COUNT; n++) {
		_vecMeshList[n] = NULL;
	}
	_vecMeshList[0] = new CEffectModel;
	_vecMeshList[0]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[0]->CreateTriangle();

	_vecMeshList[1] = new CEffectModel;
	_vecMeshList[1]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[1]->CreateRect();

	_vecMeshList[2] = new CEffectModel;
	_vecMeshList[2]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[2]->CreatePlaneRect();

	_vecMeshList[3] = new CEffectModel;
	_vecMeshList[3]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[3]->CreatePlaneTriangle();

	_vecMeshList[4] = new CEffectModel;
	_vecMeshList[4]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[4]->CreateRectZ();

	_vecMeshList[5] = new CEffectModel;
	_vecMeshList[5]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[5]->CreateCone(8, 3, 2);

	_vecMeshList[6] = new CEffectModel;
	_vecMeshList[6]->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_vecMeshList[6]->CreateCylinder(8, 3, 1, 3);

	_CShadeModel = NULL;
	_CShadeModel = new CEffectModel;
	_CShadeModel->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_CShadeModel->CreateShadeModel();


	{
		WIN32_FIND_DATA t_sfd;
		HANDLE t_hFind = NULL;

		constexpr std::string_view t_Path = "model\\effect\\*.lgo";

		if ((t_hFind = FindFirstFile(t_Path.data(), &t_sfd)) == INVALID_HANDLE_VALUE)
			return true;
		string sFileName;

		lwIPathInfo* path_info;
		m_pSys->GetInterface((LW_VOID**)&path_info, LW_GUID_PATHINFO);
		const std::string szOldPath = path_info->GetPath(PATH_TYPE_MODEL_ITEM);
		path_info->SetPath(PATH_TYPE_MODEL_ITEM, "model\\effect\\");

		do {
			if (!(t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				const std::string_view fileName{t_sfd.cFileName};
				if (!fileName.ends_with(".lgo")) {
					continue;
				}
				sFileName = t_sfd.cFileName;
				transform(sFileName.begin(), sFileName.end(),
						  sFileName.begin(),
						  [](unsigned char c) {
							  return std::tolower(c);
						  });

				_vecMeshList[_iMeshNum] = new CEffectModel;
				_vecMeshList[_iMeshNum]->InitDevice(m_pDev);
				_vecMeshList[_iMeshNum]->LoadModel(sFileName.c_str());

				_mapMesh[sFileName] = (int)_vecMeshName.size();
				_vecMeshName.push_back(sFileName.c_str());
				_iMeshNum++;
			}
		}
		while (FindNextFile(t_hFind, &t_sfd));
		FindClose(t_hFind);
		path_info->SetPath(PATH_TYPE_MODEL_ITEM, szOldPath.c_str());
	}

	return true;
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
	for (size_t n(0); n < _vecPartName.size(); n++) {
		if (_stricmp(_vecPartName[n].c_str(), pszName.c_str()) == 0) {
			return (int)n;
		}
	}
	return -1;
}

CMPPartCtrl* CMPResManger::GetPartCtrlByID(int iID) {
	if (iID > MAXPART_COUNT) {
		ToLogService("errors", LogLevel::Error, "lemon");
		return NULL;
	}
	if (iID < 0) {
		ToLogService("errors", LogLevel::Error, "ID[{}]", iID);
		return NULL;
	}
	if ((*_vecPartCtrl[iID]) == NULL) {
		const std::string t_Path = std::format("{}\\{}", _pszEFFectPath, _vecPartName[iID]);

		(*_vecPartCtrl[iID]) = new CMPPartCtrl;
		if (LW_FAILED(Corsairs::Engine::Render::PartCtrlLoader::Load(
				**_vecPartCtrl[iID], t_Path))) {
			ToLogService("errors", LogLevel::Error, "Load {} error", _vecPartName[iID]);
			return NULL;
		}
		else {
			const auto v = D3DXVECTOR3(0, 0, 0);
			(*_vecPartCtrl[iID])->MoveTo(&v);
		}
	}
	return (*_vecPartCtrl[iID]);
}

void CMPResManger::LoadTotalPartCtrl() {
	{
		WIN32_FIND_DATA t_sfd;
		HANDLE t_hFind = NULL;

		const std::string t_Path = _pszEFFectPath + "\\*.par";

		_vecPartCtrl.resize(MAXPART_COUNT);

		if ((t_hFind = FindFirstFile(t_Path.c_str(), &t_sfd)) == INVALID_HANDLE_VALUE)
			return;
		string sFileName;
		do {
			if (!(t_sfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				_iPartCtrlNum++;

				sFileName = t_sfd.cFileName;
				transform(sFileName.begin(), sFileName.end(),
						  sFileName.begin(),
						  [](unsigned char c) {
							  return std::tolower(c);
						  });
				_vecPartName.push_back(sFileName.c_str());

				_vecPartCtrl.setsize(_iPartCtrlNum);
				const std::string t_FilePath = std::format("{}\\{}", _pszEFFectPath, _vecPartName[_iPartCtrlNum - 1]);

				(*_vecPartCtrl[_iPartCtrlNum - 1]) = new CMPPartCtrl;
				if (LW_FAILED(Corsairs::Engine::Render::PartCtrlLoader::Load(
						**_vecPartCtrl[_iPartCtrlNum - 1], t_FilePath))) {
					SAFE_DELETE((*_vecPartCtrl[_iPartCtrlNum - 1]));
					ToLogService("errors", LogLevel::Error, "Load {} error", sFileName);
				}
			}
		}
		while (FindNextFile(t_hFind, &t_sfd));
		FindClose(t_hFind);
	}
}

CMPPartCtrl* CMPResManger::NewPartCtrl(const s_string& strName) {
	_iPartCtrlNum++;
	if (_iPartCtrlNum >= MAXPART_COUNT) {
		_iPartCtrlNum--;
		return NULL;
	}
	_vecPartCtrl.setsize(_iPartCtrlNum);
	_vecPartName.push_back(strName);

	(*_vecPartCtrl[_iPartCtrlNum - 1]) = new CMPPartCtrl;

	return (*_vecPartCtrl[_iPartCtrlNum - 1]);
}

void CMPResManger::DeletePartCtrl(int iID) {
}

CEffectModel* CMPResManger::NewTobMesh() {
	CEffectModel* pModel = new CEffectModel;
	pModel->InitDevice(m_pDev, m_pSysGraphics->GetResourceMgr());
	_lstTobMeshs.push_back(pModel);
	_iTobMeshNum++;
	return pModel;
}

bool CMPResManger::DeleteTobMesh(CEffectModel& rEffectModel) {
	std::list<CEffectModel*>::iterator iter = find(_lstTobMeshs.begin(), _lstTobMeshs.end(), &rEffectModel);
	if (iter != _lstTobMeshs.end()) {
		delete &rEffectModel;
		_lstTobMeshs.erase(iter);
		return true;
	}
	return false;
}

BOOL CMPResManger::OnResetDevice() {
	if (!_CEffectFile.OnResetDevice())
		return FALSE;

	EffectShaderStore::Instance().Restore(m_pSysGraphics);

	IDirect3DSurfaceX* pBackBuffer;
	m_pDev->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	pBackBuffer->GetDesc(&m_d3dBackBuffer);
	pBackBuffer->Release();

	D3DXMatrixOrthoLH(&_Mat2dViewProj, float(m_d3dBackBuffer.Width), float(m_d3dBackBuffer.Height), 0.0f, 1.0f);

	_iFontBkWidth = m_d3dBackBuffer.Width / 2;
	_iFontBkHeight = m_d3dBackBuffer.Height / 2;


	_vecMeshList[0]->CreateTriangle();
	_vecMeshList[1]->CreateRect();
	_vecMeshList[2]->CreatePlaneRect();
	_vecMeshList[3]->CreatePlaneTriangle();
	_vecMeshList[4]->CreateRectZ();
	_vecMeshList[5]->CreateCone(8, 3, 2);
	_vecMeshList[6]->CreateCylinder(8, 3, 1, 3);


	return TRUE;
}

BOOL CMPResManger::OnLostDevice() {
	if (!_CEffectFile.OnLostDevice())
		return FALSE;

	return TRUE;
}

LW_RESULT g_OnLostDevice() {
	return ResMgr.OnLostDevice();
}

LW_RESULT g_OnResetDevice() {
	return ResMgr.OnResetDevice();
}


void CMPResManger::RestoreEffect() {
	m_pDev->SetRenderStateForced(D3DRS_ZENABLE, TRUE);
	m_pDev->SetRenderStateForced(D3DRS_ZWRITEENABLE, TRUE);
	m_pDev->SetRenderStateForced(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	m_pDev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDev->SetRenderStateForced(D3DRS_ALPHATESTENABLE, FALSE);
	m_pDev->SetRenderStateForced(D3DRS_DITHERENABLE,FALSE);
	m_pDev->SetRenderStateForced(D3DRS_CULLMODE, D3DCULL_CCW); // ???????
	m_pDev->SetRenderStateForced(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_pDev->SetRenderStateForced(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	m_pDev->SetRenderStateForced(D3DRS_LIGHTING, TRUE);
	m_pDev->SetRenderStateForced(D3DRS_CLIPPING, TRUE);


	m_pDev->GetInterfaceMgr()->dev_obj->SetTextureForced(0, 0);
	m_pDev->GetInterfaceMgr()->dev_obj->SetTextureForced(1, 0);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pDev->SetTextureStageStateForced(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pDev->SetSamplerStateForced(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pDev->SetSamplerStateForced(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	m_pDev->SetSamplerStateForced(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDev->SetSamplerStateForced(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pDev->SetTextureStageStateForced(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
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

	D3DXMatrixInverse(&_MatBBoard, NULL, _pMatView);
	_MatBBoard._41 = 0.0f;
	_MatBBoard._42 = 0.0f;
	_MatBBoard._43 = 0.0f;

	D3DXMatrixTranspose(&_MatViewProjPose, _pMatViewProj);
	if (_vecValidID.size() >= MAXMSG_COUNT)
		return;

	for (WORD iw = 0; iw < MAXMSG_COUNT; ++iw) {
		if (_vecPartArray[iw]) {
			_vecPartArray[iw]->FrameMove(dwTime);
		}
	}
}

//-----------------------------------------------------------------------------
void CMPResManger::Render() {
	if (m_iCurFrame < 1)
		return;
	m_iCurFrame = 0;
	if (_vecValidID.size() >= MAXMSG_COUNT)
		return;

	for (WORD iw = 0; iw < MAXMSG_COUNT; ++iw) {
		if (_vecPartArray[iw]) {
			if (!_vecPartArray[iw]->IsPlaying()) {
				SAFE_DELETE(_vecPartArray[iw]);
				_vecValidID.push_front(iw);
				continue;
			}
			_vecPartArray[iw]->Render();
		}
	}
}

void CMPResManger::Clear() {
	for (int i = 0; i < (int)_vecPartArray.size(); ++i) {
		CMPPartCtrl* part = _vecPartArray[i];
		if (part) {
			part->Reset();
		}
	}
}

void CMPResManger::UpdateMatrix() {
	D3DXMatrixInverse(&_MatBBoard, NULL, _pMatView);
	_MatBBoard._41 = 0.0f;
	_MatBBoard._42 = 0.0f;
	_MatBBoard._43 = 0.0f;

	D3DXMatrixTranspose(&_MatViewProjPose, _pMatViewProj);
}

void CMPResManger::BeginEffect(int iIdx) {
	_CEffectFile.SetTechnique(iIdx);
	_CEffectFile.Begin(D3DXFX_DONOTSAVESTATE);
	_CEffectFile.Pass(0);
}

void CMPResManger::EndEffect() {
	_CEffectFile.End();
}

void CMPResManger::SendResMessage(const s_string& strPartName, D3DXVECTOR3 vPos, MPMap* pMap) {
	int id = GetPartCtrlID(strPartName);
	if (id < 0) {
		return;
	}
	if (_vecValidID.empty()) {
		return;
	}

	CMPPartCtrl* tctrl = GetPartCtrlByID(id);
	if (!tctrl) {
		return;
	}
	WORD idx = *_vecValidID.front();

	_vecPartArray[idx] = new CMPPartCtrl;
	_vecPartArray[idx]->CopyPartCtrl(tctrl);
	_vecPartArray[idx]->BindingRes(this);

	_vecPartArray[idx]->Reset();
	_vecPartArray[idx]->MoveTo(&vPos, pMap);
	_vecPartArray[idx]->Play(1);

	_vecValidID.pop_front();
}
