#include "StdAfx.h"

#include "GlobalInc.h"

#include "mpmodeleff.h"
#include "MPRender.h"
#include "EfxTrack.h"
#include "AssetLoaders.h"

void CEffectCortrol::FillModelUV(CEffectModel* pCModel) {
	for (WORD i = 0; i < pCModel->GetVerCount(); i++) {
		pCModel->GetDev()->SetVertexShaderConstantF(9 + i, *_vecCurCoord[i], 1);
	}
}

void CEffectCortrol::FillTextureUV(CEffectModel* pCModel) {
	for (WORD i = 0; i < pCModel->GetVerCount(); i++) {
		pCModel->GetDev()->SetVertexShaderConstantF(9 + i, *_lpCurTex[i], 1);
	}
}

void CEffectCortrol::FillDefaultUV(CEffectModel* pCModel, TEXCOORD& coord) {
	for (WORD i = 0; i < pCModel->GetVerCount(); i++) {
		pCModel->GetDev()->SetVertexShaderConstantF(9 + i, coord[i], 1);
	}
}

CMPModelEff::CMPModelEff(void) {
	_iEffNum = 0;
	ReleaseAll();
}

CMPModelEff::~CMPModelEff(void) {
	ReleaseAll();
}

/************************************************************************/
/*!	*/
/************************************************************************/
void CMPModelEff::ReleaseAll() {
	int n;
	for (n = 0; n < _iEffNum; n++) {
		m_vecEffect[n]->DeleteItem(m_pResMgr);
		delete m_vecEffect[n];
		m_vecEffect[n] = NULL;
	}
	_bLoop = true;
	_bPlay = false;

	_pCEffect = NULL;
	_iEffNum = 0;
	m_vecEffect.clear();
	_vecCortrol.clear();
	_pCurCortrol = NULL;

	m_iTimes = 0;
	m_pfDailTime = NULL;

	D3DXMatrixIdentity(&_SpmatBone);
	D3DXMatrixIdentity(&_matTempRota);

	D3DXMatrixScaling(&_matScale, 1.0f, 1.0f, 1.0f);
	D3DXMatrixRotationYawPitchRoll(&_matRota, 0, 0, 0);
	D3DXMatrixTranslation(&_matTrans, 0, 0, 0);

	_verScale = D3DXVECTOR3(1, 1, 1);
	_verRota = D3DXVECTOR3(0, 0, 0);
	_verTrans = D3DXVECTOR3(0, 0, 0);

	_idxTech = 0;
	_pCEffectFile = NULL;
	m_pMatViewProj = NULL;

	_bBindbone = false;

	_usePath = false;
	_pathName = "";
	m_pPath = NULL;
	_useSound = false;
	_soundName = "";

	_rotating = false;
	_rotaVel = 0.01f;
	_fCurRotat = 0;

	_bUseZ = true;
	_fCurRotat = 0;
	m_pResMgr = NULL;
}

void CMPModelEff::ClearEffect() {
	int n;
	for (n = 0; n < _iEffNum; n++) {
		m_vecEffect[n]->DeleteItem(m_pResMgr);
		delete m_vecEffect[n];
		m_vecEffect[n] = NULL;
	}
	m_vecEffect.clear();
	_iEffNum = 0;
}

/************************************************************************/
/*!*/
/************************************************************************/
void CMPModelEff::Reset() {
	for (int n = 0; n < _iEffNum; n++) {
		_vecCortrol[n]->Reset();
	}
	D3DXMatrixIdentity(&_SpmatBone);
	D3DXMatrixIdentity(&_matTempRota);

	D3DXMatrixScaling(&_matScale, 1.0f, 1.0f, 1.0f);
	D3DXMatrixRotationYawPitchRoll(&_matRota, 0, 0, 0);
	D3DXMatrixTranslation(&_matTrans, 0, 0, 0);

	_verPartRota = D3DXVECTOR3(0, 0, 0);
	_bBindbone = false;
	_fCurRotat = 0;
}

///************************************************************************/
///*/*/
///************************************************************************/

/************************************************************************/
/*/*/
/************************************************************************/

void CMPModelEff::FrameMoveAccel(float fDail) {
}

void CMPModelEff::Begin() {
	_pCEffectFile->SetTechnique(_idxTech);
	_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
	_pCEffectFile->Pass(0);
	m_vecEffect[0]->_dev->SetVertexShaderConstantF(4, *m_pMatViewProj, 4);
}

void CMPModelEff::End() {
	//this been added when we fixed circle shadow not sure 100% of it but so far it works
	//@moth
	_pCEffect->_dev->SetVertexShader(nullptr);
	_pCEffect->_dev->SetFVF(EFFECT_VER_FVF);
	//edit end 
	_pCEffectFile->End();
	_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, TRUE);

	_pCEffect->_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	_pCEffect->_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CMPModelEff::RenderAccel(float& fTime) {
	_pCEffectFile->SetTechnique(_idxTech);
	_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
	_pCEffectFile->Pass(0);

	D3DXMATRIX t_STemp;

	GetTransMatrix(t_STemp);

	m_vecEffect[0]->_dev->SetVertexShaderConstantF(4, *m_pMatViewProj, 4);

	for (int n = 0; n < _iEffNum; n++) {
		_pCurCortrol = _vecCortrol[n];
		if (!_pCurCortrol->IsPlay())
			continue;

		_pCEffect = m_vecEffect[n];

		_pCurCortrol->GetTransformMatrix(&_SMatResult);

		D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &t_STemp);
		if (_pCEffect->IsItem()) {
			_pCEffectFile->End();

			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);

			if (_bBindbone) {
				D3DXMatrixMultiply(&_pCurCortrol->_SMatResult, &_SMatResult, &_SpmatBone);
			}
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			_pCEffect->_pCModel->SetMatrix((lwMatrix44*)&_pCurCortrol->_SMatResult);
			_pCEffect->_pCModel->FrameMove(0);
			_pCEffect->SetTexture();
			_pCEffect->Render();
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		if (_pCEffect->IsChangeably()) {
			_pCEffectFile->End();
			_pCEffect->Begin();

			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);

			_pCEffect->SetTexture();

			if (_bBindbone) {
				D3DXMatrixMultiply(&_pCurCortrol->_SMatResult, &_SMatResult, &_SpmatBone);
			}
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);


			_pCEffect->_dev->SetTransformWorld(&_pCurCortrol->_SMatResult);

			_pCEffect->Render();
			_pCEffect->End();
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		_pCEffect->SetTexture();
		_pCEffect->Begin();
		_pCEffect->SetVertexShader();

		if (_pCEffect->IsBillBoard()) {
			if (!_bUseZ)
				_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
			D3DXVECTOR3 vtpos(&_SMatResult._41);
			D3DXMatrixMultiply(&_SMatResult, &_SMatResult, _pCEffect->getBillBoardMatrix());

			if (_bBindbone) {
				_SMatResult._41 = _SpmatBone._41;
				_SMatResult._42 = _SpmatBone._42;
				_SMatResult._43 = _SpmatBone._43;
			}
			else {
				_SMatResult._41 = vtpos.x;
				_SMatResult._42 = vtpos.y;
				_SMatResult._43 = vtpos.z;
			}
		}
		else {
			if (_bBindbone)
				D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &_SpmatBone);
		}
		Transpose(_pCurCortrol->_SMatResult, _SMatResult);

		if (_pCEffect->getType() != EFFECT_MODEL) {
			if (_pCEffect->getType() == EFFECT_MODELUV) {
				_pCurCortrol->FillModelUV(_pCEffect->_pCModel);
			}
			else {
				_pCurCortrol->FillTextureUV(_pCEffect->_pCModel);
			}
		}
		_pCEffect->_dev->SetVertexShaderConstantF(0, _pCurCortrol->_SMatResult, 4);
		_pCEffect->_dev->SetVertexShaderConstantF(8, _pCurCortrol->m_dwCurColor, 1);


		_pCEffect->Render();
		_pCEffect->End();
		_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, TRUE);
	}
	_pCEffectFile->End();
	_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, TRUE);

	_pCEffect->_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	_pCEffect->_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CMPModelEff::FrameMove(DWORD dwDailTime) {
	if (!_bPlay)
		return;
	for (int n = 0; n < _iEffNum; n++) {
		_pCurCortrol = _vecCortrol[n];
		if (!_pCurCortrol->IsPlay())
			continue;
		_pCEffect = m_vecEffect[n];

		int t_iNextFrame;

		_pCurCortrol->_fCurTime += *m_pfDailTime;
		//!
		if (_pCurCortrol->_fCurTime >= _pCEffect->getFrameTime(_pCurCortrol->_wCurFrame)) {
			_pCurCortrol->_wCurFrame++;
			if (_pCurCortrol->_wCurFrame >= _pCEffect->getFrameCount()) {
				_pCurCortrol->Stop(); //
				if (_bLoop) //
				{
					if (!IsPlay()) //!
					{
						Play2(0);
					}
					continue;
				}
				else //!
				{
					if (!IsPlay()) //!
					{
						Stop();
						return;
					}
				}
			}

			_pCurCortrol->_fCurTime = 0.0f;
		}
		//!
		if (_pCurCortrol->_wCurFrame == (/*_wFrameCount*/_pCEffect->getFrameCount() - 1))
			t_iNextFrame = 0;
		else
			t_iNextFrame = _pCurCortrol->_wCurFrame + 1;

		//!
		m_fLerp = _pCurCortrol->_fCurTime / _pCEffect->getFrameTime(_pCurCortrol->_wCurFrame);

		_pCEffect->_ilast = _pCurCortrol->_wCurFrame;
		_pCEffect->_inext = t_iNextFrame;
		_pCEffect->_flerp = m_fLerp;

		//!
		_pCEffect->GetLerpSize(&_pCurCortrol->m_SCurSize, _pCurCortrol->_wCurFrame, t_iNextFrame, m_fLerp);

		if (!_pCEffect->IsBillBoard()) {
			_pCEffect->GetLerpAngle(&_pCurCortrol->m_SCurAngle, _pCurCortrol->_wCurFrame, t_iNextFrame, m_fLerp);
		}
		//!
		_pCEffect->GetLerpPos(&_pCurCortrol->m_SCurPos, _pCurCortrol->_wCurFrame, t_iNextFrame, m_fLerp);
		//!
		_pCEffect->GetLerpColor(&_pCurCortrol->m_dwCurColor, _pCurCortrol->_wCurFrame, t_iNextFrame, m_fLerp);

		if (!_pCEffect->IsItem()) {
			if (_pCEffect->getType() == EFFECT_MODELUV) {
				//!
				_pCEffect->GetLerpCoord(_pCurCortrol->_vecCurCoord, _pCurCortrol->m_wCurCoordIndex,
										 _pCurCortrol->m_fCurCoordTime, *m_pfDailTime);
			}
			else {
				if (_pCEffect->getType() == EFFECT_MODELTEXTURE) {
					//!
					_pCEffect->GetLerpTexture(_pCurCortrol->_lpCurTex, _pCurCortrol->m_wCurTexIndex,
											   _pCurCortrol->m_fCurTexTime, *m_pfDailTime);
				}
				else if (_pCEffect->getType() == EFFECT_FRAMETEX) {
					//!
					_pCEffect->GetLerpFrame(_pCurCortrol->m_wCurTexIndex, _pCurCortrol->m_fCurTexTime,
											 *m_pfDailTime);
				}
			}
		}
		else if (_pCEffect->getType() == EFFECT_FRAMETEX) {
			//!
			_pCEffect->GetLerpFrame(_pCurCortrol->m_wCurTexIndex, _pCurCortrol->m_fCurTexTime, *m_pfDailTime);
		}
	}
}

/************************************************************************/
/*/*/
/************************************************************************/
void CMPModelEff::Render() {
	if (!_bPlay)
		return;

	if (!_bUseSoft) {
		RenderVS();
	}
	else
		RenderSoft();
}

void CMPModelEff::RenderVS() {
	D3DXMATRIX t_STemp;

	GetTransMatrix(t_STemp);


	for (int n = 0; n < _iEffNum; n++) {
		_pCurCortrol = _vecCortrol[n];
		if (!_pCurCortrol->IsPlay())
			continue;

		_pCEffect = m_vecEffect[n];

		if (_pCEffect->getType() == EFFECT_FRAMETEX) {
			if (!_pCEffect->_CTexFrame._lpCurTex || !_pCEffect->_CTexFrame._lpCurTex->IsLoadingOK())
				continue;
		}
		else {
			if (!_pCEffect->_CTextruelist._pTex || !_pCEffect->_CTextruelist._pTex->IsLoadingOK())
				continue;
		}
		if (_pCEffect->IsRotaLoop()) {
			D3DXMATRIX sMat;
			_pCEffect->GetRotaLoopMatrix(&sMat, _pCurCortrol->_fCurRotat, *m_pfDailTime);
			_pCurCortrol->GetTransformMatrix(&_SMatResult, &sMat);
		}
		else
			_pCurCortrol->GetTransformMatrix(&_SMatResult);

		D3DXMATRIX vertexShaderMat;
		D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &t_STemp);
		if (_pCEffect->IsItem()) {
			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);


			if (!_pCEffect->IsAlpah()) {
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, FALSE);
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, TRUE);
			}
			else {
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, TRUE);
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, FALSE);
				_pCEffect->_dev->SetRenderStateForced(D3DRS_CULLMODE, D3DCULL_CCW);
				//move this here
				_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			}

			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			if (_pCEffect->_pCModel) {
				if (_bBindbone) {
					D3DXMatrixMultiply(&_pCurCortrol->_SMatResult, &_SMatResult, &_SpmatBone);

					_pCEffect->_pCModel->SetMatrix((lwMatrix44*)&_pCurCortrol->_SMatResult);
				}
				else {
					_pCEffect->_pCModel->SetMatrix((lwMatrix44*)&_SMatResult);
				}

				_pCEffect->_pCModel->FrameMove(0);
				_pCEffect->SetTexture();
				D3DXMatrixTranspose(&vertexShaderMat,
									_pCEffect->_pCModel->GetPrimitive()->GetRenderCtrlAgent()->GetGlobalMatrix());
			}
			else {
				D3DXMatrixIdentity(&vertexShaderMat);
			}
			_pCEffect->SetVertexShader();
			_pCEffect->_dev->SetVertexShaderConstantF(0, vertexShaderMat, 4);
			_pCEffect->_dev->SetVertexShaderConstantF(4, *m_pMatViewProj, 4);
			_pCEffect->_dev->SetVertexShaderConstantF(8, _pCurCortrol->m_dwCurColor, 1);

			// begin by lsh
			// end			

			_pCEffect->Render();

			_pCEffectFile->End();
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		if (_pCEffect->IsChangeably()) {
			_pCEffect->Begin();

			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			_pCEffect->_dev->SetSamplerStateForced(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			if (!_pCEffect->IsAlpah()) {
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, FALSE);
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, TRUE);
			}
			else {
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, FALSE);
				_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, TRUE);
			}
			_pCEffect->SetVertexShader();
			_pCEffect->SetTexture();

			if (_bBindbone) {
				D3DXMatrixMultiply(&_pCurCortrol->_SMatResult, &_SMatResult, &_SpmatBone);
				D3DXMatrixTranspose(&vertexShaderMat, &_pCurCortrol->_SMatResult);
				_pCEffect->_dev->SetTransformWorld(&_pCurCortrol->_SMatResult);
			}
			else {
				D3DXMatrixTranspose(&vertexShaderMat, &_SMatResult);
				_pCEffect->_dev->SetTransformWorld(&_SMatResult);
			}
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetVertexShaderConstantF(0, vertexShaderMat, 4);

			_pCEffect->Render();
			_pCEffect->End();
			_pCEffectFile->End();
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		_pCEffect->Begin();
		_pCEffectFile->SetTechnique(_idxTech);
		_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
		_pCEffectFile->Pass(0);
		if (!_pCEffect->IsAlpah()) {
			_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, FALSE);
			_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, TRUE);
		}
		else {
			_pCEffect->_dev->SetRenderStateForced(D3DRS_ZWRITEENABLE, FALSE);
			_pCEffect->_dev->SetRenderStateForced(D3DRS_ALPHABLENDENABLE, TRUE);
		}
		_pCEffect->SetVertexShader();
		_pCEffect->SetTexture();

		if (_pCEffect->IsBillBoard()) {
			if (!_bUseZ)
				_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
			D3DXVECTOR3 vtpos(&_SMatResult._41);
			if (!_pCEffect->IsRotaBoard())
				D3DXMatrixIdentity(&_SMatResult);
			D3DXMatrixMultiply(&_SMatResult, &_SMatResult, _pCEffect->getBillBoardMatrix());

			if (_bBindbone) {
				_SMatResult._41 = _SpmatBone._41;
				_SMatResult._42 = _SpmatBone._42;
				_SMatResult._43 = _SpmatBone._43;
			}
			else {
				_SMatResult._41 = vtpos.x;
				_SMatResult._42 = vtpos.y;
				_SMatResult._43 = vtpos.z;
			}
		}
		else {
			if (_bBindbone)
				D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &_SpmatBone);
		}

		D3DXMatrixTranspose(&_pCurCortrol->_SMatResult, &_SMatResult);

		if (_pCEffect->getType() != EFFECT_MODEL) {
			if (_pCEffect->getType() == EFFECT_MODELUV) {
				_pCurCortrol->FillModelUV(_pCEffect->_pCModel);
			}
			else if (_pCEffect->getType() != EFFECT_FRAMETEX) {
				_pCurCortrol->FillTextureUV(_pCEffect->_pCModel);
			}
			else
				_pCurCortrol->FillDefaultUV(_pCEffect->_pCModel, _pCEffect->_CTexFrame._vecCoord);
		}

		_pCEffect->_dev->SetVertexShaderConstantF(0, _pCurCortrol->_SMatResult, 4);
		_pCEffect->_dev->SetVertexShaderConstantF(8, _pCurCortrol->m_dwCurColor, 1);
		_pCEffect->_dev->SetVertexShaderConstantF(4, *m_pMatViewProj, 4);

		_pCEffect->Render();
		_pCEffect->End();
		_pCEffectFile->End();
	}
}

void CMPModelEff::RenderSoft() {
	if (!_bPlay)
		return;

	_pCEffectFile->SetTechnique(_idxTech);
	_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
	_pCEffectFile->Pass(0);
	if (!_bUseZ) {
		_pCEffect->_dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	else {
		_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		_pCEffect->_dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}

	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	_pCEffectFile->_dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	D3DXMATRIX t_STemp;

	if (_rotating) {
		_fCurRotat += _rotaVel * *m_pfDailTime;
		if (_fCurRotat >= 6.283185f)
			_fCurRotat = _fCurRotat - 6.283185f;
		D3DXMatrixRotationAxis(&_matRota,
							   &_verRota, _fCurRotat);
	}
	D3DXMatrixMultiply(&t_STemp, &_matScale, &_matRota);
	D3DXMatrixMultiply(&t_STemp, &t_STemp, &_matTrans);


	for (int n = 0; n < _iEffNum; n++) {
		_pCurCortrol = _vecCortrol[n];
		if (!_pCurCortrol->IsPlay())
			continue;

		_pCEffect = m_vecEffect[n];

		if (!_pCEffect->_CTextruelist._pTex->IsLoadingOK())
			return;

		_pCurCortrol->GetTransformMatrix(&_SMatResult);
		D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &t_STemp);

		if (_pCEffect->IsItem()) {
			_pCEffectFile->End();

			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);
			if (_bBindbone) {
				D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &_SpmatBone);
			}
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			_pCEffect->_pCModel->SetMatrix((lwMatrix44*)&_SMatResult);
			// begin by lsh
			// end

			_pCEffect->_pCModel->FrameMove(0);

			_pCEffect->Render();
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		if (_pCEffect->IsChangeably()) {
			_pCEffectFile->End();
			_pCEffect->Begin();

			_pCEffectFile->SetTechnique(3);
			_pCEffectFile->Begin(D3DXFX_DONOTSAVESTATE);
			_pCEffectFile->Pass(0);

			_pCEffect->SetTexture();

			if (_bBindbone) {
				D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &_SpmatBone);
			}
			_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

			_pCEffect->_dev->SetTransformWorld(&_SMatResult);

			_pCEffect->Render();
			_pCEffect->End();
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			continue;
		}
		_pCEffect->SetTexture();
		_pCEffect->Begin();
		_pCEffect->_dev->SetVertexShader(NULL);
		_pCEffect->_dev->SetFVF(EFFECT_VER_FVF);


		if (_pCEffect->IsBillBoard()) {
			if (!_bUseZ)
				_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, FALSE);

			D3DXVECTOR3 vtpos(&_SMatResult._41);
			D3DXMatrixMultiply(&_SMatResult, &_SMatResult, _pCEffect->getBillBoardMatrix());

			if (_bBindbone) {
				_SMatResult._41 = _SpmatBone._41;
				_SMatResult._42 = _SpmatBone._42;
				_SMatResult._43 = _SpmatBone._43;
			}
			else {
				_SMatResult._41 = vtpos.x;
				_SMatResult._42 = vtpos.y;
				_SMatResult._43 = vtpos.z;
			}
		}
		else
			D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &_SpmatBone);

		if (_pCEffect->getType() != EFFECT_MODEL) {
			if (_pCEffect->getType() == EFFECT_MODELUV) {
				_pCurCortrol->FillModelUVSoft(_pCEffect->_pCModel);
			}
			else {
				_pCurCortrol->FillTextureUVSoft(_pCEffect->_pCModel);
			}
		}
		_pCEffect->_dev->SetRenderState(D3DRS_TEXTUREFACTOR, _pCurCortrol->m_dwCurColor);
		_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
		_pCEffect->_dev->SetTransformWorld(&_SMatResult);

		_pCEffect->Render();
		_pCEffect->End();
		_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		_pCEffect->_dev->SetTextureStageStateForced(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	}
	_pCEffectFile->End();
	_pCEffect->_dev->SetRenderState(D3DRS_ZENABLE, TRUE);
	_pCEffect->_dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}


void CMPModelEff::ShowCurFrame(int iCurSubEff, int iCurFrame) {
	{
		_pCEffect = m_vecEffect[iCurSubEff];

		//!
		D3DXVECTOR3 t_sVerSize = _pCEffect->getFrameSize(iCurFrame);

		D3DXVECTOR3 t_sVerAngle = _pCEffect->getFrameAngle(iCurFrame);

		//!
		D3DXVECTOR3 t_sVerPos = _pCEffect->getFramePos(iCurFrame);

		D3DXMATRIX t_SMat, t_SMatRot;
		D3DXMatrixScaling(&t_SMat, t_sVerSize.x, t_sVerSize.y, t_sVerSize.z);
		D3DXMatrixRotationYawPitchRoll(&t_SMatRot,
									   t_sVerAngle.y, t_sVerAngle.x, t_sVerAngle.z);
		D3DXMatrixMultiply(&_SMatResult, &t_SMat, &t_SMatRot);

		_SMatResult._41 = t_sVerPos.x;
		_SMatResult._42 = t_sVerPos.y;
		_SMatResult._43 = t_sVerPos.z;

		D3DXMatrixMultiply(&t_SMat, &_matScale, &_matRota);
		D3DXMatrixMultiply(&t_SMat, &t_SMat, &_matTrans);
		D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &t_SMat);


		//!
		D3DXCOLOR t_sColor = _pCEffect->getFrameColor(iCurFrame);

		//!

		//!
		_pCEffect->SetTexture();
		_pCEffect->_dev->SetTransformWorld(&_SMatResult);

		_pCEffect->Begin();

		_pCEffect->_dev->SetVertexShader(NULL);
		_pCEffect->_dev->SetFVF(EFFECT_VER_FVF);

		_pCEffect->Render();
		_pCEffect->End();
	}
}

void CMPModelEff::ShowTempFrame(int iCurSubEff,
								D3DXVECTOR3& pScale, D3DXVECTOR3& pRotating, D3DXVECTOR3& pTranslate,
								D3DXCOLOR& pColor, TEXCOORD& vecCoord, IDirect3DTextureX* lpTex) {
	_pCEffect = m_vecEffect[iCurSubEff];

	D3DXMATRIX t_SMat, t_SMatRot;
	D3DXMatrixScaling(&t_SMat, pScale.x, pScale.y, pScale.z);
	D3DXMatrixRotationYawPitchRoll(&t_SMatRot,
								   pRotating.y, pRotating.x, pRotating.z);
	D3DXMatrixMultiply(&_SMatResult, &t_SMat, &t_SMatRot);

	_SMatResult._41 = pTranslate.x;
	_SMatResult._42 = pTranslate.y;
	_SMatResult._43 = pTranslate.z;

	D3DXMatrixMultiply(&t_SMat, &_matScale, &_matRota);
	D3DXMatrixMultiply(&t_SMat, &t_SMat, &_matTrans);
	D3DXMatrixMultiply(&_SMatResult, &_SMatResult, &t_SMat);


	//!
	_pCEffect->SetTexture();
	_pCEffect->_dev->SetTransformWorld(&_SMatResult);

	_pCEffect->_dev->SetVertexShader(NULL);
	_pCEffect->_dev->SetFVF(EFFECT_VER_FVF);

	_pCEffect->Begin();

	_pCEffect->Render();
	_pCEffect->End();
}


/************************************************************************/
/*                                                                      */
/************************************************************************/


CMPStrip::CMPStrip() {
	_pCha = NULL;
	_pItem = NULL;
	_iDummy[0] = -1;
	_iDummy[1] = -1;

	m_iMaxLen = 256;

	_vecPath.resize(m_iMaxLen);
	_vecCtrl.resize(m_iMaxLen / 2);
	_pTex = NULL;
	_pCEffFile = NULL;
	_strTexName = "";
	m_pfDailTime = NULL;
	_fCurTime = 0;
	_bPlay = false;

	_dwColor = D3DXCOLOR(1, 1, 1, 1);
	_fLife = 1.0f;
	_fStep = 0.05f;
	_eSrcBlend = D3DBLEND_SRCALPHA;
	_eDestBlend = D3DBLEND_INVSRCALPHA;

	_bLoop = false;
}

CMPStrip::~CMPStrip() {
}

void CMPStrip::Play() {
	_vecPath.clear();
	_vecCtrl.clear();

	lwMatrix44 mat1, mat2;
	if (_pItem) {
		_pItem->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
		_pItem->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
	}
	else if (_pCha) {
		_pCha->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
		_pCha->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
	}
	else
		return;

	GetTrack(&mat1, &mat2);
	_fCurTime = 0;
	_bPlay = true;
}

void CMPStrip::UpdateFrame() {
	if (!_bPlay)
		return;
	lwMatrix44 mat1, mat2;
	if (_pItem) {
		_pItem->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
		_pItem->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
	}
	else if (_pCha) {
		_pCha->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
		_pCha->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
	}
	GetTrack(&mat1, &mat2);
}

void CMPStrip::FrameMove() {
	if (!_bPlay)
		return;
	_fCurTime += *m_pfDailTime;
	if (_fCurTime > _fStep) {
		lwMatrix44 mat1, mat2;
		if (_pItem) {
			_pItem->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
			_pItem->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
		}
		else if (_pCha) {
			_pCha->GetObjDummyRunTimeMatrix(&mat1, _iDummy[0]);
			_pCha->GetObjDummyRunTimeMatrix(&mat2, _iDummy[1]);
		}
		GetTrack(&mat1, &mat2);
		_fCurTime = 0;
	}
}

void CMPStrip::Render() {
	if (!_bPlay)
		return;
	_pCEffFile->SetTechnique(3);
	_pCEffFile->Begin(D3DXFX_DONOTSAVESTATE);
	_pCEffFile->Pass(0);
	_dev->SetRenderState(D3DRS_SRCBLEND, _eSrcBlend);
	_dev->SetRenderState(D3DRS_DESTBLEND, _eDestBlend);
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

	_dev->SetVertexShader(NULL);
	_dev->SetFVF(STRIP_FVF);

	if (_pTex && _pTex->IsLoadingOK())
		_dev->SetTexture(0, _pTex->GetTex());
	else {
		_pCEffFile->End();
		return;
	}

	_dev->SetTransformWorld(&mat);

	track* ptrack;
	D3DXCOLOR color = _dwColor;
	if (_vecCtrl.size() > 1) {
		for (int n = 0; n < _vecCtrl.size(); ++n) {
			ptrack = _vecCtrl[n];

			ptrack->FrameMove(*m_pfDailTime, color, _fLife);
			_vecPath[n * 2]->_dwDiffuse = color;
			_vecPath[n * 2 + 1]->_dwDiffuse = color;
		}
	}
	if (_vecCtrl.size() > 1)
		_dev->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, _vecPath.size() - 2, _vecPath.front(),
											 sizeof(Strip_Vertex));
	_pCEffFile->End();
}


void CMPStrip::CopyStrip(CMPStrip* pstrip) {
	m_iMaxLen = pstrip->m_iMaxLen;
	_iDummy[0] = pstrip->_iDummy[0];
	_iDummy[1] = pstrip->_iDummy[1];
	_dwColor = pstrip->_dwColor;
	_fLife = pstrip->_fLife;
	_fStep = pstrip->_fStep;
	_strTexName = pstrip->_strTexName;
	_eSrcBlend = pstrip->_eSrcBlend;
	_eDestBlend = pstrip->_eDestBlend;
}

CEffectCortrol::CEffectCortrol() {
	_bPlay = false;

	_fCurTime = 0.0f;
	_wCurFrame = 0;
	m_dwCurColor = D3DCOLOR_ARGB(0, 255, 255, 255);

	m_SCurSize = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	m_SCurAngle = D3DXVECTOR3(0, 0, 0);
	m_SCurPos = D3DXVECTOR3(0, 0, 0);

	_vecCurCoord.clear();
	m_wCurCoordIndex = 0;
	m_fCurCoordTime = 0;
	m_wCurTexIndex = 0;
	m_fCurTexTime = 0;
	_lpCurTex.clear();

	m_iCurTimes = 0;

	_vecCurCoord.resize(10);
	_lpCurTex.resize(10);
}

void CEffectCortrol::Reset() {
	_bPlay = false;
	_fCurTime = 0.0f;
	_wCurFrame = 0;
	m_wCurCoordIndex = 0;
	m_fCurCoordTime = 0;
	m_wCurTexIndex = 0;
	m_fCurTexTime = 0;
}

void CEffectCortrol::Play() {
	m_iCurTimes = 0;
	_bPlay = true;
}

void CEffectCortrol::Stop() {
	Reset();
	_bPlay = false;
}

void CEffectCortrol::GetTransformMatrix(D3DXMATRIX* pSOut, D3DXMATRIX* pRota) {
	D3DXMATRIX t_SMat, t_SMatRot;
	D3DXMatrixScaling(&t_SMat, m_SCurSize.x, m_SCurSize.y, m_SCurSize.z);
	if (!pRota) {
		D3DXMatrixRotationYawPitchRoll(&t_SMatRot,
									   m_SCurAngle.y, m_SCurAngle.x, m_SCurAngle.z);
		D3DXMatrixMultiply(pSOut, &t_SMat, &t_SMatRot);
	}
	else {
		D3DXMATRIX t_mat;
		D3DXMatrixRotationYawPitchRoll(&t_SMatRot,
									   m_SCurAngle.y, m_SCurAngle.x, m_SCurAngle.z);

		D3DXMatrixMultiply(&t_mat, &t_SMatRot, pRota);
		D3DXMatrixMultiply(pSOut, &t_SMat, &t_mat);
	}
	pSOut->_41 = m_SCurPos.x;
	pSOut->_42 = m_SCurPos.y;
	pSOut->_43 = m_SCurPos.z;
}

void CEffectCortrol::FillModelUVSoft(CEffectModel* pCModel) {
	SEFFECT_VERTEX* pVertex;
	pCModel->Lock((BYTE**)&pVertex);
	for (WORD i = 0; i < pCModel->GetVerCount(); ++i) {
		pVertex[i]._SUV = *_vecCurCoord[i];
	}
	pCModel->Unlock();
}

void CEffectCortrol::FillTextureUVSoft(CEffectModel* pCModel) {
	SEFFECT_VERTEX* pVertex;
	pCModel->Lock((BYTE**)&pVertex);
	for (WORD i = 0; i < pCModel->GetVerCount(); ++i) {
		pVertex[i]._SUV = *_lpCurTex[i];
	}
	pCModel->Unlock();
}

void CEffPath::Copy(CEffPath* pPath) {
	_iFrameCount = pPath->_iFrameCount;
	memcpy(_vecPath, pPath->_vecPath, sizeof(D3DXVECTOR3) * _iFrameCount);
	memcpy(_vecDist, pPath->_vecDist, sizeof(float) * (_iFrameCount - 1));
	memcpy(_vecDir, pPath->_vecDir, sizeof(D3DXVECTOR3) * (_iFrameCount - 1));
	m_fVel = pPath->m_fVel;
	Reset();
}

void CEffPath::FrameMove(float fDailTime) {
	if (_iFrameCount <= 0) {
		return;
	}
	m_bEnd = false;

	float fvel = m_fVel * fDailTime;
	m_fCurDist += fvel;
	while (m_fCurDist >= _vecDist[_iCurFrame]) {
		m_fCurDist -= _vecDist[_iCurFrame];
		_iCurFrame++;
		if (_iCurFrame >= _iFrameCount - 1) {
			_iCurFrame = 0;
			m_bEnd = true;
		}
	}
	m_vCurPos = _vecPath[_iCurFrame] + (_vecDir[_iCurFrame] * m_fCurDist);
}

D3DXVECTOR3* CEffPath::GetNextPos() {
	if (_iCurFrame >= _iFrameCount - 1) {
		return &m_vCurPos;
	}
	return &_vecPath[_iCurFrame - 1];
}

bool CMPModelEff::IsPlay() {
	for (int n = 0; n < _iEffNum; n++) {
		if (_vecCortrol[n]->IsPlay()) {
			return true;
		}
	}
	return false;
}

void CMPModelEff::Play(int iTime) {
	_bPlay = true;
	if (iTime > 0) {
		_bLoop = false;
		m_iTimes = iTime;
	}
	else {
		_bLoop = true;
		m_iTimes = 0;
	}
	for (int n = 0; n < _iEffNum; n++) {
		_vecCortrol[n]->Play();
		m_vecEffect[n]->PlayModel();
	}
}

void CMPModelEff::Play2(int iTime) {
	_bPlay = true;
	if (iTime > 0) {
		_bLoop = false;
		m_iTimes = iTime;
	}
	else {
		_bLoop = true;
		m_iTimes = 0;
	}
	for (int n = 0; n < _iEffNum; n++) {
		_vecCortrol[n]->Play();
	}
}

void CMPModelEff::Stop() {
	Reset();
	_bPlay = false;
	m_iTimes = 0;
}

void CMPModelEff::FreeEffect() {
	m_vecEffect.clear();
	_iEffNum = 0;
}

void CMPModelEff::BindingEffect(I_Effect* pCEffect) {
	_pCEffect = pCEffect;

	_iEffNum++;

	m_vecEffect.resize(_iEffNum);
	m_vecEffect[_iEffNum - 1] = pCEffect;

	_vecCortrol.setsize(_iEffNum);
	_vecCortrol[_iEffNum - 1]->_vecCurCoord.setsize((WORD)pCEffect->_pCModel->GetVerCount());
	_vecCortrol[_iEffNum - 1]->_lpCurTex.setsize((WORD)pCEffect->_pCModel->GetVerCount());
}

void CMPModelEff::BindingEffect(std::vector<I_Effect>& CEffectArray) {
	ClearEffect();
	int n;
	_bPlay = false;

	_iEffNum = (int)CEffectArray.size();
	m_vecEffect.clear();

	m_vecEffect.resize(_iEffNum);
	for (n = 0; n < _iEffNum; n++) {
		m_vecEffect[n] = new I_Effect;
		m_vecEffect[n]->CopyEffect(&CEffectArray[n]);
	}

	_vecCortrol.resize(_iEffNum);
	_vecCortrol.setsize(_iEffNum);
	for (n = 0; n < _iEffNum; n++) {
		_vecCortrol[n]->_vecCurCoord.setsize(CEffectArray[n]._CTexCoordlist._wVerCount);
		_vecCortrol[n]->_lpCurTex.setsize(CEffectArray[n]._CTexCoordlist._wVerCount);
	}
}

void CMPModelEff::BindingRes(CMPResManger* pResMagr) {
	_bPlay = false;

	m_pResMgr = pResMagr;
	int n;
	for (n = 0; n < _iEffNum; n++) {
		m_vecEffect[n]->BoundingRes(pResMagr);
	}

	m_pfDailTime = pResMagr->GetDailTime();
	_pCEffectFile = pResMagr->GetEffectFile();
	int idx = pResMagr->GetEffectID(m_vecEffect[0]->getEffectName());

	if (idx == -1) {
		const std::string szData = std::format("(ID{})", idx);
		MessageBox(NULL, szData.c_str(), "Error", MB_OK);
	}

	EffParameter* pParam = pResMagr->GetEffectParamByID(idx);
	_idxTech = pParam->_idxTech;

	_usePath = pParam->_usePath;
	_useSound = pParam->_useSound;
	_pathName = pParam->_pathName;
	_soundName = pParam->_soundName;

	_rotating = pParam->_rotating;
	_rotaVel = pParam->_rotaVel;
	_verRota = pParam->_verRota;

	if (_usePath) {
		m_pPath = pResMagr->GetEffPath(pResMagr->GetEffPathID(_pathName));
	}

	m_pMatViewProj = pResMagr->GetViewProjMat();
	_bUseSoft = pResMagr->_bUseSoft;

	D3DXMatrixIdentity(&_matTempRota);
}
