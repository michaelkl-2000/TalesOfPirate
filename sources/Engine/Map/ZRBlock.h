#pragma once

#include "assert.h"
#include <cstdint>
#include <memory>
#include <vector>

#define MAX_BLOCK_SECTION 512
#define MAX_BLOCK_RANGE 1024

namespace Corsairs::Engine::Render {
class MapStream;
} // namespace Corsairs::Engine::Render

//add by jze 2008.7.16
class ZRBlockData {
public:
	short sRegion{}; //
	BYTE btBlock[4] = {}; // 4
public:
	ZRBlockData() {
	}

	~ZRBlockData() {
	}

	BYTE IsBlock(BYTE no) const {
		if (btBlock[no] & 128) return 1;
		return 0;
	}

	void setBlock(BYTE no, BOOL bSet) {
		if (bSet) {
			btBlock[no] |= 128;
		}
		else {
			btBlock[no] &= 127;
		}
	}

	BOOL IsRegion(int nRegionNo) const {
		short s = 1;
		s <<= (nRegionNo - 1);
		return sRegion & s;
	}

	short GetRegionValue() const {
		return sRegion;
	}
};

//add by jze 2008.7.16
class ZRBlockSection {
public:
	std::unique_ptr<ZRBlockData[]> blockData{};
	int nX{}; // MapSection
	int nY{};
	DWORD dwDataOffset{}; //  = 0,
public:
	ZRBlockSection() = default;
	~ZRBlockSection() = default;
};

class ZRBlock {
public:
	ZRBlock();
	~ZRBlock();

	// Подключиться к уже открытому MPMap'ом MapStream'у. Кеширует геометрию
	// (sectionWidth/Height/cntX/cntY) и сбрасывает массив активных секций.
	// Указатель stream хранится без владения — MPMap живёт дольше ZRBlock'а.
	void Attach(Corsairs::Engine::Render::MapStream& stream);

	void GetBlockByRange(int CenterX, int CenterY, int range); //Block
	ZRBlockData* GetBlock(int nX, int nY); //Block
	BYTE IsGridBlock(int x, int y) const; // x,y
	short GetTileRegionAttr(int x, int y) const; // x,y
	void SetGrid(int GridX, int GridY);

private:
	std::unique_ptr<ZRBlockSection>& GetBlockSection(int nSectionX, int nSectionY); //block
	std::unique_ptr<ZRBlockSection>& LoadBlockData(int nSectionX, int nSectionY); //block
	void ClearSectionArray();

	void _LoadBlockData(ZRBlockSection& pSection); //block

public:
	BYTE _blockBuffer[MAX_BLOCK_RANGE][MAX_BLOCK_RANGE] = {};
	short _tileRegionAttr[MAX_BLOCK_SECTION][MAX_BLOCK_SECTION] = {};

private:
	// Flat layout: [sectionX * MAX_BLOCK_SECTION + sectionY]
	std::vector<std::unique_ptr<ZRBlockSection>> _sectionArray;

	std::unique_ptr<ZRBlockData> _defaultBlock; //block

	// Поток .map-файла принадлежит MPMap'у (`_stream`), а ZRBlock читает блочные
	// данные через MapLoader. Сырого fstream'а здесь больше нет — раньше ZRBlock
	// открывал тот же файл повторно, дублируя offset-таблицу/bulk-кеш и создавая
	// двойной r+b-handle в edit-режиме.
	Corsairs::Engine::Render::MapStream* _stream{nullptr};

	int _showCenterX{}; //
	int _showCenterY{};
	std::int32_t _sectionWidth{};  // Section
	std::int32_t _sectionHeight{};
	std::int32_t _sectionCntX{};  // Section
	std::int32_t _sectionCntY{};
	int _lastGridStartX{};
	int _lastGridStartY{};
	int _gridShowWidth{};
	int _gridShowHeight{};
};

inline BYTE ZRBlock::IsGridBlock(int x, int y) const //
{
	int offx = x - _lastGridStartX;
	int offy = y - _lastGridStartY;

	if (offx < 0 || offx >= _gridShowWidth) return 1;
	if (offy < 0 || offy >= _gridShowHeight) return 1;

	return _blockBuffer[offy][offx];
}

inline short ZRBlock::GetTileRegionAttr(int x, int y) const //
{
	int offx = x - _lastGridStartX / 2;
	int offy = y - _lastGridStartY / 2;

	if (offx < 0 || offx >= _gridShowWidth) return 0;
	if (offy < 0 || offy >= _gridShowHeight) return 0;

	return _tileRegionAttr[offy][offx];
}
