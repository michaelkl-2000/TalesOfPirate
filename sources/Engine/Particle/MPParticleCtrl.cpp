#include "StdAfx.h"
#include "GlobalInc.h"
#include "MPModelEff.h"
#include "MPRender.h"
#include ".\mpparticlectrl.h"
#include "Character/CharacterModelStore.h"
#include <cstring>
#include <algorithm>

using namespace std;

///************************************************************************/
///*                                                                      */
///************************************************************************/
//CMPParticleCtrl::~CMPParticleCtrl(void)
///************************************************************************/
///*                                                                      */
///************************************************************************/
//CMPParticleTrace::~CMPParticleTrace(void)
////float f = 0;
//	//
//		//! ID
//		return;

//		return;
//	//!
//		return;
//	//!
//	//!
//	//!.
//	//	 =  /  * 
//	//!
//	//!
///************************************************************************/
///*                                                                      */
///************************************************************************/
//CMPParticleRipple::~CMPParticleRipple(void)
//		//! ID
//		return;


/************************************************************************/
/* class CMPPartCtrl*/
/************************************************************************/
CMPPartCtrl::CMPPartCtrl(void) {
	_strName = "";
	_iPartNum = 0;
#ifndef USE_GAME
	//_vecPartSys.resize(MAX_PART_SYS);//6
#endif
	_fLength = 0; //0
	_fCurTime = 0;

	_iStripNum = 0;
	_pcStrip = NULL;

	_iModelNum = 0;
}

CMPPartCtrl::~CMPPartCtrl(void) {
	SAFE_DELETE_ARRAY(_pcStrip);
	for (int n = 0; n < _iModelNum; n++) {
		SAFE_DELETE(_vecModel[n]);
	}
	_vecModel.clear();
}

void CMPPartCtrl::SetStripCharacter(MPCharacter* pCha) {
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].AttachCharacter(pCha);
	}
}

void CMPPartCtrl::SetStripItem(MPSceneItem* pItem, bool bloop) {
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].AttachItem(pItem);
		_pcStrip[n].SetLoop(bloop);
	}
}

void CMPPartCtrl::SetItemDummy(MPSceneItem* pItem, int dummy1, int dummy2) {
	for (int n = 0; n < _iPartNum; ++n) {
		_vecPartSys[n]->SetItemDummy(pItem, dummy1, dummy2);
	}
}


CChaModel* CMPPartCtrl::NewModel(const s_string& strID, CMPResManger* pResMagr) {
	_iModelNum++;
	if (_iModelNum > 1) {
		_iModelNum = 1;
		return NULL;
	}
	_vecModel.resize(_iModelNum);

	_vecModel[_iModelNum - 1] = new CChaModel;
	_vecModel[_iModelNum - 1]->_dev = pResMagr->_dev;

	_vecModel[_iModelNum - 1]->LoadScript(strID);
	_vecModel[_iModelNum - 1]->SetVel(120);
	_vecModel[_iModelNum - 1]->SetPlayType(PLAY_LOOP);
	_vecModel[_iModelNum - 1]->SetCurPose(1);
	_vecModel[_iModelNum - 1]->PlayPose(0, PLAY_PAUSE);

	return _vecModel[_iModelNum - 1];
}

CChaModel* CMPPartCtrl::GetModel(int iIdx) {
	return _vecModel[iIdx];
}

CMPStrip* CMPPartCtrl::NewStrip(const s_string& strTex, CMPResManger* pResMagr) {
	SAFE_DELETE_ARRAY(_pcStrip);

	_iStripNum = 1;
	_pcStrip = new CMPStrip[_iStripNum];
	_pcStrip[0].CreateStrip(pResMagr->_dev, strTex, pResMagr);
	return &_pcStrip[0];
}

CMPStrip* CMPPartCtrl::GetStrip(int iIdx) {
	return &_pcStrip[iIdx];
}

void CMPPartCtrl::SetPlayType(int nType) {
	if (nType == 0) //
	{
		for (int n = 0; n < _iPartNum; ++n) {
			_vecPartSys[n]->SetPlayTime(0);
			_vecPartSys[n]->SetLoop(true);
		}
		for (int n = 0; n < _iModelNum; n++) {
			_vecModel[n]->SetPlayType(PLAY_LOOP);
			_vecModel[n]->Play();
		}
	}
}


void CMPPartCtrl::Play(int iTime) {
	for (int n = 0; n < _iPartNum; ++n) {
		_vecPartSys[n]->Play(iTime);
	}
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].Play();
	}
	for (int n = 0; n < _iModelNum; n++) {
		_vecModel[n]->Play();
	}
}

bool CMPPartCtrl::IsPlaying() {
	if (_iPartNum > 0) {
		for (int n = 0; n < _iPartNum; ++n) {
			if (_vecPartSys[n]->IsPlaying())
				return true;
		}
	}
	else if (_iStripNum > 0) {
		for (int n = 0; n < _iStripNum; ++n) {
			if (_pcStrip[n].IsPlaying())
				return true;
		}
	}
	else {
		for (int n = 0; n < _iModelNum; n++) {
			if (_vecModel[n]->IsPlaying())
				return true;
		}
	}
	return false;
}

void CMPPartCtrl::CopyPartCtrl(CMPPartCtrl* pPart) {
	_strName = pPart->_strName;

	_vecPartSys.resize(pPart->_iPartNum);

	for (int n = 0; n < pPart->_iPartNum; ++n) {
		AddPartSys(pPart->_vecPartSys[n]);
	}
	SAFE_DELETE_ARRAY(_pcStrip);
	_iStripNum = pPart->_iStripNum;
	if (_iStripNum) {
		_pcStrip = new CMPStrip[_iStripNum];
		for (int n = 0; n < _iStripNum; ++n) {
			_pcStrip[n].CopyStrip(&pPart->_pcStrip[n]);
		}
	}
	for (int n = 0; n < _iModelNum; n++) {
		SAFE_DELETE(_vecModel[n]);
	}
	_iModelNum = pPart->_iModelNum;
	CChaModel* pModel;
	_vecModel.resize(_iModelNum);
	for (int n = 0; n < _iModelNum; n++) {
		pModel = pPart->_vecModel[n];
		_vecModel[n] = new CChaModel;
		const std::string psID = std::format("{}", pModel->GetID());
		if (!_vecModel[n]->LoadScript(psID)) {
			ToLogService("errors", LogLevel::Error, "LoadScript {}", psID);
		}
		_vecModel[n]->SetVel(pModel->GetVel());
		_vecModel[n]->SetPlayType(pModel->GetPlayType());
		_vecModel[n]->SetCurPose(pModel->GetCurPose());
		_vecModel[n]->SetCurColor(pModel->GetCurColor());
		_vecModel[n]->SetSrcBlend(pModel->GetSrcBlend());
		_vecModel[n]->SetDestBlend(pModel->GetDestBlend());
		_vecModel[n]->PlayPose(0, PLAY_PAUSE);
	}
}

void CMPPartCtrl::BindingRes(CMPResManger* pResMagr) {
	m_pfDailTime = pResMagr->GetDailTime();
	for (int n = 0; n < _iPartNum; ++n) {
		_vecPartSys[n]->BindingRes(pResMagr);
	}
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].BindingRes(pResMagr);
	}
	for (int n = 0; n < _iModelNum; n++) {
		_vecModel[n]->_dev = pResMagr->_dev;
	}
}

CMPPartSys* CMPPartCtrl::AddPartSys(CMPPartSys* part) {
	_iPartNum++;
	if (_iPartNum > MAX_PART_SYS) {
		_iPartNum = MAX_PART_SYS;
		return NULL;
	}
	_vecPartSys.setsize(_iPartNum);
	CopyPartSys(_vecPartSys[_iPartNum - 1], part);

	return _vecPartSys[_iPartNum - 1];
}

CMPPartSys* CMPPartCtrl::NewPartSys() {
	_iPartNum++;
	if (_iPartNum > MAX_PART_SYS) {
		_iPartNum = MAX_PART_SYS;
		return NULL;
	}
	_vecPartSys.setsize(_iPartNum);
	return _vecPartSys[_iPartNum - 1];
}

void CMPPartCtrl::DeletePartSys(int iID) {
	if (_iPartNum == 0)
		return;
	_iPartNum--;
	_vecPartSys.remove(iID);
}

void CMPPartCtrl::Clear() {
	_iPartNum = 0;
	_vecPartSys.clear();
}

void CMPPartCtrl::UpdateStrip() {
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].UpdateFrame();
	}
}

void CMPPartCtrl::FrameMove(DWORD dwDailTime) {
	for (int n = 0; n < _iPartNum; ++n) {
		_vecPartSys[n]->FrameMove(dwDailTime);
	}
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].FrameMove();
	}
	for (int n = 0; n < _iModelNum; n++) {
		if (_vecModel[n]->IsPlaying()) {
			_vecModel[n]->UpdateMatrix();
			_vecModel[n]->FrameMove();
		}
	}
}

void CMPPartCtrl::Render() {
	for (int n = 0; n < _iPartNum; ++n) {
		_vecPartSys[n]->Render();
	}
	for (int n = 0; n < _iStripNum; ++n) {
		_pcStrip[n].Render();
	}
	for (int n = 0; n < _iModelNum; n++) {
		if (_vecModel[n]->IsPlaying()) {
			_vecModel[n]->Begin();
			_vecModel[n]->Render();
			_vecModel[n]->End();
		}
	}
}


void CMPPartCtrl::GetRes(CMPResManger* pResMagr, std::vector<INT>& vecTex, std::vector<INT>& vecModel,
						 std::vector<INT>& vecEff) {
	std::vector<INT>::iterator it;
	int id = -1;

	int n;
	int idtex, idmodel, ideff;
	s_string strName;
	if (_iStripNum > 0) {
		for (n = 0; n < _iStripNum; ++n) {
			strName = _pcStrip[n].GetTexName();
			id = pResMagr->GetTextureID(strName);
			if (id >= 0) {
				it = std::find(vecTex.begin(), vecTex.end(), id);
				if (it == vecTex.end()) {
					vecTex.push_back(id);
					const std::string pszPath = std::format("texture/effect/{}", strName);
					const std::string pszNewPath = std::format("effect/new/texture/{}", strName);
					if (!::CopyFile(pszPath.c_str(), pszNewPath.c_str(), FALSE)) {
					}
				}
			}
		}
	}
	else if (_iPartNum > 0) {
		for (n = 0; n < _iPartNum; ++n) {
			_vecPartSys[n]->GetRes(idtex, idmodel, ideff);
			if (idtex >= 0) {
				it = std::find(vecTex.begin(), vecTex.end(), idtex);
				if (it == vecTex.end()) {
					vecTex.push_back(idtex);
				}
			}
			if (idmodel >= 0) {
				it = std::find(vecModel.begin(), vecModel.end(), idmodel);
				if (it == vecModel.end()) {
					vecModel.push_back(idmodel);
				}
			}
			if (ideff >= 0) {
				it = std::find(vecEff.begin(), vecEff.end(), ideff);
				if (it == vecEff.end()) {
					vecEff.push_back(ideff);
				}
			}
		}
	}
}

void CMPPartCtrl::GetHitRes(CMPResManger* pResMagr, std::vector<s_string>& vecPar) {
	if (_iStripNum > 0) {
		return;
	}
	else if (_iPartNum > 0) {
		s_string strname;
		int id, n;
		for (n = 0; n < _iPartNum; ++n) {
			strname = _vecPartSys[n]->GetHitEff();

			id = pResMagr->GetPartCtrlID(strname);
			if (id >= 0) {
				vecPar.push_back(strname);
			}
		}
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

static void keyframe_proc(DWORD type, DWORD pose_id, DWORD key_id, DWORD key_frame, void* param) {
	CChaModel* pCha = ((CChaModel*)param);
	switch (type) {
	case KEYFRAME_TYPE_BEGIN: {
		pCha->SetPlaying(true);
		break;
	}
	case KEYFRAME_TYPE_KEY: {
		break;
	}
	case KEYFRAME_TYPE_END:
		if (pCha->GetPlayType() != PLAY_LOOP) {
			pCha->SetPlaying(false);
		}
		break;
	}
}

bool CChaModel::LoadScript(const s_string& strModel) {
	int charType = std::stoi(strModel);
	_iID = charType;

	const Corsairs::Common::Character::CCharacterModelInfo* modelInfo = Corsairs::Common::Character::CharacterModelStore::Instance()->Get(charType);
	if (!modelInfo) {
		return false;
	}
	if (modelInfo->_bone.empty()) {
		return false;
	}

	MPChaLoadInfo sModel;

	auto copyFixed = [](char* dst, size_t cap, std::string_view src) {
		size_t n = (std::min)(src.size(), cap - 1);
		std::memcpy(dst, src.data(), n);
		dst[n] = '\0';
	};

	copyFixed(sModel.bone, sizeof(sModel.bone), modelInfo->_bone);
	for (int i = 0; i < 5; i++) {
		copyFixed(sModel.part[i], sizeof(sModel.part[i]), modelInfo->_skins[i]);
	}

	if (!LoadChaModel(sModel)) {
		return false;
	}

	const SChaAction* pAction = CCharacterActionCache::_cache.GetCharAction(charType);
	if (!pAction) {
		return false;
	}
	if (!LoadPose(*pAction)) {
		return false;
	}
	SetPoseKeyFrameProc(keyframe_proc, (void*)this);

	return true;
}


bool CChaModel::LoadChaModel(MPChaLoadInfo& info) {
	if (info.bone[0] == '\0')
		return false;
	if (HRESULT hr = MPCharacter::Load(&info); FAILED(hr)) {
		ToLogService("errors", LogLevel::Error,
					 "[{}] MPCharacter::Load failed: bone='{}', hr=0x{:08X}",
					 __FUNCTION__, info.bone, static_cast<std::uint32_t>(hr));
		return false;
	}
	return true;
}

bool CChaModel::LoadPose(const SChaAction& SCharAct) {
	lwPoseInfo pi;
	memset(&pi, 0, sizeof(pi));
	for (const auto& action : SCharAct._actions) {
		if (!action.IsValid()) {
			continue;
		}
		pi.start = action._startFrame;
		pi.end = action._endFrame;
		const size_t kfCount = (std::min)(action._keyFrames.size(), static_cast<size_t>(MAX_KEY_FRAME_NUM));
		pi.key_frame_num = static_cast<DWORD>(kfCount);
		for (size_t k = 0; k < kfCount; k++) {
			pi.key_frame_seq[k] = action._keyFrames.at(k);
		}

		if (GetPoseCtrl()) {
			GetPoseCtrl()->InsertPose(action._actionNo, &pi, 1);
		}
	}
	return true;
}

void CChaModel::PlayPose(DWORD id, DWORD type) {
	MPCharacter::PlayPose(id, type, 0.0f, m_fVel);
}

void CChaModel::Begin() {
	_dev->GetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
	_dev->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	_dev->GetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	_dev->GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	_dev->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	_dev->GetDevice()->SetRenderState(D3DRS_SRCBLEND, _eSrcBlend);
	_dev->GetDevice()->SetRenderState(D3DRS_DESTBLEND, _eDestBlend);
	D3DMATERIALX mtl;
	mtl.Ambient = (D3DCOLORVALUE)m_dwCurColor;
	mtl.Diffuse = (D3DCOLORVALUE)m_dwCurColor;
	SetMaterial(&mtl);
	SetOpacity(m_dwCurColor.a);
}

void CChaModel::End() {
	_dev->GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);

	_dev->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	_dev->GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	_dev->GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}



bool CMPLink::Create(MPCharacter* pChaMain, int iDummy1, MPCharacter* pChaTag, int iDummy2,
					 std::string_view pszTex, int iTexNum, CMPResManger* pResMgr, D3DXVECTOR3* pEyePos,
					 MPRender* pDev) {
	_dev = pDev;
	_pCEffFile = pResMgr->GetEffectFile();
	_fDailTime = pResMgr->GetDailTime();
	_pEyePos = pEyePos;

	_vStart = D3DXVECTOR3(0, 0, 0);
	_vEnd = D3DXVECTOR3(10, 0, 0);
	_vdir = _vEnd - _vStart;
	_fdist = D3DXVec3Length(&_vdir);
	D3DXVec3Normalize(&_vdir, &_vdir);


	int n;

	_pFrame = new MPFrame[LINK_FACE];
	for (n = 0; n < LINK_FACE; n++) {
		_pFrame[n].vPos1._dwDiffuse = 0xffffffff;
		_pFrame[n].vPos2._dwDiffuse = 0xffffffff;
	}

	_pChaMain = pChaMain;
	_pChaTag = pChaTag;
	_iDummy1 = iDummy1;
	_iDummy2 = iDummy2;

	_pTex = new lwITex*[iTexNum];
	int id;
	for (n = 0; n < iTexNum; n++) {
		_pTex[n] = NULL;
		const s_string strName = std::format("{}{}", pszTex, n);
		id = pResMgr->GetTextureID(strName);
		if (id < 0)
			return false;
		_pTex[n] = pResMgr->GetTextureByIDlw(id);
	}
	return true;
}

void CMPLink::ColArc(float fradius) {
	D3DXVECTOR3 veyedir, vcross;
	const auto v = *_pEyePos - _vStart;

	D3DXVec3Normalize(&veyedir, &v);
	D3DXVec3Cross(&vcross, &_vdir, &veyedir);
	D3DXVec3Normalize(&vcross, &vcross);


	D3DXVECTOR3 vOrg(-fradius, 0, 0);
	D3DXVECTOR3 vTemp;


	D3DXMATRIX mat;
	int n;
	int num = 10;
	float fstep = (D3DX_PI / 2.0f) / float(num);
	float fhei;
	float ft;
	for (n = 0; n < num; ++n) {
		D3DXMatrixRotationY(&mat, n * fstep);
		D3DXVec3TransformCoord(&vTemp, &vOrg, &mat);
		fhei = vTemp.z;

		ft = fradius + vTemp.x;
		if (ft < 0.00001f) {
			ft = 0.0f;
		}
		vTemp = _vStart + _vdir * ft;
		vTemp.z = fhei;

		ft = ft / _fdist;


		_pFrame[n].vPos1._SPos = vTemp + (-vcross * 1.0f);
		_pFrame[n].vPos2._SPos = vTemp + (vcross * 1.0f);

		if (n == 0) {
			_pFrame[n].vPos1._SUV.x = 0;
			_pFrame[n].vPos1._SUV.y = 1;

			_pFrame[n].vPos2._SUV.x = 0;
			_pFrame[n].vPos2._SUV.y = 0;
		}
		else {
			_pFrame[n].vPos1._SUV.x = ft;
			_pFrame[n].vPos1._SUV.y = 1;

			_pFrame[n].vPos2._SUV.x = ft;
			_pFrame[n].vPos2._SUV.y = 0;
		}
	}
	_pFrame[num].vPos1._SPos = _vEnd + (-vcross * 1.0f);
	_pFrame[num].vPos2._SPos = _vEnd + (vcross * 1.0f);

	_pFrame[num].vPos1._SUV.x = 1;
	_pFrame[num].vPos1._SUV.y = 1;

	_pFrame[num].vPos2._SUV.x = 1;
	_pFrame[num].vPos2._SUV.y = 0;
}

void CMPLink::GetPhysique() {
	_vStart = D3DXVECTOR3(0, 0, 0);
	_vEnd = D3DXVECTOR3(10, 0, 0);

	_vdir = _vEnd - _vStart;
	_fdist = D3DXVec3Length(&_vdir);
	D3DXVec3Normalize(&_vdir, &_vdir);


	float frad = 3.0f;
	if (_fdist > frad * 2) {
		_fRadius = frad - (frad * ((_fdist / frad - 1.0f) / 10.0f));
	}
	else {
		_fRadius = frad + ((frad - _fdist) / 10.0f);
	}
	ColArc(_fRadius);
}


void CMPLink::FrameMove() {
	GetPhysique();

	_fCurTime += *_fDailTime;
	if (_fCurTime > 0.15f) {
		_iCurTex++;
		if (_iCurTex >= 4)
			_iCurTex = 0;
		_fCurTime = 0;
	}
}

void CMPLink::Render() {
	_pCEffFile->SetTechnique(0);
	_pCEffFile->Begin();
	_pCEffFile->Pass(0);


	_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

	_dev->SetTransformWorld(&mat);
	_dev->SetTexture(0, _pTex[_iCurTex]->GetTex());
	_dev->SetVertexShader(NULL);
	_dev->SetFVF(LINK_FVF);
	_dev->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,
										 20, _pFrame, sizeof(LinkVer));

	_pCEffFile->End();
}

void CMPPartCtrl::SetSkillCtrl(SkillCtrl* pCtrl) {
	for (int n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->SetSkillCtrl(pCtrl);
		}
	}
}

void CMPPartCtrl::SetAlpha(float falpha) {
	for (int n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->SetAlpha(falpha);
		}
	}
}

void CMPPartCtrl::Reset() {
	_fCurTime = 0;
	const auto v = D3DXVECTOR3(0, 0, 0);
	MoveTo(&v);
	for (int n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->Reset(false);
			_vecPartSys[n]->unFontEffCom();
		}
	}
}

void CMPPartCtrl::Stop() {
	Reset();
	for (int n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->Stop();
		}
	}
}

void CMPPartCtrl::End() {
	for (int n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->End();
		}
	}
}

void CMPPartCtrl::MoveTo(const D3DXVECTOR3* vPos, MPMap* pmap) {
	for (auto n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->MoveTo(vPos, pmap);
		}
	}
	for (auto n = 0; n < _iModelNum; n++) {
		if (_vecModel[n]->IsPlaying()) {
			_vecModel[n]->MoveTo(vPos);
		}
	}
}

void CMPPartCtrl::BindingBone(D3DXMATRIX* pMatBone) {
	for (auto n = 0; n < _iPartNum; ++n) {
		if (_vecPartSys[n]) {
			_vecPartSys[n]->BindingBone(pMatBone);
		}
	}
	for (auto n = 0; n < _iModelNum; n++) {
		if (_vecModel[n]->IsPlaying()) {
			_vecModel[n]->BindingBone(pMatBone);
		}
	}
}
