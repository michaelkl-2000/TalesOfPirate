#pragma once
class MPRender;

#include "i_effect.h"
#include "EffectFile.h"


#include "mpshademap.h"

#include "mpparticlectrl.h"

#include "MPResourceSet.h"


template <class _Ty>
class S_FVector {
	std::vector<_Ty> m_VECPath;
	int m_nCount;
	int m_nPos;

public:
	S_FVector() {
		m_VECPath.clear();
		m_nCount = 0;
		m_nPos = 0;
	}

	void resize(WORD _nSize = 100) {
		m_VECPath.resize(_nSize);
		m_nCount = _nSize;
		clear();
	}

	void setsize(WORD _nSize = 100) {
		m_nPos -= _nSize;
	}

	void clear() {
		m_nPos = m_nCount;
	}

	void push_front(_Ty& _BaseMesh) {
		--m_nPos;
		m_VECPath[m_nPos] = _BaseMesh;
	}

	void pop_front() {
		if (m_nCount > m_nPos) {
			++m_nPos;
			if (m_nPos == m_nCount) {
				clear();
			}
		}
	}

	int size() const {
		return m_nCount - m_nPos;
	}

	bool empty() {
		return m_nPos == m_nCount ? true : false;
	}

	_Ty* operator[](int i) {
		if (!empty() && i >= 0 && i < size()) {
			return &m_VECPath[m_nPos + i];
		}
		return NULL;
	}

	_Ty* front() {
		if (!empty())
			return &m_VECPath[m_nPos];
		return NULL;
	}

	_Ty* next() {
		if (!empty()) {
			if ((m_nPos + 1) != m_nCount)
				return &m_VECPath[m_nPos + 1];
		}
		return NULL;
	}
};

inline bool fEquat(float f1, float f2) {
	return fabs(f1 - f2) < 0.000001f;
}

// Точка входа в подсистему эффектов — façade поверх специализированных сторов
// из Corsairs::Engine::Render (EffPathStore / EffectStore / EffectMeshStore /
// EffectShaderStore / ParticleCtrlStore / ParticleInstancePool / TobMeshStore /
// EffectFxRenderer / EffectRenderContext / EffectDeviceCallbacks). Сама не
// владеет mesh/effect/particle-данными — только координирует InitRes-конвейер
// и проксирует legacy callsite.
class CMPResManger {
public:
	static CMPResManger& Instance();

	CMPResManger(const CMPResManger&)            = delete;
	CMPResManger& operator=(const CMPResManger&) = delete;

	void Clear();

	//  Управление прогревом массовых ресурсов при старте.
	//  m_GeomobjMap (character meshes) и m_AnimDataMap (bones), чтобы
	//  первые операции с персонажами не делали file IO.
	//  Когда false — прогрев пропускается, ресурсы грузятся on-demand
	//  через lwPhysique. Полезно для быстрого dev-loop'а / отладки
	//  on-demand путей. Раньше управлялось define'ом _UNLOADRES.
	//  Клиент устанавливает значение из [Resources] preload_at_start
	//  в Client/user/system.ini до InitRes3.
	static void SetResourcePreload(bool enabled) {
		s_resourcePreload = enabled;
	}

	static bool IsResourcePreload() {
		return s_resourcePreload;
	}

	bool LoadTotalVShader(lwISysGraphics* sys_graphics);

	bool InitRes(MPRender* pDev, D3DXMATRIX* pmat, D3DXMATRIX* pMatviewproj);

	bool InitRes2();
	bool InitRes3();

	void ReleaseTotalRes();

	int GetTexNum() {
		return _iTexNum;
	}

	int GetMeshNum();

	int GetEffectNum();
	int GetSubEffectNum(int idx);

	I_Effect* AddEffectToMgr(const s_string& strName);
	void AddUniteEffectToMgr(std::vector<I_Effect>& vecEffArray);

	int GetTextureID(const s_string& pszName);
	IDirect3DTextureX* GetTextureByID(int iID);

	lwITex* GetTextureByIDlw(int iID);
	lwITex* GetTextureByNamelw(const s_string& sName);

	int GetMeshID(const s_string& pszName);
	CEffectModel* GetMeshByID(int iID);
	CEffectModel* GetMeshByName(const s_string& sName);
	CEffectModel* GetShadeMesh();


	void DeleteMesh(CEffectModel& rEffectModel);

	int GetEffectID(const s_string& pszName);
	std::vector<I_Effect>& GetEffectByID(int iID);
	I_Effect* GetSubEffectByID(int iID, int iSubIdx);

	EffParameter* GetEffectParamByID(int iID);

	IDirect3DVertexShaderX* GetVShaderByID(int iID);
	IDirect3DVertexDeclarationX* GetVDeclByID(int iID);
	IDirect3DVertexShaderX* GetShadeVS();
	IDirect3DVertexDeclarationX* GetShadeVDecl();

	IDirect3DVertexShaderX* GetMinimapVS();

	IDirect3DVertexDeclarationX* GetMinimapVDecl();


	int GetEffPathID(const s_string& pszName);
	CEffPath* GetEffPath(int iID);

	CMPEffectFile* GetEffectFile();

	void FrameMove(DWORD dwTime);
	void Render();
	void UpdateMatrix();

	float* GetDailTime() {
		return &_fDailTime;
	}

	D3DXMATRIX* GetBBoardMat();
	D3DXMATRIX* GetViewProjMat();
	D3DXMATRIX* Get2DViewProjMat();

	int GetBackBufferWidth();
	int GetBackBufferHeight();

	int& GetFontBkWidth();
	int& GetFontBkHeight();

	D3DCAPSX* GetDevCap() {
		return &m_caps;
	}

	void BeginEffect(int iIdx);
	void EndEffect();

	void RestoreEffect();

	//!
	VEC_string& GetTotalTexName() {
		return _vecTexName;
	}

	VEC_string& GetTotalMeshName();

	VEC_string& GetTotalEffectName();


	bool IsCanFrame() {
		return m_bCanFrame;
	}

	int GetCanFrame() {
		return m_iCurFrame;
	}


	int GetPartCtrlID(const s_string& pszName);
	CMPPartCtrl* GetPartCtrlByID(int iID);

	int GetPartCtrlNum();

	CEffectModel* NewTobMesh();
	bool DeleteTobMesh(CEffectModel& rEffectModel);

	int GetTobMeshNum();

	CMPPartCtrl* NewPartCtrl(const s_string& strName);
	void DeletePartCtrl(int iID);


	void SendResMessage(const s_string& strPartName, D3DXVECTOR3 vPos, MPMap* pMap);

public:
	//!3D
	MPRender* m_pDev;

	D3DCAPSX m_caps;

	bool m_bUseSoft;
	bool m_bUseSoftOrg;

	bool m_bCanFrame;
	int m_iCurFrame;


	lwISystem* m_pSys;
	lwISysGraphics* m_pSysGraphics;

protected:
	CMPResManger();
	~CMPResManger();

	void LoadTotalRes();
	void LoadTotalData();
	bool LoadTotalTexture();
	bool LoadTotalMesh();
	bool LoadTotalEffect();
	bool LoadTotalPath();

	void LoadTotalPartCtrl();

protected:
	static bool s_resourcePreload;

	std::string _pszTexPath;
	std::string _pszMeshPath;
	std::string _pszEFFectPath;

	int _iTexNum;

	VEC_string _vecTexName;

	typedef std::map<std::string, int> TEXTURE_MAP;
	TEXTURE_MAP _mapTexture;

	// Local effect-ID → global TextureManager-ID. Сами lwITex*-объекты
	// владеет TextureManager (Phase 2 migration); CMPResManger хранит только
	// маппинг для совместимости с .par/.eff (которые ссылаются на local-ID).
	std::vector<int> _vecTexGlobalId;

	float _fSaveTime;
	float _fCurTime;
	float _fDailTime;
	bool _bInitTime;
};
