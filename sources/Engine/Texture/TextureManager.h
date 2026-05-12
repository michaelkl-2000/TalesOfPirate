#pragma once

#include "MindPowerAPI.h"
#include "lwInterface.h"
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

// Менеджер текстур — замена MPTexSet (без наследования от CRawDataSet).
// Lazy loading: текстура загружается при первом обращении через GetTexture().
// DynamicRelease(): периодически выгружает неиспользуемые текстуры.
class TextureManager {
public:
	struct Entry {
		std::string path;
		std::uint8_t bAlpha{0};
		std::uint8_t btMipmap{5};
		std::int16_t sWidth{0};
		std::int16_t sHeight{0};
		Corsairs::Engine::Render::lwITex* pTex{nullptr};
		// LastUseTime — point на steady_clock. Используется только для
		// сравнения интервалов (DynamicRelease LRU), не для wall-clock.
		// Default-инициализация = epoch, проверка через сравнение
		// с time_point{}.
		std::chrono::steady_clock::time_point lastUseTime{};
		std::uint32_t dwLoadCnt{0};

		// Поведенческие политики (Phase 2 effect-textures migration).
		//
		// forceFormat: жёсткий override D3DFORMAT после всех эвристик
		//   в LoadTexture (раньше effect-текстуры лились через
		//   `lwLoadTex(..., D3DFMT_A8R8G8B8)` напрямую из CMPResManger).
		//   Sentinel D3DFMT_UNKNOWN = без override.
		// pinned: исключить из DynamicRelease(bClearAll=false). Effect-
		//   текстуры обращаются спорадически (паузы > 8s — норма), поэтому
		//   попадали под LRU и реgrузка лагала. pinned=true гарантирует
		//   что они отпускаются только при полном Clear.
		D3DFORMAT forceFormat{D3DFMT_UNKNOWN};
		bool      pinned{false};
	};

	TextureManager();
	~TextureManager();

	static TextureManager* I() {
		return _Instance;
	}

	// Получить ID по имени файла. Создаёт запись если нет.
	// forceFormat / pinned применяются ТОЛЬКО при первом GetOrCreateID;
	// при повторном вызове с тем же path политики игнорируются (запись
	// уже зарегистрирована).
	int GetOrCreateID(std::string_view path,
	                  D3DFORMAT forceFormat = D3DFMT_UNKNOWN,
	                  bool pinned = false);

	// Lazy load — загружает текстуру при первом обращении.
	Corsairs::Engine::Render::lwITex* GetTexture(int id, bool bRequest = false);

	// Convenience — возвращает DirectX текстуру.
	IDirect3DTextureX* GetD3DTexture(int id, bool bRequest = false);

	// Информация о текстуре.
	Entry* GetInfo(int id);
	Entry* GetInfo(const char* name);

	bool IsAlpha(int id);
	SIZE GetSize(int id);

	// Выгрузка неиспользуемых текстур.
	void DynamicRelease(bool bClearAll = false);

	// Перезагрузка всех текстур (смена качества).
	void ReloadAll();
	void SetLevel(int level);

	int GetLevel() const {
		return _nTextureLevel;
	}

	void SetReleaseInterval(std::chrono::milliseconds ms) {
		_releaseInterval = ms;
	}

private:
	Corsairs::Engine::Render::lwITex* LoadTexture(Entry& entry);
	void ReleaseTexture(Entry& entry);
	std::string ResolveTexturePath(const char* filename);
	bool IsMemoryFull();

	static TextureManager* _Instance;

	std::vector<Entry> _entries;
	std::unordered_map<std::string, int> _nameIndex;

	int _nTextureLevel;
	std::chrono::milliseconds _releaseInterval{8000};
	int _nLoadedCount{0};
};
