#pragma once

#include <Util.h>
//stl
#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>

#include "lwDirectX.h"

class MPRender;
class CMPResManger;

// TODO(Ф6): убрать `using namespace` в global. Сейчас удаление ломает
// транзитивный include MPSceneItem.h (i_effect.h:235) — там цикл с
// `class MPSceneItem` базовый для эффектов. Распутывается в Ф6 вместе с
// 12 макросами s_string/LIST_*/VEC_*.
using namespace Corsairs::Engine::Render;

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
#define    s_string								std::string
#define    LIST_string							std::list<s_string>
#define    VEC_string							std::vector<s_string>
#define    LIST_int								std::list<int>
#define    LIST_dword							std::list<DWORD>
#define    VEC_dword							std::vector<DWORD>
#define    VEC_word								std::vector<WORD>
#define    VEC_int								std::vector<int>
#define    VEC_bool								std::vector<bool>
#define    VEC_ptr								std::vector<DWORD>
#define    VEC_BYTE								std::vector<BYTE>
#define    VEC_float							std::vector<float>

template <class _Ty>
class S_BVECTOR {
	std::vector<_Ty> _VECPath;
	int _nCount;
	int _nPos;

public:
	S_BVECTOR() {
		_VECPath.clear();
		_nCount = 0;
		_nPos = 0;
	}

	void resize(WORD _nSize = 100) {
		_VECPath.clear();
		_VECPath.resize(_nSize);
		clear();
	}

	void addsize(WORD _nSize = 100) {
		WORD size = (WORD)_VECPath.size();
		_VECPath.resize(size + _nSize);
	}

	void setsize(WORD _nSize = 1) {
		if (_VECPath.size() < _nSize) {
			_VECPath.resize(_nSize);
		}
		clear();
		_nCount = _nSize;
	}

	void setsizeNew(WORD _nSize = 1) {
		std::vector<_Ty> temp;
		temp.resize(_nCount);
		for (WORD i = 0; i < _nCount; i++) {
			temp[i] = _VECPath[i];
		}

		_VECPath.resize(_nSize);
		for (WORD i = 0; i < _nCount; i++) {
			_VECPath[i] = temp[i];
		}
	}

	void clear() {
		_nCount = 0;
		_nPos = 0;
	}

	void push_back(_Ty& _Base) {
		_VECPath[_nCount] = _Base;
		++_nCount;
	}

	void pop_front() {
		if (_nCount > _nPos) {
			++_nPos;
			if (_nPos == _nCount) {
				clear();
			}
		}
	}

	void pop_back() {
		if (_nCount > _nPos) {
			_nCount--;
			if (_nPos == _nCount) {
				clear();
			}
		}
	}

	int size() const {
		return _nCount - _nPos;
	}

	bool empty() const {
		return _nPos == _nCount ? true : false;
	}

	_Ty* front() {
		if (!empty())
			return &_VECPath[_nPos];
		return NULL;
	}

	_Ty* next() {
		if (!empty()) {
			if ((_nPos + 1) != _nCount)
				return &_VECPath[_nPos + 1];
		}
		return NULL;
	}

	_Ty* end() {
		if (!empty()) {
			return &_VECPath[_nCount - 1];
		}
		return NULL;
	}

	_Ty* operator[](int i) {
		if (!empty() && i >= 0 && i < size()) {
			return &_VECPath[_nPos + i];
		}
		return NULL;
	}

	void remove(int _iIndex) {
		if (empty())
			return;
		int ipos = _nPos + _iIndex;

		for (int n = ipos; n < _nCount - 1; ++n) {
			_VECPath[n] = _VECPath[n + 1];
		}
		_nCount--;
		if (_nPos == _nCount) {
			clear();
		}
		return;
	}
};


class MPRender;
class CMPResManger;
class CEffectModel;
class CTexCoordList;
class CTexList;
class I_Effect;
class CEffPath;
class EffParameter;

//!40VS,95 - 15
#define  MAX_SHADER_VERNUM		300
#define  MAX_SHADER_IDXNUM		MAX_SHADER_VERNUM  * 3 * 3

/************************************************************************/
/*  */
/************************************************************************/
struct SEFFECT_VERTEX {
	D3DXVECTOR3 _SPos;
	FLOAT _fIdx; //MESHUV
	DWORD _dwDiffuse;
	D3DXVECTOR2 _SUV;
};

#define		EFFECT_VER_FVF	(D3DFVF_XYZB1 | D3DFVF_DIFFUSE | D3DFVF_TEX1)


struct SEFFECT_SHADE_VERTEX {
	D3DXVECTOR3 _SPos;
	//float			_fIdx;//MESH
	//_fIdx[0]_fIdx[1]uv
	DWORD _dwDiffuse;
	D3DXVECTOR2 _SUV;
	D3DXVECTOR2 _SUV2;
};

#define		EFFECT_SHADE_FVF	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)


#define		MESH_TRI		"Triangle"
#define		MESH_PLANETRI	"TrianglePlane"
#define		MESH_RECT		"Rect"
#define		MESH_RECTZ		"RectZ"
#define		MESH_PLANERECT	"RectPlane"

#define		MESH_CYLINDER	"Cylinder"
#define		MESH_CONE		"Cone"
#define		MESH_SPHERE		"Sphere"


inline bool IsTobMesh(const s_string& strName) {
	return ((strName == MESH_CYLINDER) || (strName == MESH_CONE) || (strName == MESH_SPHERE));
}

inline bool IsCylinderMesh(const s_string& strName) {
	return (strName == MESH_CYLINDER);
}

inline bool IsDefaultMesh(const s_string& strName) {
	static s_string str[] =
	{
		MESH_TRI,
		MESH_PLANETRI,
		MESH_RECT,
		MESH_RECTZ,
		MESH_PLANERECT,
		MESH_CYLINDER,
		MESH_CONE,
		MESH_SPHERE,
	};
	for (int n = 0; n < 8; ++n) {
		if (strName == str[n])
			return true;
	}
	return false;
}

#include "MPSceneItem.h"
#include "MPCharacter.h"

#define _MAX_STRING					256

// Одно действие персонажа: номер анимации, диапазон кадров, ключевые кадры.
struct ActionInfo {
	short _actionNo{0};
	short _startFrame{0};
	short _endFrame{0};
	std::vector<short> _keyFrames;

	bool IsValid() const {
		return _actionNo >= 1;
	}

	// Конвертация в engine-POD lwPoseInfo (DWORD-поля, фиксированный key_frame_seq[MAX_KEY_FRAME_NUM]).
	// Используется потребителями движка: MPParticleCtrl, CCharacterModel::LoadPose.
	lwPoseInfo ToPoseInfo() const {
		lwPoseInfo pi{};
		pi.start = static_cast<DWORD>(_startFrame);
		pi.end = static_cast<DWORD>(_endFrame);
		const size_t kfCount = (std::min)(_keyFrames.size(), static_cast<size_t>(MAX_KEY_FRAME_NUM));
		pi.key_frame_num = static_cast<DWORD>(kfCount);
		for (size_t k = 0; k < kfCount; k++) {
			pi.key_frame_seq[k] = static_cast<DWORD>(_keyFrames.at(k));
		}
		return pi;
	}
};

// Блок действий одного типа персонажа.
// _actions[i] соответствует actionNo = i + 1 (1-based). Ячейки с _actionNo == 0 — пустые.
struct SChaAction {
	short _characterType{0};
	short _validCount{0};
	std::vector<ActionInfo> _actions{};

	bool IsValid() const {
		return _characterType >= 1;
	}
};

// Кэш действий персонажей в Engine-формате.
// Заполняется из CharacterActionStore (SQLite) через LoadActionDataFromStore().
class CCharacterActionCache {
public:
	CCharacterActionCache() = default;
	~CCharacterActionCache() = default;

	// Загрузить данные анимаций из CharacterActionStore в кэш.
	bool LoadActionDataFromStore();
	void Free();

	// Безопасный доступ к блоку действий по 1-based characterType.
	// nullptr если вне диапазона или блок пустой.
	// Указатель валиден до следующего LoadActionDataFromStore() / Free().
	const SChaAction* GetCharAction(int characterType) const;

	short GetMaxCharacterType() const {
		return _maxCharacterType;
	}

	short GetActualCharacterType() const {
		return _actualCharacterType;
	}

	static CCharacterActionCache _cache;

protected:
	short _maxCharacterType{0};
	short _actualCharacterType{0};
	std::vector<SChaAction> _characterActions;
};

long StringGetT(char* out, long out_max, const char* in, long* in_from, const char* end_list, long end_len);
void StringSkipCompartmentT(const char* in, long* in_from, const char* skip_list, long skip_len);

// TODO(Ф6): см. комментарий выше (строка 16).
using namespace Corsairs::Engine::Render;struct ModelParam {
	int iSegments;
	float fHei;
	float fTopRadius;
	float fBottomRadius;
	std::vector<D3DXVECTOR3> vecVer;
	void Create();
};

class CEffectModel : public MPSceneItem {
public:
	CEffectModel();

	bool Copy(const CEffectModel& rhs);

	CEffectModel(MPRender* pDev, IResourceMgr* pRes = NULL);
	virtual ~CEffectModel();

public:
	void ReleaseModel();

	void InitDevice(MPRender* pDev, IResourceMgr* pRes = NULL);

	bool CreateTriangle();
	bool CreatePlaneTriangle();
	bool CreateRect();
	bool CreatePlaneRect();
	bool CreateRectZ();

	bool CreateShadeModel(WORD wVerNum = 6, WORD wFaceNum = 2, int iGridCrossNum = 1, bool usesoft = false);

	bool CreateTob(const s_string& str, int nSeg, float fHei, float fTopRadius, float fBottomRadius) {
		if (str == MESH_CYLINDER)
			return CreateCylinder(nSeg, fHei, fTopRadius, fBottomRadius);
		if (str == MESH_CONE)
			return CreateCone(nSeg, fHei, fBottomRadius);
		return false;
	}

	bool CreateCylinder(int nSeg, float fHei, float fTopRadius, float fBottomRadius);
	bool CreateCone(int nSeg, float fHei, float fRadius);


	bool IsBoard() {
		return (!IsChangeably() && !IsItem());
	}

	bool IsChangeably() {
		return _bChangeably;
	}

	bool IsItem() {
		return _lwMesh ? false : true;
	}

	bool LoadModel(const char* pszName);

	void Begin();

	void FrameMove(DWORD dwDailTime);
	void RenderModel();

	void RenderTob(ModelParam* last, ModelParam* next, float lerp);

	void End();

	DWORD GetVerCount() {
		return _dwVerCount;
	}

	DWORD GetFaceCount() {
		return _dwFaceCount;
	}


	ILockableStreamIB* GetIndexBuffer() {
		return _lpSIB;
	}

	ILockableStreamVB* GetVertexBuffer() {
		return _lpSVB;
	}

	void Lock(BYTE** pvEffVer);
	void Unlock();
	void LockIB(BYTE** pIdx);
	void UnlockIB();

	void SetRenderNum(WORD wVer, WORD wFace);

	MPRender* GetDev();

public:
	//!3D
	MPRender* _dev;

	IResourceMgr* _pRes;

	s_string _strName;

	bool _bChangeably;

	int _nSegments;
	float _rHeight;
	float _rRadius;
	float _rBotRadius;

	SEFFECT_VERTEX* _vEffVer;

	lwITex* _oldtex;
	lwITex* _oldtex2;
	bool _bItem;

	int _iID;

	bool _bUsing;

protected:
	lwIMesh* _lwMesh;
	ILockableStreamVB* _lpSVB;
	ILockableStreamIB* _lpSIB;


	DWORD _dwVerCount;
	DWORD _dwFaceCount;

public:
	// Getters and Setters
	bool IsUsing() {
		return _bUsing;
	}

	void SetUsing(bool bUsing) {
		_bUsing = bUsing;
	}
};

/************************************************************************/
/* */
/************************************************************************/
typedef std::vector<D3DXVECTOR2> TEXCOORD;

class CTexCoordList {
public:
	CTexCoordList();
	~CTexCoordList();

public:
	void GetCoordFromModel(CEffectModel* pCModel);
	void CreateTranslateCoord();

	void GetCurCoord(S_BVECTOR<D3DXVECTOR2>& vecOutCoord, WORD& wCurIndex, float& fCurTime, float fDailTime);

	void Reset();

	void Clear();

	void Copy(CTexCoordList* pList);

public:
	//!
	WORD _wVerCount;
	//.
	WORD _wCoordCount;
	float _fFrameTime;
	std::vector<TEXCOORD> _vecCoordList;

	////!
	////!
	//!
};

/************************************************************************/
/* */
/************************************************************************/
class CTexList {
public:
	CTexList();
	~CTexList();

public:
	void SetTextureName(const s_string& pszName);

	void GetTextureFromModel(CEffectModel* pCModel);

	void CreateSpliteTexture(int iRow, int iColnum);

	void GetCurTexture(S_BVECTOR<D3DXVECTOR2>& coord, WORD& wCurIndex, float& fCurTime, float fDailTime);
	void Reset();

	void Clear();

	void Remove();

	void Copy(CTexList* pList);

public:
	WORD _wTexCount;
	float _fFrameTime;
	std::vector<TEXCOORD> _vecTexList;

	//!
	s_string _vecTexName;
	//!
	IDirect3DTextureX* _lpCurTex;

	lwITex* _pTex;
};

/************************************************************************/
/* */
/************************************************************************/
class CTexFrame {
public:
	CTexFrame();
	~CTexFrame();

public:
	void GetCoordFromModel(CEffectModel* pCModel);

	void AddTexture(const s_string& pszName);

	lwITex* GetCurTexture(WORD& wCurIndex, float& fCurTime, float fDailTime);

	void Remove();

	void Copy(CTexFrame* pList);

public:
	WORD _wTexCount;
	float _fFrameTime;
	//!
	std::vector<s_string> _vecTexName;
	//!
	lwITex* _lpCurTex;

	std::vector<lwITex*> _vecTexs;

	TEXCOORD _vecCoord;
};

class CEffectFont : public CTexList, CEffectModel {
public:
	CEffectFont();
	~CEffectFont();

public:
	bool CreateEffectFont(MPRender* pDev,
						  CMPResManger* pCResMagr, int iTexID, D3DXCOLOR dwColor, bool bUseBack = false,
						  bool bmain = false);

	void SetRenderText(std::string_view pszText);

	void RenderEffectFont(D3DXMATRIX* pmat);
	void RenderEffectFontBack(D3DXMATRIX* pmat);

protected:
	s_string _strText;
	int _iTextNum;
	std::vector<int> _vecCurText;
	int _iTextureID;

	bool _bUseBack;
	s_string _strBackBmp;
	lwITex* _lpBackTex;

	SEFFECT_VERTEX t_SEffVer[4];


	D3DXCOLOR _dwColor;
};

enum EFFECT_TYPE {
	EFFECT_NONE = 0,
	EFFECT_FRAMETEX = 1,
	EFFECT_MODELUV = 2,
	EFFECT_MODELTEXTURE = 3,
	EFFECT_MODEL = 4,
};

#define ENUMTOSTR(s)  #s

/************************************************************************/
/* Effect base class */
/************************************************************************/
namespace Corsairs::Engine::Render { class EffectLoader; }

class I_Effect {
	// Loader имеет доступ к protected/private state'у — он сериализует все
	// поля как раньше делал I_Effect::SaveToFile/LoadFromFile.
	friend class ::Corsairs::Engine::Render::EffectLoader;

public:
	I_Effect(void);
	~I_Effect(void);

public:
	void DestroyTobMesh(CMPResManger* resMgr);

	//!
	virtual void ReleaseAll();
	//!
	virtual void Reset();
	virtual void Init(MPRender* pDev, EFFECT_TYPE eType,
					  WPARAM wParam, LPARAM lParam);

	void SetTexture();
	void SetVertexShader(); //		{ _dev->SetVertexShader(*_pdwVShader);}
	//!
	virtual void Begin() {
		if (_pCModel)
			_pCModel->Begin();
	}

	virtual void Render();

	virtual void End() {
		if (_pCModel)
			_pCModel->End();
	}

	//!
public:
	//!
	WORD getFrameCount() {
		return _wFrameCount;
	}

	WORD setFrameCount(WORD wFrameCount) {
		return _wFrameCount = wFrameCount;
	}

	//!
	EFFECT_TYPE getType() {
		return _eEffectType;
	}

	void setType(EFFECT_TYPE eType) {
		_eEffectType = eType;
	}

	//!
	float getLength() {
		return _fLength;
	}

	void setLength(float fLength) {
		_fLength = fLength;
	}

	//!
	float getFrameTime(WORD wIndex) {
		return _vecFrameTime[wIndex];
	}

	void setFrameTime(WORD wIndex, float fTime) {
		_vecFrameTime[wIndex] = fTime;
	}

	//!
	D3DXVECTOR3 getFrameSize(WORD wIndex) {
		return _vecFrameSize[wIndex];
	}

	void setFrameSize(WORD wIndex, D3DXVECTOR3& SVerSize) {
		_vecFrameSize[wIndex] = SVerSize;
	}

	//!
	D3DXVECTOR3& getFrameAngle(WORD wIndex) {
		return _vecFrameAngle[wIndex];
	}

	void setFrameAngle(WORD wIndex, D3DXVECTOR3& SVerAngle) {
		_vecFrameAngle[wIndex] = SVerAngle;
	}

	//!
	D3DXVECTOR3& getFramePos(WORD wIndex) {
		return _vecFramePos[wIndex];
	}

	void setFramePos(WORD wIndex, D3DXVECTOR3& SVerPos) {
		_vecFramePos[wIndex] = SVerPos;
	}

	//!
	D3DXCOLOR& getFrameColor(WORD wIndex) {
		return _vecFrameColor[wIndex];
	}

	void setFrameColor(WORD wIndex, D3DXCOLOR& SVerColor) {
		_vecFrameColor[wIndex] = SVerColor;
	}

	//!
	float getFrameCoordTime() {
		return _CTexCoordlist._fFrameTime;
	}

	void setFrameCoordTime(float fTime) {
		_CTexCoordlist._fFrameTime = fTime;
	}

	int getFrameCoordCount() {
		return (int)_CTexCoordlist._vecCoordList.size();
	}

	void setFrameCoordCount(int iNum) {
		_CTexCoordlist._wCoordCount = iNum;
		_CTexCoordlist._vecCoordList.resize(_CTexCoordlist._wCoordCount);
	}

	//!
	void getFrameCoord(TEXCOORD& vecOutCoord, WORD wIndex) {
		vecOutCoord.clear();
		vecOutCoord.resize(_CTexCoordlist._wVerCount);
		for (WORD n = 0; n < _CTexCoordlist._wVerCount; ++n) {
			vecOutCoord[n] = _CTexCoordlist._vecCoordList[wIndex][n];
		}
	}

	void setFrameCoord(TEXCOORD& vecInCoord, WORD wIndex) {
		_CTexCoordlist._vecCoordList[wIndex] = vecInCoord;
	}

	//!
	TEXCOORD& getFrameTexture(WORD wIndex) {
		return _CTextruelist._vecTexList[wIndex];
	}

	void setFrameTexture(WORD wIndex, TEXCOORD& lptex) {
		_CTextruelist._vecTexList[wIndex] = lptex;
	}

	void SpliteTexture(int iRow, int iCol);
	void SetTextureTime(float ftime);

	bool IsModelRect() {
		return _strModelName == MESH_RECT;
	}

	bool IsModelPlaneRect() {
		return _strModelName == MESH_PLANERECT;
	}

	bool IsModelTri() {
		return _strModelName == MESH_TRI;
	}

	bool IsItem();

	bool IsChangeably();

	void GetLerpSize(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp);
	void GetLerpAngle(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp);
	void GetLerpPos(D3DXVECTOR3* pSOut, WORD wIdx1, WORD wIdx2, float fLerp);
	void GetLerpColor(D3DXCOLOR* pSOut, WORD wIdx1, WORD wIdx2, float fLerp);

	//!
	void GetLerpCoord(S_BVECTOR<D3DXVECTOR2>& vecOutCoord, WORD& wCurIndex, float& fCurTime, float fDailTime) {
		_CTexCoordlist.GetCurCoord(vecOutCoord, wCurIndex, fCurTime, fDailTime);
	}

	void GetLerpTexture(S_BVECTOR<D3DXVECTOR2>& vecOutCoord, WORD& wCurIndex, float& fCurTime, float fDailTime) {
		_CTextruelist.GetCurTexture(vecOutCoord, wCurIndex, fCurTime, fDailTime);
	}

	void GetLerpFrame(WORD& wCurIndex, float& fCurTime, float fDailTime) {
		_CTexFrame.GetCurTexture(wCurIndex, fCurTime, fDailTime);
	}

	void GetLerpVertex(WORD wIdx1, WORD wIdx2, float fLerp) {
		_ilast = wIdx1;
		_inext = wIdx2;
		_flerp = fLerp;
	}

	void GetRotaLoopMatrix(D3DXMATRIX* pmat, float& pCurRota, float fTime);

	//!
	void BindingResInit(CMPResManger* _CResMagr);
	//!0123shade
	int BoundingRes(CMPResManger* _CResMagr, const char* pszParentName = "temp");

	s_string GetTextureName() {
		return _CTextruelist._vecTexName;
	}

	void SetTextureName(const s_string& pszName) {
		_CTextruelist.SetTextureName(pszName);
	}

	void SetModel(CEffectModel* pCModel);

	void ChangeTexture(const s_string& pszName) {
		if (_eEffectType == EFFECT_FRAMETEX) {
		}
		else {
			SetTextureName(pszName);
			_CTextruelist.GetTextureFromModel(_pCModel);
		}
	}

	//!
	void ChangeModel(CEffectModel* pCModel, CMPResManger* pCResMagr);

	//!
	s_string& getEffectName() {
		return _strEffectName;
	}

	void setEffectName(const s_string& strName) {
		_strEffectName = strName;
	}

	//!
	s_string& getEffectModelName();

	void setEffectModelName(const s_string& strModelName) {
		_strModelName = strModelName;
	}

	//!BILLBOARD
	void setBillBoardMatrix(D3DXMATRIX* pMatBBoard) {
		_SpmatBBoard = pMatBBoard;
	}

	D3DXMATRIX* getBillBoardMatrix() {
		return _SpmatBBoard;
	}

	bool IsBillBoard() {
		return _bBillBoard;
	}

	void SetLoop(bool bloop) {
		_bRotaLoop = bloop;
	}

	bool IsRotaLoop() {
		return _bRotaLoop;
	}

	D3DXVECTOR4* GetRotaLoop() {
		return &_vRotaLoop;
	}

	bool IsAlpah() {
		return _bAlpha;
	}

	void EnableAlpha(bool balpha) {
		_bAlpha = balpha;
	}


	D3DBLEND GetAlphaTypeSrc() {
		return _eSrcBlend;
	}

	D3DBLEND GetAlphaTypeDest() {
		return _eDestBlend;
	}

	void SetAlphaType(D3DBLEND eSrcBlend, D3DBLEND eDestBlend) {
		_eSrcBlend = eSrcBlend;
		_eDestBlend = eDestBlend;
	}

	// Сериализация перенесена в Corsairs::Engine::Render::EffectLoader::
	// {LoadElement,SaveElement} — data-классы по соглашению проекта не
	// должны содержать I/O.

	void IsSame();

	void RemoveTexBack() {
		_CTextruelist.Remove();
	}

	void AddFrameTex(const s_string& str) {
		if (_eEffectType == EFFECT_FRAMETEX) {
			_CTexFrame.AddTexture(str);
		}
	}

	VEC_string& GetFrameTex() {
		{
			return _CTexFrame._vecTexName;
		}
	}

	void RemoveFrameTex() {
		_CTexFrame.Remove();
	}

	void InitTopParam() {
		if (_iUseParam > 0)
			return;
		_iUseParam = 1;
	}

	void ClearTopParam() {
		_iUseParam = 0;
	}

	void SetTobParam(int nFrame, int nSegments, float rHeight, float rRadius, float rBotRadius);
	void GetTobParam(int nFrame, int& nSegments, float& rHeight, float& rRadius, float& rBotRadius);

	int IsUseParam() {
		return _iUseParam;
	}

	bool IsRotaBoard() {
		return _bRotaBoard;
	}

	void SetRoatBoard(bool bRota) {
		_bRotaBoard = bRota;
	}


	void GetRes(CMPResManger* pCResMagr, std::vector<INT>& vecTex, std::vector<INT>& vecModel);

	void PlayModel() {
		if (!_pCModel) {
			return;
		}
		if (_pCModel->IsItem())
			_pCModel->PlayDefaultAnimation();
	}

	void ResetModel();
	void CopyEffect(I_Effect* pEff);

	void DeleteItem(CMPResManger* pResMgr);

public:
	//!3D
	MPRender* _dev;
	//!
	CTexCoordList _CTexCoordlist;
	//!
	CTexList _CTextruelist;
	//!
	CTexFrame _CTexFrame;

	CEffectModel* _pCModel;
	s_string _strModelName;

	int _nSegments;
	float _rHeight;
	float _rRadius;
	float _rBotRadius;

	s_string _strEffectName;

	int _ilast, _inext;
	float _flerp;

protected:
	//!
	EFFECT_TYPE _eEffectType;
	//()
	float _fLength;
	WORD _wFrameCount;
	//!
	VEC_float _vecFrameTime;
	//!	(1.0f)
	std::vector<D3DXVECTOR3> _vecFrameSize;
	//!
	std::vector<D3DXVECTOR3> _vecFrameAngle;
	//!
	std::vector<D3DXVECTOR3> _vecFramePos;

	//!	(0xffffffff)
	std::vector<D3DXCOLOR> _vecFrameColor;

	//!
	INT _iUseParam;
	std::vector<ModelParam> _CylinderParam;

	bool _bBillBoard;
	D3DXMATRIX* _SpmatBBoard;

	bool _bRotaLoop;
	D3DXVECTOR4 _vRotaLoop;

	bool _bRotaBoard;


	bool _bAlpha;

	int _iVSIndex;


	bool _bSizeSame;
	bool _bAngleSame;
	bool _bPosSame;
	bool _bColorSame;

	D3DBLEND _eSrcBlend;
	D3DBLEND _eDestBlend;
};

//	CEffectBase(){}
//	~CEffectBase(){}
//	//!

void Transpose(D3DMATRIX& result, D3DMATRIX& m);
