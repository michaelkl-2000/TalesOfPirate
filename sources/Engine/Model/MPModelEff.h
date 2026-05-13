#pragma once
#include "MindPowerAPI.h"


#include "mpresmanger.h"
class MPRender;

class CEffectCortrol {
public:
	CEffectCortrol();

public:
	void Reset();
	void Play();
	void Stop();

	bool IsPlay() {
		return _bPlay;
	}

	void GetTransformMatrix(D3DXMATRIX* pSOut, D3DXMATRIX* pRota = NULL);
	void FillModelUV(CEffectModel* pCModel);
	void FillTextureUV(CEffectModel* pCModel);
	void FillDefaultUV(CEffectModel* pCModel, TEXCOORD& coord);
	void FillModelUVSoft(CEffectModel* pCModel);
	void FillTextureUVSoft(CEffectModel* pCModel);
	//!
	float _fCurTime;
	//!
	WORD _wCurFrame;
	//!Diffuse
	D3DXCOLOR m_dwCurColor;
	//!
	D3DXVECTOR3 m_SCurSize;
	//!
	D3DXVECTOR3 m_SCurAngle;
	//!	
	D3DXVECTOR3 m_SCurPos;

	//!
	WORD m_wCurCoordIndex;
	//!
	float m_fCurCoordTime;
	//!
	S_BVECTOR<D3DXVECTOR2> _vecCurCoord;
	//!
	WORD m_wCurTexIndex;
	//!
	float m_fCurTexTime;
	//!
	S_BVECTOR<D3DXVECTOR2> _lpCurTex;
	int m_iCurTimes;

	bool _bPlay;

	D3DXMATRIX _SMatResult;

	float _fCurRotat;
};


class CEffPath {
public:
	CEffPath() {
		m_fVel = 1.f;
		_iCurFrame = 0;
		m_fCurDist = 0;
		m_bEnd = false;
	}

	~CEffPath() {
	}

	void Copy(CEffPath* pPath);

	void Reset() {
		_iCurFrame = 0;
		m_vCurPos = _vecPath[0];
		m_fCurDist = 0;
	}

	void SetVel(float fvel) {
		m_fVel = fvel;
	}

	float GetVel() {
		return m_fVel;
	}

	// I/O (файловое .csf/.let и in-place embedding в .par) —
	// в Corsairs::Engine::Render::EffPathLoader (см. AssetLoaders.h).
	void FrameMove(float fDailTime);

	D3DXVECTOR3* GetCurPos() {
		return &m_vCurPos;
	}

	D3DXVECTOR3* GetNextPos();

	D3DXVECTOR3* GetCurDir() {
		return &_vecDir[_iCurFrame];
	}

	float GetCurDist() {
		return _vecDist[_iCurFrame];
	}

	D3DXVECTOR3* GetStart() {
		return &_vecPath[0];
	}

	D3DXVECTOR3* GetEnd() {
		return &_vecPath[_iFrameCount - 1];
	}

	bool IsEnd() {
		return m_bEnd;
	}

public:
	bool m_bEnd;

	D3DXVECTOR3 _vecPath[200];
	float _vecDist[200];
	D3DXVECTOR3 _vecDir[200];

	int _iFrameCount;
	float m_fVel;

	int _iCurFrame;
	float m_fCurDist;
	D3DXVECTOR3 m_vCurPos;
};

class CEffPathCtrl {
public:
	CEffPathCtrl() {
		_fCurTime = 0;
		_fFrameTime = 0.1f;
	}

	~CEffPathCtrl() {
	}

	void Update(float dwDailTime, CEffPath* pPath) {
		_fCurTime += dwDailTime;
		if (_fCurTime >= _fFrameTime) {
			_iCurFrame++;
			if (_iCurFrame >= pPath->_iFrameCount)
				_iCurFrame = 0;
			_fCurTime = 0;
		}
		if (_iCurFrame == pPath->_iFrameCount - 1)
			m_iNextFrame = _iCurFrame;
		else
			m_iNextFrame = _iCurFrame + 1;
		m_fLerp = _fCurTime / _fFrameTime;
	}

public:
	int _iCurFrame;
	int m_iNextFrame;
	float m_fLerp;

	float _fCurTime;
	float _fFrameTime;

	D3DXVECTOR3 m_SCurPath;
};


class EffParameter {
public:
	EffParameter() {
		_usePath = false;
		_pathName = "";
		_useSound = false;
		_soundName = "";
		_idxTech = 0;
		_rotating = false;
		_rotaVel = 1;
		_verRota = D3DXVECTOR3(0, 0, 0);
	}

	~EffParameter() {
	}

	bool _usePath;
	s_string _pathName;
	bool _useSound;
	s_string _soundName;
	int _idxTech;

	bool _rotating;
	float _rotaVel;
	D3DXVECTOR3 _verRota;
};

class CMPModelEff // : public CEffectBase
{
public:
	CMPModelEff(void);
	~CMPModelEff(void);

public:
	//!	
	virtual void ReleaseAll();
	//!
	virtual void Reset();
	//!
	virtual void FrameMove(DWORD dwDailTime);
	//!
	virtual void Render();
	virtual void RenderVS();
	virtual void RenderSoft();

	void FrameMoveAccel(float fDail);
	void Begin();
	void RenderAccel(float& fTime);
	void End();

	void ShowCurFrame(int iCurSubEff, int iCurFrame);

	void ShowTempFrame(int iCurSubEff,
					   D3DXVECTOR3& pScale, D3DXVECTOR3& pRotating, D3DXVECTOR3& pTranslate,
					   D3DXCOLOR& pColor, TEXCOORD& vecCoord, IDirect3DTextureX* lpTex);

	bool IsLoop() {
		return _bLoop;
	}

	bool IsPlaying() {
		return _bPlay;
	}

	bool IsPlay();

	//!
	void Play(int iTime = 0);
	void Play2(int iTime = 0);
	void Stop();

	void FreeEffect();
	void BindingEffect(I_Effect* pCEffect);
	void BindingEffect(std::vector<I_Effect>& CEffectArray);
	void BindingRes(CMPResManger* pResMagr);

	int GetSubEffectFrameCount(int idx) {
		if (_iEffNum <= 0)
			return 0;

		return m_vecEffect[idx]->getFrameCount();
	}

	float GetSubEffectFrameTime(int idx, int iframe) {
		if (_iEffNum <= 0)
			return 0;

		return m_vecEffect[idx]->getFrameTime(iframe);
	}


	void BindingBone(D3DXMATRIX pmatBone, bool bFollow = false) {
		_bBindbone = bFollow;
		if (_bBindbone)
			_SpmatBone = pmatBone;
		else
			D3DXMatrixIdentity(&_SpmatBone);
	}

	void Scaling(float x, float y, float z) {
		D3DXMatrixScaling(&_matScale, x, y, z);
		_verScale.x = x;
		_verScale.y = y;
		_verScale.z = z;
	}

	float GetScalingX() {
		return _verScale.x;
	}

	float GetScalingY() {
		return _verScale.y;
	}

	float GetScalingZ() {
		return _verScale.z;
	}

	void RotatingYaw(float fDeg) //
	{
		if (_rotating)
			return;
		_verRota.z = fDeg;

		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verRota.y, _verRota.x, _verRota.z);
	}

	float GetRotaingYaw() {
		return _verRota.z;
	}

	void RotatingPitch(float fDeg) //!
	{
		if (_rotating)
			return;
		_verRota.x = fDeg;

		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verRota.y, _verRota.x, _verRota.z);
	}

	float GetRotaingPitch() {
		return _verRota.x;
	}

	void RotatingRoll(float fDeg) //!
	{
		if (_rotating)
			return;
		_verRota.y = fDeg;

		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verRota.y, _verRota.x, _verRota.z);
	}

	float GetRotaingRoll() {
		return _verRota.y;
	}


	void RotatingAxis(D3DXVECTOR3* pAxis, float fAngle) {
		D3DXMatrixRotationAxis(&_matRota, pAxis, fAngle);
	}

	void MoveTo(float x, float y, float z) {
		D3DXMatrixTranslation(&_matTrans, x, y, z);
		_verTrans.x = x;
		_verTrans.y = y;
		_verTrans.z = z;
	}

	//FrameMove,Render
	void SetAlpha(float fAlpha) {
		for (int n = 0; n < _iEffNum; ++n) {
			_pCurCortrol = _vecCortrol[n];
			_pCurCortrol->m_dwCurColor.a = fAlpha;
		}
	}

	void GetRunningDummyMatrix(D3DXMATRIX* pmat, int idx) {
		if (m_vecEffect[0]->IsItem()) {
			m_vecEffect[0]->_pCModel->GetObjDummyRunTimeMatrix((lwMatrix44*)pmat, idx);
		}
	}

	void SetRenderIdx(int idx) {
		_idxTech = idx;
	}

	void UseZBuffer(bool bused) {
		_bUseZ = bused;
	}

	void RotatingXZ(float fAngleX, float fAngleZ) {
		D3DXMATRIX mat;
		D3DXMatrixRotationX(&mat, fAngleX);
		D3DXMatrixRotationZ(&_matTempRota, fAngleZ);
		D3DXMatrixMultiply(&_matTempRota, &mat, &_matTempRota);
	}


	void RotatingYawPart(float fDeg) //
	{
		_verPartRota.z = fDeg;
		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verPartRota.y, _verPartRota.x, _verPartRota.z);
	}

	void RotatingPitchPart(float fDeg) //!
	{
		_verPartRota.x = fDeg;
		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verPartRota.y, _verPartRota.x, _verPartRota.z);
	}

	void RotatingRollPart(float fDeg) //!
	{
		_verPartRota.y = fDeg;
		D3DXMatrixRotationYawPitchRoll(&_matTempRota,
									   _verPartRota.y, _verPartRota.x, _verPartRota.z);
	}

	void GetTransMatrix(D3DXMATRIX& mat) {
		if (_rotating) {
			_fCurRotat += _rotaVel * *m_pfDailTime;
			if (_fCurRotat >= 6.283185f)
				_fCurRotat = _fCurRotat - 6.283185f;
			D3DXMatrixRotationAxis(&_matRota,
								   &_verRota, _fCurRotat);
		}

		D3DXMatrixMultiply(&mat, &_matRota, &_matTempRota);

		D3DXMatrixMultiply(&mat, &_matScale, &mat);
		D3DXMatrixMultiply(&mat, &mat, &_matTrans);
	}

	void ClearEffect();

public:
	CMPResManger* m_pResMgr;
	bool _bUseSoft;
	int _iEffNum;

	std::vector<I_Effect*> m_vecEffect;
	I_Effect* _pCEffect;

	S_BVECTOR<CEffectCortrol> _vecCortrol;
	CEffectCortrol* _pCurCortrol;


	float m_fLerp;
	bool _bLoop;

	bool _bPlay;

	int m_iTimes;

	float* m_pfDailTime;

	D3DXMATRIX _matScale;
	D3DXVECTOR3 _verScale;

	D3DXMATRIX _matRota;
	D3DXVECTOR3 _verRota;

	D3DXVECTOR3 _verPartRota;

	D3DXMATRIX _matTrans;
	D3DXVECTOR3 _verTrans;

	bool _bBindbone;
	D3DXMATRIX _SpmatBone;
	D3DXMATRIX _SMatResult;

	D3DXMATRIX _matTempRota;

	int _idxTech;
	CMPEffectFile* _pCEffectFile;

	D3DXMATRIX* m_pMatViewProj;
	bool _usePath;
	s_string _pathName;
	CEffPath* m_pPath;
	CEffPathCtrl m_CPathCtrl;
	bool _useSound;
	s_string _soundName;
	bool _rotating;
	float _rotaVel;
	float _fCurRotat;

	bool _bUseZ;
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
struct Strip_Vertex {
	D3DXVECTOR3 _SPos;
	DWORD _dwDiffuse;
	D3DXVECTOR2 _SUV;
};

#define		STRIP_FVF	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

namespace Corsairs::Engine::Render { class PartCtrlLoader; }

class CMPStrip {
	// .par-сериализация — в PartCtrlLoader::{Load,Save}Strip.
	friend class ::Corsairs::Engine::Render::PartCtrlLoader;

public:
	CMPStrip();
	~CMPStrip();

	void CreateStrip(MPRender* pDev, const s_string& strTexName, CMPResManger* pResMagr) {
		_dev = pDev;
		_strTexName = strTexName;
		BindingRes(pResMagr);
	}

	void BindingRes(CMPResManger* pResMagr) {
		_dev = pResMagr->_dev;
		_pCEffFile = pResMagr->GetEffectFile();
		int id = pResMagr->GetTextureID(_strTexName);
		if (id < 0) {
			_pTex = NULL;
		}
		else {
			_pTex = pResMagr->GetTextureByIDlw(id);
		}
		m_pfDailTime = pResMagr->GetDailTime();

		_vecPath.resize(m_iMaxLen);
		_vecCtrl.resize(m_iMaxLen / 2);
	}

	struct track {
		track() {
			_fCurTime = 0;
		}

		void FrameMove(float fDailTime, D3DXCOLOR& dwColor, float fLife) {
			if (_fCurTime >= fLife) {
				dwColor.a = 0;
				return;
			}
			dwColor.a = 1.0f + ((-1.0f) * (_fCurTime / fLife));
			_fCurTime += fDailTime;
		}

		float _fCurTime;
	};

	void Play();

	bool IsPlaying() {
		return _bPlay;
	}

	void UpdateFrame();
	void FrameMove();
	void Render();

public:
	void AttachCharacter(MPCharacter* pCha) {
		_pCha = pCha;
	}

	void AttachItem(MPSceneItem* pItem) {
		_pItem = pItem;
	}

	void SetDumy(int iIdx, int iDummy) {
		_iDummy[iIdx] = iDummy;
	}

	int GetDumy(int iIdx) {
		return _iDummy[iIdx];
	}


	void GetTrack(lwMatrix44* dummy1, lwMatrix44* dummy2) {
		Strip_Vertex path;
		track tul;
		int count = _vecPath.size();
		float d = (float)1/*(m_iMaxLen/2)*/;
		if (count >= m_iMaxLen) {
			if (!_bLoop) {
				_bPlay = false;
				return;
			}
			else {
				m_iMaxLen += 100;
				_vecPath.addsize(100);
				_vecCtrl.addsize(50);
				if (m_iMaxLen > 2000) {
					_bPlay = false;
					return;
				}
			}
		}
		if (count <= 0) {
			path._SPos.x = dummy1->_41;
			path._SPos.y = dummy1->_42;
			path._SPos.z = dummy1->_43;

			path._dwDiffuse = _dwColor;
			path._SUV.x = 0;
			path._SUV.y = 1;
			_vecPath.push_back(path);

			path._SPos.x = dummy2->_41;
			path._SPos.y = dummy2->_42;
			path._SPos.z = dummy2->_43;

			path._dwDiffuse = _dwColor;
			path._SUV.x = 0;
			path._SUV.y = 0;
			_vecPath.push_back(path);
			_vecCtrl.push_back(tul);
		}
		else {
			path._SPos.x = dummy1->_41;
			path._SPos.y = dummy1->_42;
			path._SPos.z = dummy1->_43;

			path._dwDiffuse = _dwColor;
			path._SUV.x = _vecPath[count - 2]->_SUV.x + d;
			path._SUV.y = 1;
			_vecPath.push_back(path);

			path._SPos.x = dummy2->_41;
			path._SPos.y = dummy2->_42;
			path._SPos.z = dummy2->_43;

			path._dwDiffuse = _dwColor;
			path._SUV.x = _vecPath[count - 1]->_SUV.x + d;
			path._SUV.y = 0;
			_vecPath.push_back(path);
			_vecCtrl.push_back(tul);
		}
	}

	float GetLife() {
		return _fLife;
	}

	void SetLife(float flife) {
		_fLife = flife;
	}

	float GetStep() {
		return _fStep;
	}

	void SetStep(float fstep) {
		_fStep = fstep;
	}

	D3DXCOLOR GetColor() {
		return _dwColor;
	}

	void SetColor(D3DXCOLOR color) {
		_dwColor = color;
	}

	int GetMaxLength() {
		return m_iMaxLen;
	}

	void SetMaxLength(int len) {
		m_iMaxLen = len;
		_vecPath.resize(m_iMaxLen);
		_vecCtrl.resize(m_iMaxLen / 2);
	}

	s_string GetTexName() {
		return _strTexName;
	}

	void SetTexName(const s_string& str, CMPResManger* pResMagr) {
		_strTexName = str;
		int id = pResMagr->GetTextureID(_strTexName);
		if (id < 0)
			_pTex = NULL;
		else {
			_pTex = pResMagr->GetTextureByIDlw(id);
		}
	}

	D3DBLEND GetSrcBlend() {
		return _eSrcBlend;
	}

	void SetSrcBlend(D3DBLEND srcblend) {
		_eSrcBlend = srcblend;
	}

	D3DBLEND GetDestBlend() {
		return _eDestBlend;
	}

	void SetDestBlend(D3DBLEND destblend) {
		_eDestBlend = destblend;
	}

	void CopyStrip(CMPStrip* pstrip);

	void SetLoop(bool bloop) {
		_bLoop = bloop;
	}

protected:
	MPRender* _dev;

	bool _bLoop;

	int m_iMaxLen;

	MPCharacter* _pCha;
	MPSceneItem* _pItem;
	int _iDummy[2]; //DUMY

	D3DXCOLOR _dwColor;
	float _fLife;
	float _fStep;

	S_BVECTOR<Strip_Vertex> _vecPath;
	S_BVECTOR<track> _vecCtrl;

	lwITex* _pTex;
	s_string _strTexName;

	CMPEffectFile* _pCEffFile;

	float* m_pfDailTime;
	float _fCurTime;
	bool _bPlay;

	D3DBLEND _eSrcBlend;
	D3DBLEND _eDestBlend;
};

// Чисто-данные обёртка одного .eff-файла. Используется
// Corsairs::Engine::Render::EffectLoader для Load/Save/LoadEx; CMPResManger
// поверх неё может делать engine-binding (Reset/_dev) уже в рантайм-callsite.
//
// Layout файла .eff:
//   [DWORD       version]               — version из заголовка (на момент чтения).
//   [int         _idxTech]
//   [bool        _usePath][char[32]   _pathName]
//   [bool        _useSound][char[32]  _soundName]
//   [bool        _rotating]
//   [D3DXVECTOR3 _verRota][float      _rotaVel]
//   [int         count]
//   [count × EffectLoader::SaveElement/LoadElement сериализация]
struct EffectFileInfo {
	std::uint32_t          version{0};
	EffParameter           param{};
	std::vector<I_Effect>  effects{};
};
