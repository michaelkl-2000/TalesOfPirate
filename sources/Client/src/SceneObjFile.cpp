#include	"Stdafx.h"
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<time.h>
#include	"SceneObjFile.h"
#include "SceneFileLoaders.h"
#include	"GameConfig.h"
#include "scene.h"
#include "GameApp.h"
#include "SceneObj.h"
#include "SceneObjRecordStore.h"

CSceneObjFile g_ObjFile;


CSceneObjFile::CSceneObjFile() {
	m_bInitSuccess = false;
	m_fRdWr = NULL;
	m_fAppend = NULL;
	m_SSectionIndex = NULL;
}

CSceneObjFile::~CSceneObjFile() {
	Free();
	//end_RBO();
}

// Added by clp
void CSceneObjFile::_init_RBO(const std::string& filename) {
	filename_RBO = filename + "rbo";
	RBOinfoList.clear();
	//std::ifstream file ( filename_RBO.c_str() );
	//struct ReallyBigObjectInfo info;
	//while ( file >> info )
	//{
	//	RBOinfoList.insert( info );
	//}
	//file.close();
}

void CSceneObjFile::end_RBO() {
	_Serialize_RBO();
	//_Serialize_RBO_ToMap();
}

void CSceneObjFile::_Serialize_RBO() {
	if (!filename_RBO.empty()) {
		// RboLoader::Save сам пропускает пустой items (раньше пустой список
		// порождал 0-байтный .rbo на диске).
		Corsairs::Engine::Scene::RboLoader::Save(filename_RBO, RBOinfoList);
	}
}

void CSceneObjFile::_Serialize_RBO_ToMap() {
	SSceneObjInfo infoex[MAX_MAP_SECTION_OBJ];
	CGameScene* scene = CGameApp::GetCurScene();
	MPTerrain* terrain = scene->GetTerrain();

	std::set<struct ReallyBigObjectInfo>::iterator itr = RBOinfoList.begin();
	std::set<struct ReallyBigObjectInfo>::iterator end = RBOinfoList.end();

	while (itr != end) {
		int sectionX = itr->position.x / (100.f * terrain->GetSectionWidth());
		int sectionY = itr->position.y / (100.f * terrain->GetSectionHeight());

		int sectionNumber = sectionY * terrain->GetSectionCntX() + sectionX;

		long sectionObjCount = 0;

		if (ReadSectionObjInfo(sectionNumber, infoex, &sectionObjCount)) {
			sectionObjCount++;
			int RBOIndex = sectionObjCount - 1;
			if (RBOIndex <= MAX_MAP_SECTION_OBJ) {
				infoex[RBOIndex].sTypeID = itr->typeID;
				infoex[RBOIndex].nX = itr->position.x;
				infoex[RBOIndex].nY = itr->position.y;
				infoex[RBOIndex].sYawAngle = itr->orientation.w;
				infoex[RBOIndex].sHeightOff = itr->position.z;
			}
			g_ObjFile.WriteSectionObjInfo(sectionNumber, infoex, sectionObjCount);
		}
		++itr;
	}
}

long CSceneObjFile::Init(const char* ptcsFileName, bool bSilence) {
	using ObjFileLoader = Corsairs::Engine::Scene::ObjFileLoader;

	long lRet = 1;
	long lFileSize = 0;
	char tcsPrint[256];
	FILE* fFile = nullptr;

	Free();

	_tchmod(ptcsFileName, _S_IWRITE);

	fFile = fopen(ptcsFileName, "rb");

	// Added by clp
	std::string filename(ptcsFileName);
	filename.erase(filename.length() - 3);
	_init_RBO(filename.c_str());

	if (fFile == nullptr) {
		if (CreateFile(ptcsFileName) == 0) {
			if (!bSilence) {
				_stprintf(tcsPrint, "%s %s %s", GetLanguageString(355).c_str(), ptcsFileName,
						  GetLanguageString(356).c_str());
				MessageBox(nullptr, tcsPrint, GetLanguageString(25).c_str(), 0);
			}
			lRet = 0;
			goto end;
		}
		fFile = _tfopen(ptcsFileName, "rb");
	}

	if (!ObjFileLoader::ReadHeader(fFile, m_SFileHead, lFileSize)) {
		lRet = 0;
		goto end;
	}
	if (m_SFileHead.lFileSize != lFileSize) {
		if (!bSilence) {
			_stprintf(tcsPrint, "%s %s", ptcsFileName, GetLanguageString(357).c_str());
			MessageBox(nullptr, tcsPrint, GetLanguageString(25).c_str(), 0);
		}
		lRet = 0;
		goto end;
	}
	if (m_SFileHead.lVersion == OBJ_FILE_VER500) //
	{
		fclose(fFile);
		if (ConvertObjFileVer(ptcsFileName) <= 0) //
		{
			lRet = 0;
			goto end;
		}

		fFile = _tfopen(ptcsFileName, "rb");
		if (fFile == NULL) {
			if (!bSilence) {
				_stprintf(tcsPrint, "%s %s", ptcsFileName, GetLanguageString(358).c_str());
				MessageBox(nullptr, tcsPrint, GetLanguageString(25).c_str(), 0);
			}
			lRet = 0;
			goto end;
		}

		if (!ObjFileLoader::ReadHeader(fFile, m_SFileHead, lFileSize)) {
			lRet = 0;
			goto end;
		}
	}
	if (m_SFileHead.lVersion != OBJ_FILE_VER600) {
		if (!bSilence) {
			_stprintf(tcsPrint, "%s %s", ptcsFileName, GetLanguageString(340).c_str());
			MessageBox(nullptr, tcsPrint, GetLanguageString(25).c_str(), 0);
		}
		lRet = 0;
		goto end;
	}

	m_SSectionIndex = new (SSectionIndex[m_SFileHead.iSectionCntX * m_SFileHead.iSectionCntY]);
	if (m_SSectionIndex == NULL) {
		lRet = 0;
		goto end;
	}
	if (!ObjFileLoader::ReadSectionIndexTable(fFile, m_SSectionIndex,
			static_cast<std::size_t>(m_SFileHead.iSectionCntX) * m_SFileHead.iSectionCntY)) {
		lRet = 0;
		goto end;
	}

	if (GlobalAppConfig.IsEditor())
		m_fRdWr = _tfopen(ptcsFileName, "r+b");
	else
		m_fRdWr = _tfopen(ptcsFileName, "rb");

	if (m_fRdWr == NULL) {
		if (!bSilence) {
			_stprintf(tcsPrint, "%s %s", ptcsFileName, GetLanguageString(339).c_str());
			MessageBox(nullptr, tcsPrint, GetLanguageString(25).c_str(), 0);
		}
		lRet = 0;
		goto end;
	}

	if (GlobalAppConfig.IsEditor())
		m_fAppend = _tfopen(ptcsFileName, "a+b");
	else
		m_fAppend = _tfopen(ptcsFileName, "rb");

	if (m_fAppend == NULL) {
		if (!bSilence) {
			_stprintf(tcsPrint, "%s %s", ptcsFileName, GetLanguageString(359).c_str());
			MessageBox(NULL, tcsPrint, GetLanguageString(25).c_str(), 0);
		}
		lRet = 0;
		goto end;
	}

	m_bInitSuccess = true;

end:
	if (lRet == 0) {
		if (m_fRdWr) {
			fclose(m_fRdWr);
			m_fRdWr = NULL;
		}
		if (m_fAppend) {
			fclose(m_fAppend);
			m_fAppend = NULL;
		}
		if (m_SSectionIndex) {
			delete [] m_SSectionIndex;
			m_SSectionIndex = NULL;
		}
	}
	if (fFile)
		fclose(fFile);

	return lRet;
}

void CSceneObjFile::Free(void) {
	m_bInitSuccess = false;
	if (m_fRdWr) {
		fclose(m_fRdWr);
		m_fRdWr = NULL;
	}
	if (m_fAppend) {
		fclose(m_fAppend);
		m_fAppend = NULL;
	}
	if (m_SSectionIndex) {
		delete [] m_SSectionIndex;
		m_SSectionIndex = NULL;
	}
}

long CSceneObjFile::CreateFile(const char* ptcsFileName,
							   int iSectionCntX, int iSectionCntY, int iSectionWidth,
							   int iSectionHeight, int iMaxSectionObjNum) {
	// В non-editor режиме старая логика отказывалась создавать файл (открывала
	// "rb" и сразу падала по NULL). ObjFileLoader::CreateEmpty вызывается только
	// из editor-пути (через Init), где режим уже подразумевается. Сохраняем
	// прежнее поведение через явную проверку.
	if (!GlobalAppConfig.IsEditor()) {
		return 0;
	}
	if (!Corsairs::Engine::Scene::ObjFileLoader::CreateEmpty(
			ptcsFileName, iSectionCntX, iSectionCntY,
			iSectionWidth, iSectionHeight, iMaxSectionObjNum)) {
		return 0;
	}
	return 1;
}

long CSceneObjFile::ConvertObjFileVer(const char* ptcsFileName, bool bBackUp) // 500600(section)
{
	if (!GlobalAppConfig.IsEditor()) {
		return -1;
	}
	char tcsPrint[256];
	_stprintf(tcsPrint, GetLanguageString(360).c_str(), ptcsFileName);
	MessageBox(NULL, tcsPrint, GetLanguageString(361).c_str(), 0);

	const long lRet = Corsairs::Engine::Scene::ObjFileLoader::ConvertVer500ToVer600(
		ptcsFileName, bBackUp);

	if (lRet == 2) {
		_stprintf(tcsPrint, GetLanguageString(362).c_str(), ptcsFileName,
				  bBackUp ? (std::string{ptcsFileName} + ".bak").c_str() : ptcsFileName);
		MessageBox(NULL, tcsPrint, GetLanguageString(363).c_str(), 0);
	}
	return lRet;
}

long CSceneObjFile::ReadSectionObjInfo(int nSectionNO, SSceneObjInfo* SSceneObj, long* lSectionObjNum) {
	using ObjFileLoader = Corsairs::Engine::Scene::ObjFileLoader;

	if (!m_bInitSuccess)
		return 0;

	if (nSectionNO >= m_SFileHead.iSectionCntX * m_SFileHead.iSectionCntY)
		return 0;

	if ((*lSectionObjNum = m_SSectionIndex[nSectionNO].iObjNum) > 0) {
		if (!ObjFileLoader::ReadSceneObjs(m_fRdWr, m_SSectionIndex[nSectionNO].lObjInfoPos,
				SSceneObj, m_SSectionIndex[nSectionNO].iObjNum)) {
			return 0;
		}
		// Координаты в файле относительные внутри секции; при чтении
		// разворачиваем обратно в мировые.
		const int sectionXBase = nSectionNO % m_SFileHead.iSectionCntX
								  * m_SFileHead.iSectionWidth * 100;
		const int sectionYBase = nSectionNO / m_SFileHead.iSectionCntX
								  * m_SFileHead.iSectionHeight * 100;
		for (int i = 0; i < m_SSectionIndex[nSectionNO].iObjNum; i++) {
			SSceneObj[i].nX += sectionXBase;
			SSceneObj[i].nY += sectionYBase;
			SSceneObjInfo* pObj = (SSceneObj + i);
			if (pObj->GetID() == 0) {
				g_logManager.InternalLog(LogLevel::Error, "errors", GetLanguageString(364));
			}
		}
	}

	return 1;
}

long CSceneObjFile::WriteSectionObjInfo(int nSectionNO, SSceneObjInfo* SSceneObj, long lSectionObjNum) {
	using ObjFileLoader = Corsairs::Engine::Scene::ObjFileLoader;

	if (!m_bInitSuccess)
		return 0;

	if (nSectionNO >= m_SFileHead.iSectionCntX * m_SFileHead.iSectionCntY)
		return 0;

	m_SSectionIndex[nSectionNO].iObjNum = lSectionObjNum;
	if (lSectionObjNum > 0) {
		FILE* fFile = nullptr;
		if (m_SSectionIndex[nSectionNO].lObjInfoPos > 0) {
			fFile = m_fRdWr;
			if (std::fseek(fFile, m_SSectionIndex[nSectionNO].lObjInfoPos, SEEK_SET) != 0) {
				return 0;
			}
		}
		else {
			m_SSectionIndex[nSectionNO].lObjInfoPos = m_SFileHead.lFileSize;
			m_SFileHead.lFileSize += sizeof(SSceneObjInfo) * m_SFileHead.iSectionObjNum;
			if (!ObjFileLoader::WriteHeader(m_fRdWr, m_SFileHead)) {
				return 0;
			}
			std::fflush(m_fRdWr);
			fFile = m_fAppend;
		}
		// Конвертация координат в относительные.
		const int sectionXBase = nSectionNO % m_SFileHead.iSectionCntX
								  * m_SFileHead.iSectionWidth * 100;
		const int sectionYBase = nSectionNO / m_SFileHead.iSectionCntX
								  * m_SFileHead.iSectionHeight * 100;
		for (int j = 0; j < m_SSectionIndex[nSectionNO].iObjNum; j++) {
			SSceneObj[j].nX -= sectionXBase;
			SSceneObj[j].nY -= sectionYBase;
		}
		// Пишем фиксированную заполненность iSectionObjNum (не реальное число
		// объектов): формат хранит «слот» под потенциальный максимум.
		if (!ObjFileLoader::WriteSceneObjs(fFile, -1, SSceneObj,
				static_cast<std::size_t>(m_SFileHead.iSectionObjNum))) {
			return 0;
		}
		std::fflush(fFile);
	}
	else {
		m_SSectionIndex[nSectionNO].lObjInfoPos = 0;
	}

	if (!ObjFileLoader::WriteSectionIndexEntry(m_fRdWr,
			static_cast<std::size_t>(nSectionNO),
			m_SSectionIndex[nSectionNO])) {
		return 0;
	}
	std::fflush(m_fRdWr);

	// Sanity check: размер файла должен совпасть с m_SFileHead.lFileSize.
	long lFileSize;
	std::fseek(m_fRdWr, 0, SEEK_END);
	lFileSize = std::ftell(m_fRdWr);
	if (lFileSize != m_SFileHead.lFileSize) {
		MessageBox(nullptr, GetLanguageString(365).c_str(), GetLanguageString(25).c_str(), 0);
	}

	return 1;
}

long CSceneObjFile::TrimFile(const char* ptcsFileName, bool bBackUp) {
	long lRet = 1;
	_TCHAR tcsBackUpName[_MAX_FNAME] = _TEXT("");
	long i;
	long lPos = 0;
	SFileHead SHead;
	long lMaxSectionNum;
	FILE *fFileOld = NULL, *fFileNew = NULL;
	char* pszSectionInfo = NULL;
	SSectionIndex* pSSectionIndex = NULL;
	unsigned long ulFileSize;

	_tcscpy(tcsBackUpName, ptcsFileName);
	for (i = 0; i < _MAX_FNAME - (long)_tcslen(ptcsFileName); i += 4) {
		_tcscat(tcsBackUpName, _TEXT(".bak"));
		fFileNew = _tfopen(tcsBackUpName, _TEXT("rb"));
		if (fFileNew == NULL)
			break;
		else {
			fclose(fFileNew);
			fFileNew = NULL;
		}
	}
	if (i >= _MAX_FNAME - (long)_tcslen(ptcsFileName)) {
		lRet = -1; // 
		goto end;
	}
	if (_trename(ptcsFileName, tcsBackUpName) != 0) // 
	{
		lRet = -2; // 
		goto end;
	}

	fFileOld = _tfopen(tcsBackUpName, _TEXT("rb"));
	if (fFileOld == NULL) {
		lRet = -3; // 
		goto end;
	}

	if (GlobalAppConfig.IsEditor())
		fFileNew = _tfopen(ptcsFileName, _TEXT("wb"));
	else
		fFileNew = _tfopen(ptcsFileName, _TEXT("rb"));
	if (fFileNew == NULL) {
		lRet = -3; // 
		goto end;
	}

	fseek(fFileOld, 0, SEEK_END);
	ulFileSize = ftell(fFileOld);
	fseek(fFileOld, 0, SEEK_SET);
	fread((void*)&SHead, sizeof(SFileHead), 1, fFileOld);
	if (_tcscmp(SHead.tcsTitle, _TEXT("HF Object File!")) != 0
		|| SHead.lVersion != OBJ_FILE_VER600) // || SHead.lFileSize != i)
	{
		lRet = -4; // 
		goto end;
	}

	lMaxSectionNum = SHead.iSectionCntX * SHead.iSectionCntY;
	pSSectionIndex = new (SSectionIndex[lMaxSectionNum]);
	if (pSSectionIndex == NULL) {
		lRet = -5; // 
		goto end;
	}
	fread((void*)pSSectionIndex, sizeof(SSectionIndex), lMaxSectionNum, fFileOld);
	pszSectionInfo = new (char[sizeof(SSceneObjInfo) * lMaxSectionNum]);
	if (pszSectionInfo == NULL) {
		lRet = -5;
		goto end;
	}

	fseek(fFileNew, sizeof(SFileHead) + sizeof(SSectionIndex) * lMaxSectionNum, SEEK_SET);

	for (i = 0; i < lMaxSectionNum && ulFileSize >= (unsigned long)ftell(fFileOld); i++) {
		if (pSSectionIndex[i].iObjNum > 0) //
		{
			fseek(fFileOld, pSSectionIndex[i].lObjInfoPos, SEEK_SET);
			fread(pszSectionInfo, sizeof(SSceneObjInfo) * SHead.iSectionObjNum, 1, fFileOld);
			pSSectionIndex[i].lObjInfoPos = ftell(fFileNew);
			fwrite(pszSectionInfo, sizeof(SSceneObjInfo) * SHead.iSectionObjNum, 1, fFileNew);
		}
		else {
			pSSectionIndex[i].lObjInfoPos = 0;
		}
	}

	SHead.lFileSize = ftell(fFileNew);
	fseek(fFileNew, 0, SEEK_SET);
	fwrite(&SHead, sizeof(SFileHead), 1, fFileNew);
	fwrite(pSSectionIndex, sizeof(SSectionIndex), lMaxSectionNum, fFileNew);

	if (!bBackUp) {
		fclose(fFileOld);
		fFileOld = NULL;
		_tremove(tcsBackUpName);
	}

end:
	if (fFileOld)
		fclose(fFileOld);
	if (fFileNew)
		fclose(fFileNew);
	if (pSSectionIndex)
		delete [] pSSectionIndex;
	if (pszSectionInfo)
		delete [] pszSectionInfo;
	return lRet;
}

long CSceneObjFile::TrimDirectory(const char* ptcsDirectory, bool bBackUp) {
	char tcsFileName[_MAX_FNAME], tcsPath[_MAX_PATH];
	size_t lLen;
	_finddata_t c_file;
	long hFile;
	const char* ptcsRecordFile = "TrimRecord.txt";
	FILE* fRecord = NULL;
	_timeb tTimeBuffer;
	_TCHAR tcsPrint[256];

	if (bBackUp)
		_stprintf(tcsPrint, GetLanguageString(366).c_str(), ptcsDirectory);
	else
		_stprintf(tcsPrint, GetLanguageString(367).c_str(), ptcsDirectory);
	if (IDYES != MessageBox(NULL, tcsPrint, GetLanguageString(361).c_str(), MB_YESNO))
		return 1;

	lLen = _tcslen(ptcsDirectory);
	_tcscpy(tcsPath, ptcsDirectory);
	if (tcsPath[lLen - 1] != '\\' || tcsPath[lLen - 1] != '/') {
		tcsPath[lLen] = '/';
		tcsPath[lLen + 1] = 0;
	}

	_stprintf(tcsFileName, "%s%s", tcsPath, ptcsRecordFile);
	if (GlobalAppConfig.IsEditor())
		fRecord = _tfopen(tcsFileName, "w");
	else
		fRecord = _tfopen(tcsFileName, "rb");
	if (fRecord == NULL)
		return 0;
	_ftime(&tTimeBuffer);
	_ftprintf(fRecord, GetLanguageString(368).c_str(), _tctime(&tTimeBuffer.time));

	_stprintf(tcsFileName, "%s%s", tcsPath, "*.obj");
	if ((hFile = (long)_tfindfirst(tcsFileName, &c_file)) == -1L) {
		fclose(fRecord);
		return 1;
	}
	else {
		if (!(c_file.attrib & _A_SUBDIR)) {
			_stprintf(tcsFileName, "%s%s", tcsPath, c_file.name);
			if (c_file.attrib & _A_RDONLY)
				_tchmod(tcsFileName, _S_IREAD | _S_IWRITE);
			_ftprintf(fRecord, "%s:\n", tcsFileName);
			switch (TrimFile(tcsFileName, bBackUp)) {
			case 1:
				_ftprintf(fRecord, GetLanguageString(369).c_str());
				break;
			case -1:
				_ftprintf(fRecord, GetLanguageString(370).c_str());
				break;
			case -2:
				_ftprintf(fRecord, GetLanguageString(371).c_str());
				break;
			case -3:
				_ftprintf(fRecord, GetLanguageString(372).c_str());
				break;
			case -4:
				_ftprintf(fRecord, GetLanguageString(373).c_str());
				break;
			case -5:
				_ftprintf(fRecord, GetLanguageString(374).c_str());
				break;
			default:
				_ftprintf(fRecord, GetLanguageString(375).c_str());
			}
		}
		while (_findnext(hFile, &c_file) == 0) {
			if (!(c_file.attrib & _A_SUBDIR)) {
				_stprintf(tcsFileName, "%s%s", tcsPath, c_file.name);
				if (c_file.attrib & _A_RDONLY)
					_tchmod(tcsFileName, _S_IREAD | _S_IWRITE);
				_ftprintf(fRecord, "%s:\n", tcsFileName);
				switch (TrimFile(tcsFileName, bBackUp)) {
				case 1:
					_ftprintf(fRecord, GetLanguageString(369).c_str());
					break;
				case -1:
					_ftprintf(fRecord, GetLanguageString(370).c_str());
					break;
				case -2:
					_ftprintf(fRecord, GetLanguageString(371).c_str());
					break;
				case -3:
					_ftprintf(fRecord, GetLanguageString(372).c_str());
					break;
				case -4:
					_ftprintf(fRecord, GetLanguageString(373).c_str());
					break;
				case -5:
					_ftprintf(fRecord, GetLanguageString(374).c_str());
					break;
				default:
					_ftprintf(fRecord, GetLanguageString(375).c_str());
				}
			}
		}

		_findclose(hFile);
	}

	if (fRecord)
		fclose(fRecord);
	return 1;
}
