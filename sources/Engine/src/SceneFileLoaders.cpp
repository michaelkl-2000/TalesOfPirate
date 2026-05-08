// Реализация ObjFileLoader (.obj) и RboLoader (.rbo). Вынесено в Engine,
// чтобы и Game.exe, и тулзы (PkoTool) могли пользоваться одним и тем же
// I/O-кодом без дублирования.

#include "stdafx.h"
#include "SceneFileLoaders.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ios>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

// =============================================================================
// Операторы сериализации ReallyBigObjectInfo (текстовая форма для .rbo и
// std::set ordering).
// =============================================================================

std::ostream& operator<<(std::ostream& os, const ReallyBigObjectInfo& info) {
    os << info.typeID << ' ';
    os << info.position.x << ' ' << info.position.y << ' ' << info.position.z << ' ';
    os << info.orientation.w << ' ' << info.orientation.x << ' '
       << info.orientation.y << ' ' << info.orientation.z << ' ';
    os << info.terrainHeight;
    os << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, ReallyBigObjectInfo& info) {
    is >> info.typeID;
    is >> info.position.x >> info.position.y >> info.position.z;
    is >> info.orientation.w >> info.orientation.x
       >> info.orientation.y >> info.orientation.z;
    is >> info.terrainHeight;
    return is;
}

void operator<<(std::FILE* file, const ReallyBigObjectInfo& info) {
    std::fprintf(file, "%d %f %f %f %f %f %f %f %f\n",
                 info.typeID,
                 info.position.x,
                 info.position.y,
                 info.position.z,
                 info.orientation.w,
                 info.orientation.x,
                 info.orientation.y,
                 info.orientation.z,
                 info.terrainHeight);
}

bool operator<(const ReallyBigObjectInfo& a, const ReallyBigObjectInfo& b) {
    return (a.typeID < b.typeID
            || a.position.x < b.position.x
            || a.position.y < b.position.y
            || a.position.z < b.position.z
            || a.orientation.w < b.orientation.w
            || a.orientation.x < b.orientation.x
            || a.orientation.y < b.orientation.y
            || a.orientation.z < b.orientation.z
            || a.terrainHeight < b.terrainHeight);
}

namespace Corsairs::Engine::Scene {

namespace fs = std::filesystem;

namespace {

struct FileCloser {
    void operator()(std::FILE* fp) const noexcept {
        if (fp != nullptr) {
            std::fclose(fp);
        }
    }
};
using UniqueFile = std::unique_ptr<std::FILE, FileCloser>;

[[nodiscard]] long FileSizeAndRewind(std::FILE* fp) {
    if (std::fseek(fp, 0, SEEK_END) != 0) {
        return -1;
    }
    const long sz = std::ftell(fp);
    if (std::fseek(fp, 0, SEEK_SET) != 0) {
        return -1;
    }
    return sz;
}

} // namespace

// =============================================================================
// ObjFileLoader
// =============================================================================

bool ObjFileLoader::ReadHeader(std::FILE* fp, ::SFileHead& out, long& outFileSize) {
    outFileSize = FileSizeAndRewind(fp);
    if (outFileSize < static_cast<long>(sizeof(::SFileHead))) {
        return false;
    }
    return std::fread(&out, sizeof(::SFileHead), 1, fp) == 1;
}

bool ObjFileLoader::WriteHeader(std::FILE* fp, const ::SFileHead& head) {
    if (std::fseek(fp, 0, SEEK_SET) != 0) {
        return false;
    }
    return std::fwrite(&head, sizeof(::SFileHead), 1, fp) == 1;
}

bool ObjFileLoader::ReadSectionIndexTable(std::FILE* fp,
                                           ::SSectionIndex* out, std::size_t count) {
    if (count == 0) {
        return true;
    }
    return std::fread(out, sizeof(::SSectionIndex), count, fp) == count;
}

bool ObjFileLoader::WriteSectionIndexTable(std::FILE* fp,
                                            const ::SSectionIndex* in, std::size_t count) {
    if (count == 0) {
        return true;
    }
    return std::fwrite(in, sizeof(::SSectionIndex), count, fp) == count;
}

bool ObjFileLoader::WriteSectionIndexEntry(std::FILE* fp, std::size_t entryIndex,
                                            const ::SSectionIndex& entry) {
    const long off = static_cast<long>(sizeof(::SFileHead)
                                        + sizeof(::SSectionIndex) * entryIndex);
    if (std::fseek(fp, off, SEEK_SET) != 0) {
        return false;
    }
    return std::fwrite(&entry, sizeof(::SSectionIndex), 1, fp) == 1;
}

bool ObjFileLoader::ReadSceneObjs(std::FILE* fp, long offset,
                                   ::SSceneObjInfo* out, std::size_t count) {
    if (count == 0) {
        return true;
    }
    if (offset >= 0 && std::fseek(fp, offset, SEEK_SET) != 0) {
        return false;
    }
    return std::fread(out, sizeof(::SSceneObjInfo), count, fp) == count;
}

bool ObjFileLoader::WriteSceneObjs(std::FILE* fp, long offset,
                                    const ::SSceneObjInfo* in, std::size_t count) {
    if (count == 0) {
        return true;
    }
    if (offset >= 0 && std::fseek(fp, offset, SEEK_SET) != 0) {
        return false;
    }
    return std::fwrite(in, sizeof(::SSceneObjInfo), count, fp) == count;
}

bool ObjFileLoader::CreateEmpty(std::string_view file,
                                 int sectionCntX, int sectionCntY,
                                 int sectionWidth, int sectionHeight,
                                 int sectionObjNum) {
    UniqueFile fp{std::fopen(std::string{file}.c_str(), "wb")};
    if (!fp) {
        return false;
    }

    ::SFileHead head{};
    std::strncpy(head.tcsTitle, kMagicTitle, sizeof(head.tcsTitle) - 1);
    head.lVersion = kCurrentVersion;
    head.lFileSize = static_cast<long>(sizeof(::SFileHead)
                                        + sizeof(::SSectionIndex) * sectionCntX * sectionCntY);
    head.iSectionCntX = sectionCntX;
    head.iSectionCntY = sectionCntY;
    head.iSectionHeight = sectionHeight;
    head.iSectionWidth = sectionWidth;
    head.iSectionObjNum = sectionObjNum;

    if (std::fwrite(&head, sizeof(::SFileHead), 1, fp.get()) != 1) {
        return false;
    }

    const std::size_t total = static_cast<std::size_t>(sectionCntX) * sectionCntY;
    if (total > 0) {
        std::vector<::SSectionIndex> table(total);
        if (std::fwrite(table.data(), sizeof(::SSectionIndex), total, fp.get()) != total) {
            return false;
        }
    }
    return true;
}

long ObjFileLoader::ConvertVer500ToVer600(std::string_view file, bool keepBackup) {
    const std::string filePath{file};

    // Найти свободное .bak имя.
    std::string bakPath = filePath;
    {
        char tcsBuf[_MAX_FNAME] = "";
        std::strncpy(tcsBuf, filePath.c_str(), sizeof(tcsBuf) - 1);
        const std::size_t baseLen = std::strlen(tcsBuf);
        bool found = false;
        for (std::size_t i = 0; i + 4 < sizeof(tcsBuf) - baseLen; i += 4) {
            std::strncat(tcsBuf, ".bak", sizeof(tcsBuf) - std::strlen(tcsBuf) - 1);
            UniqueFile probe{std::fopen(tcsBuf, "rb")};
            if (!probe) {
                found = true;
                break;
            }
        }
        if (!found) {
            return -1;
        }
        bakPath = tcsBuf;
    }

    std::error_code ec;
    fs::rename(filePath, bakPath, ec);
    if (ec) {
        return -2;
    }

    UniqueFile fpOld{std::fopen(bakPath.c_str(), "rb")};
    if (!fpOld) {
        return -3;
    }
    UniqueFile fpNew{std::fopen(filePath.c_str(), "wb")};
    if (!fpNew) {
        return -1;
    }

    long fileSize = 0;
    ::SFileHead head{};
    if (!ReadHeader(fpOld.get(), head, fileSize)) {
        return -3;
    }
    if (head.lVersion != kLegacyVersion500) {
        return 1;
    }

    const std::size_t lMaxSectionNum = static_cast<std::size_t>(head.iSectionCntX) * head.iSectionCntY;
    std::vector<::SSectionIndex> idx(lMaxSectionNum);
    std::vector<::SSceneObjInfo> objs(head.iSectionObjNum);

    if (!ReadSectionIndexTable(fpOld.get(), idx.data(), lMaxSectionNum)) {
        return -4;
    }
    if (std::fseek(fpNew.get(),
                    static_cast<long>(sizeof(::SFileHead)
                                       + sizeof(::SSectionIndex) * lMaxSectionNum),
                    SEEK_SET) != 0) {
        return -4;
    }

    for (std::size_t i = 0;
         i < lMaxSectionNum
         && static_cast<long>(static_cast<unsigned long>(fileSize))
            >= std::ftell(fpOld.get());
         ++i) {
        if (idx[i].iObjNum > 0) {
            if (std::fseek(fpOld.get(), idx[i].lObjInfoPos, SEEK_SET) != 0) {
                return -4;
            }
            if (std::fread(objs.data(),
                            sizeof(::SSceneObjInfo) * head.iSectionObjNum,
                            1, fpOld.get()) != 1) {
                return -4;
            }
            for (long j = 0; j < idx[i].iObjNum; ++j) {
                const int nSectionX = static_cast<int>(i) % head.iSectionCntX
                                      * head.iSectionWidth * 100;
                const int nSectionY = static_cast<int>(i) / head.iSectionCntX
                                      * head.iSectionHeight * 100;
                objs[j].nX -= nSectionX;
                objs[j].nY -= nSectionY;
            }
            idx[i].lObjInfoPos = std::ftell(fpNew.get());
            if (std::fwrite(objs.data(),
                             sizeof(::SSceneObjInfo) * head.iSectionObjNum,
                             1, fpNew.get()) != 1) {
                return -4;
            }
        }
        else {
            idx[i].lObjInfoPos = 0;
        }
    }

    head.lVersion = kCurrentVersion;
    head.lFileSize = std::ftell(fpNew.get());
    if (!WriteHeader(fpNew.get(), head)) {
        return -4;
    }
    if (!WriteSectionIndexTable(fpNew.get(), idx.data(), lMaxSectionNum)) {
        return -4;
    }

    fpNew.reset();
    fpOld.reset();

    if (!keepBackup) {
        std::error_code ec2;
        fs::remove(bakPath, ec2);
    }
    return 2;
}

// =============================================================================
// RboLoader
// =============================================================================

bool RboLoader::Load(std::string_view file, std::set<::ReallyBigObjectInfo>& out) {
    out.clear();

    std::ifstream is(std::string{file}.c_str());
    if (!is.is_open()) {
        // Отсутствие файла — это нормальный случай.
        return true;
    }

    const char c1 = static_cast<char>(is.get());
    const char c2 = static_cast<char>(is.get());
    if (c1 == '\\' && c2 == '\\') {
        std::string tmp;
        std::getline(is, tmp, '\n');
    }
    else {
        is.putback(c2);
        is.putback(c1);
    }

    ::ReallyBigObjectInfo info;
    while (is >> info) {
        out.insert(info);
    }
    return true;
}

bool RboLoader::Save(std::string_view file, const std::set<::ReallyBigObjectInfo>& items) {
    if (items.empty()) {
        return true;
    }

    std::ofstream os(std::string{file}.c_str());
    if (!os.is_open()) {
        return false;
    }
    for (const auto& info : items) {
        os << info;
    }
    return os.good();
}

} // namespace Corsairs::Engine::Scene
