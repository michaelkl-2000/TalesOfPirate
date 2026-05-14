#include "stdafx.h"
#include "ZRBlock.h"

#include "AssetLoaders.h"  // Corsairs::Engine::Render::{MapStream, MapLoader}

ZRBlock::ZRBlock()
	: _sectionArray(MAX_BLOCK_SECTION * MAX_BLOCK_SECTION)
	  , _defaultBlock(std::make_unique<ZRBlockData>()) {
}

ZRBlock::~ZRBlock() = default;

void ZRBlock::Attach(Corsairs::Engine::Render::MapStream& stream) {
	_stream = &stream;
	const auto& header = stream.Header();
	_sectionWidth = header.SectionWidth;
	_sectionHeight = header.SectionHeight;
	_sectionCntX = stream.SectionCountX();
	_sectionCntY = stream.SectionCountY();
	ClearSectionArray();
}

void ZRBlock::GetBlockByRange(int CenterX, int CenterY, int range) {
	if (_stream == nullptr || !_stream->IsOpen()) {
		return;
	}

	_gridShowWidth = range * 2;
	_gridShowHeight = range * 2;

	_showCenterX = CenterX;
	_showCenterY = CenterY;

	Corsairs::Util::MPTimer t;
	t.Begin();
	int nCurSectionX = (int)(_showCenterX - (float)range / 2.0f) / _sectionWidth;
	int nCurSectionY = (int)(_showCenterY - (float)range / 2.0f) / _sectionHeight;

	int nEndSectionX = (int)(_showCenterX + (float)range / 2.0f) / _sectionWidth;
	int nEndSectionY = (int)(_showCenterY + (float)range / 2.0f) / _sectionHeight;

	int nShowSectionCntX = nEndSectionX - nCurSectionX;
	int nShowSectionCntY = nEndSectionY - nCurSectionY;

	if (range % _sectionWidth != 0) {
		nShowSectionCntX++;
	}
	if (range % _sectionHeight != 0) nShowSectionCntY++;

	for (int y = 0; y < nShowSectionCntY; y++) {
		int nSectionY = nCurSectionY + y;

		if (nSectionY < 0 || nSectionY >= _sectionCntY) {
			continue;
		}
		for (int x = 0; x < nShowSectionCntX; x++) {
			int nSectionX = nCurSectionX + x;

			if (nSectionX < 0 || nSectionX >= _sectionCntX) {
				continue;
			}

			if (!GetBlockSection(nSectionX, nSectionY)) {
				LoadBlockData(nSectionX, nSectionY);
			}
		}
	}
}

std::unique_ptr<ZRBlockSection>& ZRBlock::GetBlockSection(int nSectionX, int nSectionY) {
	return _sectionArray[nSectionX * MAX_BLOCK_SECTION + nSectionY];
}

std::unique_ptr<ZRBlockSection>& ZRBlock::LoadBlockData(int nSectionX, int nSectionY) {
	auto block = std::make_unique<ZRBlockSection>();
	block->nX = nSectionX;
	block->nY = nSectionY;

	_LoadBlockData(*block);
	auto& slot = _sectionArray[nSectionX * MAX_BLOCK_SECTION + nSectionY];
	slot = std::move(block);

	return slot;
}


void ZRBlock::_LoadBlockData(ZRBlockSection& block) {
	if (_stream == nullptr) {
		return;
	}

	const std::uint32_t off = _stream->SectionOffset(block.nX, block.nY);
	block.dwDataOffset = off;
	if (off == 0) {
		return;
	}

	block.blockData = std::make_unique<ZRBlockData[]>(_sectionWidth * _sectionHeight);

	if (LW_FAILED(Corsairs::Engine::Render::MapLoader::ReadSectionBlockData(
			*_stream, block.nX, block.nY, block.blockData.get()))) {
		ToLogService("errors", LogLevel::Error,
					 "[ZRBlock::_LoadBlockData] ReadSectionBlockData failed for section [{},{}]",
					 block.nX, block.nY);
	}
}


ZRBlockData* ZRBlock::GetBlock(int nX, int nY) {
	if (_sectionWidth == 0 || _sectionHeight == 0) {
		return _defaultBlock.get();
	}
	const int width = _sectionCntX * _sectionWidth;
	const int height = _sectionCntY * _sectionHeight;
	if (nX >= width || nY >= height || nX < 0 || nY < 0) {
		return _defaultBlock.get();
	}

	int nSectionX = nX / _sectionWidth;
	int nSectionY = nY / _sectionHeight;

	auto& pBlock = GetBlockSection(nSectionX, nSectionY);

	if (pBlock && pBlock->blockData) {
		int nOffX = nX % _sectionWidth;
		int nOffY = nY % _sectionHeight;
		return pBlock->blockData.get() + nOffY * _sectionWidth + nOffX;
	}

	return _defaultBlock.get();
}

void ZRBlock::SetGrid(int GridX, int GridY) {
	_lastGridStartX = GridX;
	_lastGridStartY = GridY;
}

void ZRBlock::ClearSectionArray() {
	for (auto& section : _sectionArray) {
		section.reset();
	}
}
