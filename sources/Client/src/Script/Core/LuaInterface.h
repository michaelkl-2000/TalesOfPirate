#include "uiguidata.h"
#include "sceneobjset.h"
#include "Effect/EffectRecordStore.h"

//---------------------------------------------------------------------------
// Info_Script
//---------------------------------------------------------------------------
extern int GetChaPhotoTexID(int nTypeID);

inline int GetEffectPhotoTexID(int nTypeID) {
	Corsairs::Common::Effect::CEffectInfo* pInfo = Corsairs::Common::Effect::GetEffectInfo(nTypeID);
	if (pInfo) {
		return pInfo->nPhotoTexID;
	}
	return 0;
}

inline int GetTerrainTextureType(int nID) {
	MPTerrainInfo* pInfo = GetTerrainInfo(nID);
	if (!pInfo) {
		return -1;
	}
	return pInfo->btType;
}

inline int GetSceneObjPhotoTexID(int nTypeID) {
	CSceneObjInfo* pInfo = GetSceneObjInfo(nTypeID);
	if (pInfo) {
		return pInfo->_photoTexId;
	}
	return 0;
}

inline int GetSceneObjPhotoTexType(int nTypeID) {
	CSceneObjInfo* pInfo = GetSceneObjInfo(nTypeID);
	if (pInfo) {
		return pInfo->_style;
	}
	return -1;
}
