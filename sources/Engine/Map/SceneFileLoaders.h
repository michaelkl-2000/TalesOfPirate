#pragma once

// Лоадеры клиентских сцен-файлов: .obj (binary scene-object) и .rbo (text
// "really big object" sidecar). По смыслу параллельны движковому
// Corsairs::Engine::Render::MapLoader: всё файловое I/O живёт в Loader-классах,
// stateful-обёртки (CSceneObjFile, CGameScene) только поддерживают runtime-
// state и делегируют чтение/запись.
//
// Файл живёт в Engine — тулзы (PkoTool) подключают MindPower3D.lib и могут
// вызывать ObjFileLoader/RboLoader без зависимости от Client.

#include <cstdio>
#include <iosfwd>
#include <set>
#include <string>
#include <string_view>

#include <d3dx9math.h>  // D3DXVECTOR3 / D3DXQUATERNION для ReallyBigObjectInfo
#include <tchar.h>      // _TCHAR для SFileHead.tcsTitle

// =============================================================================
// POD-типы файлов сцены
// =============================================================================

// .obj формат версии 600 (текущая) и 500 (legacy, конвертируется при чтении).
#define OBJ_FILE_VER100     100
#define OBJ_FILE_VER200     200
#define OBJ_FILE_VER300     300
#define OBJ_FILE_VER400     400
#define OBJ_FILE_VER500     500
#define OBJ_FILE_VER600     600
#define MAX_MAP_SECTION_OBJ 25

// Заголовок .obj-файла.
struct SFileHead {
    _TCHAR tcsTitle[16]; // "HF Object File!"
    int lVersion;
    long lFileSize;

    int iSectionCntX;
    int iSectionCntY;
    int iSectionWidth;  // Tile
    int iSectionHeight; // Tile
    int iSectionObjNum;
};

// Запись в section-index таблице. Указывает на начало per-section
// SSceneObjInfo[iSectionObjNum] и реальное число валидных объектов.
struct SSectionIndex {
    long lObjInfoPos;
    int iObjNum;
};

struct SSceneObjInfo {
    short sTypeID; // 2type(0: , 1: ), ID
    int nX;
    int nY;
    short sHeightOff;
    short sYawAngle;
    short sScale;

    short GetType() {
        return sTypeID >> (sizeof(short) * 8 - 2);
    }

    short GetID() {
        return ~(0x0003 << (sizeof(short) * 8 - 2)) & sTypeID;
    }
};

// "Really Big Object": сцена-объект, чьё положение хранится отдельным sidecar-
// файлом .rbo и не утрамбовывается в section-таблицу .obj. Текстовый формат:
// одна запись на строку, поля через пробел, опциональный комментарий первой
// строки `\\…\n`.
typedef struct ReallyBigObjectInfo {
    int typeID;
    D3DXVECTOR3 position;
    D3DXQUATERNION orientation;
    float terrainHeight;
} ReallyBigObjectInfo;

std::ostream& operator<<(std::ostream& os, const ReallyBigObjectInfo& info);
std::istream& operator>>(std::istream& is, ReallyBigObjectInfo& info);
bool operator<(const ReallyBigObjectInfo& a, const ReallyBigObjectInfo& b);
void operator<<(std::FILE* file, const ReallyBigObjectInfo& info);

namespace Corsairs::Engine::Scene {

// =============================================================================
// .obj — binary scene-object file (HF Object File!)
//
// Layout:
//   [SFileHead]
//   [SSectionIndex × cntX*cntY]   — per section: lObjInfoPos / iObjNum
//   [SSceneObjInfo[iSectionObjNum]] — переменное число секций по lObjInfoPos
//
// Версии: 500 / 600 (текущая = 600). При Init версии 500 файл апгрейдится до
// 600 in-place через ConvertVer500ToVer600 (создаёт .bak, перезаписывает
// оригинал).
// =============================================================================

class ObjFileLoader {
public:
    static constexpr int kCurrentVersion = 600;
    static constexpr int kLegacyVersion500 = 500;
    static constexpr const char* kMagicTitle = "HF Object File!";

    // Прочитать SFileHead из открытого FILE*, делает fseek в начало, после
    // чтения file pointer стоит сразу за заголовком. Возвращает file_size в
    // outFileSize (уже прочитан через fseek+ftell, чтобы вызывающий мог
    // сравнить с header.lFileSize).
    [[nodiscard]] static bool ReadHeader(std::FILE* fp, ::SFileHead& out,
                                          long& outFileSize);
    [[nodiscard]] static bool WriteHeader(std::FILE* fp, const ::SFileHead& head);

    [[nodiscard]] static bool ReadSectionIndexTable(std::FILE* fp,
                                                     ::SSectionIndex* out, std::size_t count);
    [[nodiscard]] static bool WriteSectionIndexTable(std::FILE* fp,
                                                      const ::SSectionIndex* in, std::size_t count);
    [[nodiscard]] static bool WriteSectionIndexEntry(std::FILE* fp, std::size_t entryIndex,
                                                      const ::SSectionIndex& entry);

    [[nodiscard]] static bool ReadSceneObjs(std::FILE* fp, long offset,
                                             ::SSceneObjInfo* out, std::size_t count);
    [[nodiscard]] static bool WriteSceneObjs(std::FILE* fp, long offset,
                                              const ::SSceneObjInfo* in, std::size_t count);

    [[nodiscard]] static bool CreateEmpty(std::string_view file,
                                           int sectionCntX, int sectionCntY,
                                           int sectionWidth, int sectionHeight,
                                           int sectionObjNum);

    static long ConvertVer500ToVer600(std::string_view file, bool keepBackup);
};

// =============================================================================
// .rbo — text sidecar для CSceneObjFile, по одной записи ReallyBigObjectInfo
// на строку. Опциональный комментарий в первой строке начинается с `\\`.
// =============================================================================

class RboLoader {
public:
    [[nodiscard]] static bool Load(std::string_view file,
                                    std::set<::ReallyBigObjectInfo>& out);
    // Если items пуст — файл НЕ создаётся. См. историю в feedback (51 размер-0
    // .rbo в Client/map/ — последствие безусловного ofstream на пустом списке).
    [[nodiscard]] static bool Save(std::string_view file,
                                    const std::set<::ReallyBigObjectInfo>& items);
};

} // namespace Corsairs::Engine::Scene
