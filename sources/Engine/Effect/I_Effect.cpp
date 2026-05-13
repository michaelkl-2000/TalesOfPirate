#include <algorithm>

#include "StdAfx.h"

#include "GlobalInc.h"
#include "MPModelEff.h"

#include "i_effect.h"
#include "MPRender.h"
#include "Character/CharacterActionStore.h"



I_Effect::I_Effect(void) {
	_dev = NULL;

	ReleaseAll();

	_bBillBoard = false;
	_bSizeSame = true;
	_bAngleSame = true;
	_bPosSame = true;
	_bColorSame = true;

	_eSrcBlend = D3DBLEND_SRCALPHA;
	_eDestBlend = D3DBLEND_INVSRCALPHA;

	_iUseParam = 0;
	_ilast = _inext = 0;
	_flerp = 0;

	_bRotaLoop = false;
	_vRotaLoop = D3DXVECTOR4(0, 0, 0, 0);

	_bAlpha = true;
	_bRotaBoard = true;
}

I_Effect::~I_Effect(void) {
}

void I_Effect::DestroyTobMesh(CMPResManger* resMgr) {
	if (_pCModel) {
		if (!IsTobMesh(_strModelName))
			resMgr->DeleteMesh(*_pCModel);
		else
			resMgr->DeleteTobMesh(*_pCModel);
	}
}


void I_Effect::Init(MPRender* pDev, EFFECT_TYPE eType, WPARAM wParam, LPARAM lParam) {
	_dev = pDev;
	_eEffectType = eType;
	_fLength = 5.0f; //!2
	_wFrameCount = (WORD)wParam; //!

	_vecFrameTime.resize(_wFrameCount);

	_vecFrameSize.resize(_wFrameCount);
	_vecFrameAngle.resize(_wFrameCount);
	_vecFramePos.resize(_wFrameCount);
	_vecFrameColor.resize(_wFrameCount);

	_iUseParam = 0;
	_CylinderParam.resize(_wFrameCount);

	for (WORD n = 0; n < _wFrameCount; n++) {
		_vecFrameTime[n] = _fLength / _wFrameCount;

		_vecFrameSize[n] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		_vecFrameAngle[n] = D3DXVECTOR3(0, 0, 0);
		_vecFramePos[n] = D3DXVECTOR3(0, 0, 0);

		_vecFrameColor[n] = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
	//!VS
	_bBillBoard = (bool)lParam;
	if (_bBillBoard)
		_iVSIndex = 2;
	if (_eEffectType == EFFECT_MODEL)
		_iVSIndex = 1;


	////_vecFrameColor[3]	=D3DCOLOR_ARGB(200,255,255,255);
}

void I_Effect::Reset() {
	_CTexCoordlist.Reset();
	_CTextruelist.Reset();
}

void I_Effect::ReleaseAll() {
	_eEffectType = EFFECT_NONE;
	_fLength = 0.0f;
	_wFrameCount = 0;

	_vecFrameTime.clear();
	_vecFrameSize.clear();
	_vecFrameAngle.clear();
	_vecFramePos.clear();
	_vecFrameColor.clear();

	_CTexCoordlist.Clear();
	_CTextruelist.Clear();

	_CylinderParam.clear();
	_pCModel = NULL;
	_strModelName = "";

	_strEffectName = "";
	_SpmatBBoard = NULL;

	_iVSIndex = 0;
}

void I_Effect::BindingResInit(CMPResManger* _CResMagr) {
	if (_eEffectType == EFFECT_FRAMETEX) {
	}
	else if (_pCModel) {
		_CTexCoordlist.GetCoordFromModel(_pCModel);
		_CTextruelist.GetTextureFromModel(_pCModel);
		if (IsCylinderMesh(_pCModel->_strName)) {
			_iUseParam = 0;

			for (int n = 0; n < _wFrameCount; ++n) {
				_CylinderParam[n].iSegments = _nSegments;
				_CylinderParam[n].fTopRadius = _rRadius;
				_CylinderParam[n].fBottomRadius = _rBotRadius;
				_CylinderParam[n].fHei = _rHeight;
				_CylinderParam[n].Create();
			}
		}
	}
}

//-----------------------------------------------------------------------------
int I_Effect::BoundingRes(CMPResManger* _CResMagr, const char* pszParentName) {
	//!0123shade
	int t_iID = 0;

	if (_eEffectType == EFFECT_FRAMETEX) {
		for (WORD n = 0; n < _CTexFrame._wTexCount; ++n) {
			_CTexFrame._vecTexs[n] =
				_CResMagr->GetTextureByNamelw(_CTexFrame._vecTexName[n]);
			if (!_CTexFrame._vecTexs[n]) {
				const std::string pszMsg = std::format("[{}][{}]",
													   pszParentName, _CTextruelist._vecTexName);
				g_logManager.InternalLog(LogLevel::Error, "errors", pszMsg.c_str());
				return 1;
			}
		}
	}
	else {
		_CTextruelist._pTex =
			_CResMagr->GetTextureByNamelw(_CTextruelist._vecTexName);
		if (!_CTextruelist._pTex) {
			const std::string pszMsg = std::format("[{}][{}]",
												   pszParentName, _CTextruelist._vecTexName);
			g_logManager.InternalLog(LogLevel::Error, "errors", pszMsg.c_str());
			return 1;
		}
		_CTextruelist._lpCurTex = _CTextruelist._pTex->GetTex();
	}


	_SpmatBBoard = _CResMagr->GetBBoardMat();

	if (_pCModel && _pCModel->_strName == _strModelName) {
		return 0;
	}
	if (_pCModel) {
		if (!IsTobMesh(_strModelName)) {
			_CResMagr->DeleteMesh(*_pCModel);
			_pCModel = 0;
		}
		else {
			_CResMagr->DeleteTobMesh(*_pCModel);
			_pCModel = 0;
		}
	}


	if (!_bBillBoard && IsTobMesh(_strModelName)) {
		if (!_pCModel) {
			_pCModel = _CResMagr->NewTobMesh();
			if (!_pCModel->CreateTob(_strModelName, _nSegments, _rHeight, _rRadius, _rBotRadius)) {
				const std::string pszMsg = std::format("[{}][{}]",
													   pszParentName, _strModelName);
				g_logManager.InternalLog(LogLevel::Error, "errors", pszMsg.c_str());
				return 2;
			}
		}
	}
	else {
		_pCModel = _CResMagr->GetMeshByName(_strModelName);
		if (!_pCModel) {
			const std::string pszMsg = std::format("[{}][{}]",
												   pszParentName, _CTextruelist._vecTexName);
			g_logManager.InternalLog(LogLevel::Error, "errors", pszMsg.c_str());
			return 2;
		}

		if (_bBillBoard) {
			_strModelName = MESH_PLANERECT;
		}
		else {
			if (_eEffectType == EFFECT_FRAMETEX)
				_CTexFrame.GetCoordFromModel(_pCModel);
		}
	}

	// Success
	return 0;
}

//-----------------------------------------------------------------------------
void I_Effect::SetModel(CEffectModel* pCModel) {
	if (_pCModel && !IsTobMesh(_pCModel->_strName)) {
		_pCModel->SetUsing(false);
		_pCModel = 0;
	}
	_pCModel = pCModel;
	_strModelName = _pCModel->_strName;
}

//-----------------------------------------------------------------------------
void I_Effect::ResetModel() {
	if (_pCModel->IsItem()) {
		_pCModel->SetUsing(false);
		_pCModel = 0;
	}
}

void I_Effect::DeleteItem(CMPResManger* pResMgr) {
	if (_pCModel)
		if (!IsTobMesh(_strModelName)) {
			pResMgr->DeleteMesh(*_pCModel);
			_pCModel = 0;
		}
		else {
			pResMgr->DeleteTobMesh(*_pCModel);
			_pCModel = 0;
		}
}

void I_Effect::GetRes(CMPResManger* pCResMagr, std::vector<INT>& vecTex, std::vector<INT>& vecModel) {
	int t_iID = 0;
	std::vector<INT>::iterator it;

	if (_eEffectType == EFFECT_FRAMETEX) {
		for (WORD n = 0; n < _CTexFrame._wTexCount; ++n) {
			t_iID = pCResMagr->GetTextureID(_CTexFrame._vecTexName[n]);
			if (t_iID != -1) {
				it = std::find(vecTex.begin(), vecTex.end(), t_iID);
				if (it == vecTex.end()) {
					vecTex.push_back(t_iID);
				}
			}
		}
	}
	else {
		t_iID = pCResMagr->GetTextureID(_CTextruelist._vecTexName);
		if (t_iID != -1) {
			it = std::find(vecTex.begin(), vecTex.end(), t_iID);
			if (it == vecTex.end()) {
				vecTex.push_back(t_iID);
			}
		}
	}

	if (_bBillBoard) {
	}
	else {
		if (IsTobMesh(_strModelName)) {
		}
		else {
			t_iID = pCResMagr->GetMeshID(_strModelName);
			if (t_iID >= 7) {
				it = std::find(vecModel.begin(), vecModel.end(), t_iID);
				if (it == vecModel.end()) {
					vecModel.push_back(t_iID);
				}
			}
		}
	}
}

void I_Effect::ChangeModel(CEffectModel* pCModel, CMPResManger* pCResMagr) {
	if (!pCModel)
		return;

	if (_bBillBoard) {
		if (pCModel->_strName != MESH_PLANERECT)
			return;
	}
	if (_pCModel && !IsTobMesh(_pCModel->_strName)) {
		pCResMagr->DeleteMesh(*_pCModel);
		_pCModel = 0;
	}

	if (IsTobMesh(pCModel->_strName)) {
		if (IsTobMesh(_pCModel->_strName))
			pCResMagr->DeleteTobMesh(*pCModel);

		_pCModel = pCResMagr->NewTobMesh();
		_pCModel->CreateTob(pCModel->_strName,
							 pCModel->_nSegments, pCModel->_rHeight, pCModel->_rRadius, pCModel->_rBotRadius);
		_strModelName = pCModel->_strName;

		_nSegments = pCModel->_nSegments;
		_rHeight = pCModel->_rHeight;
		_rRadius = pCModel->_rRadius;
		_rBotRadius = pCModel->_rBotRadius;
		if (IsCylinderMesh(pCModel->_strName)) {
			_iUseParam = 0;
			_CylinderParam.resize(_wFrameCount);

			for (int n = 0; n < _wFrameCount; ++n) {
				_CylinderParam[n].iSegments = _nSegments;
				_CylinderParam[n].fTopRadius = _rRadius;
				_CylinderParam[n].fBottomRadius = _rBotRadius;
				_CylinderParam[n].fHei = _rHeight;
				_CylinderParam[n].Create();
			}
		}
		return;
	}
	else {
		_pCModel = pCModel;
		_strModelName = pCModel->_strName;
		if (_eEffectType == EFFECT_FRAMETEX) {
			_CTexFrame.GetCoordFromModel(_pCModel);
		}
		else
			_CTextruelist.GetTextureFromModel(_pCModel);
	}
}


void I_Effect::CopyEffect(I_Effect* pEff) {
	_dev = pEff->_dev;;

	//!
	_CTexCoordlist.Copy(&pEff->_CTexCoordlist);
	//!
	_CTextruelist.Copy(&pEff->_CTextruelist);
	//!
	_CTexFrame.Copy(&pEff->_CTexFrame);

	if (_pCModel && !IsTobMesh(_pCModel->_strName)) {
		_pCModel->SetUsing(false);
		_pCModel = 0;
	}
	_pCModel = 0;
	_strModelName = pEff->_strModelName;

	_nSegments = pEff->_nSegments;
	_rHeight = pEff->_rHeight;
	_rRadius = pEff->_rRadius;
	_rBotRadius = pEff->_rBotRadius;

	_strEffectName = pEff->_strEffectName;
	_eEffectType = pEff->_eEffectType;
	_fLength = pEff->_fLength;
	_wFrameCount = pEff->_wFrameCount;

	_vecFrameTime.resize(_wFrameCount);
	_vecFrameSize.resize(_wFrameCount);
	_vecFrameAngle.resize(_wFrameCount);
	_vecFramePos.resize(_wFrameCount);
	_vecFrameColor.resize(_wFrameCount);

	_vecFrameTime = pEff->_vecFrameTime;
	_vecFrameSize = pEff->_vecFrameSize;
	_vecFrameAngle = pEff->_vecFrameAngle;
	_vecFramePos = pEff->_vecFramePos;
	_vecFrameColor = pEff->_vecFrameColor;

	_iUseParam = pEff->_iUseParam;
	_CylinderParam.resize(_wFrameCount);
	if (_iUseParam > 0) {
		_CylinderParam = pEff->_CylinderParam;
		for (int n = 0; n < _wFrameCount; ++n) {
			_CylinderParam[n].Create();
		}
	}
	else {
		if (IsCylinderMesh(_strModelName)) {
			for (int n = 0; n < _wFrameCount; ++n) {
				_CylinderParam[n].iSegments = _nSegments;
				_CylinderParam[n].fTopRadius = _rRadius;
				_CylinderParam[n].fBottomRadius = _rBotRadius;
				_CylinderParam[n].fHei = _rHeight;
				_CylinderParam[n].Create();
			}
		}
	}

	_bBillBoard = pEff->_bBillBoard;
	_bRotaLoop = pEff->_bRotaLoop;
	_vRotaLoop = pEff->_vRotaLoop;
	_bRotaBoard = pEff->_bRotaBoard;
	_bAlpha = pEff->_bAlpha;

	_iVSIndex = pEff->_iVSIndex;


	_bSizeSame = pEff->_bSizeSame;
	_bAngleSame = pEff->_bAngleSame;
	_bPosSame = pEff->_bPosSame;
	_bColorSame = pEff->_bColorSame;

	_eSrcBlend = pEff->_eSrcBlend;
	_eDestBlend = pEff->_eDestBlend;
}

bool I_Effect::IsChangeably() {
	if (_pCModel)
		return _pCModel->IsChangeably();
	return true;
}

void I_Effect::IsSame() {
	_bSizeSame = true;
	_bAngleSame = true;
	_bPosSame = true;
	_bColorSame = true;

	int m;
	for (int n = 1; n < _wFrameCount; n++) {
		m = n - 1;
		if (_vecFrameSize[n] != _vecFrameSize[m])
			_bSizeSame = false;
		if (_vecFrameAngle[n] != _vecFrameAngle[m])
			_bAngleSame = false;
		if (_vecFramePos[n] != _vecFramePos[m])
			_bPosSame = false;
		if (_vecFrameColor[n] != _vecFrameColor[m])
			_bColorSame = false;
	}
}

void I_Effect::SpliteTexture(int iRow, int iCol) {
	if (_eEffectType == EFFECT_FRAMETEX) {
	}
	else
		_CTextruelist.CreateSpliteTexture(iRow, iCol);
}

void I_Effect::SetTextureTime(float ftime) {
	if (_eEffectType == EFFECT_FRAMETEX) {
		_CTexFrame._fFrameTime = ftime;
	}
	else
		_CTextruelist._fFrameTime = ftime;
}

void I_Effect::SetTexture() {
	if (IsItem()) {
		lwITex* tex = NULL;
		lwITex* tex2 = NULL;

		if (_eEffectType == EFFECT_FRAMETEX) {
			if (_CTexFrame._lpCurTex && _CTexFrame._lpCurTex->IsLoadingOK()) {
				_pCModel->ResetItemTexture(0, _CTexFrame._lpCurTex, &tex);
				_pCModel->ResetItemTexture(1, _CTexFrame._lpCurTex, &tex2);
			}
		}
		else {
			if (_CTextruelist._pTex && _CTextruelist._pTex->IsLoadingOK()) {
				_pCModel->ResetItemTexture(0, _CTextruelist._pTex, &tex);
				_pCModel->ResetItemTexture(1, _CTextruelist._pTex, &tex2);
			}
			else
				return;
		}
	}
	else {
		if (_eEffectType == EFFECT_FRAMETEX) {
			if (_CTexFrame._lpCurTex && _CTexFrame._lpCurTex->IsLoadingOK())
				_dev->SetTexture(0, _CTexFrame._lpCurTex->GetTex());
		}
		else {
			if (_CTextruelist._pTex && _CTextruelist._pTex->IsLoadingOK())
				_dev->SetTexture(0, _CTextruelist._pTex->GetTex());
			else
				return;
		}
	}
}

void I_Effect::SetVertexShader() {
	_dev->SetVertexShader(CMPResManger::Instance().GetVShaderByID(_iVSIndex));
	_dev->SetVertexDeclaration(CMPResManger::Instance().GetVDeclByID(_iVSIndex));
}

void I_Effect::Render() {
	_dev->SetRenderState(D3DRS_SRCBLEND, _eSrcBlend);
	_dev->SetRenderState(D3DRS_DESTBLEND, _eDestBlend);

	if (_pCModel) {
		if (IsUseParam())
			_pCModel->RenderTob(&_CylinderParam[_ilast], &_CylinderParam[_inext], _flerp);
		else
			_pCModel->RenderModel();
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
CEffectModel::CEffectModel() {
	_dev = NULL;
	_strName = "";

	_lwMesh = NULL;
	_pRes = NULL;
	_vEffVer = 0;

	_dwVerCount = 0;
	_dwFaceCount = 0;

	_bChangeably = false;
	_oldtex = NULL;
	_bItem = false;
}

bool CEffectModel::Copy(const CEffectModel& rhs) {
	ReleaseModel();

	MPSceneItem::Copy(&rhs);

	InitDevice(rhs._dev, rhs._pRes);

	GetObject()->GetPrimitive()->SetState(STATE_TRANSPARENT, 0);

	_strName = rhs._strName;

	return true;
}

CEffectModel::CEffectModel(MPRender* pDev, IResourceMgr* pRes) {
	_dev = pDev;
	_strName = "";

	_lwMesh = NULL;

	_pRes = pRes;
	_vEffVer = 0;

	_dwVerCount = 0;
	_dwFaceCount = 0;
	_bChangeably = false;
	_oldtex = NULL;
	_oldtex2 = NULL;

	_bItem = false;

	_iID = -1;
}

CEffectModel::~CEffectModel() {
	ReleaseModel();
}

void CEffectModel::InitDevice(MPRender* pDev, IResourceMgr* pRes) {
	_dev = pDev;
	_pRes = pRes;
}

MPRender* CEffectModel::GetDev() {
	return _dev;
}

void CEffectModel::ReleaseModel() {
	if (!_lwMesh) {
		if (_bItem) {
			lwITex* oldtex;
			this->ResetItemTexture(0, _oldtex, &oldtex);
			this->ResetItemTexture(1, _oldtex2, &oldtex);

			Destroy();
		}
	}
	SAFE_RELEASE(_lwMesh);

	SAFE_DELETE_ARRAY(_vEffVer);
	_bItem = false;

	_iID = -1;
}


bool CEffectModel::CreateTriangle() {
	//// mesh
	//// test lock method
	//vbrender
	_dwVerCount = 3;
	_dwFaceCount = 1;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);


	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLEFAN;
	mi.subset_num = 1;
	mi.vertex_num = 3;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.vertex_seq[0] = lwVector3(-0.5f, 0, 0);
	mi.vertex_seq[1] = lwVector3(0, 0, 0.5f);
	mi.vertex_seq[2] = lwVector3(0.5f, 0, 0);
	mi.blend_seq[0].weight[0] = 0;
	mi.blend_seq[1].weight[0] = 1;
	mi.blend_seq[2].weight[0] = 2;
	mi.vercol_seq[0] = 0xffffffff;
	mi.vercol_seq[1] = 0xffffffff;
	mi.vercol_seq[2] = 0xffffffff;
	mi.texcoord0_seq[0] = lwVector2(0.0f, 1.0f);
	mi.texcoord0_seq[1] = lwVector2(0.5f, 0);
	mi.texcoord0_seq[2] = lwVector2(1.0f, 1.0f);
	lwSubsetInfo_Construct(&mi.subset_seq[0], 1, 0, 3, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (TRI): vertex_num={}, ret={}",
					 __FUNCTION__, mi.vertex_num, static_cast<long long>(r));
		return 0;
	}

	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (TRI): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return 0;
	}

	_lpSVB = _lwMesh->GetLockableStreamVB();
	_lpSIB = _lwMesh->GetLockableStreamIB();


	_strName = MESH_TRI;

	return true;
}

bool CEffectModel::CreatePlaneTriangle() {
	_dwVerCount = 3;
	_dwFaceCount = 1;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);
	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLEFAN;
	mi.subset_num = 1;
	mi.vertex_num = 3;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.vertex_seq[0] = lwVector3(-0.5f, 0.5f, 0);
	mi.vertex_seq[1] = lwVector3(0, -0.5f, 0);
	mi.vertex_seq[2] = lwVector3(0.5f, 0.5f, 0);
	mi.blend_seq[0].weight[0] = 0;
	mi.blend_seq[1].weight[0] = 1;
	mi.blend_seq[2].weight[0] = 2;
	mi.vercol_seq[0] = 0xffffffff;
	mi.vercol_seq[1] = 0xffffffff;
	mi.vercol_seq[2] = 0xffffffff;
	mi.texcoord0_seq[0] = lwVector2(0.0f, 1.0f);
	mi.texcoord0_seq[1] = lwVector2(0.5f, 0);
	mi.texcoord0_seq[2] = lwVector2(1.0f, 1.0f);
	lwSubsetInfo_Construct(&mi.subset_seq[0], 1, 0, 3, 0);


	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (PLANETRI): vertex_num={}, ret={}",
					 __FUNCTION__, mi.vertex_num, static_cast<long long>(r));
		return 0;
	}

	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (PLANETRI): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return 0;
	}

	_lpSVB = _lwMesh->GetLockableStreamVB();
	_lpSIB = _lwMesh->GetLockableStreamIB();

	_strName = MESH_PLANETRI;
	return true;
}

bool CEffectModel::CreateRect() {
	_dwVerCount = 4;
	_dwFaceCount = 2;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);

	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLEFAN;
	mi.subset_num = 1;
	mi.vertex_num = 4;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.vertex_seq[0] = lwVector3(-0.5f, 0, 0);
	mi.vertex_seq[1] = lwVector3(-0.5f, 0, 1.0f);
	mi.vertex_seq[2] = lwVector3(0.5f, 0, 1.0f);
	mi.vertex_seq[3] = lwVector3(0.5f, 0, 0);

	mi.blend_seq[0].weight[0] = 0;
	mi.blend_seq[1].weight[0] = 1;
	mi.blend_seq[2].weight[0] = 2;
	mi.blend_seq[3].weight[0] = 3;

	mi.vercol_seq[0] = 0xffffffff;
	mi.vercol_seq[1] = 0xffffffff;
	mi.vercol_seq[2] = 0xffffffff;
	mi.vercol_seq[3] = 0xffffffff;

	mi.texcoord0_seq[0] = lwVector2(0.0f, 1.0f);
	mi.texcoord0_seq[1] = lwVector2(0.0f, 0);
	mi.texcoord0_seq[2] = lwVector2(1.0f, 0.0f);
	mi.texcoord0_seq[3] = lwVector2(1.0f, 1.0f);

	lwSubsetInfo_Construct(&mi.subset_seq[0], 2, 0, 4, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (RECT): vertex_num={}, ret={}",
					 __FUNCTION__, mi.vertex_num, static_cast<long long>(r));
		return false;
	}
	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (RECT): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return false;
	}
	_lpSVB = _lwMesh->GetLockableStreamVB();
	_lpSIB = _lwMesh->GetLockableStreamIB();

	_strName = MESH_RECT;

	return true;
}

bool CEffectModel::CreateRectZ() {
	_dwVerCount = 4;
	_dwFaceCount = 2;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);

	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLEFAN;
	mi.subset_num = 1;
	mi.vertex_num = 4;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.vertex_seq[0] = lwVector3(0, 0, 0);
	mi.vertex_seq[1] = lwVector3(0, 0, 1);
	mi.vertex_seq[2] = lwVector3(0, 1, 1);
	mi.vertex_seq[3] = lwVector3(0, 1, 0);

	mi.blend_seq[0].weight[0] = 0;
	mi.blend_seq[1].weight[0] = 1;
	mi.blend_seq[2].weight[0] = 2;
	mi.blend_seq[3].weight[0] = 3;

	mi.vercol_seq[0] = 0xffffffff;
	mi.vercol_seq[1] = 0xffffffff;
	mi.vercol_seq[2] = 0xffffffff;
	mi.vercol_seq[3] = 0xffffffff;

	mi.texcoord0_seq[0] = lwVector2(0.0f, 1.0f);
	mi.texcoord0_seq[1] = lwVector2(0.0f, 0);
	mi.texcoord0_seq[2] = lwVector2(1.0f, 0.0f);
	mi.texcoord0_seq[3] = lwVector2(1.0f, 1.0f);

	lwSubsetInfo_Construct(&mi.subset_seq[0], 2, 0, 4, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (RECTZ): vertex_num={}, ret={}",
					 __FUNCTION__, mi.vertex_num, static_cast<long long>(r));
		return false;
	}
	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (RECTZ): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return false;
	}
	_lpSVB = _lwMesh->GetLockableStreamVB();
	_lpSIB = _lwMesh->GetLockableStreamIB();

	_strName = MESH_RECTZ;

	return true;
}

bool CEffectModel::CreatePlaneRect() {
	_dwVerCount = 4;
	_dwFaceCount = 2;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);
	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLEFAN;
	mi.subset_num = 1;
	mi.vertex_num = 4;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.vertex_seq[0] = lwVector3(-0.5f, -0.5f, 0);
	mi.vertex_seq[1] = lwVector3(-0.5f, 0.5f, 0);
	mi.vertex_seq[2] = lwVector3(0.5f, 0.5f, 0);
	mi.vertex_seq[3] = lwVector3(0.5f, -0.5f, 0);

	mi.blend_seq[0].weight[0] = 0;
	mi.blend_seq[1].weight[0] = 1;
	mi.blend_seq[2].weight[0] = 2;
	mi.blend_seq[3].weight[0] = 3;

	mi.vercol_seq[0] = 0xffffffff;
	mi.vercol_seq[1] = 0xffffffff;
	mi.vercol_seq[2] = 0xffffffff;
	mi.vercol_seq[3] = 0xffffffff;

	mi.texcoord0_seq[0] = lwVector2(0.0f, 1.0f);
	mi.texcoord0_seq[1] = lwVector2(0.0f, 0);
	mi.texcoord0_seq[2] = lwVector2(1.0f, 0.0f);
	mi.texcoord0_seq[3] = lwVector2(1.0f, 1.0f);

	lwSubsetInfo_Construct(&mi.subset_seq[0], 2, 0, 4, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (PLANERECT): vertex_num={}, ret={}",
					 __FUNCTION__, mi.vertex_num, static_cast<long long>(r));
		return false;
	}
	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (PLANERECT): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return false;
	}
	_lpSVB = _lwMesh->GetLockableStreamVB();
	_lpSIB = _lwMesh->GetLockableStreamIB();

	_strName = MESH_PLANERECT;

	return true;
}


bool CEffectModel::CreateCone(int nSeg, float fHei, float fRadius) {
	_nSegments = nSeg;
	_rHeight = fHei;
	_rRadius = 0;
	_rBotRadius = fRadius;

	_dwVerCount = _nSegments * 3;
	_dwFaceCount = _nSegments * 2;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);

	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLESTRIP;
	mi.subset_num = 1;
	mi.vertex_num = _dwVerCount;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);

	int nCurrentSegment;
	int idx = 0;

	float rDeltaSegAngle = (2.0f * D3DX_PI / _nSegments);
	float rSegmentLength = 1.0f / (float)_nSegments;
	float ny0 = (90.0f - (float)D3DXToDegree(atan(_rHeight / _rBotRadius))) / 90.0f;
	for (nCurrentSegment = 0; nCurrentSegment <= _nSegments; nCurrentSegment++) {
		float x0 = _rBotRadius * sinf(nCurrentSegment * rDeltaSegAngle);
		float z0 = _rBotRadius * cosf(nCurrentSegment * rDeltaSegAngle);

		mi.vertex_seq[idx].x = 0.0f;
		mi.vertex_seq[idx].z = _rHeight;
		mi.vertex_seq[idx].y = 0.0f;
		mi.texcoord0_seq[idx].x = 1.0f - (rSegmentLength * (float)nCurrentSegment);
		mi.texcoord0_seq[idx].y = 0.0f;
		idx++;

		mi.vertex_seq[idx].x = x0;
		mi.vertex_seq[idx].z = 0.0f;
		mi.vertex_seq[idx].y = z0;
		mi.texcoord0_seq[idx].x = 1.0f - (rSegmentLength * (float)nCurrentSegment);
		mi.texcoord0_seq[idx].y = 1.5f;
		idx++;
	}
	for (WORD n = 0; n < _dwVerCount; n++) {
		mi.blend_seq[n].weight[0] = n;
		mi.vercol_seq[n] = 0xffffffff;
	}

	lwSubsetInfo_Construct(&mi.subset_seq[0], _dwFaceCount, 0, _dwVerCount, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (CONE): vertex_num={}, face_count={}, ret={}",
					 __FUNCTION__, _dwVerCount, _dwFaceCount, static_cast<long long>(r));
		return 0;
	}

	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (CONE): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return 0;
	}
	_lpSVB = _lwMesh->GetLockableStreamVB();

	_strName = MESH_CONE;
	_bChangeably = true;
	return true;
}

bool CEffectModel::CreateCylinder(int nSeg, float fHei, float fTopRadius, float fBottomRadius) {
	_nSegments = nSeg;
	_rHeight = fHei;
	_rRadius = fTopRadius;
	_rBotRadius = fBottomRadius;

	_dwVerCount = _nSegments * 3;
	_dwFaceCount = _nSegments * 2;

	if (_lwMesh != NULL)
		SAFE_RELEASE(_lwMesh);

	if (_vEffVer != NULL)
		SAFE_DELETE_ARRAY(_vEffVer);

	_vEffVer = new SEFFECT_VERTEX[_dwVerCount];
	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_VER_FVF;
	mi.pt_type = D3DPT_TRIANGLESTRIP;
	mi.subset_num = 1;
	mi.vertex_num = _dwVerCount;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);

	int nCurrentSegment;
	int idx = 0;

	float rDeltaSegAngle = (2.0f * D3DX_PI / _nSegments);
	float rSegmentLength = 1.0f / (float)_nSegments;
	float ny0 = (90.0f - (float)D3DXToDegree(atan(_rHeight / _rRadius))) / 90.0f;
	for (nCurrentSegment = 0; nCurrentSegment <= _nSegments; nCurrentSegment++) {
		float x0 = _rRadius * sinf(nCurrentSegment * rDeltaSegAngle);
		float z0 = _rRadius * cosf(nCurrentSegment * rDeltaSegAngle);

		mi.vertex_seq[idx].x = x0;
		mi.vertex_seq[idx].z = _rHeight;
		mi.vertex_seq[idx].y = z0;

		_vEffVer[idx]._SPos = (D3DXVECTOR3)mi.vertex_seq[idx];

		mi.texcoord0_seq[idx].x = 1.0f - (rSegmentLength * (float)nCurrentSegment);
		mi.texcoord0_seq[idx].y = 0.0f;

		_vEffVer[idx]._SUV = (D3DXVECTOR2)mi.texcoord0_seq[idx];

		idx++;

		x0 = _rBotRadius * sinf(nCurrentSegment * rDeltaSegAngle);
		z0 = _rBotRadius * cosf(nCurrentSegment * rDeltaSegAngle);
		mi.vertex_seq[idx].x = x0;
		mi.vertex_seq[idx].z = 0.0f;
		mi.vertex_seq[idx].y = z0;

		_vEffVer[idx]._SPos = (D3DXVECTOR3)mi.vertex_seq[idx];

		mi.texcoord0_seq[idx].x = 1.0f - (rSegmentLength * (float)nCurrentSegment);
		mi.texcoord0_seq[idx].y = 1.0f;

		_vEffVer[idx]._SUV = (D3DXVECTOR2)mi.texcoord0_seq[idx];

		idx++;
	}
	for (WORD n = 0; n < _dwVerCount; n++) {
		mi.blend_seq[n].weight[0] = n;
		mi.vercol_seq[n] = 0xffffffff;

		_vEffVer[n]._fIdx = n;
		_vEffVer[n]._dwDiffuse = 0xffffffff;
	}

	lwSubsetInfo_Construct(&mi.subset_seq[0], _dwFaceCount, 0, _dwVerCount, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (CYLINDER): vertex_num={}, face_count={}, ret={}",
					 __FUNCTION__, _dwVerCount, _dwFaceCount, static_cast<long long>(r));
		return 0;
	}

	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (CYLINDER): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
		return 0;
	}
	_lpSVB = _lwMesh->GetLockableStreamVB();

	_strName = MESH_CYLINDER;
	_bChangeably = true;
	return true;
}

bool CEffectModel::CreateShadeModel(WORD wVerNum, WORD wFaceNum, int iGridCrossNum, bool usesoft) {
	_dwVerCount = wVerNum;
	_dwFaceCount = wFaceNum;

	_pRes->CreateMesh(&_lwMesh);
	_lwMesh->SetStreamType(STREAM_LOCKABLE);

	lwMeshInfo mi;
	mi.fvf = EFFECT_SHADE_FVF;
	mi.pt_type = D3DPT_TRIANGLELIST;
	mi.subset_num = 1;
	mi.vertex_num = wVerNum;
	mi.vertex_seq = LW_NEW(lwVector3[mi.vertex_num]);
	mi.blend_seq = LW_NEW(lwBlendInfo[mi.vertex_num]);
	mi.vercol_seq = LW_NEW(DWORD[mi.vertex_num]);
	mi.texcoord0_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.texcoord1_seq = LW_NEW(lwVector2[mi.vertex_num]);
	mi.subset_seq = LW_NEW(lwSubsetInfo[mi.subset_num]);
	mi.index_num = wFaceNum * 6;
	mi.index_seq = LW_NEW(DWORD[mi.index_num ]);
	int nIndex = 9; //!VS9
	for (DWORD n = 0; n < mi.vertex_num; n++) {
		mi.vercol_seq[n] = 0xffffffff;
		mi.vertex_seq[n] = lwVector3(0, 0, 0); //!
		mi.texcoord0_seq[n] = lwVector2(0, 0);
		mi.texcoord1_seq[n].x = (float)nIndex;
		nIndex++;
		mi.texcoord1_seq[n].y = (float)nIndex;
		nIndex++;
	}


	nIndex = 0;
	for (int nY = 0; nY < iGridCrossNum; nY++) {
		for (int nX = 0; nX < iGridCrossNum; nX++) {
			mi.index_seq[nIndex++] = nX + nY * (iGridCrossNum + 1);
			mi.index_seq[nIndex++] = (nX + 1) + nY * (iGridCrossNum + 1);
			mi.index_seq[nIndex++] = nX + (nY + 1) * (iGridCrossNum + 1);

			mi.index_seq[nIndex++] = nX + (nY + 1) * (iGridCrossNum + 1);
			mi.index_seq[nIndex++] = (nX + 1) + nY * (iGridCrossNum + 1);
			mi.index_seq[nIndex++] = (nX + 1) + (nY + 1) * (iGridCrossNum + 1);
		}
	}
	lwSubsetInfo_Construct(&mi.subset_seq[0], wFaceNum, 0, wVerNum, 0);

	if (LW_RESULT r = _lwMesh->LoadSystemMemory(&mi); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadSystemMemory failed (SHADE): vertex_num={}, face_num={}, grid_cross={}, ret={}",
					 __FUNCTION__, mi.vertex_num, wFaceNum, iGridCrossNum, static_cast<long long>(r));
	}
	if (LW_RESULT r = _lwMesh->LoadVideoMemory(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] LoadVideoMemory failed (SHADE): ret={}",
					 __FUNCTION__, static_cast<long long>(r));
	}

	if (usesoft) {
		_lpSVB = _lwMesh->GetLockableStreamVB();
		_lpSIB = _lwMesh->GetLockableStreamIB();
	}
	else {
		_lpSVB = NULL;
		_lpSIB = NULL;
	}
	return true;
}

bool CEffectModel::LoadModel(const char* pszName) {
	_strName = pszName;


	// begin by lsh
	if (Load(pszName) == 0) {
		_oldtex = this->GetPrimitive()->GetMtlTexAgent(0)->GetTex(0);
		if (this->GetPrimitive()->GetMtlTexAgent(1))
			_oldtex2 = this->GetPrimitive()->GetMtlTexAgent(1)->GetTex(0);
		else
			_oldtex2 = NULL;
		_bItem = true;
	}
	else {
		_oldtex = NULL;
		_oldtex2 = NULL;
		_bItem = false;
	}
	// end


	return true;
}


void CEffectModel::FrameMove(DWORD dwDailTime) {
	if (!_lwMesh)
		MPSceneItem::FrameMove();
}

void CEffectModel::Begin() {
	//vs
	if (_lwMesh) {
		if (LW_RESULT r = _lwMesh->BeginSet(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] _lwMesh->BeginSet failed: name={}, ret={}",
						 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		}
	}
}

void CEffectModel::SetRenderNum(WORD wVer, WORD wFace) {
	lwMeshInfo* mi = _lwMesh->GetMeshInfo();
	lwSubsetInfo_Construct(&mi->subset_seq[0], wFace, 0, wVer, 0);
}

void CEffectModel::RenderModel() {
	if (!_lwMesh) {
		if (LW_RESULT r = this->GetPrimitive()->RenderSubset(0); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] GetPrimitive()->RenderSubset(0) failed: name={}, ret={}",
						 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		}
		if (this->GetPrimitive()->GetMtlTexAgent(1)) {
			_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, TRUE);
			if (LW_RESULT r = this->GetPrimitive()->RenderSubset(1); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] GetPrimitive()->RenderSubset(1) failed: name={}, ret={}",
							 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
			}
		}
	}
	else {
		if (LW_RESULT r = _lwMesh->DrawSubset(0); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] DrawSubset failed: name={}, ret={}",
						 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		}
	}
}

void CEffectModel::End() {
	//this been added when we fixed circle shadow not sure 100% of it but so far it works
	//@moth
	_dev->SetVertexShader(nullptr);
	_dev->SetFVF(EFFECT_VER_FVF);
	//end
	if (_lwMesh) {
		if (LW_RESULT r = _lwMesh->EndSet(); LW_FAILED(r)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] _lwMesh->EndSet failed: name={}, ret={}",
						 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		}
	}
	//same as commit before
	_dev->SetRenderState(D3DRS_ZENABLE, TRUE);
	_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CEffectModel::RenderTob(ModelParam* last, ModelParam* next, float lerp) {
	for (WORD n = 0; n < _dwVerCount; ++n) {
		D3DXVec3Lerp(&_vEffVer[n]._SPos, &last->vecVer[n], &next->vecVer[n], lerp);
	}
	_dev->SetVertexShader(NULL);
	_dev->SetFVF(EFFECT_VER_FVF);

	_dev->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, _dwFaceCount, _vEffVer, sizeof(SEFFECT_VERTEX));
}

void ModelParam::Create() {
	DWORD dwVerCount = iSegments * 3;

	vecVer.resize(dwVerCount);

	int nCurrentSegment;

	int idx = 0;

	float rDeltaSegAngle = (2.0f * D3DX_PI / iSegments);
	float rSegmentLength = 1.0f / (float)iSegments;
	float ny0 = (90.0f - (float)D3DXToDegree(atan(fHei / fTopRadius))) / 90.0f;
	for (nCurrentSegment = 0; nCurrentSegment <= iSegments; nCurrentSegment++) {
		float x0 = fTopRadius * sinf(nCurrentSegment * rDeltaSegAngle);
		float z0 = fTopRadius * cosf(nCurrentSegment * rDeltaSegAngle);

		vecVer[idx].x = x0;
		vecVer[idx].z = fHei;
		vecVer[idx].y = z0;

		idx++;

		x0 = fBottomRadius * sinf(nCurrentSegment * rDeltaSegAngle);
		z0 = fBottomRadius * cosf(nCurrentSegment * rDeltaSegAngle);
		vecVer[idx].x = x0;
		vecVer[idx].z = 0.0f;
		vecVer[idx].y = z0;

		idx++;
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
CTexCoordList::CTexCoordList() {
	Clear();
}

CTexCoordList::~CTexCoordList() {
}

void CTexCoordList::Clear() {
	_wVerCount = 0;
	_wCoordCount = 0;
	_fFrameTime = 0.0f;
	_vecCoordList.clear();

	////!
	////!
}


void CTexCoordList::CreateTranslateCoord() {
	_fFrameTime = 4.0f;

	_wVerCount = 4;
	_wCoordCount = 2;
	_vecCoordList.resize(_wCoordCount);
	D3DXVECTOR2 t_SVer[4];
	_vecCoordList[0].resize(_wVerCount);
	_vecCoordList[1].resize(_wVerCount);
	t_SVer[0] = D3DXVECTOR2(0, 0.1f);
	t_SVer[1] = D3DXVECTOR2(0, 0.0f);
	t_SVer[2] = D3DXVECTOR2(1.0f, 0.0f);
	t_SVer[3] = D3DXVECTOR2(1.0f, 0.1f);

	for (WORD i = 0; i < _wVerCount; ++i) {
		_vecCoordList[0][i] = t_SVer[i];
	}
	t_SVer[0] = D3DXVECTOR2(0, 1.2f);
	t_SVer[1] = D3DXVECTOR2(0, 0.0f);
	t_SVer[2] = D3DXVECTOR2(1.0f, 0.0f);
	t_SVer[3] = D3DXVECTOR2(1.0f, 1.2f);

	for (WORD i = 0; i < _wVerCount; ++i) {
		_vecCoordList[1][i] = t_SVer[i];
	}
}

void CTexCoordList::GetCoordFromModel(CEffectModel* pCModel) {
	if (!pCModel->IsBoard())
		return;
	_wVerCount = (WORD)pCModel->GetVerCount();
	_wCoordCount = 1;
	_fFrameTime = 3.0f;
	_vecCoordList.clear();
	_vecCoordList.resize(_wCoordCount);

	SEFFECT_VERTEX* pVertex;
	pCModel->Lock((BYTE**)&pVertex);

	for (WORD i = 0; i < _wCoordCount; i++) {
		_vecCoordList[i].resize(_wVerCount);
		for (DWORD n = 0; n < _wVerCount; n++) {
			_vecCoordList[i][n] = pVertex[n]._SUV;
		}
	}

	pCModel->Unlock();
}

void CTexCoordList::Reset() {
	////!
	////!
}

void CTexCoordList::GetCurCoord(S_BVECTOR<D3DXVECTOR2>& vecOutCoord, WORD& wCurIndex, float& fCurTime,
								float fDailTime) {
	if (_wCoordCount == 1) {
		vecOutCoord.clear();
		for (int n = 0; n < _wVerCount; ++n) {
			vecOutCoord.push_back(_vecCoordList[0][n]);
		}
		return;
	}
	WORD t_wNextIndex;
	float t_fLerp;
	fCurTime += fDailTime;
	if (fCurTime > _fFrameTime) {
		wCurIndex++;
		fCurTime = 0.0f;
	}
	if (wCurIndex >= _wCoordCount) {
		wCurIndex = 0;
	}
	if (wCurIndex == _wCoordCount - 1) {
		t_wNextIndex = wCurIndex;
	}
	else {
		t_wNextIndex = wCurIndex + 1;
	}
	t_fLerp = fCurTime / _fFrameTime;

	for (WORD n = 0; n < _wVerCount; ++n) {
		D3DXVec2Lerp(vecOutCoord[n],
					 &_vecCoordList[wCurIndex][n],
					 &_vecCoordList[t_wNextIndex][n], t_fLerp);
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
CTexList::CTexList() {
	Clear();
}

CTexList::~CTexList() {
}

void CTexList::Clear() {
	_wTexCount = 0;
	_fFrameTime = 0.1f;

	_vecTexName = "";
	_vecTexList.clear();

	_lpCurTex = NULL;
}

void CTexList::SetTextureName(const s_string& pszName) {
	_vecTexName = pszName;
}

void CTexList::Reset() {
	////!
	////!
}

//!
void CTexList::CreateSpliteTexture(int iRow, int iColnum) {
	_wTexCount = iRow * iColnum;
	float fw = 1.0f / iRow;
	float fh = 1.0f / iColnum;

	D3DXVECTOR2 suv[4];
	suv[0] = D3DXVECTOR2(0, 1.0f);
	suv[1] = D3DXVECTOR2(0, 0);
	suv[2] = D3DXVECTOR2(1.0f, 0.0f);
	suv[3] = D3DXVECTOR2(1.0f, 1.0f);

	_vecTexList.clear();
	_vecTexList.resize(_wTexCount);
	for (WORD h = 0; h < iColnum; h++) {
		for (WORD w = 0; w < iRow; w++) {
			_vecTexList[w + h * iRow].resize(4);

			_vecTexList[w + h * iRow][0].x = w * fw + suv[0].x;
			_vecTexList[w + h * iRow][0].y = h * fh + fh;

			_vecTexList[w + h * iRow][1].x = w * fw + suv[1].x;
			_vecTexList[w + h * iRow][1].y = h * fh;

			_vecTexList[w + h * iRow][2].x = _vecTexList[w + h * iRow][1].x + fw;
			_vecTexList[w + h * iRow][2].y = _vecTexList[w + h * iRow][1].y;

			_vecTexList[w + h * iRow][3].x = _vecTexList[w + h * iRow][0].x + fw;
			_vecTexList[w + h * iRow][3].y = _vecTexList[w + h * iRow][0].y;
		}
	}
}

void CTexList::GetTextureFromModel(CEffectModel* pCModel) {
	if (!pCModel->IsBoard())
		return;
	//	return;

	WORD t_wVerCount = (WORD)pCModel->GetVerCount();
	_wTexCount = 1;
	_vecTexList.clear();
	_vecTexList.resize(_wTexCount);

	SEFFECT_VERTEX* pVertex;
	pCModel->Lock((BYTE**)&pVertex);

	for (WORD i = 0; i < _wTexCount; i++) {
		_vecTexList[i].resize(t_wVerCount);
		for (WORD n = 0; n < t_wVerCount; n++) {
			_vecTexList[i][n] = pVertex[n]._SUV;
		}
	}

	pCModel->Unlock();
}

void CTexList::GetCurTexture(S_BVECTOR<D3DXVECTOR2>& coord, WORD& wCurIndex, float& fCurTime, float fDailTime) {
	if (_wTexCount == 1) {
		for (WORD i = 0; i < (WORD)coord.size(); ++i) {
			*coord[i] = _vecTexList[0][i];
		}
		return;
	}
	fCurTime += fDailTime;
	if (fCurTime > _fFrameTime) {
		wCurIndex++;
		fCurTime = 0.0f;
	}
	if (wCurIndex >= _wTexCount) {
		wCurIndex = 0;
	}
	for (WORD i = 0; i < coord.size(); ++i) {
		*coord[i] = _vecTexList[wCurIndex][i];
	}
}

void CTexList::Remove() {
	if (_wTexCount <= 1) {
		return;
	}
	_vecTexList.pop_back();
	_wTexCount--;
}

CTexFrame::CTexFrame() {
	_wTexCount = 0;
	_fFrameTime = 0.1f;

	_vecTexName.clear();
	_vecTexs.clear();

	_lpCurTex = NULL;
}

CTexFrame::~CTexFrame() {
	_vecTexName.clear();
	_vecTexs.clear();
}

void CTexFrame::GetCoordFromModel(CEffectModel* pCModel) {
	if (!pCModel->IsBoard())
		return;

	WORD t_wVerCount = (WORD)pCModel->GetVerCount();
	_vecCoord.clear();
	_vecCoord.resize(t_wVerCount);

	SEFFECT_VERTEX* pVertex;
	pCModel->Lock((BYTE**)&pVertex);
	for (WORD n = 0; n < t_wVerCount; n++) {
		_vecCoord[n] = pVertex[n]._SUV;
	}
	pCModel->Unlock();
}

void CTexFrame::AddTexture(const s_string& pszName) {
	_vecTexName.push_back(pszName);
	_wTexCount++;
	_vecTexs.resize(_wTexCount);
}

lwITex* CTexFrame::GetCurTexture(WORD& wCurIndex, float& fCurTime, float fDailTime) {
	if (_wTexCount == 0) {
		return _lpCurTex = NULL;
	}
	if (_wTexCount == 1) {
		return _lpCurTex = _vecTexs[0];
	}
	fCurTime += fDailTime;
	if (fCurTime > _fFrameTime) {
		wCurIndex++;
		fCurTime = 0.0f;
	}
	if (wCurIndex >= _wTexCount) {
		wCurIndex = 0;
	}
	return _lpCurTex = _vecTexs[wCurIndex];
}

void CTexFrame::Remove() {
	_wTexCount = 0;
	_fFrameTime = 0.1f;

	_vecTexName.clear();
	_vecTexs.clear();

	_lpCurTex = NULL;
}

/************************************************************************/
/*CEffectFont*/
/************************************************************************/
CEffectFont::CEffectFont() {
	_strText = "0123456789+-";
	_iTextNum = 12;
	_vecTexName = "number";
	_strBackBmp = "backnumber";
	_lpBackTex = NULL;
	_bUseBack = FALSE;
	_iTextureID = -1;
}

CEffectFont::~CEffectFont() {
}

bool CEffectFont::CreateEffectFont(MPRender* pDev,
								   CMPResManger* pCResMagr, int iTexID, D3DXCOLOR dwColor, bool bUseBack, bool bmain) {
	//	return false;
	_pRes = pCResMagr->_pSysGraphics->GetResourceMgr();

	_dev = pDev;
	_bUseBack = bUseBack;
	_dwColor = dwColor;
	_iTextureID = iTexID;
	s_string str[] =
	{
		"addblood",
		"subblood",
		"addsp",
		"subsp",
		"addblood",
		"subblood",
		"bao",
		"submiss",
	};
	if (!bmain) {
		_vecTexName = str[_iTextureID];
	}
	else {
		_vecTexName = std::format("{}2", str[_iTextureID]);
	}
	int id = pCResMagr->GetTextureID(_vecTexName);
	if (id < 0) {
		g_logManager.LogError("errors", "CEffectFont texture {} not found", _vecTexName.c_str());
		_lpCurTex = NULL;
		_pTex = NULL;
	}
	else {
		_pTex = pCResMagr->GetTextureByIDlw(id);

		_lpCurTex = _pTex->GetTex();
	}

	id = pCResMagr->GetTextureID(_strBackBmp);
	if (id < 0) {
		g_logManager.LogError("errors", "CEffectFont texture {} not found", _strBackBmp.c_str());
		_lpBackTex = NULL;
	}
	else {
		_lpBackTex = pCResMagr->GetTextureByIDlw(id);
	}

	float fx = 0.5f;
	float fy = 0.7f;
	if (_iTextureID == 7) {
		_iTextNum = 1;
		_vecCurText.push_back(0);
		fx = 2.0f;
		fy = 1.0f;
	}
	else
		_iTextNum = 12;

	CreateSpliteTexture(_iTextNum, 1);

	_dwVerCount = _iTextNum * 2 * 3;
	_dwFaceCount = _iTextNum * 2;


	t_SEffVer[0]._SPos = D3DXVECTOR3(-fx, -fy, 0);
	t_SEffVer[0]._fIdx = 0;
	t_SEffVer[0]._dwDiffuse = 0xffffffff;
	t_SEffVer[0]._SUV = D3DXVECTOR2(0.0f, 1.0f);

	t_SEffVer[1]._SPos = D3DXVECTOR3(-fx, fy, 0);
	t_SEffVer[1]._fIdx = 1;
	t_SEffVer[1]._dwDiffuse = 0xffffffff;
	t_SEffVer[1]._SUV = D3DXVECTOR2(0.0f, 0);

	t_SEffVer[2]._SPos = D3DXVECTOR3(fx, fy, 0);
	t_SEffVer[2]._fIdx = 2;
	t_SEffVer[2]._dwDiffuse = 0xffffffff;
	t_SEffVer[2]._SUV = D3DXVECTOR2(1.0f, 0.0f);

	t_SEffVer[3]._SPos = D3DXVECTOR3(fx, -fy, 0);
	t_SEffVer[3]._fIdx = 3;
	t_SEffVer[3]._dwDiffuse = 0xffffffff;
	t_SEffVer[3]._SUV = D3DXVECTOR2(1.0f, 1.0f);

	//	D3DUSAGE_WRITEONLY |D3DUSAGE_DYNAMIC,
	//	EFFECT_VER_FVF,
	//	return false;


	_vEffVer = new SEFFECT_VERTEX[_dwVerCount];
	for (int m = 0; m < _iTextNum; m++) {
		for (int n = 0; n < 4; n++) {
			_vEffVer[m * 4 + n]._SPos.x = t_SEffVer[n]._SPos.x + m;
			_vEffVer[m * 4 + n]._SPos.y = t_SEffVer[n]._SPos.y;
			_vEffVer[m * 4 + n]._SPos.z = 0;
			_vEffVer[m * 4 + n]._fIdx = float(n);
			_vEffVer[m * 4 + n]._dwDiffuse = 0xffffffff;
			_vEffVer[m * 4 + n]._SUV = _vecTexList[m][n];
		}
	}

	//	D3DUSAGE_WRITEONLY |D3DUSAGE_DYNAMIC,
	//	EFFECT_VER_FVF,
	//	return false;

	_strName = "FONTEFFECT";
	return true;
}

void CEffectFont::SetRenderText(std::string_view pszText) {
	if (_iTextureID == 7)
		return;

	int pos;
	char pszt[3];
	const std::size_t len = pszText.size();
	_vecCurText.clear();

	for (std::size_t m = 0; m < len; m++) {
		if (static_cast<unsigned char>(pszText[m]) & 0x80) {
			pszt[0] = pszText[m];
			pszt[1] = pszText[m + 1];
			pszt[2] = '\0';
		}
		else {
			pszt[0] = pszText[m];
			pszt[1] = '\0';
			pszt[2] = '\0';
		}
		pos = (int)_strText.find(pszt);
		if (pos != -1) {
			_vecCurText.push_back(pos);
			for (int n = 0; n < 4; n++) {
				_vEffVer[m * 4 + n]._SUV = _vecTexList[pos][n];
			}
		}
	}


	t_SEffVer[0]._SPos.x -= 4.0f;
	t_SEffVer[0]._SPos.y -= 2.0f;
	t_SEffVer[1]._SPos.x -= 4.0f;
	t_SEffVer[1]._SPos.y += 2.0f;
	t_SEffVer[2]._SPos.x += len + 2.5f;
	t_SEffVer[2]._SPos.y += 2.0f;
	t_SEffVer[3]._SPos.x += len + 2.5f;
	t_SEffVer[3]._SPos.y -= 2.0f;
}

void CEffectFont::RenderEffectFontBack(D3DXMATRIX* pmat) {
	_dev->SetVertexShader(NULL);
	_dev->SetFVF(EFFECT_VER_FVF);

	if (_lpBackTex && _lpBackTex->IsLoadingOK())
		_dev->SetTexture(0, _lpBackTex->GetTex());
	else
		return;


	_dev->SetTransformWorld(pmat);

	if (HRESULT hr = _dev->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &t_SEffVer, sizeof(SEFFECT_VERTEX));
		FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] DrawPrimitiveUP failed (back): hr=0x{:08X}",
					 __FUNCTION__, static_cast<std::uint32_t>(hr));
	}
}

void CEffectFont::RenderEffectFont(D3DXMATRIX* pmat) {
	if (_bUseBack) {
		RenderEffectFontBack(pmat);
	}
	_dev->SetTransformWorld(pmat);


	if (_pTex && _pTex->IsLoadingOK())
		_dev->SetTexture(0, _pTex->GetTex());
	else
		return;


	for (int n = 0; n < (WORD)_vecCurText.size(); n++) {
		if (HRESULT hr = _dev->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &_vEffVer[n * 4],
															  sizeof(SEFFECT_VERTEX)); FAILED(hr)) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] DrawPrimitiveUP failed (text glyph): n={}, hr=0x{:08X}",
						 __FUNCTION__, n, static_cast<std::uint32_t>(hr));
		}
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
CCharacterActionCache CCharacterActionCache::_cache;

bool CCharacterActionCache::LoadActionDataFromStore() {
	Free();

	auto* store = Corsairs::Common::Character::CharacterActionStore::Instance();
	short maxType = store->GetMaxCharacterType();
	if (maxType < 1) {
		g_logManager.LogError("errors", "CCharacterActionCache::LoadActionDataFromStore: store пуст");
		return false;
	}

	_characterActions.resize(maxType);
	_maxCharacterType = maxType;

	for (short t = 0; t < maxType; t++) {
		_characterActions[t]._characterType = t + 1;
	}

	// Первый проход: определяем максимальный actionNo по каждому типу.
	std::vector<short> maxActionPerType(maxType, 0);
	store->ForEach([&](const Corsairs::Common::Character::CCharacterActionInfo& r) {
		int t = r._characterType - 1;
		if (t < 0 || t >= _maxCharacterType) {
			return;
		}

		maxActionPerType[t] = (std::max)(r._actionNo, maxActionPerType[t]);
	});

	// Резервируем слоты под actions каждого типа.
	for (short t = 0; t < _maxCharacterType; t++) {
		short maxAct = maxActionPerType[t];
		if (maxAct < 1) {
			continue;
		}
		_characterActions[t]._actions.resize(maxAct);
	}

	// Второй проход: заполнение данных.
	store->ForEach([&](const Corsairs::Common::Character::CCharacterActionInfo& r) {
		int t = r._characterType - 1;
		if (t < 0 || t >= _maxCharacterType) {
			return;
		}
		SChaAction& block = _characterActions[t];
		if (r._actionNo < 1 || static_cast<size_t>(r._actionNo) > block._actions.size()) {
			return;
		}

		ActionInfo& info = block._actions.at(r._actionNo - 1);
		info._actionNo = r._actionNo;
		info._startFrame = r._startFrame;
		info._endFrame = r._endFrame;
		info._keyFrames = r._keyFrames;
		block._validCount++;
	});

	// Подсчёт типов, в которых есть хотя бы одно действие.
	_actualCharacterType = 0;
	for (short t = 0; t < _maxCharacterType; t++) {
		if (_characterActions[t]._validCount > 0) {
			_actualCharacterType++;
		}
	}

	return true;
}

void CCharacterActionCache::Free() {
	_characterActions.clear();
	_maxCharacterType = 0;
	_actualCharacterType = 0;
}

const SChaAction* CCharacterActionCache::GetCharAction(int characterType) const {
	if (characterType < 1 || static_cast<size_t>(characterType) > _characterActions.size()) {
		return nullptr;
	}
	const SChaAction& block = _characterActions[characterType - 1];
	return block.IsValid() ? &block : nullptr;
}

long StringGetT(char* out, long out_max, const char* in, long* in_from, const char* end_list, long end_len) {
	long offset = -1; // set offset of get string to -1 for the first do process
	long i; // temp variable

	--(*in_from); // dec (*in_from) for the first do process
	do {
		out[++offset] = in[++(*in_from)];
		for (i = end_len - 1; i >= 0; --i) {
			if (out[offset] == end_list[i]) {
				out[offset] = 0x00;
				break;
			}
		}
	}
	while (out[offset] && offset < out_max);
	return offset;
}

void StringSkipCompartmentT(const char* in, long* in_from, const char* skip_list, long skip_len) {
	long i; // temp variable

	while (in[(*in_from)]) {
		for (i = skip_len - 1; i >= 0; --i) {
			if (in[(*in_from)] == skip_list[i])
				break;
		}
		if (i < 0) break; // dismatch skip conditions, finished
		else ++(*in_from); // match skip conditions, skip it
	}
}

s_string& I_Effect::getEffectModelName() {
	return _strModelName;
}

bool I_Effect::IsItem() {
	if (_pCModel) {
		return _pCModel->IsItem();
	}
	if (strstr(_strModelName.c_str(), ".lgo")) {
		return true;
	}
	return false;
}

void I_Effect::GetLerpSize(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp) {
	if (_wFrameCount == 1 || _bSizeSame) {
		*pSOut = _vecFrameSize[0];
		return;
	}
	D3DXVec3Lerp(pSOut, &_vecFrameSize[wIdx1], &_vecFrameSize[wIdx2], fLerp);
}

void I_Effect::GetLerpAngle(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp) {
	if (_wFrameCount == 1 || _bAngleSame) {
		*pSOut = _vecFrameAngle[0];
		return;
	}
	D3DXVec3Lerp(pSOut, &_vecFrameAngle[wIdx1], &_vecFrameAngle[wIdx2], fLerp);
}

void I_Effect::GetLerpPos(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp) {
	if (_wFrameCount == 1 || _bPosSame) {
		*pSOut = _vecFramePos[0];
		return;
	}
	D3DXVec3Lerp(pSOut, &_vecFramePos[wIdx1], &_vecFramePos[wIdx2], fLerp);
}

void I_Effect::GetLerpColor(D3DXCOLOR* pSOut, WORD wIdx1, WORD wIdx2, float fLerp) {
	if (_wFrameCount == 1 || _bColorSame) {
		*pSOut = _vecFrameColor[0];
		return;
	}
	D3DXColorLerp(pSOut, &_vecFrameColor[wIdx1], &_vecFrameColor[wIdx2], fLerp);
}

void CTexCoordList::Copy(CTexCoordList* pList) {
	_wVerCount = pList->_wVerCount;
	_wCoordCount = pList->_wCoordCount;
	_fFrameTime = pList->_fFrameTime;
	_vecCoordList.resize(_wCoordCount);
	for (int n = 0; n < _wCoordCount; ++n) {
		_vecCoordList[n].resize(_wVerCount);
		_vecCoordList[n] = pList->_vecCoordList[n];
	}
}

void CTexList::Copy(CTexList* pList) {
	_wTexCount = pList->_wTexCount;
	_fFrameTime = pList->_fFrameTime;
	_vecTexList.resize(_wTexCount);
	for (int n = 0; n < _wTexCount; ++n) {
		_vecTexList[n].resize(4);
		_vecTexList[n] = pList->_vecTexList[n];
	}
	_vecTexName = pList->_vecTexName;
	_lpCurTex = NULL;
	_pTex = NULL;
}

void CTexFrame::Copy(CTexFrame* pList) {
	_wTexCount = pList->_wTexCount;
	_fFrameTime = pList->_fFrameTime;
	_vecTexName.resize(_wTexCount);
	_vecTexs.resize(_wTexCount);
	for (int n = 0; n < _wTexCount; ++n) {
		_vecTexName[n] = pList->_vecTexName[n];
	}
	_vecCoord.resize(pList->_vecCoord.size());
	_vecCoord = pList->_vecCoord;
}

void CEffectModel::Lock(BYTE** pvEffVer) {
	if (_lpSVB == 0) {
		*pvEffVer = 0;
		return;
	}

	if (LW_RESULT r = _lpSVB->Lock(0, 0, (void**)pvEffVer, 0); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] _lpSVB->Lock failed: name={}, ret={}",
					 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		MessageBox(NULL, "lock error msglock error", "error", 0);
		*pvEffVer = 0;
		assert(false);
	}
}

void CEffectModel::Unlock() {
	if (LW_RESULT r = _lpSVB->Unlock(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] _lpSVB->Unlock failed: name={}, ret={}",
					 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
	}
}

void CEffectModel::LockIB(BYTE** pIdx) {
	if (LW_RESULT r = _lpSIB->Lock(0, 0, (void**)pIdx, 0); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] _lpSIB->Lock failed: name={}, ret={}",
					 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
		MessageBox(NULL, "lock error msglock error", "error", 0);
		assert(false);
	}
}

void CEffectModel::UnlockIB() {
	if (LW_RESULT r = _lpSIB->Unlock(); LW_FAILED(r)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] _lpSIB->Unlock failed: name={}, ret={}",
					 __FUNCTION__, _strName.c_str(), static_cast<long long>(r));
	}
}

void I_Effect::SetTobParam(int nFrame, int nSegments, float rHeight, float rRadius, float rBotRadius) {
	_iUseParam = 0;
	_CylinderParam[nFrame].iSegments = nSegments;
	_CylinderParam[nFrame].fTopRadius = rRadius;
	_CylinderParam[nFrame].fBottomRadius = rBotRadius;
	_CylinderParam[nFrame].fHei = rHeight;
	_CylinderParam[nFrame].Create();

	_iUseParam = 1;
}

void I_Effect::GetTobParam(int nFrame, int& nSegments, float& rHeight, float& rRadius, float& rBotRadius) {
	nSegments = _CylinderParam[nFrame].iSegments;
	rRadius = _CylinderParam[nFrame].fTopRadius;
	rBotRadius = _CylinderParam[nFrame].fBottomRadius;
	rHeight = _CylinderParam[nFrame].fHei;
}

void I_Effect::GetRotaLoopMatrix(D3DXMATRIX* pmat, float& pCurRota, float fTime) {
	pCurRota += _vRotaLoop.w * fTime;
	if (pCurRota >= 6.283185f) {
		pCurRota = pCurRota - 6.283185f;
	}
	const auto v = D3DXVECTOR3(_vRotaLoop.x, _vRotaLoop.y, _vRotaLoop.z);
	D3DXMatrixRotationAxis(pmat, &v, pCurRota);
}

void Transpose(D3DMATRIX& result, D3DMATRIX& m) {
	result.m[0][0] = m.m[0][0];
	result.m[0][1] = m.m[1][0];
	result.m[0][2] = m.m[2][0];
	result.m[0][3] = m.m[3][0];
	result.m[1][0] = m.m[0][1];
	result.m[1][1] = m.m[1][1];
	result.m[1][2] = m.m[2][1];
	result.m[1][3] = m.m[3][1];
	result.m[2][0] = m.m[0][2];
	result.m[2][1] = m.m[1][2];
	result.m[2][2] = m.m[2][2];
	result.m[2][3] = m.m[3][2];
	result.m[3][0] = m.m[0][3];
	result.m[3][1] = m.m[1][3];
	result.m[3][2] = m.m[2][3];
	result.m[3][3] = m.m[3][3];
}
